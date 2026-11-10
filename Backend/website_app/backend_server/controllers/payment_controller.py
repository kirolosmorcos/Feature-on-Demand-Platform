import json
from flask import Blueprint, jsonify, request, current_app
from datetime import datetime, timezone, timedelta
from services.mtls_service import vehicle_mtls_required
from models import (
    PaymentTransaction,
    FeatureLicense,
    FeatureEcuRequirement,
    Feature,
    FeatureProduct,
    PricingType,
    SubscriptionPeriod,
    LicenseStatus,
    PaymentStatus,
    Currency
)

payment_bp = Blueprint('payment', __name__)

@payment_bp.route('/webhook', methods=['POST', 'OPTIONS'])
# @vehicle_mtls_required
def payment_webhook():
    if request.method == 'OPTIONS':
        return '', 200
    data = request.json
    services = current_app.db_service.get_payment_service()
    try:
        result = services.handle_payment_webhook(data)
        return jsonify(result), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400
    




@payment_bp.route('/purchase', methods=['POST'])
@payment_bp.route('/purchase/', methods=['POST'])
# @vehicle_mtls_required
def purchase_feature(): 
    data = request.get_json()

    if not data:
        return jsonify({"error": "Request data is required"}), 400
    
    required_fields = ["feature_code", "product_code", "car_id"]

    for field in required_fields:
        if field not in data:
            return jsonify({"error": f"'{field}' is required"}), 400
    
    try:
        payment_service = current_app.db_service.get_payment_service()
        payment_url = payment_service.purchase_feature(data)
        return jsonify({
            "message": "Payment created successfully",
            "payment_url": payment_url
        }), 201
    
    except ValueError as e:
        return jsonify({"error": str(e)}), 400
    except Exception as e:
        return jsonify({"error": str(e)}), 500