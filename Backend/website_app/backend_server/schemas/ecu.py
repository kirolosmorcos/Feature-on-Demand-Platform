from marshmallow import Schema, fields
class PlainECUSchema(Schema):
    id = fields.Str(dump_only=True)
    ecu_id = fields.Str(required=True)
    name = fields.Str(required=True)
    model_number = fields.Str(required=True)


class ECUVersionSchema(Schema):
    version_number = fields.Str(dump_only=True)
    compatible_car_types = fields.List(fields.Str(), dump_only=True)
    hex_file_path = fields.Str(dump_only=True)


class ECUCarTypeEcuItemSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    versions_count = fields.Int(dump_only=True)


class ECUDetailSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    versions = fields.List(fields.Nested(ECUVersionSchema), dump_only=True)


class ECUCompatibleItemSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    compatible_versions = fields.List(fields.Str(), dump_only=True)


class ECUListItemSchema(Schema):
    id = fields.Str(dump_only=True, attribute="_id")
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    versions = fields.List(fields.Nested(ECUVersionSchema), dump_only=True)    