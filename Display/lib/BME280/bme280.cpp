#include "bme280.h"

#include <Arduino.h>
#include <Wire.h>
#include <cstddef>
#include <cstdint>

#define SDA_PIN 8
#define SCL_PIN 9

namespace  {
    constexpr uint8_t BME_ADDRESS{0x76};
    constexpr uint8_t REG_CTRL_MEAS{0xF4};
    constexpr uint8_t REG_TEMP_DATA{0xFA};
    constexpr uint8_t REG_CALIB_T{0x88};
}

bool BmeTemperature::readRegisters(uint8_t reg, uint8_t *data, std::size_t length)
{
    Wire.beginTransmission(BME_ADDRESS);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        Serial.printf("Failed to select register: %02x\n", reg);
        return false;
    }

    const auto received = Wire.requestFrom(BME_ADDRESS, length);
    if (received != length) {
        while (Wire.available()) {
            Wire.read();
        }

        Serial.println("Failed to read registers");
        return false;
    }

    for (std::size_t i = 0; i < length; ++i) {
        data[i] = static_cast<uint8_t>(Wire.read());
    }

    return true;

}

bool BmeTemperature::writeRegister(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(BME_ADDRESS);
    Wire.write(reg);
    Wire.write(value);

    if (Wire.endTransmission() != 0) {
        Serial.printf("Failed to write register: %02x\n", reg);
        return false;
    }

    return true;
}

bool BmeTemperature::initTemperatureCalibration()
{
    uint8_t data[6];

    if (!readRegisters(REG_CALIB_T, data, sizeof(data))) {
        return false;
    }

#ifdef DEBUG
    Serial.printf(
        "Calibration bytes: %02X %02X %02X %02X %02X %02X\n",
        data[0], data[1], data[2], data[3], data[4], data[5]
    );
#endif

    _calibration.digT1 =
        static_cast<uint16_t>(data[0] | (data[1] << 8));

    _calibration.digT2 =
        static_cast<int16_t>(data[2] | (data[3] << 8));

    _calibration.digT3 =
        static_cast<int16_t>(data[4] | (data[5] << 8));

    return true;
}

bool BmeTemperature::readRawTemperature(uint32_t& adcT)
{
    uint8_t data[3];

    if (!readRegisters(REG_TEMP_DATA, data, sizeof(data))) {
        return false;
    }

#ifdef  DEBUG
    Serial.printf(
        "Temperature bytes: %02X %02X %02X\n",
        data[0], data[1], data[2]
    );
#endif

    adcT = (static_cast<uint32_t>(data[0]) << 12) |
        (static_cast<uint32_t>(data[1]) << 4)  |
        (static_cast<uint32_t>(data[2]) >> 4);

    return true;
}

BmeTemperature::TemperatureResult BmeTemperature::compensateTemperature(uint32_t adcT)
{
    const int32_t adc = static_cast<int32_t>(adcT);
    const int32_t var1 =(((adc >> 3) - (static_cast<int32_t>(_calibration.digT1) << 1))* _calibration.digT2) >> 11;
    const int32_t delta = (adc >> 4) - static_cast<int32_t>(_calibration.digT1);
    const int32_t var2 = (((delta * delta) >> 12) * _calibration.digT3) >> 14;
    const int32_t tFine = var1 + var2;
    const int32_t temperature = (tFine * 5 + 128) >> 8;

    return {
        temperature,
        tFine
    };
}

bool BmeTemperature::initTemperatureSensor()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!initTemperatureCalibration()) {
        Serial.println("Calibration initialization failed");
        return false;
    }

    if (!writeRegister(REG_CTRL_MEAS, 0x27)) {
        Serial.println("BME280 temperature configuration failed");
        return false;
    }

    return true;
}

bool BmeTemperature::setup()
{
    return initTemperatureSensor();
}

void BmeTemperature::loop()
{
    uint32_t adcT;

    if (!readRawTemperature(adcT)) {
        delay(1000);
        return;
    }

    _tempResult = compensateTemperature(adcT);

#ifdef DEBUG
    Serial.printf("Raw: %lu, Temperature: %ld.%02ld C, t_fine: %ld\n",
        static_cast<unsigned long>(adcT),
        static_cast<long>(_tempResult.hundredthsC / 100),
        static_cast<long>(abs(_tempResult.hundredthsC % 100)),
        static_cast<long>(_tempResult.tFine)
    );
#endif
}

int32_t BmeTemperature::getTemperatureInt() const
{
    return _tempResult.hundredthsC;
}

float BmeTemperature::getTemperaturetFloat() const
{
    return _tempResult.hundredthsC / 100.0;
}