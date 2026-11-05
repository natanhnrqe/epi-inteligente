
#include <Arduino.h>

#include "actuators/Buzzer.h"
#include "safety/AlertManager.h"

constexpr int BUZZER_PIN = 23;

Buzzer buzzer(BUZZER_PIN);
AlertManager alerts(buzzer);

void setup() {
    Serial.begin(115200);

    buzzer.begin();

    Serial.println("Sistema inicializado!");
}

void loop() {
    alerts.update();
}
