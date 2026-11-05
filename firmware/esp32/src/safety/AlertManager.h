
#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <Arduino.h>
#include "actuators/Buzzer.h"

class Buzzer;

enum class AlertPattern {
    NONE,
    SHORT,
    ATTENTION,
    CRITICAL
};

class AlertManager {
private:
    Buzzer& buzzer;
    AlertPattern currentPattern;

    bool phaseOn;
    bool running;
    int remainingBeeps;
    unsigned long lastChange;

    static constexpr unsigned long BEEP_DURATION = 120;
    static constexpr unsigned long PAUSE_DURATION = 180;

    void startPulsePattern(int quantity);

public:
    explicit AlertManager(Buzzer& buzzer);

    void shortAlert();
    void attentionAlert();
    void criticalAlert();

    void update();
    void stop();
};

#endif
