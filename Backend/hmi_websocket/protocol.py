import json
from typing import Dict, Any

class Protocol:
    # Existing message types
    HANDSHAKE = "HANDSHAKE"
    UPDATE_CHECK = "UPDATE_CHECK"
    UPDATE_RESPONSE = "UPDATE_RESPONSE"
    DOWNLOAD_REQUEST = "DOWNLOAD_REQUEST"
    DOWNLOAD_START = "DOWNLOAD_START"
    FILE_CHUNK = "FILE_CHUNK"
    DOWNLOAD_COMPLETE = "DOWNLOAD_COMPLETE"
    ERROR = "ERROR"
    DOWNLOAD_ACK = "DOWNLOAD_ACK"
    CHUNK_ACK = "CHUNK_ACK"
    
    # NEW: Flashing feedback message types
    FLASHING_FEEDBACK = "FLASHING_FEEDBACK"
    FLASHING_FEEDBACK_ACK = "FLASHING_FEEDBACK_ACK"
    SERVER_METRICS_REQUEST = "SERVER_METRICS_REQUEST"
    SERVER_METRICS_RESPONSE = "SERVER_METRICS_RESPONSE"

    @staticmethod
    def create_message(msg_type: str, payload: Dict) -> str:
        """Create a formatted message to send over socket"""
        message = {
            "type": msg_type,
            "payload": payload
        }
        # Convert to JSON and add message length prefix
        json_msg = json.dumps(message)
        # msg_length = len(json_msg)
        # return f"{msg_length:010d}{json_msg}".encode()
        return json_msg

    @staticmethod
    def parse_message(data: str) -> Dict[str, Any]:
        """Parse received message"""
    
        return json.loads(data)

    @staticmethod
    def create_handshake_response(success: bool, message: str) -> str:
        return Protocol.create_message(Protocol.HANDSHAKE, {
            "success": success,
            "message": message
        })

    @staticmethod
    def create_update_response(updates_needed: Dict[str, str]) -> str:
        return Protocol.create_message(Protocol.UPDATE_RESPONSE, {
            "updates_needed": updates_needed
        })

    @staticmethod
    def create_error_message(error_code: int, error_message: str) ->str:
        return Protocol.create_message(Protocol.ERROR, {
            "code": error_code,
            "message": error_message
        })

    # NEW: Flashing feedback protocol methods
    @staticmethod
    def create_flashing_feedback_ack(success: bool, message: str, session_id: str = None) ->str:
        """Create acknowledgment for flashing feedback"""
        payload = {
            "success": success,
            "message": message,
            "timestamp": json.dumps({"$date": {"$numberLong": str(int(json.loads(json.dumps({})).get("timestamp", 0) * 1000))}}) if "timestamp" in locals() else None
        }
        if session_id:
            payload["session_id"] = session_id
        return Protocol.create_message(Protocol.FLASHING_FEEDBACK_ACK, payload)

    @staticmethod
    def create_metrics_response(metrics: Dict[str, Any]) -> str:
        """Create server metrics response"""
        return Protocol.create_message(Protocol.SERVER_METRICS_RESPONSE, {
            "metrics": metrics,
            "timestamp": json.dumps({"$date": {"$numberLong": str(int(__import__("time").time() * 1000))}})
        })