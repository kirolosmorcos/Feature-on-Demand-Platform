from typing import Dict, List, Optional
#from .enums import Enum, VehicleHealth
#from .ecu import ECU
from datetime import datetime, timezone

# from django.forms import BooleanField
from mongoengine import Document, StringField, ListField, DateTimeField, ReferenceField, BooleanField

class Car (Document):
    car_id = StringField(required=True, unique=True)  # VIN or unique vehicle ID
        
    car_type = ReferenceField('CarType', required=True)
        
    # api_key_hash = StringField(required=True)
    # activation_code = StringField(required=True)
    certificate_serial = StringField(required=True)

    is_active = BooleanField(default=True)
    
    created_at = DateTimeField(default=lambda: datetime.now(timezone.utc))

    meta = {
            "collection": "cars",
            "indexes": [
                "car_id",
                "car_type",
                "is_active"
            ]
        }

class CarType(Document):
    """Class representing a car type"""
    name = StringField(required=True)
    model_number = StringField(required=True)
    ecus = ListField(ReferenceField('ECU'))
    car_ids = ListField(StringField(), required=False)
    # ecu_ids = ListField(StringField())   redundant with ecus field, can be removed
    # manufactured_count = IntField(default=0)
    #car_ids = ListField(ReferenceField('Car', reverse_delete_rule=PULL), required=False)  #very large list, not needed here, can be tracked in Car collection instead (comment later)
    
    meta = {
        'collection': 'car_types',
        'indexes': [{
            'fields': ['name', 'model_number'],
            'unique': True
        }]
    }

