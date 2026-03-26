#pragma once

#include <Arduino.h>

class IPAddress;

class ArtNetSender {
 public:
  static constexpr uint16_t kDmxSize = 512;

  bool begin();
  void setUniverse(uint16_t universe) { _universe = universe; }
  uint16_t universe() const { return _universe; }

  bool setChannel(uint16_t channel1Based, uint8_t value);
  bool setBuffer(const uint8_t* data, uint16_t length);
  bool sendFrame();

  // Optional target override; if unset, module may use broadcast.
  void setTargetIp(const IPAddress& ip);

 private:
  bool buildDmxPacket(uint8_t* outBuffer, uint16_t outCapacity, uint16_t& outLength);

  uint16_t _universe = 0;
  uint8_t _dmx[kDmxSize] = {0};
  uint8_t _sequence = 1;
  uint32_t _targetIpRaw = 0xFFFFFFFF;  // Default broadcast 255.255.255.255
};
