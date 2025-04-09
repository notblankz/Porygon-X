#include "../include/updatePIDcontroller.h"
#include "../include/extraHelpers.h"
#include "../include/register.h"
#include <PID_v1.h>

Preferences preferences;
extern PID myPID;

PIDValues readPIDValues() {
    preferences.begin("PID", true);
    PIDValues pid;
    pid.Kp = preferences.getFloat("Kp", 1.0);
    pid.Ki = preferences.getFloat("Ki", 0.01);
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
        myPID.SetTunings(newKp, newKi, newKi);
    } else {
        Serial.println("No change in PID Values required");
    }
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
    } else {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
}
