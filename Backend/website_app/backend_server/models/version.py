from typing import Dict, List, Optional
from functools import total_ordering
from mongoengine import Document, StringField, ListField, IntField

@total_ordering
class Version(Document):
    """Class representing a firmware version"""
    version_number = StringField(required=True)
    major_version = IntField(required=True)
    minor_version = IntField(required=True)
    patch_version = IntField(required=True)
    compatible_car_types = ListField(StringField(), required=True)
    hex_file_path = StringField(required=True, unique=True)

    def __eq__(self, other):
        if not isinstance(other, Version):
            return NotImplemented
        return (self.major_version, self.minor_version, self.patch_version) == (other.major_version, other.minor_version, other.patch_version)
    def __lt__(self, other):
        if not isinstance(other, Version):
            return NotImplemented
        return (self.major_version, self.minor_version, self.patch_version) < (other.major_version, other.minor_version, other.patch_version)

    meta = {
        'collection': 'versions',
        'indexes': ['version_number', 'hex_file_path']
    }