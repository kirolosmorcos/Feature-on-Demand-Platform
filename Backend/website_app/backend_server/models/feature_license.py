from typing import Dict, List, Optional
from datetime import datetime, timezone
from .enums import LicenseStatus
from mongoengine import Document, StringField, DateTimeField, ReferenceField, EnumField, ListField, DictField
class FeatureLicense(Document):
    """
    Business entitlement per vehicle.
    Source of truth for backend decisions.
    """
    car_id = StringField(required=True)
    feature_code = StringField()
    product_code = StringField()
    feature = ReferenceField('Feature', required=True)
    product = ReferenceField('FeatureProduct', required=True)
 
    license_status = EnumField(LicenseStatus, required=True)
 
    ecu_tokens = ListField(DictField())
 
    valid_from = DateTimeField()
    valid_until = DateTimeField()
 

    meta = {
        'collection': 'feature_licenses',
        'indexes': [
            
            {
                'fields': ['license_status', 'valid_until']
            },
            {
                'fields': ['license_status']
            }
        ]
    }