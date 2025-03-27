#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"
#include "../lib/secrets.h"

const int LED_PIN = 2;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

BluetoothSerial SerialBT;

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

    Serial.println("Bluetooth Started! Waiting for connection...");
    SerialBT.begin("Porygon-X");
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
