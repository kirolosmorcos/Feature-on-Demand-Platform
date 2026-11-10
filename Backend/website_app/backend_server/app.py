import os
from flask import Flask,jsonify
from flask_cors import CORS
from flask_jwt_extended import JWTManager
from werkzeug.exceptions import UnprocessableEntity


from controllers.car_type_controller import car_type_bp
from controllers.ecu_controller import ecu_bp
from controllers.version_controller import version_bp
from controllers.request_controller import request_bp
from controllers.feature_controller import feature_bp
from controllers.payment_controller import payment_bp
from controllers.auth_controller import auth_bp
from controllers.deactivation_controller import deactivation_bp
from services.database_service import DatabaseService
from app_json_encoder import MongoJSONEncoder

def create_app(test_config=None):
    # Create and configure the app
    app = Flask(__name__, instance_relative_config=True)
    
    app.config["JWT_SECRET_KEY"] = os.getenv("JWT_SECRET_KEY")
    jwt=JWTManager(app)

    # Register the custom JSON provider
    app.json = MongoJSONEncoder(app)

    # Enable CORS
    CORS(app)

    app.config.from_mapping(
        SECRET_KEY=os.environ.get("SECRET_KEY", "dev"),
        DATA_DIRECTORY=os.path.join(app.instance_path, 'data'),
    )

    if test_config is None:
        app.config.from_pyfile('config.py', silent=True)
    else:
        app.config.from_mapping(test_config)

    # Ensure the instance folder exists
    try:
        os.makedirs(app.instance_path, exist_ok=True)
        os.makedirs(app.config['DATA_DIRECTORY'], exist_ok=True)
    except OSError:
        pass

    # Initialize the database service
    app.db_service = DatabaseService(app.config['DATA_DIRECTORY'])

    # Register blueprints
    app.register_blueprint(car_type_bp, url_prefix='/api/car-types')
    app.register_blueprint(ecu_bp, url_prefix='/api/ecus')
    app.register_blueprint(version_bp, url_prefix='/api/versions')
    app.register_blueprint(request_bp, url_prefix='/api/requests') 
    app.register_blueprint(feature_bp, url_prefix='/api/features')
    app.register_blueprint(deactivation_bp, url_prefix='/api/deactivation')
    app.register_blueprint(payment_bp, url_prefix='/api/payment')
    app.register_blueprint(auth_bp, url_prefix='/api/auth')
    
    
    @app.errorhandler(UnprocessableEntity)
    def handle_422(err):
        data = getattr(err, "data", None)
        return jsonify({
            "error": "validation_error",
            "messages": data.get("messages") if isinstance(data, dict) else str(err)
        }), 422

    @app.route('/')
    def index():
        return {
            "message": "Automotive Firmware Management API",
            "version": "1.0.0"
        }

    return app


app = create_app()

# if __name__ == '__main__':
#     app.run(debug=True) 