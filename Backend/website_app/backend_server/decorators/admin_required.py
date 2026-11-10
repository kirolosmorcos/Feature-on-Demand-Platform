from functools import wraps
from flask import jsonify
from flask_jwt_extended import verify_jwt_in_request, get_jwt


def admin_required():
    def wrapper(fn):
        @wraps(fn)
        def decorator(*args, **kwargs):
            verify_jwt_in_request()

            claims = get_jwt()
            clearance_level = claims.get("clearance_level")

            if clearance_level != "ADMIN":
                return jsonify({
                    "error": "Admin clearance is required for this action."
                }), 403

            return fn(*args, **kwargs)

        return decorator

    return wrapper