# SIGHTLINE

SIGHTLINE is a DIY remote follow-spot control system for intelligent moving head fixtures.

This repository is organized around two independent firmware targets plus shared data/docs:

```text
SIGHTLINE/
  controller-firmware/
  fixture-node-firmware/
  shared/
  docs/
```

## System Overview

- `controller-firmware`: ESP32 ground controller that reads physical controls and transmits Art-Net.
- `fixture-node-firmware`: ESP32 node that receives Art-Net, outputs DMX512, and hosts config UI.
- `shared`: fixture profile JSON examples and shared format documentation.
- `docs`: architecture, roadmap, and implementation notes.

## Why this layout

- Clear separation of deployable firmware targets.
- Independent PlatformIO projects to avoid environment coupling.
- Shared fixture profiles kept outside firmware for reuse and tooling.
- Folder names are short, explicit, and scalable as the project grows.

## Build (from each firmware folder)

- `pio run`
