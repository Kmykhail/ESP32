#include <ezButton.h>

constexpr uint8_t BTN_PIN  = 7;
constexpr uint8_t ADC_PIN  = 8;
constexpr uint8_t LED_PIN  = 47;

class ButtonControl {
private:
  ezButton _button;
  bool _status{false};

public:
  explicit ButtonControl(uint8_t pin) : _button(pin) {
    _button.setDebounceTime(50);  
  }

  void update() {
    _button.loop();
    if (_button.isPressed()) {
      _status = !_status;
      Serial.printf("[BTN] State: %s\n", _status ? "ON" : "OFF");
    }
  }

  bool isOn() const { return _status; }
};

class Ldr {
private:
  const uint8_t _pin;
  static constexpr uint32_t _sampleIntervalMs{10};
  static constexpr float _alpha{0.15f};

  float _filteredValue{0.0f};
  uint32_t _lastSampleMs{0};

public:
  explicit Ldr(uint8_t pin) : _pin(pin) {}

  void begin() {
    _filteredValue = static_cast<float>(analogRead(_pin));
  }

  void update() {
    uint32_t now = millis();
    if (now - _lastSampleMs >= _sampleIntervalMs) {
      _lastSampleMs = now;

      uint16_t rawADC = analogRead(_pin);
      _filteredValue = (_alpha * rawADC) + ((1.0f - _alpha) * _filteredValue);
    }
  }

  uint16_t getValue() const {
    return static_cast<uint16_t>(_filteredValue);
  }

  void reset() {
    _filteredValue = 0.0f;
    _lastSampleMs = 0;
  }
};

class Led {
private:
  const uint8_t _pin;
  static constexpr uint32_t _freq{5000};
  static constexpr uint8_t _resolution{8};

  uint8_t calculateNightlightDuty(uint16_t adcFiltered) const {
    uint8_t duty = static_cast<uint8_t>(adcFiltered >> 4); 
    return 255 - duty;
  }

public:
  explicit Led(uint8_t pin) : _pin(pin) {}

  bool begin() {
    return ledcAttach(_pin, _freq, _resolution);
  }

  void setBrightnessFromAdc(uint16_t adcFiltered) {
    ledcWrite(_pin, calculateNightlightDuty(adcFiltered));
  }

  void stop() { 
    ledcWrite(_pin, 0);
  }
};

ButtonControl btnControl(BTN_PIN);
Ldr ldr(ADC_PIN);
Led led(LED_PIN);

void setup() {
  Serial.begin(115200);

  if (!led.begin()) {
    Serial.println("Initialization error, PWM ERROR!");
    while (true) { delay(1000); }
  }

  ldr.begin();
  Serial.println("READY");
}

void loop() {
  btnControl.update();

  if (btnControl.isOn()) {
    ldr.update();
    led.setBrightnessFromAdc(ldr.getValue());
  } else {
    ldr.reset();
    led.stop();
  }
}