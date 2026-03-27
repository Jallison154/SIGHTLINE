#include "AbstractToDmxMapper.h"

#include <cstring>

namespace sightline_v2 {

namespace {
void set8(uint8_t* dmx, uint16_t ch, uint8_t v) {
  if (ch == 0 || ch > 512) return;
  dmx[ch - 1] = v;
}
void set16(uint8_t* dmx, uint16_t coarse, uint16_t v) {
  if (coarse == 0 || coarse >= 512) return;
  dmx[coarse - 1] = static_cast<uint8_t>((v >> 8) & 0xFF);
  dmx[coarse] = static_cast<uint8_t>(v & 0xFF);
}
void set16Or8(uint8_t* dmx, uint16_t coarse, uint16_t fine, uint16_t v) {
  if (coarse == 0 || coarse > 512) return;
  if (fine > 0 && fine <= 512 && coarse < 512) {
    set16(dmx, coarse, v);
    return;
  }
  // Coarse-only profile: downscale uint16 pan/tilt to 8-bit high byte.
  set8(dmx, coarse, static_cast<uint8_t>((v >> 8) & 0xFF));
}
}

void AbstractToDmxMapper::map(const AbstractControls& c, const FixtureProfile& p, uint8_t* outUniverse, uint16_t outSize) {
  if (!outUniverse || outSize < 512) return;
  std::memset(outUniverse, 0, 512);

  if (c.hasPan) set16Or8(outUniverse, p.pan.coarse, p.pan.fine, c.pan16);
  if (c.hasTilt) set16Or8(outUniverse, p.tilt.coarse, p.tilt.fine, c.tilt16);
  if (p.intensity.channel) set8(outUniverse, p.intensity.channel, c.hasIntensity ? c.intensity : p.defaultIntensity);
  if (p.iris.channel) set8(outUniverse, p.iris.channel, c.hasIris ? c.iris : p.defaultIris);
  if (p.zoom.channel) set8(outUniverse, p.zoom.channel, c.hasZoom ? c.zoom : p.defaultZoom);
  if (p.focus.channel) set8(outUniverse, p.focus.channel, c.hasFocus ? c.focus : p.defaultFocus);
  if (p.shutter.channel) set8(outUniverse, p.shutter.channel, c.hasShutter ? c.shutter : p.defaultShutter);
  if (p.color.channel) set8(outUniverse, p.color.channel, c.hasColor ? c.color : p.defaultColor);
}

}  // namespace sightline_v2
