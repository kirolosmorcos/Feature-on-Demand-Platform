from typing import List, Optional, Dict 
from flask import  jsonify
import os
import os
from typing import List, Optional, Dict
from mongoengine.errors import NotUniqueError
import uuid
from datetime import datetime, timezone, timedelta

from models import Car, FeatureProduct, FeatureLicense, PaymentTransaction, Feature, FeatureEcuRequirement
from models.enums import PricingType, SubscriptionPeriod, Currency  , LicenseStatus, PaymentStatus
from .deactivation_service import DeactivationService



class PaymentService:
    
    def purchase_feature (self ,data: dict) :

        car_id = data["car_id"]
        product_code = data["product_code"]
        feature_code = data["feature_code"]

        car = self.validate_car(car_id)
        feature = self.validate_feature(feature_code)
        product = self.validate_product(product_code, feature)
       

        existing_transaction = self.check_pending_payment(car_id, product_code)
        if existing_transaction:
            return self.payment_url(existing_transaction)
        
        license = self.create_feature_license(car_id, feature, product)

        try:
          transaction = self.create_payment_transaction(car_id, product,license)
          return self.payment_url(transaction)
        
        except Exception as e:
            if license.license_status == LicenseStatus.PENDING_PAYMENT:
                license.delete()
            raise ValueError(f"Failed to create payment transaction: {str(e)}")
    
    
    

    def validate_car(self, car_id: str) -> Car:
        car = Car.objects(car_id=car_id ,is_active=True ).first()
        if not car:
            raise ValueError(f"Car with ID '{car_id}' does not exist")
        return car
    
    def validate_product(self, product_code: str, feature: str) -> FeatureProduct:
        product = FeatureProduct.objects(product_code=product_code, feature=feature).first()
        if not product:
            raise ValueError(f"Product with code '{product_code}' for feature '{feature.feature_code}' does not exist")
        return product
    
    def validate_feature(self, feature_code: str) -> Feature:
        feature = Feature.objects(feature_code=feature_code).first()
        if not feature:
            raise ValueError(f"Feature with code '{feature_code}' does not exist")
        return feature
    
    def check_pending_payment(self, car_id: str, product_code: str) -> Optional[PaymentTransaction]:
        return PaymentTransaction.objects(car_id=car_id, product_code=product_code, status="pending").first()
    
    def create_feature_license(self, car_id: str, feature: Feature, product: FeatureProduct) -> FeatureLicense:

        existing_license = FeatureLicense.objects(car_id=car_id, 
                                                  feature=feature,license_status__in=["active","entitled","pending_payment"]).first()
        if existing_license:
            if existing_license.license_status ==  LicenseStatus.PENDING_PAYMENT:
                return existing_license
            raise ValueError("An active license already exists for this feature and car")                                           
        

        try:
            license = FeatureLicense(
                car_id=car_id,
                feature=feature,
                product=product,
                feature_code=feature.feature_code, #added for easier querying -John-
                product_code=product.product_code,
                license_status=LicenseStatus.PENDING_PAYMENT
            )
            license.save()
            return license
        except NotUniqueError:
            raise ValueError("An active license already exists for this feature and car")
        except Exception as e:
            raise ValueError(f"Failed to create feature license: {str(e)}")
        
    def create_payment_transaction(self, car_id: str, product: FeatureProduct, license: FeatureLicense) -> PaymentTransaction:

        try:
            transaction = PaymentTransaction(
                transaction_id=str(uuid.uuid4()),
                car_id=car_id,
                product_code=product.product_code,
                license=license,
                amount=product.price,
                currency=Currency.USD,  # Assuming USD for simplicity
                status="pending"
            )
            transaction.save()
            return transaction
        except NotUniqueError:
          raise ValueError("Transaction ID collision. Please retry.")
        
    def payment_url(self, transaction: PaymentTransaction) -> str:

         return (
        "https://ramezemad02.github.io/Payment/pay.html"
        f"?tx={transaction.transaction_id}"
        f"&amount={transaction.amount}"
       )   
        
    def handle_payment_webhook(self, data: dict) -> Dict[str, str]:

        now = datetime.now(timezone.utc)

        transaction_id = data.get('transaction_id')
        amount = data.get('amount')
        currency = data.get('currency', Currency.USD.value)  # Default to USD if not provided
        status = data.get('status')

        ISSUER_ID = os.getenv("ISSUER_ID", 1)


        payment_transaction = PaymentTransaction.objects(transaction_id=transaction_id).first()
        if not payment_transaction:
            raise ValueError(f"PaymentTransaction with ID '{transaction_id}' not found")
        
        product_code = payment_transaction.product_code
        car_id = payment_transaction.car_id
        feature_product = FeatureProduct.objects(product_code=product_code).first()
        if not feature_product:
            raise ValueError(f"FeatureProduct with code '{product_code}' not found")
        if payment_transaction.status != PaymentStatus.PENDING:
            raise ValueError("PaymentTransaction not pending")
        if payment_transaction.amount != amount:
            raise ValueError("Amount mismatch")
        if payment_transaction.currency.value != currency:
            raise ValueError(f"Currency mismatch: received {payment_transaction.currency}, expected {currency}")


        feature_license = FeatureLicense.objects(car_id=car_id, product_code=product_code, license_status=LicenseStatus.PENDING_PAYMENT).first()
        if not feature_license:
            raise ValueError(f"FeatureLicense for car '{car_id}' and product '{product_code}' not found")
        if not feature_license.feature:
            raise ValueError("Associated Feature not found")

        if status != PaymentStatus.SUCCESS.value:
            payment_transaction.status = PaymentStatus.FAILED
            feature_license.license_status = LicenseStatus.PAYMENT_FAILED
            feature_license.save()
            payment_transaction.save()
            raise ValueError("Payment failed")
        


        feature_license.license_status = LicenseStatus.ENTITLED
        feature_license.valid_from = now
        if feature_product.pricing_type == PricingType.ONE_TIME:
            feature_license.valid_until = None
        elif feature_product.pricing_type == PricingType.SUBSCRIPTION:

            if not feature_product.subscription_period:
                raise ValueError("Subscription period not defined for subscription product")

            if feature_product.subscription_period == SubscriptionPeriod.MONTHLY:
                feature_license.valid_until = now + timedelta(minutes=5)  # For testing purposes, set to 2 minutes. Change to days=30 for production.   
            elif feature_product.subscription_period == SubscriptionPeriod.YEARLY:
                feature_license.valid_until = now + timedelta(minutes=20) # For testing purposes, set to 20 minutes. Change to days=365 for production.

        else:
            raise ValueError("Invalid pricing type")

        ecu_tokens = []

        feature_ecu_requirement = FeatureEcuRequirement.objects(feature=feature_license.feature).all()

        for req in feature_ecu_requirement:
            ecu_token = {
                "ecu_name": str(req.ecu.name),
                "license_token": {
                    "license_id": feature_license.id,
                    "ecu_id": req.ecu.id,
                    "feature_id": feature_license.feature.id,
                    "valid_from": feature_license.valid_from,
                    "valid_until": feature_license.valid_until if feature_license.valid_until else None,
                    "issuer_id": int(ISSUER_ID)
                }
            }
            ecu_tokens.append(ecu_token)
        
        feature_license.ecu_tokens = ecu_tokens
        
        payment_transaction.status = PaymentStatus.SUCCESS
        payment_transaction.save()
        feature_license.save()

        if feature_license.valid_until is not None:
            try:
                deactivation_service = DeactivationService()
                deactivation_service.schedule_license_expiration(feature_license)
            except Exception as e:
                return jsonify({
                    'error': f'Payment succeeded, but expiration scheduling failed: {str(e)}'
                }), 500

        return ({'message': 'Payment processed successfully'})

