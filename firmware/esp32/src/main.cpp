
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "actuators/Buzzer.h"
#include "safety/AlertManager.h"
#include "displays/OledDisplay.h"
#include "models/EmergencyProfile.h"
#include "sensors/Mpu6050Sensor.h"
#include "safety/FallDetector.h"
#include "network/WebServerManager.h"
#include "models/SystemStatus.h"
#include "storage/EmployeeProfileStorage.h"
#include "logging/SystemLogger.h"

// Buzzer
constexpr int BUZZER_PIN = 23;
Buzzer buzzer(BUZZER_PIN);
AlertManager alerts(buzzer);
FallDetector fallDetector(alerts);

// OLED
constexpr int OLED_SDA = 19;
constexpr int OLED_SCL = 18;
OledDisplay oled(Wire);

// Perfil de emergência
EmergencyProfile profile;
EmployeeProfileStorage profileStorage;

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

WebServerManager webServer(alerts);
bool webServerReady = false;
const char* WIFI_SSID = "Oi_7C96";
const char* WIFI_PASSWORD = "A4vP6PEQ";

SystemStatus systemStatus;

void updateSystemStatus() {

    switch (fallDetector.getState()) {

        case FallState::NORMAL:
            systemStatus.fallState = "NORMAL";
            systemStatus.emergency = false;
            systemStatus.buzzerActive = false;
            break;

        case FallState::WAITING_IMMOBILITY:
            systemStatus.fallState = "WAITING_IMMOBILITY";
            systemStatus.emergency = false;
            systemStatus.buzzerActive = false;
            break;

        case FallState::ALERT:
            systemStatus.fallState = "ALERT";
            systemStatus.emergency = true;
            systemStatus.buzzerActive = true;
            break;
    }

    systemStatus.battery = simulatedBattery;
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Inicializando sistema...");

    buzzer.begin();

    oledReady = oled.begin(OLED_SDA, OLED_SCL);

    if (oledReady) {
        Serial.println("OLED inicializado!");
        oled.showBattery(simulatedBattery);
    } else {
        Serial.println("Erro ao inicializar OLED!");
    }

    Serial.println();
    Serial.println("Inicializando sensor ICM-20689...");

    mpuReady = mpuSensor.begin(MPU_SDA, MPU_SCL);

    if (mpuReady) {
        Serial.println("Sensor inicializado com sucesso!");
        Serial.println(
            "tempo,accX,accY,accZ,gyroX,gyroY,gyroZ,temperatura,magnitude"
        );

        SystemLogger::info("Sensor ICM-20689 inicializado");
    } else {
        Serial.println("Falha ao inicializar sensor!");

    }

    systemStatus.sensorReady = mpuReady;

    Serial.println();
    Serial.println("[WEB] Iniciando servidor...");

    webServerReady = webServer.begin(WIFI_SSID, WIFI_PASSWORD);

    if (!webServerReady) {
        Serial.println("Falha ao inicializar servidor ou LittleFS!");
    } else {
        SystemLogger::info("Servidor e armazenamento inicializados");


        if (profileStorage.load(profile)) {
            SystemLogger::info("Perfil de emergencia carregado");

            Serial.println("Perfil de emergencia carregado!");
        } else {
            SystemLogger::warn(
                "Nenhum perfil encontrado ou erro na leitura"
            );

            Serial.println(
                "Nenhum perfil encontrado ou erro na leitura."
            );
        }
    }


    if (oledReady && emergencyMode) {
        oled.showEmergencyPage(profile, currentPage);
    }

    SystemLogger::info("Sistema inicializado");

    Serial.println();
    Serial.println("Sistema inicializado!");
}


void loop() {

    alerts.update();

    unsigned long now = millis();

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


    if (mpuReady && now - lastSensorRead >= SENSOR_INTERVAL) {
        lastSensorRead = now;

        Mpu6050Data sensorData;

        if (mpuSensor.readData(sensorData)) {
            systemStatus.sensorData = sensorData;
            systemStatus.sensorReady = true;

            fallDetector.update(sensorData);
        }
    }
    updateSystemStatus();

    if (webServerReady) {
        webServer.updateStatus(systemStatus);
        webServer.handleClient();
    }
}



