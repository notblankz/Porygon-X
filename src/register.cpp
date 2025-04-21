#include "Register.h"
#include "../lib/secrets.h"
#include "../include/extraHelpers.h"
#include "../include/updatePIDcontroller.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const String serverURL = WEBSITE_URL;

extern bool isWiFiConnected;

void connectWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    isWiFiConnected = true;
    customBlink();
    Serial.print("\nESP32 IP Address: ");
    Serial.println(WiFi.localIP());
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
        Serial.printf("\nESP32 Registered: %s | PID: Kp=%.2f, Ki=%.2f, Kd=%.2f", WiFi.localIP().toString().c_str(), pid.Kp, pid.Ki, pid.Kd);
    } else {
        Serial.println("Error in ESP32 registration");
        Serial.println(httpResponseCode);
    }
    http.end();
}
