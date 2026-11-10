from marshmallow import Schema, fields
from .ecu import PlainECUSchema
from .version import PlainVersionSchema

class PlainFeatureSchema(Schema):
    id=fields.Str(dump_only=True)
    feature_code=fields.Str(required=True)
    name=fields.Str(required=True)

class PlainFeatureProductSchema(Schema):
    id=fields.Str(dump_only=True)
    product_code=fields.Str(required=True)
    price=fields.Float(required=True)
    pricing_type=fields.Str(required=True)
    subscription_period=fields.Str(required=False)

class PlainFeatureEcuRequirementSchema(Schema):
    id=fields.Str(dump_only=True)
    ecu_id=fields.Str(required=True)
    first_supported_version_id=fields.Str(required=True)

class CreateFeatureSchema(PlainFeatureSchema):
    products=fields.List(fields.Nested(PlainFeatureProductSchema), dump_only=True,required=True)
    ecu_requirements=fields.List(fields.Nested(PlainFeatureEcuRequirementSchema), dump_only=True,required=True)


class FirmwareDetailsSchema(Schema):
   ecu=fields.Nested(PlainECUSchema, dump_only=True, required=True)
   current_version=fields.Nested(PlainVersionSchema, dump_only=True, required=True)
   target_version=fields.Nested(PlainVersionSchema, dump_only=True, required=True)
                                


class FeatureDiscoverySchema(PlainFeatureSchema):
    products=fields.List(fields.Nested(PlainFeatureProductSchema), dump_only=True)
    purchased=fields.Boolean(required=True)
    need_update=fields.Boolean(required=True)
    firmware_details = fields.List(
    fields.Nested(FirmwareDetailsSchema),
    dump_only=True)




   