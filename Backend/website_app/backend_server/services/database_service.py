from database_manager import DatabaseManager
from services.car_type_service import CarTypeService
from services.ecu_service import ECUService
from services.version_service import VersionService
from services.request_service import RequestService
from services.feature_service import FeatureService
from services.payment_service import PaymentService
from services.deactivation_service import DeactivationService
from services.auth_service import AuthService

class DatabaseService:
    """
    Factory service that provides access to all individual collection services.
    Refactored to rely on the updated DatabaseManager which now holds the service instances.
    """
    
    def __init__(self, data_directory: str):
        """Initialize the database service with database manager"""
        # The DatabaseManager now initializes all services internally
        self.db_manager = DatabaseManager(data_directory)
    
    def get_car_type_service(self) -> CarTypeService:
        return CarTypeService()
    
    def get_ecu_service(self) -> ECUService:
        return ECUService()
    
    def get_version_service(self) -> VersionService:
        return VersionService()

    def get_request_service(self) -> RequestService:
        return RequestService()
    
    def get_feature_service(self) -> FeatureService:
        return FeatureService()
    
    def get_payment_service(self) -> PaymentService:
        return PaymentService()
    
    def get_deactivation_service(self) -> DeactivationService:
        return DeactivationService()
    
    def get_auth_service(self) -> AuthService:
        return AuthService()
