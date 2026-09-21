#pragma once

template <uint8_t Pin, uint32_t Freq = 5000, uint8_t Resolution = 8>
class Led {
private:

  uint8_t calculateNightlightDuty(uint16_t adcFiltered) const {
    uint8_t duty = static_cast<uint8_t>(adcFiltered >> 4); 
    return 255 - duty;
  }

public:
  [[nodiscard]] bool begin() {
    return ledcAttach(Pin, Freq, Resolution);
  }

  void setBrightnessFromAdc(uint16_t adcFiltered) {
    ledcWrite(Pin, calculateNightlightDuty(adcFiltered));
  }

  void stop() { 
    ledcWrite(Pin, 0);
  }
};