from flask import  jsonify, request, current_app
from flask_smorest import Blueprint
from flask_jwt_extended import jwt_required
from models import CarType, Car, ECU, Version
from schemas import CreateCarTypeSchema, CreateCarTypeResponseSchema, CarTypeSummarySchema, CarTypeDetailSchema, UpdateCarTypeSchema,CarTypeStatisticsSchema,CarTypeByEcuItemSchema
from mongoengine.errors import NotUniqueError, ValidationError, DoesNotExist
import secrets
import hashlib
from decorators.admin_required import admin_required



car_type_bp = Blueprint('car_type', __name__)



@car_type_bp.route('/', methods=['GET'])
@car_type_bp.route('', methods=['GET'])
@car_type_bp.response(200, CarTypeSummarySchema(many=True))
@jwt_required()
def get_all_car_types():

    """Get all car types"""

    service = current_app.db_service.get_car_type_service()
    car_types = service.get_all()

    
    return [service.serialize_summary(car_type) for car_type in car_types]


@car_type_bp.route('/<name>/', methods=['GET'])
@car_type_bp.route('/<name>', methods=['GET'])
@car_type_bp.response(200, CarTypeDetailSchema)
@jwt_required()
def get_car_type(name):

    """Get a specific car type by name"""

    service = current_app.db_service.get_car_type_service()
    car_type = service.get_by_name(name.lower())

    if not car_type:
        return {'error': 'Car type not found'}, 404

    return service.serialize_detail(car_type)


@car_type_bp.route('/model/<model_number>/', methods=['GET'])
@car_type_bp.route('/model/<model_number>', methods=['GET'])
@car_type_bp.response(200, CarTypeDetailSchema)
@jwt_required()
def get_car_type_by_model(model_number):

    """Get a specific car type by model number"""

    service = current_app.db_service.get_car_type_service()
    car_type = service.get_by_model_number(model_number)

    if not car_type:
        return {'error': 'Car type not found'}, 404

   
    return service.serialize_detail(car_type)


@car_type_bp.route('/', methods=['POST'])
@car_type_bp.route('', methods=['POST'])
@car_type_bp.arguments(CreateCarTypeSchema)
@car_type_bp.response(201, CreateCarTypeResponseSchema)
@admin_required()
def create_car_type(data):
    
    """Create a new car type"""

    db_service = current_app.db_service
    car_type_service = db_service.get_car_type_service()
    ecu_service = db_service.get_ecu_service()

    try:
        car_type = car_type_service.create_with_dependencies(data, ecu_service)

        return {
            'message': 'Car type created successfully',
            'car_type': {
                'name': car_type.name,
                'model_number': car_type.model_number,
                'ecu_count': len(car_type.ecus),
                'car_ids': car_type.car_ids
            }
        }, 201

    except ValueError as e:
        message = str(e)

        if 'already exists' in message:
            return {
                'message': message,
                'car_type': None
            }, 409

        return {
            'message': message,
            'car_type': None
        }, 400

    except NotUniqueError:
        return {
            'message': 'A car type with this name or model number already exists.',
            'car_type': None
        }, 409

    except ValidationError as e:
        return {
            'message': f'Validation error: {str(e)}',
            'car_type': None
        }, 400

    except Exception as e:
        current_app.logger.error(f"Error creating car type: {e}")
        return {
            'message': f'Failed to create car type: {str(e)}',
            'car_type': None
        }, 500

# Needs to be tested
@car_type_bp.route('/<name>/', methods=['PUT', 'PATCH'])
@car_type_bp.route('/<name>', methods=['PUT', 'PATCH'])
@car_type_bp.arguments(UpdateCarTypeSchema)
@car_type_bp.response(200, CarTypeDetailSchema)
@admin_required()
def update_car_type(data,name):

    """
    Update cars for an existing car type by name.
    Allowed only:
    - add new cars
    - remove existing cars
    Not allowed:
    - edit existing certificate_serial
    - edit car type name/model/ecus
    """

    db_service = current_app.db_service
    car_type_service = db_service.get_car_type_service()

    try:
        updated_car_type = car_type_service.update_cars(name, data['cars'])

        if not updated_car_type:
            return {'error': f'Car Type with name "{name}" not found.'}, 404

        return car_type_service.serialize_detail(updated_car_type)

    except ValueError as e:
        return {'error': str(e)}, 400
    except NotUniqueError:
        return {'error': 'Update failed: duplicate car id detected.'}, 409
    except Exception as e:
        current_app.logger.error(f"Error updating car type: {e}")
        return {'error': f'Internal Server Error: {str(e)}'}, 500


@car_type_bp.route('/<name>/', methods=['DELETE'])
@car_type_bp.route('/<name>', methods=['DELETE'])
@admin_required()
def delete_car_type(name):
    """Delete a car type and its related cars"""
    service = current_app.db_service.get_car_type_service()

    success = service.delete(name)

    if not success:
        return {'error': 'Car type not found or failed to delete'}, 404

    return {'message': 'Car type and related cars deleted successfully'}, 200


@car_type_bp.route('/statistics/', methods=['GET'])
@car_type_bp.route('/statistics', methods=['GET'])
@car_type_bp.response(200, CarTypeStatisticsSchema)
@jwt_required()
def get_statistics():
    """Get statistics about car types"""
    service = current_app.db_service.get_car_type_service()
    stats = service.get_statistics()
    return stats


@car_type_bp.route('/by-ecu/<ecu_name>/', methods=['GET'])
@car_type_bp.route('/by-ecu/<ecu_name>', methods=['GET'])
@car_type_bp.response(200, CarTypeByEcuItemSchema(many=True))
@jwt_required()
def get_car_types_by_ecu_name(ecu_name):
    """Get all car types that have an ECU with the given name"""
    service = current_app.db_service.get_ecu_service()
    car_types = service.get_by_ecu_name(ecu_name)

    if not car_types:
        return []

    return [service.serialize_car_type_by_ecu(car_type) for car_type in car_types]