#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"
#include "../lib/secrets.h"
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <Preferences.h>

const int LED_PIN = 2;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const String serverURL = WEBSITE_URL;

struct PIDValues {
    float Kp;
    float Ki;
    float Kd;
};

BluetoothSerial SerialBT;
AsyncWebServer server(80);
Preferences preferences;

PIDValues readPIDValues() {
    preferences.begin("PID", true);

    PIDValues pid;
    pid.Kp = preferences.getFloat("Kp", 1.0);
    pid.Ki = preferences.getFloat("Ki", 0.1);
    pid.Kd = preferences.getFloat("Kd", 0.01);

    preferences.end();
    return pid;
}


void savePIDValues(float newKp, float newKi, float newKd) {
    preferences.begin("PID", false);

    preferences.putFloat("Kp", newKp);
    preferences.putFloat("Ki", newKi);
    preferences.putFloat("Kd", newKd);

    preferences.end();
}


void updatePIDValues(float newKp, float newKi, float newKd) {
    PIDValues oldPID = readPIDValues();

    if (oldPID.Kp != newKp || oldPID.Ki != newKi || oldPID.Kd != newKd) {
        Serial.printf("Updated PID Values -> Kp: %.2f, Ki: %.2f, Kd: %.2f\n", newKp, newKi, newKd);
        savePIDValues(newKp, newKi, newKd);
    } else {
        Serial.println("No change in PID Values required");
    }
}

void customBlink() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(200);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(200);
}

void registerESP() {
    WiFiClient client;
    HTTPClient http;

    if (!http.begin(client, serverURL)) {
        Serial.println("HTTP Connection failed!");
        return;
    }
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["recv_IP"] = WiFi.localIP().toString();
    PIDValues pid = readPIDValues();
    jsonDoc["old_Kp"] = pid.Kp;
    jsonDoc["old_Ki"] = pid.Ki;
    jsonDoc["old_Kd"] = pid.Kd;

    String requestBody;
    serializeJson(jsonDoc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0) {
        // Serial.println("ESP32 Registered Successfully");
        Serial.printf("\nESP32 Registered: %s | PID: Kp=%.2f, Ki=%.2f, Kd=%.2f", WiFi.localIP().toString().c_str(), pid.Kp, pid.Ki, pid.Kd);
    }
    else {
        Serial.println("Error in ESP32 registration");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void handleUpdatePID(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    Serial.println("Received PID Update Request");

    String jsonStr = "";
    for (size_t i = 0; i < len; i++) {
        jsonStr += (char)data[i];
    }
    Serial.println("Received JSON: " + jsonStr);

    StaticJsonDocument<200> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, jsonStr);

    if (!error) {
        float newKp = jsonDoc["Kp"];
        float newKi = jsonDoc["Ki"];
        float newKd = jsonDoc["Kd"];

        updatePIDValues(newKp, newKi, newKd);

        customBlink();

        request->send(200, "application/json", "{\"message\":\"PID updated\"}");
    }
    else {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
}

void setup() {

    pinMode(LED_PIN, OUTPUT);

    Serial.begin(115200);

    // <--- Logger for Getting all WiFi connections --->
    // Serial.println("Scanning for available networks...");
    // int numNetworks = WiFi.scanNetworks();

    // if (numNetworks == 0) {
    //     Serial.println("No networks found");
    // } else {
    //     Serial.println("Discovered Networks");
    //     for (int i = 0; i < numNetworks; i++) {
    //         Serial.printf("%d: %s (Signal Strength: %d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    //     }
    // }

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi");
    customBlink();
    Serial.print("\nESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway IP: ");
    Serial.println(WiFi.gatewayIP());

    Serial.println("Bluetooth Started! Waiting for connection...");
    SerialBT.begin("Porygon-X");

    registerESP();

    server.on("/updatePID", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleUpdatePID);

    server.begin();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Hello, world"); });
}

void loop() {
    if (SerialBT.available())
    {
        String receivedData = SerialBT.readStringUntil('\n');
        receivedData.trim();
        Serial.println(receivedData);
    }
}
