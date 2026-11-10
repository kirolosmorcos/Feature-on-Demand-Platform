from datetime import datetime
from typing import List, Optional, Dict
import logging

from cachetools import cachedmethod
from operator import attrgetter
from services.cashe_service import discovery_reference_cache

from mongoengine.errors import NotUniqueError

from services.signing_service import SigningService
from .database_service import VersionService

from models import Feature, FeatureEcuRequirement, ECU, Version, FeatureProduct, Car, FeatureLicense
from models.enums import PricingType, SubscriptionPeriod, Currency, LicenseStatus

from schemas import FeatureDiscoverySchema, CreateFeatureSchema,FirmwareDetailsSchema

logger = logging.getLogger(__name__)

class FeatureService:

    def __init__(self):
        self.discovery_reference_cache = discovery_reference_cache

    def features_discovery(self, data):

        car_id = data.get("car_id")
        if not car_id:
            raise ValueError("car_id is required")
        
        current_versions = data.get("current_versions", [])
        current_versions_map = self._build_current_versions_map(current_versions)

        if not current_versions_map:
            raise ValueError("Current versions are required")
        
        car = self._get_active_car(car_id)
 
        car_type = car.car_type

        # -------------------------------
        # INPUT PREPROCESSING
        # -------------------------------
   
        car_ecus = list(current_versions_map.keys())
        if not car_ecus:
            raise ValueError("Car type has no ECUs defined")

        car_ecu_objects = ECU.objects(name__in=car_ecus)
        car_ecu_ids = {ecu.id for ecu in car_ecu_objects}

        if not car_ecu_ids:
            raise ValueError("No valid ECUs found in current_versions")

        # -------------------------------
        # LOAD ALL DATA (ONE TIME)
        # -------------------------------
        

        reference_data = self._get_discovery_reference_data()

        all_features = reference_data["all_features"]
        requirements_map = reference_data["requirements_map"]
        products_map = reference_data["products_map"]
        ecu_map = reference_data["ecu_map"]
        version_map = reference_data["version_map"]
        ecu_versions_map = reference_data["ecu_versions_map"]
        ecu_version_lookup = reference_data["ecu_version_lookup"]


        licensed_features = self._get_licensed_feature_ids(car_id)


        # -------------------------------
        # COMPATIBILITY CACHE
        # -------------------------------
        version_service = VersionService()
        compat_cache = {}

        # -------------------------------
        # MAIN LOOP
        # -------------------------------
        response = []

        for feature in all_features:

            requirements = requirements_map.get(feature.id, [])
            if not requirements:
                continue

            feature_dict = {
                "name": feature.name,
                "feature_code": feature.feature_code,
                "products": products_map.get(feature.id, []),
                "purchased": feature.id in licensed_features,
                "need_update": False,
                "firmware_details": []
            }

            feature_supported = True

            for req in requirements:

                req_ecu_id = req._data["ecu"].id
                if req_ecu_id not in car_ecu_ids:
                    feature_supported = False
                    break

                req_ecu = ecu_map.get(req_ecu_id)
                if not req_ecu:
                    feature_supported = False
                    break

                required_version = version_map.get(
                    req._data["first_supported_version"].id
                )

                ecu_versions = ecu_versions_map.get(req_ecu_id, [])
                if not ecu_versions:
                    feature_supported = False
                    break

                # -------------------------------
                # COMPATIBILITY CACHE
                # -------------------------------
                key = (req_ecu_id, car_type.name)

                if key not in compat_cache:
                    compat_cache[key] = version_service.get_compatible_versions(
                        car_type_name=car_type.name,
                        versions=ecu_versions
                    )

                compatible_versions = compat_cache[key]

                filtered = [v for v in compatible_versions if v >= required_version]

                if not filtered:
                    feature_supported = False
                    break

                target_version = min(filtered)
                latest_version = max(filtered)

                # -------------------------------
                # CURRENT VERSION LOOKUP (O(1))
                # -------------------------------
                current_version_number = current_versions_map.get(req_ecu.name)

                if not current_version_number:
                    raise ValueError(
                        f"Current version for ECU '{req_ecu.name}' is required"
                    )

                current_version = ecu_version_lookup[req_ecu_id].get(
                    current_version_number
                )

                if current_version is None:
                    raise ValueError(
                        f"Version '{current_version_number}' not found for ECU '{req_ecu.name}'"
    )

                if current_version < target_version:
                    feature_dict["need_update"] = True
                    feature_dict["firmware_details"].append({
                        "ecu": req_ecu,
                        "current_version": current_version,
                        "target_version": latest_version
                    })

            if feature_supported:
                response.append(feature_dict)

        return response


    def _get_active_car(self, car_id):
        """
        Load car and validate that it exists and is active.
        """
        car = Car.objects(car_id=car_id).first()

        if not car:
            raise ValueError("Car not found")

        if not car.is_active:
            raise ValueError("Car is not active")

        return car

    def _build_current_versions_map(self, current_versions):
        """
        Convert request current_versions list into a fast lookup map:
        {
            "ECU_NAME": "VERSION_NUMBER"
        }
        """
        return {
            item["ecu"]: item["version"]
            for item in current_versions
            if "ecu" in item and "version" in item
        }

    def _get_licensed_feature_ids(self, car_id):
        """
        Load active/entitled licenses for this car and return a set of feature IDs.
        This is dynamic per car, so we do not cache it in the shared reference cache.
        """
        licenses = FeatureLicense.objects(
            car_id=car_id,
            license_status__in=["active", "entitled"]
        )

        return {
            license_obj._data["feature"].id
            for license_obj in licenses
        }


    @cachedmethod(attrgetter("discovery_reference_cache"))
    def _get_discovery_reference_data(self):
        """
        Build and cache all shared reference data needed by feature discovery.

        Why cached:
        - features, requirements, products, ECUs, and versions are expensive to load
        - these change relatively infrequently
        - rebuilding all maps every request is wasteful

        Cache policy:
        - stored in memory
        - TTL is 5 minutes
        - after expiration, data is rebuilt automatically
        """

        # -------------------------------
        # LOAD SHARED DATA
        # -------------------------------
        # logger.info("Rebuilding feature discovery reference cache")
        print("Rebuilding feature discovery reference cache", flush=True)
        
        all_features = list(Feature.objects())
        all_requirements = list(FeatureEcuRequirement.objects())
        all_products = list(FeatureProduct.objects())

        # -------------------------------
        # BUILD REQUIREMENTS MAP
        # feature_id -> [requirements]
        # -------------------------------
        requirements_map = {}
        for requirement in all_requirements:
            feature_id = requirement._data["feature"].id
            requirements_map.setdefault(feature_id, []).append(requirement)

        # -------------------------------
        # BUILD PRODUCTS MAP
        # feature_id -> [products]
        # -------------------------------
        products_map = {}
        for product in all_products:
            feature_id = product._data["feature"].id
            products_map.setdefault(feature_id, []).append(product)

        # -------------------------------
        # PRELOAD ALL ECUs USED IN REQUIREMENTS
        # ecu_id -> ecu
        # -------------------------------
        ecu_ids = {
            requirement._data["ecu"].id
            for requirement in all_requirements
        }

        all_ecus = list(ECU.objects(id__in=ecu_ids))
        ecu_map = {
            ecu.id: ecu
            for ecu in all_ecus
        }

        # -------------------------------
        # PRELOAD ALL VERSIONS NEEDED
        # version_id -> version
        # -------------------------------
        version_ids = set()

        # versions referenced in feature requirements
        for requirement in all_requirements:
            version_ids.add(requirement._data["first_supported_version"].id)

        # versions referenced inside each ECU
        for ecu in all_ecus:
            for version_ref in ecu._data["versions"]:
                version_ids.add(version_ref.id)

        all_versions = list(Version.objects(id__in=version_ids))
        version_map = {
            version.id: version
            for version in all_versions
        }

        # -------------------------------
        # BUILD ECU -> LIST OF VERSION OBJECTS
        # -------------------------------
        ecu_versions_map = {}
        for ecu in all_ecus:
            ecu_versions_map[ecu.id] = [
                version_map[version_ref.id]
                for version_ref in ecu._data["versions"]
                if version_ref.id in version_map
            ]

        # -------------------------------
        # BUILD ECU -> VERSION_NUMBER -> VERSION OBJECT
        # -------------------------------
        ecu_version_lookup = {}
        for ecu in all_ecus:
            ecu_version_lookup[ecu.id] = {
                version.version_number: version
                for version in ecu_versions_map[ecu.id]
            }

        return {
            "all_features": all_features,
            "requirements_map": requirements_map,
            "products_map": products_map,
            "ecu_map": ecu_map,
            "version_map": version_map,
            "ecu_versions_map": ecu_versions_map,
            "ecu_version_lookup": ecu_version_lookup,
        }

    def invalidate_discovery_cache(self):
        """
        Clear cached discovery reference data manually.

        Call this after admin operations or data updates affecting:
        - features
        - feature requirements
        - feature products
        - ECUs
        - versions
        """
        discovery_reference_cache.clear()  
        # logger.info("Feature discovery reference cache invalidated manually") 
        print("Feature discovery reference cache invalidated manually", flush=True)     



    def create_feature (self ,data: dict) -> Feature:

        self.validate_create_payload(data)

        feature_code = data["feature_code"].lower().strip()
        feature_name = data["feature_name"].strip()

        if Feature.objects(feature_code=feature_code).first():
            raise ValueError(f"Feature code '{feature_code}' already exists")

        feature = Feature(
            feature_code=feature_code,
            name=feature_name
        )

        try:
            feature.save()


            for product_data in data["products"]:
                self.create_feature_product(feature, product_data)

            for ev in data["ecu_versions"]:
                self.create_feature_requirement(feature, ev)

            self.invalidate_discovery_cache()    

            return feature

        except Exception as e:
            self.rollback_feature_creation(feature)
            raise e
        

    def validate_create_payload(self, data: dict):
      
        required_fields = ["feature_code", "feature_name", "products", "ecu_versions"]

        for field in required_fields:
            if field not in data:
                raise ValueError(f"Missing required field: {field}")

        if not isinstance(data["products"], list) or not data["products"]:
            raise ValueError("Feature must contain at least one product")

        if not isinstance(data["ecu_versions"], list) or not data["ecu_versions"]:
            raise ValueError("Feature must contain at least one ECU requirement")  
        
        durations = [] 
        for product in data["products"]:
            duration = product["duration"].lower().strip()
            if duration in durations:
                raise ValueError(f"Duplicate product duration '{duration}' is not allowed")
            durations.append(duration)

        
        ecus = []
        for ev in data["ecu_versions"]:
            ecu_key = (ev["ecu"], ev["ecuModel"])
            if ecu_key in ecus:
                raise ValueError(f"Duplicate ECU '{ev['ecu']}' ({ev['ecuModel']}) is not allowed")
            ecus.append(ecu_key)

    def create_feature_product(self, feature: Feature, product_data: dict):

        product_code = product_data["product_code"].lower().strip()
        duration = product_data["duration"].lower().strip()

        if FeatureProduct.objects(product_code=product_code).first():
            raise ValueError(f"Product code '{product_code}' already exists")
        

        pricing_type = (
            PricingType.ONE_TIME
            if duration == "lifetime"
            else PricingType.SUBSCRIPTION
        )

        subscription_period = None 
        if duration == "month":
            subscription_period = SubscriptionPeriod.MONTHLY
        elif duration == "year":
            subscription_period = SubscriptionPeriod.YEARLY

        product = FeatureProduct (
            product_code = product_code,
            feature = feature,
            pricing_type = pricing_type,
            price = product_data["price"],
            currency = Currency.USD,
            subscription_period = subscription_period
        )

        product.save()

    def create_feature_requirement(self, feature: Feature, ev: dict):

        ecu_name = ev["ecu"]
        ecu_model = ev["ecuModel"]
        version_number = ev["version"]

        ecu = ECU.objects(
            name=ecu_name,
            model_number=ecu_model
        ).first()

        if not ecu:
            raise ValueError(f"ECU '{ecu_name}' ({ecu_model}) not found")

        version = next(
            (v for v in ecu.versions if v.version_number == version_number),
            None
        )

        if not version:
         raise ValueError(
            f"Version '{version_number}' not found for ECU '{ecu_name}'"
        )

        requirement = FeatureEcuRequirement(
            feature=feature,
            ecu=ecu,
            first_supported_version=version
        )

        requirement.save()
 
    def rollback_feature_creation(self, feature: Feature):
        """
        Ensures DB consistency if any step fails.
        """

        FeatureProduct.objects(feature=feature).delete()
        FeatureEcuRequirement.objects(feature=feature).delete()

        try:
            feature.delete()
        except Exception:
            pass

    def get_all_features(self):
        """Get all features"""
        features = Feature.objects()

        return [
            {
                "feature_name": feature.name,
                "feature_code": feature.feature_code
            }
            for feature in features
        ]
    
    def delete_feature(self, feature_code: str):

        feature = Feature.objects(feature_code=feature_code).first()

        if not feature:
            raise ValueError("Feature not found")

        FeatureProduct.objects(feature=feature).delete()
        FeatureEcuRequirement.objects(feature=feature).delete()
        feature.delete()

        self.invalidate_discovery_cache()

    def get_feature_by_code(self, feature_code: str) -> Optional[Dict]:

        feature = Feature.objects(feature_code=feature_code).first()

        if not feature:
            raise ValueError("Feature not found")

        products = FeatureProduct.objects(feature=feature)
        requirements = FeatureEcuRequirement.objects(feature=feature)

        return {
            "feature_name": feature.name,
            "feature_code": feature.feature_code,
            "products": [
                {
                    "product_code": p.product_code,
                    "price": p.price,
                    "duration": (
                        "lifetime"
                        if p.pricing_type == PricingType.ONE_TIME
                        else (
                            "month"
                            if p.subscription_period == SubscriptionPeriod.MONTHLY
                            else "year"
                        )
                    )
                }
                for p in products
            ],
            "ecu_versions": [
                {
                    "ecu": r.ecu.name,
                    "ecuModel": r.ecu.model_number,
                    "version": r.first_supported_version.version_number
                }
                for r in requirements
            ]
        }
    
    def update_feature(self, feature_code: str, data: dict) -> Dict:

        feature = Feature.objects(feature_code=feature_code).first()

        if not feature:
            raise ValueError("Feature not found")
        
        self.validate_update_payload(feature, data)
        self.update_feature_products(feature, data["products"])
        self.update_feature_requirements(feature, data["ecu_versions"])

        self.invalidate_discovery_cache()

        return feature
    
    def validate_update_payload(self, feature: Feature, data: dict):

        required_fields = ["feature_code", "feature_name", "products", "ecu_versions"]

        for field in required_fields:
            if field not in data:
                raise ValueError(f"Missing required field: {field}")
        
        if data["feature_code"].lower().strip() != feature.feature_code:
            raise ValueError("Feature code cannot be changed")
        
        if data["feature_name"].strip() != feature.name:
            raise ValueError("Feature name cannot be changed")
        
        if not isinstance(data["products"], list) or not data["products"]:
            raise ValueError("Feature must contain at least one product")
        
        if not isinstance(data["ecu_versions"], list) or not data["ecu_versions"]:
            raise ValueError("Feature must contain at least one ECU requirement")
        
        self.validat_unique_durations(data["products"])
        self.validate_unique_ecus(data["ecu_versions"])

    def validat_unique_durations(self, products: List):

        durations = []
        for product in products:
            duration = product["duration"].lower().strip()
            if duration in durations:
                raise ValueError(f"Duplicate product duration '{duration}' is not allowed")
            durations.append(duration)

    def validate_unique_ecus(self, ecu_versions: List):

        ecus = []
        for ev in ecu_versions:
            ecu_key = (ev["ecu"], ev["ecuModel"])
            if ecu_key in ecus:
                raise ValueError(f"Duplicate ECU '{ev['ecu']}' ({ev['ecuModel']}) is not allowed")
            ecus.append(ecu_key)

    def update_feature_products(self, feature: Feature, products: List):

        existing_products = FeatureProduct.objects(feature=feature)

        existing_map = {p.product_code: p for p in existing_products}

        incoming_codes = [
            p["product_code"].lower().strip() for p in products
        ]

        for product in existing_products:
            if product.product_code not in incoming_codes:
                product.delete()

        for product_data in products:

            product_code = product_data["product_code"].lower().strip()

            pricing_type = (
                PricingType.ONE_TIME
                if product_data["duration"].lower().strip() == "lifetime"
                else PricingType.SUBSCRIPTION
            )

            subscription_period = None
            if product_data["duration"].lower().strip() == "month":
                subscription_period = SubscriptionPeriod.MONTHLY
            elif product_data["duration"].lower().strip() == "year":
                subscription_period = SubscriptionPeriod.YEARLY

            if product_code in existing_map:
                product = existing_map[product_code]
                product.pricing_type = pricing_type
                product.price = product_data["price"]
                product.subscription_period = subscription_period
                product.save()
            else:
                self.create_feature_product(feature, product_data)

    def update_feature_requirements(self, feature: Feature, ecu_versions: List):

        existing_requirements = FeatureEcuRequirement.objects(feature=feature)

        existing_map = {
            (r.ecu.name, r.ecu.model_number): r for r in existing_requirements
        }

        incoming_keys = [
            (ev["ecu"], ev["ecuModel"]) for ev in ecu_versions
        ]

        for requirement in existing_requirements:
            key = (requirement.ecu.name, requirement.ecu.model_number)
            if key not in incoming_keys:
                requirement.delete()

        for ev in ecu_versions:

            ecu_name = ev["ecu"]
            ecu_model = ev["ecuModel"]
            version_number = ev["version"]

            ecu = ECU.objects(
                name=ecu_name,
                model_number=ecu_model
            ).first()

            if not ecu:
                raise ValueError(f"ECU '{ecu_name}' ({ecu_model}) not found")

            version = next(
                (v for v in ecu.versions if v.version_number == version_number),
                None
            )

            if not version:
                raise ValueError(
                    f"Version '{version_number}' not found for ECU '{ecu_name}'"
                )

            key = (ecu_name, ecu_model)

            if key in existing_map:
                requirement = existing_map[key]
                requirement.first_supported_version = version
                requirement.save()
            else:
                self.create_feature_requirement(feature, ev)

    def config_enable_service(self, car_id: str, feature_code: str, product_code: str) -> FeatureLicense:
        """Checks all pre-requisites for enabling a feature configuration before purchase confirmation."""
        feature_license = FeatureLicense.objects(car_id=car_id, feature_code=feature_code, product_code=product_code,license_status__in=["entitled","active"]).first()

        if not feature_license or not isinstance(feature_license, FeatureLicense):
            raise ValueError("FeatureLicense not found")
            
        if feature_license.license_status == LicenseStatus.ACTIVE:
            raise ValueError("FeatureLicense already active")

        if feature_license.license_status != LicenseStatus.ENTITLED:
            raise ValueError("FeatureLicense not entitled")

        if not feature_license.ecu_tokens:
            raise ValueError("No ECU tokens found for this license")

        signing_service = SigningService()
        for token in feature_license.ecu_tokens:
            if not token.get('license_token'):
                raise ValueError("Missing license token in ECU tokens")
            if 'valid_from' not in token['license_token'] or 'valid_until' not in token['license_token']:
                raise ValueError("License token must have valid_from and valid_until fields")
            
            try:
                signing_service.sign_token(token['license_token'])
            except Exception as e:
                raise ValueError(f"Error occurred while signing token: {e}")

        return feature_license

    def config_complete_service(self, car_id: str, feature_code: str, product_code: str) -> bool:
        """Finalizes feature configuration by activating the license and performing necessary updates."""
        feature_license = FeatureLicense.objects(car_id=car_id, feature_code=feature_code, product_code=product_code, license_status=LicenseStatus.ENTITLED).first()
        if not feature_license or not isinstance(feature_license, FeatureLicense):
            raise ValueError("FeatureLicense not found")

        feature_license.license_status = LicenseStatus.ACTIVE
        # feature_license.valid_from = datetime.now(timezone.utc) should be set at purchase time, not activation time

        try:
            feature_license.save()
            return True
        except Exception as e:
            print(f"Error activating feature license: {str(e)}")
            raise ValueError(f'Failed to activate feature license: {str(e)}')