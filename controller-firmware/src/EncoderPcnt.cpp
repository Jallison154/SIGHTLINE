#include "EncoderPcnt.h"

namespace {
constexpr int16_t kCounterHighLimit = 32767;
constexpr int16_t kCounterLowLimit = -32768;
}

bool EncoderPcnt::begin(const EncoderPcntConfig& config) {
  if (config.pinA < 0 || config.pinB < 0) {
    return false;
  }

  _unit = config.unit;
  _invertDirection = config.invertDirection;

  pcnt_config_t chA{};
  chA.pulse_gpio_num = config.pinA;
  chA.ctrl_gpio_num = config.pinB;
  chA.channel = PCNT_CHANNEL_0;
  chA.unit = _unit;
  chA.pos_mode = PCNT_COUNT_DEC;
  chA.neg_mode = PCNT_COUNT_INC;
  chA.lctrl_mode = PCNT_MODE_REVERSE;
  chA.hctrl_mode = PCNT_MODE_KEEP;
  chA.counter_h_lim = kCounterHighLimit;
  chA.counter_l_lim = kCounterLowLimit;

  pcnt_config_t chB{};
  chB.pulse_gpio_num = config.pinB;
  chB.ctrl_gpio_num = config.pinA;
  chB.channel = PCNT_CHANNEL_1;
  chB.unit = _unit;
  chB.pos_mode = PCNT_COUNT_INC;
  chB.neg_mode = PCNT_COUNT_DEC;
  chB.lctrl_mode = PCNT_MODE_REVERSE;
  chB.hctrl_mode = PCNT_MODE_KEEP;
  chB.counter_h_lim = kCounterHighLimit;
  chB.counter_l_lim = kCounterLowLimit;

  if (pcnt_unit_config(&chA) != ESP_OK) {
    return false;
  }
  if (pcnt_unit_config(&chB) != ESP_OK) {
    return false;
  }

  // Hardware glitch filter reduces bounce/noise without CPU cost.
  pcnt_set_filter_value(_unit, config.glitchFilterCycles);
  pcnt_filter_enable(_unit);

  pcnt_counter_pause(_unit);
  pcnt_counter_clear(_unit);
  pcnt_counter_resume(_unit);

  _ready = true;
  return true;
}

int32_t EncoderPcnt::readAndResetDelta() {
  if (!_ready) {
    return 0;
  }
  int16_t count = 0;
  if (pcnt_get_counter_value(_unit, &count) != ESP_OK) {
    return 0;
  }
  pcnt_counter_clear(_unit);
  const int32_t delta = static_cast<int32_t>(count);
  return _invertDirection ? -delta : delta;
}
