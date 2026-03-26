#pragma once

#include <Arduino.h>

class ArtNetSender {
 public:
  bool begin();
  bool sendDmx(uint16_t universe, const uint8_t* dmx, uint16_t dmxLength);

 private:
  bool buildDmxPacket(uint16_t universe, const uint8_t* dmx, uint16_t dmxLength, uint8_t* outBuffer,
                      uint16_t outCapacity, uint16_t& outLength);
  uint8_t _sequence = 1;
};
