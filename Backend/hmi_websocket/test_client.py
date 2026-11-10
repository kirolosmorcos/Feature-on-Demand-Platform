import asyncio
import json
import websockets
import os
import binascii
from datetime import datetime


URI = "ws://localhost:8000/ecu"

async def test_ota_flow():
    async with websockets.connect(URI) as ws:
        print("✅ Connected to server")

        # ------------------------------------------------------------
        # 1️⃣ HANDSHAKE
        handshake = {
            "type": "HANDSHAKE",
            "payload": {
                "car_type": "mercedes c180",
                "car_id": "mc180a",
                "service_type": "checkingForUpdate",
                "metadata": {
                    "ecu_versions": {
                        "ecu1": "v1.0.0"
                    }
                }
            }
        }

        print("➡️ Sending HANDSHAKE")
        await ws.send(json.dumps(handshake))
        response = await ws.recv()
        print("🔁 HANDSHAKE RESPONSE:", response)
        response = await ws.recv()
        print("🔁 Initial Update Response:", response)

                # 2️⃣ Update check
        update_check = {
            "type": "UPDATE_CHECK",
            
            "payload": {
                "ecu_versions": {
                    "ecu1": "v1.0.0",
                    "ecu2": "v1.0.0"
                }
            }
        
        }
        await ws.send(json.dumps(update_check))
        response = await ws.recv()
        data = json.loads(response)
        version_info = data.get("payload", {}).get("updates_needed", {}).get("ecu1", {})
        print("🔁 UPDATE RESPONSE:", response)

        # ------------------------------------------------------------
        # 3️⃣ DOWNLOAD_REQUEST
        download_request = {
        "type": "DOWNLOAD_REQUEST",
        "payload": {
            "required_versions": {
                "ecu1": {
                    "model": "ecu1.21",
                    "version":"v4.1.1"
                },
                "ecu2": {
                    "model": "ecu23",
                    "version":"v2.0.0"
                }

            }
        }
        }

        print("➡️ Sending DOWNLOAD_REQUEST")
        await ws.send(json.dumps(download_request))

        # ------------------------------------------------------------
        # 4️⃣ Receive download messages
        base_dir = "downloaded_hex"
        timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        session_dir = os.path.join(base_dir, timestamp)
        os.makedirs(session_dir, exist_ok=True)
        transferred_files = {}

        while True:
            message = await ws.recv()
            print("🔁 Received message:", message)
            data = json.loads(message)
            msg_type = data.get("type")
            payload = data.get("payload", {})

            print("📩 RECEIVED:", msg_type)

            # ✅ MUST ACK DOWNLOAD_START
            if msg_type == "DOWNLOAD_START":
                await ws.send(json.dumps({
                    "type": "DOWNLOAD_ACK",
                    "payload": {}
                }))
                continue

            elif msg_type == "FILE_CHUNK":
                ecu_name = payload["ecu_name"]
                offset = payload["offset"]
                data_hex = payload["data"]

                chunk_data = binascii.unhexlify(data_hex)
                chunk_size = len(chunk_data)
                transferred_files.setdefault(ecu_name, 0)
                transferred_files[ecu_name] += chunk_size

                if offset % 32768 == 0:
                    print(f"⬇  Receiving {ecu_name}... {offset} bytes")

                # Save chunk to file in this session’s folder
                file_path = os.path.join(session_dir, f"{ecu_name}.hex")
                with open(file_path, "ab") as f:
                    f.write(chunk_data)
                await ws.send(json.dumps({
                    "type": "CHUNK_ACK",
                    "payload": {
                        "ecu_name": data["payload"]["ecu_name"],
                        "offset": data["payload"]["offset"]
                    }
                }))
                continue

            if msg_type == "DOWNLOAD_COMPLETE":
                print("✅ Download completed")
                break

asyncio.run(test_ota_flow())
