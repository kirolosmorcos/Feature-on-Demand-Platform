from enum import Enum
# hmi
class ServiceType(Enum):
    CHECK_FOR_UPDATE = "checkingForUpdate"
    DOWNLOAD_UPDATE = "downloadingNewUpdate"
    SENSOR_DATA = "sensorData"
# hmi
class RequestStatus(Enum):
    CHECKING_AUTHENTICITY = "checkingAuthenticity"
    AUTHENTICATED = "authenticated"
    NON_AUTHENTICATED = "nonAuthenticated"
    SERVICE_IN_PROGRESS = "serviceInProgress"
    FINISHED_SUCCESSFULLY = "finishedSuccessfully"
    FAILED = "failed"
    REJECTED = "rejected"
# hmi backend
class DownloadStatus(Enum):
    PREPARING_FILES = "preparingFiles"
    SENDING_IN_PROGRESS = "sendingInProgress"
    FINISHED_SUCCESSFULLY = "finishedSuccessfully"
    FAILED_PARTIAL_SUCCESS = "failedWithSomeFilesSendSuccessfully"
    ALL_FAILED = "allFailed"
# vehicle health
class VehicleHealth(Enum):
    Healthy = "Healthy"
    Critical = "Critical"
# license status
class LicenseStatus(Enum):
    AVAILABLE = "available"
    PENDING_PAYMENT = "pending_payment"
    PAYMENT_FAILED = "payment_failed"
    ENTITLED = "entitled"
    ACTIVE = "active"
    EXPIRED = "expired"
    REVOKED = "revoked"
# Pricing
class PricingType(Enum):
    ONE_TIME = "one_time"
    SUBSCRIPTION = "subscription"
# Subscription period
class SubscriptionPeriod(Enum):
    MONTHLY = "monthly"
    YEARLY = "yearly"
   
# Currency
class Currency(Enum):
    USD = "USD"
    EUR = "EUR"
    GBP = "GBP"
    EGP = "EGP"
# Payment status
class PaymentStatus(Enum):
    PENDING = "pending"
    SUCCESS = "success"
    FAILED = "failed"
    REFUNDED = "refunded"

# Clearance level
class ClearanceLevel(Enum):
    USER = "USER"
    VMANAGER = "VMANAGER"
    ADMIN = "ADMIN"

# class ServiceType(Enum):
#     """Enum for service types"""
#     DIAGNOSTICS = "DIAGNOSTICS"
#     REPAIR = "REPAIR"
#     MAINTENANCE = "MAINTENANCE"
#     UPDATE = "UPDATE"
#     OTHER = "OTHER"

# class RequestStatus(Enum):
#     """Enum for request status"""
#     PENDING = "PENDING"
#     IN_PROGRESS = "IN_PROGRESS"
#     COMPLETED = "COMPLETED"
#     FAILED = "FAILED"
#     CANCELLED = "CANCELLED"

