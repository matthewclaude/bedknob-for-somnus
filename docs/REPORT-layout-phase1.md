# REPORT — Screen layout pass, phase 1 (Section S + Tier A)

Date: 2026-09-04. Branch `main`. Not part of either commit.
Plan: `docs/PLAN-screen-layout-fixes.md`; measurements: `docs/REPORT-screen-layout-audit.md` (now with a "Resolution — phase 1" section).

## Verdict

**DONE.** Two commits on `main`: `a009840` (Section S: 10 new scenarios + the S3 stub hook + the settings-pad re-aim, 40 PNGs regenerated as the BEFORE set) and `5bf68a5` (Tier A: A1–A5 in four `dial_ui` files, the AFTER PNGs, the two doc edits). Simulator rebuilt and run at each commit; `idf.py build` clean with zero warnings. Every "Verifies as" row passes by measurement. No tag, no version bump, no push, no flash. Tiers B/C/D untouched. Three deviations from the plan text, all listed below; one residual (A1 at level +15 vs the chassis ring) flagged for the owner.

## Gate

```
$ git --no-optional-locks status --short
(empty)
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")
$ git describe --tags --abbrev=0
somnus-v0.1.5
$ git branch --show-current
main            (HEAD was 09d1ba2)
```

## Files touched (whole pass)

- `simulator/main.c`, `simulator/sim_state.h`, `simulator/stubs.c` (commit 1)
- `firmware/dial-idf/components/dial_ui/{scr_dial.c, scr_settings.c, scr_sidepick.c, dial_list.c}` (commit 2)
- `docs/screens/*.png` (both), `docs/PLAN-screen-layout-fixes.md`, `docs/REPORT-screen-layout-audit.md` (commit 2)

Nothing outside `components/dial_ui/`, `simulator/`, `docs/screens/`, `docs/`. No state / power / networking / worker code.

## Commit 1 — `a009840a19f5aa7ce2ce7a4c2fe26e5f62281f7b`

`sim: scenarios for the layout audit (S1-S9, timezone, night-face)`

### What was added

| Row | Scenario / PNG | Implementation |
|---|---|---|
| S1 | `dial-celsius` | `units_c = true`, zone A `temp_dc = 200`, `actual_c = 20.0` (HOLDING); SCR_MENU bounce first because the previous scenario ended on the same `(SCR_DIAL, ZONE_A)` pair and `ui_router_go` no-ops on it |
| S2 | `dial-relative-max` | `rel_mode = true`, `temp_dc = DIAL_REL_MAX_DC` (420, level +15), same bounce |
| S3 | `settings-timezone-raw` | new `sim_set_fake_iana_tz()` hook (`sim_state.h` + `stubs.c`); `dial_time_get_iana_tz()` returns the fake only while set, default still `false`; `dial_time_valid()` untouched (`false`); +8 detents from Brightness(1) onto Timezone(9); hook cleared after the snapshot. Zone string: see Deviations |
| S4 | `wifi-confirm` | SCR_WIFI, +3 detents (Network(1) → Change network(4)), `sim_tap(180,180)` on the focused row through the real pointer indev |
| S5 | `night-mode` | SCR_NIGHT_MODE with `night_on` (sim default) and the stub clock invalid → §7 note renders |
| S6 | `pad-discovery` | `phase = PH_PAD_DISCOVERY`, `phase_err = "Still looking (checking more slowly)...\n137/254"` |
| S7 | `update-failed` | `ota.status = OTA_FAILED`, `ota.err = "check failed (HTTP -1)"`; no knob-walk (see Deviations) |
| S8 | `pad-degraded-real` | `dial_state_set_pad_url("http://unreachable.invalid:8080")` for PH_DEGRADED, then `phase_err = "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)"`, `retry_in_s = 27` |
| S9 | `scenario_settings_pad` | `sim_knob(7)` → `sim_knob(9)`; comment rewritten with the current 13-row order |
| — | `timezone`, `night-face` | plain navigate-and-snapshot (SCR_TIMEZONE arg 0 = from Settings; SCR_NIGHT_FACE) |

Also in `apply_baseline()`: `phase_err[0] = '\0'` and `retry_in_s = 0`, because `dial_state_set_pad_url()` only resets the phase and S6/S8 would otherwise leak their reason text into later PH_DEGRADED renders.

### Raw simulator output (commit-1 tree, final run)

```
$ cmake --build build
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
build exit=0
$ ./build/dial_sim
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (5 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote sidepick         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/sidepick.png (5 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (4 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (4 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (4 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (7 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (5 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (5 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (3 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (5 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (5 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (1 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (4 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (6 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (7 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (4 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (5 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (2 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (4 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (5 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (5 distinct colors sampled)
done: 40 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
dial_sim exit=0
```

(The first run at this step, before the S3 zone and S7 detent corrections, is identical apart from those two PNGs; not reproduced.)

### `git show --stat` (commit 1)

```
a009840a19f5aa7ce2ce7a4c2fe26e5f62281f7b
sim: scenarios for the layout audit (S1-S9, timezone, night-face)

 docs/screens/about-wifi-real.png       | Bin 24711 -> 24114 bytes
 docs/screens/about-wifi-worst.png      | Bin 25677 -> 25079 bytes
 docs/screens/dial-celsius.png          | Bin 0 -> 27052 bytes
 docs/screens/dial-relative-max.png     | Bin 0 -> 26516 bytes
 docs/screens/night-face.png            | Bin 0 -> 16444 bytes
 docs/screens/night-mode.png            | Bin 0 -> 18545 bytes
 docs/screens/pad-degraded-real.png     | Bin 0 -> 19447 bytes
 docs/screens/pad-discovery.png         | Bin 0 -> 19370 bytes
 docs/screens/settings-timezone-raw.png | Bin 0 -> 24943 bytes
 docs/screens/settings.png              | Bin 19952 -> 21654 bytes
 docs/screens/timezone.png              | Bin 0 -> 17571 bytes
 docs/screens/update-failed.png         | Bin 0 -> 22551 bytes
 docs/screens/update.png                | Bin 24716 -> 24854 bytes
 docs/screens/wifi-confirm.png          | Bin 0 -> 22447 bytes
 simulator/main.c                       | 232 +++++++++++++++++++++++++++++++--
 simulator/sim_state.h                  |   8 ++
 simulator/stubs.c                      |  24 +++-
 17 files changed, 254 insertions(+), 10 deletions(-)
```

`settings-pad.png` at +9 is byte-identical to the checked-in file (md5 `312b85f5…`), so of the audit's five "stale" shots only four changed here (`settings`, `update`, `about-wifi-real`, `about-wifi-worst`).

### BEFORE measurements (renders at `a009840`)

Method: ink = pixels differing from the panel bg (#101418) by > 40 in any channel, inside the 180 px mask (alpha 255). Column runs split on ≥ 8 empty columns. Far-row (A5) measured at threshold 12 because that row is faded to opa 100/255. Script: scratchpad `ink.py` (PIL + numpy).

| Item | PNG | Measurement |
|---|---|---|
| A1 | `dial.png` | "72" ink x 130–230 × y 100–181 (band); °F unit ink **257–275 × 114–128**. Matches the audit (257–275, 114–128). |
| A1 | `dial-celsius.png` | one merged run **87–276 × 100–182**: "20.0" and °C are not separable — the unit is under the digits (audit: 18 px overprint) |
| A1 | `dial-relative-max.png` | "+15" and "LEVEL" merged 117–297; unit band 205–297: "LEVEL" starts at 236 inside the "5" (audit: overprint at ≥ +10) |
| A3 | `dial-update.png` | label ink (diff vs `dial.png`) **131–229 × 325–337**; dots **161–166 × 337–342** → one shared row (audit: 325–336 vs 334–342 at its threshold; same 2 px overlap) |
| A5 | `settings.png` | far row (y 318–350): max ink radius **178.3** = on the mask edge; value fragments end at 243, chord at y 341 is 80–260 |
| A5 | `wifi-info.png` | far row: max ink radius **178.3**; value fragments 201–264 |
| A2 | `settings.png` Night mode row (y 220–292) | one ink band 52–309 × 248–268: value drawn through the label |
| A2 | `settings-timezone-raw.png` focused row | label 37–55 and value 60–322 in one 171–188 band: "America/Mexico_City" through "Timezone" |
| A4 | `sidepick.png` | **no ink** in y 20–110 — the title is covered by the halves |

## Commit 2 — `5bf68a5b598d2b8465ac83cead7b0077ac03b977`

`ui: layout fixes A1-A5 from the screen audit`

### What changed

- **A1 `scr_dial.c`:** fixed `lv_obj_align(s_unit_lbl, CENTER, 266−CX, 122−CY)` deleted; new `place_unit()` = `lv_obj_align_to(s_unit_lbl, s_temp_lbl, LV_ALIGN_OUT_RIGHT_TOP, 22, -6)`, called after `lv_label_set_text` in `render_numeral()` and `alt_show_water()`. The two "−10…+10" comments (the `s_rel` static's and `render_value()`'s) now say −15…+15. `lv_obj_align_to` calls `lv_obj_update_layout` on the screen first, so the numeral's fresh width is what gets measured (checked in LVGL 8.4's `lv_obj_pos.c`).
- **A2 `scr_settings.c`:** Night mode and Timezone rows: label `LEFT_MID 0,−16`; value `width LV_PCT(100)`, `LONG_DOT`, `LEFT_MID 0,16`. Unconditional.
- **A3 `scr_dial.c`:** `s_ota_lbl` `330 − CY` → `326 − CY`; create() comment now carries the 316→337 budget, y = 326, ink 321–332 arithmetic. `scr_standby.c` untouched.
- **A4 `scr_sidepick.c`:** title `lv_label_create` block moved after the halves loop and the divider (just before `apply_highlight()`); `TOP_MID 36` → `72`.
- **A5 `dial_list.c`:** `ZOOM_MIN 168` → `140`; comment carries the chord arithmetic (content 101–259 vs chord 96–264; old 85.5–274.5 vs 80–260).
- Docs: PLAN status line at the top; audit report "Resolution — phase 1" appended.

### Raw simulator output (commit-2 tree)

```
$ cmake --build build
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_dial.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_sidepick.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_settings.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_list.c.o
[ 43%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
build exit=0
$ ./build/dial_sim
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
done: 40 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
dial_sim exit=0
```

### `idf.py build` — last 40 lines

```
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_dial.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_settings.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_sidepick.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_list.c.obj
[ 97%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
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
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
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

Every warning line in the full log (`grep -i warning`, CMake-policy lines excluded): **none**. Lines matching `dial_ui` and `warning`: **none**. Exit 0. (The three `NOTE:` lines about Kconfig bool defaults are ESP-IDF's own, unrelated to this tree.)

### `git show --stat` (commit 2)

```
5bf68a5b598d2b8465ac83cead7b0077ac03b977
ui: layout fixes A1-A5 from the screen audit

 docs/PLAN-screen-layout-fixes.md                   |   2 +
 docs/REPORT-screen-layout-audit.md                 |  44 +++++++++++++++++++++
 docs/screens/about-battery-pct.png                 | Bin 21673 -> 20542 bytes
 docs/screens/about-battery-usb.png                 | Bin 21782 -> 20650 bytes
 docs/screens/about-wifi-real.png                   | Bin 24114 -> 23213 bytes
 docs/screens/about-wifi-worst.png                  | Bin 25079 -> 24177 bytes
 docs/screens/about.png                             | Bin 19116 -> 18520 bytes
 docs/screens/brightness-menu.png                   | Bin 20590 -> 19828 bytes
 docs/screens/dial-celsius.png                      | Bin 27052 -> 27029 bytes
 docs/screens/dial-relative-max.png                 | Bin 26516 -> 26538 bytes
 docs/screens/dial-relative.png                     | Bin 25705 -> 25719 bytes
 docs/screens/dial-update.png                       | Bin 26120 -> 26114 bytes
 docs/screens/dial.png                              | Bin 25125 -> 25134 bytes
 docs/screens/menu.png                              | Bin 21386 -> 20975 bytes
 docs/screens/netpick.png                           | Bin 19246 -> 18724 bytes
 docs/screens/night-face.png                        | Bin 16444 -> 16192 bytes
 docs/screens/night-mode.png                        | Bin 18545 -> 17945 bytes
 docs/screens/settings-pad.png                      | Bin 23420 -> 22925 bytes
 docs/screens/settings-timezone-raw.png             | Bin 24943 -> 24187 bytes
 docs/screens/settings.png                          | Bin 21654 -> 20899 bytes
 docs/screens/sidepick.png                          | Bin 12683 -> 15393 bytes
 docs/screens/timezone.png                          | Bin 17571 -> 17164 bytes
 docs/screens/update-failed.png                     | Bin 22551 -> 21741 bytes
 docs/screens/update.png                            | Bin 24854 -> 23727 bytes
 docs/screens/wifi-info.png                         | Bin 18684 -> 18040 bytes
 firmware/dial-idf/components/dial_ui/dial_list.c   |  14 ++++++-
 firmware/dial-idf/components/dial_ui/scr_dial.c    |  43 +++++++++++++++-----
 .../dial-idf/components/dial_ui/scr_settings.c     |  24 ++++++++++-
 .../dial-idf/components/dial_ui/scr_sidepick.c     |  18 ++++++---
 29 files changed, 126 insertions(+), 19 deletions(-)
```

### AFTER measurements and pass/fail

| # | "Verifies as" | Measured | Result |
|---|---|---|---|
| A1 | `dial.png` unchanged within 2 px | °F unit ink 257–275×114–128 → **256–274×115–129** (1 px left, 1 px down). Whole-panel diff vs commit 1: bbox 256–275×114–129, **92 px**. Nothing else moved. | **pass** |
| A1 | `dial-celsius.png` "20.0 °C" no overlap | "20.0" 87–270; °C **297–317** × 115–129 → 27 px clear | **pass** |
| A1 | `dial-relative-max.png` "+15 LEVEL" | "+15" 117–246; "LEVEL" **273–334** × 116–129 → 27 px clear of the digits; legible | **pass** (residual below) |
| A2 | `settings.png` two clean lines | Night mode row: label band **234–253**, value band **265–277**, 12 empty rows between | **pass** |
| A2 | `settings-timezone-raw.png` two clean lines | Timezone row: label **155–172**, value **188–204** ("America/Mexico_City" full, no ellipsis) | **pass** |
| A3 | `dial-update.png` ink 321–332, 2 px above dots, 5 px below disc | label ink **131–229 × 321–333**; dots 337–342 → rows 334–336 empty (3 px); disc bottom 316 → rows 317–320 empty (4 px) | **pass** — the 1 px difference from the plan's 321–332 is the threshold convention (the same method read the BEFORE as 325–337 where the audit said 325–336) |
| A4 | `sidepick.png` title visible inside the 288 px chord | title ink **63–296 × 75–89**; chord at y 89 is 24–336 (39 px margin each side); max radius 155.8 | **pass** |
| A5 | `settings.png` "Absolute" / `wifi-info.png` "dBm" fully inside the mask | far row max ink radius **174.2** / **173.8** (was 178.3 = clipped); content 103–~250, chord at the band's bottom 96–264. In the current renders the far rows are "Night face / Number only" and "Signal / Strong -48 dBm", both complete. Every other list shot (about×5, brightness-menu, menu, netpick, night-face, night-mode, settings×3, timezone, update×2) eyeballed: far rows smaller and dimmer, neighbours unchanged. | **pass** |

Note on A5's row content: the audit's "Absolute" was the far row of a pre-2026-09-02 Settings order; today's opening Settings screenshot has "Night face / Number only" in that slot. The chord check is the same regardless of the text.

### Residual — A1 at level +15 (owner's call, not fixed)

With the unit riding the numeral, "LEVEL" at +15 ends at x = 334, and the r = 165 chassis ring's inner edge at y 116–129 is at x ≈ 315–322, so the final "L" sits on the ring by ~12–17 px. Visible in `dial-relative-max.png` (the accent arc is at full extent there, so it is orange under the "L"; at lower levels it would be the dim track). The plan's chord check (≤ 347) was against the mask, not the ring. "+3 LEVEL" ends at 317 and every absolute value is clear. Fix options if wanted: gap 22 → 16 (moves the "L" to ~328), or a smaller unit at |level| ≥ 10. Recorded in the audit report's Resolution section too.

## Deviations from the plan's Section S / Tier A text

1. **S3 zone string:** `America/Mexico_City` instead of the plan's `America/Los_Angeles`. Los_Angeles is in `DIAL_TZ_IANA[]` and rendered as the curated label "Pacific" (checked on the first run), so it never exercised the raw-IANA path the scenario exists for. Mexico_City is a real, portal-detectable zone outside the list with the same 19 characters, so the audit's §2b width numbers still apply. The hook mechanism is exactly as the owner decided (stub grows a small override; default behaviour unchanged).
2. **S7 no knob-walk:** the plan (via `scenario_update`'s pattern) implies +1 detent; `scr_update.c` already opens on "Check for updates" (row 1: Back/Check for updates/Installed/Auto-update/Beta builds), so +1 put the FAILED row above focus. Dropped the detent so the three-line row renders in the focused slot. `scenario_update`'s own comment ("Installed(1)/Check for updates(2)") is stale for the same reason — left alone, not a Section S row; flagged here.
3. **PLAN status note names commit 2 by title, not SHA:** the note has to be inside commit 2, whose SHA does not exist until it is made. It cites `a009840` for Section S and "the commit immediately after it" for Tier A; the SHA is `5bf68a5` (this report).
4. **A1 "file-header" wording:** the "−10…+10" text was not in the file header but in two comments (`s_rel`'s at line ~123 and `render_value()`'s at ~404); both updated to −15…+15.
5. **`apply_baseline()` gained two resets** (`phase_err`, `retry_in_s`) — simulator hygiene the new S6/S8 scenarios need; not in the plan's row text.

Everything else: none. A2, A3, A4, A5 edits are the plan's exact lines.

## Not verified (needs hardware)

- Plan verification steps 3–4: flash to `/dev/cu.usbmodem83401`, boot confirmation, and the eyes-on list (°C across the knob range, Settings with night on / no zone, Settings and Wi-Fi lists scrolled to their ends for A5's curvature, side pick via Settings, an available update on the face). **Not flashed.**
- Whether the physical bezel, which covers a few px more than the simulator's 180 px mask, still shows the full far row at zoom 140 (≈5 px margin in the simulator).
- A1 during a live range-stop nudge (the unit is expected to stay put while `s_num_box` animates; only static renders were checked).
- The night-face minimal path (unit hidden, `num_140`) with the new `place_unit()` call — compiles and the hidden label is aligned harmlessly, but no night-face dial scenario exists to render it.
