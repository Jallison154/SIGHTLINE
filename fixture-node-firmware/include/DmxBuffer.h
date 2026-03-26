#pragma once

#include <Arduino.h>

class DmxBuffer {
 public:
  static constexpr uint16_t kUniverseSize = 512;

  void clear();
  bool applyPacketData(const uint8_t* data, uint16_t length);
  const uint8_t* data() const { return _channels; }

 private:
  uint8_t _channels[kUniverseSize] = {0};
};
