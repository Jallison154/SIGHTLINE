#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct FaderValues {
  uint8_t intensity = 0;
  uint8_t iris = 0;
  uint8_t zoom = 0;
  uint8_t focus = 0;
};

class FaderInputService {
 public:
  bool begin();
  void tick(uint32_t nowMs);
  FaderValues current() const { return _values; }

 private:
  FaderValues _values;
};

}  // namespace sightline_v2
