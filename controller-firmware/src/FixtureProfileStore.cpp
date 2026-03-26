#include "FixtureProfileStore.h"

bool FixtureProfileStore::begin() {
  // TODO(HW): Initialize filesystem (SPIFFS/LittleFS) if loading external profile files.
  return true;
}

String FixtureProfileStore::loadProfileJsonText(String& outError) {
  // TODO(HW): Replace with selected profile loading from filesystem or persisted config.
  (void)outError;
  return R"json(
{
  "id": "generic-moving-head-16bit-20ch",
  "fixture_name": "Generic Moving Head",
  "manufacturer": "Generic",
  "mode_name": "20ch Extended",
  "channel_count": 20,
  "channels": {
    "pan": { "coarse": 1, "fine": 2, "invert": false },
    "tilt": { "coarse": 3, "fine": 4, "invert": false },
    "dimmer": { "channel": 5 },
    "shutter": { "channel": 6 },
    "zoom": { "channel": 7 },
    "focus": { "channel": 8 },
    "iris": { "channel": 9 },
    "color": { "channel": 10 }
  },
  "defaults": {
    "shutter_open": true,
    "dimmer": 255,
    "deadband": 0.02,
    "smoothing_alpha": 0.25,
    "pan_speed_scale": 1.0,
    "tilt_speed_scale": 1.0
  }
}
)json";
}

bool FixtureProfileStore::loadSelectedProfile(FixtureProfile& outProfile, String& outError) {
  const String json = loadProfileJsonText(outError);
  if (json.isEmpty()) {
    if (outError.isEmpty()) {
      outError = "Selected profile JSON is empty";
    }
    return false;
  }
  return _parser.parse(json, outProfile, outError);
}
