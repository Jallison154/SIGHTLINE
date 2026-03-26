#include "FixtureProfileCodec.h"

#include <ArduinoJson.h>

namespace sightline_v2 {

bool FixtureProfileCodec::decode(const String& json, FixtureProfile& outProfile, String& outError) const {
  StaticJsonDocument<4096> doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    outError = String("profile JSON parse failed: ") + err.c_str();
    return false;
  }

  outProfile = FixtureProfile{};
  outProfile.id = doc["id"] | "";
  outProfile.fixtureName = doc["fixture_name"] | "";
  outProfile.manufacturer = doc["manufacturer"] | "";
  outProfile.modeName = doc["mode_name"] | "";
  outProfile.channelCount = doc["channel_count"] | 0;

  outProfile.pan.coarse = doc["channels"]["pan"]["coarse"] | 0;
  outProfile.pan.fine = doc["channels"]["pan"]["fine"] | 0;
  outProfile.pan.invert = doc["channels"]["pan"]["invert"] | false;
  outProfile.tilt.coarse = doc["channels"]["tilt"]["coarse"] | 0;
  outProfile.tilt.fine = doc["channels"]["tilt"]["fine"] | 0;
  outProfile.tilt.invert = doc["channels"]["tilt"]["invert"] | false;

  outProfile.intensity.channel = doc["channels"]["intensity"]["channel"] | 0;
  outProfile.iris.channel = doc["channels"]["iris"]["channel"] | 0;
  outProfile.zoom.channel = doc["channels"]["zoom"]["channel"] | 0;
  outProfile.focus.channel = doc["channels"]["focus"]["channel"] | 0;
  outProfile.shutter.channel = doc["channels"]["shutter"]["channel"] | 0;
  outProfile.color.channel = doc["channels"]["color"]["channel"] | 0;

  outProfile.defaultIntensity = doc["defaults"]["intensity"] | 0;
  outProfile.defaultIris = doc["defaults"]["iris"] | 0;
  outProfile.defaultZoom = doc["defaults"]["zoom"] | 0;
  outProfile.defaultFocus = doc["defaults"]["focus"] | 0;
  outProfile.defaultShutter = doc["defaults"]["shutter"] | 0;
  outProfile.defaultColor = doc["defaults"]["color"] | 0;

  return true;
}

bool FixtureProfileCodec::encode(const FixtureProfile& p, String& outJson) const {
  StaticJsonDocument<4096> doc;
  doc["id"] = p.id;
  doc["fixture_name"] = p.fixtureName;
  doc["manufacturer"] = p.manufacturer;
  doc["mode_name"] = p.modeName;
  doc["channel_count"] = p.channelCount;

  JsonObject ch = doc.createNestedObject("channels");
  JsonObject pan = ch.createNestedObject("pan");
  pan["coarse"] = p.pan.coarse;
  pan["fine"] = p.pan.fine;
  pan["invert"] = p.pan.invert;
  JsonObject tilt = ch.createNestedObject("tilt");
  tilt["coarse"] = p.tilt.coarse;
  tilt["fine"] = p.tilt.fine;
  tilt["invert"] = p.tilt.invert;

  if (p.intensity.channel > 0) ch["intensity"]["channel"] = p.intensity.channel;
  if (p.iris.channel > 0) ch["iris"]["channel"] = p.iris.channel;
  if (p.zoom.channel > 0) ch["zoom"]["channel"] = p.zoom.channel;
  if (p.focus.channel > 0) ch["focus"]["channel"] = p.focus.channel;
  if (p.shutter.channel > 0) ch["shutter"]["channel"] = p.shutter.channel;
  if (p.color.channel > 0) ch["color"]["channel"] = p.color.channel;

  JsonObject defs = doc.createNestedObject("defaults");
  defs["intensity"] = p.defaultIntensity;
  defs["iris"] = p.defaultIris;
  defs["zoom"] = p.defaultZoom;
  defs["focus"] = p.defaultFocus;
  defs["shutter"] = p.defaultShutter;
  defs["color"] = p.defaultColor;

  outJson.clear();
  serializeJsonPretty(doc, outJson);
  return !outJson.isEmpty();
}

}  // namespace sightline_v2
