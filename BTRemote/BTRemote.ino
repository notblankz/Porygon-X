#include <SoftwareSerial.h>

SoftwareSerial BTSerial(10, 11);

int VRx = A0;
int VRy = A1;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
}

void loop() {
  int xValue = analogRead(VRx);
  int yValue = analogRead(VRy);

  BTSerial.print(xValue);
  BTSerial.print(",");
  BTSerial.print(yValue);
  BTSerial.println();

  Serial.print(xValue);
  Serial.print(",");
  Serial.print(yValue);
  Serial.println();

  delay(50);
}

// // AT  # Should respond "OK"
// // AT+ROLE=1  # Set HC-05 as master
// // AT+BIND=XX,XX,XX,XX,XX,XX  # Replace with ESP32 MAC Address
// // AT+CMODE=0  # Connect only to bound device
// // AT+RESET  # Restart HC-05