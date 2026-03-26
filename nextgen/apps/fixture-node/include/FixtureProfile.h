#pragma once

#include <Arduino.h>

namespace sightline_v2 {

struct Channel8Map {
  uint16_t channel = 0;  // 1-based DMX, 0 means unmapped
  bool invert = false;
  bool hasLimits = false;
  uint8_t minValue = 0;
  uint8_t maxValue = 255;
};

struct Channel16Map {
  uint16_t coarse = 0;
  uint16_t fine = 0;
  bool invert = false;
  bool hasLimits = false;
  uint16_t minValue = 0;
  uint16_t maxValue = 65535;
};

struct FixtureProfile {
  String id;
  String fixtureName;
  String manufacturer;
  String modeName;
  uint16_t channelCount = 0;

  Channel16Map pan;
  Channel16Map tilt;
  Channel8Map intensity;
  Channel8Map iris;
  Channel8Map zoom;
  Channel8Map focus;
  Channel8Map shutter;
  Channel8Map color;

  uint8_t defaultIntensity = 0;
  uint8_t defaultIris = 0;
  uint8_t defaultZoom = 0;
  uint8_t defaultFocus = 0;
  uint8_t defaultShutter = 0;
  uint8_t defaultColor = 0;
};

}  // namespace sightline_v2
