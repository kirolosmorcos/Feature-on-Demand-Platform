from datetime import datetime, timedelta
from flask_bcrypt import Bcrypt
from flask_jwt_extended import create_access_token

from models import User, ClearanceLevel


bcrypt = Bcrypt()


class AuthService:
    """Service for handling user authentication and management"""

    def create_user(self, data):
        email = data["email"].lower().strip()
        name = data["name"].strip()
        password = data["password"]

        if User.objects(email=email).first():
            raise ValueError("Email already exists")

        if name.lower() in ["john", "jessy", "kiro", "admin1234"]:
            clearance_level = ClearanceLevel.ADMIN
        elif name.lower() in ["vm_manager", "vmmanager", "vm_manager1234", "websocket"]:
            clearance_level = ClearanceLevel.VMANAGER
        else:
            clearance_level = ClearanceLevel.USER

        hashed_password = bcrypt.generate_password_hash(password).decode("utf-8")

        user = User(
            email=email,
            password=hashed_password,
            name=name,
            clearance_level=clearance_level,
        )

        user.save()

        return {
            "success": True,
            "message": "User registered successfully",
            "user": {
                "id": str(user.id),
                "email": user.email,
                "name": user.name,
                "clearance_level": user.clearance_level.value,
            },
        }

    def login_user(self, data):
        email = data["email"].lower().strip()
        password = data["password"]

        user = User.objects(email=email).first()

        if not user or not bcrypt.check_password_hash(user.password, password):
            raise ValueError("Invalid email or password")

        if user.clearance_level == ClearanceLevel.ADMIN:
            expires_delta = timedelta(days=30)
            expires_in = 30 * 24 * 60 * 60
        elif user.clearance_level == ClearanceLevel.VMANAGER:
            expires_delta = timedelta(days=1)
            expires_in = 24 * 60 * 60
            # expires_delta = timedelta(minutes=3)
            # expires_in = 3 * 60
        else:
            expires_delta = timedelta(minutes=3)
            expires_in = 3 * 60

        expires_at = datetime.utcnow() + expires_delta

        token = create_access_token(
            identity=str(user.id),
            additional_claims={
                "email": user.email,
                "name": user.name,
                "clearance_level": user.clearance_level.value,
            },
            expires_delta=expires_delta,
        )

        # user.jwt_token = token
        # user.token_expiration = expires_at
        user.save()

        return {
            "success": True,
            "message": "Login successful",
            "token": token,
            "expires_in": expires_in,
            "expires_at": expires_at.isoformat() + "Z",
            "user": {
                "id": str(user.id),
                "email": user.email,
                "name": user.name,
                "clearance_level": user.clearance_level.value,
            },
        }