from flask import jsonify, request
import base64

from cryptography import x509
from cryptography.hazmat.backends import default_backend

from functools import wraps

from models import Car

def get_client_cert():

    cert_header = request.headers.get("X-ARR-ClientCert")

    if not cert_header:
        return None

    try:

        cert_bytes = base64.b64decode(cert_header)

        cert = x509.load_der_x509_certificate(
            cert_bytes,
            default_backend()
        )

        return cert

    except Exception:
        return None


def get_serial(cert):

    return format(
        cert.serial_number,
        'X'
    ).upper()


def vehicle_mtls_required(f):

    @wraps(f)
    def wrapper(*args, **kwargs):

        cert = get_client_cert()

        if not cert:
            return jsonify({ "error": "Vehicle certificate required" }), 401

        serial = get_serial(cert)

        vehicle = Car.objects(
            certificate_serial=serial,
            is_active=True
        ).first()

        if not vehicle:
            return jsonify({ "error": "Unauthorized vehicle" }), 403    

        request.vehicle = vehicle

        return f(*args, **kwargs)

    return wrapper