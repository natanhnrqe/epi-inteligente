#include <Arduino.h>
#include "Buzzer.h"

Buzzer::Buzzer(int pin) {
    this->pin = pin;
    this->active = false;
}

void Buzzer::begin() {
    bool configured = ledcAttach(pin, FREQUENCY, RESOLUTION);

    if (!configured) {
        Serial.println("Erro ao configurar PWM do buzzer!");
        return;
    }

    ledcWrite(pin, 0);

    Serial.println("Buzzer configurado!");
}

void Buzzer::activate() {
    ledcWrite(pin, DUTY_CYCLE);
    active = true;
}

void Buzzer::deactivate() {
    ledcWrite(pin, 0);
    active = false;
}

bool Buzzer::isActive() const {
    return active;
}


