# IoT-Based Smart Energy Meter with Edge Analytics

An industrial-grade IoT energy monitoring system built on the **ESP32 MCU architecture** that measures AC consumption metrics (Voltage, Current, Power) and executes safety-edge analytics locally to prevent electrical hardware damage.

## 🚀 Key System Features
* **True RMS Calculations:** Samples AC sine-wave patterns sequentially over a 20ms time block (50Hz frequency standard) to eliminate zero-crossing cancellation calculation artifacts.
* **Edge Intelligence Optimization:** Locally processes current consumption limits on-chip. Bypasses cloud compute latency to trip safety relays in under **20 milliseconds** upon fault detection.
* **Telemetry Streaming:** Publishes system metrics seamlessly as compact JSON payloads via **MQTT architecture** over Wi-Fi.

## 🛠️ Tech Stack & Components
* **Hardware:** ESP32 Board, ACS712 Current Sensor (5A Variant), ZMPT101B Voltage Transformer Module, 5V SPDT Relay.
* **Firmware/Development Framework:** Embedded C++ (Arduino Core Platform), PubSubClient MQTT Library.
* **Protocols:** MQTT, TCP/IP, WebSockets.

## 📈 System Architectural Workflow

1. **Physical Sensing Layer:** Sensor probes continuously interface into ESP32 ADC1 pins.
2. **On-Chip Signal Processing:** Firmware cancels out standard 1.65V operational hardware bias offsets and executes Root-Mean-Square math algorithms.
3. **Edge Decision Logic:** Evaluation pipeline checks if local threshold rules are breached.
4. **Cloud Reporting:** Telemetry payload parses cleanly outward to specific MQTT Broker queues.

## 🔧 Installation & Verification Setup

### Prerequisites
Install the following libraries inside your development workspace environment:
* PubSubClient by Nick O'Leary

---
Developed by an Electronics & Communication Engineering undergraduate student at **IIT (ISM) Dhanbad**.
