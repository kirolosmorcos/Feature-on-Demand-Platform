from flask import Blueprint, jsonify, request, current_app
from flask_jwt_extended import jwt_required
from datetime import datetime
from models import ServiceRequest, DownloadRequest, RequestStatus, ServiceType, ECUSnapShot, CarType
from decorators.vm_required import vm_required
from models.enums import *
from services.mtls_service import vehicle_mtls_required

request_bp = Blueprint('request', __name__)


@request_bp.route('/service/', methods=['POST'])
@request_bp.route('/service', methods=['POST'])
def create_service_request():
    """Create a new service request"""
    data = request.json

    if not data or not data.get('car_id') or not data.get('car_type'):
        return jsonify({'error': 'Missing required fields'}), 400

    # Extract data
    car_id = data.get('car_id')
    car_type = data.get('car_type')
    ip_address = data.get('ip_address', request.remote_addr)
    port = data.get('port', 0)

    # Validate service type
    try:
        service_type = ServiceType(data.get('service_type', 'DIAGNOSTICS'))
    except ValueError:
        return jsonify({'error': 'Invalid service type'}), 400

    # Create service request
    service_request = ServiceRequest(
        timestamp=datetime.now(),
        car_type=car_type,
        car_id=car_id,
        ip_address=ip_address,
        port=port,
        service_type=service_type,
        metadata=data.get('metadata', {}),
        status=RequestStatus.PENDING
    )

    # Save to database
    request_service = current_app.db_service.get_request_service()
    success = request_service.create_service_request(service_request)

    if success:
        return jsonify({'message': 'Service request created successfully'}), 201
    else:
        return jsonify({'error': 'Failed to create service request'}), 500


@request_bp.route('/download/', methods=['POST'])
@request_bp.route('/download', methods=['POST'])
@vm_required()
def create_download_request():
    data = request.json
    if not data:
        return jsonify({'error': 'Missing request body'}), 400

    required_fields = ['car_id', 'car_type', 'required_versions']
    for field in required_fields:
        if field not in data:
            return jsonify({'error': f'Missing {field}'}), 400

    car_id = data['car_id']
    car_type = data['car_type']
    required_versions = data['required_versions']  # dict: ecu -> version
    old_versions = data.get('old_versions', [])
    ip_address = data.get('ip_address', request.remote_addr)
    port = data.get('port', 0)

    request_service = current_app.db_service.get_request_service()

    try:
        download_request = request_service.create_download_request_service(
            car_id=car_id,
            car_type=car_type,
            required_versions=required_versions,
            old_versions=old_versions,
            ip_address=ip_address,
            port=port
        )
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
   
  
    return jsonify({
        #request_id is mongo ObjectId string of DownloadRequest
        'request_id': str(download_request.id),
        'total_size': download_request.total_size,
        "files": download_request.files,
    }), 201


@request_bp.route('/download/<car_id>/status/', methods=['PUT'])
@request_bp.route('/download/<car_id>/status', methods=['PUT'])
def update_download_status(car_id):
    """Update the status of a download request"""
    data = request.json

    if not data or 'status' not in data:
        return jsonify({'error': 'Missing status field'}), 400

    # Validate status
    try:
        status = DownloadStatus(data['status'])
    except ValueError:
        return jsonify({'error': 'Invalid status value'}), 400

    # Get transferred size if provided
    transferred_size = data.get('transferred_size')

    # Update status
    request_service = current_app.db_service.get_request_service()
    success = request_service.update_download_request_status(
        car_id, status, transferred_size)

    if success:
        return jsonify({'message': 'Status updated successfully'})
    else:
        return jsonify({'error': 'Failed to update status'}), 500


@request_bp.route('/car/<car_id>/', methods=['GET'])
@request_bp.route('/car/<car_id>', methods=['GET'])
def get_requests_for_car(car_id):
    """Get all requests for a specific car"""
    request_service = current_app.db_service.get_request_service()

    # Get both service and download requests
    service_requests = request_service.get_service_requests_by_car_id(car_id)
    download_requests = request_service.get_download_requests_by_car_id(car_id)

    result = {
        'service_requests': service_requests,
        'download_requests': download_requests
    }

    return jsonify(result)


@request_bp.route('/service/status/<status>/', methods=['GET'])
@request_bp.route('/service/status/<status>', methods=['GET'])
def get_service_requests_by_status(status):
    """Get all service requests with a specific status"""
    try:
        status_enum = RequestStatus(status)
    except ValueError:
        return jsonify({'error': 'Invalid status value'}), 400

    request_service = current_app.db_service.get_request_service()
    service_requests = request_service.get_service_requests_by_status(
        status_enum)

    return jsonify(service_requests)


@request_bp.route('/download/status/<status>/', methods=['GET'])
@request_bp.route('/download/status/<status>', methods=['GET'])
def get_download_requests_by_status(status):
    """Get all download requests with a specific status"""
    try:
        status_enum = RequestStatus(status)
    except ValueError:
        return jsonify({'error': 'Invalid status value'}), 400

    request_service = current_app.db_service.get_request_service()
    download_requests = request_service.get_download_requests_by_status(
        status_enum)

    return jsonify(download_requests)


@request_bp.route('/download/active/', methods=['GET'])
@request_bp.route('/download/active', methods=['GET'])
def get_active_downloads():
    """Get all active download requests"""
    request_service = current_app.db_service.get_request_service()
    active_downloads = request_service.get_active_download_requests()

    return jsonify(active_downloads)

@request_bp.route("/auth", methods=["POST"])
@request_bp.route("/auth/", methods=["POST"])
@vm_required()
def authenticate_car():
    """Authenticate a car based on car_type and car_id"""
    """used by HMI to authenticate cars before allowing connections"""
    """must modify to seacrch by car model too"""

    data = request.json or {}

    car_type = data.get("car_type")
    car_id = data.get("car_id")
    certificate_serial=data.get("serial_number")

    if not car_type or not car_id or not certificate_serial:
        return jsonify({
            "authenticated": False,
            "message": "Missing car_type or car_id or serial_number"
        }), 400

    # Load CarType
    car_type_service = current_app.db_service.get_car_type_service()
    if car_type_service.check_authentication(car_id, car_type, certificate_serial):


        return jsonify({
            "authenticated": True,
            "message": "Connection established"
        })
    else:
        return jsonify({
            "authenticated": False,
            "message": "Authentication failed"
        }), 401

@request_bp.route("/update-check", methods=["POST"])
@request_bp.route("/update-check/", methods=["POST"])
@vm_required()
def update_check():
    """Check for updates for a car based on its current ECU versions"""
    """used by HMI to check if a car needs updates"""
    """must modify to seacrch by car model too"""

    data = request.json or {}

    car_type_name = data.get("car_type")
    current_versions = data.get("ecu_versions", {})

    car_type_service = current_app.db_service.get_car_type_service()
    version_service = current_app.db_service.get_version_service()
    ecu_service = current_app.db_service.get_ecu_service()

    car_type = car_type_service.get_by_name(car_type_name)
    if not car_type:
        return jsonify({"error": "Car type not found"}), 404
    
    known_ecu_names = {ecu.name for ecu in car_type.ecus}

    # Validate ECU names coming from HMI   
    # ""el mafrod man3mlsh el check dh 3alshn el hmi haia el ba3ta el ecu names el batst5dmha fa lazm yakono s7""
    # ""But to be safe, we do the validation"" ""but we also need to validate that this ecu has this sended""

    for ecu_name in current_versions.keys():
        if ecu_name not in known_ecu_names:
            return jsonify({
                "error": f"ECU '{ecu_name}' does not exist in car type '{car_type_name}'"
            }), 400
    updates_needed = {}
    requested_ecus =[]
    for ecu_name in current_versions.keys():
        ecu_object = ecu_service.get_by_name(ecu_name)
        if ecu_object:
            requested_ecus.append(ecu_object)
    
    
    for ecu in requested_ecus:
        # Get latest version compatible with this car type
        latest = version_service.get_latest_compatible_version(ecu, car_type_name)
        if not latest:
            continue

        current_ver = current_versions.get(ecu.name, "").lower()
        current_ver_obj = version_service.get_by_version_number(current_ver) if current_ver else None

        if current_ver_obj and current_ver_obj < latest:
            updates_needed[ecu.name] = latest.version_number

    return jsonify({"updates": updates_needed})

@request_bp.route('/download/<request_id>/offset', methods=['POST'])
@request_bp.route('/download/<request_id>/offset/', methods=['POST'])
@vm_required()
def update_download_offset(request_id):
    """Stores the offset of a specific file in a download request"""
    data = request.json or {}
    ecu_name = data.get("ecu_name")
    offset = data.get("offset")

    if offset is None or ecu_name is None:
        return jsonify({"error": "Missing ecu_name or offset"}), 400

    request_service = current_app.db_service.get_request_service()
    """request_id is mongo ObjectId string of DownloadRequest""" 
    req = request_service.get_download_by_id(request_id)

    if not req:
        return jsonify({"error": "DownloadRequest not found"}), 404

    # Update offset in files list
    for f in req.files:
        if f["ecu_name"] == ecu_name:
            f["offset"] = offset
            break

    # Update total transferred size
    req.transferred_size = sum(f["offset"] for f in req.files)

    req.save()

    return jsonify({"saved": True})


### Diagnostic endpoints ###

@request_bp.route('/sensor-data', methods=['POST'])
@request_bp.route('/sensor-data/', methods=['POST'])
@vehicle_mtls_required
def receive_sensor_data():
    data = request.json
    if not data or not data.get('car_id') or not data.get('car_type'):
        return jsonify({'error': 'Missing required fields'}), 400

    car_id = data.get('car_id')
    car_type = data.get('car_type')
    sensors = data.get('sensor_data')
    ecu_snapshots = []
    
    for ecu_name, sensor_data in sensors.items():
        if sensor_data is None:
            return jsonify({'error': f'Missing sensor data for ECU {ecu_name}'}), 400
        ecu_snapshots.append(ECUSnapShot(
            timestamp=datetime.now(),
            ecu_name=ecu_name,
            sensor_data=sensor_data
        ))

    request_service = current_app.db_service.get_request_service()
    try:
        car_health_snapshot = request_service.evaluate_car_health(
            ecu_snapshots, car_type=car_type, car_id=car_id
        )

        return jsonify({
            'message': 'Sensor data saved successfully',
            'health': car_health_snapshot.overall_health.value if car_health_snapshot else None
        }), 201
    except ValueError as e:
        return jsonify({'error': str(e)}), 400
    except Exception as e:
        return jsonify({'error': 'Failed to process sensor data', 'details': str(e)}), 500

@request_bp.route('/vehicle-health/<car_id>/', methods=['GET'])
@request_bp.route('/vehicle-health/<car_id>', methods=['GET'])
@jwt_required()
def get_vehicle_health(car_id):
    """Get the most recent vehicle health snapshot for a specific car"""
    request_service = current_app.db_service.get_request_service()
    snapshot = request_service.get_vehicle_health(car_id)

    if snapshot:
        return jsonify({
            'car_id': snapshot.car_id,
            'car_type': snapshot.car_type,
            'overall_health': snapshot.overall_health.value,
            'ecu_snapshots': [
                {
                    'ecu_name': ecu.ecu_name,
                    'sensor_data': ecu.sensor_data,
                } for ecu in snapshot.ecu_snapshots
            ]
        })
    else:
        return jsonify({'error': 'No health data found for this car'}), 404


@request_bp.route('/active-dota/<car_id>/', methods=['POST'])
@request_bp.route('/active-dota/<car_id>', methods=['POST'])
@jwt_required()
def active_dota(car_id):
    deactivation_service = current_app.db_service.get_deactivation_service()
    try:
         deactivation_service.send_active_dota_message(car_id)
    except ValueError as e:
        return jsonify({'error': str(e)}), 400

    

    return jsonify({"message": "Active DOTA message sent to IoT Hub for car_id: " + car_id})

@request_bp.route('/vehicle-health/all/<car_id>/', methods=['GET'])
@request_bp.route('/vehicle-health/all/<car_id>', methods=['GET'])
@jwt_required()
def get_all_vehicle_health(car_id):
    """Get all vehicle health snapshots for a specific car"""
    status = request.args.get('status') # e.g., HEALTHY, CRITICAL
    start_date = request.args.get('start_date')
    end_date = request.args.get('end_date')

    request_service = current_app.db_service.get_request_service()

    snapshots = request_service.get_all_car_health_snapshots(car_id=car_id, status=status, start_date=start_date, end_date=end_date)

    return jsonify([
        {   
            'id': str(snapshot.id),
            'timestamp': snapshot.timestamp.isoformat(),
            'car_type': snapshot.car_type,
            'overall_health': snapshot.overall_health.value,
            'ecu_snapshots': [
                {
                    'ecu_name': ecu.ecu_name,
                    'sensor_data': ecu.sensor_data,
                } for ecu in snapshot.ecu_snapshots
            ]
        } for snapshot in snapshots
    ])

@request_bp.route('/vehicle-health/get-all-cars', methods=['GET'])
@request_bp.route('/vehicle-health/get-all-cars/', methods=['GET'])
@jwt_required()
def get_all_cars_health():
    """Get the most recent vehicle health snapshot for all cars"""
    status = request.args.get('status') # e.g., HEALTHY, CRITICAL

    request_service = current_app.db_service.get_request_service()
    snapshots = request_service.get_all_cars_health(status=status)

    return jsonify([
        {
            'timestamp': snapshot.timestamp.isoformat(),
            'car_id': snapshot.car_id,
            'car_type': snapshot.car_type,
            'overall_health': snapshot.overall_health.value,
            'ecu_snapshots': [
                {
                    'ecu_name': ecu.ecu_name,
                    'sensor_data': ecu.sensor_data,
                } for ecu in snapshot.ecu_snapshots
            ]
        } for snapshot in snapshots
    ])
# {
#     "car_type"
#     "car_id"
#     "ecus": ["ECU1", "ECU2"],
#     "sensor_data": {
#         "ECU1": {"temp": 90, "pressure": 30},
#         "ECU2": {"temp": 85, "pressure": 28}
#     }
# }