# somnus-dial (ESP-IDF firmware)

Native **ESP-IDF (C)** firmware for the Waveshare ESP32-S3 round touch-LCD
knob — the dial in **Bedknob for Somnus**. It drives a **Somnus Pad** directly
over the pad's own local HTTP API: three endpoints on your LAN (`GET
/api/state`, `POST /api/power`, `POST /api/target_t`), no account, no cloud,
nothing in between. Wi-Fi setup, finding the pad, the whole UI and
over-the-air updates all run on the dial itself.

## Just want to use the dial?

Flash it from your browser at
[https://matthewclaude.github.io/somnus-dial-releases/](https://matthewclaude.github.io/somnus-dial-releases/)
— Chrome or Edge, a **USB-A to USB-C data cable**, click Install. No
toolchain needed. That flash writes the whole chip and so **always erases** a
dial's settings; after it, every future update arrives over the air
(Menu → Update) and keeps your settings. The rest of this README covers
building and modifying the firmware yourself.

## Hardware

- **Board:** Waveshare `ESP32-S3-Knob-Touch-LCD-1.8` — round 1.8" touch LCD
  + rotary encoder knob, ESP32-S3, 16 MB flash, 8 MB PSRAM. Parts table in
  [`../README.md`](../README.md).
- **Cable:** a **USB-A to USB-C data cable**, straight into your computer,
  for flashing and serial monitoring. A **C-to-C cable will not work** with
  this board: it negotiates plug orientation itself, which defeats the
  orientation trick below. A charge-only cable powers the board and
  enumerates nothing.
- **Port:** the board enumerates as a USB-serial device — e.g.
  `/dev/cu.usbmodem2101` on macOS, `/dev/ttyUSB0` or `/dev/ttyACM0` on Linux,
  `COM<N>` on Windows. Pass it to `idf.py` with `-p <PORT>`; if you omit `-p`,
  `idf.py` will try to auto-detect it.
- **No port, or the wrong one?** The dial's single USB-C socket reaches a
  different chip depending on which way the connector sits — rotate the
  same connector 180° in the **dial's own socket** (don't swap which end
  of the cable goes where) to switch. For flashing and monitoring you want
  the **S3**, which shows up as `usbmodem*` / "USB JTAG/serial debug unit";
  the other orientation reaches a companion chip that enumerates as
  `usbserial*` instead.

## Prerequisites

**This section and Build & flash below are the developer path** —
building or modifying the firmware yourself. If you just want a working
dial, use the browser flasher above instead; nothing here is required for
that.

This firmware is pinned to **ESP-IDF v6.0**, target **esp32s3**. Install it
with Espressif's standard flow (no project-specific scripts needed):

```bash
git clone -b v6.0 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh              # puts idf.py on PATH for this shell session
```

`export.sh` only sets up the current shell — re-source it (`. $IDF_PATH/export.sh`)
in every new terminal before running `idf.py`. See Espressif's own
[Get Started guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html)
for platform-specific prerequisites (Python, USB drivers, etc.) and
troubleshooting.

## Build & flash

With ESP-IDF v6.0 installed from above, this produces your own build and
puts it on a dial over USB:

```bash
cd firmware/dial-idf
idf.py build
idf.py -p <PORT> flash monitor
```

**Do not run `idf.py set-target`.** The board configuration (quad flash,
octal PSRAM, 16 MB, custom partition table) is already in place —
`sdkconfig.defaults` is tracked and `sdkconfig` is generated from it on the
first build — and `set-target` regenerates `sdkconfig` from scratch and
discards that configuration.

A wire flash writes only the bootloader, partition table, OTA data and the
app (see `build/flash_args`), so unlike the browser flasher it **preserves**
settings. No configuration or secrets file is required — a freshly flashed
dial boots straight into on-device setup (see below). `build/`,
`managed_components/`, and `sdkconfig` are generated (git-ignored); `idf.py`
recreates them.

**Optional, developers only:** to skip re-entering Wi-Fi credentials on every
reflash during development, `cp main/secrets.h.example main/secrets.h` and
fill in your network. `secrets.h` is git-ignored and only pre-seeds NVS the
first time there are no stored credentials — it changes nothing for anyone
who doesn't create it.

### Partition table

From [`partitions.csv`](partitions.csv) (16 MB flash, two OTA slots so
updates ship over the air with rollback). Offsets are the ones the build
assigns; the CSV leaves them blank.

| Name | Type | SubType | Offset | Size |
|---|---|---|---|---|
| `nvs` | data | nvs | `0x9000` | 64K |
| `otadata` | data | ota | `0x19000` | 8K |
| `phy_init` | data | phy | `0x1b000` | 4K |
| `ota_0` | app | ota_0 | `0x20000` | 4M |
| `ota_1` | app | ota_1 | `0x420000` | 4M |
| `assets` | data | spiffs | `0x820000` | 7936K |

`assets` reserves the tail for future fonts/images without repartitioning
(repartitioning wipes NVS and forces every flashed unit through setup again).

### Release images and manual esptool flashing

Each GitHub Release (tag `somnus-vX.Y.Z`, published to
[matthewclaude/somnus-dial-releases](https://github.com/matthewclaude/somnus-dial-releases))
carries two images:

- **`somnus-dial.bin`** — the OTA app image, what the dial downloads for
  itself over the air.
- **`somnus-dial-merged.bin`** — a full-flash image with bootloader +
  partition table + OTA data + app already combined at their real offsets,
  written starting at **`0x0`**. That's the image the browser flasher
  writes, and you can write it yourself the same way:

```bash
python -m esptool --chip esp32s3 -p <PORT> -b 921600 write-flash 0x0 somnus-dial-merged.bin
```

If you're building locally and want to reproduce what `idf.py flash` does
with a manual `esptool` invocation, the offsets come from the table above
and `build/flash_args`:

```bash
python -m esptool --chip esp32s3 -p <PORT> -b 921600 \
  --before default-reset --after hard-reset write-flash \
  --flash-mode dio --flash-size 16MB --flash-freq 80m \
  0x0     build/bootloader/bootloader.bin \
  0x8000  build/partition_table/partition-table.bin \
  0x19000 build/ota_data_initial.bin \
  0x20000 build/somnus-dial.bin
```

Prefer `idf.py -p <PORT> flash` for everyday development — it derives these
offsets itself and is far less likely to go stale if the partition table
ever changes.

## Components

One line each, as they exist in [`components/`](components/):

- `dial_display` — QSPI panel (SH8601) + touch + LVGL bring-up; owns the LVGL task and lock.
- `dial_haptics` — DRV2605 LRA effects from a small dedicated task, with the Off / Low / High / Auto strength clamp and NVS-cached autocal.
- `dial_knob` — rotary encoder decoding (`bidi_switch_knob`, Espressif `iot_knob` lineage).
- `dial_net` — Wi-Fi bring-up, NVS-backed credentials, the SoftAP captive portal and network scan.
- `dial_ota` — GitHub Releases version check + `esp_https_ota` download/apply/rollback.
- `dial_pad_discovery` — subnet scan that finds the pad when its address is unknown or has moved.
- `dial_power` — idle-driven backlight dimming/standby, day/night/night-clock duty tables, wake-consumes-first-input.
- `dial_somnus` — the pad client: the three local HTTP API calls, zone-mode write guard, last-error text.
- `dial_state` — the single mutex-protected app-state snapshot, the UI→worker command queue, NVS-backed preferences.
- `dial_time` — SNTP plus an embedded IANA→POSIX timezone table, persisted so the clock survives reboots.
- `dial_ui` — screen router (`ui_router`) and one `scr_*.c` per screen.
- `i2c_bsp` — the shared I²C bus (touch controller and haptics driver).
- `lcd_bl_pwm_bsp` — backlight PWM.
- `lcd_touch_bsp` — CST816 touch controller.

How they fit together is in [`../../docs/ARCHITECTURE.md`](../../docs/ARCHITECTURE.md).

## First boot

The full walkthrough is in the [root README](../../README.md#first-boot-on-the-dial).
In short, a freshly flashed (or factory-reset) dial does setup on the
device itself:

1. **Welcome.** A splash screen; any tap dismisses it.
2. **Wi-Fi.** The dial can't reach anything until it has your network, and
   offers two ways to give it that:
   - **From your phone:** the dial names a temporary network (`SomnusDial-XXXX`).
     Join it from your phone's Wi-Fi settings and a setup page opens on its
     own (it hijacks DNS so most phones pop the page automatically); if it
     doesn't, open any page in a browser. Pick your home network and enter
     its password. The page also passes your phone's timezone to the dial.
   - **On the dial:** tap "Set up on the dial" to skip the phone. Turn the
     knob to pick your network from a scanned list, then type the password
     with the on-screen character wheel. A wrong password sends you back to
     this screen for the same network with a message, not to square one.

   2.4 GHz only — this hardware doesn't support 5 GHz networks.
3. **Timezone.** If the setup page couldn't supply one (some phone browsers
   don't), the dial shows a short picker the first time it reaches the pad.
4. **Finding the pad.** The dial probes the last known address, then scans
   your subnet for anything answering `/api/state` on port 8080. No IP to
   type. If it finds nothing, Settings → Pad Address takes one by hand.
5. **Bed Mode.** Settings → Bed Mode: **One Bed** or **Dual Sides**, matching
   the toggle in the Somnus app. The pad's API cannot report this, so the
   dial trusts you and names the mode on the face. In Dual Sides mode a
   fresh dial also asks "Which side of the bed?" once.
6. **The dial screen.** From here on: the live temperature dial, with a
   swipe to the other side (Dual Sides) and to the menu.

## Troubleshooting / FAQ

- **"That password didn't work. Try again."** — the password screen tells you
  the join was rejected and puts you right back on it for the same network;
  just retype it.
- **My Wi-Fi network doesn't show up / won't connect.** This hardware is
  **2.4 GHz only** — it cannot join 5 GHz-only networks. If your router
  broadcasts both bands under one SSID, make sure the 2.4 GHz radio is
  actually enabled.
- **The dial is stuck on "Connecting…" / "Pad unreachable".** Those screens
  show the actual error and a retry countdown; the dial keeps retrying with
  backoff and rescans the subnet every few minutes. Menu → Wi-Fi → Change
  network, Menu → Update → Check for updates and Settings → Factory reset
  all work while the dial is in this state. If the pad never answers,
  confirm its local API is enabled and that it is on the same 2.4 GHz
  network; `curl http://<pad-ip>:8080/api/state` from a laptop is the quick
  test.
- **Factory reset (from the dial):** Menu → Settings → Factory reset, tap
  twice within 3 seconds to confirm. This erases all stored Wi-Fi
  credentials, the pad address, timezone and preferences, and restarts the
  dial as if freshly flashed.
- **Recovering a bricked/misbehaving unit:** if the dial won't boot cleanly
  or a factory reset from Settings isn't reachable, the easy path is the
  [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/)
  again — it always erases the whole chip, which is exactly what you want
  here. The developer equivalent, from a checkout with ESP-IDF set up:
  ```bash
  idf.py -p <PORT> erase-flash flash
  ```
  This repository *is* the factory image for the dial's own ESP32-S3 — there
  is no separate stock firmware to restore it to. (`firmware/backups/` holds
  local flash backups made during hardware bring-up, but that backup is of
  the companion chip, not the dial's own flash — it isn't a path back to a
  "factory" dial image.) If you want to return the board to Waveshare's own
  stock demo instead of this project, see Waveshare's wiki for the
  `ESP32-S3-Knob-Touch-LCD-1.8` product.
- **Filing a bug report:** run `idf.py -p <PORT> monitor` while reproducing
  the issue and include the log output — most failures (Wi-Fi, the pad
  connection, update checks) log a specific reason on this console.

## Provenance

The hardware bring-up (`components/dial_display`, the three `*_bsp`
components, `main/main.c`'s panel init, `partitions.csv`,
`components/dial_display/user_config.h`) is derived from Waveshare's official
`ESP32-S3-Knob-Touch-LCD-1.8` ESP-IDF demo (`08_LVGL_Test` for display + touch
+ LVGL, `04_Encoder_Test` for the knob). Modifications for ESP-IDF **v6.0**:

- Flash set to **16 MB** (the demo shipped 8 MB; the board is 16 MB, and its
  8 MB `factory` partition overflowed 8 MB) — see `partitions.csv` for the
  dual-OTA layout this firmware uses instead.
- BSP component `CMakeLists.txt` files migrated off the removed catch-all
  `driver` component to the specific `esp_driver_i2c` / `esp_driver_gpio` /
  `esp_driver_ledc` / `esp_timer` components.
- Encoder pins (`EXAMPLE_ENCODER_ECA_PIN=8`, `ECB_PIN=7`) in
  `components/dial_display/user_config.h`.

The display uses the managed `esp_lcd_sh8601` QSPI driver (Waveshare drives
this panel via the SH8601 driver + a custom init sequence in `main.c`); touch
is the CST816 on I2C (SDA=GPIO11/SCL=GPIO12); the knob is `iot_knob` on
GPIO8/7.

Everything above that bring-up — Wi-Fi provisioning, the screen router and
every screen, haptics, power management, the OTA pipeline, timekeeping — is
inherited from Orion Dial, the project this firmware was forked from; none
of it comes from Waveshare's demo. `dial_somnus` (the pad client),
`dial_pad_discovery` (the subnet scan) and the Bed Mode / Pad Address /
Timezone settings were written for this fork.
