#pragma once

#include <Arduino.h>

#include "FixtureProfileManager.h"

namespace sightline_v2 {

class FixtureProfileWebApi {
 public:
  bool begin(FixtureProfileManager& manager);
  void tick();

 private:
  FixtureProfileManager* _manager = nullptr;
};

}  // namespace sightline_v2
