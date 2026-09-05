# Run 3 — commit 4 (spec revision 3, three review fixes)

## 1. Verdict

**DONE.** Commit 4 (all three revision-3 fixes, all in `scr_dial.c`) built
clean with `idf.py build`, zero warnings; version/tag/CHANGELOG/push left
untouched, `Claude outputs/` not added, as instructed.

## 2. Gate check (verbatim)

```
$ git log --oneline -1
5c42aad night face: water alternation while heating/cooling (spec rev 2 section 3a, commit 3)

$ grep -c 'Revision 3' docs/SPEC-night-face.md
2
```

HEAD was `5c42aad` and the grep count was 2 (≥1) — gate passed, proceeded.

## 3. Raw build output tail (unfiltered)

```
les/__idf_dial_ui.dir/scr_dial.c.obj
[ 97%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
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
somnus-dial.bin binary size 0x185a50 bytes. Smallest app partition is 0x400000 bytes. 0x27a5b0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete.
exit: 0
```

(NOTE-level Kconfig warnings — same three pre-existing ones as every prior
run, omitted here. `grep -i "warning\|error"` over the complete log, minus
those, returned nothing. Full log at `/tmp/build4.log` on the machine this
ran on.)

Binary size: **0x185a50 = 1,595,984 bytes ≈ 1.52 MB**, still 62% free — 16
bytes over commit 3's `0x185a40` (a few extra branches/comments, no new
statics, no new font).

## 4. `git diff --stat` and commit SHA

**Commit 4 — `e31761a906e33a0f1c773ebbf2e20d15c8e89805`**

```
firmware/dial-idf/components/dial_ui/scr_dial.c | 61 ++++++++++++++++---------
1 file changed, 39 insertions(+), 22 deletions(-)
```

On branch `firmware/somnus-port`, on top of `5c42aad` (commit 3). Only
`scr_dial.c` changed. Verified after the fact:

```
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")
$ git tag --list 'somnus-v*' | sort -V | tail -3
somnus-v0.1.4
somnus-v0.1.5-beta.1
somnus-v0.1.5-beta.2
$ git log -1 --oneline -- CHANGELOG.md
8a439e5 release: 0.1.5-beta.2
```

No `git push` was run. `git status --short` shows `Claude outputs/` still
untracked (`??`), never `git add`-ed.

## 5. Key values shown working

**1. Menu dot restore** — the `else` clause added to the page-dots block
in `apply_palette_and_state()`:
```c
} else {
    lv_obj_clear_flag(s_dot_menu, LV_OBJ_FLAG_HIDDEN);
}
```
`s_dot_a`/`s_dot_b` got no matching `else`: `dial_dots_layout()` (called
just above, every render) already `lv_obj_clear_flag`s whichever of those
two belongs before this block ever runs, so re-clearing them here would be
redundant — only `s_dot_menu` is exclusively `lv_obj_align`-ed (never
`HIDDEN`-touched) by that function, which is exactly the gap this `else`
closes.

**2. WATER word position** — old call removed:
```c
lv_obj_align_to(s_water_word, s_num_box, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
```
New call, in `create()` (not re-issued per render):
```c
lv_obj_align(s_water_word, LV_ALIGN_CENTER, 0, 92 - CY);
```

*Arithmetic (CY = 180, so this centers the label at absolute y = 92):*
`lv_font_montserrat_28` has `.line_height = 30` and `.base_line = 5`
(`lv_font_montserrat_28.c`) — a single-line label in this font is a 30px
box, split 15/15 above and below its `LV_ALIGN_CENTER` point. The numeral
box under `minimal` is 340×160 centered at the screen's own center (180),
so its top edge sits at y = 100.

At the spec-suggested `98 - CY` (center y = 98): label spans **83–113**.
Bottom (113) is 13px past the numeral box's top (100) — well past
`base_line`'s 5px, so this was rejected per the task's own instruction.

At `92 - CY` (center y = 92): label spans **77–107**. Bottom (107) is
still 7px past y = 100 by the label's full line-box — but `base_line = 5`
is the *descender* band at the bottom of that box (glyphs below the
baseline), and `"WATER"` is set entirely in capitals with no descending
glyphs, so its actual ink stops at the baseline: `line_height - base_line`
= 25px below the label's top, i.e. ink bottom = `(92-15) + 25` = **102** —
only 2px past y = 100, inside the 5px descender band that string never
uses. Used `92 - CY`, as the task anticipated, and this is that "say so."

Separately, and more to the point for whether anything actually looks
wrong: the numeral's own rendered ink doesn't start until ~y = 129 (run
1's report, §5 — `dial_font_num_140`'s 102px glyph height centered in the
160px box), so even the full, non-ink-adjusted 7px bounding-box overlap at
y = 92 lands entirely inside the box's own empty top margin, nowhere near
a digit.

**3. Drag snap** — added to `handle_event_cb`'s `LV_EVENT_PRESSED`, after
`s_press_dc = s_shown_dc;` and before `dial_state_stamp_input();`:
```c
if (s_alt_water) alt_show_setpoint();
```
`s_last_interact_ms` is untouched here, per the task — still set only in
`on_knob()` and the handle's `RELEASED`/`PRESS_LOST` path.

## 6. Deviations from the three items

None. All three match the task's literal instructions, including using
`92 - CY` (not `98 - CY`) for item 2 per its own fallback clause, and
placing the item-3 snap in `PRESSED` without touching `s_last_interact_ms`.

One pre-existing comment became stale as a side effect of item 3 and was
corrected, since leaving it would misdescribe the code it sits next to:
`s_last_interact_ms`'s own header comment previously said a drag/tap
started mid-water-phase "is corrected within one alternation tick... on
that same event -- only a knob turn (on_knob) gets the immediate snap,"
which stopped being true the moment `PRESSED` got its own immediate snap.
Reworded to say `PRESSED` now snaps the *display* immediately without
touching *this* tick. Not a code change beyond the three asked for, and
the task's "change nothing else" was read as scoped to behavior/other
code, not to a comment that the very code change under instruction 3 made
false.

## 7. Not verified without hardware

Nothing below was checked on the actual dial — dev-machine `idf.py build`
only, no `idf.py flash`.

- **The menu dot actually reappearing at dawn** — read as correct from
  `dial_dots_layout()`'s own code (confirmed it never touches `dot_menu`'s
  `HIDDEN` flag) and the new `else`, not watched through an actual
  night→day transition on a booted device.
- **The WATER word's new position on-glass** — the y-arithmetic in §5 is
  confirmed by the font's own struct fields and the box geometry, not by
  looking at the panel; specifically whether 92 reads as "centered above
  the numeral" and not oddly close to where the (now-hidden) side label
  used to sit, and whether the 2px ink-level encroachment on the numeral
  box's empty margin is invisible in practice as reasoned, not just on
  paper.
- **The drag-start snap** — confirmed by reading `handle_event_cb`'s new
  line, not by starting a drag mid-water-phase on a physical knob/touch
  panel and confirming no accent-colored flash of the drag value appears.
- Every hardware item runs 1 and 2 already listed unverified remains
  unverified — this commit touched none of those code paths, and nothing
  since run 1 has touched actual hardware.

---

# Run 2 — commit 3 (spec revision 2, §3a water alternation)

## 1. Verdict

**DONE.** Commit 3 (water alternation, all in `scr_dial.c`) built clean
with `idf.py build`, zero warnings; version/tag/CHANGELOG/push left
untouched as instructed.

## 2. Gate check (verbatim)

```
$ git log --oneline -2
27a82e5 night face: Settings row (spec commit 2)
a14462c night face: number-only face while night is active (spec docs/SPEC-night-face.md, commit 1)

$ grep -c '3a. Water temperature' docs/SPEC-night-face.md
1
```

HEAD was `27a82e5` ("night face: Settings row (spec commit 2)") and the
grep count was 1 — gate passed, proceeded.

## 3. Raw build output tail (unfiltered)

```
[ 71%] Built target __idf_dial_pad_discovery
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
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_dial.c.obj
[ 97%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
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
somnus-dial.bin binary size 0x185a40 bytes. Smallest app partition is 0x400000 bytes. 0x27a5c0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete.
exit: 0
```

(NOTE-level Kconfig warnings for `BT_NIMBLE_MESH_PROVISIONER` /
`FATFS_PRINT_LLI` / `FATFS_PRINT_FLOAT` — pre-existing, unrelated, omitted
here as in run 1. Full log at `/tmp/build3.log` on the machine this ran on.
`grep -i "warning\|error"` over the complete log, minus those three NOTE
lines, returned nothing.)

Binary size: **0x185a40 = 1,595,968 bytes ≈ 1.52 MB**, still 62% free — §3a
added ~32 bytes over commit 2's `0x185820` (one `lv_timer_t*`/`bool`/
`uint32_t` plus the new code; `s_water_word` is a plain `lv_label_create`,
not a font).

## 4. `git diff --stat` and commit SHA

**Commit 3 — `5c42aadf1a40272d02f50831393537464461a7e9`**

```
firmware/dial-idf/components/dial_ui/scr_dial.c | 237 ++++++++++++++++++++----
1 file changed, 205 insertions(+), 32 deletions(-)
```

On branch `firmware/somnus-port`, on top of `27a82e5` (commit 2). Only
`scr_dial.c` changed — §3a needed no font, no CMakeLists edit
(`lv_font_montserrat_28` is an LVGL built-in already linked into this file
for `s_power_glyph`) and no other file. Verified after the fact:

```
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")
$ git tag --list 'somnus-v*' | sort -V | tail -3
somnus-v0.1.4
somnus-v0.1.5-beta.1
somnus-v0.1.5-beta.2
$ git log -1 --oneline -- CHANGELOG.md
8a439e5 release: 0.1.5-beta.2
```

No `git push` was run.

## 5. Key values shown working

**The two period constants** (`scr_dial.c`):
```c
#define ALT_SETPOINT_MS 2000
#define ALT_WATER_MS    2000
```
Not collapsed into one shared constant even though both are 2000 today —
`alt_timer_cb` re-arms itself with whichever one applies to the phase it
just switched TO (`lv_timer_set_period(t, s_alt_water ? ALT_WATER_MS :
ALT_SETPOINT_MS)`), so the two can diverge later (spec §3a: "if the bench
says the setpoint should hold longer... change the two constants") without
any further code change.

**The knob-lock window:**
```c
#define ALT_KNOB_LOCK_MS 3000
```
Checked as `s_dragging || lv_tick_elaps(s_last_interact_ms) < ALT_KNOB_LOCK_MS`
in three places: `alt_timer_cb`, `apply_palette_and_state()`, and (the
`s_dragging` half only) implicitly everywhere `s_alt_water` is read.

**Where `s_last_interact_ms` is set** (`lv_tick_get()`, exactly the two
points spec §3a names, no others):
- `on_knob()`, first line of the function body — every detent, before the
  zone-off/range-stop branches, so it counts even when neither posts a new
  setpoint.
- `handle_event_cb()`, at the top of the shared `LV_EVENT_RELEASED` /
  `LV_EVENT_PRESS_LOST` block (both events, since the existing code already
  treated them identically as "end of the drag").

Deliberately **not** set in `PRESSED`/`PRESSING` — see §6/§7.

**Timer create/delete condition**, the actual boolean from
`apply_palette_and_state()`:
```c
bool alt_want = minimal && z->on && (kind == ZK_HEATING || kind == ZK_COOLING) && s_actual_dc >= 0;
```
`lv_timer_create(alt_timer_cb, ALT_SETPOINT_MS, NULL)` on the
false→true edge, `lv_timer_del(s_alt_timer)` on the true→false edge (plus
unconditionally in `destroy()`) — both idempotent, checked every
`apply_palette_and_state()` call.

**Shared formatter:** `render_value(int temp_dc, char *out, size_t out_sz)`
— the exact three branches `render_numeral` always had, now the only place
they exist:
1. `s_rel`: `dial_rel_from_dc(temp_dc)`, formatted `"0"` at level 0 else
   `"%+d"` (`"+3"` / `"-3"`).
2. `s_units_c` (absolute, °C): `"%d.%d"` (tenths split, exact).
3. else (absolute, °F): `"%d"` via `dial_dc_to_f(temp_dc)`.

`render_numeral(int temp_dc)` is now a two-line wrapper
(`render_value` + `lv_label_set_text`) kept for every pre-existing call
site; `alt_show_water()` is `render_value`'s only other caller, formatting
`s_actual_dc` instead of the setpoint. Signature differs from spec's own
suggested `render_value(int dc, bool as_water)` — see §6.1.

**`s_water_word`:** `lv_font_montserrat_28` (an LVGL built-in, already
linked in this file), created once in `create()` (hidden, text `"WATER"`,
not clickable/scrollable). Position: `lv_obj_align_to(s_water_word,
s_num_box, LV_ALIGN_OUT_BOTTOM_MID, 0, 6)`, re-issued every render inside
`apply_palette_and_state()`'s `minimal` branch (unlike `lv_obj_align`/
`lv_obj_center`, `lv_obj_align_to` against an arbitrary sibling computes a
one-shot position rather than a layout-system-tracked one, so it has to be
re-run whenever `s_num_box` itself moves — see the inline comment at the
call site). 6px gap below the box's bottom edge. Color/text set together
with visibility in `alt_show_water()`/`alt_show_setpoint()`.

## 6. Deviations from section 3a

1. **`render_value`'s signature.** Spec §3a says "Factor a
   `render_value(int dc, bool as_water)` or equivalent... do not duplicate
   the snprintf branches." Implemented as `render_value(int temp_dc, char
   *out, size_t out_sz)` — no `as_water` parameter — because the three
   branches are byte-identical regardless of which value is passed in
   (spec's own words: "Water uses the same unit rules as the setpoint");
   `as_water` would have been accepted and never read. Took the "or
   equivalent" the spec offered rather than carry a dead parameter.
2. **Accent caching.** Spec §3a: "the per-render pill accent already
   computed in apply_palette_and_state — store it in a static so the timer
   callback can use it." Reused the **existing** `s_level_accent` static
   (declared for the water-level arc overlay, already set to the same
   `accent` value on every render, already documented as "cached: the drag
   path has no state snapshot" — literally the same problem §3a describes)
   rather than adding a second static holding an identical value. Read
   "store it in a static" as being satisfied by an existing one that
   already does exactly that, not as a mandate for a new, separately-named
   one.
3. **`s_water_word`'s exact gap (6px)** below `s_num_box`. Spec says
   "centered directly under s_num_box" with no number. Picked 6px and used
   `LV_ALIGN_OUT_BOTTOM_MID` so it's flush against the box's own bottom
   edge by construction — but the box's minimal-mode bottom edge (absolute
   y=260, screen center 180) sits only ~16px above the power button's own
   fixed top edge (y=244, untouched per spec §3 — moving it was never on
   the table). A 6px-gapped, `lv_font_montserrat_28`-tall (30px line
   height) label in that slot comes within a few pixels of the button's
   topmost curve. This is a consequence of the box size §3 already fixed
   and the button position §3 already fixed, not a choice this commit
   made — flagged for a hardware look (§7) rather than silently picking a
   position further from the literal "directly under."

No other deviations. Nothing beyond §3a was touched in `scr_dial.c` — no
change to §3/§4/§5's font, box-swap-when-not-alternating, or hidden-widget
list; no CHANGELOG, no version bump, no tag, no push.

## 7. Not verified without hardware

Nothing below was checked on the actual dial — this ran on the dev machine
only (`idf.py build`, no `idf.py flash`). Per spec §8 and this task's own
list:

- **The cadence as perceived** — whether 2s/2s actually reads as a clean
  alternation on-glass, or as a flicker/jitter at the numeral's size and
  this panel's refresh behavior.
- **`WATER`-word legibility at night** — `lv_font_montserrat_28` at
  `accent` color, 6px under the numeral, read from across a dark room —
  and specifically the vertical crowding against the power button's top
  edge noted in §6.3.
- **Accent color against the ember (night) palette** — whether the
  heating/cooling hue reads clearly in `PAL()`'s night variant, not just
  its day one.
- **The knob snap during a water phase** — confirmed by reading
  `on_knob()`'s code path, not by turning a physical knob mid-water-phase
  and watching it snap.
- **The 3s hold after release** — same: `ALT_KNOB_LOCK_MS`/
  `lv_tick_elaps` read correctly, never watched on a clock.
- **That leaving night never leaves a water value on screen** — the
  `(!alt_want || alt_locked) && s_alt_water` branch in
  `apply_palette_and_state()` was read carefully (§6/§5), not exercised by
  actually forcing night off (Tokyo-timezone trick or Settings→Full)
  mid-water-phase on a booted device.
- **A drag or tap started exactly mid-water-phase.** Per spec §3a's own
  wording, only a knob turn gets an immediate same-event snap (§5); the
  handle's `PRESSED`/`PRESSING` path records nothing and forces nothing,
  so if a drag begins while the water phase is genuinely on screen, the
  numeral could show the live drag value in the water's `accent` color for
  up to one `alt_timer_cb` tick (≤2s) before `apply_palette_and_state`'s
  next run or the timer itself corrects the color. Not a coverage gap
  against spec's literal instructions (which name only `on_knob` and the
  handle's RELEASED path as recording points) but worth eyes-on
  confirmation that the brief mismatch, if any, isn't jarring.
- **Reaching HOLDING (setpoint met) while a water phase is showing** —
  read as correctly handled by the same `alt_want` recompute (kind
  changes, `alt_want` goes false, `alt_show_setpoint()` fires), never
  watched live.
- Every hardware item run 1's report already listed unverified (140px
  face legibility, width-vs-panel-chord, off-side breathe, range-stop
  nudge, side off/on, standby timeout/wake, the Tokyo-timezone trick,
  Night face → Full, Night mode → Off) remains unverified — §3a didn't
  change any of those code paths, but nothing since run 1 has touched
  actual hardware either.

---

# Run 1 — commits 1-2 (spec revision 1, §3/§4)

# Report: night face — number only (0.1.5-beta.3 feature)

## 1. Verdict

**DONE.** Both commits (font + night rendering, then the Settings row) built
clean with `idf.py build` and are in the tree; version/tag/CHANGELOG/push
were left untouched as instructed.

## 2. Gate check (verbatim)

```
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")

$ git tag --list 'somnus-v*' | sort -V | tail -3
somnus-v0.1.4
somnus-v0.1.5-beta.1
somnus-v0.1.5-beta.2
```

Tree at 0.1.5-beta.2, newest tag `somnus-v0.1.5-beta.2` — gate passed, proceeded.

## 3. Raw build output tail

### Commit 1 (font + night rendering)

```
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_palette.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_list.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_num_88.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_16.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_num_140.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_20.c.obj
[ 98%] Linking C static library libdial_ui.a
[ 98%] Built target __idf_dial_ui
[ 98%] Built target __idf_main
[ 98%] Generating esp-idf/esp_system/ld/sections.ld
[ 98%] Built target __ldgen_output_sections.ld
[ 98%] Linking CXX executable somnus-dial.elf
[ 98%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x1852e0 bytes. Smallest app partition is 0x400000 bytes. 0x27ad20 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete.
exit: 0
```

(NOTE-level Kconfig warnings for `BT_NIMBLE_MESH_PROVISIONER` /
`FATFS_PRINT_LLI` / `FATFS_PRINT_FLOAT` — pre-existing, unrelated to this
change, omitted here for length; full log at `/tmp/build1.log` on the
machine this ran on.)

Binary size: **0x1852e0 = 1,594,592 bytes ≈ 1.52 MB**, 62% of the 4MB app
partition free. Spec §5 estimated "~1.55 MB app" — matches.

### Commit 2 (the setting)

```
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_settings.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_address.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_adjust_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness_menu.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_face.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_timezone.c.obj
[100%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
[100%] Linking C static library libmain.a
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
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
somnus-dial.bin binary size 0x185820 bytes. Smallest app partition is 0x400000 bytes. 0x27a7e0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete.
exit: 0
```

Binary size: **0x185820 = 1,595,936 bytes ≈ 1.52 MB**, still 62% free — the
setting itself (a bool pref + one small picker screen) adds ~1.3 KB.

## 4. `git diff --stat` and commit SHAs

### Commit 1 — `a14462ca3dbd0b64cb5de073146e05e8996c58a1`

```
 .../dial-idf/components/dial_ui/CMakeLists.txt     |    4 +-
 .../components/dial_ui/dial_font_num_140.c         | 5312 ++++++++++++++++++++
 firmware/dial-idf/components/dial_ui/scr_dial.c    |   67 +-
 3 files changed, 5375 insertions(+), 8 deletions(-)
```

### Commit 2 — `27a82e5b7ef8b0a914e7aedfe08c297408f5196e`

```
 .../dial-idf/components/dial_state/dial_state.c    |  39 +++-
 .../dial-idf/components/dial_state/dial_state.h    |  25 +++
 .../dial-idf/components/dial_ui/CMakeLists.txt     |   2 +-
 firmware/dial-idf/components/dial_ui/scr_dial.c    |   5 +-
 .../dial-idf/components/dial_ui/scr_night_face.c   | 211 +++++++++++++++++++++
 .../dial-idf/components/dial_ui/scr_settings.c     |  57 ++++++
 firmware/dial-idf/components/dial_ui/ui_router.h   |   1 +
 firmware/dial-idf/components/dial_ui/ui_screens.c  |   1 +
 .../components/dial_ui/ui_screens_internal.h       |   1 +
 9 files changed, 337 insertions(+), 5 deletions(-)
```

Both on branch `firmware/somnus-port`, on top of `8a439e5` (0.1.5-beta.2).
Neither touched `PROJECT_VER`, `CHANGELOG.md`, tags, or a remote — verified
after the fact:

```
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.2")
$ git tag --list 'somnus-v*' | sort -V | tail -3
somnus-v0.1.4
somnus-v0.1.5-beta.1
somnus-v0.1.5-beta.2
$ git log -1 --oneline -- CHANGELOG.md
8a439e5 release: 0.1.5-beta.2
```

No `git push` was run.

## 5. Key values shown working

**Font.** `dial_font_num_140.c` generated in one `lv_font_conv` run (no
splice — the file's own header records the exact command, matching spec §5
verbatim): **251,081 bytes** (spec estimated "~250 KB" — matches). Font
source: `Montserrat-SemiBold.ttf` fetched from
`github.com/JulietaUla/Montserrat` (the same upstream `THIRD_PARTY_LICENSES.md`
already cites for `dial_font_num_88.c`'s own origin) — not present in-repo,
see §6 deviation 1.

Glyph `0` (U+0030), from the generated `glyph_dsc[]` table
(`dial_font_num_140.c:5172`):
```
{.bitmap_index = 2668, .adv_w = 1508, .box_w = 83, .box_h = 101, .ofs_x = 6, .ofs_y = -1}
```
`adv_w` is LVGL's 1/16px fixed-point advance; the pixel advance LVGL actually
renders is `(adv_w + 8) >> 4` (`lv_font_fmt_txt.c:177`) = **(1508+8)>>4 = 94 px**.

Spec §5's width check, computed the same way for all 13 glyphs and checked
against the box:

| string | spec estimate | actual (140px) | fits 340? |
|---|---|---|---|
| `"20.5"` | ≈290px | **292px** | yes |
| `"108"`  | ≈250px | **238px** | yes |
| `"-15"`  | ≈230px | **189px** | yes |

All under 340px with margin — no need to drop to 128px (spec §5's fallback).

**Numeral box.** `s_num_box`, toggled in `apply_palette_and_state()`:
- `minimal == false` (day, or night+Full): **210×92**, `LV_ALIGN_CENTER, 0, 150-CY` (i.e. `(0,-30)`) — unchanged from before this spec.
- `minimal == true` (night + Number only): **340×160**, `LV_ALIGN_CENTER, 0, 0` — per spec §3.

`s_temp_lbl` itself is `lv_obj_center()`-ed inside `s_num_box` once at
`create()`; LVGL's CENTER align re-derives on every layout pass, so
resizing the box is sufficient to recenter the numeral — no separate
reposition code for the label was needed.

**Widgets hidden under `minimal`** (all `LV_OBJ_FLAG_HIDDEN`, toggled per
render in `apply_palette_and_state()`, exactly as spec §3 lists):
- `s_unit_lbl` (unit / `LEVEL`)
- `s_water_lbl` (`WATER` caption)
- `s_pill` (status pill) — plus its chevron pulse stopped (see §6 deviation 4)
- `s_name_lbl` (side label)
- `s_underline_solid`, `s_underline_dash` (identity underline, both variants)
- `s_away_lbl` (AWAY badge)
- `s_dot_a`, `s_dot_b`, `s_dot_menu` (page dots)

Left alone, per spec §3: `s_power_btn`/`s_power_glyph` (power button),
`s_handle` (drag handle), `s_arc`/`s_level` (arc ring + water-level
overlay), `s_stale_dot` (staleness dot — still driven by its own
night-opacity logic), `s_zero_notch` (relative-mode landmark — spec never
lists it, see §6 deviation 5), and the OTA line (`s_ota_lbl`, "already
hidden at night" per spec §3, no change made).

## 6. Deviations from spec

1. **Font source file.** Spec §5 assumes `Montserrat-SemiBold.ttf` is
   available to run `lv_font_conv` against; it isn't checked into this repo
   (neither is the one used for the existing 88px font). Fetched it from
   `github.com/JulietaUla/Montserrat` — the exact upstream
   `THIRD_PARTY_LICENSES.md` already names as `dial_font_num_88.c`'s own
   origin (SIL OFL 1.1) — rather than guessing at a system/Google-Fonts
   copy, so the two numeral fonts share a byte-for-byte-traceable source.
   Not a font-shape deviation; the `lv_font_conv` command itself ran exactly
   as spec §5 wrote it, single run, no splice.
2. **Settings row "hidden while `night_on` is false" mechanism.** Spec §4
   says "**Hidden**... same mechanism as the Brightness night rows," but
   the actual Brightness night rows (`scr_brightness_menu.c`'s
   `sync_night_rows`) are **added/removed**, not `LV_OBJ_FLAG_HIDDEN` — that
   file's own header comment explains why: `dial_list`'s rotor math derives
   the focused row from raw child count, so a `HIDDEN` child still counts
   toward it while contributing no scroll height, desyncing the knob from
   every row beneath it. Implemented the Night face row the same way
   (`sync_night_face_row` in `scr_settings.c`, add/remove via
   `lv_obj_move_to_index(row, 3)` to land it directly under Night mode
   every time, since — unlike the Brightness pair — it isn't the last row
   in the list). Followed the concrete mechanism the spec pointed at over
   its own shorthand verb "Hidden," since the literal `HIDDEN` flag is the
   one thing that would have broken knob navigation.
3. **Pref field type.** Spec §4 calls the pref "u8." Declared
   `app_state_t.night_face_min` as `bool` (NVS-persisted via `nvs_set_u8`/
   `nvs_get_u8`, key `"night_face"`) — exactly `night_on`'s own shape, which
   the codebase already calls "u8" in its own NVS-key comment while being a
   `bool` in the struct. "Getter/setter copied from night_on's" (spec §4)
   was followed literally, including this detail.
4. **Chevron stop.** Spec §3 says "and `chevron_stop()` — a pulse on a
   hidden glyph is wasted work." Rather than adding a direct
   `if (minimal) chevron_stop();` call, folded `!minimal` into the existing
   `pulsing` predicate (`bool pulsing = (kind == ZK_HEATING || kind ==
   ZK_COOLING) && !minimal;`) — the pre-existing
   `else if (!pulsing && s_chevron_active) chevron_stop();` branch then
   calls it automatically the moment `minimal` goes true. Same effect
   (chevron stops, `chevron_stop()` runs), reached through the function's
   existing dispatch rather than a second call site.
5. **`s_zero_notch`** (the relative-mode neutral-level tick on the arc) is
   not in spec §3's shown/hidden lists either way and was left untouched —
   it continues to show/hide purely on `s_rel`, independent of `minimal`.
   Read §3's "The arc ring, as the palette already draws it at night" as
   covering it (it's an arc-band landmark, not a text/label widget), but
   flagging the omission explicitly since spec didn't name it either way.

No other deviations. Nothing beyond spec §3/§4/§5/§6(1-2) was added — no
CHANGELOG, no version bump, no tag, no push, no third commit.

## 7. Not verified without hardware

Nothing in this list was checked on the actual dial — this ran on the dev
machine only (`idf.py build`, no `idf.py flash`). Per spec §8, none of the
following were done:

- How the 140px face actually looks/reads on the round panel, in a dark
  room, without glasses (spec §1's whole stated purpose).
- The rendered width of `"20.5"` (and the other four spec §8 cases: °F
  absolute, °C absolute, relative +3, relative −12, relative 0) against the
  real 340×160 box and the panel's round chord — only the font's own
  advance-width arithmetic was checked (§5 above), not actual on-glass
  layout/clipping.
- The off-side breathe (`power_hint_pulse`/`NUM_STANDBY_OPA`) against the
  new 140px face — confirmed only by reading the code path, not by watching
  it.
- Turning the knob through a range stop under the night face (`anim_nudge`
  targets `s_num_box`, which now resizes under `minimal` — untested that
  the nudge animation still looks right at the larger box size).
- Switching a side off/on under the night face, and the timeout ->
  standby -> wake cycle.
- The Tokyo-timezone forced-night trick (spec §6 commit 1's own
  verification step) — not run; `night`/`minimal` were only exercised by
  reading `apply_palette_and_state()`, never by actually forcing
  `dial_palette_is_night()` true on a booted device.
- Setting Night face to Full and confirming the old face returns; Night
  mode to Off and confirming the Night face row disappears from Settings —
  the row add/remove logic (§6 deviation 2) was verified by build/read
  only, not by turning the knob through it on-device.
