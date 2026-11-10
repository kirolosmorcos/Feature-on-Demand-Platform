import os
import base64
from typing import List, Optional, Dict
from Crypto.PublicKey import RSA
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256
# import bincopy
# import zlib
# import struct

PRIVATE_KEY_PATH = os.getenv("PRIVATE_KEY_PATH")

class SigningService:
    def __init__(self):
        with open(PRIVATE_KEY_PATH, 'r') as key_file:
            self.private_key = RSA.import_key(key_file.read())

    def sign_token(self, token_data: dict) -> None:
        """Sign the token data using RSA and return the signature as a base64-encoded string."""

        license_id_uint = int(str(token_data['license_id'])[-8:], 16)
        ecu_id_uint = int(str(token_data['ecu_id'])[-8:], 16)
        feature_id_uint = int(str(token_data['feature_id'])[-4:], 16)
        valid_from_uint = int(token_data['valid_from'].timestamp())
        valid_until_uint = int(token_data['valid_until'].timestamp()) if token_data['valid_until'] else 0

        data = bytearray(19)
        data[0:4] = license_id_uint.to_bytes(4, 'little')
        data[4:8] = ecu_id_uint.to_bytes(4, 'little')
        data[8:10] = feature_id_uint.to_bytes(2, 'little')
        data[10:14] = valid_from_uint.to_bytes(4, 'little')
        data[14:18] = valid_until_uint.to_bytes(4, 'little')
        data[18] = token_data['issuer_id']

        h = SHA256.new(data)
        signature = pkcs1_15.new(self.private_key).sign(h)

        signature_b64 = base64.b64encode(signature).decode('utf-8')
        # print(len(signature))  # should be 256
        # print(", ".join(str(b) for b in signature))
        token_data['signature'] = signature_b64

    def sign_deactivation_token(self, token_data: dict) -> None:
        """Sign the token data using RSA and return the signature as a base64-encoded string."""

        license_id_uint = int(str(token_data['license_id'])[-8:], 16)
        ecu_id_uint = int(str(token_data['ecu_id'])[-8:], 16)
        feature_id_uint = int(str(token_data['feature_id'])[-4:], 16)
        revoked_at_uint = int(token_data['revoked_at'].timestamp())

        data = bytearray(14)
        data[0:4] = license_id_uint.to_bytes(4, 'little')
        data[4:8] = ecu_id_uint.to_bytes(4, 'little')
        data[8:10] = feature_id_uint.to_bytes(2, 'little')
        data[10:14] = revoked_at_uint.to_bytes(4, 'little')

        h = SHA256.new(data)
        signature = pkcs1_15.new(self.private_key).sign(h)

        signature_b64 = base64.b64encode(signature).decode('utf-8')
        token_data['signature'] = signature_b64

    # def sign_hex_file(self, input_hex_file, output_hex_file) -> None:
    #     """Sign the contents of a hex file and return the signature as a base64-encoded string."""
        
    #     firmware = bincopy.BinFile(input_hex_file)
    #     #firmware.fill(b'\xFF') # check with the ecu

    #     raw_binary = firmware.as_binary()
    #     calculated_crc = zlib.crc32(raw_binary) & 0xFFFFFFFF

    #     crc_bytes = struct.pack('<I', calculated_crc)
    #     crc_address = firmware.maximum_address
    #     firmware.add_binary(crc_bytes, address=crc_address)


    #     with open(output_hex_file, 'w') as f:
    #         f.write(firmware.as_srec(number_of_data_bytes=16))
        

    # if __name__ == "__main__":
    #     signing_service = SigningService()
    #     input_hex_file = r"E:\college\Graduation Project\cmpc_Z4_0.srec"
    #     output_hex_file = r"E:\college\Graduation Project\cmpc_Z4_0_signed.srec"
    #     signing_service.sign_hex_file(input_hex_file, output_hex_file)

    #     ecu = bincopy.BinFile(output_hex_file)

    #     end_address = ecu.maximum_address
    #     crc_address = end_address - 4
    #     embedded_crc_bytes = ecu.as_binary(minimum_address=crc_address, maximum_address=end_address)
    #     embedded_crc = struct.unpack('<I', embedded_crc_bytes)[0]
    #     ecu.fill(b'\xFF')
    #     flash_payload = ecu.as_binary(minimum_address=ecu.minimum_address, maximum_address=crc_address)

    #     ecu_calculated_crc = zlib.crc32(flash_payload) & 0xFFFFFFFF
    #     if embedded_crc == ecu_calculated_crc:
    #         print("\n✅ SUCCESS: The ECUs calculated CRC matches the backend's injected CRC!")
    #     else:
    #         print("\n❌ FAILED: CRCs do not match.")