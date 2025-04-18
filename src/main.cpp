// Self-Balancing code and algorithm inspired by H0W3
// Source: https://github.com/H0W3/Self-Balancing-Robot-PID

#include <Arduino.h>
#include "BluetoothSerial.h"
#include "WiFi.h"
#include "../lib/secrets.h"
#include <ESPAsyncWebServer.h>
#include "UpdatePIDController.h"
#include "Register.h"
#include "ExtraHelpers.h"
#include <PID_v1.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <Preferences.h>

// Stepper Motor pins
#define DIR_PIN 27
#define STEP_PIN 14
#define DIR_PIN_2 25
#define STEP_PIN_2 26
#define RESET 13
#define SLEEP 12

#define MS1 5
#define MS2 18
#define MS3 19

// MPU6050 Object
MPU6050 mpu(Wire);

// PID Variables
double setpoint = 0;
double input, output;

// Define PID Controller Object in Global Scope
PID myPID(&input, &output, &setpoint, 0.0, 0.0, 0.0, DIRECT);

// Timing Variables
unsigned long lastPIDUpdate = 0;
const unsigned long PIDInterval = 10;

// Stepper variables
int motorSpeed = 0;
unsigned long lastStepTime1 = 0;
unsigned long lastStepTime2 = 0;

// Task handle variable - reference pointer to PID function in core 0
TaskHandle_t pidTaskHandle;

// Function prototypes
void pidLoop(void* parameter);
void motorControl();

const int LED_PIN = 2;
String btInputBuffer = "";
BluetoothSerial SerialBT;
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // Get stored PID Gain control variables
    PIDValues pid = readPIDValues();
    Serial.printf("Boot PID: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", pid.Kp, pid.Ki, pid.Kd);

    pinMode(LED_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN_2, OUTPUT);
    pinMode(STEP_PIN_2, OUTPUT);
    pinMode(RESET, OUTPUT);
    pinMode(SLEEP, OUTPUT);
    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(MS3, OUTPUT);

    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);
    digitalWrite(MS3, HIGH);

    digitalWrite(LED_PIN, HIGH);
    delay(10000);
    digitalWrite(LED_PIN, LOW);

    // ---- Enable the stepper motors ----
    digitalWrite(RESET, HIGH);
    digitalWrite(SLEEP, HIGH);

    // ---- MPU6050 Stuff ----
    Wire.begin(21, 22);
    Wire.setClock(400000);
    byte status = mpu.begin();
    while (status != 0) {
        Serial.println("MPU6050 connection failed");
        status = mpu.begin();
    }
    Serial.println("MPU6050 Initialized successfully");

    mpuCalibratingBlink();

    // Calibrate MPU6050
    mpu.calcOffsets(true, true);

    mpuCalibrationCompleteBlink();

    // ---- PID Controller Stuff ----
    myPID.SetMode(AUTOMATIC);
    myPID.SetOutputLimits(-12000, 12000);
    myPID.SetTunings(pid.Kp, pid.Ki, pid.Kd);
    myPID.SetSampleTime(10);

    // --- Pin pidLoop() to always run on core 0
    xTaskCreatePinnedToCore(pidLoop, "PID Task", 10000, NULL, 2, &pidTaskHandle, 0);
    delay(4000);

    // ---- Connect to WiFi and register ESP32 to backend
    connectWiFi();
    registerESP();
    server.on("/updatePID", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL, handleUpdatePID);
    server.begin();

    // ---- Start and connect to bluetooh remote ----
    Serial.println("\nBluetooth Started! Waiting for connection...");
    SerialBT.begin("Porygon-X");
}

void loop() {
    // if (SerialBT.available()) {
    //     String receivedData = SerialBT.readStringUntil('\n');
    //     receivedData.trim();
    //     Serial.println(receivedData);
    // }

    while (SerialBT.available()) {
        char c = SerialBT.read();

        if (c == '\n') {
            int commaIdx = btInputBuffer.indexOf(",");
            if (commaIdx == -1)
                continue;
            int x = btInputBuffer.substring(0, commaIdx).toInt();
            int y = btInputBuffer.substring(commaIdx + 1).toInt();
            Serial.printf("Joystick: x=%d :: y=%d\n", x, y);
            btInputBuffer = "";
        }
        else {
            btInputBuffer += c;
        }
    }

    motorControl();
}

void pidLoop(void* parameter) {
    while (true) {
        unsigned long currentMillis = millis();
        if (currentMillis - lastPIDUpdate >= PIDInterval) {
            lastPIDUpdate = currentMillis;

            // ---- Update MPU6050 angle ----
            mpu.update();
            delay(1);
            double angle = mpu.getAngleY();

            // ---- Smoothening the value, since MPU6050 gives a shaky reading ----
            static double smoothedAngle = 0.0;
            smoothedAngle += 0.3 * (angle - smoothedAngle);

            // ---- Update the input with the new angle and compute the PID output ----
            input = smoothedAngle;
            myPID.Compute();

            // ---- Case the float output into int ----
            if (abs(smoothedAngle) < 0.4) {
                output = 0;
                motorSpeed = 0;
                Serial.println("Angle within deadzone, no PID compute");
                continue;
            }

            // --- Apply a low-pass filter to the output in order to avoid jitters on low angles ----
            static int prevSpeed = 0;
            // !--- If Angle of bot is >= 30 degrees, then use the entire output values since we need higher motor speeds ---!
            double alpha = constrain((abs(smoothedAngle) / 30.0), 0.2, 1.0);
            motorSpeed = prevSpeed + (output - prevSpeed) * alpha;
            prevSpeed = motorSpeed;

            // ---- Set directions for the motor; if motorSpeed == +ive -> move forward else backword ----
            if (motorSpeed > 0) {
                digitalWrite(DIR_PIN, HIGH);
                digitalWrite(DIR_PIN_2, HIGH);
            }
            else if (motorSpeed < 0) {
                digitalWrite(DIR_PIN, LOW);
                digitalWrite(DIR_PIN_2, LOW);
            }

            // ---- Debug prints ----
            Serial.print("P: ");
            Serial.print(myPID.GetKp());
            Serial.print(" I: ");
            Serial.print(myPID.GetKi());
            Serial.print(" D: ");
            Serial.print(myPID.GetKd());
            Serial.print(" Angle: ");
            Serial.print(smoothedAngle);
            Serial.print(" PID Computed Output: ");
            Serial.print(output);
            Serial.print(" Motor Speed: ");
            Serial.println(motorSpeed);
        }

        delay(1);
    }
}

void motorControl() {
    // ---- Control the motor pulses ----
    if (abs(motorSpeed) > 0) {
        // ---- Calculate the required interval between each pulse ----
        unsigned long stepInterval1 = 1000000 / abs(motorSpeed);
        if (micros() - lastStepTime1 >= stepInterval1) {
            lastStepTime1 = micros();
            digitalWrite(STEP_PIN, HIGH);
            delayMicroseconds(2); // default pulse width according to A4988 Datasheets
            digitalWrite(STEP_PIN, LOW);
        }
    }

    if (abs(motorSpeed) > 0) {
        // ---- Calculate the required interval between each pulse ----
        unsigned long stepInterval2 = 1000000 / abs(motorSpeed);
        if (micros() - lastStepTime2 >= stepInterval2) {
            lastStepTime2 = micros();
            digitalWrite(STEP_PIN_2, HIGH);
            delayMicroseconds(2); // default pulse width according to A4988 Datasheets
            digitalWrite(STEP_PIN_2, LOW);
        }
    }
}
