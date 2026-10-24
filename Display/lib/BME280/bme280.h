#include <cstddef>
#include <cstdint>

class Bme {
protected:
    virtual ~Bme() = default;
    virtual bool initTemperatureSensor() = 0;
    virtual bool setup() = 0;
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
        int32_t hundredthsC{0};
        int32_t tFine{0};
    };

    bool readRegisters(uint8_t reg, uint8_t *data, std::size_t length) ;
    bool writeRegister(uint8_t reg, uint8_t value);

    bool initTemperatureCalibration();
    bool readRawTemperature(uint32_t &adcT);
    TemperatureResult compensateTemperature(uint32_t adcT);

    bool initTemperatureSensor() override;

private:
    TemperatureCalibration _calibration;
    TemperatureResult _tempResult;

public:
    explicit BmeTemperature() = default;
    ~BmeTemperature() = default;

    bool setup() override;
    void loop() override;

    int32_t getTemperatureInt() const;
    float getTemperaturetFloat() const;
};
