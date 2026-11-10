from urllib.parse import unquote
import logging
from datetime import datetime
from models import *
from enums import *
from protocol import Protocol
from backend_client import BackendClient
import json
from cryptography import x509
from cryptography.hazmat.backends import default_backend


class HMIService:
    def __init__(self):
        self.backend = BackendClient()
        # Login once on startup
        if not self.backend .login():
            raise Exception("Backend authentication failed")

        print("VM authenticated successfully")
        self.chunk_size = 8192

    async def get_client_cert(self, websocket):
        client_cert = websocket.headers.get("x-client-cert")
        if not client_cert:

            logging.error("No client certificate provided")

            await websocket.close(code=4001)

        return client_cert
     
    async def extract_serial(self, client_cert):
        
        # Decode escaped nginx cert
        decoded_cert = unquote(client_cert)

        cert = x509.load_pem_x509_certificate(
            decoded_cert.encode(),
            default_backend()
        )

        serial_number = format(cert.serial_number, 'X')

        return serial_number.upper()

    # Keep all existing methods unchanged
    async def check_authentication(self, request: RequestSession, websocket, serial_number: str) -> bool:
        try:
            backend_result = self.backend.authenticate_car(
                car_id=request.car_id,
                car_type=request.car_type,
                serial_number=serial_number
            )

            if backend_result.get("authenticated"):
                request.status = RequestStatus.AUTHENTICATED
                #self.active_requests[request.car_id] = request
                return True

            request.status = RequestStatus.NON_AUTHENTICATED
            raise Exception("Authentication failed")    
            return False

        except Exception as e:
            logging.error(f"Authentication error: {e}")
            request.status = RequestStatus.FAILED
            await websocket.send_text(
                Protocol.create_error_message(500, str(e))
            )

            await websocket.close(code=1008)  # Internal error
            return False


    async def check_for_updates(self, request: RequestSession, websocket):
        try:
            ecu_versions = request.metadata.get("ecu_versions", {})

            backend_result = self.backend.check_for_updates(
                car_type=request.car_type,
                ecu_versions=ecu_versions
            )

            if "error" in backend_result:
                raise Exception(backend_result["error"])

            updates_needed = backend_result.get("updates", {})

            response = Protocol.create_update_response(updates_needed)
            await websocket.send_text(response)

            request.status = RequestStatus.FINISHED_SUCCESSFULLY

        except Exception as e:
            logging.error(f"Update check error: {e}")
            request.status = RequestStatus.FAILED

            await websocket.send_text(
                Protocol.create_error_message(500, str(e))
            )

            await websocket.close(code=1008)  


    # ... Keep all other existing methods unchanged (handle_download_request, send_new_versions, etc.) ...

    async def handle_download_request(self, request: RequestSession, websocket):
     try:
        payload = {
            "car_id": request.car_id,
            "car_type": request.car_type,
            "required_versions": request.metadata.get("required_versions", {}),
            "old_versions": request.metadata.get("old_versions", {}),
            "ip_address": request.ip_address,
            "port": request.port
        }

        backend_response = self.backend.create_download_request(payload)

        if "error" in backend_response:
            raise Exception(backend_response["error"])

        request_id = backend_response["request_id"]
        files = backend_response["files"]

        download_session = DownloadRequestSession(
            timestamp=datetime.now(),
            car_type=request.car_type,
            car_id=request.car_id,
            request_id=request_id,
            files=files,
            status=DownloadStatus.PREPARING_FILES
        )

      #  self.active_downloads[request.car_id] = download_session

        await self.send_new_versions(download_session, websocket)

     except Exception as e:
        logging.error(f"Download request error: {e}")
        request.status = RequestStatus.FAILED
        await websocket.send_text(
            Protocol.create_error_message(500, "Download request failed")
        )
        await websocket.close(code=1008)  

    async def send_new_versions(self, session: DownloadRequestSession, websocket):
        request_id = session.request_id
        files = session.files  # backend provided

        # Build file size map
        file_info = {
            f["ecu_name"]: f["size"]
            for f in files
        }

        start_msg = Protocol.create_message(
            Protocol.DOWNLOAD_START,
            {
                "request_id": request_id,
                "files": file_info
            }
        )
        await websocket.send_text(start_msg)
        logging.info(f"Sent DOWNLOAD_START for request_id={request_id}")

        # Wait for car ack
        raw_message = await websocket.receive_text()
        ack = json.loads(raw_message)
        if not ack or ack["type"] != Protocol.DOWNLOAD_ACK:
            raise Exception("Client did not acknowledge download start")

        # Transfer each file
        successful = 0
        for f in files:
            ecu = f["ecu_name"]
            try:
                await self.transfer_file(
                    websocket,
                    request_id=request_id,
                    ecu_name=ecu,
                    file_size=f["size"]
                )
                successful += 1
            except Exception as e:
                logging.error(f"Error transferring {ecu}: {e}")
                

        if successful == len(files):
            session.status = DownloadStatus.FINISHED_SUCCESSFULLY
        elif successful > 0:
            session.status = DownloadStatus.FAILED_PARTIAL_SUCCESS
        else:
            session.status = DownloadStatus.ALL_FAILED

        complete_msg = Protocol.create_message(
            Protocol.DOWNLOAD_COMPLETE,
            {
                "status": session.status.value,
                "successful_transfers": successful,
                "total_files": len(files)
            }
        )

        await websocket.send_text(complete_msg)

    async def transfer_file(self, websocket, request_id: str, ecu_name: str, file_size: int):
        offset = 0

        while offset < file_size:
            backend_chunk = self.backend.get_chunk(
                request_id=request_id,
                ecu_name=ecu_name,
                offset=offset,
                size=self.chunk_size
            )

            if "error" in backend_chunk:
                raise Exception(backend_chunk["error"])

           

            data_hex = backend_chunk["data"]

            msg = Protocol.create_message(
                Protocol.FILE_CHUNK,
                {
                    "ecu_name": ecu_name,
                    "offset": offset,
                    "data": data_hex
                }
            )
            await websocket.send_text(msg)

            # wait for ack
            raw_message = await websocket.receive_text()
            ack = json.loads(raw_message)
            
            if backend_chunk.get("done"):
                            break
            if not ack or ack["type"] != Protocol.CHUNK_ACK:
                raise Exception("Chunk not acknowledged")

            # Update offset
            offset += len(bytes.fromhex(data_hex))

            # Notify backend
            self.backend.update_offset(
                request_id=request_id,
                ecu_name=ecu_name,
                offset=offset
            )

    # def receive_message(self, client_socket):
    #     try:
    #         return Protocol.parse_message_raw(client_socket)
    #     except:
    #         return None
        
    # def receive_message(self, client_socket) -> Optional[Dict]:
    #     """Receive and parse a message from the client"""
    #     try:
    #         # First receive message length (10 bytes)
    #         length_data = b""
    #         while len(length_data) < 10:
    #             chunk = client_socket.recv(10 - len(length_data))
    #             if not chunk:
    #                 logging.error("Connection closed by peer while receiving message length")
    #                 return None
    #             length_data += chunk
            
    #         message_length = int(length_data.decode())
            
    #         # Receive the actual message
    #         message_data = b""
    #         while len(message_data) < message_length:
    #             remaining = message_length - len(message_data)
    #             chunk = client_socket.recv(min(self.chunk_size, remaining))
    #             if not chunk:
    #                 logging.error("Connection closed by peer while receiving message data")
    #                 return None
    #             message_data += chunk

    #         return Protocol.parse_message(message_data)

    #     except socket.timeout:
    #         logging.error("Socket timeout while receiving message")
    #         return None
    #     except ConnectionError as e:
    #         logging.error(f"Connection error while receiving message: {str(e)}")
    #         return None
    #     except Exception as e:
    #         logging.error(f"Error receiving message: {str(e)}")
    #         return None
