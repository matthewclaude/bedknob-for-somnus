# Report — plugged-in / on-battery indicator (0.1.5-beta.4)

**Verdict: DONE.** Both commits built clean and landed exactly as specified
(detector/state/About row in commit 1, the two glyphs in commit 2); nothing
out of section 10.6's scope was added; PROJECT_VER/CHANGELOG/tag/push were
left untouched as instructed. What's unverified is exactly the hardware
bench work section 10.7 calls for — this was a build-and-static-review pass,
no target flashed.

## Gate check (verbatim)

```
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.3")

$ git tag --list 'somnus-v*' | sort -V | tail -1
somnus-v0.1.5-beta.3

$ git rev-parse HEAD somnus-v0.1.5-beta.3
1aafca53ae6c7b281abe4370b59063f6fc527abf
1aafca53ae6c7b281abe4370b59063f6fc527abf

$ git --no-optional-locks status --short | grep -v '^??'
(nothing — grep exit 1)
```

All four checks passed on the first run. No stop needed.

## Build — commit 1 (raw tail)

```
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
somnus-dial.bin binary size 0x1884a0 bytes. Smallest app partition is 0x400000 bytes. 0x277b60 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
```

No compiler warnings from any touched file. (The build log did carry one
pre-existing, unrelated `CMake Warning (deprecated)` at the top level
`CMakeLists.txt:5 cmake_minimum_required` — not from anything in this
change, present before and after.)

## Build — commit 2 (raw tail)

```
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
somnus-dial.bin binary size 0x1886b0 bytes. Smallest app partition is 0x400000 bytes. 0x277950 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
```

No compiler warnings from any touched file. Size grew by `0x210` bytes
(528 bytes) over commit 1 — the two glyphs, the shared helper, and the two
screens' wiring.

## git diff --stat and commit SHAs

**Commit 1** — `e7ff49b` "power: USB/battery detector, power_src in state,
About row (docs/SPEC-power-sensing.md section 10, commit 1)"

```
 .../dial-idf/components/dial_power/CMakeLists.txt  |   2 +-
 .../dial-idf/components/dial_power/dial_power.c    | 181 +++++++++++++++++++++
 .../dial-idf/components/dial_state/dial_state.c    |  11 ++
 .../dial-idf/components/dial_state/dial_state.h    |  28 ++++
 firmware/dial-idf/components/dial_ui/scr_about.c   |  25 +++
 5 files changed, 246 insertions(+), 1 deletion(-)
```

**Commit 2** — `8245021` "power: battery glyph and 3 s plug-in glyph on dial
face and standby (section 10, commit 2)"

```
 firmware/dial-idf/components/dial_ui/scr_dial.c    | 21 +++++
 firmware/dial-idf/components/dial_ui/scr_standby.c | 15 ++++
 .../components/dial_ui/ui_screens_internal.h       | 95 ++++++++++++++++++++++
 3 files changed, 131 insertions(+)
```

Both commits touch only the files their own bullet list named. Gate
re-verified clean (no unstaged tracked changes) after both commits.

## Key values shown working

**Threshold / debounce constants** (`dial_power.c`, top of file, named
per instruction):

```c
#define PWR_ADC_CHANNEL         ADC_CHANNEL_0
#define PWR_ADC_ATTEN           ADC_ATTEN_DB_12
#define PWR_ADC_BITWIDTH        ADC_BITWIDTH_12
#define PWR_SAMPLE_EVERY_TICKS  10      // power_task ticks at 100ms -> 1 sample/s
#define PWR_RING_N               10     // slope tiebreak window (§10.2)
#define PWR_BOOT_MIN_SAMPLES      5     // UNKNOWN until this many samples exist
#define PWR_DEBOUNCE_N             5    // consecutive agreeing samples to change state
#define PWR_ENTER_PLUGGED_MV    4280    // hysteresis: enter PLUGGED at >= 4.28V
#define PWR_LEAVE_PLUGGED_MV    4180    // hysteresis: leave PLUGGED at <= 4.18V
#define PWR_SLOPE_TIEBREAK_MV      20   // mean(newest5) - mean(oldest5), mV
```

**Exact boolean for each state transition** (`pwr_classify`/
`pwr_sample_and_classify`, `dial_power.c`):

- Enter PLUGGED (threshold): `mv >= PWR_ENTER_PLUGGED_MV`
- Enter/leave to BATTERY (threshold): `mv <= PWR_LEAVE_PLUGGED_MV`
- Slope tiebreak, charging: `slope > PWR_SLOPE_TIEBREAK_MV` (only once the
  10-sample ring is full — `pwr_ring_slope_mv` returns `false` before that,
  which `pwr_classify` treats as `PWR_UNKNOWN`, i.e. inconclusive; see
  "Deviations" below)
- Slope tiebreak, discharging: `slope < -PWR_SLOPE_TIEBREAK_MV`
- Boot gate (stay UNKNOWN, no vote taken): `s_pwr_ring_n < PWR_BOOT_MIN_SAMPLES`
- Debounce commit gate: `s_pwr_pending_count >= PWR_DEBOUNCE_N && vote != s_power_src`
  — both must hold, in `pwr_sample_and_classify`; the guard is written as its
  negation (`if (... < ... || vote == s_power_src) return;`) directly above
  the commit.
- Glyph "plug-in confirmation" trigger (`power_glyph_apply`,
  `ui_screens_internal.h`): `prev == PWR_BATTERY && !pg->charge_timer`

**Where the commit-on-change happens:** `dial_power.c`'s
`pwr_sample_and_classify()` calls `dial_state_set_power_mv(mv)`
(`dial_state.c`, no generation bump — direct `xSemaphoreTake` / field write /
`xSemaphoreGive`, same shape as `dial_state_set_ui_zone`) on **every**
1-second sample, unconditionally. `dial_state_commit(mut_power_src, &vote)`
(the ordinary generation-bumping path) is called **only** inside the
debounce-gate block above, i.e. only on an actual `power_src` transition.
`mut_power_src` itself only touches `st->power_src` — `power_mv` is already
current in the store from the direct write moments earlier, so it rides
along for free, exactly as §10.3 describes.

**About row's refresh mechanism:** `scr_about.c` already had an `on_state`
(for the Serial row), so per the task's own conditional ("if scr_about has
no on_state, add a timer") no timer was added — `render_power_row()` was
folded into the existing `on_state`. `on_state` fires on every generation
bump (any commit, not just a power one) and once immediately on screen
entry (`ui_router_go` calls it right after `create()` with a fresh
`dial_state_get()`), so the row shows the live number the instant the
screen opens and stays in step with §10.3's "refreshes whenever anything
else changes" — `dial_state_get()` always copies the live store regardless
of whether the read follows a generation bump, so the no-commit `power_mv`
write is visible the next time anything triggers a re-render.

**Glyph slot arithmetic** (`components/dial_ui/scr_dial.c`,
`lv_font_montserrat_16.c`'s `line_height = 18`): the glyph is
`lv_obj_align(..., LV_ALIGN_CENTER, 0, 46 - CY)`, i.e. vertically centered
at absolute y=46, spanning **y=37 to y=55** (46 ± 9, half of 18px — the
task's own "roughly 38-54" is the same span to LVGL's own rounding). The
staleness dot is a 10px circle centered at y=26, spanning **y=21-31** — a
6px gap (31→37) to the glyph's top, no overlap. The WATER word
(`lv_font_montserrat_28`, `line_height=30`) is centered at y=92, spanning
**y=77-107** — a 22px gap (55→77) to the glyph's bottom, no overlap. Both
gaps confirmed by direct arithmetic, not just visual inspection (no
hardware/simulator render was done — see "could not be verified" below).

**3 s timer ownership/deletion:** owned by `power_glyph_t.charge_timer`
(one instance per screen: `scr_dial.c`'s `s_batt_glyph`, `scr_standby.c`'s
`s_batt_glyph`). Created in `power_glyph_apply` with
`lv_timer_create(power_glyph_charge_timer_cb, 3000, pg)` and
`lv_timer_set_repeat_count(pg->charge_timer, 1)`. LVGL's own `lv_timer_exec`
decrements `repeat_count` to 0 *before* invoking the callback and
auto-deletes the underlying `lv_timer_t` right after the callback returns
(confirmed by reading `managed_components/lvgl__lvgl/src/misc/lv_timer.c`)
— so `power_glyph_charge_timer_cb` only hides the label and sets
`pg->charge_timer = NULL`; it must NOT (and does not) call `lv_timer_del`
itself, or it would double-free the timer LVGL is about to delete on its
own. Two other paths delete it explicitly, before it would have
auto-fired: `power_glyph_apply`'s `cur == PWR_BATTERY` branch (an unplug
mid-flash cancels the flash) and `power_glyph_destroy` (called from both
screens' `destroy()`, alongside `s_batt_glyph_last = PWR_UNKNOWN` so a
fresh `create()` starts clean).

**Font-range check (both symbols), result: PRESENT, commit 2 not blocked.**
`lv_font_montserrat_16.c`'s sparse cmap stores codepoints as
`unicode_list[i] = codepoint - range_start` with `range_start = 176`:
- `LV_SYMBOL_CHARGE` = `0xF0E7` (61671). `61671 - 176 = 61495 = 0xF037` —
  present in `unicode_list_1` (verified by reading the array).
- `LV_SYMBOL_BATTERY_EMPTY` = `0xF244` (62020). `62020 - 176 = 61844 =
  0xF194` — present in `unicode_list_1` (verified the same way).

Both deltas were located by direct hex arithmetic against the array
literal, not by rendering — see "could not be verified" below for what
that still leaves open.

## Deviations from section 10

**One, in an area section 10.2 leaves genuinely underspecified: the slope
tiebreak's behavior before the 10-sample ring is full.** §10.2 says "keep
the last 10 samples; if the mean of the newest 5 exceeds the mean of the
oldest 5 by...", which presumes a full 10-sample window. It doesn't say
what an in-band (4.18–4.28V) reading should do during the ~5-10s after
boot (or after any gap) where the ring has fewer than 10 samples but at
least the `PWR_BOOT_MIN_SAMPLES` (5) needed to leave UNKNOWN at all. This
implementation treats that case as **inconclusive** (`pwr_ring_slope_mv`
returns `false`, `pwr_classify` returns `PWR_UNKNOWN`, which breaks the
debounce run without changing `power_src`) rather than computing a slope
over a partial, overlapping window. In practice this only matters if the
pin happens to sit inside the 100mV band during that early window — most
readings (charged-and-plugged ~4.6-4.7V, or a battery well below 4.18V)
classify immediately by threshold regardless. Flagged here for the owner
to confirm against the bench measurement §10.2 itself calls "open" (time
from plug-in to 4.28V on a depleted cell) — if that measurement shows the
climb regularly sits in-band for more than ~10s early after a cold boot,
this corner case is worth a second look.

Everything else: none. No percentage, no low-battery warning, no setting,
nothing added beyond section 10.2/10.3/10.4's two commits — section 10.6's
exclusions were not touched (no percentage/curve, no low-battery warning,
no face takeover, no deep-sleep work, no brownout-warning reuse of the
sample). PROJECT_VER, CHANGELOG.md untouched; no tag; no push.

## What could not be verified without hardware

- **The plug-in curve and time-to-4280mV on a depleted cell** (§10.2's own
  "open measurement, before building") — needs a real board and a
  depleted cell on the bench with `screen` open reading the `ESP_LOGD`
  sample line.
- **Marginal-cable behavior** — whether the hysteresis + debounce + slope
  combination actually prevents flapping on a real bad cable, as opposed
  to just on paper.
- **Both glyphs actually rendering** — the font-range check confirms the
  codepoints are compiled into `lv_font_montserrat_16`'s glyph table, not
  that they draw correctly on the physical 368×448 round LCD (glyph
  metrics, bitmap data, or LVGL's symbol-font fallback path were not
  visually inspected — no simulator, no target flash, in this pass).
- **Night opacity** (`LV_OPA_40` on the dial face) actually reading as dim
  in a dark room, the way the design intends.
- **Standby ink** (`neutral_holding` at night) actually matching the
  clock number's own dimness by eye, not just by using the same palette
  field.
- **That nothing ever draws while PLUGGED at steady state** — verified by
  code inspection (`power_glyph_apply`'s `cur == PWR_PLUGGED` branch only
  acts on a fresh `BATTERY → PLUGGED` transition; otherwise it returns
  with the label already hidden or already correctly showing CHARGE), not
  observed on a running dial left plugged in for a while.
- The ADC read itself (`pwr_read_mv`/`pwr_adc_init`) — `adc_oneshot_new_unit`,
  channel config, and `adc_cali_create_scheme_curve_fitting` compiled and
  linked correctly, but were never executed against real GPIO1 silicon in
  this pass (no target flash, no `idf.py monitor`).
