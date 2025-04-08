#ifndef UPDATE_PID_CONTROLLER_H
#define UPDATE_PID_CONTROLLER_H

#include <Arduino.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

struct PIDValues {
    float Kp;
    float Ki;
    float Kd;
};

PIDValues readPIDValues();
void savePIDValues(float newKp, float newKi, float newKd);
void updatePIDValues(float newKp, float newKi, float newKd);
void handleUpdatePID(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

#endif
