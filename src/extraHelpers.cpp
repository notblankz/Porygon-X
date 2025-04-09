#include "ExtraHelpers.h"

void customBlink() {
    const int LED_PIN = 2;
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

void mpuCalibratingBlink() {
    const int LED_PIN = 2;

    for (int i = 0; i < 8; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(250);
        digitalWrite(LED_PIN, LOW);
        delay(250);
    }
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
}

void mpuCalibrationCompleteBlink() {
    const int LED_PIN = 2;

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }

    delay(300);

    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(400);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}
