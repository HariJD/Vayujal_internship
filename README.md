# IoT-Enabled Atmospheric Water Generator (AWG)

## Overview
This project presents a **fully automated, IoT-enabled Atmospheric Water Generator (AWG)** system designed to extract water from atmospheric air and dispense it safely.  
The system integrates **environmental sensing, real-time control logic, cloud connectivity, and automated safety cutoffs**, making it reliable, scalable, and production-ready.
The complete solution was **designed, implemented, calibrated, tested, and deployed** as part of an internship project at **Vayujal Technologies**.
##  Objectives
- Develop a **fully automated AWG system** based on environmental conditions  
- Enable **real-time water quality monitoring**
- Implement **secure IoT cloud integration**
- Transmit operational data to cloud servers using MQTT
- Ensure **independent control** of:
  - Power
  - Water generation
  - Water dispensing
  - Safety cutoffs

##  System Architecture
The AWG system is divided into **two independent sub-systems**:

###  Generation Unit
Responsible for atmospheric water generation by controlling the compressor.

**Sensors Used:**
- Voltage Sensor
- Current Sensor
- Temperature & Humidity Sensor (SHT25)

**Control Parameters:**
- Ambient temperature & humidity
- Input voltage stability
- Current consumption safety
  
### Dispensing Unit
Responsible for water storage monitoring and safe dispensing.

**Sensors Used:**
- Ultrasonic Sensor (Tank Level)
- TDS Sensor (Water Quality)
- Flow Sensor (Dispensing Rate)

**Control Parameters:**
- Tank level threshold
- Water quality limits
- Flow consistency

## Control Logic

###🔹 Compressor Control (Relay 1)
- Activated only if **voltage, temperature, and humidity** are within safe limits
- Includes a **1-minute safety delay** during startup
- Immediate shutdown if any parameter goes out of range
- Prevents compressor short cycling and mechanical stress

### 🔹 Dispensing Control (Relay 2)
- Activated when:
  - Tank level is within safe limits
  - TDS value is acceptable
- Includes a **1-minute stabilization delay**
- Can operate independently even if generation is OFF

## ☁️ Cloud Integration
The system uses **MQTT protocol** to send real-time data to **Azure IoT Hub**.

### Cloud Architecture:
- **Azure IoT Hub** – Secure device telemetry ingestion
- **Azure Event Grid** – Event routing
- **Azure Functions** – Serverless processing
- **Azure Cosmos DB** – NoSQL sensor data storage
- **Azure SQL Database** – Structured reporting data
- **Azure Logic Apps** – Workflow automation

**Features:**
- Live monitoring dashboards
- Historical data analysis
- Event-based alerts
- Future AI/ML readiness
- 
## Microcontroller Evolution
- **Arduino Mega** – Initial sensor testing & calibration
- **ESP32** – Final deployment with full IoT capability
- Designed to be scalable toward **ESP-IDF** based firmware

---

##  Deployment & Testing

### Phase 1
- Generation and dispensing units tested independently
- Sensor calibration and cutoff logic verified
- Laptop-powered test setup

### Phase 2
- Full system integration using ESP32
- Cloud data transmission validated
- Real-world operational testing
