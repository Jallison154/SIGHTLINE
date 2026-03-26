#include "ControlTxService.h"

#include <IPAddress.h>
#include <WiFiUdp.h>

namespace sightline_v2 {

namespace {
WiFiUDP g_controlTxUdp;
}

bool ControlTxService::begin(uint16_t controlPort) {
  _port = controlPort;
  // TODO(HW): Ensure Ethernet is initialized and link is up before sending frames.
  return g_controlTxUdp.begin(0);
}

void ControlTxService::setTargetIp(const String& ip) { _targetIp = ip; }
void ControlTxService::setTargetNodeId(const String& nodeId) { _targetNodeId = nodeId; }
void ControlTxService::setControllerId(const String& controllerId) { _controllerId = controllerId; }

bool ControlTxService::sendFrame(const ControlFrameV1& frameIn) {
  IPAddress ip;
  if (!ip.fromString(_targetIp)) return false;

  ControlFrameV1 f = frameIn;
  f.controllerId = _controllerId;
  f.targetNodeId = _targetNodeId;

  String payload;
  if (!_codec.encode(f, payload)) return false;

  if (!g_controlTxUdp.beginPacket(ip, _port)) return false;
  g_controlTxUdp.write(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length());
  return g_controlTxUdp.endPacket();
}

}  // namespace sightline_v2
