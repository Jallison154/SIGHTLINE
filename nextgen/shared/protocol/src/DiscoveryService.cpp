#include "DiscoveryService.h"

#include <WiFiUdp.h>

namespace sightline_v2 {

namespace {
WiFiUDP g_discoveryUdp;
}  // namespace

bool DiscoveryService::begin(uint16_t discoveryPort) {
  _port = discoveryPort;
  // TODO(HW): Ensure Ethernet link is ready before opening discovery socket.
  return g_discoveryUdp.begin(_port);
}

void DiscoveryService::setSelfAnnouncement(const DiscoveryMessageV1& self) { _self = self; }

void DiscoveryService::tick(uint32_t nowMs) {
  sendAnnouncementIfDue(nowMs);
  receiveAnnouncements(nowMs);
  _registry.expireStale(nowMs, 3000, 10000);
}

void DiscoveryService::sendAnnouncementIfDue(uint32_t nowMs) {
  if ((nowMs - _lastTxMs) < _txPeriodMs) return;
  _lastTxMs = nowMs;

  DiscoveryMessageV1 out = _self;
  out.sentAtMs = nowMs;

  String payload;
  if (!_codec.encode(out, payload)) return;

  // TODO(HW): Choose proper broadcast target for local network policy.
  const IPAddress broadcastIp(255, 255, 255, 255);
  if (!g_discoveryUdp.beginPacket(broadcastIp, _port)) return;
  g_discoveryUdp.write(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length());
  g_discoveryUdp.endPacket();
}

void DiscoveryService::receiveAnnouncements(uint32_t nowMs) {
  const int len = g_discoveryUdp.parsePacket();
  if (len <= 0 || len > 1024) return;

  char buf[1025] = {0};
  const int read = g_discoveryUdp.read(reinterpret_cast<uint8_t*>(buf), sizeof(buf) - 1);
  if (read <= 0) return;

  DiscoveryMessageV1 msg;
  String err;
  if (!_codec.decode(String(buf), msg, err)) return;
  _registry.upsertFromAnnouncement(msg, nowMs);
}

}  // namespace sightline_v2
