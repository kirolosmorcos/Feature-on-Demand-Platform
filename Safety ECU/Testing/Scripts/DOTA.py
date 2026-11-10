#!/usr/bin/env python3

import argparse
import socket
import struct
import sys
import time
import json

# --- Diagnostic DoIP Constants ---
DOIP_ROUTING_ACT_REQ     = 0x0005
DOIP_ROUTING_ACT_RES     = 0x0006
DOIP_PT_DIAGNOSTIC_REQ   = 0x4300
DOIP_PT_DIAGNOSTIC_RES   = 0x6300

DIAG_STATUS_SUCCESS      = 0x00
DIAG_STATUS_INVALID_LEN  = 0x01
DIAG_STATUS_FAILED       = 0x02
DIAG_STATUS_INVALID_ID   = 0x03

# --- Network Defaults ---
ECU_PORT = 15000
DEFAULT_TESTER_ADDR = 0x0E00
DEFAULT_ECU_ADDR = 0x0E80


class DoIPFrame:
    @staticmethod
    def build(payload_type: int, payload: bytes) -> bytes:
        return struct.pack(">BBHI", 0x02, 0xFD, payload_type, len(payload)) + payload


class DoIPDiagTester:
    def __init__(self, ip, port=ECU_PORT, tester_addr=DEFAULT_TESTER_ADDR, ecu_addr=DEFAULT_ECU_ADDR):
        self.ip = ip
        self.port = port
        self.tester_addr = tester_addr
        self.ecu_addr = ecu_addr
        self.sock = None

    def connect(self):
        try:
            print(f"[*] Connecting to {self.ip}:{self.port}...")
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5)
            self.sock.connect((self.ip, self.port))
            print("[✓] Connected\n")
            return True
        except Exception as e:
            print(f"[✗] Connection failed: {e}\n")
            return False

    def close(self):
        if self.sock:
            self.sock.close()
            print("[✓] Disconnected")

    def send_frame(self, payload_type, payload):
        frame = DoIPFrame.build(payload_type, payload)
        self.sock.sendall(frame)

    def recv_frame(self):
        try:
            header = self.sock.recv(8)
            if len(header) < 8:
                return None, None

            payload_type = struct.unpack(">H", header[2:4])[0]
            payload_len = struct.unpack(">I", header[4:8])[0]
            payload = b""

            while len(payload) < payload_len:
                chunk = self.sock.recv(payload_len - len(payload))
                if not chunk:
                    break
                payload += chunk

            return payload_type, payload

        except socket.timeout:
            print("\n[✗] Timeout waiting for response")
            return None, None
        except Exception as e:
            print(f"\n[✗] Error receiving frame: {e}")
            return None, None

    def test_routing(self, address_extension=0x10):
        print("[*] Performing Routing Activation...")
        request = struct.pack(">HHB", self.tester_addr, self.ecu_addr, address_extension)
        self.send_frame(DOIP_ROUTING_ACT_REQ, request)
        resp_type, resp_payload = self.recv_frame()

        if resp_type != DOIP_ROUTING_ACT_RES or len(resp_payload) < 6:
            print("[✗] Routing activation failed")
            return False

        if resp_payload[5] == 0x00:
            print("[✓] Routing activated successfully")
            return True

        print(f"[✗] Routing activation rejected. Status=0x{resp_payload[5]:02X}")
        return False

    def request_diagnostics(self, car_id: str):
        print(f"[*] Requesting diagnostic data for Car ID: '{car_id}'...")

        encoded_car_id = car_id.encode('ascii')
        if len(encoded_car_id) > 15:
            encoded_car_id = encoded_car_id[:15]
        
        payload = struct.pack("16s", encoded_car_id)
        
        self.send_frame(DOIP_PT_DIAGNOSTIC_REQ, payload)
        resp_type, resp_payload = self.recv_frame()

        if resp_type != DOIP_PT_DIAGNOSTIC_RES:
            print(f"[✗] Unexpected response type: 0x{resp_type:04X if resp_type else 0}")
            return False, None, None

        if len(resp_payload) < 21:
            print(f"[✗] Response payload too short ({len(resp_payload)} bytes).")
            return False, None, None

        return self.parse_diagnostic_response(resp_payload)

    def parse_diagnostic_response(self, payload: bytes):
        status = payload[0]
        
        if status == DIAG_STATUS_SUCCESS:
            print("[✓] Data retrieved successfully.")
            # Handle struct alignment padding injected by the C compiler
            offset = 17
            if len(payload) >= 22: 
                offset = 18
            
            try:
                # Big-Endian unpack (>HH) as established previously
                temp, pressure = struct.unpack_from(">HH", payload, offset)
                return True, temp, pressure
            except struct.error:
                print("[✗] Could not unpack temperature and pressure fields.")
                return False, None, None
        else:
            print(f"[✗] Diagnostic request failed with status: 0x{status:02X}")
            return False, None, None


def parse_int(value):
    return int(value, 0)


def main():
    parser = argparse.ArgumentParser(description="Standalone DoIP Diagnostic to JSON Exporter")

    # Network Arguments
    parser.add_argument("--ip", required=True, help="IP address of the ECU")
    parser.add_argument("--port", type=int, default=15000, help="ECU TCP Port (Default: 15000)")
    parser.add_argument("--tester-addr", default="0x0E00")
    parser.add_argument("--ecu-addr", default="0x0E80")
    
    # Diagnostic Arguments
    parser.add_argument("--car-id", required=True, help="The 16-character Car ID string to query")
    parser.add_argument("--car-type", default="mercedes c180", help="Car type description for JSON")
    parser.add_argument("--ecu-name", default="ecu1", help="Name of the ECU node for the JSON structure")
    parser.add_argument("--output", default="sensor_data.json", help="Output JSON filename")

    args = parser.parse_args()

    tester = DoIPDiagTester(
        ip=args.ip, 
        port=args.port,
        tester_addr=parse_int(args.tester_addr), 
        ecu_addr=parse_int(args.ecu_addr)
    )

    if not tester.connect():
        sys.exit(1)

    try:
        # Step 1: Routing
        if not tester.test_routing():
            sys.exit(1)

        time.sleep(0.5)

        # Step 2: Request Diagnostics
        success, temp, press = tester.request_diagnostics(args.car_id)

        if success:
            # Step 3: Build and Save JSON
            json_output = {
                "car_id": args.car_id,
                "car_type": args.car_type,
                "sensor_data": {
                    args.ecu_name: {
                        "temprature": temp,
                        "pressure": press
                    }
                }
            }

            with open(args.output, "w") as json_file:
                json.dump(json_output, json_file, indent=4)
            
            print(f"\n[✓] Data successfully saved to {args.output}:")
            print(json.dumps(json_output, indent=4))
            sys.exit(0)
        else:
            sys.exit(1)

    finally:
        tester.close()


if __name__ == "__main__":
    main()