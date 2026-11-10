from marshmallow import Schema, fields, validate


class PlainCarTypeVersionSchema(Schema):
    version_number = fields.Str(required=True, validate=validate.Length(min=1))
    major_version = fields.Int(required=True)
    minor_version = fields.Int(required=True)
    patch_version = fields.Int(required=True)
    compatible_car_types = fields.List(fields.Str(), required=True)
    hex_file_path = fields.Str(required=True, validate=validate.Length(min=1))


class PlainCarTypeECUSchema(Schema):
    name = fields.Str(required=True, validate=validate.Length(min=1))
    model_number = fields.Str(required=True, validate=validate.Length(min=1))
    versions = fields.List(fields.Nested(PlainCarTypeVersionSchema), required=False)


class PlainCarSchema(Schema):
    car_id = fields.Str(required=True, validate=validate.Length(min=1))
    certificate_serial = fields.Str(required=True, validate=validate.Length(min=1))

class CarTypeCarSchema(Schema):
    car_id = fields.Str(dump_only=True)
    certificate_serial = fields.Str(dump_only=True)

class CreateCarTypeSchema(Schema):
    name = fields.Str(required=True, validate=validate.Length(min=1))
    model_number = fields.Str(required=True, validate=validate.Length(min=1))
    cars = fields.List(fields.Nested(PlainCarSchema), required=False)
    ecus = fields.List(fields.Nested(PlainCarTypeECUSchema), required=False)


class UpdateCarTypeSchema(Schema):
    cars = fields.List(fields.Nested(PlainCarSchema), required=True)


class CarTypeSummarySchema(Schema):
    id = fields.Str(dump_only=True)
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    car_ids_count = fields.Int(dump_only=True)
    car_ids = fields.List(fields.Str(), dump_only=True)
    ecus_count = fields.Int(dump_only=True)
    ecus_id = fields.List(fields.Str(), dump_only=True)


class CarTypeDetailVersionSchema(Schema):
    version_number = fields.Str(dump_only=True)
    compatible_car_types = fields.List(fields.Str(), dump_only=True)
    hex_file_path = fields.Str(dump_only=True)


class CarTypeDetailECUSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    versions = fields.List(fields.Nested(CarTypeDetailVersionSchema), dump_only=True)


class CarTypeDetailSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    car_ids = fields.List(fields.Str(), dump_only=True)
    cars = fields.List(fields.Nested(CarTypeCarSchema), dump_only=True)
    ecus = fields.List(fields.Nested(CarTypeDetailECUSchema), dump_only=True)

class CreateCarTypeResultSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    ecu_count = fields.Int(dump_only=True)
    car_ids = fields.List(fields.Str(), dump_only=True)


class CreateCarTypeResponseSchema(Schema):
    message = fields.Str(dump_only=True)
    car_type = fields.Nested(CreateCarTypeResultSchema, dump_only=True)    


class CarTypeStatisticsItemSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    car_ids_count = fields.Int(dump_only=True)
    ecu_count = fields.Int(dump_only=True)


class CarTypeStatisticsSchema(Schema):
    total_car_types = fields.Int(dump_only=True)
    total_manufactured = fields.Int(dump_only=True)
    car_type_details = fields.List(fields.Nested(CarTypeStatisticsItemSchema), dump_only=True)


class CarTypeByEcuItemEcuSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    versions_count = fields.Int(dump_only=True)


class CarTypeByEcuItemSchema(Schema):
    name = fields.Str(dump_only=True)
    model_number = fields.Str(dump_only=True)
    car_ids_count = fields.Int(dump_only=True)
    ecus = fields.List(fields.Nested(CarTypeByEcuItemEcuSchema), dump_only=True)    