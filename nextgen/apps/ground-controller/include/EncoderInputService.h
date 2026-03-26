#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct EncoderDeltas {
  int32_t panDelta = 0;
  int32_t tiltDelta = 0;
};

class EncoderInputService {
 public:
  bool begin();
  void tick(uint32_t nowMs);
  EncoderDeltas readAndResetDeltas();

 private:
  bool _simulate = true;
  int32_t _simPan = 0;
  int32_t _simTilt = 0;
  EncoderDeltas _pending;
};

}  // namespace sightline_v2
