#include "ArtNetSender.h"

#include <cstring>

#include <IPAddress.h>
#include <WiFiUdp.h>

namespace {
constexpr uint8_t kArtNetId[8] = {'A', 'r', 't', '-', 'N', 'e', 't', '\0'};
constexpr uint16_t kOpDmx = 0x5000;
constexpr uint16_t kProtocolVersion = 14;
constexpr uint16_t kArtNetPort = 6454;
constexpr uint16_t kHeaderSize = 18;
WiFiUDP g_udp;
}  // namespace

bool ArtNetSender::begin() {
  // TODO(HW): Bring up Ethernet (ETH.begin) with board-specific PHY pins, clock mode, and PHY type.
  // TODO(HW): Wait for link-up and configure static IP or DHCP as needed for show network.
  // UDP bind to any local port for outbound Art-Net transmission.
  const bool ok = g_udp.begin(0);
  Serial.printf("ArtNetSender begin: udp=%s\n", ok ? "ok" : "failed");
  return ok;
}

bool ArtNetSender::setChannel(uint16_t channel1Based, uint8_t value) {
  if (channel1Based == 0 || channel1Based > kDmxSize) {
    return false;
  }
  _dmx[channel1Based - 1] = value;
  return true;
}

bool ArtNetSender::setBuffer(const uint8_t* data, uint16_t length) {
  if (!data) {
    return false;
  }
  if (length > kDmxSize) {
    length = kDmxSize;
  }
  std::memset(_dmx, 0, sizeof(_dmx));
  std::memcpy(_dmx, data, length);
  return true;
}

void ArtNetSender::setTargetIp(const IPAddress& ip) {
  _targetIpRaw = static_cast<uint32_t>(ip);
}

bool ArtNetSender::buildDmxPacket(uint8_t* outBuffer, uint16_t outCapacity, uint16_t& outLength) {
  outLength = static_cast<uint16_t>(kHeaderSize + kDmxSize);
  if (outCapacity < outLength) {
    return false;
  }

  std::memcpy(outBuffer, kArtNetId, sizeof(kArtNetId));
  outBuffer[8] = static_cast<uint8_t>(kOpDmx & 0xFF);
  outBuffer[9] = static_cast<uint8_t>((kOpDmx >> 8) & 0xFF);
  outBuffer[10] = static_cast<uint8_t>((kProtocolVersion >> 8) & 0xFF);
  outBuffer[11] = static_cast<uint8_t>(kProtocolVersion & 0xFF);
  outBuffer[12] = _sequence++;
  outBuffer[13] = 0;  // Physical port (unused)
  outBuffer[14] = static_cast<uint8_t>(_universe & 0xFF);
  outBuffer[15] = static_cast<uint8_t>((_universe >> 8) & 0xFF);
  outBuffer[16] = static_cast<uint8_t>((kDmxSize >> 8) & 0xFF);
  outBuffer[17] = static_cast<uint8_t>(kDmxSize & 0xFF);
  std::memcpy(outBuffer + kHeaderSize, _dmx, kDmxSize);
  return true;
}

bool ArtNetSender::sendFrame() {
  uint8_t packet[kHeaderSize + kDmxSize] = {0};
  uint16_t packetLength = 0;
  if (!buildDmxPacket(packet, sizeof(packet), packetLength)) {
    return false;
  }

  // TODO(HW): Confirm broadcast policy for your network; optionally switch to unicast target.
  const IPAddress targetIp(_targetIpRaw);
  const bool beginOk = g_udp.beginPacket(targetIp, kArtNetPort);
  if (!beginOk) {
    return false;
  }
  g_udp.write(packet, packetLength);
  const bool sent = g_udp.endPacket();

  static uint32_t frameCounter = 0;
  if (++frameCounter % 40 == 0) {
    Serial.printf("Art-Net tx frame=%lu universe=%u bytes=%u\n", static_cast<unsigned long>(frameCounter), _universe,
                  packetLength);
  }
  return sent;
}
