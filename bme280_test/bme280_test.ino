#include <Wire.h>
#include <bitset>
#include <string>

constexpr uint8_t SDA_PIN{8};
constexpr uint8_t SCL_PIN{9};
constexpr uint8_t bme_address{0x76};
constexpr uint8_t ctrl_meas_register{0xF4};
constexpr uint8_t digT1_register{0x88};
constexpr uint8_t digT2_register{0x8A};
constexpr uint8_t digT3_register{0x8C};

struct Dig {
    uint16_t digT1;
    int16_t digT2;
    int16_t digT3;
};

Dig dig_result;

bool setNormalModeForTemprature(uint8_t reg) {
    
    // set temprature oversampling: set bit#5
    // set pressure oversampling: set bit#2
    // set normal mode: set bits #1, #0
    constexpr uint8_t data = (1u << 5) | (1u << 2) | (1u << 1) | (1u << 0);

    Wire.beginTransmission(bme_address);
    Wire.write(reg);
    Wire.write(data);

    if (auto value = Wire.endTransmission(); value != 0) {
        Serial.printf("Failed to correct end transmission, value: %d\n", value);
        return false;
    }

    return true;
}

bool initDigCoefficient() {
    {
        uint8_t dig[2];
        if(readRegisters(digT1_register, dig, 2)) {
            dig_result.digT1 = (dig[1] << 8) | dig[0];
        } else {
            return false;
        }
    }

    {
        uint8_t dig[2];
        if(readRegisters(digT2_register, dig, 2)) {
            uint16_t raw = (dig[1] << 8) | dig[0];
            dig_result.digT2 = static_cast<int16_t>(raw);
        } else {
            return false;
        }
    }
    
    {
        uint8_t dig[2];
        if(readRegisters(digT3_register, dig, 2)) {
            uint16_t raw = (dig[1] << 8) | dig[0];
            dig_result.digT3 = static_cast<int16_t>(raw);
        } else {
            return false;
        }
    }

    return true;
}

void setup() {
    Serial.begin(115200);

    Wire.begin(SDA_PIN, SCL_PIN);

    if (!setNormalModeForTemprature(ctrl_meas_register)) {
        exit(-1);
    }

    if (!initDigCoefficient()) {
        Serial.printf("Failed initialize dig");
        exit(-1);
    }
}

bool readRegisters(uint8_t reg, uint8_t *data, size_t length) 
{
    Wire.beginTransmission(bme_address);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        Serial.println("Failed to select register");
        delay(3000);
        return false;
    }

    auto received = Wire.requestFrom(bme_address, length);
    if (received != length) {
        Serial.println("Received number of bytes not equalt to length");
        while (Wire.available()) {
            Wire.read();
        }
        return false;
    }

    for (auto i = 0uz; i < length; ++i) {
        data[i] = Wire.read();
    }

    return true;
}

int32_t compensateTemperature(uint32_t adc_T, const Dig& dig)
{
    const int32_t adc = static_cast<int32_t>(adc_T);
    const int32_t dig_T1 = static_cast<int32_t>(dig.digT1);
    const int32_t dig_T2 = static_cast<int32_t>(dig.digT2);
    const int32_t dig_T3 = static_cast<int32_t>(dig.digT3);

    const int32_t var1 =
        (((adc >> 3) - (dig_T1 << 1)) * dig_T2) >> 11;

    const int32_t delta = (adc >> 4) - dig_T1;

    const int32_t var2 =
        (((delta * delta) >> 12) * dig_T3) >> 14;

    const int32_t t_fine = var1 + var2;

    const int32_t temperature = (t_fine * 5 + 128) >> 8;

    Serial.printf("var1=%ld, var2=%ld, t_fine=%ld\n",
    static_cast<long>(var1),
    static_cast<long>(var2),
    static_cast<long>(t_fine));

    return temperature;
}

void loop() {
    uint8_t ctrl_meas;
    if (readRegisters(ctrl_meas_register, &ctrl_meas, 1)) {
        Serial.printf("ctrl_meas: %x\n", ctrl_meas);
    } else {
        Serial.println("I2C read for ctrl_meas failed");
    }

    uint8_t data[3];
    static constexpr uint8_t temperature_register{0xFA};
    auto isSuccess = readRegisters(temperature_register, data, 3);
    if (isSuccess) {
        const uint32_t adc_T =
            (static_cast<uint32_t>(data[0]) << 12) |
            (static_cast<uint32_t>(data[1]) << 4) |
            (static_cast<uint32_t>(data[2]) >> 4);

        auto temperature = compensateTemperature(adc_T, dig_result);

        Serial.printf("Raw temperature: %lu, compensated temperature: %ld.%02ld C\n",
            static_cast<unsigned long>(adc_T),
            static_cast<long>(temperature / 100),
            static_cast<long>(abs(temperature % 100)));


    } else {
        Serial.print("I2C read failed");
        return;
    }

    delay(3000);
}
