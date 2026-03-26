#pragma once

#include <Arduino.h>

#include "FixtureProfileParser.h"
#include "FixtureProfile.h"

class FixtureProfileStore {
 public:
  bool begin();
  bool loadSelectedProfile(FixtureProfile& outProfile, String& outError);

 private:
  String loadProfileJsonText(String& outError);

  FixtureProfileParser _parser;
};
