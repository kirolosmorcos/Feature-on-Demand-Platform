from typing import Dict, List, Optional
from mongoengine import Document, StringField, DateTimeField, EnumField
from .enums import ClearanceLevel
from datetime import datetime

class User(Document):
    email = StringField(required=True, unique=True)
    password = StringField(required=True)
    name = StringField(required=True)
    created_at = DateTimeField(default=datetime.utcnow)

    # jwt_token = StringField()
    clearance_level = EnumField(ClearanceLevel, default=ClearanceLevel.USER)
    # token_expiration = DateTimeField()

    meta = {
        "collection": "users",
        "indexes": [
            "email",
            "clearance_level"
        ]
    }