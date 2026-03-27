# SIGHTLINE Browser USB Installer

This folder hosts browser-based USB flashing assets for SIGHTLINE (WLED-style flow).
Installer UI supports role selection between:
- **Controller firmware** (`deviceRole=controller`)
- **Fixture Node firmware** (`deviceRole=fixture_node`)

## Recommended implementation

- **ESP Web Tools** (`esp-web-install-button`) using **Web Serial**.
- Why: low maintenance, proven ecosystem, easy branded installer page.

## File layout

- `fixture-node/index.html` – branded install page
- `fixture-node/manifest.json` – Fixture Node ESP Web Tools manifest
- `fixture-node/manifest-controller.json` – Controller ESP Web Tools manifest
- `fixture-node/assets/...` – generated binaries for flashing

## Build + package binaries (Fixture Node)

From `fixture-node-firmware/`:

```bash
pio run -e esp32_eth_dev
pio run -e esp32_eth_dev -t buildfs
python3 tools/prepare_web_installer.py --env esp32_eth_dev
```

This copies:

- `bootloader.bin` (0x1000)
- `partitions.bin` (0x8000)
- `firmware.bin` (0x10000)
- optional `littlefs.bin` (0x190000; when `buildfs` was run)

into `web-installer/fixture-node/assets/esp32_eth_dev/` and regenerates `manifest.json`.

## Controller packaging status

Controller role selection is wired to `manifest-controller.json`.  
If controller assets are missing, the page shows a clear availability warning.

TODO: add a controller packaging script mirroring `fixture-node-firmware/tools/prepare_web_installer.py`.

## Hosting

Serve `web-installer/fixture-node/` over HTTPS (or localhost for development).
Do **not** open `index.html` directly with `file://` (the browser will block manifest fetch).

Examples:

```bash
cd web-installer/fixture-node
./serve_local.sh 8080
```

Then open: `http://localhost:8080` (works in local dev for many browsers).

Shortcut from repo root:

```bash
./make_installer.sh 8080
```

## Browser support

Web Serial requires Chromium-based browsers:

- Chrome / Edge / Opera (desktop)
- Not supported in Safari
- Firefox support is limited/non-default

## TODO (pipeline integration)

- Add dedicated controller asset packaging script (`deviceRole=controller`)
- Automate `build + buildfs + prepare_web_installer.py` in CI
- Publish installer assets + manifest to project website/releases
- Add versioned firmware metadata in manifest (`version`, changelog links)
