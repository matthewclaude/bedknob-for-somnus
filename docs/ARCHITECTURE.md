# Architecture

The dial is a single ESP32-S3 firmware image ([`firmware/dial-idf`](../firmware/dial-idf)).
There is no server, hub, account or cloud in the picture — the device joins
your Wi-Fi and talks plain HTTP to the Somnus Pad on the same LAN:

```
knob dial (ESP32-S3)  ──Wi-Fi──►  Somnus Pad, http://<pad-ip>:8080  (LAN)
                                    GET  /api/state
                                    POST /api/power
                                    POST /api/target_t
```

Those three endpoints are the pad's entire local API
([`reference/local_api.yml`](../reference/local_api.yml), v0.2.0) and the
only contract the firmware relies on. The dial's other two peers are
`pool.ntp.org` for the clock and GitHub for updates.

This doc is a contributor orientation to `main/main.c` and `components/`, not
a spec — read the source (especially the comment block at the top of
`main.c` and each component's header) for the load-bearing details. The
numbers below are the constants in the tree as of this writing.

## Two tasks and a store

Two tasks, deliberately kept from touching each other's data structures:

- **The LVGL task** (`dial_display`, priority 5, core 1) owns the display,
  touch, and the screen router (`dial_ui`: `ui_router` + one `scr_*.c` per
  screen). It renders from `dial_state` snapshots and never blocks on the
  network.
- **The worker task** (`worker_task` in `main.c`, priority 3, core 0 — below
  the LVGL task and on the Wi-Fi/lwIP core) is the single network task:
  Wi-Fi bring-up, then the pad connect loop, then the steady-state
  command/poll loop. It never touches LVGL.

They meet only through **`dial_state`**, a single `app_state_t` snapshot
behind a mutex. The worker commits changes with `dial_state_commit()`, which
bumps a generation counter; the router's dispatcher timer notices the change
and re-renders the active screen from a fresh copy. Knob detents flow the
other way — the encoder's `esp_timer` callback only feeds an atomic
accumulator that the dispatcher drains into the active screen, and every
input (knob or touch) stamps `dial_state_stamp_input()`, which the worker's
poll gate reads.

Screens talk to the worker through the **UI→worker command queue** in
`dial_state`: `xQueueCreate(16, …)`, and when it is full `dial_cmd_post()`
drops the **oldest** entry to make room for the newcomer (a knob spin must
end on its last value, not its first). Command kinds (`cmd_kind_t`):
`CMD_SET_TEMP` and `CMD_TOGGLE_ON` (the hot pair, coalesced per zone),
`CMD_WIFI_RESET`, `CMD_FACTORY_RESET`, `CMD_OTA_CHECK`, `CMD_OTA_APPLY`,
`CMD_OTA_CLEAR_FAILED`, `CMD_PAD_SETTINGS_CHANGED`, `CMD_TZ_CHANGED`.

Two smaller tasks exist off the hot paths: `dial_haptics` plays DRV2605
effects from its own queue-fed task so nobody blocks on I²C, and
`dial_pad_discovery` spawns and joins a few short-lived probe tasks while the
worker waits.

## The pad client: `dial_somnus`

`dial_somnus` is the **single, non-reentrant pad client**, called only from
the worker task. It has no session to keep: every call is one
`esp_http_client` request with a 5 s timeout against the persisted base URL
(Settings → Pad Address; the compiled fallback is only the spec's example
address and is never a real pad). `dial_somnus_connect()` is a real `GET
/api/state` used as a reachability probe; `dial_somnus_get_state()` parses
both sides plus the pad's `error` flag; `dial_somnus_set_temp()` and
`dial_somnus_set_power()` POST one side each. A failed read leaves the
last-known-good state untouched.

**Zone mode is a user setting because the API cannot report it.** The Somnus
app has a One Bed / Dual Sides toggle, but nothing in `/api/state` says which
mode the pad is in, and in One Bed mode the spec declares writes to `side1`
undefined (the pad mirrors `side0` to `side1` itself). So the dial keeps its
own Bed Mode preference (Settings → Bed Mode, NVS-backed via
`dial_state_get/set_zone_mode`), applies it with `dial_somnus_set_zone_mode()`,
and **in single-zone mode never writes `side1`** — `set_temp` and `set_power`
for `side1` return success without a network call, and that skip is not
reported as an error. Reads always fetch both sides; in One Bed mode `side1`
simply mirrors `side0`, which is harmless to read.

## Finding the pad: `dial_pad_discovery`

The pad advertises no mDNS or SSDP, so when the persisted address fails the
worker runs a **subnet scan** (`docs/SPEC-pad-discovery.md`). This is a
separate probe path, not `dial_somnus`: the client above is one instance and
non-reentrant, and the scan needs several probes in flight. Each probe is its
own short-lived `esp_http_client`, four at a time, against port 8080 on an
ordered candidate list (the failed address's neighbourhood, the dial's own
DHCP neighbourhood, the conventional static and DHCP ranges, then everything
else); `/24` or tighter only, 256 hosts max. Pass 1 uses a 300 ms timeout;
only if that finds nothing does pass 2 sweep again at 600 ms, so the worst
case is about 57.6 s. A hit is validated by decoding the response as the
pad's `/api/state` JSON, never by trusting a 200 alone. The first failure
after boot always scans; after that a 5-minute cooldown keeps an unreachable
pad from having its subnet swept on every retry. A found address is
persisted, and the connect loop picks it up on its next iteration.

## The worker's connect loop

After `dial_net_bringup()` returns with an IP and `dial_time_start()` has
kicked off SNTP, the worker loops until it can reach the pad:

1. Re-read the persisted pad URL (so a Settings change during the loop takes
   effect), set `PH_SOMNUS_CONNECTING`, and probe it with
   `dial_somnus_connect()`. Success breaks the loop.
2. On failure, if `dial_pad_discovery_should_attempt()` says so, set
   `PH_PAD_DISCOVERY` and run the scan. A hit is persisted and the loop
   retries immediately, skipping the backoff.
3. Otherwise set `PH_DEGRADED` with the pad's last error and call
   `backoff_wait()`: 5 s, doubling each round to a 60 s cap.

`backoff_wait()` is not a sleep. It publishes a countdown for the error
screen **and services the command queue** between attempts, which is what
makes Change network, Factory reset and Check for updates work while a dial
is stuck: `CMD_WIFI_RESET` and `CMD_FACTORY_RESET` write NVS and reboot;
the OTA commands run against `dial_ota` (Wi-Fi is up; the download can never
overlap the scan because both happen on this one task); `CMD_TZ_CHANGED`
applies a zone; `CMD_PAD_SETTINGS_CHANGED` cuts the wait short and resets the
backoff so the next probe uses the new address at once; `CMD_SET_TEMP` and
`CMD_TOGGLE_ON` are discarded, since there is no pad to write to and a
minutes-old knob position would be worse than nothing.

Once the probe succeeds the worker re-reads Bed Mode from the store (not a
capture taken before the loop), applies it to `dial_somnus`, seeds the pad's
fixed setpoint range (12.0–42.3 °C, from the spec; there is no discovery
call to report it), takes a first poll, and sets `PH_READY`.

## The steady-state loop

Each tick waits up to 300 ms on the command queue, then decides whether to
poll.

- **Commands.** A rare command (anything but the hot pair) is handled at
  once by `handle_immediate_cmd()`, and the poll is marked due immediately
  with the fast-confirm count re-armed. A burst of `CMD_SET_TEMP` /
  `CMD_TOGGLE_ON` is coalesced per zone — at most one net power change and
  the final temperature — before being written to the pad.
- **Poll cadence.** `GET /api/state` runs at most every **10 s** when idle
  (`POLL_INTERVAL_US`). Right after any write it runs every **2 s** for
  **3** rounds (`POLL_CONFIRM_US`, `POLL_CONFIRM_N`), because the bed takes a
  few seconds to actually start heating or cooling and a 10 s cadence would
  insist nothing is happening. Both cadences sit behind a quiet gate: no
  poll runs until there has been no user input for **2.5 s**
  (`KNOB_SETTLE_US`), so a read can never land mid-spin.
- **Failure handling.** Wi-Fi down sets `PH_WIFI_LOST` and waits for
  `dial_net`'s own reconnect. Three consecutive failed polls set
  `PH_DEGRADED`; the next good poll sets `PH_READY` again. Neither leaves
  the steady-state loop, and no state requires a reboot to escape.
- **Housekeeping on the same tick:** the 21:00–07:00 night-mode flip, the
  standby/backlight level from `dial_power`, the 6-hourly automatic update
  check, the auto-update window (below), and clearing a stale OTA failure
  after about 25 s so the Update row never wedges.

A poll result is committed through a snapshot that also remembers when the
request started; a poll that predates the user's latest input is not allowed
to overwrite the optimistic value the face is already showing.

## Phases

`app_state_t.phase` (`conn_phase_t` in `dial_state.h`) drives both the UI
router and the worker's state machine: `PH_BOOT`, `PH_WIFI_CONNECTING`,
`PH_WIFI_PORTAL` (SoftAP + captive portal up), `PH_WIFI_LOST`,
`PH_SOMNUS_CONNECTING` (probing the persisted address), `PH_PAD_DISCOVERY`
(the scan is running — a real, long phase, not a blip), `PH_READY`, and
`PH_DEGRADED` (network up, pad calls failing, retrying with backoff). Every
failure is a phase plus a backoff, never a dead end.

## The UI task and `nav_policy`

The router (`ui_router.h`) creates one LVGL screen per view on enter and
destroys it on exit, driven by a small vtable per screen (`create`,
`destroy`, `on_state`, `on_knob`, `on_gesture`). Every router entry point
runs in the LVGL task; screens never take the LVGL lock themselves.

`nav_policy()` in `main.c` is the one function that decides which screen the
current state demands, consulted by the dispatcher on every state change. In
outline: a fresh device shows the welcome splash before Wi-Fi is up;
`PH_WIFI_PORTAL` shows the portal instructions (with the on-device network
picker and password wheel reachable from it); `PH_PAD_DISCOVERY` has its own
live-progress screen; the other pre-ready phases show the connecting or
error screen with the retry countdown. An OTA download takes the whole
screen over regardless of phase. Two setup gates fire only at `PH_READY`:
the **timezone picker** when no zone has ever been applied (an iPhone-
provisioned dial has none — `docs/SPEC-timezone-source.md`), and the
**side pick** on a fresh Dual Sides dial. Once a user is on a screen they
chose deliberately (Settings, the update sheet, a picker), a routine poll
landing is not allowed to yank them off it.

Screens in the tree: connecting, pad discovery, Wi-Fi portal, network pick,
passkey, dial, menu, standby clock, error, welcome, side pick, settings,
timezone, pad address, adjust mode, brightness menu and picker, Wi-Fi,
about, update, updating, update prompt.

## Updates: `dial_ota`

`dial_ota` checks the **public releases repo**
`matthewclaude/somnus-dial-releases` over the GitHub API — `/releases/latest`
normally; with **Beta builds** on, the `/tags?per_page=50` list to find the
newest `somnus-v*` tag and then `/releases/tags/<tag>` for that release's
object, so prereleases count and a beta can never scroll out of a capped list
(the `0.1.5` fix) — compares the tag (prefix `somnus-v`) against the running
`esp_app_get_description()->version` semver-aware, and on a newer release
records the `somnus-dial.bin` asset URL. Applying is `esp_https_ota` into the
inactive OTA slot, following GitHub's redirect to
`objects.githubusercontent.com`; TLS on both hosts verifies against an embedded
multi-root PEM. The bootloader's rollback is enabled
(`CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE`): a freshly installed image stays
provisional until the worker's first successful pad poll, or a 30 s stable-boot
fallback timer, marks it valid; otherwise the next reset reverts.

The automatic check runs every 6 hours. Discovery is deliberately split: an
ambient "Update available" line on the dial and standby faces, and a one-tap
prompt sheet raised at most once a day and only on a wake edge. **Auto-update**
(off by default) installs unattended in a fixed 09:00–11:00 window, only at
`PH_READY`, outside night mode, after 30 minutes without input, at most once
per window, and backs off a version that has failed twice.
`docs/SPEC-update-prompt.md` and `docs/SPEC-ota-readiness.md` are the design
records.

## Time: `dial_time`

`dial_time_start()` starts `esp_netif_sntp` against `pool.ntp.org` and
restores the persisted POSIX TZ rule. Zones are set by IANA name
(`dial_time_set_iana_tz()`), resolved through an **embedded copy of the
posix_tz_db zones table** (about 400 zones) to a POSIX rule that is applied
with `setenv("TZ")`/`tzset()` and persisted in NVS namespace `time` as both
`posix_tz` and `iana_tz`. The name arrives either from the Wi-Fi setup page
(the phone's browser fills a hidden field) or from the dial's own picker,
which offers a short curated list (`DIAL_TZ_IANA` in `dial_state.h`) because
400 rows is not a knob-scrollable list. Because `setenv("TZ")` is global libc
state that the worker reads on every tick, a zone picked on the LVGL task is
routed through `CMD_TZ_CHANGED` and applied on the worker.

## Where state lives

Everything persists to **NVS**, split by namespace:

| NVS namespace | Owner | Holds |
|---|---|---|
| `wifi` | `dial_net` | SSID/password, the "setup requested" flag |
| `ui` | `dial_state` | Pad address (`pad_url`), Bed Mode (`pad_1zone`), side (`zone`), °F/°C (`units`), temperature scale (`relmode`), rotation (`rot`), adjustment mode (`sched_follow`), haptics level, the three brightness levels (`bri_day2` / `bri_nite2` / `bri_nclk`), screen timeout (`scr_to`), and the update prefs (`beta`, `ota_auto`, `ota_defer`, `ota_skip`, `ota_shown`) |
| `haptics` | `dial_haptics` | DRV2605 autocal results, so the driver skips recalibration on every boot |
| `time` | `dial_time` | `posix_tz` and `iana_tz` |

Factory reset erases all of NVS and reboots into the Wi-Fi portal as a fresh
device.

## Temperature units

The pad's wire is **°C** with fractional targets allowed (spec range
12.0–42.3). The dial carries setpoints internally in **tenths of °C**
(`temp_dc`), steps one whole degree per detent to match the Somnus app's
levels, and renders either as a real temperature (°F or °C) or as the app's
−15…+15 level scale (`docs/SPEC-somnus-relative-scale.md`). Conversion to
°F, where shown, happens only at the display boundary.
