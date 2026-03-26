#include "DmxOutput.h"

#include <cstring>

HardwareSerial* DmxOutput::selectSerial(uint8_t uartIndex) {
  if (uartIndex == 1) return &Serial1;
  if (uartIndex == 2) return &Serial2;
  return nullptr;
}

bool DmxOutput::begin(const DmxOutputConfig& config) {
  _config = config;
  _serial = selectSerial(_config.uartIndex);
  if (_serial == nullptr) {
    return false;
  }

  // TODO(HW): Confirm TX pin mapping for chosen ESP32 + PHY board.
  // DMX uses 250k baud, SERIAL_8N2.
  _serial->begin(_config.baudRate, SERIAL_8N2, -1, _config.txPin);

  if (_config.directionPin >= 0) {
    pinMode(_config.directionPin, OUTPUT);
    // High enables TX on many RS-485 transceivers, but verify your part.
    digitalWrite(_config.directionPin, _config.keepDriverEnabled ? HIGH : LOW);
  }

  _frameBytes[0] = 0x00;  // DMX start code
  _lastOutputMs = 0;
  _framesOutput = 0;
  _lastFrameDurationUs = 0;
  return true;
}

void DmxOutput::tick(uint32_t nowMs, const DmxBuffer& source) {
  const uint32_t kDmxFramePeriodMs = _config.framePeriodUs / 1000;
  if ((nowMs - _lastOutputMs) < kDmxFramePeriodMs) {
    return;
  }
  _lastOutputMs = nowMs;
  sendFrame(source);
  _framesOutput++;
}

void DmxOutput::sendBreakAndMab() {
  // TODO(HW): For production, implement deterministic break/MAB using ESP-IDF UART APIs
  // (uart_write_bytes_with_break / direct UART registers), tested on final board.
  // Current portable fallback approximates break by temporarily lowering baud.
  _serial->flush();
  _serial->updateBaudRate(90000);
  _serial->write(0x00);
  _serial->flush();
  _serial->updateBaudRate(_config.baudRate);
  delayMicroseconds(12);  // Approximate Mark-After-Break (>=8us)
}

void DmxOutput::sendFrame(const DmxBuffer& source) {
  const uint32_t startUs = micros();

  if (_config.directionPin >= 0 && !_config.keepDriverEnabled) {
    digitalWrite(_config.directionPin, HIGH);
  }

  sendBreakAndMab();

  std::memcpy(&_frameBytes[1], source.data(), DmxBuffer::kUniverseSize);
  _serial->write(_frameBytes, sizeof(_frameBytes));
  _serial->flush();

  if (_config.directionPin >= 0 && !_config.keepDriverEnabled) {
    digitalWrite(_config.directionPin, LOW);
  }

  _lastFrameDurationUs = micros() - startUs;
}
