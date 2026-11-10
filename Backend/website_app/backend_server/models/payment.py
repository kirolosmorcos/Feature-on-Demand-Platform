from typing import Dict, List, Optional
from datetime import datetime, timezone
from mongoengine import Document, StringField, FloatField, DateTimeField ,EnumField, ReferenceField
from .enums import Currency, PaymentStatus


class PaymentTransaction(Document):
   transaction_id = StringField(required=True, unique=True)
   car_id = StringField(required=True)
   product_code = StringField(required=True)

   license = ReferenceField('FeatureLicense', required=True)
 
   amount = FloatField(required=True)
   currency = EnumField(Currency, required=True, default=Currency.USD)
 
   status = EnumField(PaymentStatus, required=True) #issue!
 
   timestamp = DateTimeField(default=lambda: datetime.now(timezone.utc))
 
   meta = {
        "collection": "payment_transactions",
        "indexes": [
            {"fields": ["transaction_id"], "unique": True},
            ("car_id", "product_code"),
            ("car_id", "status"),
        ],
   }