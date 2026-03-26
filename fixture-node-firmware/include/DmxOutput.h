#pragma once

#include <Arduino.h>
#include <driver/uart.h>

#include "DmxBuffer.h"

struct DmxOutputConfig {
  // UART index for ESP32 HardwareSerial (commonly 1 or 2).
  uint8_t uartIndex = 2;
  // DMX TX output pin connected to RS-485 DI.
  int8_t txPin = 17;
  // Optional RE/DE pin for RS-485 transceiver direction control.
  int8_t directionPin = -1;
  // DMX requires 250000 baud, 8 data bits, no parity, 2 stop bits.
  uint32_t baudRate = 250000;
  uint32_t framePeriodUs = 25000;  // ~40Hz refresh.
  uint32_t mabUs = 12;             // Mark-after-break target
  bool keepDriverEnabled = true;
};

class DmxOutput {
 public:
  bool begin(const DmxOutputConfig& config = DmxOutputConfig{});
  void tick(uint32_t nowMs, const DmxBuffer& source);

  uint32_t framesOutput() const { return _framesOutput; }
  uint32_t lastFrameDurationUs() const { return _lastFrameDurationUs; }

 private:
  void sendFrameData(const DmxBuffer& source);
  void sendBreak();
  HardwareSerial* selectSerial(uint8_t uartIndex);
  uart_port_t selectUartPort(uint8_t uartIndex);

  enum class TxState : uint8_t {
    kIdle = 0,
    kWaitingMab = 1
  };

  DmxOutputConfig _config;
  HardwareSerial* _serial = nullptr;
  uart_port_t _uartPort = UART_NUM_MAX;
  uint8_t _frameBytes[513] = {0};  // start code + 512 slots
  uint32_t _lastOutputMs = 0;
  uint32_t _framesOutput = 0;
  uint32_t _lastFrameDurationUs = 0;
  uint32_t _nextFrameDueUs = 0;
  uint32_t _mabReadyUs = 0;
  TxState _txState = TxState::kIdle;
};
