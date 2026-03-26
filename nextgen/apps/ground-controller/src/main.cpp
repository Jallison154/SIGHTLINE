#include <Arduino.h>

#include "ControllerApp.h"

ControllerApp g_app;

void setup() {
  Serial.begin(115200);
  g_app.begin();
}

void loop() { g_app.tick(millis()); }
