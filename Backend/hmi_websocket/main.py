from fastapi import FastAPI, WebSocket, WebSocketDisconnect, APIRouter, Request
import logging
from server import router as websocket_router


app = FastAPI()
@app.middleware("http")
async def log_requests(request, call_next):

    print("PATH:", request.url.path)

    print("HEADERS:", request.headers)

    response = await call_next(request)

    print("STATUS:", response.status_code)

    return response
# Register WebSocket routes
@app.websocket("/test")
async def test(websocket: WebSocket):

    await websocket.accept()

    await websocket.send_text("WORKING")
app.include_router(websocket_router)

@app.get("/health")
def health():
    return {"status": "ok"}




# from fastapi import FastAPI, WebSocket, WebSocketDisconnect
# import logging
# import json
# from models import *
# from database_manager import DatabaseManager
# from protocol import Protocol
# from datetime import datetime


# #import debugpy
# import base64

# # debugpy.listen(5678)
# # print("⏳ Waiting for debugger...")


# db_manager = DatabaseManager()
# car_types = db_manager.load_all_data()


# CHUNK_SIZE = 8192  # same as before


# logging.basicConfig(level=logging.INFO)

# app = FastAPI()


# @app.websocket("/ecu")
# async def ecu_socket(websocket: WebSocket):
#     await websocket.accept()
#     logging.info("✅ Client connected")

#     try:
#         raw_message = await websocket.receive_text()
#         message = json.loads(raw_message)

#         if message.get("type") != Protocol.HANDSHAKE:
#             await websocket.send_text(
#                 Protocol.create_error_message(400, "Handshake required")
#             )
#             return

#         payload = message.get("payload", {})
#         car_id = payload.get("car_id")
#         car_type = payload.get("car_type")
#         service_type = payload.get("service_type", ServiceType.CHECK_FOR_UPDATE.value)

#         if not car_id or not car_type:
#             await websocket.send_text(
#                 Protocol.create_error_message(400, "Missing car_id or car_type")
#             )
#             return

#         request = Request(
#             timestamp=datetime.now(),
#             car_id=car_id,
#             car_type=car_type,
#             ip_address="websocket",
#             port=0,
#             service_type=ServiceType(service_type),
#             metadata=payload.get("metadata", {}),
#             status=RequestStatus.CHECKING_AUTHENTICITY
#         )

#         # Authenticate
#         if not authenticate_car(request):
#             await websocket.send_text(
#                 Protocol.create_error_message(401, "Authentication failed")
#             )
#             return

#         await websocket.send_text(
#             Protocol.create_message(Protocol.HANDSHAKE, {
#                 "status": "authenticated",
#                 "message": "Connection established"
#             })
#         )

#         logging.info(f"✅ Car {car_id} authenticated")
#         try:
#             # Keep connection alive (next steps will go here)
#             while True:
#                 msg = json.loads(await websocket.receive_text())
#                 logging.info(f"📩 Next message: {msg}")
                
#                 msg_type = msg.get("type")
#                 payload = msg.get("payload", {})

#                 # Handle different message types here
#                 if msg_type == Protocol.UPDATE_CHECK:
#                     await handle_update_check(
#                     websocket=websocket,
#                     request=request,
#                     payload=payload
#                 )
#                 elif msg_type == Protocol.DOWNLOAD_REQUEST:
#                     await handle_download_request(
#                         websocket=websocket,
#                         request=request,
#                         payload=payload
#                     )
#         except Exception as e:
#             logging.error(f"WS ERROR: {e}")



#     except WebSocketDisconnect:
#         logging.info("❌ Client disconnected")

# def authenticate_car(request: Request) -> bool:
#     try:
#         car_type = next(
#             (ct for ct in car_types if ct.name.lower() == request.car_type.lower()),
#             None
#         )
#         if not car_type:
#             return False

#         if request.car_id.lower() not in [cid.lower() for cid in car_type.car_ids]:
#             return False

#         request.status = RequestStatus.AUTHENTICATED
#         return True

#     except Exception as e:
#         logging.error(f"Auth error: {e}")
#         return False


# async def handle_update_check(websocket, request, payload):
#     """
#     Handle UPDATE_CHECK message
#     """
#     try:
#         # 1️⃣ Get client ECU versions
#         client_versions = payload.get("ecu_versions", {})

       
      

#         # 3️⃣ Find car type
#         car_type = next(
#             (ct for ct in car_types if ct.name == request.car_type),
#             None
#         )

#         if not car_type:
#             await websocket.send_text(
#                 Protocol.create_error_message(
#                     404, "Car type not found"
#                 )
#             )
#             return

#         # 4️⃣ Compare versions
#         updates_needed = car_type.check_for_updates(client_versions)

#         # 5️⃣ Send response
#         await websocket.send_text(
#             Protocol.create_update_response(updates_needed)
#         )

#     except Exception as e:
#         await websocket.send_text(
#             Protocol.create_error_message(
#                 500, f"Update check failed: {str(e)}"
#             )
#         )
# async def handle_download_request(websocket: WebSocket, request: Request, payload: dict):

#     """Handle download request for new ECU versions (WebSocket)"""
#     try:
#         logging.info(
#             f"starting new download request for car:{request.car_id}"
#         )

#         required_versions = payload.get("required_versions", {})
#         old_versions = payload.get("old_versions", {})
#         file_offsets = payload.get("file_offsets", {})

#         if not required_versions:
#             raise Exception("No versions specified for download")

#         download_request = DownloadRequest(
#             timestamp=datetime.now(),
#             car_type=request.car_type,
#             car_id=request.car_id,
#             ip_address="websocket",
#             port=0,
#             required_versions=required_versions,
#             old_versions=old_versions,
#             status=DownloadStatus.PREPARING_FILES,
#             active_transfers={},
#             file_offsets=file_offsets
#         )

#         #active_downloads[request.car_id] = download_request

#         # Start download process
#         await send_new_versions(download_request, websocket)

#     except Exception as e:
#         logging.error(f"Download request error: {str(e)}")
#         request.status = RequestStatus.FAILED
#         await websocket.send_text(
#             Protocol.create_error_message(500, "Download request failed")
#         )
# async def send_new_versions(download_request: DownloadRequest, websocket: WebSocket):
#     """Send new ECU versions to client (WebSocket)"""
#     try:
#         car_type = next(
#             (ct for ct in car_types if ct.name == download_request.car_type),
#             None
#         )

#         if not car_type:
#             raise Exception("Car type not found")

#         download_request.status = DownloadStatus.SENDING_IN_PROGRESS

#         files_info = {}
#         total_size = 0

#         for ecu_name, version_number in download_request.required_versions.items():
#             ecu = next((e for e in car_type.ecus if e.name == ecu_name), None)
#             if not ecu:
#                 continue

#             version = next(
#                 (v for v in ecu.versions if v.version_number == version_number),
#                 None
#             )
#             if not version:
#                 continue

#             file_size = db_manager.get_file_size(version.hex_file_path)
#             total_size += file_size

#             offset = download_request.file_offsets.get(ecu_name, 0)

#             files_info[ecu_name] = {
#                 "path": version.hex_file_path,
#                 "size": file_size,
#                 "transferred": offset
#             }

#         download_request.total_size = total_size

#         # 1️⃣ DOWNLOAD_START
#         await websocket.send_text(
#             Protocol.create_message(
#                 Protocol.DOWNLOAD_START,
#                 {
#                     "total_size": total_size,
#                     "files": {k: v["size"] for k, v in files_info.items()},
#                     "file_offsets": {
#                         k: v["transferred"] for k, v in files_info.items()
#                     }
#                 }
#             )
#         )

#         # 2️⃣ Wait for DOWNLOAD_ACK
#         ack = json.loads(await websocket.receive_text())
#         if ack.get("type") != "DOWNLOAD_ACK":
#             raise Exception("Client did not acknowledge download start")

#         # 3️⃣ Transfer files
#         successful_transfers = 0
#         for ecu_name, file_info in files_info.items():
#             try:
#                 await transfer_file(
#                     websocket,
#                     ecu_name,
#                     file_info["path"],
#                     file_info["size"],
#                     download_request,
#                     start_offset=file_info["transferred"]
#                 )
#                 successful_transfers += 1
#             except Exception as e:
#                 logging.error(f"Error transferring {ecu_name}: {str(e)}")

#         # 4️⃣ DOWNLOAD_COMPLETE
#         await websocket.send_text(
#             Protocol.create_message(
#                 Protocol.DOWNLOAD_COMPLETE,
#                 {
#                     "status": download_request.status.value,
#                     "successful_transfers": successful_transfers,
#                     "total_files": len(files_info)
#                 }
#             )
#         )

#     except Exception as e:
#         logging.error(f"File transfer error: {str(e)}")
#         download_request.status = DownloadStatus.ALL_FAILED
#         await websocket.send_text(
#             Protocol.create_error_message(500, "File transfer failed")
#         )

# async def transfer_file(

#     websocket: WebSocket,
#     ecu_name: str,
#     file_path: str,
#     file_size: int,
#     download_request: DownloadRequest,
#     start_offset: int = 0
# ):
#     """Transfer a single file to client (WebSocket)"""

#     offset = start_offset

#     while offset < file_size:
#         chunk = db_manager.get_hex_file_chunk(
#             file_path,
#             CHUNK_SIZE,
#             offset
#         )

#         if not chunk:
#             raise Exception(f"Failed to read chunk from {file_path}")

#         # SEND FILE_CHUNK
#         await websocket.send_text(
#             Protocol.create_message(
#                 Protocol.FILE_CHUNK,
#                 {
#                     "ecu_name": ecu_name,
#                     "offset": offset,
#                     "data": chunk.hex()
#                 }
#             )
#         )

#         # WAIT FOR CHUNK_ACK (same as TCP)
#         ack = json.loads(await websocket.receive_text())
#         if ack.get("type") != "CHUNK_ACK":
#             raise Exception("Chunk not acknowledged")

#         offset += len(chunk)
#         download_request.transferred_size += len(chunk)
