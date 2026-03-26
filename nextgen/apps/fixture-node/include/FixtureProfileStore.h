#pragma once

#include <Arduino.h>

#include "FixtureProfileCodec.h"
#include "FixtureProfileValidator.h"

namespace sightline_v2 {

class FixtureProfileStore {
 public:
  bool begin();
  bool listProfileIds(String& outJsonArray) const;
  bool loadProfile(const String& profileId, FixtureProfile& outProfile, String& outError) const;
  bool saveProfile(const FixtureProfile& profile, String& outError);
  bool deleteProfile(const String& profileId, String& outError);

  bool loadActiveProfileId(String& outProfileId) const;
  bool setActiveProfileId(const String& profileId, String& outError);

 private:
  String profilePath(const String& profileId) const;
  String activeProfileMarkerPath() const;

  FixtureProfileCodec _codec;
  FixtureProfileValidator _validator;
};

}  // namespace sightline_v2
