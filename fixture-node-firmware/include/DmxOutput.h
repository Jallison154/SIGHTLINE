#pragma once

#include <Arduino.h>

#include "DmxBuffer.h"

class DmxOutput {
 public:
  bool begin();
  void tick(uint32_t nowMs, const DmxBuffer& source);

  uint32_t framesOutput() const { return _framesOutput; }

 private:
  uint32_t _lastOutputMs = 0;
  uint32_t _framesOutput = 0;
};
