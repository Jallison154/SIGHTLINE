#pragma once

#include <Arduino.h>

#include "FixtureProfile.h"

class FixtureProfileParser {
 public:
  bool parse(const String& json, FixtureProfile& outProfile, String& outError);

 private:
  bool validate(const FixtureProfile& profile, String& outError);
};
