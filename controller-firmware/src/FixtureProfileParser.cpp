#include "FixtureProfileParser.h"

#include <ArduinoJson.h>

namespace {
uint16_t readU16(JsonVariantConst v, uint16_t fallback = 0) { return v.is<uint16_t>() ? v.as<uint16_t>() : fallback; }
bool validChannel(uint16_t channel) { return channel >= 1 && channel <= 512; }
}  // namespace

bool FixtureProfileParser::parse(const String& json, FixtureProfile& outProfile, String& outError) {
  StaticJsonDocument<4096> doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    outError = String("Profile parse error: ") + err.c_str();
    return false;
  }

  outProfile = FixtureProfile{};
  outProfile.id = doc["id"] | "";
  outProfile.fixtureName = doc["fixture_name"] | "";
  outProfile.manufacturer = doc["manufacturer"] | "";
  outProfile.modeName = doc["mode_name"] | "";
  outProfile.channelCount = readU16(doc["channel_count"]);

  outProfile.pan.coarse = readU16(doc["channels"]["pan"]["coarse"]);
  outProfile.pan.fine = readU16(doc["channels"]["pan"]["fine"]);
  outProfile.pan.invert = doc["channels"]["pan"]["invert"] | false;
  outProfile.pan.hasLimits = doc["channels"]["pan"]["limits"]["min"].is<uint16_t>() &&
                             doc["channels"]["pan"]["limits"]["max"].is<uint16_t>();
  outProfile.pan.minValue = readU16(doc["channels"]["pan"]["limits"]["min"]);
  outProfile.pan.maxValue = readU16(doc["channels"]["pan"]["limits"]["max"], 65535);

  outProfile.tilt.coarse = readU16(doc["channels"]["tilt"]["coarse"]);
  outProfile.tilt.fine = readU16(doc["channels"]["tilt"]["fine"]);
  outProfile.tilt.invert = doc["channels"]["tilt"]["invert"] | false;
  outProfile.tilt.hasLimits = doc["channels"]["tilt"]["limits"]["min"].is<uint16_t>() &&
                              doc["channels"]["tilt"]["limits"]["max"].is<uint16_t>();
  outProfile.tilt.minValue = readU16(doc["channels"]["tilt"]["limits"]["min"]);
  outProfile.tilt.maxValue = readU16(doc["channels"]["tilt"]["limits"]["max"], 65535);

  outProfile.dimmer.channel = readU16(doc["channels"]["dimmer"]["channel"]);
  outProfile.dimmer.invert = doc["channels"]["dimmer"]["invert"] | false;
  outProfile.dimmer.hasLimits = doc["channels"]["dimmer"]["limits"]["min"].is<uint8_t>() &&
                                doc["channels"]["dimmer"]["limits"]["max"].is<uint8_t>();
  outProfile.dimmer.minValue = doc["channels"]["dimmer"]["limits"]["min"] | 0;
  outProfile.dimmer.maxValue = doc["channels"]["dimmer"]["limits"]["max"] | 255;

  outProfile.shutter.channel = readU16(doc["channels"]["shutter"]["channel"]);
  outProfile.zoom.channel = readU16(doc["channels"]["zoom"]["channel"]);
  outProfile.focus.channel = readU16(doc["channels"]["focus"]["channel"]);
  outProfile.iris.channel = readU16(doc["channels"]["iris"]["channel"]);
  outProfile.color.channel = readU16(doc["channels"]["color"]["channel"]);

  outProfile.defaultShutterOpen = doc["defaults"]["shutter_open"] | true;
  outProfile.defaultDimmer = doc["defaults"]["dimmer"] | 255;
  outProfile.deadband = doc["defaults"]["deadband"] | 0.02f;
  outProfile.smoothingAlpha = doc["defaults"]["smoothing_alpha"] | 0.25f;
  outProfile.panSpeedScale = doc["defaults"]["pan_speed_scale"] | 1.0f;
  outProfile.tiltSpeedScale = doc["defaults"]["tilt_speed_scale"] | 1.0f;

  return validate(outProfile, outError);
}

bool FixtureProfileParser::validate(const FixtureProfile& p, String& outError) {
  if (p.fixtureName.isEmpty() || p.manufacturer.isEmpty() || p.modeName.isEmpty()) {
    outError = "fixture_name, manufacturer, and mode_name are required";
    return false;
  }
  if (p.channelCount == 0 || p.channelCount > 512) {
    outError = "channel_count must be 1..512";
    return false;
  }
  if (!validChannel(p.pan.coarse) || !validChannel(p.pan.fine) || !validChannel(p.tilt.coarse) ||
      !validChannel(p.tilt.fine)) {
    outError = "pan/tilt coarse+fine channels are required and must be 1..512";
    return false;
  }
  if (p.pan.hasLimits && p.pan.minValue > p.pan.maxValue) {
    outError = "pan limits invalid (min > max)";
    return false;
  }
  if (p.tilt.hasLimits && p.tilt.minValue > p.tilt.maxValue) {
    outError = "tilt limits invalid (min > max)";
    return false;
  }
  if (p.dimmer.hasLimits && p.dimmer.minValue > p.dimmer.maxValue) {
    outError = "dimmer limits invalid (min > max)";
    return false;
  }

  const uint16_t channelsToCheck[] = {p.pan.coarse, p.pan.fine, p.tilt.coarse, p.tilt.fine, p.dimmer.channel,
                                      p.shutter.channel, p.zoom.channel, p.focus.channel, p.iris.channel,
                                      p.color.channel};
  for (uint16_t c : channelsToCheck) {
    if (c == 0) continue;  // optional channels
    if (c > p.channelCount) {
      outError = "Mapped channel exceeds channel_count";
      return false;
    }
  }

  return true;
}
