#include <Arduino.h>
#include <Wire.h>

#include "actuators/Buzzer.h"
#include "safety/AlertManager.h"
#include "displays/OledDisplay.h"
#include "models/EmergencyProfile.h"

//Global do Buzzer
constexpr int BUZZER_PIN = 23; // Pino no esp32
Buzzer buzzer(BUZZER_PIN);
AlertManager alerts(buzzer);

// Pino do Oled
constexpr int OLED_SDA = 19;
constexpr int OLED_SCL = 18;

// Variaveis do Profile
EmergencyProfile profile;
int currentPage = 0;
unsigned long lastPageChange = 0;
constexpr unsigned long PAGE_INTERVAL = 3000;
bool emergencyMode = true;


OledDisplay oled(Wire);
bool oledReady = false;
int simulatedBattery = 100;
unsigned long lastBatteryUpdate = 0;
constexpr unsigned long BATTERY_INTERVAL = 5000;

void setup() {
    Serial.begin(115200);

    buzzer.begin();

    oledReady = oled.begin(OLED_SDA, OLED_SCL);

    profile.name = "Joao da Silva";
    profile.blood = "O+";
    profile.contactName = "Maria";
    profile.contactPhone = "41999999999";
    profile.allergies = "Dipirona";
    profile.observations = "Diabetes";

    if (oledReady) {
        Serial.println("OLED inicializado!");

        //oled.showBattery(simulatedBattery);
    } else {
        Serial.println("Erro ao inicializar OLED!");
    }

    if (oledReady && emergencyMode) {
        oled.showEmergencyPage(profile, currentPage);
    }

    Serial.println("Sistema inicializado!");
}

void loop() {
    alerts.update();

    if (!oledReady || !emergencyMode) {
        return;
    }

    unsigned long now = millis();

    if (now - lastPageChange >= PAGE_INTERVAL) {

        lastPageChange = now;

        currentPage++;

        if (currentPage > 3) {
            currentPage = 0;
        }

        oled.showEmergencyPage(profile, currentPage);
    }
}
