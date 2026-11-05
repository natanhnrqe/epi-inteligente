
#include "FallDetector.h"
#include <math.h>

// Limites experimentais para os testes.
static constexpr float ACCELERATION_THRESHOLD = 20.0f;

static constexpr float GYROSCOPE_STILL_THRESHOLD = 0.15f;

static constexpr float MIN_STATIC_ACCELERATION = 8.8f;
static constexpr float MAX_STATIC_ACCELERATION = 10.8f;

static constexpr unsigned long IMMOBILITY_DURATION = 5000;

FallDetector::FallDetector(AlertManager& alertManager)
    : alertManager(alertManager),
      currentState(FallState::NORMAL),
      immobilityStart(0),
      immobilityTimerStarted(false) {
}

float FallDetector::calculateAccelerationMagnitude(
    const Mpu6050Data& data
) {
    return sqrtf(
        data.accelerationX * data.accelerationX +
        data.accelerationY * data.accelerationY +
        data.accelerationZ * data.accelerationZ
    );
}

float FallDetector::calculateGyroscopeMagnitude(
    const Mpu6050Data& data
) {
    return sqrtf(
        data.gyroscopeX * data.gyroscopeX +
        data.gyroscopeY * data.gyroscopeY +
        data.gyroscopeZ * data.gyroscopeZ
    );
}

bool FallDetector::isHighAcceleration(const Mpu6050Data& data) {
    float acceleration = calculateAccelerationMagnitude(data);

    return acceleration >= ACCELERATION_THRESHOLD;
}

bool FallDetector::isImmobile(const Mpu6050Data& data) {
    float acceleration = calculateAccelerationMagnitude(data);
    float gyroscope = calculateGyroscopeMagnitude(data);

    bool accelerationStable =
        acceleration >= MIN_STATIC_ACCELERATION &&
        acceleration <= MAX_STATIC_ACCELERATION;

    bool rotationStable =
        gyroscope <= GYROSCOPE_STILL_THRESHOLD;

    return accelerationStable && rotationStable;
}

void FallDetector::update(const Mpu6050Data& data) {

    switch (currentState) {

        case FallState::NORMAL:

            if (isHighAcceleration(data)) {
                currentState = FallState::WAITING_IMMOBILITY;

                immobilityTimerStarted = false;

                Serial.println("Aceleracao elevada detectada!");
            }

            break;

        case FallState::WAITING_IMMOBILITY:

            if (isImmobile(data)) {

                if (!immobilityTimerStarted) {
                    immobilityStart = millis();
                    immobilityTimerStarted = true;

                    Serial.println("Imobilidade detectada. Aguardando...");
                }

                if (millis() - immobilityStart >= IMMOBILITY_DURATION) {

                    currentState = FallState::ALERT;

                    Serial.println("Possivel queda confirmada!");

                    alertManager.criticalAlert();
                }

            } else {
                // Qualquer movimento reinicia a contagem.
                immobilityTimerStarted = false;
            }

            break;

        case FallState::ALERT:

            // Aguarda um reset externo para rearmar o detector.
            break;
    }
}

void FallDetector::reset() {
    currentState = FallState::NORMAL;

    immobilityStart = 0;
    immobilityTimerStarted = false;

    alertManager.stop();

    Serial.println("Detector de queda reiniciado.");
}

FallState FallDetector::getState() const {
    return currentState;
}
