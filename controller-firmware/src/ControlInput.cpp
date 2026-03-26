#include "ControlInput.h"

bool ControlInput::begin() {
  // TODO(HW): Assign actual ESP32 GPIOs for the pan encoder channels.
  constexpr int kPanEncoderPinA = -1;
  constexpr int kPanEncoderPinB = -1;
  // TODO(HW): Assign actual ESP32 GPIOs for the tilt encoder channels.
  constexpr int kTiltEncoderPinA = -1;
  constexpr int kTiltEncoderPinB = -1;

  EncoderPcntConfig panCfg;
  panCfg.pinA = kPanEncoderPinA;
  panCfg.pinB = kPanEncoderPinB;
  panCfg.unit = PCNT_UNIT_0;
  panCfg.glitchFilterCycles = 100;

  EncoderPcntConfig tiltCfg;
  tiltCfg.pinA = kTiltEncoderPinA;
  tiltCfg.pinB = kTiltEncoderPinB;
  tiltCfg.unit = PCNT_UNIT_1;
  tiltCfg.glitchFilterCycles = 100;

  const bool panOk = _panEncoder.begin(panCfg);
  const bool tiltOk = _tiltEncoder.begin(tiltCfg);
  _useSimulation = !(panOk && tiltOk);

  if (_useSimulation) {
    Serial.println("ControlInput: encoder pins not configured; using simulation deltas.");
  }
  return true;
}

void ControlInput::update(uint32_t nowMs, uint32_t dtMs, ControlState& outState) {
  (void)nowMs;
  (void)dtMs;

  if (!_useSimulation) {
    // Hardware path: non-blocking read of accumulated encoder movement since last tick.
    outState.panEncoderDelta = _panEncoder.readAndResetDelta();
    outState.tiltEncoderDelta = _tiltEncoder.readAndResetDelta();
  } else {
    // Simulation fallback keeps full motion pipeline testable without wiring.
    _simPanAccumulator = (_simPanAccumulator + 1) % 400;
    _simTiltAccumulator = (_simTiltAccumulator + 1) % 600;

    outState.panEncoderDelta = (_simPanAccumulator < 200) ? 1 : -1;
    if ((_simPanAccumulator % 80) < 10) {
      outState.panEncoderDelta *= 3;
    }

    outState.tiltEncoderDelta = (_simTiltAccumulator < 300) ? 1 : -1;
    if ((_simTiltAccumulator % 120) < 12) {
      outState.tiltEncoderDelta *= 2;
    }
  }

  // TODO(HW): Wire physical knobs/buttons later.
  outState.dimmer = 255;
  outState.zoom = 0;
  outState.iris = 0;
  outState.focus = 0;
  outState.color = 0;
}
