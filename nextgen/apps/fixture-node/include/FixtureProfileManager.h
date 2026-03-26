#pragma once

#include <Arduino.h>

#include "FixtureProfileStore.h"

namespace sightline_v2 {

class FixtureProfileManager {
 public:
  bool begin();
  bool activateProfile(const String& profileId, String& outError);
  const FixtureProfile& activeProfile() const { return _active; }
  bool hasActiveProfile() const { return _hasActive; }
  const String& activeProfileId() const { return _activeId; }

  FixtureProfileStore& store() { return _store; }

 private:
  bool _hasActive = false;
  String _activeId;
  FixtureProfile _active;
  FixtureProfileStore _store;
};

}  // namespace sightline_v2
