#include "ControlRxService.h"

#include <WiFiUdp.h>

namespace sightline_v2 {

namespace {
WiFiUDP g_controlRxUdp;
}

bool ControlRxService::begin(uint16_t controlPort, const String& nodeId) {
  _port = controlPort;
  _nodeId = nodeId;
  // TODO(HW): Ensure Ethernet is initialized and link is up before binding socket.
  return g_controlRxUdp.begin(_port);
}

bool ControlRxService::acceptFrame(const ControlFrameV1& frame, const String& ownerControllerId, bool nodeClaimed,
                                   uint32_t nowMs) {
  _status.framesSeen++;
  if (frame.targetNodeId != _nodeId) {
    _status.framesRejected++;
    return false;
  }

  if (nodeClaimed && frame.controllerId != ownerControllerId) {
    _status.framesRejected++;
    return false;
  }

  if (_status.framesAccepted > 0) {
    _status.lastFrameIntervalMs = nowMs - _status.lastAcceptedAtMs;
  }
  _status.framesAccepted++;
  _status.lastAcceptedAtMs = nowMs;
  _status.receiving = true;
  _latest = frame;
  _hasFrame = true;
  return true;
}

bool ControlRxService::poll(uint32_t nowMs, const String& ownerControllerId, bool nodeClaimed) {
  const int len = g_controlRxUdp.parsePacket();
  if (len <= 0 || len > 1024) return false;

  char buf[1025] = {0};
  const int read = g_controlRxUdp.read(reinterpret_cast<uint8_t*>(buf), sizeof(buf) - 1);
  if (read <= 0) return false;

  ControlFrameV1 frame;
  String err;
  if (!_codec.decode(String(buf), frame, err)) {
    _status.framesRejected++;
    return false;
  }

  const bool accepted = acceptFrame(frame, ownerControllerId, nodeClaimed, nowMs);
  if (_status.receiving && (nowMs - _status.lastAcceptedAtMs) > 1500) {
    _status.receiving = false;
  }
  return accepted;
}

}  // namespace sightline_v2
