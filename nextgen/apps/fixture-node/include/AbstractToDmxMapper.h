#pragma once

#include <Arduino.h>

#include "ControlProtocol.h"
#include "FixtureProfile.h"

namespace sightline_v2 {

class AbstractToDmxMapper {
 public:
  void map(const AbstractControls& controls, const FixtureProfile& profile, uint8_t* outUniverse, uint16_t outSize);
};

}  // namespace sightline_v2
