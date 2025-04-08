#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"
#include "../lib/secrets.h"
#include <ESPAsyncWebServer.h>
#include "UpdatePIDController.h"
#include "Register.h"
#include "ExtraHelpers.h"

const int LED_PIN = 2;
BluetoothSerial SerialBT;
AsyncWebServer server(80);

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);

    connectWiFi();
    registerESP();
    server.on("/updatePID", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleUpdatePID);
    server.begin();

    Serial.println("\nBluetooth Started! Waiting for connection...");
    SerialBT.begin("Porygon-X");
}

void loop() {
    if (SerialBT.available()) {
        String receivedData = SerialBT.readStringUntil('\n');
        receivedData.trim();
        Serial.println(receivedData);
    }
}
