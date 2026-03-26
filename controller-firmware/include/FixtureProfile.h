#pragma once

#include <Arduino.h>

struct Channel8Map {
  uint16_t channel = 0;  // 1-based DMX index, 0 means "not present"
  bool invert = false;
  bool hasLimits = false;
  uint8_t minValue = 0;
  uint8_t maxValue = 255;
};

struct Channel16Map {
  uint16_t coarse = 0;  // 1-based DMX index
  uint16_t fine = 0;    // 1-based DMX index, 0 means "8-bit only"
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
  Channel8Map dimmer;
  Channel8Map shutter;
  Channel8Map zoom;
  Channel8Map iris;
  Channel8Map focus;
  Channel8Map color;

  bool defaultShutterOpen = true;
  uint8_t defaultDimmer = 255;
  float deadband = 0.02f;
  float smoothingAlpha = 0.25f;
  float panSpeedScale = 1.0f;
  float tiltSpeedScale = 1.0f;
};
