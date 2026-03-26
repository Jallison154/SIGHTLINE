#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct ButtonState {
  bool shutterOpen = true;
  uint8_t color = 0;
};

class ButtonInputService {
 public:
  bool begin();
  void tick(uint32_t nowMs);
  ButtonState current() const { return _state; }

 private:
  ButtonState _state;
};

}  // namespace sightline_v2
