
#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <ICM20689.h>

struct Mpu6050Data {
    float accelerationX;
    float accelerationY;
    float accelerationZ;

    float gyroscopeX;
    float gyroscopeY;
    float gyroscopeZ;

    float temperature;
};

class Mpu6050Sensor {
private:
    TwoWire& bus;
    ICM20689 imu;

public:
    explicit Mpu6050Sensor(TwoWire& bus);

    bool begin(int sdaPin, int sclPin);

    bool readData(Mpu6050Data& data);
};

#endif
