#pragma once

#include <Arduino.h>

#include "DmxBuffer.h"

class ArtNetReceiver {
 public:
  bool begin(uint16_t listenUniverse);
  bool poll(DmxBuffer& outBuffer);
  void setUniverse(uint16_t universe) { _listenUniverse = universe; }

  uint32_t packetsSeen() const { return _packetsSeen; }
  uint32_t packetsAccepted() const { return _packetsAccepted; }
  uint32_t packetsIgnoredUniverse() const { return _packetsIgnoredUniverse; }

 private:
  bool parseAndApply(const uint8_t* packet, uint16_t length, DmxBuffer& outBuffer);

  uint16_t _listenUniverse = 0;
  uint32_t _packetsSeen = 0;
  uint32_t _packetsAccepted = 0;
  uint32_t _packetsIgnoredUniverse = 0;
};
