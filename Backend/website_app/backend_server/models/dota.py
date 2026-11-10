from typing import Dict, List, Optional
from .enums import VehicleHealth
from datetime import datetime, timezone
from mongoengine import Document, StringField, IntField, ListField, DictField, DateTimeField, EnumField, ReferenceField

class ECUSnapShot(Document):
    """Class representing a snapshot of an ECU's data"""
    timestamp = DateTimeField(default=lambda: datetime.now(timezone.utc))
    ecu_name = StringField(required=True)
    ecu_id = StringField()
    sensor_data = DictField(required=True)

    meta={
        'collection': 'ecu_snapshots',
        'indexes': ['ecu_name', 'timestamp']
    }

class VehicleHealthSnapshot(Document):
    """Class representing a snapshot of a vehicle's health data"""
    timestamp = DateTimeField(default=lambda: datetime.now(timezone.utc))
    car_type = StringField(required=True) #will be needed for on demand diagnostics
    car_id = StringField(required=True)
    ecu_snapshots = ListField(ReferenceField('ECUSnapShot'))
    overall_health = EnumField(VehicleHealth, required=True)

    meta={
        'collection': 'vehicle_health_snapshots',
        'indexes': ['car_type', 'car_id', 'timestamp']
    }
