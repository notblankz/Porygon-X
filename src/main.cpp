#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
    Serial.begin(115200);
    SerialBT.begin("Porygon-X");
    Serial.println("Bluetooth Started! Waiting for connection...");
}

void loop() {
    if (SerialBT.available()) {  // Check if data is received
        String receivedData = SerialBT.readStringUntil('\n');
        receivedData.trim();

        Serial.print("Received: ");
        Serial.println(receivedData);
    }
}
