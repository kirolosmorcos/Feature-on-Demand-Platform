from typing import List, Optional, Dict
from mongoengine.errors import NotUniqueError
from models import CarType, ECU, Version,Car
# from pymongo import 

class CarTypeService:
    """Service for handling car type operations"""
    def get_all(self) -> List[CarType]:
        """Get all car types"""
        return list(CarType.objects.all())

    def get_by_name(self, name: str) -> Optional[CarType]:
        """Get a car type by name"""
        return CarType.objects(name=name.lower()).first()

    def get_by_model_number(self, model_number: str) -> Optional[CarType]:
        """Get a car type by model number"""
        return CarType.objects(model_number=model_number.lower()).first()


    def save(self, car_type: CarType) -> bool:
        """
        Save a car type to the database, ensuring name and model number are individually unique.
        Throws an exception if a car type with the same name OR model number already exists.
        """
        try:
            # Convert to lowercase
            car_type.name = car_type.name.lower().strip()
            car_type.model_number = car_type.model_number.lower().strip()
            car_type.save()
            return True
        except NotUniqueError as e:
            print(f"Error CarType with name '{car_type.name}' or model number '{car_type.model_number}' already exists: {str(e)}")
            return False
        except Exception as e:
            print(f"Error saving car type: {str(e)}")
            return False
        

    def update(self, name: str, data: Dict) -> bool:
        """Update specific fields of a car type"""
        try:
            # Create a copy of the data to avoid modifying the original
            car=self.get_by_name(name)
            if not car:
                print(f"CarType with name '{name}' not found for update.")
                return False
            for key, value in data.items():
                if key in ['name', 'model_number']:
                    value = value.lower().strip()
                if hasattr(car, key):
                    setattr(car, key, value)
            car.save()
            return True
        except NotUniqueError as e:
            print(f"Error: Update failed. The name or model number provided already exists.")
            return False  
        except Exception as e:
            print(f"Error updating car type: {str(e)}")
            return False
    

    def delete(self, name: str) -> bool:
        """Delete a car type, its related cars, and remove it from version compatibility lists."""
        try:
            car_type = self.get_by_name(name)
            if not car_type:
                print(f"CarType with name '{name}' not found for deletion.")
                return False

            normalized_name = car_type.name.lower().strip()

            # delete related cars first
            Car.objects(car_type=car_type).delete()

            # remove car type name from all version compatibility lists
            for version in Version.objects(compatible_car_types=normalized_name):
                updated_compatible_car_types = [
                    ct for ct in version.compatible_car_types
                    if ct.lower().strip() != normalized_name
                ]
                version.compatible_car_types = updated_compatible_car_types
                version.save()

            # then delete the car type
            car_type.delete()
            return True

        except Exception as e:
            print(f"Error deleting car type: {str(e)}")
            return False

    def get_statistics(self) -> Dict:
        """Get statistics about car types"""
        try:
            car_types = self.get_all()

            # Calculate some basic statistics
            stats = {
                "total_car_types": len(car_types),
                "total_manufactured": sum(len(ct.car_ids) for ct in car_types),
                "car_type_details": []
            }

            for car_type in car_types:
                stats["car_type_details"].append({
                    "name": car_type.name,
                    "model_number": car_type.model_number,
                    "car_ids_count": len(car_type.car_ids),
                    "ecu_count": len(car_type.ecus)
                })

            return stats
        except Exception as e:
            print(f"Error getting car type statistics: {str(e)}")
            return {"error": str(e)}
    
    def check_authentication(self, car_id: str, car_type: str, serial_number: str) -> bool:
        """Check if the provided car_id, car_type, and serial_number are valid for authentication"""
        try:
            car_type_obj = self.get_by_name(car_type)
            if not car_type_obj:
                print(f"CarType '{car_type}' not found for authentication.")
                return False
            
            car = Car.objects(car_id=car_id, car_type=car_type_obj, certificate_serial=serial_number).first()
            if not car:
                print(f"Car with ID '{car_id}', type '{car_type}', and serial number '{serial_number}' not found for authentication.")
                return False
          
            
            
            return True
        except Exception as e:
            print(f"Error during authentication check: {str(e)}")
            return False


    def update_cars(self, name: str, cars_data: List[Dict]) -> Optional[CarType]:
        """
        Replace-all behavior for cars only:
        - existing cars may remain
        - new cars may be added
        - missing cars will be removed
        - existing cars cannot have certificate_serial changed
        """
        car_type = self.get_by_name(name.lower())
        if not car_type:
            return None

        new_car_ids = [car['car_id'].lower().strip() for car in cars_data if car.get('car_id')]

        existing_car_docs = list(Car.objects(car_type=car_type))
        existing_car_map = {car.car_id: car for car in existing_car_docs}
        incoming_car_ids = set()

        for car_data in cars_data:
            car_id = car_data['car_id'].lower().strip()
            certificate_serial = car_data['certificate_serial'].strip()
            incoming_car_ids.add(car_id)

            existing_car = Car.objects(car_id=car_id).first()

            if existing_car and str(existing_car.car_type.id) != str(car_type.id):
                raise ValueError(f'Car with ID {car_id} already exists.')

            if car_id in existing_car_map:
                if existing_car_map[car_id].certificate_serial != certificate_serial:
                    raise ValueError(
                        f'Updating certificate_serial for existing car {car_id} is not allowed.'
                    )

        car_type.car_ids = new_car_ids
        car_type.save()

        for car_data in cars_data:
            car_id = car_data['car_id'].lower().strip()
            certificate_serial = car_data['certificate_serial'].strip()

            if car_id not in existing_car_map:
                Car(
                    car_id=car_id,
                    car_type=car_type,
                    certificate_serial=certificate_serial
                ).save()

        for existing_car_id, existing_car_doc in existing_car_map.items():
            if existing_car_id not in incoming_car_ids:
                existing_car_doc.delete()

        return car_type
    

    def serialize_detail(self, car_type: CarType) -> Dict:
        ecus = []
        for ecu in car_type.ecus:
            versions = []
            for version in ecu.versions:
                versions.append({
                    'version_number': version.version_number,
                    'compatible_car_types': version.compatible_car_types,
                    'hex_file_path': version.hex_file_path
                })

            ecus.append({
                'name': ecu.name,
                'model_number': ecu.model_number,
                'versions': versions
            })

        cars = []
        for car in Car.objects(car_type=car_type):
            cars.append({
                'car_id': car.car_id,
                'certificate_serial': car.certificate_serial
            })

        return {
            'name': car_type.name,
            'model_number': car_type.model_number,
            'car_ids': car_type.car_ids,
            'cars': cars,
            'ecus': ecus
        }
    

    def create_with_dependencies(self, data: Dict, ecu_service) -> CarType:
        """
        Create a car type with:
        - duplicate checks for name/model_number
        - ECU reuse or creation
        - Version creation
        - Car creation with certificate_serial
        """
        existing_car_type = self.get_by_name(data['name'].lower())
        if existing_car_type:
            raise ValueError('A car type with this name already exists.')

        existing_model = self.get_by_model_number(data['model_number'].lower().strip())
        if existing_model:
            raise ValueError('A car type with this model number already exists.')

        ecu_objects_list = []

        for ecu_data in data.get('ecus', []):
            existing_ecu = ecu_service.get_by_name_and_model(
                ecu_data['name'],
                ecu_data['model_number']
            )

            if existing_ecu:
                ecu_objects_list.append(existing_ecu)
            else:
                versions = []
                for version_data in ecu_data.get('versions', []):
                    versions.append(Version(
                        version_number=version_data['version_number'],
                        major_version=version_data['major_version'],
                        minor_version=version_data['minor_version'],
                        patch_version=version_data['patch_version'],
                        compatible_car_types=version_data['compatible_car_types'],
                        hex_file_path=version_data['hex_file_path']
                    ))

                new_ecu = ECU(
                    name=ecu_data['name'],
                    model_number=ecu_data['model_number'],
                    versions=versions
                )

                success = ecu_service.save(new_ecu)
                if not success:
                    raise ValueError(f'Failed to save ECU {new_ecu.name}')

                ecu_objects_list.append(new_ecu)

        cars_data = data.get('cars', [])
        car_ids = [car['car_id'] for car in cars_data]

        for car_data in cars_data:
            car_id = car_data['car_id']
            if not car_id:
                continue

            existing_car = Car.objects(car_id=car_id).first()
            if existing_car:
                raise ValueError(f'Car with ID {car_id} already exists.')

        car_type = CarType(
            name=data['name'],
            model_number=data['model_number'],
            ecus=ecu_objects_list,
            car_ids=car_ids
        )

        success = self.save(car_type)
        if not success:
            raise ValueError('Failed to create car type.')

        for car_data in cars_data:
            car_id = car_data['car_id']
            certificate_serial = car_data['certificate_serial']

            if not car_id:
                continue

            new_car = Car(
                car_id=car_id,
                car_type=car_type,
                certificate_serial=certificate_serial
            )
            new_car.save()

        return car_type
    
    def serialize_summary(self, car_type: CarType) -> Dict:
        return {
            'id': str(car_type.id),
            'name': car_type.name,
            'model_number': car_type.model_number,
            'car_ids_count': len(car_type.car_ids),
            'car_ids': car_type.car_ids,
            'ecus_count': len(car_type.ecus),
            'ecus_id': [str(ecu.id) for ecu in car_type.ecus],
        }    