#include <Arduino.h>
#include <math.h>
#include <unity.h>
#include <Wire.h>

#include "actuators/Buzzer.h"
#include "displays/OledDisplay.h"
#include "models/EmergencyProfile.h"
#include "safety/AlertManager.h"
#include "safety/FallDetector.h"
#include "sensors/Mpu6050Sensor.h"

constexpr int MPU_SDA = 21;
constexpr int MPU_SCL = 22;
constexpr int OLED_SDA = 19;
constexpr int OLED_SCL = 18;
constexpr int BUZZER_PIN = 23;

Buzzer* buzzer;
AlertManager* alertManager;
FallDetector* detector;
Mpu6050Sensor* sensor;
OledDisplay* display;
bool buzzerInitialized = false;

void setUp(void) {
    buzzer = new Buzzer(BUZZER_PIN);
    alertManager = new AlertManager(*buzzer);
    detector = new FallDetector(*alertManager);
    sensor = new Mpu6050Sensor(Wire);
    display = new OledDisplay(Wire);

    if (!buzzerInitialized) {
        buzzer->begin();
        buzzerInitialized = true;
    }
}

void tearDown(void) {
    alertManager->stop();
    delete detector;
    delete alertManager;
    delete buzzer;
    delete sensor;
    delete display;
}

void test_sensor_reads_finite_values() {
    TEST_ASSERT_TRUE_MESSAGE(
        sensor->begin(MPU_SDA, MPU_SCL),
        "Sensor ICM-20689 nao inicializou"
    );

    Mpu6050Data data{};
    TEST_ASSERT_TRUE_MESSAGE(
        sensor->readData(data),
        "Falha ao ler dados do sensor"
    );

    TEST_ASSERT_TRUE(isfinite(data.accelerationX));
    TEST_ASSERT_TRUE(isfinite(data.accelerationY));
    TEST_ASSERT_TRUE(isfinite(data.accelerationZ));
    TEST_ASSERT_TRUE(isfinite(data.gyroscopeX));
    TEST_ASSERT_TRUE(isfinite(data.gyroscopeY));
    TEST_ASSERT_TRUE(isfinite(data.gyroscopeZ));
    TEST_ASSERT_TRUE(isfinite(data.temperature));
}

void test_buzzer_can_be_activated_and_stopped() {
    TEST_ASSERT_FALSE(buzzer->isActive());

    buzzer->activate();
    TEST_ASSERT_TRUE(buzzer->isActive());

    buzzer->deactivate();
    TEST_ASSERT_FALSE(buzzer->isActive());
}

void test_fall_detector_ignores_acceleration_below_threshold() {
    Mpu6050Data data{};
    data.accelerationX = 19.99f;

    detector->update(data);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::NORMAL),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_FALSE(buzzer->isActive());
}

void test_fall_detector_alerts_after_five_seconds_of_immobility() {
    Mpu6050Data impactData{};
    impactData.accelerationX = 20.0f;

    Mpu6050Data immobileData{};
    immobileData.accelerationZ = 9.8f;

    detector->update(impactData);
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::WAITING_IMMOBILITY),
        static_cast<int>(detector->getState())
    );

    detector->update(immobileData);
    delay(4000);
    detector->update(immobileData);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::WAITING_IMMOBILITY),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_FALSE(buzzer->isActive());

    delay(1100);
    detector->update(immobileData);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::ALERT),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_TRUE(buzzer->isActive());

    detector->reset();

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::NORMAL),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_FALSE(buzzer->isActive());
}

void test_fall_detector_restarts_immobility_after_movement() {
    Mpu6050Data impactData{};
    impactData.accelerationX = 20.0f;

    Mpu6050Data immobileData{};
    immobileData.accelerationZ = 9.8f;

    Mpu6050Data movementData{};
    movementData.accelerationZ = 12.0f;

    detector->update(impactData);
    detector->update(immobileData);
    delay(3000);
    detector->update(movementData);

    delay(3000);
    detector->update(immobileData);
    delay(4000);
    detector->update(immobileData);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::WAITING_IMMOBILITY),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_FALSE(buzzer->isActive());

    delay(1100);
    detector->update(immobileData);

    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(FallState::ALERT),
        static_cast<int>(detector->getState())
    );
    TEST_ASSERT_TRUE(buzzer->isActive());
}

void test_display_initializes_and_renders_all_pages() {
    TEST_ASSERT_TRUE_MESSAGE(
        display->begin(OLED_SDA, OLED_SCL),
        "Display OLED nao inicializou"
    );

    EmergencyProfile profile;
    profile.name = "Fulano de Tal";
    profile.contactName = "Ciclano";
    profile.contactPhone = "41999999999";
    profile.blood = "O+";
    profile.allergies = "Nenhuma";
    profile.observations = "Nenhuma";

    display->showBattery(0);
    display->showBattery(100);

    for (int page = 0; page < 4; ++page) {
        display->showEmergencyPage(profile, page);
    }
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_sensor_reads_finite_values);
    RUN_TEST(test_buzzer_can_be_activated_and_stopped);
    RUN_TEST(test_fall_detector_ignores_acceleration_below_threshold);
    RUN_TEST(test_fall_detector_alerts_after_five_seconds_of_immobility);
    RUN_TEST(test_fall_detector_restarts_immobility_after_movement);
    RUN_TEST(test_display_initializes_and_renders_all_pages);

    UNITY_END();
}

void loop() {}
