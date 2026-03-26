#include "FixtureProfileValidator.h"

namespace sightline_v2 {

namespace {
bool inRange(uint16_t v, uint16_t lo, uint16_t hi) { return v >= lo && v <= hi; }
}

bool FixtureProfileValidator::validate(const FixtureProfile& p, String& outError) const {
  if (p.id.isEmpty() || p.fixtureName.isEmpty() || p.manufacturer.isEmpty() || p.modeName.isEmpty()) {
    outError = "id/fixture_name/manufacturer/mode_name are required";
    return false;
  }
  if (!inRange(p.channelCount, 1, 512)) {
    outError = "channel_count must be 1..512";
    return false;
  }
  if (!inRange(p.pan.coarse, 1, p.channelCount) || !inRange(p.pan.fine, 1, p.channelCount) ||
      !inRange(p.tilt.coarse, 1, p.channelCount) || !inRange(p.tilt.fine, 1, p.channelCount)) {
    outError = "pan/tilt coarse+fine must be within channel_count";
    return false;
  }
  if (p.pan.hasLimits && p.pan.minValue > p.pan.maxValue) {
    outError = "pan limits invalid";
    return false;
  }
  if (p.tilt.hasLimits && p.tilt.minValue > p.tilt.maxValue) {
    outError = "tilt limits invalid";
    return false;
  }
  return true;
}

}  // namespace sightline_v2
