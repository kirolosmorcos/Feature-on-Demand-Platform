import requests

url = "https://ota-web-app.azurewebsites.net/api/features/available"

payload = {
    "car_id": "mc180a",
    "current_versions": [
        {
            "ecu": "ecu1",
            "version": "v1.0.0"
        },
        {
            "ecu": "ecu2",
            "version": "v1.0.0"
        }
    ]
}

response = requests.post(
    url,
    json=payload,

    # mTLS certificate
    cert=("vehicle.crt", "vehicle.key")
)

print("Status:", response.status_code)
print(response.json())