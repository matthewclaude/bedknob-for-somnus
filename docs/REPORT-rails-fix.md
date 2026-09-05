# REPORT — fix(dial): absolute rails 12-42 whole degrees; knob and drag snap to the grid

Date: 2026-09-05 17:42 CDT. Branch `main`, starting from `40b784a` (A1b) + `42eb3f4` (the A1b report, committed by the gate).
Toolchain ESP-IDF v6.0 via `~/esp/esp-idf/export.sh`; simulator via `cmake --build build`.

## Verdict

**DONE — committed as `db7f9d8`, both routes off the grid closed (rails 120–420 in main.c, knob snaps-then-steps, drag release snaps), all six requested simulator cases driven through the real `on_knob` and pointer-indev paths and rendered as specified, `idf.py fullclean && idf.py build` with zero compiler warnings, wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.5, compile time 17:39:59, no assert/panic/abort/Guru/`E (` lines, no reset).** No tag, no version bump, no push. The one behavioural note for the owner: from an app-set 33.5 the spec's own formula makes one detent **up** read 35 (34 is the snap, 35 is the step), one detent down 33 — never 34.5; see "Not verified", item 3.

## Gate

```
$ git add docs/REPORT-layout-a1b.md && git commit -m "docs: A1b report"
[main 42eb3f4] docs: A1b report
$ git --no-optional-locks status --short
(empty)
$ git log --oneline -2
42eb3f4 docs: A1b report
40b784a ui(A1b): °C setpoint without a zero decimal; no unit label in relative mode
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")
```

Gate passed; proceeded.

## The fix — full diff (`git diff 42eb3f4 db7f9d8`, text files; the six new PNGs are binary and listed in the stat below)

```diff
diff --git a/docs/REPORT-screen-layout-audit.md b/docs/REPORT-screen-layout-audit.md
index f14de2d..1156586 100644
--- a/docs/REPORT-screen-layout-audit.md
+++ b/docs/REPORT-screen-layout-audit.md
@@ -498,3 +498,29 @@ to the A1 renders. A new scenario, `dial-night-water.png`, renders the
 Number-only night face mid water-phase in °C with the water at 23.4 °C and
 the setpoint at 34 °C: "23.4" in the accent with WATER above it, proving
 the tenth survives. Report: `docs/REPORT-layout-a1b.md`.
+
+**Range-stop / drag grid bug (2026-09-05, found on hardware during the
+phase-1 eyes-on; the drag half in the A1b bench log):** in absolute mode the
+setpoint could walk off the whole-degree grid by two routes. (a) Knob:
+`main.c` seeded the dial's rails from the API's accept range, `{ 120, 423 }`,
+and `on_knob()`'s absolute branch stepped `s_shown_dc ± 10` and clamped to
+that 423, so one detent above 42.0 landed on 42.3 and every later value
+carried the 3 until the 12.0 rail reset it. (b) Drag: the handle's release
+handler in `scr_dial.c` snapped to the grid only in relative mode and posted
+the raw `value_from_point()` tenth otherwise (bench log: 42.3, then 41.3 …
+13.3 by knob). Fix, in one commit: the rails are now `{ 120, 420 }` — the
+Somnus app's whole-degree scale, identical to `DIAL_REL_MIN_DC/MAX_DC`; the
+pad accepts up to 42.3 but the dial never sets it (dial_state.h comments
+updated to match, dial_somnus.h left as the API description). `on_knob()`
+snaps the displayed value to its nearest whole degree before stepping
+(round-half-up, then the existing clamp), so an off-grid start such as 42.3
+or an app-set 33.5 lands on the grid on the first detent and the range-stop
+test still fires at both rails from on-grid values. The release handler
+snaps to the nearest whole degree in absolute mode exactly as it snaps to a
+level in relative mode, before the render and the post. Six simulator
+scenarios drive it through the real `on_knob` and pointer-indev paths:
+`rails-420-up.png` "42" (pinned, nothing posted), `rails-420-up-down.png`
+"41" (posts 410), `rails-423.png` "42.3" with the handle pinned at the rail
+end, `rails-423-up.png` "42" (posts 420 — not pinned, by design),
+`rails-drag-337-live.png` "33.7" mid-drag, `rails-drag-337.png` "34" on
+release (posts 340). Report: `docs/REPORT-rails-fix.md`.
diff --git a/firmware/dial-idf/components/dial_state/dial_state.h b/firmware/dial-idf/components/dial_state/dial_state.h
index 5f50d07..28dad78 100644
--- a/firmware/dial-idf/components/dial_state/dial_state.h
+++ b/firmware/dial-idf/components/dial_state/dial_state.h
@@ -81,8 +81,10 @@ static inline int dial_c_to_f(float c) { return (int)lroundf(c * 1.8f + 32.0f);
 // convert a whole-°F value back into the canonical unit.)
 static inline int dial_dc_to_f(int dc) { return (int)lroundf((float)dc * 0.18f + 32.0f); }
 
-// ABSOLUTE display range, tenths of °C. Superseded at runtime by the pad's
-// own fixed range (dial_somnus.h: 12.0-42.3°C, seeded once into
+// ABSOLUTE display range, tenths of °C. Superseded at runtime by the dial's
+// fixed rails (12.0-42.0°C, the Somnus app's whole-degree scale — the same
+// rails as relative mode; the pad itself accepts up to 42.3 per
+// dial_somnus.h but the dial never sets it — seeded once into
 // app_state_t.temp_min_dc/temp_max_dc right after the first successful
 // connect — see main.c's worker_task) — screens read it through
 // dial_state_temp_min_dc()/dial_state_temp_max_dc() below, never these
@@ -324,7 +326,8 @@ typedef struct {
     // rounds the pad's reported float to the nearest 0.1°C on the way in, the
     // one deliberate, one-directional C(float)->C(int tenths) quantization
     // this design makes; nothing downstream of this field ever converts
-    // through °F. Spec range 12.0-42.3°C = 120-423 here.
+    // through °F. The dial's own rails are 12.0-42.0°C = 120-420 here (the
+    // pad accepts up to 42.3 and may report a value this dial never set).
     int   temp_dc;
     float actual_c;     // measured water temp (current_c); <0 = unknown (mirrors
                         // somnus_side_state_t.has_current — the pad reports no
@@ -423,12 +426,14 @@ typedef struct {
     // again with no further plumbing. Not session-optimistic like Orion's
     // version was; there's no write path to be optimistic about.
     bool    away;
-    // Absolute temperature range, tenths of °C, mirrored here once
+    // Absolute temperature range, tenths of °C, seeded here once
     // worker_task connects successfully. Somnus's pad has no discovery
-    // call to report its own rails (unlike Orion's list_devices), but the
-    // local_api spec fixes them at 12.0-42.3°C regardless of pad -- see
-    // dial_somnus.h. This, not the DIAL_TEMP_MIN_DC/MAX_DC constants, is what
-    // the arc range / knob clamp / drag clamp use in ABSOLUTE mode.
+    // call to report its own rails (unlike Orion's list_devices); the dial
+    // uses the Somnus app's whole-degree scale, 12.0-42.0°C, identical to
+    // the relative rails (DIAL_REL_MIN_DC/MAX_DC) -- the pad accepts up to
+    // 42.3 (local_api spec, dial_somnus.h) but the dial never sets it. This,
+    // not the DIAL_TEMP_MIN_DC/MAX_DC constants, is what the arc range /
+    // knob clamp / drag clamp use in ABSOLUTE mode.
     // -1 = not yet known (fresh boot, before the first successful connect)
     // -- dial_state_temp_min_dc()/_max_dc() below fall back to the
     // DIAL_TEMP_MIN_DC/MAX_DC constants then.
diff --git a/firmware/dial-idf/components/dial_ui/scr_dial.c b/firmware/dial-idf/components/dial_ui/scr_dial.c
index 5332a66..ff59086 100644
--- a/firmware/dial-idf/components/dial_ui/scr_dial.c
+++ b/firmware/dial-idf/components/dial_ui/scr_dial.c
@@ -897,11 +897,23 @@ static void handle_event_cb(lv_event_t *e)
     // LV_EVENT_RELEASED / LV_EVENT_PRESS_LOST — end of the drag. "The knob
     // wins" (§3a): starts the post-release lock window here; s_dragging
     // itself already covered the drag proper.
+    //
+    // Commit exactly on the grid in BOTH scales: the relative-level grid
+    // when s_rel, else the nearest whole degree (round-half-up; dc >=
+    // s_arc_min >= 120, never negative). value_from_point() maps the finger
+    // to any tenth, and until 2026-09-05 absolute mode posted that raw tenth
+    // — the bench log showed 42.3 from a drag, then 41.3, 40.3 … by knob.
+    // The pad must never be asked for a tenth from this dial; the snap
+    // happens before the render and the post so the numeral, the fill, the
+    // handle and the bed all land on the same whole degree.
     s_dragging = false;
     s_last_interact_ms = lv_tick_get();
     int dc = s_shown_dc;
-    if (s_rel) {                          // commit exactly on the relative-level grid
-        dc = dial_rel_to_dc(dial_rel_from_dc(dc));
+    if (s_rel) dc = dial_rel_to_dc(dial_rel_from_dc(dc));
+    else       dc = ((dc + 5) / 10) * 10;
+    if (dc < s_arc_min) dc = s_arc_min;   // 42.0 rounds to itself; the clamp only
+    if (dc > s_arc_max) dc = s_arc_max;   // matters if a rail ever sits off-grid
+    if (dc != s_shown_dc) {
         s_shown_dc = dc;
         lv_arc_set_value(s_arc, dc);
         render_numeral(dc);
@@ -1411,16 +1423,27 @@ static bool on_knob(int detents)
     // Absolute: one detent = 10 tenths = exactly 1.0°C (the Q1 units fix's
     // design decision — matches the Somnus app's own whole-degree scale, so
     // the dial and the app never disagree about the setpoint, even though
-    // the pad itself accepts finer values), clamped to s_arc_min/s_arc_max —
-    // the same device-reported (or fallback) rails configure_arc_range() just
-    // set, read back here rather than re-deriving them, since this branch
-    // only runs when s_rel is false (so s_arc_min/max already hold the
-    // absolute range, not the relative one).
+    // the pad itself accepts finer values), stepped from the DISPLAYED value
+    // snapped to its nearest whole degree first — the same rule
+    // dial_rel_step applies to levels. Without the snap an off-grid start
+    // (a 42.3 posted by an older build's rail, or a 33.5 set from the app)
+    // carried its tenth through every later detent until a rail reset it.
+    // Round-half-up: s_shown_dc >= s_arc_min >= 120 here, never negative,
+    // so plain integer division is a true floor. Then clamped to
+    // s_arc_min/s_arc_max — the rails configure_arc_range() just set (120/420
+    // from main.c, or the fallback), read back here rather than re-derived,
+    // since this branch only runs when s_rel is false (so s_arc_min/max
+    // already hold the absolute range, not the relative one). The range-stop
+    // test below (nf == s_shown_dc) still fires at both rails from an
+    // on-grid value; from an off-grid value it deliberately does NOT
+    // (423 + up -> base 420 -> 430 -> clamp 420 != 423): that detent snaps
+    // the display and the bed onto the grid instead of nudging.
     int nf;
     if (s_rel) {
         nf = dial_rel_step(s_shown_dc, detents);
     } else {
-        nf = s_shown_dc + detents * 10;
+        int base = ((s_shown_dc + 5) / 10) * 10;   // nearest whole degree
+        nf = base + detents * 10;
         if (nf < s_arc_min) nf = s_arc_min;
         if (nf > s_arc_max) nf = s_arc_max;
     }
diff --git a/firmware/dial-idf/main/main.c b/firmware/dial-idf/main/main.c
index 7474efb..e61e93e 100644
--- a/firmware/dial-idf/main/main.c
+++ b/firmware/dial-idf/main/main.c
@@ -1051,13 +1051,18 @@ static void worker_task(void *arg)
     dial_somnus_set_zone_mode(dial_state_get_zone_mode());
     ESP_LOGI(TAG, "pad connected at %s", pad_url);
 
-    // Fixed pad range (local_api spec, dial_somnus.h) — no discovery call to
-    // report it, unlike Orion's list_devices, so seed it once here instead
-    // of per-poll. Hardcoded directly in the canonical unit (tenths of °C):
-    // 12.0-42.3°C == 120-423dc, no conversion needed or wanted — there is no
-    // °F anywhere upstream of this to convert from.
+    // Fixed dial rails — no discovery call to report them, unlike Orion's
+    // list_devices, so seed them once here instead of per-poll. Hardcoded
+    // directly in the canonical unit (tenths of °C): 12.0-42.0°C == 120-420dc,
+    // no conversion needed or wanted — there is no °F anywhere upstream of
+    // this to convert from. These are the Somnus app's own whole-degree scale
+    // 12-42, identical to the relative rails (DIAL_REL_MIN_DC/MAX_DC): the
+    // pad accepts up to 42.3 (local_api spec, dial_somnus.h) but the dial
+    // never sets it — seeding the API's 423 here put one detent above 42.0
+    // on 42.3 and every value after it off the whole-degree grid until the
+    // 12.0 rail reset it (found on hardware 2026-09-05).
     {
-        temp_range_t range = { 120, 423 };
+        temp_range_t range = { 120, 420 };
         dial_state_commit(mut_temp_range, &range);
     }
 
diff --git a/simulator/main.c b/simulator/main.c
index 1d81c75..e2ac608 100644
--- a/simulator/main.c
+++ b/simulator/main.c
@@ -122,6 +122,29 @@ static void sim_tap(lv_coord_t x, lv_coord_t y)
     pump_ms(60);
 }
 
+// A press at (x0,y0) that travels to (x1,y1) and is HELD there — the same
+// pointer indev as sim_tap, so LV_EVENT_PRESSED lands on whatever sits at
+// the start point and LV_EVENT_PRESSING follows the point. Two intermediate
+// steps so the move looks like a finger, not a teleport. sim_release() ends
+// it (LV_EVENT_RELEASED). Used to drive scr_dial's setpoint handle.
+static void sim_drag_to(lv_coord_t x0, lv_coord_t y0, lv_coord_t x1, lv_coord_t y1)
+{
+    s_ptr_x = x0; s_ptr_y = y0;
+    s_ptr_pressed = true;
+    pump_ms(60);
+    for (int i = 1; i <= 3; i++) {
+        s_ptr_x = x0 + (x1 - x0) * i / 3;
+        s_ptr_y = y0 + (y1 - y0) * i / 3;
+        pump_ms(60);
+    }
+}
+
+static void sim_release(void)
+{
+    s_ptr_pressed = false;
+    pump_ms(60);
+}
+
 /* ---- PNG output ----------------------------------------------------------*/
 
 static void ensure_dir(const char *path)
@@ -447,6 +470,105 @@ static void scenario_dial_night_water(void)
     dial_palette_set_night(false);
 }
 
+/* ---- absolute-mode rails + whole-degree grid (fix(dial), 2026-09-05) ----
+ * The dial's absolute rails are 120..420 (main.c seeds them on connect —
+ * the Somnus app's whole-degree scale, same as the relative rails). The
+ * sim's baseline leaves temp_min_dc/temp_max_dc at -1 (fallback 100..450),
+ * so these scenarios seed the connected values themselves and put them
+ * back after, leaving every other dial render untouched. °C so the numeral
+ * shows the tenth if one survives. ui_temp_dc is cleared too: sim_knob /
+ * a drag on the dial post through dial_state_set_ui_temp, and on_state
+ * prefers that over temp_dc, so a stale one from the previous scenario
+ * would otherwise win. Same SCR_MENU bounce as scenario_dial_celsius. */
+static void rails_setup(int temp_dc)
+{
+    apply_baseline();
+    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
+    pump_ms(100);
+    app_state_t *st = sim_state_ptr();
+    st->units_c = true;
+    st->temp_min_dc = 120;
+    st->temp_max_dc = 420;
+    st->ui_temp_dc[ZONE_A] = -1;
+    st->ui_temp_dc[ZONE_B] = -1;
+    st->ui_zone = ZONE_A;
+    zone_state_t *a = &st->zones[ZONE_A];
+    a->on = true;
+    a->temp_dc = temp_dc;
+    a->actual_c = 26.0f;   // below every setpoint used here: heating
+    st->generation++;
+    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
+    pump_ms(600);
+}
+
+static void rails_teardown(void)
+{
+    app_state_t *st = sim_state_ptr();
+    st->temp_min_dc = -1;
+    st->temp_max_dc = -1;
+    st->ui_temp_dc[ZONE_A] = -1;
+    st->ui_temp_dc[ZONE_B] = -1;
+    st->generation++;
+}
+
+// Screen point on the ring at a setpoint, for the pointer indev. Mirrors
+// scr_dial.c's position_handle / value_from_point mapping: 270° sweep from
+// 135° (lower-left) clockwise, radius = ARC_R − arc width / 2 = 165 − 8 (the
+// handle's ext_click_area of 14 px makes a few px of error irrelevant).
+static void ring_point(int dc, int lo, int hi, lv_coord_t *x, lv_coord_t *y)
+{
+    float frac = (float)(dc - lo) / (float)(hi - lo);
+    float ang  = (135.0f + frac * 270.0f) * 3.14159265f / 180.0f;
+    *x = (lv_coord_t)(180 + lroundf(157.0f * cosf(ang)));
+    *y = (lv_coord_t)(180 + lroundf(157.0f * sinf(ang)));
+}
+
+// From the 42.0 rail: one detent up is the range stop ("42" stays, the
+// numeral nudges), then one detent down reads "41". Before the fix the
+// rail was 423 and the first detent read "42.3".
+static void scenario_rails_420(void)
+{
+    rails_setup(420);
+    sim_knob(1);
+    pump_until_idle(800);   // the range-stop nudge is an lv_anim
+    snapshot("rails-420-up");
+    sim_knob(-1);
+    pump_until_idle(800);
+    snapshot("rails-420-up-down");
+    rails_teardown();
+}
+
+// A pad setpoint of 42.3 (set by an older build or the app) above the
+// 42.0 rail: renders as "42.3" with the handle pinned at the rail end
+// (position_handle clamps frac to 1, lv_arc clamps the value), and the
+// first detent — up, against the rail — snaps it onto the grid: "42".
+static void scenario_rails_423(void)
+{
+    rails_setup(423);
+    snapshot("rails-423");
+    sim_knob(1);
+    pump_until_idle(800);
+    snapshot("rails-423-up");
+    rails_teardown();
+}
+
+// A handle drag that lands off-grid: press the handle at 30.0, drag it to
+// the ring angle for 33.7 and hold — the live numeral follows the finger
+// ("33.7"); release — the commit snaps to the nearest whole degree ("34").
+static void scenario_rails_drag_337(void)
+{
+    rails_setup(300);
+    lv_coord_t x0, y0, x1, y1;
+    ring_point(300, 120, 420, &x0, &y0);
+    ring_point(337, 120, 420, &x1, &y1);
+    sim_drag_to(x0, y0, x1, y1);
+    snapshot("rails-drag-337-live");
+    sim_release();
+    pump_until_idle(800);
+    snapshot("rails-drag-337");
+    rails_teardown();
+}
+
 // Also documents the M7 permanent "Update" row (replaces the M6 conditional
 // "Install X.Y.Z" row — confirmation moved into SCR_UPDATE itself, this row
 // is now pure navigation): sets the OTA status to available with a pending
@@ -1006,6 +1128,9 @@ int main(void)
     scenario_dial_celsius();
     scenario_dial_relative_max();
     scenario_dial_night_water();
+    scenario_rails_420();
+    scenario_rails_423();
+    scenario_rails_drag_337();
     scenario_menu();
     scenario_update();
     scenario_update_prompt();
```

Notes on the three code parts:

1. **`main.c`** — `range = { 120, 420 }`; the comment now says these are the Somnus app's whole-degree scale 12–42, identical to the relative rails, and that the pad accepts up to 42.3 but the dial never sets it. `dial_state.h`'s three comments that said "12.0-42.3°C" (the range macro block, `temp_dc`'s field comment, and `temp_min_dc/temp_max_dc`'s) say the same; the one at line ~117 describing the API's accepted range in the relative-scale history is about the API and was left. `dial_somnus.h` untouched.
2. **`on_knob()` absolute branch** — `base = ((s_shown_dc + 5) / 10) * 10; nf = base + detents * 10;` then the existing clamp. Confirmed by simulation (below): from 420, +1 → nf 420 == s_shown_dc → pinned (nudge, nothing posted); from 120, −1 → same by symmetry (clamp to 120 == 120); from 423, +1 → base 420 → 430 → clamp 420 ≠ 423 → not pinned, renders "42", posts 420. `s_shown_dc ≥ s_arc_min ≥ 120` in this branch, so the round-half-up is a true floor.
3. **Release handler** — the `if (s_rel)` block became a two-way snap: level grid when `s_rel`, else `((dc + 5) / 10) * 10`, followed by a clamp to `s_arc_min/max` (a no-op today — 420 rounds to itself — kept so an off-grid rail could never post) and a re-render only if the snap moved the value. Then the unchanged `if (dc != s_press_dc) post_temp_for(...)`. Consequence worth knowing: a press-and-release on the handle that does not move, starting from an off-grid value such as 42.3, now snaps and posts 42.0 (previously nothing was posted). That is the intended "the pad must never be asked for a tenth from this dial".

## Step 4 — every reader of `dial_state_temp_max_dc()` / `s_arc_max` / `s_arc_min` (post-fix line numbers)

`scr_dial.c`:

| Line | Function | Use | 420 vs 423 |
|---|---|---|---|
| 133 | (static) | `s_arc_min = -1, s_arc_max = -1` declaration | n/a |
| 292–294 | `level_angle()` | clamps the WATER value to the rails and maps it to the 270° sweep | fine — a water reading above 42.0 (or a stale 42.3 setpoint used as `target_dc`) clamps to the rail end |
| 308–313 | `configure_arc_range()` | `mn/mx = s_rel ? DIAL_REL_* : dial_state_temp_*_dc(st)`, `lv_arc_set_range`, caches into `s_arc_min/max` | fine — the arc endpoint is now 420 = the relative rail, so the two scales share one geometry; `lv_arc` clamps a value of 423 to 420 |
| 328, 334 | `position_handle()` | early-out if the range is empty; `frac = (dc − min)/(max − min)` clamped to 0..1 | fine — 423 gives frac 1.01 → clamped to 1 → handle at the rail end (`rails-423.png`) |
| 355 | `value_from_point()` | maps the finger angle to `min + frac·(max − min)` | fine — lands on any tenth in 120..420; the release snap (below) grids it |
| 914–915 | `handle_event_cb()` release | new clamp after the snap | fine (no-op with on-grid rails) |
| 1002 | `create()` | resets `s_arc_min/max = -1` so `configure_arc_range` re-applies on the new arc | n/a |
| 1447–1448 | `on_knob()` | clamp after snap-and-step | fine — 420 is on the grid, so the clamp never produces a tenth |

Elsewhere (no behaviour change): `main.c` 487–488 (`mut_temp_range` copies the seeded pair), `dial_state.h` 762–768 (the two accessors with the 100/450 fallback), `dial_state.c` 162–163 and `simulator/sim_state.c` 43–44 (the −1 "unknown" seed), `simulator/main.c` 490–491 / 507–508 (the new scenarios seeding 120/420 and putting −1 back). No other component reads the rails.

**Existing 42.3 on the pad** (older build, or set in the app): `rails-423.png` — the numeral reads "42.3" (A1b keeps a non-zero tenth), the arc fill is clamped at full sweep by `lv_arc`, the handle sits at the rail end via the `frac` clamp, nothing is posted by the render; the first detent (`rails-423-up.png`) posts 420 and reads "42". A drag from it would snap on release the same way. The fallback rails (100/450, before the first successful connect) are unchanged and off-grid-free too.

## Simulator

The harness can drive both paths: `sim_knob()` already drained detents into the active screen's `on_knob`, and the pointer indev used by `scr_passkey`'s pre-fill tap gives `LV_EVENT_PRESSED/PRESSING/RELEASED` on whatever sits at the point — so a new `sim_drag_to()` (press, three intermediate moves, hold) + `sim_release()` drives the setpoint handle exactly as a finger does. The rails scenarios seed `temp_min_dc/temp_max_dc = 120/420` themselves (the sim's baseline leaves them at −1 → the 100/450 fallback) and put −1 back afterwards, so all 41 existing PNGs are byte-identical to `42eb3f4` (`git status` shows only the six new files). Handle position is computed by `ring_point()` mirroring `position_handle()` (radius 165 − 8).

Raw `cmake --build build && ./build/dial_sim` output — the `[cmd] SET_TEMP …` lines are the sim's `dial_cmd_post` stub printing what the dial would have asked the pad for:

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/sim_state.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/stubs.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_router.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_screens.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_connecting.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_setup.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_dial.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_netpick.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_passkey.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_menu.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_wifi.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_about.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_updating.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update_prompt.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_welcome.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_sidepick.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_settings.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_timezone.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_discovery.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_address.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_adjust_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness_menu.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_mode.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_face.c.o
[ 48%] Linking C executable dial_sim
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
done: 47 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
RUN_EXIT=0
```

| Scenario | PNG | Numeral | Posted |
|---|---|---|---|
| start 420, +1 detent | `rails-420-up.png` | **"42"** (pinned; numeral nudge settled) | nothing — range stop |
| … then −1 detent | `rails-420-up-down.png` | **"41"** | `SET_TEMP temp_dc=410` |
| start 423 (stale pad value) | `rails-423.png` | "42.3", handle at the rail end, fill full | nothing |
| … +1 detent | `rails-423-up.png` | **"42"** | `SET_TEMP temp_dc=420` (snapped, not pinned — by design) |
| drag handle 30.0 → ring angle for 33.7, held | `rails-drag-337-live.png` | "33.7" (live, follows the finger) | nothing yet |
| … release | `rails-drag-337.png` | **"34"** | `SET_TEMP temp_dc=340` |

The six PNGs were eyeballed (numeral, unit, handle, fill) and match the table. 47 screens rendered in total.

## `idf.py fullclean && idf.py build` — raw tail (last 40 lines)

Exit 0; 31 `dial_ui` objects compiled.

```
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_20.c.obj
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
somnus-dial.bin binary size 0x189190 bytes. Smallest app partition is 0x400000 bytes. 0x276e70 bytes (62%) free.
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

Every line of the build log containing "warning" (case-insensitive):

```
4: CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
11: This warning is for project developers.  Use -Wno-author or -Wno-deprecated
```

Both are the one CMake deprecation note about `cmake_minimum_required` in the project `CMakeLists.txt` (pre-existing, not a compiler warning). Compiler warnings: **none**. `dial_ui` warnings: **none**.

App descriptor (`esptool image-info build/somnus-dial.bin`): App version **0.1.5**, Compile time **Sep  5 2026 17:39:59**.

## Port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

Native USB-Serial/JTAG port, no CH340 — no "flip the plug".

## `idf.py -p /dev/cu.usbmodem83401 flash` — raw output

Exit 0. Nothing was recompiled by the flash step (no `Building C object` lines — the clean build above is what went on the wire). Warning lines in the flash log: none. From `Executing action: flash` to the end; esptool's terminal control sequences and in-place progress rewrites stripped, leaving the final state of each progress line.

```
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target blank_ota_data
[  0%] Built target _project_elf_src
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_https_ota
[  0%] Built target __idf_esp_http_server
[  1%] Built target bootloader_ld_in_preprocess
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
[ 15%] Built target __idf_vfs
[ 71%] Built target __idf_esp_hal_gpspi
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Built target __idf_esp_hal_dma
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
[ 56%] Built target __idf_unity
[ 56%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_dial_state
[ 56%] Built target __idf_esp_hal_lcd
[ 56%] Built target __idf_esp_hal_ledc
[ 56%] Built target __idf_esp_driver_i2c
[ 56%] Built target __idf_dial_time
[ 57%] Built target __idf_esp_driver_spi
[ 57%] Built target __idf_esp_driver_gptimer
[ 57%] Built target __idf_dial_knob
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_driver_touch_sens
[ 58%] Built target __idf_esp_hal_mcpwm
[ 58%] Built target __idf_esp_hal_pcnt
[ 58%] Built target __idf_esp_hal_rmt
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_eth
[ 59%] Built target __idf_esp_driver_twai
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_hid
[ 60%] Built target __idf_perfmon
[ 60%] Built target __idf_protobuf-c
[ 61%] Built target __idf_wear_levelling
[ 62%] Built target __idf_esp_https_server
[ 62%] Built target __idf_esp_driver_ledc
[ 63%] Built target __idf_esp_lcd
[ 64%] Built target __idf_dial_ota
[ 64%] Built target __idf_driver
[ 64%] Built target __idf_rt
[ 65%] Built target __idf_spiffs
[ 65%] Built target __idf_cmock
[ 65%] Built target __idf_dial_net
[ 65%] Built target __idf_dial_somnus
[ 66%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_esp_driver_pcnt
[ 66%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_esp_driver_mcpwm
[ 69%] Built target __idf_esp_driver_rmt
[ 69%] Built target __idf_esp_driver_sdspi
[ 70%] Built target __idf_protocomm
[ 71%] Built target __idf_dial_pad_discovery
[ 71%] Built target __idf_espressif__esp_lcd_sh8601
[ 71%] Built target __idf_esp_driver_sdmmc
[ 72%] Built target __idf_esp_local_ctrl
[ 72%] Built target __idf_fatfs
[ 97%] Built target __idf_lvgl__lvgl
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
somnus-dial.bin binary size 0x189190 bytes. Smallest app partition is 0x400000 bytes. 0x276e70 bytes (62%) free.
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
Compressed 22576 bytes to 14460...

Writing at 0x00000000 [                              ]   0.0% 0/14460 bytes... 
Writing at 0x00005830 [==============================] 100.0% 14460/14460 bytes... 
Wrote 22576 bytes (14460 compressed) at 0x00000000 in 0.3 seconds (711.0 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (766.3 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.0 seconds (1480.9 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1610128 bytes to 983057...

Writing at 0x00020000 [                              ]   0.0% 0/983057 bytes... 
Writing at 0x0002b916 [                              ]   1.7% 16384/983057 bytes... 
Writing at 0x0003858d [                              ]   3.3% 32768/983057 bytes... 
Writing at 0x0004b755 [>                             ]   5.0% 49152/983057 bytes... 
Writing at 0x000531e9 [>                             ]   6.7% 65536/983057 bytes... 
Writing at 0x0005ba47 [=>                            ]   8.3% 81920/983057 bytes... 
Writing at 0x000652f0 [=>                            ]  10.0% 98304/983057 bytes... 
Writing at 0x00070da0 [==>                           ]  11.7% 114688/983057 bytes... 
Writing at 0x00083ba4 [==>                           ]  13.3% 131072/983057 bytes... 
Writing at 0x0008a79f [===>                          ]  15.0% 147456/983057 bytes... 
Writing at 0x00093b54 [===>                          ]  16.7% 163840/983057 bytes... 
Writing at 0x0009c1cc [====>                         ]  18.3% 180224/983057 bytes... 
Writing at 0x000a47d9 [====>                         ]  20.0% 196608/983057 bytes... 
Writing at 0x000aa0e2 [=====>                        ]  21.7% 212992/983057 bytes... 
Writing at 0x000af96b [=====>                        ]  23.3% 229376/983057 bytes... 
Writing at 0x000b4e34 [======>                       ]  25.0% 245760/983057 bytes... 
Writing at 0x000bac18 [======>                       ]  26.7% 262144/983057 bytes... 
Writing at 0x000c00bd [=======>                      ]  28.3% 278528/983057 bytes... 
Writing at 0x000c5a9f [=======>                      ]  30.0% 294912/983057 bytes... 
Writing at 0x000caa8f [========>                     ]  31.7% 311296/983057 bytes... 
Writing at 0x000d0026 [========>                     ]  33.3% 327680/983057 bytes... 
Writing at 0x000d6554 [=========>                    ]  35.0% 344064/983057 bytes... 
Writing at 0x000db948 [=========>                    ]  36.7% 360448/983057 bytes... 
Writing at 0x000e1371 [==========>                   ]  38.3% 376832/983057 bytes... 
Writing at 0x000e641d [==========>                   ]  40.0% 393216/983057 bytes... 
Writing at 0x000eb8e0 [===========>                  ]  41.7% 409600/983057 bytes... 
Writing at 0x000f0fbb [===========>                  ]  43.3% 425984/983057 bytes... 
Writing at 0x000f6c98 [============>                 ]  45.0% 442368/983057 bytes... 
Writing at 0x000fbe58 [============>                 ]  46.7% 458752/983057 bytes... 
Writing at 0x0010124b [=============>                ]  48.3% 475136/983057 bytes... 
Writing at 0x001061f8 [=============>                ]  50.0% 491520/983057 bytes... 
Writing at 0x0010b30a [==============>               ]  51.7% 507904/983057 bytes... 
Writing at 0x00110cb8 [==============>               ]  53.3% 524288/983057 bytes... 
Writing at 0x00116243 [===============>              ]  55.0% 540672/983057 bytes... 
Writing at 0x0011b986 [===============>              ]  56.7% 557056/983057 bytes... 
Writing at 0x00120e75 [================>             ]  58.3% 573440/983057 bytes... 
Writing at 0x00126444 [================>             ]  60.0% 589824/983057 bytes... 
Writing at 0x0012beb4 [=================>            ]  61.7% 606208/983057 bytes... 
Writing at 0x0013137a [=================>            ]  63.3% 622592/983057 bytes... 
Writing at 0x0013649e [==================>           ]  65.0% 638976/983057 bytes... 
Writing at 0x0013b6d1 [==================>           ]  66.7% 655360/983057 bytes... 
Writing at 0x001410d7 [===================>          ]  68.3% 671744/983057 bytes... 
Writing at 0x00146616 [===================>          ]  70.0% 688128/983057 bytes... 
Writing at 0x0014bb2d [====================>         ]  71.7% 704512/983057 bytes... 
Writing at 0x0015105b [====================>         ]  73.3% 720896/983057 bytes... 
Writing at 0x00156adf [=====================>        ]  75.0% 737280/983057 bytes... 
Writing at 0x0015c298 [=====================>        ]  76.7% 753664/983057 bytes... 
Writing at 0x00161391 [======================>       ]  78.3% 770048/983057 bytes... 
Writing at 0x001665a3 [======================>       ]  80.0% 786432/983057 bytes... 
Writing at 0x0016bf0a [=======================>      ]  81.7% 802816/983057 bytes... 
Writing at 0x001710f6 [=======================>      ]  83.3% 819200/983057 bytes... 
Writing at 0x00176284 [========================>     ]  85.0% 835584/983057 bytes... 
Writing at 0x0017b65f [========================>     ]  86.7% 851968/983057 bytes... 
Writing at 0x001814be [=========================>    ]  88.3% 868352/983057 bytes... 
Writing at 0x00186bd3 [=========================>    ]  90.0% 884736/983057 bytes... 
Writing at 0x0018d618 [==========================>   ]  91.7% 901120/983057 bytes... 
Writing at 0x00192892 [==========================>   ]  93.3% 917504/983057 bytes... 
Writing at 0x00197ee5 [===========================>  ]  95.0% 933888/983057 bytes... 
Writing at 0x0019e34b [===========================>  ]  96.7% 950272/983057 bytes... 
Writing at 0x001a3720 [============================> ]  98.3% 966656/983057 bytes... 
Writing at 0x001a9185 [============================> ] 100.0% 983040/983057 bytes... 
Writing at 0x001a9190 [==============================] 100.0% 983057/983057 bytes... 
Wrote 1610128 bytes (983057 compressed) at 0x00020000 in 10.2 seconds (1259.5 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

All four segments hash-verified; hard reset via RTS.

## Boot capture

Raw pyserial read at 115200 for 40 s from 17:41:38 CDT, right after the flash returned. File: **`bench-logs/2026-09-05-flash-rails.log`** (6682 bytes, 125 lines; gitignored). Starts mid-bootloader (segment 3) as before. A quiet boot this time — no knob or drag activity in the window (0 `POST /api` lines).

```
I (379) boot: Loaded app from partition at offset 0x20000
I (379) boot: Set actual ota_seq=1 in otadata[0]
I (785) app_init: Project name:     somnus-dial
I (789) app_init: App version:      0.1.5
I (793) app_init: Compile time:     Sep  5 2026 17:39:59
I (802) app_init: ESP-IDF:          v6.0
I (1212) ota: boot pending-verify: false
I (5715) dial_somnus: zone mode set to single (One Bed)
I (5715) app: pad connected at http://192.168.1.169:8080
I (6125) ota: boot not pending verification (state 2) -- nothing to do
```

- App version **0.1.5**, compile time **Sep  5 2026 17:39:59** — the clean build. Booted from the app at 0x20000 (`ota_seq=1`), pad connected at 5.7 s (Wi-Fi took longer to associate than on the A1b boot; nothing else differs).
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (5715) dial_somnus: zone mode set to single (One Bed) (the only case-insensitive `e (` hit is "single (One Bed)").
- `rst:` lines: **0** — no reboot.
- `W (` lines, none in the requested pattern set:

```
W (1200) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1389) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

## Git

```
$ git log --oneline -3
db7f9d8 fix(dial): absolute rails 12-42 whole degrees; knob and drag snap to the grid
42eb3f4 docs: A1b report
40b784a ui(A1b): °C setpoint without a zero decimal; no unit label in relative mode
$ git diff --stat 42eb3f4 db7f9d8
 docs/REPORT-screen-layout-audit.md                 |  26 +++++
 docs/screens/rails-420-up-down.png                 | Bin 0 -> 26044 bytes
 docs/screens/rails-420-up.png                      | Bin 0 -> 26847 bytes
 docs/screens/rails-423-up.png                      | Bin 0 -> 26847 bytes
 docs/screens/rails-423.png                         | Bin 0 -> 27850 bytes
 docs/screens/rails-drag-337-live.png               | Bin 0 -> 26955 bytes
 docs/screens/rails-drag-337.png                    | Bin 0 -> 26574 bytes
 .../dial-idf/components/dial_state/dial_state.h    |  21 ++--
 firmware/dial-idf/components/dial_ui/scr_dial.c    |  39 +++++--
 firmware/dial-idf/main/main.c                      |  17 ++-
 simulator/main.c                                   | 125 +++++++++++++++++++++
 11 files changed, 206 insertions(+), 22 deletions(-)
```

Fix commit: **`db7f9d89f76c71c6ea04fa2a452ca8576717258b`**. Gate commit: `42eb3f4`. Working tree after this task: only this file (`docs/REPORT-rails-fix.md`) untracked; `bench-logs/` gitignored. PROJECT_VER still 0.1.5. No tag, no push.

## Deviations from the spec

1. **The fix commit was amended once.** The first commit (`9683939`) carried a typo in the session-link trailer; it was amended to `db7f9d8` with identical content before anything was flashed or reported. Nothing had been pushed.
2. **Release handler structure.** The spec said to snap "exactly the way the `s_rel` branch snaps"; that branch always re-rendered after snapping. The new code snaps in both scales, adds a clamp to `s_arc_min/max` (a no-op with on-grid rails, kept as a guarantee), and re-renders only if the snap changed the value. Same result on screen and on the wire.
3. **Simulator rails are seeded per scenario, not in `apply_baseline()`.** Seeding 120/420 in the baseline would have changed every existing dial render (the fallback 100/450 sets the arc geometry of `dial.png` etc.). Keeping it local means the 41 existing PNGs are untouched; the trade-off is that those older renders still show the fallback geometry, as they always have.
4. **`dial_state.h` gained comment text beyond the two named fields** — the range-macro block comment at the top of the file also said "12.0-42.3°C" and was updated. Comments only, as allowed.
5. **Commit message trailers.** Both commits carry the session's two attribution trailer lines after the spec's title; the titles are verbatim.

Everything else: none.

## Not verified without hardware

1. **Knob walk 12 → 42 → up → up → down reads 42, 42, 41.** Simulated (`rails-420-up.png`, `rails-420-up-down.png`) with the range-stop nudge and no post on the pinned detents; the real detent decoder, the haptic soft stop, and the pad's `target_t` echo in the log are hardware-only.
2. **Drag the handle to near the top and release — whole-degree numeral, whole `target_t` in the log.** Simulated end-to-end with the pointer indev (`33.7` live → `34`, `SET_TEMP 340`); the touch controller's real point stream and the wire value are hardware-only. Expect `POST /api/target_t {"side0":{"target_t":34}}`-style whole numbers only.
3. **Set 33.5 from the Somnus app, one detent on the dial → 34 or 33, never 34.5.** By the specified formula the snap is to 34 (round-half-up) and the step is from there: one detent **down reads 33**, one detent **up reads 35**, never 34.5 or 34.5±1. If the expectation is that the first detent up should read 34 (snap only, no step), that is a different rule from relative mode's `dial_rel_step`, which this fix deliberately mirrors — owner's call. Also unverified: that the pad reports 33.5 back as 33.5 (the dial rounds the reported float to tenths on the way in) so that the "42.3"-style render is what appears before the detent.
4. **Restore the pad's setpoint afterwards** — the owner's bench step; nothing in this task changed the pad's setpoint (the capture window had no activity).
