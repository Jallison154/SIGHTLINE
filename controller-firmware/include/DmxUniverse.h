#pragma once

#include <Arduino.h>

class DmxUniverse {
 public:
  static constexpr uint16_t kSize = 512;

  void clear();
  bool setChannel8(uint16_t channel1Based, uint8_t value);
  bool setChannel16(uint16_t coarseChannel1Based, uint16_t value);

  const uint8_t* data() const { return _channels; }
  uint16_t size() const { return kSize; }

 private:
  uint8_t _channels[kSize] = {0};
};
