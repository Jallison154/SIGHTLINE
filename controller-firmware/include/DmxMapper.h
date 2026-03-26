#pragma once

#include <Arduino.h>

#include "ControlInput.h"
#include "DmxUniverse.h"
#include "FixtureProfile.h"

class DmxMapper {
 public:
  void apply(const FixtureProfile& profile, const ControlState& control, uint16_t pan16, uint16_t tilt16,
             DmxUniverse& outUniverse);

 private:
  static uint16_t maybeInvert16(uint16_t input, bool invert);
};
