#include "ArtNetReceiver.h"

#include <WiFiUdp.h>

namespace {
constexpr uint16_t kArtNetPort = 6454;
constexpr uint16_t kOpDmx = 0x5000;
constexpr uint16_t kHeaderSize = 18;
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

bool ArtNetReceiver::parseAndApply(const uint8_t* packet, uint16_t length, DmxBuffer& outBuffer) {
  if (!packet || length < kHeaderSize) {
    return false;
  }
  if (memcmp(packet, "Art-Net\0", 8) != 0) {
    return false;
  }

  const uint16_t opCode = static_cast<uint16_t>(packet[8] | (packet[9] << 8));
  if (opCode != kOpDmx) {
    return false;
  }

  const uint16_t universe = static_cast<uint16_t>(packet[14] | (packet[15] << 8));
  if (universe != _listenUniverse) {
    _packetsIgnoredUniverse++;
    return false;
  }

  const uint16_t dmxLength = static_cast<uint16_t>((packet[16] << 8) | packet[17]);
  if (dmxLength > 512 || (kHeaderSize + dmxLength) > length) {
    return false;
  }

  if (!outBuffer.applyPacketData(packet + kHeaderSize, dmxLength)) {
    return false;
  }
  _packetsAccepted++;
  return true;
}

bool ArtNetReceiver::poll(DmxBuffer& outBuffer) {
  const int packetLength = g_udp.parsePacket();
  if (packetLength <= 0) {
    return false;
  }

  uint8_t packet[600] = {0};
  const int readCount = g_udp.read(packet, sizeof(packet));
  if (readCount <= 0) {
    return false;
  }

  _packetsSeen++;
  return parseAndApply(packet, static_cast<uint16_t>(readCount), outBuffer);
}
