import os
from typing import List, Optional, Dict
from models import Version
from azure.storage.blob import BlobClient
# We don't need 'from database_manager' for type hinting if it causes circular imports, 
# but we keep it if your structure allows.
from mongoengine.errors import NotUniqueError

class VersionService:
    """Service for handling firmware version operations using MongoEngine"""
    
    # def __init__(self, db_manager):
    #     """
    #     Initialize with database manager.
    #     Required to access file system methods (get_hex_file_chunk, etc.)
    #     """
    #     self.db_manager = db_manager
    
    def get_all(self) -> List[Version]:
        """Get all versions"""
        return list(Version.objects.all())
    
    def get_by_version_number(self, version_number: str, hex_file_path: str = None) -> Optional[Version]:
        """Get a version by its number and optionally hex file path"""
        if hex_file_path:
            return Version.objects(version_number=version_number, hex_file_path=hex_file_path).first()
        return Version.objects(version_number=version_number).first()
    
    def get_by_ids(self, version_ids: List) -> List[Version]:
        """Get versions by their IDs"""
        if not version_ids:
            return []
        return list(Version.objects(id__in=version_ids))
    
    def save(self, version: Version) -> bool:
        """
        Save a version. 
        Note: The user's original code had 'upsert' logic based on (version_number + hex_path).
        We replicate that logic here.
        """
        try:
            # 1. Check if this version already exists
            existing = Version.objects(
                version_number=version.version_number, 
                hex_file_path=version.hex_file_path
            ).first()

            if existing:
                # Update the existing record with new data
                existing.compatible_car_types = [c.lower() for c in version.compatible_car_types]
                # (Update other fields if Version model has more)
                existing.save()
                # Update the ID on the passed object so the caller knows it
                version.id = existing.id
                return True
            else:
                # Create new
                # Ensure compatible types are lowercase
                version.compatible_car_types = [c.lower() for c in version.compatible_car_types]
                version.save()
                return True
                
        except Exception as e:
            print(f"Error saving version: {str(e)}")
            return False
    
    def update(self, version_number: str, hex_file_path: str, data: Dict) -> bool:
        """Update specific fields of a version"""
        try:
            version = self.get_by_version_number(version_number, hex_file_path)
            if not version:
                return False
            
            for field, value in data.items():
                if hasattr(version, field):
                    setattr(version, field, value)
            
            version.save()
            return True
        except Exception as e:
            print(f"Error updating version: {str(e)}")
            return False
    
    def delete(self, version_number: str, hex_file_path: str) -> bool:
        """Delete a version"""
        try:
            version = self.get_by_version_number(version_number, hex_file_path)
            if version:
                version.delete()
                return True
            return False
        except Exception as e:
            print(f"Error deleting version: {str(e)}")
            return False
    
    def get_compatible_versions(self, car_type_name: str, versions=None):
        target = car_type_name.lower()

        if versions is not None:
            result = []
            for v in versions:
                if not hasattr(v, 'compatible_car_types'):
                    continue

                # convert once
                compatible = {c.lower() for c in v.compatible_car_types}

                if target in compatible:
                    result.append(v)

            return result

        return list(Version.objects(compatible_car_types=target))   
    # --- File System Delegates (Pass-through to DatabaseManager) ---
    
    def get_latest_compatible_version(self, ecu, car_type_name):
        """ Returns the highest version compatible with the given car_type """
        """'ecu' is an ECU object with a 'versions' list."""
        """Must provide model of car type to filter versions"""

        if not ecu or not ecu.versions:
            return None

        compatible_versions = self.get_compatible_versions(
            car_type_name,
            versions=ecu.versions   # Already loaded list
        )

        if not compatible_versions:
            return None

        return max(compatible_versions)
 

    def get_hex_file_chunk(self, blob_url: str, chunk_size: int, offset: int) -> bytes:
        blob_client = BlobClient.from_blob_url(
        blob_url,
        credential=os.environ["HEX_STORAGE_ACCOUNT_KEY"]
    )

        stream = blob_client.download_blob(offset=offset, length=chunk_size)
        return stream.readall()
    
    def get_hex_file(self, blob_url: str) -> bytes:
        blob_client = BlobClient.from_blob_url(
        blob_url,
        credential=os.environ["HEX_STORAGE_ACCOUNT_KEY"]
    )

        stream = blob_client.download_blob()
        return stream.readall()

    def get_file_size(self, blob_url: str) -> int:
     blob_client = BlobClient.from_blob_url(
        blob_url,
        credential=os.environ["HEX_STORAGE_ACCOUNT_KEY"]
    )

     props = blob_client.get_blob_properties()
     return props.size