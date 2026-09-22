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

  void process(std::string_view cmd_view, std::string_view arg_view) {
    Serial.printf(
      "Command: %.*s",
      static_cast<int>(cmd_view.size()),
      cmd_view.data()
    );

    if (!arg_view.empty()) {
      Serial.printf(
        ", argument: %.*s",
        static_cast<int>(arg_view.size()),
        arg_view.data()
      );
    }
    Serial.println();
    Serial.flush();
  }

public:
  void updata() {
    while(Serial.available()) {
      auto sym = static_cast<char>(Serial.read());
      if (std::isspace(sym)) {
        if (sym == '\n') {
          const std::size_t commandLenght = _argBegin > 0 ? _argBegin : _pos;
          const std::size_t argLength = _argBegin > 0 ? _pos - _argBegin : 0;
          process(
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

SerialCommandHandler sch;

void setup() {
  Serial.begin(115200);
}

void loop() {
  sch.updata();
}

