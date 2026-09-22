#pragma once

template <uint8_t Pin, uint32_t Freq = 5000, uint8_t Resolution = 8>
class Led {
private:
  uint8_t _dutty{0};

  void calculateNightlightDuty(uint16_t adcFiltered) {
    uint8_t duty = static_cast<uint8_t>(adcFiltered >> 4); 
    _dutty = 255 - duty;
  }

public:
  [[nodiscard]] bool begin() {
    return ledcAttach(Pin, Freq, Resolution);
  }

  void setBrightnessFromAdc(uint16_t adcFiltered) {
    calculateNightlightDuty(adcFiltered);
    ledcWrite(Pin, getCurrentDutty());
  }

  void stop() { 
    ledcWrite(Pin, 0);
    _dutty = 0;
  }

  [[nodiscard]] uint8_t getCurrentDutty() const {
    return _dutty;
  }
};