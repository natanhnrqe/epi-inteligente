#include <Arduino.h>
#include "actuators/Buzzer.h"

constexpr int BUZZER_PIN = 23;

Buzzer buzzer(BUZZER_PIN);

void setup() {
    Serial.begin(115200);
    buzzer.begin();

    Serial.println("Sistema inicializado!");
}

void loop() {
    buzzer.activate();
}