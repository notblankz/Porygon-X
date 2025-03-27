#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"

const char *ssid = "oneplus";
const char *password = "hahahaha";

BluetoothSerial SerialBT;

void setup()
{
    Serial.begin(115200);
    SerialBT.begin("Porygon-X");
    Serial.println("Bluetooth Started! Waiting for connection...");

    // connecting to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("Connected to WiFi");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
    if (SerialBT.available())
    { // Check if data is received
        String receivedData = SerialBT.readStringUntil('\n');
        receivedData.trim();

        Serial.print("Received: ");
        Serial.println(receivedData);
    }
}
