# Fixture Node Firmware

ESP32-based fixture node firmware for SIGHTLINE.

## Responsibilities

- Receive Art-Net over Ethernet
- Output DMX512 to moving head fixture
- Serve lightweight web UI for configuration
- Persist settings in non-volatile memory

## Structure

- `platformio.ini`: PlatformIO project config
- `include/`: module headers
- `src/`: application entrypoint and modules
- `lib/`: private reusable components for this target
- `data/`: static web assets
- `config/`: local config placeholders and defaults

## PlatformIO Flashing Quick Start

From `fixture-node-firmware/`:

1. Build firmware:

```bash
pio run -e esp32_eth_dev
```

2. Upload firmware:

```bash
pio run -e esp32_eth_dev -t upload
```

3. Upload filesystem (LittleFS web UI assets from `data/`):

```bash
pio run -e esp32_eth_dev -t uploadfs
```

4. Open serial monitor:

```bash
pio device monitor -b 115200
```

Optional combined flow after web/UI edits:

```bash
pio run -e esp32_eth_dev -t upload && pio run -e esp32_eth_dev -t uploadfs
```

## Browser USB flashing (Web Serial)

SIGHTLINE now includes a browser-based installer (WLED-style) under:

- `../web-installer/fixture-node/index.html`

Prepare installer binaries:

```bash
pio run -e esp32_eth_dev
pio run -e esp32_eth_dev -t buildfs
python3 tools/prepare_web_installer.py --env esp32_eth_dev
```

From repo root, you can run the full pipeline + local hosting in one command:

```bash
./make_installer.sh 8080
```

Then host `web-installer/fixture-node/` and open it in a Chromium-based browser.

Note: `partitions_sightline.csv` currently uses a larger LittleFS partition (single factory app slot, no OTA slots) to fit web assets and branding images.

## Web UI (LittleFS)

The fixture node serves HTML, CSS, JS, and `sightline_logo.png` from LittleFS. Filenames in `data/` are case-sensitive and must match URL paths (use lowercase, e.g. `sightline_logo.png`). After you change any file under `data/`, upload the filesystem image to the board (not just the firmware):

```bash
pio run -e esp32_eth_dev -t uploadfs
```

To replace the logo, overwrite `data/sightline_logo.png` and run `uploadfs` again.

### Data folder checklist (ready for uploadfs)

- `data/index.html`
- `data/style.css`
- `data/app.js`
- `data/sightline_logo.png`

These are served by `WebUiServer` from LittleFS at:

- `/`
- `/style.css`
- `/app.js`
- `/sightline_logo.png`

## DMX512 Output Notes

`DmxOutput` uses ESP32 UART in `250000` baud, `SERIAL_8N2`, with a full universe frame:

- start code (`0x00`)
- 512 channel bytes
- continuous refresh (default `~40Hz`, `25ms`)

Board-specific tuning remains marked as `TODO(HW)`:

- final UART index and TX pin
- RS-485 DE/RE direction control pin behavior
- deterministic BREAK/MAB generation using ESP-IDF UART primitives
