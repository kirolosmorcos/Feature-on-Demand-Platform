from typing import Dict, List, Optional
from mongoengine import Document, StringField, ListField,ReferenceField
#from .version import Version

class ECU(Document):
    """Class representing an Electronic Control Unit"""
    name=StringField(required=True)
    model_number=StringField(required=True)
    versions=ListField(ReferenceField('Version'))
    meta = {
        'collection': 'ecus',
        'indexes': [{
            'fields': ['name', 'model_number'],
            'unique': True
        }]
    }