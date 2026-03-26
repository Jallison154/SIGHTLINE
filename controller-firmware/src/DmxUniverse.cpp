#include "DmxUniverse.h"

#include <cstring>

void DmxUniverse::clear() { std::memset(_channels, 0, sizeof(_channels)); }

bool DmxUniverse::setChannel8(uint16_t channel1Based, uint8_t value) {
  if (channel1Based == 0 || channel1Based > kSize) {
    return false;
  }
  _channels[channel1Based - 1] = value;
  return true;
}

bool DmxUniverse::setChannel16(uint16_t coarseChannel1Based, uint16_t value) {
  if (coarseChannel1Based == 0 || coarseChannel1Based >= kSize) {
    return false;
  }
  _channels[coarseChannel1Based - 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
  _channels[coarseChannel1Based] = static_cast<uint8_t>(value & 0xFF);
  return true;
}
