import os
import json
from datetime import timezone, datetime

from azure.servicebus import ServiceBusClient, ServiceBusMessage

from models.feature_license import FeatureLicense
from models.download import DownloadRequest
from models.enums import LicenseStatus
from services.signing_service import SigningService

from azure.iot.hub import IoTHubRegistryManager
import os

import json


class DeactivationService :

    QUEUE_NAME = os.getenv("BUS_QUEUE_NAME", "license-expiration-queue")
    TERMINAL_STATUSES = {LicenseStatus.EXPIRED, LicenseStatus.REVOKED}
    EXPIRABLE_STATUSES = {LicenseStatus.ENTITLED, LicenseStatus.ACTIVE}

    def schedule_license_expiration(self, feature_license):
        connection_str = os.getenv("SERVICE_BUS_CONNECTION")
        if not connection_str:
            raise ValueError("SERVICE_BUS_CONNECTION is not configured")
        try:
            if not feature_license.valid_until:
                raise ValueError("feature_license.valid_until is missing")

            valid_until = feature_license.valid_until

            # Make sure valid_until is timezone-aware UTC
            if valid_until.tzinfo is None:
                valid_until = valid_until.replace(tzinfo=timezone.utc)

            message_body = json.dumps({
                "license_id": str(feature_license.id)
            })

            servicebus_client = ServiceBusClient.from_connection_string(connection_str)

            with servicebus_client:
                sender = servicebus_client.get_queue_sender(queue_name=self.QUEUE_NAME)
                with sender:
                    sender.schedule_messages(
                        messages=ServiceBusMessage(message_body),
                        schedule_time_utc=valid_until
                    )
        except Exception as e:
            print(f"Error occurred while scheduling license expiration: {e}")
            raise e
        except ValueError as ve:
            print(f"Validation error: {ve}")
            raise ve    
        

    def expire_license(self, license_id):
        feature_license = FeatureLicense.objects(id=license_id).first()

        if not feature_license:
            return {
                "status": "ignored",
                "reason": "license_not_found"
            }

        if feature_license.license_status in self.TERMINAL_STATUSES:
            return {
                "status": "ignored",
                "reason": "already_terminal",
                "license_status": feature_license.license_status.value
            }

        if feature_license.license_status not in self.EXPIRABLE_STATUSES:
            return {
                "status": "ignored",
                "reason": "status_not_expirable",
                "license_status": feature_license.license_status.value
            }

        if not feature_license.valid_until:
            return {
                "status": "ignored",
                "reason": "missing_valid_until"
            }

        feature_license.license_status = LicenseStatus.EXPIRED
        feature_license.save()
        self .send_disable_feature_message(feature_license)


        return {
            "status": "processed",
            "license_id": str(feature_license.id),
            "new_license_status": LicenseStatus.EXPIRED.value
        }
    
    
    def __init__(self):
        self.connection_string = os.getenv("IOTHUB_CONNECTION_STRING")
        self.registry_manager = IoTHubRegistryManager(self.connection_string)

    def twin_disable_feature(self, car_id, feature_code):
        twin_patch = {
            "properties": {
                "desired": {
                    "features": {
                        feature_code: False
                    }
                }
            }
        }

        self.registry_manager.update_twin(car_id, twin_patch, "*")
        print(f"Feature {feature_code} disabled for {car_id}")

    def send_disable_feature_message(self, feature_license: FeatureLicense):
        try:
            signing_service = SigningService()
            for ecu_token in feature_license.ecu_tokens:
        
                signing_service.sign_token(ecu_token['license_token'])
            payload = json.dumps({
                "action": "disable",
                "feature_code": feature_license.feature_code,
                "ecu_tokens": feature_license.ecu_tokens
            }, default=str)  # default=str to handle ObjectId serialization

            # ✅ Send string directly
            self.registry_manager.send_c2d_message(feature_license.car_id, payload)

            print(f"Message sent: {payload}")

        except Exception as e:
            print("Error:", str(e))

    def send_active_dota_message(self, car_id):
        try:
            payload = json.dumps({
                "action": "active_dota",
               
            })

            # ✅ Send string directly
            self.registry_manager.send_c2d_message(car_id, payload)

            print(f"Message sent: {payload}")

        except Exception as e:
            print("Error:", str(e))
    
  