from typing import Dict, List, Optional
from mongoengine import Document, EmbeddedDocument, EmbeddedDocumentField, EnumField, StringField, FloatField, ListField, BooleanField, ReferenceField
from .enums import PricingType, SubscriptionPeriod, Currency
class Feature(Document):
   feature_code = StringField(required=True, unique=True)
   name = StringField(required=True)
 
   meta = {
      'collection': 'features',
      'indexes': ['feature_code'],
   }
class FeatureProduct(Document):
   """
   Defines how a feature is sold.
   OEM-owned, not user-defined.
   """
   product_code = StringField(required=True, unique=True)
   feature = ReferenceField('Feature', required=True)
 
   pricing_type = EnumField(PricingType, required=True)
 
   price = FloatField(required=True)
   currency = EnumField(Currency, required=True, default=Currency.USD)
 
   subscription_period = EnumField(SubscriptionPeriod, required=False)
 
 
   meta = {
      'collection': 'feature_products',
      'indexes': [
         'product_code',
         'feature',
      ],
   }
class FeatureEcuRequirement(Document):
   feature = ReferenceField('Feature', required=True)
   ecu = ReferenceField('ECU', required=True)
   first_supported_version = ReferenceField('Version', required=True)
 
   meta = {
       'collection': 'feature_ecu_requirements',
       'indexes': [
         ('feature', 'ecu'),
       ],
   }


