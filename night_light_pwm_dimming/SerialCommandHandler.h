#pragma once

#include <array>
#include <cctype>
#include <string_view>

class SerialCommandHandler {
private:
  static constexpr std::size_t BUF_SIZE {64};
  std::array<char, BUF_SIZE> _buffer;
  std::size_t _pos{0};
  std::size_t _argBegin{0};

  void clear() {
    _pos = 0;
    _argBegin = 0;
  }

  void process(
    auto &btn,
    auto &led,
    auto &ldr,
    std::string_view cmd_view,
    std::string_view arg_view
  ) {
    if (cmd_view == "STATUS") {      
      Serial.println("---STATUS---");
      Serial.printf("Button: %s\n", btn.isOn() ? "ON" : "OFF");
      Serial.printf("LED Duty: %i\n", led.getCurrentDutty());
      Serial.printf("LDR Raw/Filtered: %d / %d\n", ldr.getRaw(), ldr.getFiltered());
      Serial.println("------------");
    }

    Serial.flush();
  }

public:
  void updata(auto &btn, auto &led, auto &ldr) {
    while(Serial.available()) {
      auto sym = static_cast<char>(Serial.read());
      if (std::isspace(sym)) {
        if (sym == '\n') {
          const std::size_t commandLenght = _argBegin > 0 ? _argBegin : _pos;
          const std::size_t argLength = _argBegin > 0 ? _pos - _argBegin : 0;
          process(
            btn, led, ldr,
            std::string_view{_buffer.data(), commandLenght},
            std::string_view{_buffer.data() + _argBegin, argLength}
          );
          clear();
        } else if (!_argBegin) {
          _argBegin = _pos;
        }
      } else {
        if (_pos < BUF_SIZE) {
          _buffer[_pos++] = sym;
        } else {
          Serial.println("[ERROR] Command buffer overflow");
          clear();
        }
      }
    }
  }
};