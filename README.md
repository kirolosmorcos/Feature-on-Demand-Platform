# Feature-on-Demand Platform for Software-Defined Vehicles

A graduation project developed at **Ain Shams University, Faculty of Engineering**, with support from **eJad**.

The project extends an existing Software-Defined Vehicle (SDV) platform with a **Feature-on-Demand (FoD)** workflow and enhanced **Over-the-Air (OTA)** capabilities. It demonstrates how vehicle software features can be delivered, activated, updated, and diagnosed remotely.

The main demonstration use case is **Blind Spot Detection (BSD)**, with vehicle and sensor behavior validated in a simulated environment using CARLA.

## Project Overview

Traditional vehicle software updates often focus on updating complete ECU firmware. This project explores a more flexible approach: delivering and activating individual software features after the vehicle has been manufactured.

The framework brings together a cloud backend, a web dashboard, an in-vehicle HMI, and embedded ECUs. It also extends ECU communication from CAN-oriented workflows toward Automotive Ethernet and TCP/IP.

## Key Features

- **Feature-on-Demand:** Feature selection, entitlement validation, activation, and deactivation.
- **OTA / FOTA updates:** Backend-managed software packages, ECU image versions, and update workflows over Ethernet.
- **Blind Spot Detection:** A safety-related feature executed on the Safety ECU using simulated sensor data from CARLA.
- **Diagnostics Over-the-Air (DoTA):** Remote diagnostic operations using UDS over Ethernet.
- **Authentication and authorization:** Vehicle/user authentication and authorization checks for feature and update operations.
- **License management:** Feature entitlement and license lifecycle handling, including expiration and deactivation.
- **Dashboard and HMI integration:** In-vehicle interfaces for feature workflows, vehicle telemetry, and driver warnings.

## System Architecture

The platform is organized into the following main subsystems:

1. **Cloud backend** — Flask-based services for authentication, feature and license management, OTA package handling, version tracking, and diagnostics.
2. **Web dashboard** — React interface for interacting with backend services and managing platform features.
3. **HMI** — In-vehicle interface and orchestration layer connecting the user, cloud services, and vehicle ECUs.
4. **Safety ECU** — Runs the Blind Spot Detection logic and supports Ethernet-based communication, OTA installation, and diagnostics.
5. **Dashboard ECU** — Displays vehicle telemetry, warnings, and Blind Spot Detection status.
6. **CARLA simulator** — Supplies simulated vehicle/sensor data for the demonstration.

### High-Level Communication Flow

```text
                 Web Dashboard
                       |
                 HTTPS / REST
                       |
                Cloud Backend
                 /         \
       OTA / FoD services   Version & Diagnostics
                 |
             HMI / Gateway
                 |
          Automotive Ethernet
                 |
              Safety ECU
          (Blind Spot Detection)
                 |
          BSD status over TCP
                 |
             Dashboard ECU

       CARLA Simulator
          |         |
     Sensor data   Vehicle telemetry
          |         |
      Safety ECU  Dashboard ECU
```

## Technology Stack

| Area | Technologies |
|---|---|
| Backend | Python, Flask, REST APIs |
| Frontend | React.js |
| Database | MongoDB |
| Cloud infrastructure | Microsoft Azure |
| Deployment | Docker, Docker Compose |
| Real-time communication | WebSocket |
| Vehicle communication | Automotive Ethernet, TCP/IP, TCP, UDP |
| Diagnostics | UDS over Ethernet / DoIP |
| Safety ECU | NXP MPC5748G, FreeRTOS, CycloneTCP |
| HMI platform | Raspberry Pi 4, Android Automotive OS (AAOS) / Automotive Grade Linux (AGL) |
| Dashboard | QNX, LVGL |
| Simulation | CARLA |

## Repository Structure

- `Backend/` — Backend services, APIs, OTA/HMI communication services, and deployment configuration.
- `Dashboard/` — Web dashboard frontend.
- `HMI/` — In-vehicle HMI application and related components.
- `Safety ECU/` — Embedded Safety ECU firmware and related project files.
- `Safety_JS/` — Supporting feature-management components.

## Main Workflows

### Feature Activation

1. The user selects a feature through the application.
2. The request is sent to the backend.
3. The backend checks the user's entitlement and license validity.
4. An authorized activation request is sent to the vehicle HMI.
5. The HMI forwards the request to the Safety ECU.
6. The ECU enables the feature and reports its status.

### OTA Update

1. The backend manages ECU software packages and their versions.
2. The vehicle checks whether an update is available.
3. The update package is delivered to the target ECU through the vehicle communication path.
4. The ECU performs the update workflow and reports the result.
5. The backend's version records are synchronized.

### Blind Spot Detection

1. CARLA provides simulated sensor data.
2. The Safety ECU processes the data and evaluates left/right blind-spot conditions.
3. BSD status is sent to the Dashboard ECU over TCP.
4. The dashboard displays the corresponding driver warning.

### Diagnostics Over-the-Air

The platform includes a DoTA workflow based on UDS over Ethernet, supporting remote diagnostic interaction with vehicle ECUs.

## Demo

The demonstration video will be linked here after it is uploaded to the repository.

## Notes

- This project is a **graduation-project prototype** focused on technical feasibility and functional demonstration in a simulated SDV environment.
- The repository contains multiple subsystems; hardware-specific components may require their corresponding target boards, toolchains, and development environments.
- Cloud services and local development configurations require environment-specific settings. Do not commit real credentials or private keys.

## Acknowledgements

Developed as a Senior-2 Graduation Project in Computer and Systems Engineering, Faculty of Engineering, Ain Shams University (Spring 2026).

We gratefully acknowledge the support of **eJad**, our academic advisor, and the project mentors.
