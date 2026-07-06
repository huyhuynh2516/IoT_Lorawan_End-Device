# IoT LoRaWAN End Device
Embedded firmware and hardware implementation of a portable LoRaWAN end device developed for my Graduation Project.
The device collects GPS and LoRaWAN signal information, transmits data to ChirpStack through a LoRaWAN gateway, and supports wireless coverage mapping and prediction.
## Project Overview
This project is part of an end-to-end IoT system for measuring and predicting LoRaWAN network coverage.
The embedded device is responsible for:
- Acquiring GPS coordinates
- Measuring LoRaWAN signal information
- Sending data periodically to ChirpStack via LoRaWAN
- Supporting outdoor data collection for coverage analysis
The collected data is then processed by the backend and visualized on a web application for network coverage prediction.
## Hardware
- STM32F103C8T6
- LoRaWAN Module
- GPS Module
- Antenna
- Battery Supply
## Software
- Arduino IDE
- STM32Cube IDE
- LoRaWAN Library
- GPS Library
##  Repository Structure
f103_data_GPS_Lora/      # Embedded firmware
image/                   # Hardware design, device photos, and system screenshots
README.md
## Features
- GPS location acquisition
- LoRaWAN uplink communication
- Periodic data transmission
- Portable battery-powered operation
- Compatible with ChirpStack network server
## ⚙️ How to Run

### Requirements
- Arduino IDE
- STM32 Board Package
- Required Libraries
- LoRaWAN Gateway
- ChirpStack Server
### Steps
1. Open the project in Arduino IDE.
2. Configure LoRaWAN parameters.
3. Flash the firmware to the STM32 board.
4. Power on the device.
5. Verify uplink packets on ChirpStack.
## System Architecture
STM32 + GPS
      │
      ▼
 LoRaWAN Network
      │
      ▼
 Gateway
      │
      ▼
 ChirpStack
      │
      ▼
 Backend
      │
      ▼
 Database
      │
      ▼
 Web Application
##  Web Dashboard
The collected data can be visualized on the web platform.
**Website:**
https://demo.lora-estimate-map.uk/?lat=16.069198&lng=108.115559
##  My Contribution
My responsibilities in this graduation project include:
- Hardware integration
- Embedded firmware development
- GPS integration
- LoRaWAN communication
- Device testing
- Outdoor measurement and data collection
Backend, web application, and machine learning modules were developed collaboratively by other team members.
## 📄 License
This repository is provided for educational and portfolio purposes.
