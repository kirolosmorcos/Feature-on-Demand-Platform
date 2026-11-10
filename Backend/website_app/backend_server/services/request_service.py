from typing import List, Optional, Dict
from models import RequestStatus, ServiceRequest, DownloadRequest, ECUSnapShot, VehicleHealthSnapshot
from database_manager import DatabaseManager
from services.car_type_service import CarTypeService
from mongoengine.errors import DoesNotExist
import logging
from datetime import datetime, timezone
from models.enums import *
from bson import ObjectId
# from services.version_service import VersionService
# from services.ecu_service import ECUService

# Configure logging
logger = logging.getLogger('RequestService')

class RequestService:
    """Service for handling service and download requests using MongoEngine"""
    
    # def __init__(self, db_manager: DatabaseManager = None):
    #     """Initialize with database manager (optional for MongoEngine)"""
    #     self.db_manager = db_manager
    car_type_service = CarTypeService()
    
    # --- Service Request Methods ---

    def get_all_service_requests(self) -> List[ServiceRequest]:
        """Get all service requests"""
        try:
            # Returns actual Python objects, not dicts
            return list(ServiceRequest.objects.all())
        except Exception as e:
            logger.error(f"Error getting service requests: {str(e)}")
            return []
    
    def get_service_requests_by_car_id(self, car_id: str) -> List[ServiceRequest]:
        """Get service requests for a specific car"""
        try:
            return list(ServiceRequest.objects(car_id=car_id))
        except Exception as e:
            logger.error(f"Error getting service requests by car ID: {str(e)}")
            return []
    
    def get_service_requests_by_status(self, status) -> List[ServiceRequest]:
        """Get service requests with a specific status"""
        try:
            # Handle both Enum object and string value
            status_val = status.value if hasattr(status, 'value') else status
            return list(ServiceRequest.objects(status=status_val))
        except Exception as e:
            logger.error(f"Error getting service requests by status: {str(e)}")
            return []
    
    def create_service_request(self, service_request: ServiceRequest) -> bool:
        """Create a new service request"""
        try:
            # Since service_request is now a MongoEngine Document, we just save it.
            # If the input is NOT a Document (e.g. from legacy code), we might need to create one.
            if not isinstance(service_request, ServiceRequest):
                 logger.error("Error: Input is not a valid ServiceRequest document")
                 return False

            service_request.save()
            return True
        except Exception as e:
            logger.error(f"Error creating service request: {str(e)}")
            return False
    
    def update_service_request_status(self, request_id: str, status) -> bool:
        """Update the status of a service request"""
        try:
            req = ServiceRequest.objects(id=request_id).first()
            if req:
                status_val = status.value if hasattr(status, 'value') else status
                req.status = status_val
                req.save()
                return True
            return False
        except Exception as e:
            logger.error(f"Error updating service request status: {str(e)}")
            return False
    
    # --- Download Request Methods ---

    def get_all_download_requests(self) -> List[DownloadRequest]:
        try:
            return list(DownloadRequest.objects.all())
        except Exception as e:
            logger.error(f"Error getting download requests: {str(e)}")
            return []
    
    def get_download_requests_by_car_id(self, car_id: str) -> List[DownloadRequest]:
        try:
            return list(DownloadRequest.objects(car_id=car_id))
        except Exception as e:
            logger.error(f"Error getting download requests by car ID: {str(e)}")
            return []
    
    def get_download_requests_by_status(self, status) -> List[DownloadRequest]:
        try:
            status_val = status.value if hasattr(status, 'value') else status
            return list(DownloadRequest.objects(status=status_val))
        except Exception as e:
            logger.error(f"Error getting download requests by status: {str(e)}")
            return []
    
    def create_download_request_service(
        self,
        car_id: str,
        car_type: str,
        required_versions: dict,
        old_versions: dict,
        ip_address: str,
        port: int
    ) -> DownloadRequest:
        # snapshot = self.get_vehicle_health(car_id) #block if no sensor data?
        # if snapshot and snapshot.overall_health != VehicleHealth.Healthy:
        #     raise ValueError(f"Car {car_id} is not healthy, cannot create download request")

        from services.version_service import VersionService
        from services.ecu_service import ECUService
        ecu_service = ECUService() 
        version_service = VersionService()

        files = []
        total_size = 0
        required_versions_list = []
        """No checks here, assume caller validated input and required ecus and versions exist in this car type"""
        for ecu_name, ecu_info in required_versions.items():
            # ecu_info is a dict with 'model' and 'version'
            ecu_model = ecu_info.get("model")
            version_number = ecu_info.get("version")

            # Fetch ECU by name and model
            ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)
            if not ecu:
             raise ValueError(f"Invalid ECU or model: {ecu_name} ({ecu_model})")
            # 2. Get compatible versions for this car type
            compatible_versions = version_service.get_compatible_versions(
            car_type,
            versions=ecu.versions
        )

        # 3. Find requested version inside compatible list
            version = next(
             (v for v in compatible_versions if v.version_number == version_number),
            None
        )

            if not version:
             raise ValueError(
                f"Version {version_number} not compatible with car type {car_type}"
            )
          
            
            # Get hex path for this ECU and version
            hex_path = version.hex_file_path 
            if not hex_path:
                raise ValueError(f"Invalid ECU or version: {ecu_name} ({ecu_model})")
             
             

            size = version_service.get_file_size(hex_path)
            files.append({
             "ecu_name": ecu_name,
                "ecu_model": ecu_model,
                "version": version_number,
                "size": size,
                "offset": 0,
                "hex_path": hex_path
            })
            required_versions_list.append({
            "ecu_name": ecu_name,
            "model": ecu_model,
            "version": version_number
        })
            total_size += size

        download_request = DownloadRequest(
            timestamp=datetime.now(timezone.utc),
            car_type=car_type,
            car_id=car_id,
            ip_address=ip_address,
            port=port,
            required_versions=required_versions_list,
            old_versions=old_versions or [{}],
            total_size=total_size,
            transferred_size=0,
            status=DownloadStatus.PREPARING_FILES,
            active_transfers={f["ecu_name"]: False for f in files},  
            files=files
        )

        download_request.save()
        return download_request

    
    def update_download_request_status(self, car_id: str, status, transferred_size: int = None) -> bool:
        """Update the status of an active download request for a car"""
        try:
            # Find requests for this car that are NOT completed
            # logic: status__ne="COMPLETED"
            # Note: You might need to import RequestStatus to check the string value
            
            status_val = status.value if hasattr(status, 'value') else status
            completed_val = "COMPLETED" # Or RequestStatus.COMPLETED.value
            
            # Find the active request
            req = DownloadRequest.objects(
                car_id=car_id, 
                status__ne=completed_val
            ).first()

            if req:
                req.status = status_val
                if transferred_size is not None:
                    req.transferred_size = transferred_size
                req.save()
                return True
            return False
        except Exception as e:
            logger.error(f"Error updating download request status: {str(e)}")
            return False
    
    def get_active_download_requests(self) -> List[DownloadRequest]:
        """Get all active download requests"""
        try:
            # Filter out completed, failed, cancelled
            # Assumes status is stored as string
            ignored_statuses = ["COMPLETED", "FAILED", "CANCELLED"]
            return list(DownloadRequest.objects(status__nin=ignored_statuses))
        except Exception as e:
            logger.error(f"Error getting active download requests: {str(e)}")
            return []
        
    def get_download_by_id(self, request_id: str):
        try:
            return DownloadRequest.objects(id=ObjectId(request_id)).first()
        except Exception:
            return None

    # --- Diagnostics methods ---
    def create_ecu_snapshot(self, ecu_snaphot: ECUSnapShot) -> bool:
        """Create a new ECU snapshot"""
        try:
            if not isinstance(ecu_snaphot, ECUSnapShot):
                logger.error("Error: Input is not a valid ECUSnapShot document")
                return False

            ecu_snaphot.save()
            return True
        except Exception as e:
            logger.error(f"Error creating ECU snapshot: {str(e)}")
            return False

    def create_vehicle_health_snapshot(self, vehicle_health_snapshot: VehicleHealthSnapshot) -> bool:
        """Create a new vehicle health snapshot"""
        try:
            if not isinstance(vehicle_health_snapshot, VehicleHealthSnapshot):
                logger.error("Error: Input is not a valid VehicleHealthSnapshot document")
                return False

            vehicle_health_snapshot.save()
            return True
        except Exception as e:
            logger.error(f"Error creating vehicle health snapshot: {str(e)}")
            return False

    def evaluate_car_health(self, ecu_snapshots: List[ECUSnapShot], car_type: str, car_id: str) -> VehicleHealthSnapshot:
        """Evaluate the overall health of the car based on recent ECU snapshots"""
        
        car_type_obj = self.car_type_service.get_by_name(car_type)
        if not car_type_obj:
            raise ValueError(f"Car type '{car_type}' not found in database")

        valid_ecus = {ecu.name for ecu in car_type_obj.ecus} if car_type_obj else set()

        health_status = VehicleHealth.Healthy

        for snapshot in ecu_snapshots:
            if snapshot.ecu_name not in valid_ecus:
                raise ValueError(f"ECU '{snapshot.ecu_name}' does not belong to car type '{car_type}'")

            self.create_ecu_snapshot(snapshot)  # Save each ECU snapshot to the database

            for sensor, value in snapshot.sensor_data.items():
                if sensor == "temperature" and value > 90:
                    health_status = VehicleHealth.Error
                    break
                if sensor == "pressure" and value < 25:
                    health_status = VehicleHealth.Error
                    break
        
        vehiclehealthsnapshot = VehicleHealthSnapshot(
            timestamp=datetime.now(timezone.utc),
            car_type=car_type,
            car_id=car_id,
            ecu_snapshots=ecu_snapshots,
            overall_health=health_status
        )
        self.create_vehicle_health_snapshot(vehiclehealthsnapshot)  # Save the health snapshot to the database

        return vehiclehealthsnapshot

    def get_vehicle_health(self, car_id: str) -> Optional[VehicleHealthSnapshot]:
        """Get the most recent vehicle health snapshot for a specific car"""
        try:
            return VehicleHealthSnapshot.objects(car_id=car_id).order_by('-timestamp').first()
        except Exception as e:
            logger.error(f"Error getting vehicle health snapshot: {str(e)}")
            return None

    def get_all_car_health_snapshots(self, car_id: str, status: str = None, start_date: str = None, end_date: str = None) -> List[VehicleHealthSnapshot]:
        """Get all vehicle health snapshots for a specific car"""
        try:
            query = VehicleHealthSnapshot.objects(car_id=car_id)
            if status:
                query = query.filter(overall_health=status)
            if start_date:
                query = query.filter(timestamp__gte=start_date)
            if end_date:
                query = query.filter(timestamp__lte=end_date)
            return list(query.order_by('-timestamp'))
        except Exception as e:
            logger.error(f"Error getting vehicle health snapshots: {str(e)}")
            return []
    
    def get_all_cars_health(self, status: str = None) -> List[VehicleHealthSnapshot]:
        """Get the most recent health snapshot for all cars"""
        try:
            # This is a bit tricky in MongoEngine, we might need to do it in Python
            all_snapshots = VehicleHealthSnapshot.objects.order_by('-timestamp')
            all_snapshots = all_snapshots.filter(overall_health=status) if status else all_snapshots
            
            latest_snapshots = {}
            for snapshot in all_snapshots:
                if snapshot.car_id not in latest_snapshots:
                    latest_snapshots[snapshot.car_id] = snapshot
            return list(latest_snapshots.values())
        except Exception as e:
            logger.error(f"Error getting all cars health snapshots: {str(e)}")
            return []
# {
#     "car_type"
#     "car_id"
#     "ecus": ["ECU1", "ECU2"],
#     "sensor_data": {
#         "ECU1": {"temp": 90, "pressure": 30},
#         "ECU2": {"temp": 85, "pressure": 28}
#     }
# }