#include "ArtNetSender.h"

#include <cstring>

namespace {
constexpr uint8_t kArtNetId[8] = {'A', 'r', 't', '-', 'N', 'e', 't', '\0'};
constexpr uint16_t kOpDmx = 0x5000;
constexpr uint16_t kProtocolVersion = 14;
constexpr uint16_t kArtNetPort = 6454;
constexpr uint16_t kHeaderSize = 18;
}  // namespace

bool ArtNetSender::begin() {
  // TODO(HW): Bring up Ethernet interface (ESP32 ETH) and UDP socket here.
  // TODO(HW): Configure target IP, subnet, and gateway or DHCP behavior.
  Serial.println("ArtNetSender initialized (packet build enabled, network send stubbed).");
  return true;
}

bool ArtNetSender::buildDmxPacket(uint16_t universe, const uint8_t* dmx, uint16_t dmxLength,
                                  uint8_t* outBuffer, uint16_t outCapacity, uint16_t& outLength) {
  if (!dmx || !outBuffer || dmxLength > 512) {
    return false;
  }
  outLength = static_cast<uint16_t>(kHeaderSize + dmxLength);
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
  outBuffer[14] = static_cast<uint8_t>(universe & 0xFF);
  outBuffer[15] = static_cast<uint8_t>((universe >> 8) & 0xFF);
  outBuffer[16] = static_cast<uint8_t>((dmxLength >> 8) & 0xFF);
  outBuffer[17] = static_cast<uint8_t>(dmxLength & 0xFF);
  std::memcpy(outBuffer + kHeaderSize, dmx, dmxLength);
  return true;
}

bool ArtNetSender::sendDmx(uint16_t universe, const uint8_t* dmx, uint16_t dmxLength) {
  uint8_t packet[kHeaderSize + 512] = {0};
  uint16_t packetLength = 0;
  if (!buildDmxPacket(universe, dmx, dmxLength, packet, sizeof(packet), packetLength)) {
    return false;
  }

  // TODO(HW): Send packet via UDP to broadcast or configured Art-Net target on port 6454.
  // Stub behavior for first working version:
  static uint32_t frameCounter = 0;
  frameCounter++;
  if ((frameCounter % 40) == 0) {
    Serial.printf("Art-Net stub tx: universe=%u len=%u port=%u\n", universe, packetLength, kArtNetPort);
  }
  return true;
}
