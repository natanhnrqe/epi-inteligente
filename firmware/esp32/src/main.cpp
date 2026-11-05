
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "actuators/Buzzer.h"
#include "safety/AlertManager.h"
#include "displays/OledDisplay.h"
#include "models/EmergencyProfile.h"
#include "sensors/Mpu6050Sensor.h"

// Buzzer
constexpr int BUZZER_PIN = 23;
Buzzer buzzer(BUZZER_PIN);
AlertManager alerts(buzzer);

// OLED
constexpr int OLED_SDA = 19;
constexpr int OLED_SCL = 18;
OledDisplay oled(Wire);

// Perfil de emergência
EmergencyProfile profile;
int currentPage = 0;

unsigned long lastPageChange = 0;
constexpr unsigned long PAGE_INTERVAL = 3000;

bool emergencyMode = true;
bool oledReady = false;

int simulatedBattery = 100;
unsigned long lastBatteryUpdate = 0;
constexpr unsigned long BATTERY_INTERVAL = 5000;

// Sensor ICM-20689
TwoWire mpuBus(1);

constexpr int MPU_SDA = 21;
constexpr int MPU_SCL = 22;

Mpu6050Sensor mpuSensor(mpuBus);
bool mpuReady = false;

unsigned long lastSensorRead = 0;
constexpr unsigned long SENSOR_INTERVAL = 20;


void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Inicializando sistema...");

    // Inicializa buzzer sem acionar o alarme.
    buzzer.begin();

    // Inicializa OLED.
    oledReady = oled.begin(OLED_SDA, OLED_SCL);

    if (oledReady) {
        Serial.println("OLED inicializado!");
    } else {
        Serial.println("Erro ao inicializar OLED!");
    }

    // Perfil de emergência.
    profile.name = "Joao da Silva";
    profile.blood = "O+";
    profile.contactName = "Maria";
    profile.contactPhone = "41999999999";
    profile.allergies = "Dipirona";
    profile.observations = "Diabetes";

    // Inicializa o sensor.
    Serial.println();
    Serial.println("Inicializando sensor ICM-20689...");

    mpuReady = mpuSensor.begin(MPU_SDA, MPU_SCL);

    if (mpuReady) {
        Serial.println("Sensor inicializado com sucesso!");
        Serial.println("tempo,accX,accY,accZ,gyroX,gyroY,gyroZ,temperatura,magnitude");
    } else {
        Serial.println("Falha ao inicializar sensor!");
    }

    // Exibe primeira página do perfil.
    if (oledReady && emergencyMode) {
        oled.showEmergencyPage(profile, currentPage);
    }

    Serial.println();
    Serial.println("Sistema inicializado!");
}


void loop() {
    alerts.update();

    unsigned long now = millis();

    // Atualização independente do OLED.
    if (oledReady && emergencyMode) {
        if (now - lastPageChange >= PAGE_INTERVAL) {
            lastPageChange = now;

            currentPage++;

            if (currentPage > 3) {
                currentPage = 0;
            }

            oled.showEmergencyPage(profile, currentPage);
        }
    }


    if (mpuReady) {
        if (now - lastSensorRead >= SENSOR_INTERVAL) {
            lastSensorRead = now;

            Mpu6050Data sensorData;

            if (mpuSensor.readData(sensorData)) {

                float magnitude = sqrtf(
                    sensorData.accelerationX * sensorData.accelerationX +
                    sensorData.accelerationY * sensorData.accelerationY +
                    sensorData.accelerationZ * sensorData.accelerationZ
                );

                // Formato CSV para facilitar a análise dos dados.
                Serial.print(now);
                Serial.print(",");

                Serial.print(sensorData.accelerationX);
                Serial.print(",");
                Serial.print(sensorData.accelerationY);
                Serial.print(",");
                Serial.print(sensorData.accelerationZ);
                Serial.print(",");

                Serial.print(sensorData.gyroscopeX);
                Serial.print(",");
                Serial.print(sensorData.gyroscopeY);
                Serial.print(",");
                Serial.print(sensorData.gyroscopeZ);
                Serial.print(",");

                Serial.print(sensorData.temperature);
                Serial.print(",");

                Serial.println(magnitude);
            }
        }
    }

}
