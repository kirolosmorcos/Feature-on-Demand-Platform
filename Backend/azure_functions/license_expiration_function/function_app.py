import json
import logging
import os

import azure.functions as func
import requests

app = func.FunctionApp()


@app.function_name(name="LicenseExpirationTrigger")
@app.service_bus_queue_trigger(
    arg_name="msg",
    queue_name="license-expiration-queue",
    connection="AzureWebJobsServiceBus"
)
def license_expiration_trigger(msg: func.ServiceBusMessage):
    try:
        body = msg.get_body().decode("utf-8")
        logging.info(f"Received Service Bus message: {body}")

        data = json.loads(body)

        license_id = data.get("license_id")
        if not license_id:
            raise ValueError("license_id is missing from Service Bus message")

        backend_url = os.environ.get("BACKEND_INTERNAL_URL")
        internal_secret = os.environ.get("INTERNAL_SECRET")

        if not backend_url:
            raise ValueError("BACKEND_INTERNAL_URL is not configured")

        if not internal_secret:
            raise ValueError("INTERNAL_SECRET is not configured")

        headers = {
            "Content-Type": "application/json",
            "X-INTERNAL-KEY": internal_secret
        }

        payload = {
            "license_id": license_id
        }

        response = requests.post(
            backend_url,
            json=payload,
            headers=headers,
            timeout=30
        )

        logging.info(f"Backend response status: {response.status_code}")
        logging.info(f"Backend response body: {response.text}")

        response.raise_for_status()

    except Exception as e:
        logging.exception(f"Failed to process license expiration message: {str(e)}")
        raise