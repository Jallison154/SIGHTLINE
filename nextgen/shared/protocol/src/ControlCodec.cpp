#include "ControlCodec.h"

#include <ArduinoJson.h>

namespace sightline_v2 {

bool ControlCodec::encode(const ControlFrameV1& f, String& outJson) {
  StaticJsonDocument<768> doc;
  doc["schema_version"] = f.schemaVersion;
  doc["msg_type"] = f.msgType;
  doc["session_id"] = f.sessionId;
  doc["controller_id"] = f.controllerId;
  doc["target_node_id"] = f.targetNodeId;
  doc["frame_seq"] = f.frameSeq;
  doc["sent_at_ms"] = f.sentAtMs;

  JsonObject c = doc.createNestedObject("controls");
  if (f.controls.hasPan) c["pan"] = f.controls.pan;
  if (f.controls.hasTilt) c["tilt"] = f.controls.tilt;
  if (f.controls.hasIntensity) c["intensity"] = f.controls.intensity;
  if (f.controls.hasIris) c["iris"] = f.controls.iris;
  if (f.controls.hasZoom) c["zoom"] = f.controls.zoom;
  if (f.controls.hasFocus) c["focus"] = f.controls.focus;
  if (f.controls.hasShutter) c["shutter"] = f.controls.shutter;
  if (f.controls.hasColor) c["color"] = f.controls.color;

  if (!f.controls.extraJson.isEmpty()) {
    StaticJsonDocument<128> extraDoc;
    if (deserializeJson(extraDoc, f.controls.extraJson) == DeserializationError::Ok) {
      c["extra"] = extraDoc.as<JsonVariantConst>();
    }
  }

  outJson.clear();
  serializeJson(doc, outJson);
  return !outJson.isEmpty();
}

bool ControlCodec::decode(const String& json, ControlFrameV1& outFrame, String& outError) {
  StaticJsonDocument<768> doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    outError = String("decode failed: ") + err.c_str();
    return false;
  }

  outFrame.schemaVersion = doc["schema_version"] | 0;
  outFrame.msgType = doc["msg_type"] | "";
  outFrame.sessionId = doc["session_id"] | "";
  outFrame.controllerId = doc["controller_id"] | "";
  outFrame.targetNodeId = doc["target_node_id"] | "";
  outFrame.frameSeq = doc["frame_seq"] | 0;
  outFrame.sentAtMs = doc["sent_at_ms"] | 0;

  JsonObject c = doc["controls"];
  if (!c.isNull()) {
    outFrame.controls.hasPan = c.containsKey("pan");
    outFrame.controls.pan = c["pan"] | 0.5f;
    outFrame.controls.hasTilt = c.containsKey("tilt");
    outFrame.controls.tilt = c["tilt"] | 0.5f;
    outFrame.controls.hasIntensity = c.containsKey("intensity");
    outFrame.controls.intensity = c["intensity"] | 0;
    outFrame.controls.hasIris = c.containsKey("iris");
    outFrame.controls.iris = c["iris"] | 0;
    outFrame.controls.hasZoom = c.containsKey("zoom");
    outFrame.controls.zoom = c["zoom"] | 0;
    outFrame.controls.hasFocus = c.containsKey("focus");
    outFrame.controls.focus = c["focus"] | 0;
    outFrame.controls.hasShutter = c.containsKey("shutter");
    outFrame.controls.shutter = c["shutter"] | 0;
    outFrame.controls.hasColor = c.containsKey("color");
    outFrame.controls.color = c["color"] | 0;
  }

  if (outFrame.schemaVersion != 1 || outFrame.msgType != "control_frame" || outFrame.controllerId.isEmpty() ||
      outFrame.targetNodeId.isEmpty()) {
    outError = "invalid control frame envelope";
    return false;
  }
  return true;
}

}  // namespace sightline_v2
