import requests
import logging
import os


class BackendClient:

    def __init__(self, base_url= os.getenv("BACKEND_SERVER_URL")):
        self.base_url = base_url
        self.timeout = 10
        self.log = logging.getLogger("BackendClient")

         # VM credentials
        self.email = os.getenv("VM_EMAIL")
        self.password = os.getenv("VM_PASSWORD")

        # JWT token
        self.token = None

    def login(self):

        try:

            res = requests.post(
                f"{self.base_url}/auth/login",
                json={
                    "email": self.email,
                    "password": self.password
                },
                timeout=self.timeout
            )

            data = res.json()

            if res.status_code != 200:

                self.log.error(f"VM login failed: {data}")

                return False

            self.token = data["token"]

            self.log.info("VM authenticated successfully")

            return True

        except Exception as e:

            self.log.error(str(e))

            return False
        
      # ------------------------------------------------------------------
    # AUTH HEADERS
    # ------------------------------------------------------------------

    def get_headers(self):
        if not self.token:

            success = self.login()

            if not success:
                raise Exception(
                    "VM authentication failed"
                )

        return {
            "Authorization": f"Bearer {self.token}"
        }
    
    def request_with_auth(self, method, endpoint,
                      retry=True, **kwargs):

        headers = kwargs.pop("headers", {})
        headers.update(self.get_headers())

        response = requests.request(
            method=method,
            url=f"{self.base_url}{endpoint}",
            headers=headers,
            timeout=self.timeout,
            **kwargs
        )

        # VM JWT expired
        if response.status_code == 401 and retry:

            self.log.warning(
                "VM JWT expired. Re-authenticating..."
            )

            # clear old token
            self.token = None

            # login again
            success = self.login()

            if not success:
                raise Exception(
                    "VM login failed"
                )

            # retry original request once
            return self.request_with_auth(
                method,
                endpoint,
                retry=False,
                **kwargs
            )

        return response


    # -------------------------------------------- Backend API Methods ------------------------------


    # 1. AUTHENTICATION --------------------------------------------------------

    def authenticate_car(self, car_id: str, car_type: str,serial_number: str):
        try:
            res = self.request_with_auth(
                "POST",
                "/requests/auth",
                json={
                    "car_id": car_id,
                    "car_type": car_type,
                    "serial_number": serial_number
                }
            )
            return res.json()
        except Exception as e:
            return {"error": str(e)}


    # 2. UPDATE CHECK ----------------------------------------------------------

    def check_for_updates(self, car_type: str, ecu_versions: dict):
        try:
            res = requests.post(
                f"{self.base_url}/requests/update-check",
                json={"car_type": car_type, "ecu_versions": ecu_versions},
                timeout=self.timeout,
                headers=self.get_headers()
            )
            return res.json()
        except Exception as e:
            return {"error": str(e)}


    # 3. CREATE DOWNLOAD REQUEST ----------------------------------------------

    def create_download_request(self, payload: dict):
        """
        Payload expected:
        {
            "car_id": "...",
            "car_type": "...",
            "required_versions": { ECU_Name: {model, version} },
            "old_versions": {...},
            "ip_address": "...",
            "port": 1234
        }
        """
        try:
            res = requests.post(
                f"{self.base_url}/requests/download",
                json=payload,
                timeout=self.timeout,
                headers=self.get_headers()
            )
            return res.json()
        except Exception as e:
            return {"error": str(e)}


    # 4. GET HEX FILE CHUNK ---------------------------------------------------

    def get_chunk(self, request_id: str, ecu_name: str, offset: int, size: int):
        """
        Calls:
          GET /chunk/<request_id>/<ecu_name>?offset=xxx&size=yyy
        """
        try:
            res = requests.get(
                f"{self.base_url}/versions/chunk/{request_id}/{ecu_name}",
                params={"offset": offset, "size": size},
                timeout=self.timeout,
                headers=self.get_headers()
            )
            return res.json()
        except Exception as e:
            return {"error": str(e)}


    # 5. UPDATE OFFSET FOR RESUME ---------------------------------------------

    def update_offset(self, request_id: str, ecu_name: str, offset: int):
        try:
            res = requests.post(
                f"{self.base_url}/requests/download/{request_id}/offset",
                json={"ecu_name": ecu_name, "offset": offset},
                timeout=self.timeout,          
                headers=self.get_headers()
            )
            return res.json()
        except Exception as e:
            return {"error": str(e)}


