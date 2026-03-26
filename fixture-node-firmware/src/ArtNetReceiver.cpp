#include "ArtNetReceiver.h"

#include <cstring>
#include <WiFiUdp.h>

namespace {
constexpr uint16_t kArtNetPort = 6454;
constexpr uint16_t kOpDmx = 0x5000;
constexpr uint16_t kHeaderSize = 18;
constexpr uint16_t kArtNetProtocolMin = 14;
WiFiUDP g_udp;
}  // namespace

bool ArtNetReceiver::begin(uint16_t listenUniverse) {
  _listenUniverse = listenUniverse;

  // TODO(HW): Verify Ethernet interface init before opening UDP on chosen ESP32 ETH board.
  if (!g_udp.begin(kArtNetPort)) {
    return false;
  }
  return true;
}

bool ArtNetReceiver::parseAndApply(const uint8_t* packet, uint16_t length, uint32_t nowMs, DmxBuffer& outBuffer) {
  if (!packet || length < kHeaderSize) {
    _packetsBad++;
    return false;
  }
  if (std::memcmp(packet, "Art-Net\0", 8) != 0) {
    _packetsBad++;
    return false;
  }

  const uint16_t opCode = static_cast<uint16_t>(packet[8] | (packet[9] << 8));
  if (opCode != kOpDmx) {
    // Not bad; just not ArtDMX.
    return false;
  }

  const uint16_t protocol = static_cast<uint16_t>((packet[10] << 8) | packet[11]);
  if (protocol < kArtNetProtocolMin) {
    _packetsBad++;
    return false;
  }

  const uint16_t universe = static_cast<uint16_t>(packet[14] | (packet[15] << 8));
  if (universe != _listenUniverse) {
    _packetsIgnoredUniverse++;
    return false;
  }

  const uint16_t dmxLength = static_cast<uint16_t>((packet[16] << 8) | packet[17]);
  if (dmxLength == 0 || dmxLength > 512 || (dmxLength & 0x1) != 0 || (kHeaderSize + dmxLength) > length) {
    _packetsBad++;
    return false;
  }

  if (!outBuffer.applyPacketData(packet + kHeaderSize, dmxLength)) {
    _packetsBad++;
    return false;
  }

  if (_packetsAccepted > 0) {
    _lastFrameIntervalMs = nowMs - _lastAcceptedMs;
  }
  _lastAcceptedMs = nowMs;
  _packetsAccepted++;
  return true;
}

bool ArtNetReceiver::poll(uint32_t nowMs, DmxBuffer& outBuffer) {
  const int packetLength = g_udp.parsePacket();
  if (packetLength <= 0) {
    return false;
  }

  const int readCount = g_udp.read(_packetBuffer, sizeof(_packetBuffer));
  if (readCount <= 0) {
    _packetsBad++;
    return false;
  }

  _packetsSeen++;
  _lastPacketMs = nowMs;
  return parseAndApply(_packetBuffer, static_cast<uint16_t>(readCount), nowMs, outBuffer);
}

ArtNetSignalStatus ArtNetReceiver::signalStatus(uint32_t nowMs) const {
  ArtNetSignalStatus out;
  out.listenUniverse = _listenUniverse;
  out.packetsSeen = _packetsSeen;
  out.packetsAccepted = _packetsAccepted;
  out.packetsIgnoredUniverse = _packetsIgnoredUniverse;
  out.packetsBad = _packetsBad;
  out.lastPacketMs = _lastPacketMs;
  out.lastAcceptedMs = _lastAcceptedMs;
  out.lastFrameIntervalMs = _lastFrameIntervalMs;
  out.hasSignal = (_lastAcceptedMs > 0) && ((nowMs - _lastAcceptedMs) <= _lastSignalTimeoutMs);
  return out;
}
