from fastapi import FastAPI, WebSocket, WebSocketDisconnect, APIRouter
import logging
import json
from models import *
from protocol import Protocol
from datetime import datetime
from enums import *
from hmi_service import HMIService



router = APIRouter()
handlers = HMIService()


@router.websocket("/ecu")
async def ecu_socket(websocket: WebSocket):
        await websocket.accept()
        try:
            
            logging.info(f"starting new thread handling request from client ip: {websocket.client.host} , port number: {websocket.client.port}")
            raw_message = await websocket.receive_text()
            message = json.loads(raw_message)

            if not message:
                logging.error(f"No initial message received ")
                return
            
            logging.info(f"received new message ")  

            if message['type'] != Protocol.HANDSHAKE:
                logging.error(f"Invalid initial message type ")
                await websocket.send_text(Protocol.create_error_message(400, "Invalid initial message (Invalid handshake)"))
                return

            # Extract client information
            payload = message['payload']
            car_type = payload.get('car_type')
            car_id = payload.get('car_id')

            client_cert = await handlers.get_client_cert(websocket)
            serial_number = await handlers.extract_serial(client_cert)
            service_type = ServiceType(payload.get("service_type"))  ## for future use for service type now update and download only

            if not car_type or not car_id:
                logging.error(f"Missing required information in handshake ")
                await websocket.send_text(Protocol.create_error_message(400, "Missing required information"))
                return
            
            logging.info(f"a car with car ID: {car_id} and car_type: {car_type}")

            logging.info(f" a car with car ID: {car_id} and car_type: {car_type}. service needed:: {service_type}")    ## for future use for service type now update and download only

            # Create requestSession object
            request = RequestSession(
                timestamp=datetime.now(),
                car_type=car_type,
                car_id=car_id,
                ip_address= websocket.client.host,
                port=websocket.client.port,
                service_type=service_type,
                metadata=payload.get('metadata', {}),
                status=RequestStatus.CHECKING_AUTHENTICITY
            )
            logging.info(f"new Request has been created for car with client ip:{request.ip_address}. status:{request.status}")

            # Authenticate and process request
            if await handlers.check_authentication(request, websocket,serial_number ):
                logging.info(f"Authentication success for request from client ip:{request.ip_address}")
                await websocket.send_text(Protocol.create_message(Protocol.HANDSHAKE, {
                    'status': 'authenticated',
                    'message': 'Connection established'
                }))

                # Initial update check
                logging.info(f"Performing initial update check for car ID: {car_id}")
                await handlers.check_for_updates(request, websocket)

                # Keep connection alive and handle subsequent requests
                logging.info(f"Maintaining connection for car ID: {car_id} to handle subsequent requests")
                while True:
                    logging.info(f"Waiting for next request from car ID: {car_id}")
                    raw_message = await websocket.receive_text()
                    message = json.loads(raw_message)
                    
                    if not message:
                        logging.info(f"Client {car_id} disconnected gracefully")
                        break

                    logging.info(f"Received request from car ID: {car_id}, message type: {message['type']}")

                    if message['type'] == Protocol.DOWNLOAD_REQUEST:
                        logging.info(f"Processing download request from car ID: {car_id}")
                        request.service_type = ServiceType.DOWNLOAD_UPDATE
                        request.metadata = message['payload']
                        await handlers.handle_download_request(request, websocket)
                        logging.info(f"Completed download request processing for car ID: {car_id}")
                    
                    elif message['type'] == Protocol.UPDATE_CHECK:
                        logging.info(f"Processing update check request from car ID: {car_id}")
                        request.service_type = ServiceType.CHECK_FOR_UPDATE
                        request.metadata = message['payload']
                        await handlers.check_for_updates(request, websocket)
                        logging.info(f"Completed update check for car ID: {car_id}")
                    
                    # NEW: Handle flashing feedback
                    # elif message['type'] == Protocol.FLASHING_FEEDBACK:
                    #     logging.info(f"📋 Processing flashing feedback from car ID: {car_id}")
                    #     await handlers.handle_flashing_feedback(request, message['payload'], websocket)
                    #     logging.info(f"✅ Completed flashing feedback processing for car ID: {car_id}")
                    
                    # # NEW: Handle server metrics request
                    # elif message['type'] == Protocol.SERVER_METRICS_REQUEST:
                    #     logging.info(f"📊 Processing metrics request from car ID: {car_id}")
                    #     await handlers.handle_metrics_request(request, message['payload'], websocket)
                    #     logging.info(f"✅ Completed metrics request for car ID: {car_id}")
                    
                    else:
                        logging.warning(f"Unknown message type '{message['type']}' received from car ID: {car_id}")
                        await websocket.send_text(Protocol.create_error_message(
                            400, f"Unknown message type: {message['type']}"
                        ))


      
        except WebSocketDisconnect:
            logging.warning("Client disconnected")
            return


    