from azure.iot.device import IoTHubDeviceClient
import json

CONNECTION_STRING = "HostName=Deactivation.azure-devices.net;DeviceId=mc180a;SharedAccessKey=jo0Uoid0Z+f4XTXAGVqKcEU9wh0R3uGtCnocDTAHEm0="
client = IoTHubDeviceClient.create_from_connection_string(CONNECTION_STRING)



def handle_message(message):
    data = message.data.decode()

    print("Message received:", data)



def disable_feature(feature):
    print(f"{feature} is now disabled")


client.on_message_received = handle_message
# client.on_twin_desired_properties_patch_received = twin_patch_handler

client.connect()

print("Device listening for updates...")

while True:
    pass