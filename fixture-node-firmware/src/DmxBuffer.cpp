#include "DmxBuffer.h"

#include <cstring>

void DmxBuffer::clear() { std::memset(_channels, 0, sizeof(_channels)); }

bool DmxBuffer::applyPacketData(const uint8_t* data, uint16_t length) {
  if (!data) {
    return false;
  }
  if (length > kUniverseSize) {
    length = kUniverseSize;
  }
  std::memset(_channels, 0, sizeof(_channels));
  std::memcpy(_channels, data, length);
  return true;
}
