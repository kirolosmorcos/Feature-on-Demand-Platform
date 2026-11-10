from flask import jsonify, request, current_app
from flask_smorest import Blueprint
from flask_jwt_extended import jwt_required
from models import Feature, FeatureEcuRequirement, ECU, FeatureLicense, FeatureProduct, DownloadRequest, LicenseStatus
from schemas import CreateFeatureSchema,FeatureDiscoverySchema
from services.mtls_service import vehicle_mtls_required

from decorators.admin_required import admin_required
feature_bp = Blueprint('feature', __name__)


@feature_bp.route('/available', methods=['POST'])
# @feature_bp.arguments(CreateFeatureSchema)
@feature_bp.response(200, FeatureDiscoverySchema(many=True))

@vehicle_mtls_required

def features_discovery():

   
    try:
        data = request.get_json()
        feature_service = current_app.db_service.get_feature_service()
        features = feature_service.features_discovery(data)
        return features, 200
    except ValueError as e:
        return jsonify({"error": str(e)}), 400
    except Exception as e:
        return jsonify({"error": str(e)}), 500



@feature_bp.route('/config-enable', methods=['POST'])
@vehicle_mtls_required
def config_enable():
    data = request.json
    if not data or not data.get('feature_code') or not data.get('product_code'):
        return jsonify({'error': 'feature_code and product_code are required'}), 400    

    car_id = data.get('car_id')  
    feature_code = data.get('feature_code')
    product_code = data.get('product_code')
    
    try:
        feature_service = current_app.db_service.get_feature_service()
        feature_license = feature_service.config_enable_service(car_id, feature_code, product_code)
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception as e:
        return jsonify({'error': str(e)}), 500

    config_payload = {
        "car_id": car_id,
        "feature_code": feature_code,
        "feature_name": feature_license.feature.name,
        "feature_activation": True,
        "ecu_tokens": feature_license.ecu_tokens
    }

    return jsonify({'configuration': config_payload}), 200


@feature_bp.route('/config-complete', methods=['POST'])
@vehicle_mtls_required
def config_complete():
    data = request.json
    if not data or not data.get('feature_code') or not data.get('product_code'):
        return jsonify({'error': 'feature_code and product_code are required'}), 400
    
    car_id = data.get('car_id')  
    feature_code = data.get('feature_code')
    product_code = data.get('product_code')

    try:
        feature_service = current_app.db_service.get_feature_service()
        if(feature_service.config_complete_service(car_id, feature_code, product_code)):
            return jsonify({'message': 'Feature configuration completed and license activated'}), 200

    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@feature_bp.route('/', methods=['POST'])
@feature_bp.route('', methods=['POST'])
@admin_required()
def create_feature():
    data = request.get_json()

    if not data:
        return jsonify({"error": "Request data is required"}), 400
    
    required_fields = ["feature_code", "feature_name", "products", "ecu_versions"]

    for field in required_fields:
        if field not in data:
            return jsonify({"error": f"'{field}' is required"}), 400
    
    try:
        feature_service = current_app.db_service.get_feature_service()
        feature = feature_service.create_feature(data)
        return jsonify({
            "message": "Feature created successfully",
            "feature": {
                "feature_code": feature.feature_code,
            }
        }), 201
    
    except ValueError as e:
        return jsonify({"error": str(e)}), 400
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@feature_bp.route('/<feature_code>', methods=['PUT', 'PATCH'])
@feature_bp.route('/<feature_code>/', methods=['PUT', 'PATCH'])
@admin_required()
def update_feature(feature_code):
    data = request.get_json()

    if not data:
        return jsonify({"error": "Request data is required"}), 400
    
    try:
        feature_service = current_app.db_service.get_feature_service()
        updated_feature = feature_service.update_feature(feature_code, data)
        return jsonify({
            "message": "Feature updated successfully",
            "feature": {
                "feature_code": updated_feature.feature_code,
            }
        }), 200
    
    except ValueError as e:
        return jsonify({"error": str(e)}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@feature_bp.route('/<feature_code>', methods=['DELETE'])
@feature_bp.route('/<feature_code>/', methods=['DELETE'])
@admin_required()
def delete_feature(feature_code):
    try:
        feature_service = current_app.db_service.get_feature_service()
        feature_service.delete_feature(feature_code)
        return jsonify({"message": f"Feature '{feature_code}' deleted successfully"}), 200
    except ValueError as e:
        return jsonify({"error": str(e)}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@feature_bp.route('/', methods=['GET'])
@feature_bp.route('', methods=['GET'])
@jwt_required()
def get_all_features():
   
    try:
        feature_service = current_app.db_service.get_feature_service()
        features = feature_service.get_all_features()
        return jsonify(features), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500
    



@feature_bp.route('/<feature_code>', methods=['GET'])
@feature_bp.route('/<feature_code>/', methods=['GET'])
@jwt_required()
def get_feature(feature_code):
    try:
        feature_service = current_app.db_service.get_feature_service()
        feature = feature_service.get_feature_by_code(feature_code)
        if not feature:
            return jsonify({"error": "Feature not found"}), 404
        return jsonify(feature), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500

