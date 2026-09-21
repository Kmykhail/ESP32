#pragma once

template <uint8_t Pin>
class ButtonControl {
private:
  ezButton _button{Pin};
  bool _status{false};

public:
  explicit ButtonControl() {
    _button.setDebounceTime(50);  
  }

  void update() {
    _button.loop();
    if (_button.isPressed()) {
      _status = !_status;
      Serial.printf("[BTN] State: %s\n", _status ? "ON" : "OFF");
    }
  }

  [[nodiscard]] bool isOn() const { return _status; }
};
