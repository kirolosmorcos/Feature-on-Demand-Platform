from datetime import datetime
from typing import List, Dict, Optional
from dataclasses import dataclass, field
from enums import ServiceType, RequestStatus, DownloadStatus
# --- Firmware and ECU session models (runtime only) ---
@dataclass
class VersionSession:
    version_number: str
    compatible_car_types: List[str]
    hex_file_path: str

@dataclass
class ECUSession:
    name: str
    model_number: str
    versions: List[VersionSession]
    
    def get_latest_version(self) -> VersionSession:
        # Assuming versions are stored in order, latest being last
        print(f"latest version {self.versions[-1] if self.versions else None}")
        return self.versions[-1] if self.versions else None

@dataclass
class CarTypeSession:
    name: str
    model_number: str
    ecus: List[ECUSession]
    manufactured_count: int
    car_ids: List[str]
    
    def check_for_updates(self, current_versions: Dict[str, str]) -> Dict[str, str]:
        print("Entered check_for_updates")
        """
        Check if car needs updates by comparing current versions with latest versions
        Returns dict of ECU names and their required update versions
        """
        updates_needed = {}
        
        for ecu in self.ecus:
            print(f"\n\n current_versions: ")
            print(current_versions.values)

        #    current_versions = {key.lower(): value.lower() for key, value in current_versions.items()}

            print(current_versions)
            if ecu.name in current_versions:
                latest_version = ecu.get_latest_version()
                if latest_version and current_versions[ecu.name] != latest_version.version_number:
                    print(f"latest_version.version_number: {latest_version.version_number}")
                    updates_needed[ecu.name] = latest_version.version_number
        return updates_needed

@dataclass
class RequestSession:
    timestamp: datetime
    car_type: str
    car_id: str
    ip_address: str
    port: int
    service_type: ServiceType
    metadata: Dict  # Contains ECU versions
    status: RequestStatus

@dataclass
class DownloadRequestSession:
    timestamp: datetime
    car_type: str
    car_id: str
    request_id: str
    files: List[Dict]
    status: DownloadStatus = DownloadStatus.PREPARING_FILES
   

# NEW: Flashing feedback models
@dataclass
class FlashingFeedbackSession:
    """Represents flashing results feedback from a car"""
    session_id: str
    car_id: str
    car_type: str
    flashing_timestamp: datetime
    overall_status: str  # "completed", "partial_failure", "failed"
    total_ecus: int
    successful_ecus: List[str]
    rolled_back_ecus: List[str]
    final_ecu_versions: Dict[str, str]  # ECU name -> final version after flashing
    android_app_version: str
    beaglebone_version: str
    request_id: str  # Links back to the original download request
    received_timestamp: datetime = field(default_factory=datetime.now)
    
@dataclass
class EcuFlashingResultSession:
    """Detailed result for individual ECU flashing"""
    ecu_name: str
    status: str  # "success", "rolled_back", "failed", "system_failure"
    old_version: str
    attempted_version: str
    final_version: str
    flashing_attempts: int
    error_message: Optional[str] = None

@dataclass
class FlashingSession:
    """Complete flashing session information"""
    session_id: str
    car_id: str
    car_type: str
    start_time: datetime
    end_time: datetime
    overall_status: str
    ecu_results: List[EcuFlashingResultSession]
    total_ecus: int
    successful_ecus: int
    rolled_back_ecus: int
    failed_ecus: int
    download_request_id: str
    beaglebone_version: str
    android_version: str
    notes: Optional[str] = None

# NEW: Server metrics and analytics models
@dataclass
class FlashingMetricsSession:
    """Server-side flashing metrics and analytics"""
    car_type: str
    ecu_name: str
    version: str
    success_rate: float
    total_attempts: int
    successful_attempts: int
    failed_attempts: int
    rollback_attempts: int
    last_updated: datetime = field(default_factory=datetime.now)

@dataclass
class CarFlashingHistorySession:
    """Historical flashing data for a specific car"""
    car_id: str
    car_type: str
    total_flashing_sessions: int
    successful_sessions: int
    partial_success_sessions: int
    failed_sessions: int
    last_flashing_date: Optional[datetime] = None
    current_ecu_versions: Dict[str, str] = field(default_factory=dict)
    flashing_sessions: List[str] = field(default_factory=list)  # Session IDs