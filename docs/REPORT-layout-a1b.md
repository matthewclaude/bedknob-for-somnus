# REPORT — A1b: °C setpoint without a zero decimal; no unit label in relative mode

Date: 2026-09-05 17:27 CDT. Branch `main`. Follow-up to `5bf68a5` (Tier A).
Toolchain ESP-IDF v6.0 via `~/esp/esp-idf/export.sh`; simulator via `cmake --build build`.

## Verdict

**DONE — committed as `40b784a`, simulator renders verified by measurement, `idf.py` clean build with zero compiler warnings, wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.5, no assert/panic/abort/Guru/`E (` lines, no reset).** No tag, no version bump, no push. One finding from the bench log that is outside this task's scope and needs the owner's decision: in absolute mode a touch drag commits the raw finger value without snapping to the whole-degree grid, so a dragged °C setpoint (the log shows 42.3 → 13.3) keeps its tenth and renders four glyphs again. See "Finding" at the end.

## Gate

```
$ git --no-optional-locks status --short
?? docs/REPORT-layout-phase1-flash.md
$ git add docs/REPORT-layout-phase1-flash.md && git commit -m "docs: phase 1 flash report"
[main e2f2996] docs: phase 1 flash report
$ git --no-optional-locks status --short
(empty)
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")
$ git log --oneline -1
e2f2996 docs: phase 1 flash report
```

Gate passed; proceeded.

## Change — exact diff of `scr_dial.c` (`git diff e2f2996 40b784a -- firmware/dial-idf/components/dial_ui/scr_dial.c`)

```diff
diff --git a/firmware/dial-idf/components/dial_ui/scr_dial.c b/firmware/dial-idf/components/dial_ui/scr_dial.c
index 3566f4f..5332a66 100644
--- a/firmware/dial-idf/components/dial_ui/scr_dial.c
+++ b/firmware/dial-idf/components/dial_ui/scr_dial.c
@@ -14,6 +14,7 @@
 #include "dial_haptics.h"
 #include "dial_ota.h"
 #include <math.h>
+#include <stdlib.h>
 #include <time.h>
 
 LV_FONT_DECLARE(dial_font_num_88)
@@ -53,7 +54,7 @@ static lv_obj_t *s_handle;       // setpoint drag handle — the ONLY temp touch
  * Recomputed only when a poll moves the water or the user moves the target.
  */
 // The setpoint numeral's opacity while the side is OFF, on either face, along
-// with its unit/LEVEL suffix and the WATER caption. Pitched below everything
+// with its unit suffix and the WATER caption. Pitched below everything
 // else so the numeral joins the chassis rather than sitting above it: once the
 // pill is hidden and the power button has gone quiet, a full-strength numeral
 // is the last thing that still reads as a live setting. It also has to lose
@@ -407,8 +408,12 @@ static void apply_identity(const dial_palette_t *pal, bool night)
 // dial_font_num_140 (docs/SPEC-night-face.md §5). Absolute mode shows the
 // dc value directly (a trivial /10 split, exact — no rounding, since it IS
 // the canonical unit) when s_units_c, or dial_dc_to_f() when not (M4 units
-// toggle). °F is display-only math with no bearing on what gets stored or
-// posted: after the Q1 units fix, absolute mode steps in exact whole 1.0°C
+// toggle). A zero tenth is dropped ("34", not "34.0"): every setpoint is a
+// whole degree (1.0 °C per level, level 0 = 27.0, rails 12/42), so the ".0"
+// carried nothing and cost a glyph — four glyphs put the unit on the ring and
+// under the handle at every °C value (A1b). A non-zero tenth is kept, which
+// is what the shared water reading (below) needs: "23.4" stays "23.4".
+// °F is display-only math with no bearing on what gets stored or posted: after the Q1 units fix, absolute mode steps in exact whole 1.0°C
 // increments, so the °F numeral now steps IRREGULARLY (e.g. 68, 70, 72, 73,
 // 75 — each is the nearest whole °F to a clean whole-°C value). That
 // irregularity is the correct, expected result of the setpoint actually
@@ -430,7 +435,12 @@ static void render_value(int temp_dc, char *out, size_t out_sz)
         if (lvl == 0) snprintf(out, out_sz, "0");
         else          snprintf(out, out_sz, "%+d", lvl);   // "+3" / "-3"
     } else if (s_units_c) {
-        snprintf(out, out_sz, "%d.%d", temp_dc / 10, temp_dc % 10);
+        // abs() on the remainder only: dc can't go negative on this pad
+        // (floor 12.0 °C), but a sub-zero water reading must not print a
+        // second '-' from the tenths.
+        int tenths = abs(temp_dc % 10);
+        if (tenths == 0) snprintf(out, out_sz, "%d", temp_dc / 10);
+        else             snprintf(out, out_sz, "%d.%d", temp_dc / 10, tenths);
     } else {
         snprintf(out, out_sz, "%d", dial_dc_to_f(temp_dc));
     }
@@ -441,8 +451,10 @@ static void render_value(int temp_dc, char *out, size_t out_sz)
 // "+10"…"+15" in relative) ran straight through it (2026-09-04 layout
 // audit §1a). OUT_RIGHT_TOP with a 22px gap and −6 lift puts the °F case
 // exactly where the fixed slot had it (box x 254–275 vs 255–277 before,
-// same y), and the widest cases ("20.0 °C" → 320, "+15 LEVEL" → 335) stay
-// inside the chord at the unit's y-band (x ≤ 347). lv_obj_align_to
+// same y). After A1b the widest cases are "42 °C" and "108 °F" — three
+// glyphs plus the unit — which sit well inside the chord at the unit's
+// y-band (x ≤ 347); relative mode has no unit label at all, so the old
+// "+15 LEVEL" case no longer exists. lv_obj_align_to
 // re-lays-out the screen first, so the numeral's fresh width is what gets
 // measured. The range-stop nudge animates s_num_box's x, not the label, so
 // the unit stays put during a nudge exactly as before.
@@ -641,14 +653,14 @@ static void apply_palette_and_state(const app_state_t *st)
     }
     lv_obj_set_style_text_color(s_unit_lbl, pal->ink_secondary, 0);
     lv_obj_set_style_text_opa(s_unit_lbl, z->on ? LV_OPA_COVER : NUM_STANDBY_OPA, 0);
-    // Relative mode has no unit — the suffix names the quantity instead. The
-    // measured-water caption above keeps its degree, so an absolute
-    // reference stays on the face in every mode.
-    if (st->rel_mode) lv_label_set_text(s_unit_lbl, "LEVEL");
-    else              lv_label_set_text(s_unit_lbl, st->units_c ? "\xC2\xB0" "C" : "\xC2\xB0" "F");
-    // Night face (§3): unit / LEVEL label hidden under minimal.
-    if (minimal) lv_obj_add_flag(s_unit_lbl, LV_OBJ_FLAG_HIDDEN);
-    else         lv_obj_clear_flag(s_unit_lbl, LV_OBJ_FLAG_HIDDEN);
+    // Relative mode has no unit and shows no label: the signed numeral
+    // ("+15") is the whole reading, and the "LEVEL" suffix it used to carry
+    // ran onto the ring for |level| >= 10 (A1b). The measured-water caption
+    // above keeps its degree, so an absolute reference stays on the face in
+    // every mode. Hidden under minimal too (night face, §3).
+    lv_label_set_text(s_unit_lbl, st->units_c ? "\xC2\xB0" "C" : "\xC2\xB0" "F");
+    if (minimal || st->rel_mode) lv_obj_add_flag(s_unit_lbl, LV_OBJ_FLAG_HIDDEN);
+    else                         lv_obj_clear_flag(s_unit_lbl, LV_OBJ_FLAG_HIDDEN);
 
     // Neutral landmark: a tick at 12 o'clock (level 0's center) shown only in
     // relative mode — it turns the ring from a plain bar into a bipolar
```

Notes on the diff:

- Change 1 is the `s_units_c` branch of `render_value()`: a zero tenth is dropped, a non-zero tenth is kept. `abs()` guards the remainder (hence `#include <stdlib.h>`); dc cannot be negative on this pad (floor 12.0 °C), so the guard is only there so a hypothetical sub-zero water reading never prints a second '-'. Note the integer part would still print as "0" for −0.5 (C truncates `-5 / 10` to 0); irrelevant on this hardware, recorded for honesty.
- Change 2 is the unit-label block of `apply_palette_and_state()`: the label always carries °C/°F and is hidden if `minimal || st->rel_mode`. The `"LEVEL"` string is gone; the comment above it and `place_unit()`'s comment are rewritten as specified. The file-header comment's "unit/LEVEL suffix" also lost its "LEVEL".
- The `render_value()` block comment gained four lines explaining the drop-a-zero-tenth rule and why the water reading keeps its tenth.

## Simulator scenario — `simulator/main.c` diff

The spec asked for one scenario rendering a fractional water reading in the night-face alternation if none existed. None did (the 40 existing scenarios never call `dial_palette_set_night()`, and none of the dial ones is heating with `night_face_min` set). The alternation **is** renderable: night is a plain palette flag (`dial_palette_set_night()`, public in `dial_palette.h`), `sim_state_reset()` ships `night_face_min = true`, and `pump_ms()` advances `lv_tick`, so the §3a `lv_timer` fires. `scr_dial.c`'s `create()` leaves the alternation unlocked (`s_last_interact_ms = 0`), the timer is created on the `on_state` inside `ui_router_go` with period 2000 ms, first fires into the water phase at +2000 and holds it to +4000, so the scenario pumps 3000 ms and snapshots mid-phase. Added `scenario_dial_night_water` (°C, setpoint 34.0 → "34", water 23.4 → "23.4", heating) right after `scenario_dial_relative_max`, with the day palette restored afterwards. Two stale comments in the same file that still said the "LEVEL" suffix is visible were corrected.

```diff
diff --git a/simulator/main.c b/simulator/main.c
index 265461d..1d81c75 100644
--- a/simulator/main.c
+++ b/simulator/main.c
@@ -349,8 +349,8 @@ static void scenario_dial_update(void)
 // commit 018d8f6) — so the
 // render proves the spliced '+' glyph draws AND that an off-grid device
 // value shows as the nearest level. Water below the setpoint keeps the
-// heating overlay + pill on screen, and the neutral notch/"LEVEL" suffix
-// are visible.
+// heating overlay + pill on screen, and the neutral notch is visible (the
+// "LEVEL" suffix is gone since A1b — relative mode shows no unit label).
 static void scenario_dial_relative(void)
 {
     apply_baseline();
@@ -393,7 +393,9 @@ static void scenario_dial_celsius(void)
 
 // The Home face in RELATIVE scale at the +15 rail (DIAL_REL_MAX_DC, 42.0°C):
 // the widest relative numeral ("+15" -- three glyphs, the spliced '+' plus
-// the wide '1'/'5'), the other §1a collision case for the "LEVEL" suffix.
+// the wide '1'/'5'), the other §1a collision case for the old "LEVEL"
+// suffix — since A1b relative mode shows no unit label at all, so this
+// render proves the bare "+15" with nothing to its right.
 // Same SCR_MENU bounce as scenario_dial_celsius, same reason.
 static void scenario_dial_relative_max(void)
 {
@@ -413,6 +415,38 @@ static void scenario_dial_relative_max(void)
     snapshot("dial-relative-max");
 }
 
+// The night face (Number only) mid water-alternation (docs/SPEC-night-face.md
+// §3a) in °C with a FRACTIONAL water reading: A1b drops a zero tenth from
+// the shared °C renderer ("34", not "34.0") and this is the case that must
+// keep its tenth — the water at 23.4 °C must read "23.4", in the accent, with
+// the WATER word under it. Night comes from dial_palette_set_night() (the
+// real firmware's night worker is what calls it); sim_state_reset() already
+// ships night_face_min = true. Timing: create() leaves the alternation
+// unlocked (s_last_interact_ms = 0 and the sim tick is well past
+// ALT_KNOB_LOCK_MS by now), the timer is created on the on_state inside
+// ui_router_go and first fires at +2000 ms into the water phase, which
+// holds until +4000 — so a 3000 ms pump lands mid-phase. Same SCR_MENU
+// bounce as scenario_dial_celsius, same reason; day palette restored after.
+static void scenario_dial_night_water(void)
+{
+    apply_baseline();
+    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
+    pump_ms(100);
+    app_state_t *st = sim_state_ptr();
+    st->units_c = true;
+    st->ui_zone = ZONE_A;
+    zone_state_t *a = &st->zones[ZONE_A];
+    a->on = true;
+    a->temp_dc = 340;     // 34.0C -> "34" on the setpoint phase
+    a->actual_c = 23.4f;  // below setpoint: heating -> alternation runs; "23.4"
+    st->generation++;
+    dial_palette_set_night(true);
+    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
+    pump_ms(3000);
+    snapshot("dial-night-water");
+    dial_palette_set_night(false);
+}
+
 // Also documents the M7 permanent "Update" row (replaces the M6 conditional
 // "Install X.Y.Z" row — confirmation moved into SCR_UPDATE itself, this row
 // is now pure navigation): sets the OTA status to available with a pending
@@ -971,6 +1005,7 @@ int main(void)
     scenario_dial_relative();
     scenario_dial_celsius();
     scenario_dial_relative_max();
+    scenario_dial_night_water();
     scenario_menu();
     scenario_update();
     scenario_update_prompt();
```

## Raw simulator build + run output (`cmake --build build && ./build/dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_dial.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
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
done: 41 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
RUN_EXIT=0
```

41 screens rendered (40 + `dial-night-water`). `cmp` against the pre-run backup: only `dial-celsius.png`, `dial-relative.png`, `dial-relative-max.png` changed and `dial-night-water.png` is new; the other 37 are byte-identical to the checked-in files.

## Measurements

Method as in the audit (PIL + numpy; ink = pixels differing from bg `#101418` by > 40 in any channel with alpha 255, restricted to r < 150 from centre to exclude the ring; boxes split on ≥ 8 empty columns; "previous commit" = `e2f2996`'s PNG, i.e. the A1 render). Raw output of the measuring script:

```
== dial-celsius: 360x360, inner-ink rows 58..315
   band y58-69: box x134-225 y58-69  (w=92)
   band y81-83: box x150-209 y81-83  (w=60)
   band y92-182: box x31-57 y95-168  (w=27)
   band y92-182: box x126-231 y92-182  (w=106)
   band y92-182: box x258-278 y115-129  (w=21)
   band y191-315: box x31-76 y191-288  (w=46)
   band y191-315: box x118-242 y201-315  (w=125)
   diff vs previous commit: 5810 px, bbox x86-317 y115-182
== dial-relative-max: 360x360, inner-ink rows 31..315
   band y31-315: box x31-329 y31-315  (w=299)
   diff vs previous commit: 364 px, bbox x273-334 y116-129
== dial-night-water: 360x360, inner-ink rows 31..315
   band y31-315: box x31-324 y31-315  (w=294)
   (new file, no previous)
== dial: 360x360, inner-ink rows 58..315
   band y58-315: box x31-87 y63-288  (w=57)
   band y58-315: box x120-239 y58-315  (w=120)
   band y58-315: box x256-274 y115-129  (w=19)
   diff vs previous commit: 0 px (identical)
== dial-relative: 360x360, inner-ink rows 31..315
   band y31-315: box x31-254 y31-315  (w=224)
   diff vs previous commit: 364 px, bbox x256-317 y116-129
== dial-update: 360x360, inner-ink rows 58..329
   band y58-329: box x31-87 y63-288  (w=57)
   band y58-329: box x120-239 y58-329  (w=120)
   band y58-329: box x256-274 y115-129  (w=19)
   diff vs previous commit: 0 px (identical)
```

Read-out against the spec's acceptance:

| Check | Result |
|---|---|
| `dial-celsius.png` shows "20 °C", no decimal | yes — numeral ink x 126–231 (was "20.0" 87–270); the °C unit box is **x 258–278 × y 115–129** |
| °C unit right edge ≤ 280 (the °F slot) | **278** ≤ 280 — 37 px short of the ring's inner edge at x ≥ 315 (A1 had it at 297–317) |
| `dial-relative-max.png` shows "+15" with no unit label | yes — the 364 px that differ from A1 are the old "LEVEL" area, bbox x 273–334 × y 116–129; nothing else moved |
| `dial.png` (°F) unchanged within 2 px | **0 px differ** (identical); `dial-update.png` also 0 px |
| Fractional water reading in the night-face alternation | `dial-night-water.png`: "23.4" in the accent, "WATER" above it, unit hidden (minimal), power button only — eyeballed from the PNG, matches SPEC-night-face §3a |

`dial-relative.png` (level +3) also lost its "LEVEL" (364 px, x 256–317) — same change, expected.

## `idf.py build` — raw tail (last 40 lines)

Run as `idf.py fullclean && idf.py build` (see Deviations, 1). Exit 0. 31 `dial_ui` objects compiled.

```
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_16.c.obj
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

Every line of the full build log containing "warning" (case-insensitive):

```
4: CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
11: This warning is for project developers.  Use -Wno-author or -Wno-deprecated
```

That single hit is CMake's own deprecation note about `cmake_minimum_required` in the project `CMakeLists.txt` (present on the phase-1 build too, not shown in that report because its grep excluded CMake policy lines). Compiler warnings: **none**. `dial_ui` warnings: **none**. The three `NOTE:` Kconfig lines are ESP-IDF's own.

App descriptor (`python -m esptool --chip esp32s3 image-info build/somnus-dial.bin`):

```
Project name: somnus-dial
App version: 0.1.5
Compile time: Sep  5 2026 17:24:47
ESP-IDF: v6.0
```

## Port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

Native USB-Serial/JTAG port present, no CH340 — no "flip the plug".

## `idf.py -p /dev/cu.usbmodem83401 flash` — raw output

Exit 0. The flash's own `all` step recompiled `scr_dial.c` (one object — the comment re-wrap committed after the clean build, see Deviations 4) and re-linked with **no warning lines** (`grep -i warning` on the flash log finds only the same CMake deprecation note). Below is from `Executing action: flash` to the end; the only edit is that esptool's terminal control sequences and the in-place progress-bar rewrites were stripped, leaving the final state of each progress line.

```
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target blank_ota_data
[  0%] Built target _project_elf_src
[  0%] Built target partition_table_bin
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target memory_ld_in_preprocess
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_https_ota
[  0%] Built target __idf_esp_http_server
[  2%] Built target _project_elf_src
[  2%] Built target bootloader_ld_in_preprocess
[  8%] Built target __idf_log
[  1%] Built target __idf_esp_http_client
[ 16%] Built target __idf_esp_rom
[  1%] Built target __idf_tcp_transport
[ 18%] Built target __idf_esp_common
[  1%] Built target __idf_esp_driver_i2s
[ 25%] Built target __idf_esp_hw_support
[  2%] Built target __idf_esp_adc
[ 26%] Built target __idf_esp_system
[  3%] Built target __idf_esp-tls
[ 32%] Built target __idf_efuse
[  3%] Built target __idf_http_parser
[ 47%] Built target __idf_bootloader_support
[  3%] Built target __idf_esp_hal_i2c
[ 50%] Built target __idf_esp_security
[  3%] Built target __idf_esp_gdbstub
[ 52%] Built target __idf_esp_hal_security
[  3%] Built target __idf_esp_wifi
[ 56%] Built target __idf_esp_hal_ana_conv
[  3%] Built target __idf_esp_coex
[ 59%] Built target __idf_esp_hal_uart
[ 62%] Built target __idf_esp_hal_wdt
[  8%] Built target __idf_wpa_supplicant
[ 64%] Built target __idf_esp_hal_timg
[  9%] Built target __idf_esp_netif
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[ 15%] Built target __idf_lwip
[ 71%] Built target __idf_esp_hal_gpspi
[ 15%] Built target __idf_vfs
[ 74%] Built target __idf_esp_hal_dma
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 75%] Built target __idf_micro-ecc
[ 15%] Built target __idf_esp_phy
[ 76%] Built target __idf_esp_hal_pmu
[ 16%] Built target __idf_nvs_flash
[ 79%] Built target __idf_esp_hal_usb
[ 17%] Built target __idf_nvs_sec_provider
[ 84%] Built target __idf_esp_hal_gpio
[ 18%] Built target __idf_esp_event
[ 88%] Built target __idf_hal
[ 19%] Built target __idf_esp_driver_uart
[ 93%] Built target __idf_soc
[ 19%] Built target __idf_esp_psram
[ 95%] Built target __idf_xtensa
[ 19%] Built target __idf_esp_ringbuf
[ 97%] Built target __idf_main
[ 20%] Built target __idf_esp_timer
[ 98%] Built target bootloader.elf
[ 21%] Built target __idf_cxx
[100%] Built target gen_bootloader_binary
[ 21%] Built target __idf_pthread
[100%] Built target gen_project_binary
[ 22%] Built target __idf_esp_libc
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 24%] Built target __idf_freertos
[100%] Built target bootloader_check_size
[100%] Built target app
[ 27%] Built target __idf_esp_hw_support
[ 27%] No install step for 'bootloader'
[ 28%] Built target __idf_esp_hal_i2s
[ 28%] Completed 'bootloader'
[ 28%] Built target __idf_esp_hal_touch_sens
[ 29%] Built target bootloader
[ 29%] Built target __idf_esp_hal_pmu
[ 29%] Built target __idf_esp_hal_usb
[ 30%] Built target __idf_esp_hal_gpio
[ 31%] Built target __idf_soc
[ 32%] Built target __idf_heap
[ 33%] Built target __idf_log
[ 33%] Built target __idf_hal
[ 34%] Built target __idf_esp_rom
[ 34%] Built target __idf_esp_common
[ 36%] Built target __idf_esp_system
[ 37%] Built target __idf_spi_flash
[ 38%] Built target __idf_bootloader_support
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
[ 50%] Built target __idf_efuse
[ 50%] Built target __idf_esp_partition
[ 50%] Built target __idf_app_update
[ 50%] Built target __idf_esp_bootloader_format
[ 50%] Built target __idf_esp_app_format
[ 51%] Built target __idf_esp_hal_security
[ 52%] Built target __idf_esp_hal_mspi
[ 52%] Built target __idf_esp_hal_clock
[ 52%] Built target __idf_esp_hal_gpspi
[ 52%] Built target __idf_esp_hal_dma
[ 53%] Built target __idf_esp_stdio
[ 54%] Built target __idf_xtensa
[ 55%] Built target __idf_espressif__cjson
[ 55%] Built target __idf_esp_driver_i2c
[ 55%] Built target __idf_dial_knob
[ 55%] Built target __idf_esp_hal_ledc
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_dial_state
[ 56%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_dial_time
[ 57%] Built target __idf_unity
[ 57%] Built target __idf_esp_hal_lcd
[ 58%] Built target __idf_esp_hal_cam
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_hal_pcnt
[ 59%] Built target __idf_esp_hal_mcpwm
[ 59%] Built target __idf_esp_hal_rmt
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_esp_driver_twai
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_protobuf-c
[ 60%] Built target __idf_esp_hid
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_esp_driver_ledc
[ 61%] Built target __idf_perfmon
[ 62%] Built target __idf_spiffs
[ 63%] Built target __idf_driver
[ 64%] Built target __idf_wear_levelling
[ 64%] Built target __idf_rt
[ 65%] Built target __idf_esp_lcd
[ 65%] Built target __idf_dial_ota
[ 65%] Built target __idf_dial_net
[ 65%] Built target __idf_dial_somnus
[ 66%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_cmock
[ 66%] Built target __idf_esp_driver_pcnt
[ 67%] Built target __idf_esp_driver_mcpwm
[ 67%] Built target __idf_esp_driver_sd_intf
[ 68%] Built target __idf_esp_driver_rmt
[ 69%] Built target __idf_esp_driver_sdspi
[ 69%] Built target __idf_espressif__esp_lcd_sh8601
[ 70%] Built target __idf_dial_pad_discovery
[ 71%] Built target __idf_protocomm
[ 71%] Built target __idf_esp_driver_sdmmc
[ 72%] Built target __idf_esp_local_ctrl
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Built target __idf_fatfs
[ 97%] Built target __idf_dial_display
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Built target __idf_i2c_bsp
[ 97%] Built target __idf_dial_haptics
[ 97%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Built target __ldgen_output_sections.ld
[100%] Built target somnus-dial.elf
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
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
Compressed 22576 bytes to 14462...

Writing at 0x00000000 [                              ]   0.0% 0/14462 bytes... 
Writing at 0x00005830 [==============================] 100.0% 14462/14462 bytes... 
Wrote 22576 bytes (14462 compressed) at 0x00000000 in 0.3 seconds (714.8 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (703.0 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.0 seconds (1448.7 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1610064 bytes to 982976...

Writing at 0x00020000 [                              ]   0.0% 0/982976 bytes... 
Writing at 0x0002b91f [                              ]   1.7% 16384/982976 bytes... 
Writing at 0x00038594 [>                             ]   3.3% 32768/982976 bytes... 
Writing at 0x0004b760 [>                             ]   5.0% 49152/982976 bytes... 
Writing at 0x000531eb [=>                            ]   6.7% 65536/982976 bytes... 
Writing at 0x0005ba4b [=>                            ]   8.3% 81920/982976 bytes... 
Writing at 0x000652f5 [==>                           ]  10.0% 98304/982976 bytes... 
Writing at 0x00070dab [==>                           ]  11.7% 114688/982976 bytes... 
Writing at 0x00083bad [===>                          ]  13.3% 131072/982976 bytes... 
Writing at 0x0008a7a7 [===>                          ]  15.0% 147456/982976 bytes... 
Writing at 0x00093b5c [====>                         ]  16.7% 163840/982976 bytes... 
Writing at 0x0009c1be [====>                         ]  18.3% 180224/982976 bytes... 
Writing at 0x000a47e4 [=====>                        ]  20.0% 196608/982976 bytes... 
Writing at 0x000aa0f1 [=====>                        ]  21.7% 212992/982976 bytes... 
Writing at 0x000af97a [======>                       ]  23.3% 229376/982976 bytes... 
Writing at 0x000b4e4b [======>                       ]  25.0% 245760/982976 bytes... 
Writing at 0x000bac23 [=======>                      ]  26.7% 262144/982976 bytes... 
Writing at 0x000c00d0 [=======>                      ]  28.3% 278528/982976 bytes... 
Writing at 0x000c5ab2 [========>                     ]  30.0% 294912/982976 bytes... 
Writing at 0x000caab0 [========>                     ]  31.7% 311296/982976 bytes... 
Writing at 0x000d003c [=========>                    ]  33.3% 327680/982976 bytes... 
Writing at 0x000d65a1 [=========>                    ]  35.0% 344064/982976 bytes... 
Writing at 0x000db952 [==========>                   ]  36.7% 360448/982976 bytes... 
Writing at 0x000e1382 [==========>                   ]  38.3% 376832/982976 bytes... 
Writing at 0x000e6435 [===========>                  ]  40.0% 393216/982976 bytes... 
Writing at 0x000eb915 [===========>                  ]  41.7% 409600/982976 bytes... 
Writing at 0x000f0fde [============>                 ]  43.3% 425984/982976 bytes... 
Writing at 0x000f6cb1 [============>                 ]  45.0% 442368/982976 bytes... 
Writing at 0x000fbe65 [=============>                ]  46.7% 458752/982976 bytes... 
Writing at 0x00101264 [=============>                ]  48.3% 475136/982976 bytes... 
Writing at 0x00106202 [==============>               ]  50.0% 491520/982976 bytes... 
Writing at 0x0010b31c [==============>               ]  51.7% 507904/982976 bytes... 
Writing at 0x00110cdd [===============>              ]  53.3% 524288/982976 bytes... 
Writing at 0x00116259 [===============>              ]  55.0% 540672/982976 bytes... 
Writing at 0x0011b9a6 [================>             ]  56.7% 557056/982976 bytes... 
Writing at 0x00120e7c [================>             ]  58.3% 573440/982976 bytes... 
Writing at 0x00126461 [=================>            ]  60.0% 589824/982976 bytes... 
Writing at 0x0012bed4 [=================>            ]  61.7% 606208/982976 bytes... 
Writing at 0x001313a8 [==================>           ]  63.3% 622592/982976 bytes... 
Writing at 0x001364b7 [==================>           ]  65.0% 638976/982976 bytes... 
Writing at 0x0013b70b [===================>          ]  66.7% 655360/982976 bytes... 
Writing at 0x001410f3 [===================>          ]  68.3% 671744/982976 bytes... 
Writing at 0x00146642 [====================>         ]  70.0% 688128/982976 bytes... 
Writing at 0x0014bb42 [====================>         ]  71.7% 704512/982976 bytes... 
Writing at 0x00151086 [=====================>        ]  73.3% 720896/982976 bytes... 
Writing at 0x00156af4 [=====================>        ]  75.0% 737280/982976 bytes... 
Writing at 0x0015c2c4 [======================>       ]  76.7% 753664/982976 bytes... 
Writing at 0x001613c1 [======================>       ]  78.3% 770048/982976 bytes... 
Writing at 0x001665b8 [=======================>      ]  80.0% 786432/982976 bytes... 
Writing at 0x0016bf1b [=======================>      ]  81.7% 802816/982976 bytes... 
Writing at 0x00171118 [========================>     ]  83.3% 819200/982976 bytes... 
Writing at 0x001762b1 [========================>     ]  85.0% 835584/982976 bytes... 
Writing at 0x0017b684 [=========================>    ]  86.7% 851968/982976 bytes... 
Writing at 0x001814dd [=========================>    ]  88.3% 868352/982976 bytes... 
Writing at 0x00186bf1 [==========================>   ]  90.0% 884736/982976 bytes... 
Writing at 0x0018d652 [==========================>   ]  91.7% 901120/982976 bytes... 
Writing at 0x001928b5 [===========================>  ]  93.3% 917504/982976 bytes... 
Writing at 0x00197f2b [===========================>  ]  95.0% 933888/982976 bytes... 
Writing at 0x0019e384 [============================> ]  96.7% 950272/982976 bytes... 
Writing at 0x001a3754 [============================> ]  98.3% 966656/982976 bytes... 
Writing at 0x001a9150 [==============================] 100.0% 982976/982976 bytes... 
Wrote 1610064 bytes (982976 compressed) at 0x00020000 in 10.2 seconds (1260.8 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

All four segments written and hash-verified; hard reset via RTS at the end.

## Boot capture

Raw pyserial read (`serial.Serial(port, 115200, timeout=0.2)`, reopening if the port drops) for 40 s, started at 17:26:49 CDT right after the flash returned. File: **`bench-logs/2026-09-05-flash-a1b.log`** (9002 bytes, 158 lines; gitignored). It starts mid-bootloader (esp_image segment 3) because the reader attached ~0.3 s into the boot, as before.

Quoted lines:

```
I (382) boot: Loaded app from partition at offset 0x20000
I (383) boot: Set actual ota_seq=1 in otadata[0]
I (788) app_init: Project name:     somnus-dial
I (792) app_init: App version:      0.1.5
I (796) app_init: Compile time:     Sep  5 2026 17:24:47
I (805) app_init: ESP-IDF:          v6.0
I (1215) ota: boot pending-verify: false
I (2768) dial_somnus: zone mode set to single (One Bed)
I (2768) app: pad connected at http://192.168.1.169:8080
I (2827) ota: boot not pending verification (state 2) -- nothing to do
```

- App version **0.1.5**, compile time **Sep  5 2026 17:24:47** (the clean build; a re-link does not change `__DATE__`/`__TIME__` in the app descriptor), booted from the app at 0x20000 (`ota_seq=1`), pad connected.
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (2768) dial_somnus: zone mode set to single (One Bed). (The only case-insensitive `e (` hit is "single (One Bed)", as in the phase-1 capture.)
- `rst:` lines: **0** — no reboot.
- `W (` lines (none in the requested pattern set):

```
W (1203) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1400) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

- From 21 s into the capture the log shows knob and power activity on the dial — someone at the bench switched side 0 on and swept the setpoint across the range, which is exactly the eyes-on pass this report says it cannot do. Those lines, verbatim:

```
I (21058) dial_somnus: POST /api/power {"side0":{"is_on":true}}
I (24194) dial_somnus: POST /api/target_t {"side0":{"target_t":32}}
I (24809) dial_somnus: POST /api/target_t {"side0":{"target_t":31}}
I (25126) dial_somnus: POST /api/target_t {"side0":{"target_t":30}}
I (25227) dial_somnus: POST /api/target_t {"side0":{"target_t":29}}
I (25780) dial_somnus: POST /api/target_t {"side0":{"target_t":24}}
I (26145) dial_somnus: POST /api/target_t {"side0":{"target_t":23}}
I (26258) dial_somnus: POST /api/target_t {"side0":{"target_t":22}}
I (26654) dial_somnus: POST /api/target_t {"side0":{"target_t":19}}
I (26737) dial_somnus: POST /api/target_t {"side0":{"target_t":17}}
I (26984) dial_somnus: POST /api/target_t {"side0":{"target_t":16}}
I (27376) dial_somnus: POST /api/target_t {"side0":{"target_t":13}}
I (27883) dial_somnus: POST /api/target_t {"side0":{"target_t":12}}
I (28396) dial_somnus: POST /api/target_t {"side0":{"target_t":20}}
I (29115) dial_somnus: POST /api/target_t {"side0":{"target_t":33}}
I (29525) dial_somnus: POST /api/target_t {"side0":{"target_t":35}}
I (29931) dial_somnus: POST /api/target_t {"side0":{"target_t":38}}
I (30340) dial_somnus: POST /api/target_t {"side0":{"target_t":42.3}}
I (32905) dial_somnus: POST /api/target_t {"side0":{"target_t":41.3}}
I (33521) dial_somnus: POST /api/target_t {"side0":{"target_t":40.3}}
I (34286) dial_somnus: POST /api/target_t {"side0":{"target_t":39.3}}
I (34959) dial_somnus: POST /api/target_t {"side0":{"target_t":38.3}}
I (35443) dial_somnus: POST /api/target_t {"side0":{"target_t":37.3}}
I (35801) dial_somnus: POST /api/target_t {"side0":{"target_t":36.3}}
I (36119) dial_somnus: POST /api/target_t {"side0":{"target_t":33.3}}
I (36485) dial_somnus: POST /api/target_t {"side0":{"target_t":30.3}}
I (36895) dial_somnus: POST /api/target_t {"side0":{"target_t":27.3}}
I (36973) dial_somnus: POST /api/target_t {"side0":{"target_t":24.3}}
I (37338) dial_somnus: POST /api/target_t {"side0":{"target_t":23.3}}
I (37817) dial_somnus: POST /api/target_t {"side0":{"target_t":14.3}}
I (38167) dial_somnus: POST /api/target_t {"side0":{"target_t":13.3}}
I (38638) dial_somnus: POST /api/target_t {"side0":{"target_t":12}}
I (40073) dial_somnus: POST /api/target_t {"side0":{"target_t":13}}
I (40376) dial_somnus: POST /api/target_t {"side0":{"target_t":15}}
```

## Git

```
$ git log --oneline -4
40b784a ui(A1b): °C setpoint without a zero decimal; no unit label in relative mode
e2f2996 docs: phase 1 flash report
4b114c5 docs: phase 1 layout report
5bf68a5 ui: layout fixes A1-A5 from the screen audit
$ git diff --stat e2f2996 40b784a
 docs/REPORT-screen-layout-audit.md              |  22 +++++++++++++
 docs/screens/dial-celsius.png                   | Bin 27029 -> 26578 bytes
 docs/screens/dial-night-water.png               | Bin 0 -> 25180 bytes
 docs/screens/dial-relative-max.png              | Bin 26538 -> 25951 bytes
 docs/screens/dial-relative.png                  | Bin 25719 -> 25182 bytes
 firmware/dial-idf/components/dial_ui/scr_dial.c |  40 +++++++++++++++--------
 simulator/main.c                                |  41 ++++++++++++++++++++++--
 7 files changed, 86 insertions(+), 17 deletions(-)
```

A1b commit: **`40b784a818c14d5d53479718cf09a885d40684f9`**. Gate commit: `e2f2996`. Working tree after this task: only this file (`docs/REPORT-layout-a1b.md`) untracked; `bench-logs/` is gitignored. PROJECT_VER still 0.1.5. No tag, no push.

## Deviations from the spec

1. **`idf.py fullclean` before `idf.py build`.** Same reason as the phase-1 flash: an incremental build would have compiled only `scr_dial.c`, making "zero warnings from dial_ui" unprovable for the component as a whole. Same source, same version.
2. **`simulator/main.c` is in the commit.** The spec's "no other files" line and its "add one simulator scenario if none exists" instruction conflict; the scenario needs `main.c`, so it was added there (plus two comment corrections in the same file about the now-absent "LEVEL" suffix). The commit is otherwise `scr_dial.c` + PNGs + the audit report's A1b paragraph, as specified.
3. **`#include <stdlib.h>` added to `scr_dial.c`** for `abs()` — inside the one permitted file, listed for completeness.
4. **One comment re-wrap after the clean build.** The patch had left a 130-column comment line in `render_value()`'s block comment; it was re-wrapped before the commit. The clean-build binary predates that edit; the flash step's own rebuild recompiled `scr_dial.c` from the committed source (comment-only change, no code difference) and that is what was written to the dial. The app descriptor's compile time is the clean build's.
5. **Commit message trailers.** Both commits carry the session's two attribution trailer lines after the spec's title; the titles are verbatim.
6. **The boot capture is not a quiet boot.** From 21 s on it contains the owner's knob activity; the first 21 s are the clean boot the spec asked to check.

Everything else: none.

## Finding — dragged setpoints are fractional in absolute mode (out of scope, not fixed)

The capture's `target_t` sequence 32 → 12 (whole degrees, knob detents) then 42.3 → 41.3 → … → 13.3 → 12 shows a touch drag landed on 42.3 °C and the following detents stepped 1.0 from there until the 12.0 rail clamped it back to the grid. The code agrees: `scr_dial.c`'s release handler snaps to the level grid only `if (s_rel)`; in absolute mode it posts the raw `value_from_point()` dc. So the premise "every Somnus setpoint is a whole degree" holds for the knob but not for the arc drag, and on such a value the °C numeral is "42.3" — four glyphs, unit back on the ring — until a rail or a units change re-grids it. The one-line fix would be to snap the release to whole degrees in absolute mode the way relative mode already snaps to levels (that also stops the pad being asked for 42.3). Not done here: the spec limits this task to the two changes above. Owner's call.

## What could not be verified without eyes on the dial

- °C across the knob range: that "12" … "42" render with the °C unit clear of the ring and handle at every value (only 20 °C is rendered in the simulator; the width argument covers the rest since "42" is the widest two-digit value in this font). The bench log shows the sweep happened; the screen was not seen.
- Relative mode at +15: the bare "+15" with nothing to its right on the real panel (the simulator's `dial-relative-max.png` shows it; hardware unseen).
- The night-face water alternation with a fractional reading: "23.4 / WATER" swapping with "34" every 2 s in the accent on the actual night face (the simulator freezes one phase; timing, colour under the night brightness curve, and the return to "34" on a detent are hardware-only).
- The dragged-fractional case above on the panel (how "42.3 °C" now looks with the unit riding the numeral).
