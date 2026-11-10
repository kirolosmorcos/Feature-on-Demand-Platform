# Feature-on-Demand Platform for Software-Defined Vehicles

A graduation project developed at **Ain Shams University, Faculty of Engineering**, with support from **eJad** (Spring 2026).

The project extends an existing Software-Defined Vehicle (SDV) platform with a **Feature-on-Demand (FoD)** framework and enhanced **Over-the-Air (OTA)** capabilities. It demonstrates remote software delivery, feature activation and deactivation, firmware version management, and diagnostics.

The main demonstration use case is **Blind Spot Detection (BSD)**, validated in a simulated vehicle environment using CARLA.

## My Contribution — Backend and Cloud

My work focused on the software platform connecting the web application, cloud services, and vehicle-side components.

- Worked on the **Flask backend** and REST API layer used by the web dashboard and vehicle-management workflows.
- Worked with the backend's layered **Controller–Service–Model** structure, keeping HTTP request handling, business logic, and database access separated.
- Worked with **MongoDB and MongoEngine** for application data, including vehicle, ECU, firmware-version, feature, license, and service-request records.
- Contributed to backend workflows for **OTA package/version management**, firmware retrieval, and feature management.
- Worked on the cloud-connected OTA communication flow, including the HMI/vehicle communication service.
- Worked with the **React dashboard** and its integration with backend APIs.
- Worked on the Azure-based deployment and cloud-service integration described below.

> This section describes my backend/cloud focus. The Safety ECU firmware, embedded dashboard, and HMI platform are team subsystems and are not presented here as my individual contribution.

## Backend Architecture

The main application backend is implemented in **Python using Flask**. It exposes RESTful APIs and exchanges JSON data with the React frontend.

### Layered design

The backend follows a Controller–Service–Model architecture:

```text
React Frontend
      |
  REST / JSON
      |
  Controllers       Handle HTTP requests and responses
      |
   Services         Apply business rules and validation
      |
 MongoEngine Models Map application data to MongoDB
      |
   MongoDB
```

- **Controllers:** Flask route handlers and API endpoints.
- **Services:** Business logic, validation, compatibility checks, and workflow coordination.
- **Models:** MongoEngine document models representing database collections.
- **Schemas:** Marshmallow schemas for request validation and response serialization.

### Main backend modules

- **Authentication:** Registration and login endpoints, JWT-based user authentication, and role-based access control.
- **Vehicle and ECU management:** Car types, ECU records, associations, and compatibility information.
- **Firmware/version management:** ECU firmware version lookup, compatibility checks, firmware upload, and download.
- **Feature-on-Demand:** Feature discovery, entitlement checks, activation configuration, and activation status updates.
- **License lifecycle:** License creation, validity tracking, expiration, and feature deactivation.
- **Diagnostics:** Vehicle health records and remote diagnostic requests.

### OTA firmware delivery

The version-management APIs support:

- Retrieving firmware versions for a given ECU and model.
- Checking firmware compatibility with a vehicle type.
- Downloading firmware packages from Azure Blob Storage.
- Streaming firmware in chunks and tracking download offsets.
- Uploading new firmware packages and registering their metadata in the database.

The project also includes a separate **FastAPI WebSocket OTA service** for persistent vehicle communication. The report describes Nginx as the reverse proxy in front of this service, handling TLS termination and mutual TLS (mTLS) client-certificate validation before forwarding authenticated WebSocket traffic.

### Authentication and security

- **JWT** is used for application user authentication and role-based authorization.
- **Marshmallow** schemas validate incoming request data and serialize API responses.
- **mTLS** is used for vehicle-facing communication: client certificates are validated, and the backend checks the certificate serial number against the registered vehicle record.
- Internal service-to-service communication uses token-based authentication, as described in the project report.

## Azure Cloud Architecture

Microsoft Azure provides the cloud infrastructure and messaging services used by the platform.

| Azure component | Responsibility |
|---|---|
| **Azure App Service** | Hosts the containerized web application (React frontend and Flask backend). |
| **Azure Blob Storage** | Stores ECU firmware images and OTA packages. |
| **Azure IoT Hub** | Provides secure cloud-to-vehicle messaging, including cloud-to-device commands. |
| **Azure Service Bus** | Schedules license-expiration events and holds messages for event-driven processing. |
| **Azure Functions** | Processes Service Bus expiration events and calls the backend's internal expiration endpoint. |
| **Azure Linux Virtual Machine** | Hosts the containerized OTA communication service and Nginx reverse proxy. |

### Cloud deployment

The web application is packaged using Docker and Docker Compose. The frontend and backend run as separate containers, while configuration such as database connection strings and storage credentials is supplied through environment variables.

The OTA communication service is deployed separately on an Azure Linux Virtual Machine:

```text
Vehicle
   |
 HTTPS / WSS + mTLS
   |
 Nginx Reverse Proxy
   |
 FastAPI WebSocket OTA Service
   |
 Backend APIs / Azure services
```

Nginx exposes the secure vehicle-facing endpoint and validates client certificates. The FastAPI service is kept behind the proxy on the internal Docker network.

### Event-driven license expiration

The project uses Azure messaging and serverless processing to deactivate features when their licenses expire:

```text
Backend creates license
        |
Azure Service Bus schedules expiration
        |
Azure Function is triggered
        |
Function calls backend expiration endpoint
        |
Backend marks license as expired
        |
Backend sends command through Azure IoT Hub
        |
Vehicle HMI receives command and forwards it to ECU
        |
ECU disables feature and sends acknowledgment
```

This avoids continuously polling the database for expired licenses and keeps license decisions centralized in the backend.

## System Architecture

The complete platform consists of the following team subsystems:

1. **Cloud backend** — Flask APIs, application logic, authentication, feature/license management, firmware metadata, and diagnostics.
2. **Web dashboard** — React interface for platform administration and user interaction.
3. **OTA communication service** — FastAPI WebSocket service for vehicle update communication, protected by Nginx and mTLS.
4. **HMI** — In-vehicle interface and orchestration layer between cloud services and ECUs.
5. **Safety ECU** — Executes Blind Spot Detection logic and supports Ethernet-based communication, OTA installation, and diagnostics.
6. **Dashboard ECU** — Displays vehicle telemetry, warnings, and Blind Spot Detection status.
7. **CARLA simulator** — Supplies simulated vehicle and sensor data for validation.

## Technology Stack

| Area | Technologies |
|---|---|
| Backend APIs | Python, Flask, REST |
| OTA communication service | FastAPI, WebSocket |
| Frontend | React.js |
| Database / ODM | MongoDB, MongoEngine |
| Validation / serialization | Marshmallow |
| Authentication | JWT, mTLS, X.509 certificates |
| Cloud | Microsoft Azure App Service, Blob Storage, IoT Hub, Service Bus, Azure Functions, Linux VM |
| Deployment | Docker, Docker Compose, Nginx |
| Vehicle communication | Automotive Ethernet, TCP/IP, TCP, UDP |
| Diagnostics | UDS over Ethernet / DoIP |
| Safety ECU | NXP MPC5748G, FreeRTOS, CycloneTCP |
| HMI platform | Raspberry Pi 4, Android Automotive OS (AAOS) / Automotive Grade Linux (AGL) |
| Dashboard ECU | QNX, LVGL |
| Simulation | CARLA |

## Main Workflows

### Feature activation

1. The user selects a feature through the application.
2. The request is sent to the backend.
3. The backend checks feature compatibility, ownership, and license validity.
4. The backend issues an authorized activation configuration.
5. The vehicle HMI forwards the activation request to the target ECU.
6. The ECU enables the feature and reports its status.

### OTA update

1. The backend manages firmware packages and version metadata.
2. The vehicle checks for available compatible updates.
3. The firmware package is retrieved from cloud storage through the OTA workflow.
4. The package is transferred to the target ECU.
5. The ECU performs the update and reports the result.
6. The backend's version records are synchronized.

### Blind Spot Detection

1. CARLA provides simulated sensor data.
2. The Safety ECU processes the data and evaluates left/right blind-spot conditions.
3. BSD status is sent to the Dashboard ECU over TCP.
4. The dashboard displays the corresponding driver warning.

### Diagnostics Over-the-Air

The platform includes a DoTA workflow based on UDS over Ethernet, supporting remote diagnostic interaction with vehicle ECUs.

## Repository Structure

- `Backend/` — Flask application, APIs, models, services, schemas, and related backend services.
- `Dashboard/` — React web dashboard.
- `HMI/` — In-vehicle HMI application and related components.
- `Safety ECU/` — Embedded Safety ECU firmware and related project files.
- `Safety_JS/` — Supporting feature-management components.

## Demo

Demo video: **Add the GitHub-hosted demo link here after uploading it.**

## Project Scope

This is a **graduation-project prototype** focused on technical feasibility and functional demonstration in a simulated SDV environment. Hardware-specific components require their corresponding target boards, toolchains, and development environments.

Cloud services and local development configurations require environment-specific settings. Do not commit real credentials, certificates, or private keys.

## Acknowledgements

Developed as a Senior-2 Graduation Project in Computer and Systems Engineering, Faculty of Engineering, Ain Shams University (Spring 2026), with support from **eJad** and guidance from the academic advisor and project mentors.
