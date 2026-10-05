//
// Created by JoyBoy on 05/10/2026.
//
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
}

void loop() {
    Serial.println("ESP32 funcionando!");
    delay(1000);
}