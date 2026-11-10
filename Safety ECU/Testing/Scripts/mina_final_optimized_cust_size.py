#!/usr/bin/env python3
"""
DoIP/UDS Bootloader Tester — 100% Scapy, ISO 13400 compliant
All packet building AND response parsing uses Scapy layers exclusively.
Supports both raw binary (.bin) and Motorola S-Record (.s19) firmware files.

OPTIMIZED for high-speed Ethernet transfers with configurable block sizes.
"""

import argparse
import sys
import time
import os
import zlib
from pathlib import Path

from scapy.all import Raw
from scapy.contrib.automotive.doip import DoIP, DoIPSocket
from scapy.contrib.automotive.uds import (
    UDS,
    UDS_DSC,  UDS_DSCPR,
    UDS_SA,   UDS_SAPR,
    UDS_RC,   UDS_RCPR,
    UDS_RD,   UDS_RDPR,
    UDS_TD,   UDS_TDPR,
    UDS_RTE,  UDS_RTEPR,
    UDS_ER,   UDS_ERPR,
    UDS_TP,   UDS_TPPR,
    UDS_NR,
)

DOIP_DIAG = 0x8001
DOIP_ACK  = 0x8002
DOIP_NACK = 0x8003
ECU_PORT  = 13400

# Optimized timeouts for Ethernet (faster than CAN/LIN)
TIMEOUT_RESPONSE = 5.0    # 5 seconds for typical Ethernet response
TIMEOUT_PENDING  = 120.0  # 2 minutes for NRC 0x78 pending

# Global flag to reduce debug output
VERBOSE = False


# ---------------------------------------------------------
# S19 / Motorola S-Record Parser
# ---------------------------------------------------------
class S19Parser:
    """Motorola S-Record (.s19) file parser.

    Supports:
      S0  — header
      S1  — data, 16-bit address
      S2  — data, 24-bit address
      S3  — data, 32-bit address
      S7  — end of file, 32-bit start address
      S8  — end of file, 24-bit start address
      S9  — end of file, 16-bit start address
    """

    def __init__(self, filename: str):
        self.filename      = filename
        self.records       = []
        self.start_address = None
        self._parse()

    def _parse(self):
        with open(self.filename, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line = line.strip()
                if not line or not line.startswith('S'):
                    continue
                try:
                    record = self._parse_record(line)
                    if record:
                        self.records.append(record)
                except Exception as e:
                    print(f"   Warning: S19 parse error line {line_num}: {e}")

    def _parse_record(self, line: str):
        if len(line) < 4:
            return None

        record_type = line[0:2]
        byte_count  = int(line[2:4], 16)

        if len(line) != (byte_count * 2 + 4):
            return None

        # Verify checksum
        data_bytes = bytes.fromhex(line[2:])
        if (~sum(data_bytes[:-1]) & 0xFF) != data_bytes[-1]:
            return None

        if record_type == 'S0':
            return {'type': 'header', 'data': data_bytes[1:-1]}

        elif record_type == 'S1':
            address = int(line[4:8], 16)
            data    = bytes.fromhex(line[8:-2])
            return {'type': 'data', 'address': address, 'data': data}

        elif record_type == 'S2':
            address = int(line[4:10], 16)
            data    = bytes.fromhex(line[10:-2])
            return {'type': 'data', 'address': address, 'data': data}

        elif record_type == 'S3':
            address = int(line[4:12], 16)
            data    = bytes.fromhex(line[12:-2])
            return {'type': 'data', 'address': address, 'data': data}

        elif record_type == 'S7':
            self.start_address = int(line[4:12], 16)
            return {'type': 'eof', 'start_address': self.start_address}

        elif record_type == 'S8':
            self.start_address = int(line[4:10], 16)
            return {'type': 'eof', 'start_address': self.start_address}

        elif record_type == 'S9':
            self.start_address = int(line[4:8], 16)
            return {'type': 'eof', 'start_address': self.start_address}

        return None

    def get_memory_range(self):
        """Return (min_address, max_address) covered by all data records."""
        data_records = [r for r in self.records if r['type'] == 'data']
        if not data_records:
            return None, None
        min_addr = min(r['address'] for r in data_records)
        max_addr = max(r['address'] + len(r['data']) for r in data_records)
        return min_addr, max_addr

    def to_flat_binary(self) -> tuple:
        """
        Flatten all S-record data into one contiguous binary blob.
        Gaps between records are padded with 0xFF (erased flash default).
        Returns (base_address, binary_bytes)
        """
        data_records = sorted(
            [r for r in self.records if r['type'] == 'data'],
            key=lambda r: r['address']
        )
        if not data_records:
            raise ValueError("S19 file contains no data records")

        min_addr, max_addr = self.get_memory_range()
        total_size = max_addr - min_addr

        buf = bytearray(b'\xFF' * total_size)
        for rec in data_records:
            offset = rec['address'] - min_addr
            buf[offset: offset + len(rec['data'])] = rec['data']

        return min_addr, bytes(buf)

    def get_crc32(self) -> int:
        """
        CRC32 over the full flat binary INCLUDING 0xFF gap padding.
        Matches exactly what the bootloader received via 0x36 blocks
        and will verify in flash.
        """
        _, flat = self.to_flat_binary()
        return zlib.crc32(flat) & 0xFFFFFFFF


# ---------------------------------------------------------
# Security key — exact match to ECU's generate_recommended_key()
# ---------------------------------------------------------
def generate_recommended_key(seed: int, security_level: int = 1) -> int:
    level_masks = [0x00000000, 0x5A5A5A5A, 0xA5A5A5A5, 0x12345678, 0x87654321]
    seed &= 0xFFFFFFFF
    key      = seed
    rotation = (security_level * 3) % 32
    key = ((key << rotation) | (key >> (32 - rotation))) & 0xFFFFFFFF
    key ^= level_masks[security_level]
    key = (((key & 0xAAAAAAAA) >> 1) | ((key & 0x55555555) << 1)) & 0xFFFFFFFF
    key ^= (~seed) & 0xFFFFFFFF
    return key


# ---------------------------------------------------------
# DoIP receive — skips 0x8002 ACK, raises on 0x8003 NACK
# Returns a parsed Scapy UDS layer from the 0x8001 response
# ---------------------------------------------------------
def recv_uds(sock: DoIPSocket, timeout: float = TIMEOUT_RESPONSE) -> UDS:
    """
    Waits for a DoIP 0x8001 Diagnostic Message response.
    - Silently skips 0x8002 Positive ACK frames (ISO 13400-2 mandatory ACK)
    - Raises RuntimeError on 0x8003 Negative ACK
    - Returns parsed Scapy UDS layer
    """
    deadline = time.time() + timeout
    while time.time() < deadline:
        pkt = sock.recv()
        if pkt is None or DoIP not in pkt:
            continue

        pt = pkt[DoIP].payload_type

        if pt == DOIP_ACK:
            if VERBOSE:
                print("   <- 0x8002 ACK (skipped)")
            continue

        if pt == DOIP_NACK:
            print("   <- 0x8003 NACK")
            pkt.show()
            raise RuntimeError("DoIP NACK (0x8003) received")

        if pt == DOIP_DIAG:
            if UDS in pkt:
                return pkt[UDS]
            # DoIPSocket strips src/tgt addresses — payload is raw UDS bytes
            payload_bytes = bytes(pkt[DoIP].payload)
            return UDS(payload_bytes)

    raise TimeoutError("Timeout waiting for 0x8001 response")


def is_pending(uds_resp: UDS, req_sid: int) -> bool:
    """True if response is NRC 0x78 (responsePending) for the given request SID"""
    return (
        uds_resp.service == 0x7F
        and UDS_NR in uds_resp
        and uds_resp[UDS_NR].requestServiceId == (req_sid & 0xFF)
        and uds_resp[UDS_NR].negativeResponseCode == 0x78
    )


def check_nrc(uds_resp: UDS, ctx: str):
    """Raise a descriptive error if response is a NRC (negative response)"""
    if uds_resp.service == 0x7F and UDS_NR in uds_resp:
        nrc = uds_resp[UDS_NR].negativeResponseCode
        sid = uds_resp[UDS_NR].requestServiceId
        raise RuntimeError(
            f"{ctx} — NRC 0x{nrc:02X} for SID 0x{sid:02X}"
        )


# ---------------------------------------------------------
# Core send/receive — wraps Scapy UDS in DoIP, handles
# 0x8002 ACK skip and NRC 0x78 pending loop
# ---------------------------------------------------------
def send_uds(sock: DoIPSocket, tester: int, ecu: int,
             uds_pkt: UDS,
             timeout: float = TIMEOUT_RESPONSE,
             pending_timeout: float = TIMEOUT_PENDING,
             suppress_output: bool = False) -> UDS:
    """
    uds_pkt : Scapy UDS stack  e.g.  UDS() / UDS_DSC(diagnosticSessionType=0x02)
    Returns : Scapy UDS layer of the positive (or unexpected) response
    suppress_output : if True, don't print request/response details (used for 0x36 blocks)
    """
    pkt = DoIP(
        payload_type=DOIP_DIAG,
        source_address=tester,
        target_address=ecu
    ) / uds_pkt

    req_sid = uds_pkt[UDS].service if UDS in uds_pkt else 0x00
    
    if not suppress_output:
        wire_tx = bytes(uds_pkt)
        print(f"   -> {uds_pkt.summary()}")
        if VERBOSE:
            print(f"   -> WIRE [{wire_tx.hex(' ')}]  ({len(wire_tx)} bytes)")
    
    sock.send(pkt)

    resp = recv_uds(sock, timeout=timeout)
    
    if not suppress_output:
        wire_rx = bytes(resp)
        print(f"   <- {resp.summary()}")
        if VERBOSE:
            print(f"   <- WIRE [{wire_rx.hex(' ')}]  ({len(wire_rx)} bytes)")

    # NRC 0x78 — ECU still processing, keep waiting
    if is_pending(resp, req_sid):
        print(f"   (NRC 0x78 pending, waiting up to {pending_timeout:.0f}s ...)")
        deadline = time.time() + pending_timeout
        while time.time() < deadline:
            resp = recv_uds(sock, timeout=min(1.0, max(0.2, deadline - time.time())))
            if not suppress_output:
                print(f"   <- {resp.summary()}")
            if not is_pending(resp, req_sid):
                break

    return resp


# ---------------------------------------------------------
# UDS services — 100% Scapy layers for build AND parse
# ---------------------------------------------------------

def uds_10_02(sock, tester, ecu):
    """10 02 — Enter Programming Session"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_DSC(diagnosticSessionType=0x02))
    check_nrc(resp, "10 02")

    if resp.service != 0x50 or UDS_DSCPR not in resp:
        raise RuntimeError(f"10 02 unexpected response: {resp.summary()}")

    dscpr = resp[UDS_DSCPR]
    if dscpr.diagnosticSessionType != 0x02:
        raise RuntimeError(f"10 02 wrong session echoed: {dscpr.diagnosticSessionType:#04x}")

    print(f"   ✓ Programming session active")


def uds_27_level1(sock, tester, ecu):
    """27 01 / 27 02 — Security Access level 1"""

    # Step 1: request seed
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_SA(securityAccessType=0x01))
    check_nrc(resp, "27 01")

    if resp.service != 0x67 or UDS_SAPR not in resp:
        raise RuntimeError(f"27 01 unexpected response: {resp.summary()}")

    sa_resp    = resp[UDS_SAPR]
    seed_bytes = bytes(sa_resp.securitySeed)
    seed       = int.from_bytes(seed_bytes, "big")
    key        = generate_recommended_key(seed, 1)
    key_bytes  = key.to_bytes(4, "big")
    print(f"   Seed=0x{seed:08X}  Key=0x{key:08X}")

    # Step 2: send key
    resp2 = send_uds(sock, tester, ecu,
                     UDS() / UDS_SA(securityAccessType=0x02,
                                    securityKey=key_bytes))
    check_nrc(resp2, "27 02")

    if resp2.service != 0x67 or UDS_SAPR not in resp2:
        raise RuntimeError(f"27 02 unexpected response: {resp2.summary()}")

    print("   ✓ Security access unlocked")


def uds_31_erase(sock, tester, ecu, addr: int, size: int):
    """31 01 FF00 — Erase Memory
    ECU wire format: [31][01][FF][00][44][addr 4B][size 4B] = 13 bytes total
    addrLenFormat 0x44: high nibble=4 (size bytes), low nibble=4 (addr bytes)
    """
    option = bytes([0x44]) + addr.to_bytes(4, "big") + size.to_bytes(4, "big")

    uds_pkt = UDS() / UDS_RC(
        routineControlType=0x01,
        routineIdentifier=0xFF00,
    ) / Raw(option)

    if VERBOSE:
        wire = bytes(uds_pkt)
        print(f"   [erase wire] {wire.hex(' ')}  ({len(wire)} bytes)")

    resp = send_uds(sock, tester, ecu, uds_pkt,
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "31 01 FF00")

    if resp.service != 0x71 or UDS_RCPR not in resp:
        raise RuntimeError(f"31 01 FF00 unexpected response: {resp.summary()}")

    rcpr = resp[UDS_RCPR]
    if rcpr.routineIdentifier != 0xFF00:
        raise RuntimeError(f"31 01 FF00 wrong routine echoed: {rcpr.routineIdentifier:#06x}")

    print("   ✓ Memory erased")


def uds_34_request_download(sock, tester, ecu, addr: int, size: int) -> int:
    """34 — Request Download  →  returns chunk size in bytes
    Wire: [34][00][44][addr 4B][size 4B] = 11 bytes
      byte[1]=0x00 : dataFormatIdentifier (no compression)
      byte[2]=0x44 : memorySizeLen=4 (high nibble) | memoryAddressLen=4 (low nibble)
    """
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_RD(
                        dataFormatIdentifier=0x00,
                        memorySizeLen=4,
                        memoryAddressLen=4,
                        memoryAddress4=addr,
                        memorySize4=size
                    ),
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "34")

    if resp.service != 0x74 or UDS_RDPR not in resp:
        raise RuntimeError(f"34 unexpected response: {resp.summary()}")

    rdpr      = resp[UDS_RDPR]
    max_bytes = bytes(rdpr.maxNumberOfBlockLength)
    max_block = int.from_bytes(max_bytes, "big") if max_bytes else 1024
    chunk     = max(1, max_block - 2)   # subtract SID 0x36 + blockSequenceCounter
    print(f"   ✓ Download accepted — maxBlock={max_block}  chunkSize={chunk}")
    return chunk


def uds_36_transfer(sock, tester, ecu, fw: bytes, chunk: int):
    """36 — Transfer Data (all blocks) — OPTIMIZED for speed"""
    bsc   = 1
    off   = 0
    total = len(fw)
    blocks_sent = 0
    start_time = time.time()

    print(f"   Transferring {total} bytes in {chunk}-byte blocks")
    print(f"   Block progress: ", end="", flush=True)

    while off < total:
        part     = fw[off: off + chunk]
        is_last  = (off + len(part) >= total)

        # OPTIMIZATION: Suppress verbose output for data blocks, just show progress
        resp = send_uds(sock, tester, ecu,
                        UDS() / UDS_TD(
                            blockSequenceCounter=bsc & 0xFF,
                            transferRequestParameterRecord=part
                        ),
                        timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING,
                        suppress_output=True)  # <-- CRITICAL OPTIMIZATION
        check_nrc(resp, f"36 block {bsc}")

        if resp.service != 0x76 or UDS_TDPR not in resp:
            raise RuntimeError(f"36 block {bsc} unexpected response: {resp.summary()}")

        tdpr = resp[UDS_TDPR]
        if tdpr.blockSequenceCounter != (bsc & 0xFF):
            raise RuntimeError(
                f"36 block {bsc} wrong seq echoed: {tdpr.blockSequenceCounter}"
            )

        off += len(part)
        pct  = off * 100 // total
        blocks_sent += 1
        
        # Print progress every 10 blocks instead of every block
        if blocks_sent % 10 == 0 or is_last:
            print(f"{pct}%...", end="", flush=True)

        # After the last block, give the ECU time to finish writing flash
        # before we send 0x37. Some ECUs flush asynchronously and will
        # NRC 0x22 or drop the connection if 0x37 arrives too quickly.
        if is_last:
            post_delay = max(0.5, min(5.0, (chunk / 1024) * 0.5))
            print(f"\n   (waiting {post_delay:.1f}s for final flash write ...)")
            time.sleep(post_delay)

        bsc = (bsc % 255) + 1   # 1→2→...→255→1, never 0

    elapsed = time.time() - start_time
    throughput = (total / 1024 / 1024) / max(0.001, elapsed) if elapsed > 0 else 0
    print(f" DONE in {elapsed:.1f}s ({throughput:.2f} MB/s)")


def uds_37_exit(sock, tester, ecu):
    """37 — Request Transfer Exit"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_RTE(),
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "37")

    if resp.service != 0x77:
        raise RuntimeError(f"37 unexpected response: {resp.summary()}")

    print("   ✓ Transfer exit accepted")


def uds_31_check_crc(sock, tester, ecu, crc32: int):
    """31 01 FF01 — Check Memory CRC"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_RC(
                        routineControlType=0x01,
                        routineIdentifier=0xFF01,
                    ) / Raw(crc32.to_bytes(4, "big")),
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "31 01 FF01")

    if resp.service != 0x71 or UDS_RCPR not in resp:
        raise RuntimeError(f"31 01 FF01 unexpected response: {resp.summary()}")

    if resp[UDS_RCPR].routineIdentifier != 0xFF01:
        raise RuntimeError("31 01 FF01 wrong routine echoed")

    print("   ✓ CRC check passed")


def uds_31_finalize(sock, tester, ecu):
    """31 01 FF02 — Finalize Programming"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_RC(
                        routineControlType=0x01,
                        routineIdentifier=0xFF02,
                    ) / Raw(bytes([0x00])),   # NO_SIGNATURE
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "31 01 FF02")

    if resp.service != 0x71 or UDS_RCPR not in resp:
        raise RuntimeError(f"31 01 FF02 unexpected response: {resp.summary()}")

    print("   ✓ Programming finalized")


def uds_11_reset(sock, tester, ecu):
    """11 01 — Hard Reset"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_ER(resetType=0x01),
                    timeout=TIMEOUT_RESPONSE, pending_timeout=TIMEOUT_PENDING)
    check_nrc(resp, "11 01")

    if resp.service != 0x51 or UDS_ERPR not in resp:
        raise RuntimeError(f"11 01 unexpected response: {resp.summary()}")

    erpr = resp[UDS_ERPR]
    if erpr.resetType != 0x01:
        raise RuntimeError(f"11 01 wrong reset type echoed: {erpr.resetType}")

    print("   ✓ ECU reset accepted")


def uds_3e_00(sock, tester, ecu):
    """3E 00 — Tester Present"""
    resp = send_uds(sock, tester, ecu,
                    UDS() / UDS_TP(suppressPosRspMsgIndicator=0x00))
    check_nrc(resp, "3E 00")

    if resp.service != 0x7E or UDS_TPPR not in resp:
        raise RuntimeError(f"3E 00 unexpected response: {resp.summary()}")

    print("   ✓ Tester present acknowledged")


# ---------------------------------------------------------
# Shared flash sequence core — used by both .bin and .s19
# ---------------------------------------------------------
def _run_flash_sequence(sock, tester, ecu,
                        fw: bytes, flash_addr: int,
                        crc32: int,
                        block_size: int = 512,
                        label: str = ""):
    """
    Executes the full 9-step UDS flash sequence for a single
    contiguous firmware blob at a given base address.

    fw        : binary blob to transfer (flat, gap-padded with 0xFF if needed)
    flash_addr: target ECU flash address
    crc32     : pre-computed CRC32 matching what ECU will verify
                - for .bin : zlib.crc32(fw)
                - for .s19 : S19Parser.get_crc32()  — actual data bytes only, no padding
    block_size: chunk size in bytes (from --block_size argument)

    Steps: 10 02 → 27 → 31 FF00 → 34 → 36×N → 37 → 31 FF01 → 31 FF02 → 11 01
    """
    print("\n--- Step 1: Programming Session ---")
    uds_10_02(sock, tester, ecu)

    print("\n--- Step 2: Security Access ---")
    uds_27_level1(sock, tester, ecu)

    print(f"\n--- Step 3: Erase Memory  [{label}] ---")
    uds_31_erase(sock, tester, ecu, flash_addr, len(fw))

    print("\n--- Step 4: Request Download ---")
    negotiated_chunk = uds_34_request_download(sock, tester, ecu, flash_addr, len(fw))
    
    # Use the smaller of ECU max or user-requested block size
    chunk = min(negotiated_chunk, block_size)
    print(f"   → Using block size: {chunk} bytes (negotiated max: {negotiated_chunk}, user requested: {block_size})")

    print("\n--- Step 5: Transfer Data ---")
    uds_36_transfer(sock, tester, ecu, fw, chunk)

    print("\n--- Step 6: Transfer Exit ---")
    uds_37_exit(sock, tester, ecu)

    print(f"\n--- Step 7: CRC Check (0x{crc32:08X}) ---")
    uds_31_check_crc(sock, tester, ecu, crc32)

    print("\n--- Step 8: Finalize Programming ---")
    uds_31_finalize(sock, tester, ecu)

    print("\n--- Step 9: ECU Reset ---")
    uds_11_reset(sock, tester, ecu)


# ---------------------------------------------------------
# Main tester class
# ---------------------------------------------------------
class DoIPBootloaderTester:

    def __init__(self, ip, tester_addr, ecu_addr, port=ECU_PORT):
        self.ip          = ip
        self.port        = port
        self.tester_addr = tester_addr
        self.ecu_addr    = ecu_addr
        self.socket      = None

    def connect(self) -> bool:
        try:
            print(f"\nConnecting to {self.ip}:{self.port} ...")
            self.socket = DoIPSocket(
                self.ip,
                self.port,
                source_address=self.tester_addr,
                activate_routing=True
            )
            print("✓ TCP connection established")
            print("✓ Routing activation completed")
            return True
        except Exception as e:
            print(f"✗ Connection failed: {e}")
            return False

    def close(self):
        if self.socket:
            self.socket.close()
            print("\n✓ Socket closed")

    # ---- individual tests ----

    def test_session(self) -> bool:
        print("\n=== Diagnostic Session Control (0x10) ===")
        try:
            uds_10_02(self.socket, self.tester_addr, self.ecu_addr)
            return True
        except Exception as e:
            print(f"✗ {e}")
            return False

    def test_security(self) -> bool:
        print("\n=== Security Access (0x27) ===")
        try:
            uds_10_02(self.socket, self.tester_addr, self.ecu_addr)
            uds_27_level1(self.socket, self.tester_addr, self.ecu_addr)
            return True
        except Exception as e:
            print(f"✗ {e}")
            return False

    def test_present(self) -> bool:
        print("\n=== Tester Present (0x3E) ===")
        try:
            uds_3e_00(self.socket, self.tester_addr, self.ecu_addr)
            return True
        except Exception as e:
            print(f"✗ {e}")
            return False

    def test_download(self, file_path: str, download_addr: int, block_size: int = 512) -> bool:
        """
        Auto-detects file type by extension:
          .s19 / .srec / .mot  →  S-Record parser → flat binary → flash
          anything else        →  raw binary → flash
        
        block_size: override size in bytes for data transfer blocks
        """
        ext = Path(file_path).suffix.lower()
        if ext in ('.s19', '.srec', '.mot'):
            return self._download_s19(file_path, download_addr, block_size)
        else:
            return self._download_bin(file_path, download_addr, block_size)

    # ---- .bin download ----

    def _download_bin(self, file_path: str, download_addr: int, block_size: int = 512) -> bool:
        print(f"\n=== Firmware Download (.bin) ===")
        print(f"File:    {file_path}")
        print(f"Address: 0x{download_addr:08X}")

        if not os.path.exists(file_path):
            print(f"✗ File not found: {file_path}")
            return False

        fw   = Path(file_path).read_bytes()
        size = len(fw)
        print(f"Size:    {size} bytes ({size / 1024:.1f} KB)")

        # For raw binary: CRC over entire file — no gaps possible
        crc = zlib.crc32(fw) & 0xFFFFFFFF
        print(f"CRC32:   0x{crc:08X}  (over entire binary)")

        try:
            _run_flash_sequence(
                self.socket, self.tester_addr, self.ecu_addr,
                fw, download_addr, crc,
                block_size=block_size,
                label=f"0x{download_addr:08X}  {size / 1024:.1f} KB"
            )
            print("\n✅ FIRMWARE DOWNLOAD COMPLETE (.bin)")
            return True
        except Exception as e:
            print(f"\n✗ Download failed: {e}")
            return False

    # ---- .s19 download ----

    def _download_s19(self, file_path: str, base_addr_override: int, block_size: int = 512) -> bool:
        print(f"\n=== Firmware Download (.s19) ===")
        print(f"File:    {file_path}")

        if not os.path.exists(file_path):
            print(f"✗ File not found: {file_path}")
            return False

        try:
            parser = S19Parser(file_path)
            s19_base, fw = parser.to_flat_binary()
            size = len(fw)

            # If caller passed --addr use it as flash base,
            # otherwise fall back to the address embedded in the S19 records.
            flash_addr = base_addr_override if base_addr_override else s19_base

            min_addr, max_addr = parser.get_memory_range()
            print(f"S19 range:  0x{min_addr:08X} – 0x{max_addr:08X}")
            print(f"Flash base: 0x{flash_addr:08X}")
            print(f"Size:       {size} bytes ({size / 1024:.1f} KB)")

            if parser.start_address is not None:
                print(f"Entry point (S7/S8/S9): 0x{parser.start_address:08X}")

            # CRC over full flat binary including 0xFF gap padding
            crc = parser.get_crc32()
            print(f"CRC32:      0x{crc:08X}  (full flat binary with 0xFF padding)")

            _run_flash_sequence(
                self.socket, self.tester_addr, self.ecu_addr,
                fw, flash_addr, crc,
                block_size=block_size,
                label=f"0x{flash_addr:08X}  {size / 512:.1f} KB  (S19)"
            )
            print("\n✅ FIRMWARE DOWNLOAD COMPLETE (.s19)")
            return True

        except Exception as e:
            print(f"\n✗ S19 download failed: {e}")
            return False

    def run_test(self, test_name: str, **kwargs) -> bool:
        if test_name == "session":
            return self.test_session()
        elif test_name == "security":
            return self.test_security()
        elif test_name == "present":
            return self.test_present()
        elif test_name == "download":
            return self.test_download(kwargs["file"], kwargs["addr"], kwargs.get("block_size", 512))
        print(f"Unknown test: {test_name}")
        return False


# ---------------------------------------------------------
# MAIN
# ---------------------------------------------------------
def main():
    parser = argparse.ArgumentParser(
        description="DoIP/UDS Bootloader Tester — optimized for Ethernet, supports .bin and .s19 firmware files"
    )
    parser.add_argument("--ip",          required=True,
                        help="ECU IP address e.g. 192.168.2.20")
    parser.add_argument("--tester-addr", default="0x0E00",
                        help="Tester logical address (default 0x0E00)")
    parser.add_argument("--ecu-addr",    default="0x0E80",
                        help="ECU logical address (default 0x0E80)")
    parser.add_argument("--test",        default="download",
                        choices=["session", "security", "present", "download"],
                        help="Test to run (default: download)")
    parser.add_argument("--file",
                        help="Firmware file path (.bin or .s19/.srec/.mot)")
    parser.add_argument("--addr",        type=lambda x: int(x, 0),
                        default=0,
                        help="Flash base address e.g. 0x01040000  "
                             "(optional for .s19 — address taken from file if omitted)")
    parser.add_argument("--block_size",  type=int, default=4096,
                        help="Data block size in bytes (default: 4096). Larger = faster but uses more RAM. "
                             "Use --block_size 8192 for large files or 2048 for unreliable networks.")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Enable verbose wire protocol output")

    args = parser.parse_args()
    
    global VERBOSE
    VERBOSE = args.verbose

    tester = DoIPBootloaderTester(
        args.ip,
        int(args.tester_addr, 16),
        int(args.ecu_addr, 16)
    )

    if not tester.connect():
        sys.exit(1)

    kwargs = {}
    if args.test == "download":
        if not args.file:
            print("Error: --file required for download")
            tester.close()
            sys.exit(1)
        ext = Path(args.file).suffix.lower()
        if ext not in ('.s19', '.srec', '.mot') and not args.addr:
            print("Error: --addr required for binary (.bin) download")
            tester.close()
            sys.exit(1)
        kwargs["file"] = args.file
        kwargs["addr"] = args.addr
        kwargs["block_size"] = args.block_size

    success = tester.run_test(args.test, **kwargs)
    tester.close()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
