from typing import Dict, List, Optional
from .enums import DownloadStatus, ServiceType, RequestStatus
from datetime import datetime, timezone
from mongoengine import Document, StringField, IntField, ListField, DictField, DateTimeField, EnumField, ReferenceField

class ServiceRequest(Document):
    """Class representing a service request"""
    timestamp = DateTimeField(default=lambda: datetime.now(timezone.utc))
    car_type = StringField(required=True)
    car_id = StringField(required=True)
    ip_address = StringField(required=True)
    port = IntField(required=True)
    service_type = EnumField(ServiceType, required=True)
    metadata = DictField(required=True)
    status = EnumField(RequestStatus, required=True)

    meta={
        'collection': 'service_requests',
        'indexes': ['car_type', 'car_id', 'status', 'service_type']
    }



class DownloadRequest(Document):
    """Class representing a firmware download request"""
    
    timestamp=DateTimeField(default=lambda: datetime.now(timezone.utc))
    car_type=StringField(required=True)
    car_id=StringField(required=True)
    ip_address=StringField(required=True)
    port=IntField(required=True)
    required_versions=ListField(DictField(), required=True)
    old_versions=ListField(DictField(), required=True)
    status=EnumField(DownloadStatus, required=True)
    total_size=IntField(required=True)
    transferred_size=IntField(default=0)
    active_transfers=DictField(required=True)
    files = ListField(DictField(), required=True) # List of dicts with file info

    feature_code = StringField()
    license = ReferenceField('FeatureLicense')
    meta={
        'collection': 'download_requests',
        'indexes': ['car_type', 'car_id', 'status']
    }
