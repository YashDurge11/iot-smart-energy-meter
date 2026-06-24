/**
 * @file firmware.ino
 * @brief IoT Smart Energy Meter with Edge Analytics
 * @author Yash Durge (IIT ISM Dhanbad)
 * * Target Hardware: ESP32 Development Board
 * Sensors: ACS712 (Current), ZMPT101B (Voltage)
 * Protocol: MQTT over Wi-Fi
 */

#include <WiFi.h>
#include <PubSubClient.h>

// --- Configuration Constants ---
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER   = "broker.hivemq.com"; // Public test broker
const int   MQTT_PORT     = 1883;

// --- Hardware Pin Mappings ---
#define VOLTAGE_PIN 34  // ADC1_CH6
#define CURRENT_PIN 35  // ADC1_CH7
#define RELAY_PIN   23  // Digital Out for Emergency Cutoff

// --- Calibration Factors ---
const float VOLTAGE_CALIBRATION = 500.0; 
const float CURRENT_CALIBRATION = 0.185; // 185mV/A for 5A ACS712 model
const float ADC_REF_VOLTAGE     = 3.3;
const int   ADC_RESOLUTION      = 4095;

// --- Edge Analytics Thresholds ---
const float OVERCURRENT_THRESHOLD = 4.5;  // Amperes

// --- Global Instances ---
WiFiClient espClient;
PubSubClient client(espClient);

// --- Function Prototypes ---
void setupWiFi();
void connectMQTT();
void readSensors(float &voltageRMS, float &currentRMS, float &realPower);
void runEdgeAnalytics(float current);

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH); // Initially keep Relay closed (Load ON)

    setupWiFi();
    client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();

    static unsigned long lastTelemetryTime = 0;
    unsigned long currentTime = millis();

    // Sample sensors and publish every 3 seconds
    if (currentTime - lastTelemetryTime >= 3000) {
        float voltageRMS = 0.0;
        float currentRMS = 0.0;
        float realPower   = 0.0;

        readSensors(voltageRMS, currentRMS, realPower);
        runEdgeAnalytics(currentRMS);

        // Prepare JSON payload for MQTT
        String payload = "{";
        payload += "\"voltage_V\":" + String(voltageRMS, 2) + ",";
        payload += "\"current_A\":" + String(currentRMS, 2) + ",";
        payload += "\"power_W\":" + String(realPower, 2);
        payload += "}";

        Serial.print("Publishing Telemetry: ");
        Serial.println(payload);
        
        client.publish("ism/energy/telemetry", payload.c_str());
        lastTelemetryTime = currentTime;
    }
}

/**
 * @brief Establishes connection to the configured local Wi-Fi Network
 */
void setupWiFi() {
    delay(10);
    Serial.println("\nConnecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi Connected. IP Address: ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief Keeps the MQTT client connected to the broker
 */
void connectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);
        
        if (client.connect(clientId.c_str())) {
            Serial.println("Connected to Broker.");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
        }
    }
}

/**
 * @brief Samples AC waveform and calculates True Root Mean Square (RMS) values
 */
void readSensors(float &voltageRMS, float &currentRMS, float &realPower) {
    long long currentSumSquare = 0;
    long long voltageSumSquare = 0;
    int sampleCount = 0;

    unsigned long startTime = millis();
    
    // Sample for exactly 20ms (one complete 50Hz AC cycle)
    while (millis() - startTime < 20) {
        int rawV = analogRead(VOLTAGE_PIN) - (ADC_RESOLUTION / 2); // Remove 1.65V DC Offset
        int rawI = analogRead(CURRENT_PIN) - (ADC_RESOLUTION / 2); // Remove 1.65V DC Offset

        voltageSumSquare += ((long long)rawV * rawV);
        currentSumSquare += ((long long)rawI * rawI);
        sampleCount++;
    }

    // Calculate mean square root
    float meanV = (float)voltageSumSquare / sampleCount;
    float meanI = (float)currentSumSquare / sampleCount;

    // Convert raw ADC RMS values to engineering units
    float rmsAdcV = sqrt(meanV);
    float rmsAdcI = sqrt(meanI);

    voltageRMS = (rmsAdcV * (ADC_REF_VOLTAGE / ADC_RESOLUTION)) * VOLTAGE_CALIBRATION;
    currentRMS = (rmsAdcI * (ADC_REF_VOLTAGE / ADC_RESOLUTION)) / CURRENT_CALIBRATION;
    
    // Prevent noise floor reporting floating values at 0 load
    if (currentRMS < 0.05) currentRMS = 0.0; 

    realPower = voltageRMS * currentRMS; // Apparent power assumption for resistive loads
}

/**
 * @brief Edge Analytics Core: Evaluates safety metrics directly on-chip without cloud latency
 */
void runEdgeAnalytics(float current) {
    if (current > OVERCURRENT_THRESHOLD) {
        // Critical Anomaly Detected: Isolate load immediately
        digitalWrite(RELAY_PIN, LOW); 
        Serial.println("[CRITICAL ALARM] OVERCURRENT DETECTED! RELAY TRIPPED TRIPPED.");
        client.publish("ism/energy/alerts", "{\"status\":\"CRITICAL\",\"alert\":\"Overcurrent anomaly detected. Relay tripped.\"}");
    }
}
