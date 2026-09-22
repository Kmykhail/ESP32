#pragma once

#include <concepts>

template <typename T>
concept AnalogFilter = requires(T f, uint16_t sample) {
  {f.update(sample)} -> std::same_as<uint16_t>;
  {f.getValue()} -> std::same_as<uint16_t>;
  {f.reset()} -> std::same_as<void>;
};

template <uint8_t AlphaPercent = 15>
class EmaFilter {
private:
  static constexpr float _alpha = static_cast<float>(AlphaPercent) / 100.0f;
  float _filtered{0.0f};
  bool _initialized{false};

public:
  uint16_t update(uint16_t sample) {
    if (!_initialized) [[unlikely]] {
      _filtered = static_cast<float>(sample);
      _initialized = true;
    } else {
      _filtered = (_alpha * sample) + ((1.0f - _alpha) * _filtered);
    }

    return getValue();
  }

  [[nodiscard]] uint16_t getValue() const {
    return static_cast<uint16_t>(_filtered);
  }

  void reset() { _filtered = false; }
};

template <uint8_t Pin, AnalogFilter Filter = EmaFilter<15>>
class Ldr {
private:
  static constexpr uint32_t _sampleIntervalMs{10};
  Filter _filter;
    uint16_t _rawValue{0};
  uint32_t _lastSampleMs{0};

public:
  void update() {
    uint32_t now = millis();
    if (now - _lastSampleMs >= _sampleIntervalMs) {
      _rawValue = analogRead(Pin);
      _filter.update(_rawValue);
      _lastSampleMs = now;
    }
  }

  [[nodiscard]] uint16_t getFiltered() const {
    return _filter.getValue();
  }

  [[nodiscard]] uint16_t getRaw() const {
    return _rawValue;
  }

  void reset() {
    _filter.reset();
    _rawValue = 0;
    _lastSampleMs = 0;
  }
};