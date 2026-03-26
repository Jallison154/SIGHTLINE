#pragma once

#include <Arduino.h>

#include "FixtureProfile.h"

namespace sightline_v2 {

class FixtureProfileCodec {
 public:
  bool decode(const String& json, FixtureProfile& outProfile, String& outError) const;
  bool encode(const FixtureProfile& profile, String& outJson) const;
};

}  // namespace sightline_v2
