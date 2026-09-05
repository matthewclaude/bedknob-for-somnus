# REPORT — Standby face, commit 2: the setting (Temperature / Clock), Night standby rename

Date: 2026-09-05 18:48 CDT (capture named `2026-09-06-…` per the task). Branch `main`. Spec: `docs/SPEC-standby-face.md` §3, §4, §4b, §6 commit 2. Toolchain ESP-IDF v6.0; simulator via `cmake --build build`.

## Verdict

**DONE — `9dad0b2`: pref `ui/sb_face` (u8, default 1 = Temperature on fresh device and on upgrade, clamp-on-read to {0,1} → 1), `dial_state_get/set_standby_face()` in night_face's shape, `SCR_STANDBY_FACE` + `scr_standby_face.c` (Back / Temperature / Clock, checkmark, tap sets and returns), the "Standby face" row directly under Screen timeout, `standby_screen()` reading the pref, `SCR_STANDBY_FACE` in both sticky lists, the §4b "Night (clock)" → "Night (standby)" rename (label + caption only, pref untouched) with the 0 % comment, and the two comment fixes. `idf.py fullclean && idf.py build` with zero compiler warnings (35 `main`/`dial_ui`/`dial_state` objects), wire-flashed, 40 s boot capture clean with the expected `dial_state: sb_face: no key -> default 1` line. Simulator: 49 screens; 3 changed (the three brightness renders the rename touches), 2 new, all 44 others byte-identical.** No tag, no version bump, no push. Nothing in dial_power, nothing in Screen timeout's choices, no new standby layout. One pre-existing gap found and left for the owner (Findings).

## Gate

```
$ git --no-optional-locks status --short -uall
(empty)
$ git log --oneline -4
93c0f9c docs: SPEC-standby-face — commit 1 verified on hardware; §4b Night clock row rename
2ad672a docs: SPEC-standby-face default Temperature (owner ruling); commit 1 report
03e6259 feat(standby): standby_screen() helper; dial face at STANDBY (hard-wired, spec §6 commit 1)
1f04499 docs: report
$ grep -c 'Night standby' docs/SPEC-standby-face.md
2
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.6-beta.1")
```

All four checks passed; proceeded.

## Git

```
$ git diff --stat 93c0f9c 9dad0b2
 docs/screens/brightness-clock-off.png              | Bin 18599 -> 19139 bytes
 docs/screens/brightness-clock.png                  | Bin 21383 -> 21921 bytes
 docs/screens/brightness-menu.png                   | Bin 19828 -> 20139 bytes
 docs/screens/settings-standby-face.png             | Bin 0 -> 22595 bytes
 docs/screens/standby-face.png                      | Bin 0 -> 17468 bytes
 .../dial-idf/components/dial_state/dial_state.c    |  46 ++++-
 .../dial-idf/components/dial_state/dial_state.h    |  24 +++
 .../dial-idf/components/dial_ui/CMakeLists.txt     |   2 +-
 .../dial-idf/components/dial_ui/scr_brightness.c   |  35 ++--
 .../components/dial_ui/scr_brightness_menu.c       |  33 ++--
 .../dial-idf/components/dial_ui/scr_settings.c     |  25 ++-
 .../dial-idf/components/dial_ui/scr_standby_face.c | 213 +++++++++++++++++++++
 .../dial-idf/components/dial_ui/scr_updating.c     |   4 +-
 firmware/dial-idf/components/dial_ui/ui_router.h   |   3 +-
 firmware/dial-idf/components/dial_ui/ui_screens.c  |   1 +
 .../components/dial_ui/ui_screens_internal.h       |   1 +
 firmware/dial-idf/main/main.c                      |  26 +--
 simulator/CMakeLists.txt                           |   1 +
 simulator/main.c                                   |  44 ++++-
 simulator/sim_state.c                              |  10 +-
 20 files changed, 418 insertions(+), 50 deletions(-)
$ git log --oneline -2
9dad0b2 feat(standby): Standby face setting (Temperature/Clock, default Temperature); Night clock row renamed Night standby
93c0f9c docs: SPEC-standby-face — commit 1 verified on hardware; §4b Night clock row rename
```

Commit: **`9dad0b264a17cfd9cd4c2ec68769fa1ffb9e1045`** — code, simulator and PNGs together, as specified. Working tree after: only this report untracked. PROJECT_VER untouched.

### What each file got

- **`dial_state.h`** — `uint8_t standby_face` in `app_state_t` next to `night_face_min` with the default/NVS/clamp contract in its comment; `DIAL_SB_FACE_CLOCK 0` / `DIAL_SB_FACE_TEMP 1`; `dial_state_get_standby_face()` / `dial_state_set_standby_face()` declared with night_face's threading note.
- **`dial_state.c`** — `clamp_standby_face()` (line 102: `raw <= 1 ? raw : 1`); init default `DIAL_SB_FACE_TEMP` (154); NVS read of `"sb_face"` (228–229) with a one-line log of which way it went (245–246); `have_sb_face` joined the "nothing stored → keep init defaults" condition (249); clamp-on-apply (330); getter/setter (668–690) — mutex mutate, `generation++`, immediate `nvs_set_u8` + `nvs_commit`, the setter clamps too. `esp_log.h` + a `TAG` were added to this file (it had no logging before).
- **`main.c`** — `standby_screen()` (158–161) returns `SCR_STANDBY` for Clock, `SCR_DIAL` otherwise; the "hard-wired" comment is gone; `SCR_STANDBY_FACE` added to the have_state sticky set (403–406) and the no-state "never trap the user" set (433–437).
- **`ui_router.h`** — `SCR_STANDBY_FACE` after `SCR_NIGHT_FACE`; the `SCR_STANDBY` entry now says it is what STANDBY shows only while Standby face is Clock.
- **`ui_screens.c` / `ui_screens_internal.h` / `dial_ui/CMakeLists.txt` / `simulator/CMakeLists.txt`** — registration, extern, and both source lists.
- **`scr_standby_face.c`** (new, 213 lines) — `scr_night_face.c` with the names swapped: rows Back / Temperature / Clock, title "STANDBY FACE", cursor seeded on the current value, checkmark from `st->standby_face`, no-op reselect still exits, right-swipe back.
- **`scr_settings.c`** — header paragraph for the row (60–65); `s_val_standby_face` (107); `row_standby_face_cb` (335) → `SCR_STANDBY_FACE`; the row created right after Screen timeout (473); nulled in `destroy()` (534); value "Temperature"/"Clock" in `on_state` (583–584). Screen timeout's own comment no longer says the standby face is the clock.
- **`scr_brightness_menu.c`** — row label `"Night (clock)"` → `"Night (standby)"` (155); header table entry and the 2026-08-05 prefix note reworded; the row callback carries the §4b comment: 0 % means the standby face is off whichever face is chosen. `bri_night_clock_pct`, `"bri_nclk"`, range 0–100 and the default are untouched.
- **`scr_brightness.c`** — caption `"NIGHT (CLOCK)"` → `"NIGHT (STANDBY)"` (354) plus the five comments that said clock; the caption-offset note now records that "NIGHT (STANDBY)" (~150 px) is the widest caption and still under the 155 px the offset was tuned for.
- **`scr_updating.c`** — header no longer says nav_policy "would force SCR_STANDBY"; it names the chosen standby face.
- **Simulator** — `sim_state.c` stubs for the pair plus the default; `main.c` gains `scenario_standby_face` (the picker) and `scenario_settings_standby_face` (Settings knob-walked +4 onto the new row); the two knob-walk scenarios that count rows below Screen timeout moved one detent (+9 → +10 for Pad Address, +8 → +9 for Timezone) so their frames are unchanged; the row-order comment updated.

## The two questions (spec §4), with line numbers

**(a) Changeable from the state the device is in — yes.** The row is created unconditionally in `scr_settings.c` `create()` at line 473, right after Screen timeout (472), with no `sync_*` gate (unlike Night face at 452, which exists only while `night_on`). Its tap (335–340) is plain navigation to `SCR_STANDBY_FACE`. Reachability: `SCR_SETTINGS` is sticky in `main.c`'s have_state set (403) and in the no-state "never trap the user" set (433), so Settings is reachable at steady state and inside the connect loop; `SCR_STANDBY_FACE` itself is in both sets (406, 437), so a poll or a phase flap landing while the picker is open returns `cur` and cannot yank the user off it. The setter (`dial_state.c` 676–690) is called straight from the picker in the LVGL task, no queue, and persists immediately.

**(b) Read by something — yes, exactly one consumer.** `main.c` line 160, `standby_screen()`: `st->standby_face == DIAL_SB_FACE_CLOCK ? SCR_STANDBY : SCR_DIAL`, called at both STANDBY sites (380, 416). The setter's `generation++` (`dial_state.c` 681) is what makes `ui_router.c`'s dispatcher re-run `nav_policy`, so the next tick after a tap already routes by the new value. The only other readers are presentation: `scr_settings.c` 584 (the row's value) and `scr_standby_face.c` (checkmark, cursor seed). Nothing else needs to — dial_power, OTA, the update prompt, night dimming all read the tier.

## Simulator — raw output and the changed-PNG count

`cmake --build build && ./build/dial_sim` (second run, after adding `scenario_settings_standby_face`):

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 86%] Built target lvgl_examples
[100%] Built target lvgl_demos
BUILD_EXIT=0
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote sidepick         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/sidepick.png (5 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (3 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (4 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (1 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (4 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (6 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (5 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (5 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (5 distinct colors sampled)
done: 49 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
RUN_EXIT=0
```

`git status docs/screens` after the run: **3 changed, 2 new, 44 byte-identical** (49 total).

| PNG | Why |
|---|---|
| `brightness-menu.png` (M) | row label "Night (standby)" |
| `brightness-clock.png` (M) | caption "NIGHT (STANDBY)" |
| `brightness-clock-off.png` (M) | same caption, 0 % / Off render |
| `standby-face.png` (new) | the picker: Back / **Temperature ✓** / Clock, title STANDBY FACE |
| `settings-standby-face.png` (new) | Settings knob-walked +4: "Screen timeout — 1m" above, **"Standby face — Temperature"** focused, "Scale — Absolute" below |

`settings.png` is byte-identical: its frame opens on Brightness with the top four rows in view, and the new row (index 5) is below the fold — which is why the knob-walked scenario exists. `settings-pad.png` and `settings-timezone-raw.png` are byte-identical because their detent counts were bumped with the row shift. No `standby-temperature` scenario: the simulator's `dial_power` stub always returns `DPWR_ACTIVE` and `main.c` (hence `nav_policy`) is not linked, so the STANDBY-tier routing is hardware-only, as the spec allows.

Eyeballed: `standby-face.png`, `brightness-menu.png`, `brightness-clock.png`, `settings-standby-face.png` — all as described above.

## `idf.py fullclean && idf.py build` — raw tail (last 40 lines)

Exit 0.

```
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_20.c.obj
[100%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
[100%] Linking C static library libmain.a
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
NOTE: /Users/matthew/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: 
BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 
'n' are allowed). Value is treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 
'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is 
treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:235: 
FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value (only 'y' and 'n' are 
allowed). Value is treated as 'n'.
[100%] Built target __ldgen_output_sections.ld
[100%] Building C object CMakeFiles/somnus-dial.elf.dir/project_elf_src_esp32s3.c.obj
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189780 bytes. Smallest app partition is 0x400000 bytes. 0x276880 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
or
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x19000 build/ota_data_initial.bin 0x20000 build/somnus-dial.bin
or from the "/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build" directory
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash "@flash_args"
```

Every line containing "warning" (case-insensitive):

```
4: CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
11: This warning is for project developers.  Use -Wno-author or -Wno-deprecated
```

That is CMake's pre-existing deprecation note about `cmake_minimum_required` in the project `CMakeLists.txt`. Compiler warnings: **none** — from `main`, `dial_ui`, `dial_state`, or anywhere else. App descriptor: App version **0.1.6-beta.1**, Compile time **Sep  5 2026 18:46:18**.

## Port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

Native USB-Serial/JTAG port only — no "flip the plug".

## `idf.py -p /dev/cu.usbmodem83401 flash` — raw output

Exit 0. Nothing recompiled by the flash step (no `Building C object` lines). Warning lines beyond the same CMake note: none. From `Executing action: flash`; esptool's terminal control sequences and in-place progress rewrites stripped.

```
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target blank_ota_data
[  0%] Built target partition_table_bin
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Performing build step for 'bootloader'
[  1%] Built target __idf_esp_https_ota
[  1%] Built target __idf_esp_http_server
[  2%] Built target bootloader_ld_in_preprocess
[  2%] Built target _project_elf_src
[  8%] Built target __idf_log
[  1%] Built target __idf_esp_http_client
[ 16%] Built target __idf_esp_rom
[  1%] Built target __idf_tcp_transport
[ 18%] Built target __idf_esp_common
[  1%] Built target __idf_esp_driver_i2s
[ 25%] Built target __idf_esp_hw_support
[  2%] Built target __idf_esp_adc
[ 26%] Built target __idf_esp_system
[  2%] Built target __idf_esp-tls
[ 32%] Built target __idf_efuse
[  2%] Built target __idf_http_parser
[ 47%] Built target __idf_bootloader_support
[  2%] Built target __idf_esp_hal_i2c
[ 50%] Built target __idf_esp_security
[  2%] Built target __idf_esp_gdbstub
[ 52%] Built target __idf_esp_hal_security
[  2%] Built target __idf_esp_wifi
[ 56%] Built target __idf_esp_hal_ana_conv
[  2%] Built target __idf_esp_coex
[ 59%] Built target __idf_esp_hal_uart
[ 62%] Built target __idf_esp_hal_wdt
[  7%] Built target __idf_wpa_supplicant
[ 64%] Built target __idf_esp_hal_timg
[  8%] Built target __idf_esp_netif
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[ 14%] Built target __idf_lwip
[ 14%] Built target __idf_vfs
[ 71%] Built target __idf_esp_hal_gpspi
[ 14%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 14%] Built target __idf_esp_phy
[ 76%] Built target __idf_esp_hal_pmu
[ 15%] Built target __idf_nvs_flash
[ 79%] Built target __idf_esp_hal_usb
[ 16%] Built target __idf_nvs_sec_provider
[ 84%] Built target __idf_esp_hal_gpio
[ 17%] Built target __idf_esp_event
[ 88%] Built target __idf_hal
[ 18%] Built target __idf_esp_driver_uart
[ 93%] Built target __idf_soc
[ 18%] Built target __idf_esp_psram
[ 95%] Built target __idf_xtensa
[ 18%] Built target __idf_esp_ringbuf
[ 97%] Built target __idf_main
[ 19%] Built target __idf_esp_timer
[ 98%] Built target bootloader.elf
[ 20%] Built target __idf_cxx
[100%] Built target gen_bootloader_binary
[ 20%] Built target __idf_pthread
[100%] Built target gen_project_binary
[ 21%] Built target __idf_esp_libc
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 23%] Built target __idf_freertos
[100%] Built target bootloader_check_size
[100%] Built target app
[ 26%] Built target __idf_esp_hw_support
[ 26%] No install step for 'bootloader'
[ 27%] Built target __idf_esp_hal_i2s
[ 27%] Completed 'bootloader'
[ 27%] Built target __idf_esp_hal_touch_sens
[ 28%] Built target bootloader
[ 28%] Built target __idf_esp_hal_pmu
[ 28%] Built target __idf_esp_hal_usb
[ 29%] Built target __idf_esp_hal_gpio
[ 30%] Built target __idf_soc
[ 31%] Built target __idf_heap
[ 32%] Built target __idf_log
[ 32%] Built target __idf_hal
[ 33%] Built target __idf_esp_rom
[ 33%] Built target __idf_esp_common
[ 35%] Built target __idf_esp_system
[ 36%] Built target __idf_spi_flash
[ 37%] Built target __idf_bootloader_support
[ 38%] Built target __idf_esp_hal_ana_conv
[ 38%] Built target __idf_esp_hal_wdt
[ 38%] Built target __idf_esp_hal_timg
[ 41%] Built target tfpsacrypto
[ 42%] Built target mbedtls
[ 43%] Built target mbedx509
[ 47%] Built target builtin
[ 47%] Built target p256m
[ 48%] Built target everest
[ 48%] Built target __idf_esp_driver_dma
[ 48%] Built target __idf_esp_mm
[ 49%] Built target __idf_esp_pm
[ 50%] Built target __idf_esp_hal_uart
[ 50%] Built target __idf_esp_driver_gpio
[ 50%] Built target __idf_esp_security
[ 51%] Built target __idf_efuse
[ 51%] Built target __idf_esp_partition
[ 51%] Built target __idf_app_update
[ 51%] Built target __idf_esp_bootloader_format
[ 51%] Built target __idf_esp_app_format
[ 52%] Built target __idf_esp_hal_security
[ 53%] Built target __idf_esp_hal_mspi
[ 53%] Built target __idf_esp_hal_clock
[ 53%] Built target __idf_esp_hal_gpspi
[ 53%] Built target __idf_esp_hal_dma
[ 54%] Built target __idf_esp_stdio
[ 55%] Built target __idf_xtensa
[ 55%] Built target __idf_dial_knob
[ 55%] Built target __idf_dial_state
[ 56%] Built target __idf_espressif__cjson
[ 56%] Built target __idf_esp_hal_ledc
[ 56%] Built target __idf_dial_time
[ 56%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_esp_driver_i2c
[ 57%] Built target __idf_esp_hal_lcd
[ 57%] Built target __idf_unity
[ 57%] Built target __idf_esp_driver_spi
[ 58%] Built target __idf_esp_driver_gptimer
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_hal_mcpwm
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_hal_rmt
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_esp_hal_pcnt
[ 59%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_esp_driver_twai
[ 60%] Built target __idf_esp_driver_touch_sens
[ 61%] Built target __idf_esp_hid
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_esp_https_server
[ 62%] Built target __idf_protobuf-c
[ 62%] Built target __idf_perfmon
[ 62%] Built target __idf_rt
[ 63%] Built target __idf_spiffs
[ 64%] Built target __idf_esp_lcd
[ 64%] Built target __idf_driver
[ 64%] Built target __idf_esp_driver_ledc
[ 64%] Built target __idf_dial_ota
[ 64%] Built target __idf_dial_net
[ 64%] Built target __idf_cmock
[ 64%] Built target __idf_dial_somnus
[ 65%] Built target __idf_esp_driver_rmt
[ 65%] Built target __idf_esp_driver_sd_intf
[ 66%] Built target __idf_esp_driver_cam
[ 67%] Built target __idf_esp_driver_mcpwm
[ 68%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_esp_driver_pcnt
[ 69%] Built target __idf_protocomm
[ 69%] Built target __idf_espressif__esp_lcd_sh8601
[ 70%] Built target __idf_dial_pad_discovery
[ 70%] Built target __idf_esp_driver_sdmmc
[ 71%] Built target __idf_esp_local_ctrl
[ 71%] Built target __idf_fatfs
[ 96%] Built target __idf_lvgl__lvgl
[ 96%] Built target __idf_dial_display
[ 96%] Built target __idf_lcd_bl_pwm_bsp
[ 96%] Built target __idf_lcd_touch_bsp
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Built target __ldgen_output_sections.ld
[100%] Built target somnus-dial.elf
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189780 bytes. Smallest app partition is 0x400000 bytes. 0x276880 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app
esptool --chip esp32s3 -p /dev/cu.usbmodem83401 -b 460800 --before=default-reset --after=hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x19000 ota_data_initial.bin 0x20000 somnus-dial.bin
esptool v5.3.1
Serial port /dev/cu.usbmodem83401:
Connecting...
Connected to ESP32-S3 on /dev/cu.usbmodem83401:
Chip type:          ESP32-S3 (QFN56) (revision v0.2)
Features:           Wi-Fi, BT 5 (LE), Dual Core + LP Core, 240MHz, Embedded PSRAM 8MB (AP_3v3)
Crystal frequency:  40MHz
USB mode:           USB-Serial/JTAG
MAC:                28:84:85:4b:d9:04

Uploading stub flasher...
Running stub flasher...
Stub flasher running.
Changing baud rate to 460800...
Changed.

Configuring flash size...

Writing 'bootloader/bootloader.bin' at 0x00000000...
SHA digest in image updated.
Flash will be erased from 0x00000000 to 0x00005fff...
Compressed 22576 bytes to 14461...

Writing at 0x00000000 [                              ]   0.0% 0/14461 bytes... 
Writing at 0x00005830 [==============================] 100.0% 14461/14461 bytes... 
Wrote 22576 bytes (14461 compressed) at 0x00000000 in 0.2 seconds (729.6 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (687.9 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.0 seconds (1524.6 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1611648 bytes to 984038...

Writing at 0x00020000 [                              ]   0.0% 0/984038 bytes... 
Writing at 0x0002b8f7 [                              ]   1.7% 16384/984038 bytes... 
Writing at 0x0003857a [                              ]   3.3% 32768/984038 bytes... 
Writing at 0x0004b6c5 [>                             ]   5.0% 49152/984038 bytes... 
Writing at 0x00053201 [>                             ]   6.7% 65536/984038 bytes... 
Writing at 0x0005ba1c [=>                            ]   8.3% 81920/984038 bytes... 
Writing at 0x0006527d [=>                            ]  10.0% 98304/984038 bytes... 
Writing at 0x00070d09 [==>                           ]  11.7% 114688/984038 bytes... 
Writing at 0x00083aa9 [==>                           ]  13.3% 131072/984038 bytes... 
Writing at 0x0008a7e5 [===>                          ]  15.0% 147456/984038 bytes... 
Writing at 0x00093ba0 [===>                          ]  16.6% 163840/984038 bytes... 
Writing at 0x0009c1d5 [====>                         ]  18.3% 180224/984038 bytes... 
Writing at 0x000a47ff [====>                         ]  20.0% 196608/984038 bytes... 
Writing at 0x000aa10b [=====>                        ]  21.6% 212992/984038 bytes... 
Writing at 0x000af990 [=====>                        ]  23.3% 229376/984038 bytes... 
Writing at 0x000b4e3b [======>                       ]  25.0% 245760/984038 bytes... 
Writing at 0x000bad11 [======>                       ]  26.6% 262144/984038 bytes... 
Writing at 0x000c01a1 [=======>                      ]  28.3% 278528/984038 bytes... 
Writing at 0x000c5bac [=======>                      ]  30.0% 294912/984038 bytes... 
Writing at 0x000cabe7 [========>                     ]  31.6% 311296/984038 bytes... 
Writing at 0x000d015c [========>                     ]  33.3% 327680/984038 bytes... 
Writing at 0x000d63b0 [=========>                    ]  35.0% 344064/984038 bytes... 
Writing at 0x000dba48 [=========>                    ]  36.6% 360448/984038 bytes... 
Writing at 0x000e1482 [==========>                   ]  38.3% 376832/984038 bytes... 
Writing at 0x000e653e [==========>                   ]  40.0% 393216/984038 bytes... 
Writing at 0x000eb9b8 [===========>                  ]  41.6% 409600/984038 bytes... 
Writing at 0x000f10b6 [===========>                  ]  43.3% 425984/984038 bytes... 
Writing at 0x000f6dd1 [============>                 ]  45.0% 442368/984038 bytes... 
Writing at 0x000fbfa3 [============>                 ]  46.6% 458752/984038 bytes... 
Writing at 0x00101308 [=============>                ]  48.3% 475136/984038 bytes... 
Writing at 0x001062a0 [=============>                ]  49.9% 491520/984038 bytes... 
Writing at 0x0010b3e8 [==============>               ]  51.6% 507904/984038 bytes... 
Writing at 0x00110d1c [==============>               ]  53.3% 524288/984038 bytes... 
Writing at 0x00116385 [===============>              ]  54.9% 540672/984038 bytes... 
Writing at 0x0011ba78 [===============>              ]  56.6% 557056/984038 bytes... 
Writing at 0x00120f57 [================>             ]  58.3% 573440/984038 bytes... 
Writing at 0x00126436 [================>             ]  59.9% 589824/984038 bytes... 
Writing at 0x0012bf32 [=================>            ]  61.6% 606208/984038 bytes... 
Writing at 0x001313e9 [=================>            ]  63.3% 622592/984038 bytes... 
Writing at 0x001365bc [==================>           ]  64.9% 638976/984038 bytes... 
Writing at 0x0013b6eb [==================>           ]  66.6% 655360/984038 bytes... 
Writing at 0x00140f31 [===================>          ]  68.3% 671744/984038 bytes... 
Writing at 0x001466cd [===================>          ]  69.9% 688128/984038 bytes... 
Writing at 0x0014bbfa [====================>         ]  71.6% 704512/984038 bytes... 
Writing at 0x00151108 [====================>         ]  73.3% 720896/984038 bytes... 
Writing at 0x00156972 [=====================>        ]  74.9% 737280/984038 bytes... 
Writing at 0x0015c33a [=====================>        ]  76.6% 753664/984038 bytes... 
Writing at 0x00161427 [======================>       ]  78.3% 770048/984038 bytes... 
Writing at 0x00166660 [======================>       ]  79.9% 786432/984038 bytes... 
Writing at 0x0016bf3a [=======================>      ]  81.6% 802816/984038 bytes... 
Writing at 0x001711cf [=======================>      ]  83.2% 819200/984038 bytes... 
Writing at 0x0017632e [========================>     ]  84.9% 835584/984038 bytes... 
Writing at 0x0017b6b3 [========================>     ]  86.6% 851968/984038 bytes... 
Writing at 0x0018145c [=========================>    ]  88.2% 868352/984038 bytes... 
Writing at 0x00186a7a [=========================>    ]  89.9% 884736/984038 bytes... 
Writing at 0x0018d6ad [==========================>   ]  91.6% 901120/984038 bytes... 
Writing at 0x001929e1 [==========================>   ]  93.2% 917504/984038 bytes... 
Writing at 0x00197dd8 [===========================>  ]  94.9% 933888/984038 bytes... 
Writing at 0x0019e390 [===========================>  ]  96.6% 950272/984038 bytes... 
Writing at 0x001a3844 [============================> ]  98.2% 966656/984038 bytes... 
Writing at 0x001a925f [============================> ]  99.9% 983040/984038 bytes... 
Writing at 0x001a9780 [==============================] 100.0% 984038/984038 bytes... 
Wrote 1611648 bytes (984038 compressed) at 0x00020000 in 10.4 seconds (1242.7 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

## Boot capture

Raw pyserial read at 115200 for 40 s from 18:47:20 CDT, right after the flash returned. File **`bench-logs/2026-09-06-flash-standby-face-c2.log`** (6741 bytes, 126 lines; gitignored). Starts mid-bootloader (segment 3) as before. Quiet boot — no knob or touch activity.

```
I (379) boot: Loaded app from partition at offset 0x20000
I (380) boot: Set actual ota_seq=1 in otadata[0]
I (782) app_init: Project name:     somnus-dial
I (786) app_init: App version:      0.1.6-beta.1
I (790) app_init: Compile time:     Sep  5 2026 18:46:18
I (800) app_init: ESP-IDF:          v6.0
I (1209) ota: boot pending-verify: false
I (1328) dial_state: sb_face: no key -> default 1
I (5285) dial_somnus: zone mode set to single (One Bed)
I (5286) app: pad connected at http://192.168.1.169:8080
I (5349) ota: boot not pending verification (state 2) -- nothing to do
```

- App version **0.1.6-beta.1**, compile time **Sep  5 2026 18:46:18**; booted from the app at 0x20000; pad connected at 5.3 s.
- **`I (1328) dial_state: sb_face: no key -> default 1`** — the NVS read line the task asked for, exactly the expected form: this dial predates the key, so Temperature applies without writing anything.
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (5285) dial_somnus: zone mode set to single (One Bed) (the only case-insensitive `e (` hit is "single (One Bed)").
- `rst:` lines: **0** — no reboot.
- `W (` lines, none in the requested pattern set:

```
W (1197) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1391) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

## Deviations from the spec

1. **The row is "Night (standby)", not the bare "Night standby".** The row it replaces was `"Night (clock)"`, not "Night clock": `scr_brightness_menu.c`'s header records the owner's 2026-08-05 ruling that both night rows carry the shared `Night (…)` prefix so neither reads as the umbrella. Swapping only the qualifier keeps that pairing with "Night (in use)"; the caption follows as "NIGHT (STANDBY)". The words the user reads are the spec's.
2. **Generation bump instead of a `dial_state_commit()` call.** Item 1 says "dial_state_commit so the next nav_policy run sees it"; the pattern the task says to copy (night_face) deliberately does not use the wrapper — its header comment says so — and bumps `generation` directly under the mutex, which is the same signal `ui_router.c`'s dispatcher watches. Same effect; copied as instructed.
3. **`SCR_NIGHT_FACE` is listed nowhere in nav_policy's sticky sets**, so "add SCR_STANDBY_FACE wherever SCR_NIGHT_FACE is listed" had no anchor; it was added to both sets alongside `SCR_TIMEZONE` / `SCR_ADJUST_MODE` (the Settings-sub-screen category), which is the instruction's stated purpose. See Findings 1 for the gap that exposed.
4. **Second Settings render added rather than changing `settings`.** The spec says "re-render `settings` (new row visible)"; the row is off-frame in that scenario's own view, so `settings.png` is unchanged and `settings-standby-face.png` is the render that shows it. Changing `settings` itself to knob-walk would have churned a reference render other reports cite.
5. **`esp_log.h` and a `TAG` added to `dial_state.c`** for the requested NVS read line — the component had never logged before.
6. **Commit message trailers** appended after the verbatim title.
7. **Capture filename date** `2026-09-06-…` as instructed; machine clock read 2026-09-05 18:47 CDT.

Everything else: none.

## Findings for the owner (not changed here)

1. **`SCR_NIGHT_FACE`, `SCR_NIGHT_MODE`, and (in the have_state set) `SCR_BRIGHTNESS` are not sticky in `nav_policy`.** `main.c` 403–406 lists Settings, Brightness menu, Adjust mode, Pad Address, Timezone, and now Standby face; 433–437 adds Brightness itself but still not the two Night pickers. A routine poll landing while someone is on Night mode or Night face returns `SCR_DIAL` and yanks them off — the same class of gap the 2026-09-01 audit closed for Timezone/Adjust mode. Out of this commit's scope (spec §6 commit 2 exactly); a one-line-per-set fix when you want it.
2. The `DIAL_TEMP_*`-style clang diagnostics the IDE showed for the new file are the host clang choking on Xtensa flags and a stale index; the real build is clean.

## Not verified without hardware

- **Settings shows "Standby face — Temperature" directly under "Screen timeout"** on the panel (the simulator render does; the real list's zoom/fade at that row is what to look at), and **"Night (standby)"** under Brightness with its value.
- **Pick Clock, let it time out → the clock face**; wake → today's clock-to-dial transition.
- **Pick Temperature, let it time out → the dial face** dimmed to the standby duty; wake → brightens in place, first input swallowed.
- **The choice survives a reboot**: after picking Clock, the next boot's capture should show `dial_state: sb_face: stored 0 -> 0` instead of the no-key line, and the row should read Clock.
- **Night standby at 0 %** (Settings → Brightness → Night (standby) → 0, "Off") → a dark face at night standby whichever face is chosen; set it back afterwards (the bench value before this task was whatever `bri_nclk` holds — the pref was not touched).
- Spec §5 items 1–3 and 5 carry over from commit 1 for the Clock/Temperature switch on a live pad.
