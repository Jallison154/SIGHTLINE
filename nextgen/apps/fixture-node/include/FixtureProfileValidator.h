#pragma once

#include <Arduino.h>

#include "FixtureProfile.h"

namespace sightline_v2 {

class FixtureProfileValidator {
 public:
  bool validate(const FixtureProfile& profile, String& outError) const;
};

}  // namespace sightline_v2
