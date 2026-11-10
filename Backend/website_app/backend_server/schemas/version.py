from marshmallow import Schema, fields
from .ecu import PlainECUSchema

class PlainVersionSchema(Schema):
    id = fields.Str(dump_only=True)
    version_number= fields.Str(required=True)
    

class CreateVersionSchema(PlainVersionSchema):
    ecu=fields.Nested(PlainECUSchema, dump_only=True, required=True)\
    