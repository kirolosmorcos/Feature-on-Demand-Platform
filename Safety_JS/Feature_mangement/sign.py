from Crypto.PublicKey import RSA
from Crypto.Signature import pkcs1_15
from Crypto.Hash import SHA256

data = bytearray(19)

# MUST match C exactly
license_id = 1
ecu_id = 1
feature_id = 1
valid_from = 0
valid_until = 999999
issuer_id = 1

data[0:4] = license_id.to_bytes(4, 'little')
data[4:8] = ecu_id.to_bytes(4, 'little')
data[8:10] = feature_id.to_bytes(2, 'little')
data[10:14] = valid_from.to_bytes(4, 'little')
data[14:18] = valid_until.to_bytes(4, 'little')
data[18] = issuer_id

key = RSA.import_key(open("private.pem").read())
h = SHA256.new(data)

signature = pkcs1_15.new(key).sign(h)

print(len(signature))  # should be 256
print(", ".join(str(b) for b in signature))