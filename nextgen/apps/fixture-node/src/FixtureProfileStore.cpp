#include "FixtureProfileStore.h"

#include <LittleFS.h>

namespace sightline_v2 {

bool FixtureProfileStore::begin() {
  // TODO(HW): Configure partition sizing to hold expected profile count.
  return LittleFS.begin(true);
}

String FixtureProfileStore::profilePath(const String& profileId) const { return "/profiles/" + profileId + ".json"; }
String FixtureProfileStore::activeProfileMarkerPath() const { return "/profiles/active_profile.txt"; }

bool FixtureProfileStore::listProfileIds(String& outJsonArray) const {
  // TODO: Implement directory scan and JSON array output.
  outJsonArray = "[]";
  return true;
}

bool FixtureProfileStore::loadProfile(const String& profileId, FixtureProfile& outProfile, String& outError) const {
  File f = LittleFS.open(profilePath(profileId), "r");
  if (!f) {
    outError = "profile not found";
    return false;
  }
  const String json = f.readString();
  f.close();

  if (!_codec.decode(json, outProfile, outError)) return false;
  return _validator.validate(outProfile, outError);
}

bool FixtureProfileStore::saveProfile(const FixtureProfile& profile, String& outError) {
  if (!_validator.validate(profile, outError)) return false;

  String json;
  if (!_codec.encode(profile, json)) {
    outError = "encode failed";
    return false;
  }

  // TODO: Add atomic write pattern (temp + rename) for power-loss resilience.
  File f = LittleFS.open(profilePath(profile.id), "w");
  if (!f) {
    outError = "open write failed";
    return false;
  }
  f.print(json);
  f.close();
  return true;
}

bool FixtureProfileStore::deleteProfile(const String& profileId, String& outError) {
  if (!LittleFS.exists(profilePath(profileId))) {
    outError = "profile not found";
    return false;
  }
  return LittleFS.remove(profilePath(profileId));
}

bool FixtureProfileStore::loadActiveProfileId(String& outProfileId) const {
  File f = LittleFS.open(activeProfileMarkerPath(), "r");
  if (!f) return false;
  outProfileId = f.readString();
  outProfileId.trim();
  f.close();
  return !outProfileId.isEmpty();
}

bool FixtureProfileStore::setActiveProfileId(const String& profileId, String& outError) {
  if (profileId.isEmpty()) {
    outError = "empty profile id";
    return false;
  }
  File f = LittleFS.open(activeProfileMarkerPath(), "w");
  if (!f) {
    outError = "active profile marker write failed";
    return false;
  }
  f.print(profileId);
  f.close();
  return true;
}

}  // namespace sightline_v2
