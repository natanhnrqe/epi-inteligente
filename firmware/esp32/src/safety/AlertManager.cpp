
#include "AlertManager.h"

AlertManager::AlertManager(Buzzer& buzzer)
    : buzzer(buzzer),
      currentPattern(AlertPattern::NONE),
      phaseOn(false),
      running(false),
      remainingBeeps(0),
      lastChange(0) {
}

void AlertManager::startPulsePattern(int quantity) {
    stop();

    currentPattern = AlertPattern::ATTENTION;
    remainingBeeps = quantity;

    buzzer.activate();

    phaseOn = true;
    running = true;

    lastChange = millis();
}

void AlertManager::shortAlert() {
    currentPattern = AlertPattern::SHORT;
    startPulsePattern(1);
}

void AlertManager::attentionAlert() {
    currentPattern = AlertPattern::ATTENTION;
    startPulsePattern(3);
}

void AlertManager::criticalAlert() {
    stop();

    currentPattern = AlertPattern::CRITICAL;
    buzzer.activate();

    running = true;
}

void AlertManager::update() {
    if (!running || currentPattern == AlertPattern::CRITICAL) {
        return;
    }

    unsigned long now = millis();

    if (phaseOn && now - lastChange >= BEEP_DURATION) {
        buzzer.deactivate();

        phaseOn = false;
        lastChange = now;

        remainingBeeps--;

        if (remainingBeeps <= 0) {
            running = false;
            currentPattern = AlertPattern::NONE;
        }

        return;
    }

    if (!phaseOn && now - lastChange >= PAUSE_DURATION) {
        buzzer.activate();

        phaseOn = true;
        lastChange = now;
    }
}

void AlertManager::stop() {
    buzzer.deactivate();

    currentPattern = AlertPattern::NONE;
    phaseOn = false;
    running = false;

    remainingBeeps = 0;
}
