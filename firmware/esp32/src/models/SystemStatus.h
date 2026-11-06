#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include "../sensors/Mpu6050Sensor.h"

struct SystemStatus {
    Mpu6050Data sensorData{};

    bool sensorReady = false;
    bool buzzerActive = false;

    int battery = 100;

    String fallState = "NORMAL";
    bool emergency = false;
};

#endif