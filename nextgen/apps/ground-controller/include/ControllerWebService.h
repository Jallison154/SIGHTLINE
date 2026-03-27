#pragma once

#include <Arduino.h>

#include "ControllerRuntimeState.h"
#include "ControllerStatus.h"
#include "OwnershipClient.h"
#include "TargetSelector.h"

class WebServer;

namespace sightline_v2 {

class ControllerWebService {
 public:
  bool begin(ControllerRuntimeState& runtimeState, ControllerStatus& status, TargetSelector& targetSelector,
             OwnershipClient& ownershipClient, const String& controllerId);
  void tick();
  bool ready() const { return _ready; }

 private:
  ControllerRuntimeState* _runtimeState = nullptr;
  ControllerStatus* _status = nullptr;
  TargetSelector* _targetSelector = nullptr;
  OwnershipClient* _ownershipClient = nullptr;
  String _controllerId;
  bool _ready = false;

  void handleGetState();
  void handlePatchState();
  void handleGetTargets();
  void handleSelectTarget();
  void handleOwnershipAction();
  void handleGetStatus();
  void handleAction();

  static const char* inputModeToString(InputMode mode);
  static InputMode inputModeFromString(const String& mode);
  static const char* targetStatusToString(TargetStatus status);
};

}  // namespace sightline_v2
