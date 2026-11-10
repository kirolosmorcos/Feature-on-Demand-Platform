#!/usr/bin/env python3
"""
DoIP Diagnostic Message (0x8001) - Socket-based Tester
Direct TCP socket implementation without Scapy DoIPSocket
"""

import argparse
import sys
import socket
import struct
import time

DOIP_DIAG = 0x8001
DOIP_ACK = 0x8002
DOIP_NACK = 0x8003
DOIP_ROUTING_ACT_REQ = 0x0005
DOIP_ROUTING_ACT_RES = 0x0006

ECU_PORT = 15000
DEFAULT_TESTER_ADDR = 0x0E00
DEFAULT_ECU_ADDR = 0x0E80


class DoIPFrame:
    """DoIP Frame builder"""
    
    @staticmethod
    def build(payload_type: int, payload: bytes) -> bytes:
        """
        Build DoIP frame:
        [Version=0x02] [Inverse=0xFD] [Type(2B BE)] [Length(4B BE)] [Payload]
        """
        header = struct.pack('>BBHI', 0x02, 0xFD, payload_type, len(payload))
        return header + payload
    
    @staticmethod
    def parse(data: bytes):
        """Parse DoIP frame header and return (type, payload)"""
        if len(data) < 8:
            return None, None
        
        version = data[0]
        inverse = data[1]
        payload_type = struct.unpack('>H', data[2:4])[0]
        payload_len = struct.unpack('>I', data[4:8])[0]
        
        if version != 0x02 or inverse != 0xFD:
            return None, None
        
        if len(data) < 8 + payload_len:
            return None, None
        
        payload = data[8:8+payload_len]
        return payload_type, payload


class DoIPTester:
    """DoIP Tester using raw TCP socket"""

    def __init__(self, ip, tester_addr=DEFAULT_TESTER_ADDR, ecu_addr=DEFAULT_ECU_ADDR):
        self.ip = ip
        self.tester_addr = tester_addr
        self.ecu_addr = ecu_addr
        self.sock = None

    def connect(self) -> bool:
        try:
            print(f"[*] Connecting to {self.ip}:{ECU_PORT}...")
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5)
            self.sock.connect((self.ip, ECU_PORT))
            print("[✓] Connected\n")
            return True
        except Exception as e:
            print(f"[✗] Connection failed: {e}\n")
            return False

    def close(self):
        if self.sock:
            self.sock.close()
            print("[✓] Disconnected")

    def send_frame(self, payload_type: int, payload: bytes):
        """Send DoIP frame"""
        frame = DoIPFrame.build(payload_type, payload)
        print(f"→ Sending frame:")
        print(f"  Type: 0x{payload_type:04X}")
        print(f"  Payload length: {len(payload)}")
        print(f"  Payload: {payload.hex()}")
        self.sock.send(frame)

    def recv_frame(self):
        """Receive DoIP frame"""
        try:
            header = self.sock.recv(8)
            if len(header) < 8:
                return None, None
            
            payload_len = struct.unpack('>I', header[4:8])[0]
            payload = b""
            
            while len(payload) < payload_len:
                chunk = self.sock.recv(payload_len - len(payload))
                if not chunk:
                    break
                payload += chunk
            
            payload_type = struct.unpack('>H', header[2:4])[0]
            
            print(f"\n← Received frame:")
            print(f"  Type: 0x{payload_type:04X}")
            print(f"  Payload length: {len(payload)}")
            print(f"  Payload: {payload.hex()}")
            
            return payload_type, payload
        except socket.timeout:
            print("\n[✗] Timeout waiting for response")
            return None, None
        except Exception as e:
            print(f"\n[✗] Receive error: {e}")
            return None, None

    def test_routing(self) -> bool:
        """Test: Routing Activation (0x0005)"""
        print("="*70)
        print("TEST: ROUTING ACTIVATION")
        print("="*70)
        
        # Payload: [TA_type=0x00] [Tester Addr]
        req_payload = bytes([0x00]) + struct.pack('>H', self.tester_addr)
        
        self.send_frame(DOIP_ROUTING_ACT_REQ, req_payload)
        
        resp_type, resp_payload = self.recv_frame()
        
        if resp_type is None:
            print("[✗] No response")
            return False
        
        if resp_type != DOIP_ROUTING_ACT_RES:
            print(f"[✗] Wrong response type: 0x{resp_type:04X}")
            return False
        
        if len(resp_payload) < 5:
            print(f"[✗] Response too short: {len(resp_payload)} bytes")
            return False
        
        response_code = resp_payload[4]
        
        if response_code == 0x10:
            print("\n[✓] Routing activated successfully\n")
            return True
        else:
            print(f"\n[✗] Routing failed with code 0x{response_code:02X}\n")
            return False

    def test_diagnostics(self, data_hex: str) -> bool:
        """
        Test: Diagnostic Message (0x8001)
        
        Payload format:
        [Tester Addr (2B)] [ECU Addr (2B)] [Data (N bytes)]
        """
        print("="*70)
        print("TEST: DIAGNOSTIC MESSAGE (0x8001)")
        print("="*70)
        
        # Parse hex data
        try:
            data = bytes.fromhex(data_hex.replace(' ', ''))
        except ValueError:
            print(f"[✗] Invalid hex: {data_hex}\n")
            return False
        
        # Build diagnostic payload: [Tester] [ECU] [Data]
        diag_payload = (
            struct.pack('>H', self.tester_addr) +
            struct.pack('>H', self.ecu_addr) +
            data
        )
        
        print(f"\n  Tester: 0x{self.tester_addr:04X}")
        print(f"  ECU: 0x{self.ecu_addr:04X}")
        print(f"  Data: {data.hex()}\n")
        
        self.send_frame(DOIP_DIAG, diag_payload)
        
        resp_type, resp_payload = self.recv_frame()
        
        if resp_type is None:
            print("[✗] No response\n")
            return False
        
        if resp_type == DOIP_ACK:
            print("\n[⚠] ACK received (0x8002) - waiting for response...")
            resp_type, resp_payload = self.recv_frame()
            if resp_type is None:
                print("[✗] No response after ACK\n")
                return False
        
        if resp_type == DOIP_NACK:
            print(f"\n[✗] NACK received (0x8003)\n")
            return False
        
        if resp_type == DOIP_DIAG:
            print(f"\n[✓] Diagnostic response received\n")
            return True
        else:
            print(f"\n[?] Unknown response type: 0x{resp_type:04X}\n")
            return False


def main():
    parser = argparse.ArgumentParser(
        description="DoIP Tester - Routing & Diagnostic Messages"
    )
    parser.add_argument("--ip", required=True, help="ECU IP address")
    parser.add_argument("--tester-addr", default="0x0E00", help="Tester address")
    parser.add_argument("--ecu-addr", default="0x0E80", help="ECU address")
    parser.add_argument("--test", required=True, choices=["routing", "diagnostics"],
                       help="Test to run")
    parser.add_argument("--data", default="1002", help="Hex data to send (default: 1002)")

    args = parser.parse_args()

    tester = DoIPTester(
        args.ip,
        int(args.tester_addr, 16),
        int(args.ecu_addr, 16)
    )

    if not tester.connect():
        sys.exit(1)

    try:
        if args.test == "routing":
            success = tester.test_routing()
        else:  # diagnostics
            if not tester.test_routing():
                print("[✗] Routing failed - cannot test diagnostics\n")
                sys.exit(1)
            time.sleep(0.5)
            success = tester.test_diagnostics(args.data)
        
        sys.exit(0 if success else 1)
        
    finally:
        tester.close()


if __name__ == "__main__":
    main()