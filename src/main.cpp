#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"
#include "../lib/secrets.h"
#include "HTTPClient.h"
#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"

const int LED_PIN = 2;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

BluetoothSerial SerialBT;
AsyncWebServer server(80);

void customBlink()
{
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

void registerESP()
{
    WiFiClient client;
    HTTPClient http;

    String serverUrl = "http://192.168.133.91:3000/registerESP"; // Replace with actual server IP
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> jsonDoc;
    jsonDoc["recv_IP"] = WiFi.localIP().toString();
    jsonDoc["old_Kp"] = 1.0; // Replace with stored values if available
    jsonDoc["old_Ki"] = 1.0;
    jsonDoc["old_Kd"] = 1.0;

    String requestBody;
    serializeJson(jsonDoc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0)
    {
        Serial.println("ESP32 registered successfully");
    }
    else
    {
        Serial.println("Error in ESP32 registration");
        Serial.println(httpResponseCode);
    }

    http.end();
}

void handleUpdatePID(AsyncWebServerRequest *request)
{
    if (request->hasParam("body", true))
    {
        const AsyncWebParameter *p = request->getParam("body", true);
        String jsonStr = p->value();

        StaticJsonDocument<200> jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, jsonStr);

        if (!error)
        {
            float Kp = jsonDoc["Kp"];
            float Ki = jsonDoc["Ki"];
            float Kd = jsonDoc["Kd"];

            Serial.printf("Updated PID Values -> Kp: %.2f, Ki: %.2f, Kd: %.2f\n", Kp, Ki, Kd);
            request->send(200, "application/json", "{\"message\":\"PID updated\"}");
        }
        else
        {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    }
    else
    {
        request->send(400, "application/json", "{\"error\":\"No body received\"}");
    }
}

void setup()
{

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
    while (WiFi.status() != WL_CONNECTED)
    {
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

    server.on("/updatePID", HTTP_POST, [](AsyncWebServerRequest *request){ 
        handleUpdatePID(request); 
    });
    server.begin();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
        request->send(200, "text/plain", "Hello, world"); 
    });
}

void loop()
{
    if (SerialBT.available())
    {
        String receivedData = SerialBT.readStringUntil('\n');
        receivedData.trim();
        Serial.println(receivedData);
    }
}
