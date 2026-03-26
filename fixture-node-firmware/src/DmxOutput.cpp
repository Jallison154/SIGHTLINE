#include "DmxOutput.h"

#include <algorithm>
#include <cstring>

HardwareSerial* DmxOutput::selectSerial(uint8_t uartIndex) {
  if (uartIndex == 1) return &Serial1;
  if (uartIndex == 2) return &Serial2;
  return nullptr;
}

uart_port_t DmxOutput::selectUartPort(uint8_t uartIndex) {
  if (uartIndex == 1) return UART_NUM_1;
  if (uartIndex == 2) return UART_NUM_2;
  return UART_NUM_MAX;
}

bool DmxOutput::begin(const DmxOutputConfig& config) {
  _config = config;
  _serial = selectSerial(_config.uartIndex);
  _uartPort = selectUartPort(_config.uartIndex);
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
  _nextFrameDueUs = micros();
  _mabReadyUs = 0;
  _txState = TxState::kIdle;
  return true;
}

void DmxOutput::tick(uint32_t nowMs, const DmxBuffer& source) {
  (void)nowMs;
  const uint32_t nowUs = micros();

  if (_txState == TxState::kIdle) {
    if (static_cast<int32_t>(nowUs - _nextFrameDueUs) < 0) {
      return;
    }

    if (_config.directionPin >= 0 && !_config.keepDriverEnabled) {
      digitalWrite(_config.directionPin, HIGH);
    }

    sendBreak();
    _mabReadyUs = nowUs + _config.mabUs;
    _txState = TxState::kWaitingMab;
    return;
  }

  if (_txState == TxState::kWaitingMab) {
    if (static_cast<int32_t>(nowUs - _mabReadyUs) < 0) {
      return;
    }
    sendFrameData(source);
    _lastOutputMs = millis();
    _framesOutput++;
    _nextFrameDueUs = nowUs + _config.framePeriodUs;
    _txState = TxState::kIdle;
  }
}

void DmxOutput::sendBreak() {
  if (_serial == nullptr) {
    return;
  }
  _serial->flush();

  // Use ESP32 UART break support; avoids explicit blocking delay calls in loop.
  // TODO(HW): Validate break length on final board/transceiver with a DMX analyzer.
  if (_uartPort != UART_NUM_MAX) {
    constexpr uint32_t kBreakBits = 44;  // ~176us at 250k, safely above DMX minimum.
    uart_write_bytes_with_break(_uartPort, "", 0, kBreakBits);
  } else {
    // Fallback for unsupported UART selection.
    // TODO(HW): Replace fallback with board-specific deterministic break generation.
    _serial->updateBaudRate(90000);
    _serial->write(0x00);
    _serial->flush();
    _serial->updateBaudRate(_config.baudRate);
  }
}

void DmxOutput::sendFrameData(const DmxBuffer& source) {
  if (_serial == nullptr) {
    return;
  }
  const uint32_t startUs = micros();

  if (_config.directionPin >= 0 && !_config.keepDriverEnabled) {
    digitalWrite(_config.directionPin, HIGH);
  }

  std::memcpy(&_frameBytes[1], source.data(), DmxBuffer::kUniverseSize);
  _serial->write(_frameBytes, sizeof(_frameBytes));
  _serial->flush();

  if (_config.directionPin >= 0 && !_config.keepDriverEnabled) {
    digitalWrite(_config.directionPin, LOW);
  }

  _lastFrameDurationUs = micros() - startUs;
}
