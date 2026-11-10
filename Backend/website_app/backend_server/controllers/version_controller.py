from dotenv import load_dotenv
from flask import Blueprint, Response, jsonify, request, current_app, send_file
from flask_jwt_extended import jwt_required
import os
import io
from azure.storage.blob import BlobServiceClient, ContentSettings
import uuid
import json
from decorators.admin_required import admin_required
from decorators.vm_required import vm_required
version_bp = Blueprint('version', __name__)


@version_bp.route('/ecu/<ecu_name>/<ecu_model>/', methods=['GET'])
@version_bp.route('/ecu/<ecu_name>/<ecu_model>', methods=['GET'])
@jwt_required()
def get_versions_for_ecu(ecu_name, ecu_model):
    """Get all versions for a specific ECU"""
    # First get the ECU
    ecu_service = current_app.db_service.get_ecu_service()
    ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)

    if not ecu or not ecu.versions:
        return jsonify({'error': 'ECU not found or has no versions'}), 404

    # Convert to serializable format
    result = []
    for version in ecu.versions:
        result.append({
            'version_number': version.version_number,
            'compatible_car_types': version.compatible_car_types,
            'hex_file_path': version.hex_file_path
        })

    return jsonify(result)


@version_bp.route('/ecu/<ecu_name>/<ecu_model>/<version_number>/', methods=['GET'])
@version_bp.route('/ecu/<ecu_name>/<ecu_model>/<version_number>', methods=['GET'])
@jwt_required()
def get_version_details(ecu_name, ecu_model, version_number):
    """Get details for a specific version"""
    # First get the ECU
    ecu_service = current_app.db_service.get_ecu_service()
    ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)

    if not ecu:
        return jsonify({'error': 'ECU not found'}), 404

    # Find the version
    version = None
    for v in ecu.versions:
        if v.version_number == version_number:
            version = v
            break

    if not version:
        return jsonify({'error': 'Version not found'}), 404

    # Get file size
    version_service = current_app.db_service.get_version_service()
    file_size = version_service.get_file_size(version.hex_file_path)

    result = {
        'version_number': version.version_number,
        'compatible_car_types': version.compatible_car_types,
        'hex_file_path': version.hex_file_path,
        'file_size': file_size
    }

    return jsonify(result)


@version_bp.route('/download/<ecu_name>/<ecu_model>/<version_number>', methods=['GET'])
@version_bp.route('/download/<ecu_name>/<ecu_model>/<version_number>/', methods=['GET'])
def download_hex_file(ecu_name, ecu_model, version_number):
    """
    Download full firmware HEX file from Azure Blob Storage
    (Human / QA use only)
    """

    # 1️⃣ Get ECU
    ecu_service = current_app.db_service.get_ecu_service()
    ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)

    if not ecu:
        return jsonify({'error': 'ECU not found'}), 404

    # 2️⃣ Find version
    version = next((v for v in ecu.versions if v.version_number == version_number), None)
    if not version:
        return jsonify({'error': 'Version not found'}), 404

    blob_url = version.hex_file_path

    try:
        # 3️⃣ Download entire blob
        version_service = current_app.db_service.get_version_service()
        file_data = version_service.get_hex_file(version.hex_file_path)
    except Exception as e:
        return jsonify({'error': f'Failed to download hex file: {str(e)}'}), 500

    # 5️⃣ Send as downloadable file
    return send_file(
        io.BytesIO(file_data),
        as_attachment=True,
        download_name=f"{ecu_name}_{ecu_model}_v{version_number}.hex",
        mimetype="application/octet-stream"
    )


@version_bp.route('/stream/<ecu_name>/<ecu_model>/<version_number>/', methods=['GET'])
@version_bp.route('/stream/<ecu_name>/<ecu_model>/<version_number>', methods=['GET'])
def stream_hex_file(ecu_name, ecu_model, version_number):
    """
    Stream HEX firmware file in chunks (OTA-style)
    """

    # 1️⃣ Get ECU
    ecu_service = current_app.db_service.get_ecu_service()
    ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)

    if not ecu:
        return jsonify({'error': 'ECU not found'}), 404

    # 2️⃣ Find version
    version = next((v for v in ecu.versions if v.version_number == version_number), None)
    if not version:
        return jsonify({'error': 'Version not found'}), 404

    # 3️⃣ Query parameters
    chunk_size = request.args.get('chunk_size', 1024, type=int)
    offset = request.args.get('offset', 0, type=int)

    # Safety limits
    MAX_CHUNK_SIZE = 1024 * 1024  # 1 MB
    chunk_size = min(chunk_size, MAX_CHUNK_SIZE)

    # 4️⃣ Read file info from Azure
    version_service = current_app.db_service.get_version_service()
    file_size = version_service.get_file_size(version.hex_file_path)

    # 5️⃣ Offset validation
    if offset >= file_size:
        return '', 416  # Range Not Satisfiable

    # 6️⃣ Read chunk from Azure Blob
    chunk = version_service.get_hex_file_chunk(
        version.hex_file_path, chunk_size, offset
    )

    if not chunk:
        return jsonify({'error': 'Failed to read file chunk'}), 500

    # 7️⃣ Stream response (CORRECT way)
    return Response(
        chunk,
        status=206,  # ✅ Partial Content
        mimetype='application/octet-stream',
        headers={
            'Content-Range': f'bytes {offset}-{offset + len(chunk) - 1}/{file_size}',
            'Accept-Ranges': 'bytes',
            'Content-Length': str(len(chunk))
        }
    )

@version_bp.route('/chunk/<request_id>/<ecu_name>', methods=['GET'])
@version_bp.route('/chunk/<request_id>/<ecu_name>/', methods=['GET'])
@vm_required()
def get_chunk(request_id, ecu_name): 

    """Get a chunk of a HEX file for a download request used in HMI server"""
    """request_id is mongo ObjectId string of DownloadRequest""" 

    offset = int(request.args.get("offset", 0))
    size = int(request.args.get("size", 8192))

    request_service = current_app.db_service.get_request_service()
    version_service = current_app.db_service.get_version_service()

    req = request_service.get_download_by_id(request_id)
    if not req:
        return jsonify({"error": "DownloadRequest not found"}), 404

    file_info = next((f for f in req.files if f["ecu_name"] == ecu_name), None)
    if not file_info:
        return jsonify({"error": "ECU not part of this request"}), 404

    file_size = file_info["size"]
    hex_path = file_info["hex_path"]

    # 1) If offset already at end → done
    if offset >= file_size:
        return jsonify({"done": True})

    # 2) Read chunk from blob
    chunk = version_service.get_hex_file_chunk(hex_path, size, offset)
    if not chunk:
        return jsonify({"error": "Failed to read chunk"}), 500

    next_offset = offset + len(chunk)

    is_last = (next_offset >= file_size)

    return jsonify({
        "ecu_name": ecu_name,
        "offset": offset,
        "data": chunk.hex(),
        "done": is_last
    })


@version_bp.route('/compatible/<car_type_name>/', methods=['GET'])
@version_bp.route('/compatible/<car_type_name>', methods=['GET'])
@jwt_required()
def get_compatible_versions(car_type_name):
    """Get all versions compatible with a specific car type"""
    version_service = current_app.db_service.get_version_service()
    car_type_service = current_app.db_service.get_car_type_service()

    # Get all car types to extract ECU information
    car_types = car_type_service.get_all()

    compatible_versions = []

    # For each car type, check ECUs and their versions for compatibility
    for car_type in car_types:
        for ecu in car_type.ecus:
            for version in ecu.versions:
                if car_type_name in version.compatible_car_types:
                    compatible_versions.append({
                        'ecu_name': ecu.name,
                        'ecu_model': ecu.model_number,
                        'version_number': version.version_number,
                        'hex_file_path': version.hex_file_path
                    })

    # if not compatible_versions:
    #     return jsonify({'message': 'No compatible versions found'}), 404

    return jsonify(compatible_versions)



@version_bp.route('/upload-to-azure/', methods=['POST'])
@version_bp.route('/upload-to-azure', methods=['POST'])
@admin_required()
def upload_firmware_to_azure():
    """Upload a new firmware version to Azure Blob Storage"""
    try:
        hex_storage_account_name = os.environ.get('HEX_STORAGE_ACCOUNT_NAME')
        hex_storage_container_name = os.environ.get('HEX_STORAGE_CONTAINER_NAME')
        hex_storage_account_key = os.environ.get('HEX_STORAGE_ACCOUNT_KEY')

        # Check if the file is in the request
        if 'file' not in request.files:
            return jsonify({'error': 'No file part'}), 400
            
        file = request.files['file']
        
        # Check if a file was actually selected
        if file.filename == '':
            return jsonify({'error': 'No file selected'}), 400

        # Get other form data
        ecu_name = request.form.get('ecuName')
        ecu_model = request.form.get('ecuModel')
        version_number = request.form.get('versionNumber')
        major_version_number = int(request.form.get('major_version'))
        minor_version_number = int(request.form.get('minor_version'))
        patch_version_number = int(request.form.get('patch_version'))
        compatible_car_types = request.form.get('compatibleCarTypes')
        
        # Validate required fields
        if not all([ecu_name, ecu_model, version_number]):
            return jsonify({'error': 'Missing required fields'}), 400
            
        # Parse compatible car types from JSON string
        compatible_car_types = json.loads(compatible_car_types)

        # Convert all car type names to lowercase
        if isinstance(compatible_car_types, list):
            compatible_car_types = [car_type.lower() if isinstance(car_type, str) else car_type for car_type in compatible_car_types]        

        # Create a unique blob name
        file_extension = os.path.splitext(file.filename)[1]
        blob_name = f"{ecu_name}_{ecu_model}_{version_number}_{uuid.uuid4().hex}{file_extension}"
        
        # Connect to Azure Blob Storage
        connection_string = (f"DefaultEndpointsProtocol=https;AccountName={hex_storage_account_name};"
                            f"AccountKey={hex_storage_account_key};EndpointSuffix=core.windows.net")
        
        blob_service_client = BlobServiceClient.from_connection_string(connection_string)
        container_client = blob_service_client.get_container_client(hex_storage_container_name)
        
        # Upload the file
        blob_client = container_client.get_blob_client(blob_name)
        file_data = file.read()  # Read file content
        
        content_settings = ContentSettings(content_type=file.content_type)
        blob_client.upload_blob(file_data, overwrite=True, content_settings=content_settings)
        
        # Generate the URL for the blob
        blob_url = f"https://{hex_storage_account_name}.blob.core.windows.net/{hex_storage_container_name}/{blob_name}"
        
        # Now create the version in the database with the blob URL as hex_file_path
        db_service = current_app.db_service
        ecu_service = db_service.get_ecu_service()
        
        # Get the ECU first
        ecu = ecu_service.get_by_name_and_model(ecu_name, ecu_model)
        
        # Create the ECU if it doesn't exist
        if not ecu:
            print(f"ECU not found, creating new ECU with name: {ecu_name}, model: {ecu_model}")
            from models import ECU
            ecu = ECU(
                name=ecu_name,
                model_number=ecu_model,
                versions=[]  # Initialize with empty versions list
            )
            # Save the new ECU to the database
            ecu_service.save(ecu)
            print(f"New ECU created: {ecu}")
            
        # Create a new version and add it to the ECU
        from models import Version
        new_version = Version(
            version_number=version_number,
            compatible_car_types=compatible_car_types,
            major_version=major_version_number,
            minor_version=minor_version_number,
            patch_version=patch_version_number,
            hex_file_path=blob_url  # Store the Azure Blob URL
        )
        
        # Add the version to the ECU
        ecu.versions.append(new_version)
        
        # Save the updated ECU
        ecu_service.save(ecu)
        
        # Get the car type service
        car_type_service = db_service.get_car_type_service()

        # Find car types that have this ECU
        for car_type in car_type_service.get_all():
            # Check if this car type contains the ECU we're updating
            ecu_found = False
            for ct_ecu in car_type.ecus:
                if ct_ecu.name == ecu.name and ct_ecu.model_number == ecu.model_number:
                    ecu_found = True
                    break
            
            if ecu_found:
                # Create a list of updated ECUs
                updated_ecus = []
                for ct_ecu in car_type.ecus:
                    if ct_ecu.name == ecu.name and ct_ecu.model_number == ecu.model_number:
                        # Use the updated ECU
                        updated_ecus.append({
                            "name": ecu.name,
                            "model_number": ecu.model_number,
                            "versions": [
                                {
                                    "version_number": v.version_number,
                                    "compatible_car_types": v.compatible_car_types,
                                    "major_version": v.major_version,
                                    "minor_version": v.minor_version,
                                    "patch_version": v.patch_version,
                                    "hex_file_path": v.hex_file_path
                                } for v in ecu.versions
                            ]
                        })
                    else:
                        # Keep the existing ECU
                        updated_ecus.append({
                            "name": ct_ecu.name,
                            "model_number": ct_ecu.model_number,
                            "versions": [
                                {
                                    "version_number": v.version_number,
                                    "compatible_car_types": v.compatible_car_types,
                                    "hex_file_path": v.hex_file_path
                                } for v in (ct_ecu.versions if hasattr(ct_ecu, 'versions') and ct_ecu.versions else [])
                            ]
                        })
                
                # Create the update data dictionary
                update_data = {
                    "ecus": updated_ecus
                }
                
                # Use the update method
                result = car_type_service.update(car_type.name, update_data)
                if result:
                    print(f"Successfully updated ECU in car type: {car_type.name}")
                else:
                    print(f"Failed to update ECU in car type: {car_type.name}")

        
        return jsonify({
            'message': 'Firmware uploaded successfully to Azure Blob Storage',
            'version': {
                'version_number': version_number,
                'hex_file_path': blob_url,
                'compatible_car_types': compatible_car_types
            }
        }), 201
        
    except Exception as e:
        print(f"Error uploading firmware to Azure: {str(e)}")
        return jsonify({'error': f'Failed to upload firmware: {str(e)}'}), 500
    