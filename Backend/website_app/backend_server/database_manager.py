import os
import certifi
from mongoengine import connect, disconnect
from models import CarType, ECU, Version, ServiceRequest, DownloadRequest
from typing import Optional
import certifi


class DatabaseManager:
    def __init__(self, data_directory: str):
        self.data_directory = data_directory
        os.makedirs(data_directory, exist_ok=True)

        # Read the full SRV URI and target database from environment
        srv_uri = os.environ["MONGO_URI"]
        dbname = os.environ["MONGO_DB"]
        
        if not srv_uri:
            raise ValueError("Environment variable MONGO_URI must be set")

        disconnect(alias='default')
        try:
            self.connection = connect(
                db=dbname,
                host=srv_uri
                # tls=True,
                # tlsCAFile=certifi.where(),
                # uuidRepresentation='standard',
                # connectTimeoutMS=5000,
                # serverSelectionTimeoutMS=5000
            )

            self.connection.admin.command('ping')
        except Exception as e:
            print(f"Failed to connect to MongoDB: {e}")
            raise e


    def close(self):
        disconnect(alias='default')

    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        self.close()  