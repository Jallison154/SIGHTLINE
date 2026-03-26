#include "DmxMapper.h"

uint16_t DmxMapper::maybeInvert16(uint16_t input, bool invert) {
  if (!invert) {
    return input;
  }
  return static_cast<uint16_t>(65535 - input);
}

void DmxMapper::apply(const FixtureProfile& profile, const ControlState& control, uint16_t pan16,
                      uint16_t tilt16, DmxUniverse& outUniverse) {
  outUniverse.clear();

  uint16_t mappedPan = maybeInvert16(pan16, profile.pan.invert);
  if (profile.pan.hasLimits) {
    if (mappedPan < profile.pan.minValue) mappedPan = profile.pan.minValue;
    if (mappedPan > profile.pan.maxValue) mappedPan = profile.pan.maxValue;
  }
  if (profile.pan.coarse > 0 && profile.pan.fine > 0) {
    outUniverse.setChannel16(profile.pan.coarse, mappedPan);
  } else if (profile.pan.coarse > 0) {
    outUniverse.setChannel8(profile.pan.coarse, static_cast<uint8_t>(mappedPan >> 8));
  }

  uint16_t mappedTilt = maybeInvert16(tilt16, profile.tilt.invert);
  if (profile.tilt.hasLimits) {
    if (mappedTilt < profile.tilt.minValue) mappedTilt = profile.tilt.minValue;
    if (mappedTilt > profile.tilt.maxValue) mappedTilt = profile.tilt.maxValue;
  }
  if (profile.tilt.coarse > 0 && profile.tilt.fine > 0) {
    outUniverse.setChannel16(profile.tilt.coarse, mappedTilt);
  } else if (profile.tilt.coarse > 0) {
    outUniverse.setChannel8(profile.tilt.coarse, static_cast<uint8_t>(mappedTilt >> 8));
  }

  if (profile.dimmer.channel > 0) {
    uint8_t v = control.dimmer;
    if (profile.dimmer.hasLimits) {
      if (v < profile.dimmer.minValue) v = profile.dimmer.minValue;
      if (v > profile.dimmer.maxValue) v = profile.dimmer.maxValue;
    }
    if (profile.dimmer.invert) {
      v = static_cast<uint8_t>(255 - v);
    }
    outUniverse.setChannel8(profile.dimmer.channel, v);
  }
  if (profile.shutter.channel > 0) {
    outUniverse.setChannel8(profile.shutter.channel, profile.defaultShutterOpen ? 255 : 0);
  }
  if (profile.zoom.channel > 0) {
    outUniverse.setChannel8(profile.zoom.channel, control.zoom);
  }
  if (profile.iris.channel > 0) {
    outUniverse.setChannel8(profile.iris.channel, control.iris);
  }
  if (profile.focus.channel > 0) {
    outUniverse.setChannel8(profile.focus.channel, control.focus);
  }
  if (profile.color.channel > 0) {
    outUniverse.setChannel8(profile.color.channel, control.color);
  }
}
