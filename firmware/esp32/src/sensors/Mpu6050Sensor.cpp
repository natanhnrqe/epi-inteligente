
#include "Mpu6050Sensor.h"

Mpu6050Sensor::Mpu6050Sensor(TwoWire& bus)
    : bus(bus), imu(bus, 0x68) {
}

bool Mpu6050Sensor::begin(int sdaPin, int sclPin) {
    // Inicializa o barramento I2C nos pinos configurados.
    bus.begin(sdaPin, sclPin);

    int status = imu.begin();

    if (status < 0) {
        Serial.print("Erro ao inicializar ICM20689. Codigo: ");
        Serial.println(status);
        return false;
    }

    // Configuração do acelerômetro: +/- 8g.
    imu.setAccelRange(ICM20689::ACCEL_RANGE_8G);

    // Configuração do giroscópio: +/- 500 graus/s.
    imu.setGyroRange(ICM20689::GYRO_RANGE_500DPS);

    // Filtro passa-baixa: 21 Hz.
    imu.setDlpfBandwidth(ICM20689::DLPF_BANDWIDTH_21HZ);

    Serial.println("ICM20689 inicializado!");

    return true;
}

bool Mpu6050Sensor::readData(Mpu6050Data& data) {
    int status = imu.readSensor();

    if (status < 0) {
        return false;
    }

    data.accelerationX = imu.getAccelX_mss();
    data.accelerationY = imu.getAccelY_mss();
    data.accelerationZ = imu.getAccelZ_mss();

    data.gyroscopeX = imu.getGyroX_rads();
    data.gyroscopeY = imu.getGyroY_rads();
    data.gyroscopeZ = imu.getGyroZ_rads();

    data.temperature = imu.getTemperature_C();

    return true;
}
