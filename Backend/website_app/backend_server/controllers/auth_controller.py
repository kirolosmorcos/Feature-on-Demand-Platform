from flask import Blueprint, jsonify, request, current_app

auth_bp = Blueprint('auth', __name__)

@auth_bp.route('/register', methods=['POST'])
def register():
    """Endpoint for user registration"""
    service = current_app.db_service.get_auth_service()
    data = request.json
    try:
        message = service.create_user(data)
        return jsonify(message), 201
    except ValueError as e:
        return jsonify({"error": str(e)}), 400

@auth_bp.route('/login', methods=['POST'])
def login():
    """Endpoint for user login"""
    service = current_app.db_service.get_auth_service()
    data = request.json
    try:
        message = service.login_user(data)
        return jsonify(message), 200
    except ValueError as e:
        return jsonify({"error": str(e)}), 401

# @auth_bp.route('/vehicle', methods=['POST'])
# @auth_bp.route('/vehicle/', methods=['POST'])
# def authenticate_vehicle():

# # Validate request body:

# # car_id

# # api_key

# # Load Car from DB:

# # Check car exists

# # Check is_active = True

# # Compare:

# # Hash(api_key) == api_key_hash

# # Generate JWT:
# # Payload:
# # {
# # sub = car_id
# # exp = 1 hour
# # }

# # Return:
# # {
# # access_token,
# # expires_in
# # }