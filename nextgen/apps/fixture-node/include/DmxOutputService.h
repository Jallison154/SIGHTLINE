#pragma once

#include <Arduino.h>

namespace sightline_v2 {

class DmxOutputService {
 public:
  bool begin();
  void tick(uint32_t nowMs, const uint8_t* universe512);
  uint32_t framesOutput() const { return _framesOutput; }

 private:
  uint32_t _lastTxMs = 0;
  uint32_t _framesOutput = 0;
};

}  // namespace sightline_v2
