from mongoengine.errors import NotUniqueError, DoesNotExist
from models import ECU, CarType, Version
from typing import List, Optional, Dict

class ECUService:
    """Service for handling ECU operations using MongoEngine"""

    # def __init__(self, db_manager=None):
    #     """
    #     Initialize with optional database manager. 
    #     Kept for architectural consistency.
    #     """
    #     self.db_manager = db_manager

    # 1. READ Operations
    # ------------------
    def get_all(self) -> List[ECU]:
        """Get all ECUs"""
        return list(ECU.objects.all())

    def get_by_name_and_model(self, name: str, model_number: str) -> Optional[ECU]:
        """Get an ECU by name and model number (Case-insensitive storage)"""
        return ECU.objects(
            name=name.lower(), 
            model_number=model_number.lower()
        ).first()

    def get_by_name(self, name: str) -> Optional[ECU]:
        """Get an ECU by name"""
        return ECU.objects(name=name.lower()).first()

    def get_by_ids(self, ecus: List) -> List[ECU]:
        """Get ECUs by a list of their IDs"""
        if not ecus:
            return []
        # 'id__in' is the MongoEngine syntax for "$in" query on the ID field
        return list(ECU.objects(id__in=ecus))

    # 2. WRITE Operations
    # -------------------
    def save(self, ecu: ECU) -> bool:
        """
        Save an ECU object. 
        - Forces name/model to lowercase.
        - Automatically handles saving linked Versions if they are new.
        """
        try:
            # 1. Force Lowercase
            if ecu.name: ecu.name = ecu.name.lower().strip()
            if ecu.model_number: ecu.model_number = ecu.model_number.lower().strip()

            # 2. Handle Linked Versions
            # If the user added new Version objects to the list that aren't saved yet,
            # we must save them first to get an ID.
            if ecu.versions:
                for v in ecu.versions:
                    # If 'v' is a full object and has no ID, it's new. Save it.
                    if isinstance(v, Version) and not v.id:
                        v.save()

            # 3. Save the ECU
            ecu.save()
            return True # Successfully saved (ID is now available in ecu.id)

        except NotUniqueError:
            print(f"Error: ECU '{ecu.name}' ({ecu.model_number}) already exists.")
            return False
        except Exception as e:
            print(f"Error saving ECU: {e}")
            return False

    def update(self, name: str, model_number: str, data: Dict) -> bool:
        """Update specific fields of an ECU"""
        try:
            ecu = self.get_by_name_and_model(name, model_number)
            if not ecu:
                print(f"ECU '{name}' ({model_number}) not found.")
                return False

            for field, value in data.items():
                # Force lowercase for identifiers
                if field in ['name', 'model_number'] and isinstance(value, str):
                    value = value.lower().strip()
                
                if hasattr(ecu, field):
                    setattr(ecu, field, value)
            
            ecu.save()
            return True
        except Exception as e:
            print(f"Error updating ECU: {e}")
            return False

    def delete(self, name: str, model_number: str) -> bool:
        """Delete an ECU by name and model number"""
        try:
            ecu = self.get_by_name_and_model(name, model_number)
            if ecu:
                ecu.delete()
                return True
            return False
        except Exception as e:
            print(f"Error deleting ECU: {e}")
            return False

    # 3. COMPLEX Queries (Relationships)
    # ----------------------------------
    def get_by_ecu_name(self, ecu_name: str) -> List[CarType]:
        """
        REVERSE LOOKUP: Get all CarTypes that use this specific ECU.
        Old code manually searched IDs. New code uses MongoEngine querying.
        """
        try:
            # 1. Find the ECU first
            ecu = self.get_by_name(ecu_name)
            if not ecu:
                return []

            # 2. Find CarTypes where the 'ecus' list contains this ECU object
            # MongoEngine handles the reference lookup automatically.
            return list(CarType.objects(ecus=ecu))
            
        except Exception as e:
            print(f"Error finding CarTypes for ECU {ecu_name}: {e}")
            return []

    def get_compatible_ecus(self, car_type_name: str) -> List[Dict]:
        """
        Get all ECUs that have versions compatible with a specific car type.
        """
        try:
            compatible_ecus = []
            all_ecus = self.get_all()
            car_type_lower = car_type_name.lower()

            for ecu in all_ecus:
                # Filter versions inside this ECU
                # Assumes Version model has a field 'compatible_car_types' (List of Strings)
                valid_versions = [
                    v.version_number for v in ecu.versions 
                    if hasattr(v, 'compatible_car_types') and car_type_lower in [c.lower() for c in v.compatible_car_types]
                ]

                if valid_versions:
                    compatible_ecus.append({
                        "name": ecu.name,
                        "model_number": ecu.model_number,
                        "compatible_versions": valid_versions
                    })
            
            return compatible_ecus

        except Exception as e:
            print(f"Error getting compatible ECUs: {e}")
            return []
        
    def serialize_car_type_by_ecu(self, car_type: CarType) -> Dict:
        ecus = []
        for ecu in car_type.ecus:
            ecus.append({
                'name': ecu.name,
                'model_number': ecu.model_number,
                'versions_count': len(ecu.versions) if hasattr(ecu, 'versions') else 0
            })

        return {
            'name': car_type.name,
            'model_number': car_type.model_number,
            'car_ids_count': len(car_type.car_ids) if hasattr(car_type, 'car_ids') else 0,
            'ecus': ecus
        }    