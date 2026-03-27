#include <Arduino.h>

#include "App.h"

App g_app;

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("=== SIGHTLINE Fixture Node Boot ===");
  Serial.printf("Build: %s %s\n", __DATE__, __TIME__);
  Serial.printf("Chip model: %s, cores: %d, rev: %d\n", ESP.getChipModel(), ESP.getChipCores(), ESP.getChipRevision());
  Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
  g_app.begin();
}

void loop() { g_app.tick(millis()); }
