import os

from flask import Blueprint, jsonify, request, current_app

deactivation_bp = Blueprint('deactivation', __name__)


@deactivation_bp.route('/expire-license', methods=['POST'])
def expire_license():
    
    # internal_key = request.headers.get("X-INTERNAL-KEY")
    # expected_key = os.getenv("INTERNAL_SECRET")

    # if not expected_key or internal_key != expected_key:
    #     return jsonify({"error": "Unauthorized"}), 403

    data = request.get_json(silent=True) or {}
    license_id = data.get("license_id")

    if not license_id:
        return jsonify({"error": "license_id is required"}), 400

    try:
        deactivation_service = current_app.db_service.get_deactivation_service()
        result = deactivation_service.expire_license(license_id)

        return jsonify(result), 200

    except Exception as e:
        current_app.logger.exception("Failed to expire license")
        return jsonify({"error": str(e)}), 500


