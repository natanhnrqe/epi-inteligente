
#ifndef FALL_DETECTOR_H
#define FALL_DETECTOR_H

#include <Arduino.h>
#include "sensors/Mpu6050Sensor.h"
#include "safety/AlertManager.h"

class AlertManager;
struct Mpu6050Data;

enum class FallState {
    NORMAL,
    WAITING_IMMOBILITY,
    ALERT
};

class FallDetector {
private:
    AlertManager& alertManager;

    FallState currentState;

    unsigned long immobilityStart;
    bool immobilityTimerStarted;

    float calculateAccelerationMagnitude(const Mpu6050Data& data);
    float calculateGyroscopeMagnitude(const Mpu6050Data& data);

    bool isHighAcceleration(const Mpu6050Data& data);
    bool isImmobile(const Mpu6050Data& data);

public:
    explicit FallDetector(AlertManager& alertManager);

    void update(const Mpu6050Data& data);
    void reset();

    FallState getState() const;
};

#endif
