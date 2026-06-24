# Hardware Circuit Design & Pin Mapping

This document details the hardware layer connections for the IoT Smart Energy Meter. 

##🔌 Pin Interconnection Matrix

| Source Component | Component Pin | ESP32 Target Pin | Function / Description |
| :--- | :--- | :--- | :--- |
| **ACS712 (Current)** | VCC | 5V | Hardware Power Supply |
| | GND | GND | Common Ground Reference |
| | OUT | GPIO 35 (ADC1_CH7) | Analog Current Waveform Signal |
| **ZMPT101B (Voltage)**| VCC | 5V / 3.3V | Hardware Power Supply |
| | GND | GND | Common Ground Reference |
| | OUT | GPIO 34 (ADC1_CH6) | Analog Voltage Waveform Signal |
| **5V SPDT Relay** | VCC | 5V | Relay Coil Power |
| | GND | GND | Common Ground Reference |
| | IN | GPIO 23 | Digital Logic Output (Emergency Cutoff) |

## ⚡ Circuit Operational Warnings
1. **High Voltage Safety:** The input side of the ZMPT101B transformer and the terminal blocks of the ACS712 connect directly to mains AC power (230V/50Hz). Exercise extreme caution during physical prototyping.
2. **ADC Attenuation:** Ensure the ESP32 internal ADC attenuation is configured to handle signals up to 3.3V without clipping the peaks of the incoming AC waveforms.
