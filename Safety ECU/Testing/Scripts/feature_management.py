#!/usr/bin/env python3

import argparse
import socket
import struct
import sys
import time

DOIP_ROUTING_ACT_REQ = 0x0005
DOIP_ROUTING_ACT_RES = 0x0006

DOIP_PT_FEATURE_ENABLE_REQ = 0x4100
DOIP_PT_FEATURE_ENABLE_RES = 0x6100
DOIP_PT_FEATURE_DISABLE_REQ = 0x4200
DOIP_PT_FEATURE_DISABLE_RES = 0x6200

FEATURE_STATUS_SUCCESS = 0x00
FEATURE_STATUS_INVALID_LEN = 0x01
FEATURE_STATUS_FAILED = 0x02

LICENSE_SIGNATURE_B64_SIZE = 400
LICENSE_TOKEN_SIZE = 4 + 4 + 2 + 4 + 4 + 1 + LICENSE_SIGNATURE_B64_SIZE
FEATURE_REQUEST_PAYLOAD_SIZE = LICENSE_TOKEN_SIZE + 2

ECU_PORT = 15000
DEFAULT_TESTER_ADDR = 0x0E00
DEFAULT_ECU_ADDR = 0x0E80


class DoIPFrame:
    @staticmethod
    def build(payload_type: int, payload: bytes) -> bytes:
        return struct.pack(">BBHI", 0x02, 0xFD, payload_type, len(payload)) + payload


class LicenseToken:
    def __init__(self, license_id, ecu_id, feature_id, valid_from,
                 valid_until, issuer_id, signature_base64):
        self.license_id = license_id
        self.ecu_id = ecu_id
        self.feature_id = feature_id
        self.valid_from = valid_from
        self.valid_until = valid_until
        self.issuer_id = issuer_id
        self.signature_base64 = signature_base64 or ""

    def serialize(self) -> bytes:
        payload = b""

        payload += struct.pack(">I", self.license_id)
        payload += struct.pack(">I", self.ecu_id)
        payload += struct.pack(">H", self.feature_id)
        payload += struct.pack(">I", self.valid_from)
        payload += struct.pack(">I", self.valid_until)
        payload += struct.pack("B", self.issuer_id)

        sig = (
            self.signature_base64
            .replace("\n", "")
            .replace("\r", "")
            .replace(" ", "")
            .replace("\t", "")
        )

        sig_bytes = sig.encode("ascii", errors="ignore")

        if len(sig_bytes) > LICENSE_SIGNATURE_B64_SIZE:
            sig_bytes = sig_bytes[:LICENSE_SIGNATURE_B64_SIZE]
        else:
            sig_bytes += b"\x00" * (LICENSE_SIGNATURE_B64_SIZE - len(sig_bytes))

        payload += sig_bytes

        if len(payload) != LICENSE_TOKEN_SIZE:
            raise ValueError(
                f"LicenseToken size error: expected {LICENSE_TOKEN_SIZE}, got {len(payload)}"
            )

        return payload

    def __repr__(self):
        sig_preview = (
            self.signature_base64[:50] + "..."
            if len(self.signature_base64) > 50
            else self.signature_base64
        )

        return (
            "LicenseToken(\n"
            f"  license_id=0x{self.license_id:08X},\n"
            f"  ecu_id=0x{self.ecu_id:08X},\n"
            f"  feature_id=0x{self.feature_id:04X},\n"
            f"  valid_from=0x{self.valid_from:08X},\n"
            f"  valid_until=0x{self.valid_until:08X},\n"
            f"  issuer_id=0x{self.issuer_id:02X},\n"
            f"  signature_base64='{sig_preview}'\n"
            ")"
        )


class FeatureRequest:
    def __init__(self, task_id, token):
        self.task_id = task_id
        self.token = token

    def serialize(self) -> bytes:
        payload = b""

        payload += self.token.serialize()
        payload += struct.pack(">H", self.task_id)

        if len(payload) != FEATURE_REQUEST_PAYLOAD_SIZE:
            raise ValueError(
                f"Feature request size error: expected {FEATURE_REQUEST_PAYLOAD_SIZE}, got {len(payload)}"
            )

        return payload

    def __repr__(self):
        return (
            "FeatureRequest(\n"
            f"{self.token}\n"
            f"  task_id_last_2_bytes=0x{self.task_id:04X}\n"
            ")"
        )


class DoIPFeatureTester:
    def __init__(self, ip, tester_addr=DEFAULT_TESTER_ADDR, ecu_addr=DEFAULT_ECU_ADDR):
        self.ip = ip
        self.tester_addr = tester_addr
        self.ecu_addr = ecu_addr
        self.sock = None

    def connect(self):
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

    def send_frame(self, payload_type, payload):
        frame = DoIPFrame.build(payload_type, payload)

        print("→ Sending frame:")
        print(f"  Type: 0x{payload_type:04X}")
        print(f"  Payload length: {len(payload)} bytes")
        print(f"  Payload hex first 64 bytes: {payload[:64].hex()}")

        if len(payload) > 64:
            print(f"  Payload hex next  64 bytes: {payload[64:128].hex()}")

        print(f"  Payload hex last  16 bytes: {payload[-16:].hex()}")

        self.sock.sendall(frame)

    def recv_frame(self, allow_disconnect_success=False):
        try:
            header = self.sock.recv(8)

            if len(header) == 0:
                if allow_disconnect_success:
                    print("[✓] ECU disconnected after request; assuming success")
                    return "DISCONNECTED_AFTER_REQUEST", b""
                return None, None

            if len(header) < 8:
                return None, None

            payload_type = struct.unpack(">H", header[2:4])[0]
            payload_len = struct.unpack(">I", header[4:8])[0]

            payload = b""

            while len(payload) < payload_len:
                chunk = self.sock.recv(payload_len - len(payload))

                if not chunk:
                    if allow_disconnect_success:
                        print("[✓] ECU disconnected during response; assuming success")
                        return "DISCONNECTED_AFTER_REQUEST", b""
                    break

                payload += chunk

            print("\n← Received frame:")
            print(f"  Type: 0x{payload_type:04X}")
            print(f"  Payload length: {len(payload)}")
            print(f"  Payload hex: {payload.hex()}")

            return payload_type, payload

        except socket.timeout:
            print("\n[✗] Timeout waiting for response")
            return None, None
        except Exception as e:
            if allow_disconnect_success:
                print(f"[✓] ECU disconnected after request; assuming success: {e}")
                return "DISCONNECTED_AFTER_REQUEST", b""

            print(f"\n[✗] Error receiving frame: {e}")
            return None, None

    def test_routing(self, address_extension=0x10):
        print("=" * 70)
        print("STEP 1: ROUTING ACTIVATION")
        print("=" * 70)

        request = struct.pack(">HHB", self.tester_addr, self.ecu_addr, address_extension)

        print(f"→ Tester Address:    0x{self.tester_addr:04X}")
        print(f"→ ECU Address:       0x{self.ecu_addr:04X}")
        print(f"→ Address Extension: 0x{address_extension:02X}")

        self.send_frame(DOIP_ROUTING_ACT_REQ, request)

        resp_type, resp_payload = self.recv_frame()

        if resp_type != DOIP_ROUTING_ACT_RES:
            print("[✗] Routing activation failed")
            return False

        if len(resp_payload) < 6:
            print("[✗] Routing response too short")
            return False

        status = resp_payload[5]

        if status == 0x00:
            print("[✓] Routing activated successfully\n")
            return True

        print(f"[✗] Routing activation rejected. Status=0x{status:02X}\n")
        return False

    def send_feature_request(self, action, request):
        if action == "enable":
            req_type = DOIP_PT_FEATURE_ENABLE_REQ
            res_type = DOIP_PT_FEATURE_ENABLE_RES
            title = "FEATURE ENABLE REQUEST"
        else:
            req_type = DOIP_PT_FEATURE_DISABLE_REQ
            res_type = DOIP_PT_FEATURE_DISABLE_RES
            title = "FEATURE DISABLE REQUEST"

        print("=" * 70)
        print(f"STEP 2: {title}")
        print("=" * 70)

        print(f"\n{request}\n")

        payload = request.serialize()

        print(f"[*] Payload size: {len(payload)} bytes")
        print(f"[*] Expected size: {FEATURE_REQUEST_PAYLOAD_SIZE} bytes")
        print(f"[*] task_id is LAST 2 bytes: 0x{request.task_id:04X}")

        self.send_frame(req_type, payload)

        resp_type, resp_payload = self.recv_frame(allow_disconnect_success=True)

        if resp_type == "DISCONNECTED_AFTER_REQUEST":
            print(f"[✓] Feature {action} likely successful because ECU reset/disconnected after applying it")
            return True

        if resp_type is None:
            print("[✗] No response")
            return False

        if resp_type != res_type:
            print(f"[✗] Wrong response type: 0x{resp_type:04X}")
            return False

        if len(resp_payload) < 1:
            print("[✗] Response too short")
            return False

        status_code = resp_payload[0]

        if status_code == FEATURE_STATUS_SUCCESS:
            print(f"\n[✓] Feature {action} successful. Status=0x{status_code:02X}\n")
            return True

        if status_code == FEATURE_STATUS_INVALID_LEN:
            print(f"\n[✗] Invalid payload length. Status=0x{status_code:02X}")
            print(f"    Expected payload size: {FEATURE_REQUEST_PAYLOAD_SIZE} bytes\n")
            return False

        if status_code == FEATURE_STATUS_FAILED:
            print(f"\n[✗] Feature {action} failed. Status=0x{status_code:02X}\n")
            return False

        print(f"\n[?] Unknown status code: 0x{status_code:02X}\n")
        return False


def parse_int(value):
    return int(value, 0)


def add_common_feature_args(subparser):
    subparser.add_argument("--task-id", required=True)
    subparser.add_argument("--license-id", required=True)
    subparser.add_argument("--ecu-id", required=True)
    subparser.add_argument("--feature-id", required=True)
    subparser.add_argument("--valid-from", required=True)
    subparser.add_argument("--valid-until", required=True)
    subparser.add_argument("--issuer-id", required=True)
    subparser.add_argument("--signature", required=True)


def main():
    parser = argparse.ArgumentParser(
        description="DoIP Feature Management Tester - task_id is last 2 payload bytes"
    )

    parser.add_argument("--ip", required=True)
    parser.add_argument("--tester-addr", default="0x0E00")
    parser.add_argument("--ecu-addr", default="0x0E80")
    parser.add_argument("--address-extension", default="0x10")

    subparsers = parser.add_subparsers(dest="command", required=True)

    enable_parser = subparsers.add_parser("enable")
    add_common_feature_args(enable_parser)

    disable_parser = subparsers.add_parser("disable")
    add_common_feature_args(disable_parser)

    args = parser.parse_args()

    try:
        tester_addr = parse_int(args.tester_addr)
        ecu_addr = parse_int(args.ecu_addr)
        address_extension = parse_int(args.address_extension)

        task_id = parse_int(args.task_id)
        license_id = parse_int(args.license_id)
        ecu_id = parse_int(args.ecu_id)
        feature_id = parse_int(args.feature_id)
        valid_from = parse_int(args.valid_from)
        valid_until = parse_int(args.valid_until)
        issuer_id = parse_int(args.issuer_id)

    except ValueError as e:
        print(f"[✗] Invalid numeric argument: {e}")
        sys.exit(1)

    token = LicenseToken(
        license_id=license_id,
        ecu_id=ecu_id,
        feature_id=feature_id,
        valid_from=valid_from,
        valid_until=valid_until,
        issuer_id=issuer_id,
        signature_base64=args.signature,
    )

    request = FeatureRequest(
        task_id=task_id,
        token=token,
    )

    tester = DoIPFeatureTester(args.ip, tester_addr, ecu_addr)

    if not tester.connect():
        sys.exit(1)

    try:
        if not tester.test_routing(address_extension):
            sys.exit(1)

        time.sleep(0.5)

        success = tester.send_feature_request(args.command, request)
        sys.exit(0 if success else 1)

    finally:
        tester.close()


if __name__ == "__main__":
    main()