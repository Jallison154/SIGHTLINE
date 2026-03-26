#pragma once

#include <Arduino.h>

#include "DmxBuffer.h"

struct ArtNetSignalStatus {
  bool hasSignal = false;
  uint16_t listenUniverse = 0;
  uint32_t packetsSeen = 0;
  uint32_t packetsAccepted = 0;
  uint32_t packetsIgnoredUniverse = 0;
  uint32_t packetsBad = 0;
  uint32_t lastPacketMs = 0;
  uint32_t lastAcceptedMs = 0;
  uint32_t lastFrameIntervalMs = 0;
};

class ArtNetReceiver {
 public:
  bool begin(uint16_t listenUniverse);
  bool poll(uint32_t nowMs, DmxBuffer& outBuffer);
  void setUniverse(uint16_t universe) { _listenUniverse = universe; }
  ArtNetSignalStatus signalStatus(uint32_t nowMs) const;

 private:
  bool parseAndApply(const uint8_t* packet, uint16_t length, uint32_t nowMs, DmxBuffer& outBuffer);

  uint16_t _listenUniverse = 0;
  uint32_t _packetsSeen = 0;
  uint32_t _packetsAccepted = 0;
  uint32_t _packetsIgnoredUniverse = 0;
  uint32_t _packetsBad = 0;
  uint32_t _lastPacketMs = 0;
  uint32_t _lastAcceptedMs = 0;
  uint32_t _lastFrameIntervalMs = 0;
  uint32_t _lastSignalTimeoutMs = 1000;
  uint8_t _packetBuffer[530] = {0};
};
