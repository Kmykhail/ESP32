#include <Wire.h>

#define SDA_PIN 8
#define SCL_PIN 9

namespace  {
    constexpr uint8_t BME_ADDRESS{0x76};
    constexpr uint8_t REG_CTRL_MEAS{0xF4};
    constexpr uint8_t REG_TEMP_DATA{0xF4};
    constexpr uint8_t REG_CALIB_T{0x88};
}

class Bme {
protected:
    virtual ~Bme() = default;
    virtual bool initTemperatureSensor() = 0;
    virtual void setup() = 0;
    virtual void loop() = 0;
};

class BmeTemperature : public Bme {
private:
    struct TemperatureCalibration {
        uint16_t digT1;
        int16_t digT2;
        int16_t digT3;
    };

    struct TemperatureResult {
        int32_t hundredthsC;
        int32_t tFine;
    };

    bool readRegisters(uint8_t reg, uint8_t *data, size_t length) ;
    bool writeRegister(uint8_t reg, uint8_t value);

    bool initTemperatureCalibration();
    bool readRawTemperature(uint32_t &adcT);
    TemperatureResult compensateTemperature(uint32_t adcT);

public:
    explicit BmeTemperature();
    ~BmeTemperature() = default;

    bool initTemperatureSensor() override;
    void setup() override;
    void loop() override;
};
