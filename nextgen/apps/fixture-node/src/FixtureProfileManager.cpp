#include "FixtureProfileManager.h"

namespace sightline_v2 {

bool FixtureProfileManager::begin() {
  if (!_store.begin()) return false;

  String activeId;
  if (_store.loadActiveProfileId(activeId)) {
    String err;
    if (activateProfile(activeId, err)) return true;
  }

  // TODO: Choose first available profile if active marker missing/invalid.
  _hasActive = false;
  _activeId = "";
  return true;
}

bool FixtureProfileManager::activateProfile(const String& profileId, String& outError) {
  FixtureProfile next;
  if (!_store.loadProfile(profileId, next, outError)) {
    return false;
  }

  // Safe runtime switch model:
  // 1) Validate and parse into temporary profile
  // 2) Swap active profile only after success
  _active = next;
  _activeId = profileId;
  _hasActive = true;

  _store.setActiveProfileId(profileId, outError);
  return true;
}

}  // namespace sightline_v2
