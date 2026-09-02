# Spec: Somnus relative ("LEVEL") scale fix

Status: **APPROVED AND IN BUILD (2026-09-01).** Empirically established against
the real Somnus pad on 2026-09-01; implementing the same day.

## Problem

The dial's relative mode carries a −10..+10 lookup table (`DIAL_REL_DC` /
`DIAL_REL_LO_DC` in `components/dial_state/dial_state.h`) that was mechanically
transcribed from a whole-Fahrenheit scale belonging to Orion, the upstream
product this firmware was ported from. The Somnus app's own relative scale is
−15..+15 in whole degrees Celsius. The two disagree at every point — the
firmware has never displayed the level the Somnus app would show for the same
bed temperature.

`test/test_dial_rel.c` made this invisible: it cross-checks the dial's table
against an `ORION_REL_C[]` reference, i.e. it asserts agreement with the wrong
product. It has been passing the whole time, which is why the mismatch
survived unnoticed.

## The measured Somnus scale

Three independent measurements against the real pad, 2026-09-01:

| App level | target_t | Note |
|---|---|---|
| −15 | 12.0°C | app displays 54°F; 12.0°C = 53.6°F, rounds to 54 |
| −6 | 21.0°C | measured directly via POST then reading the app back |
| +15 | 42.0°C | app displays 108°F; 42.0°C = 107.6°F, rounds to 108 |

These three points are exactly consistent with a **uniform, Celsius-native**
scale:

- 1.0°C per level (matches this project's already-settled "1.0°C per
  app-level adjustment" finding — the Somnus API's real step size, not
  Orion's ~1.75°C average)
- level 0 = 27.0°C
- 30 one-degree intervals spanning 12.0°C..42.0°C

Check: level −15 → 27.0 − 15 = 12.0°C ✓. Level −6 → 27.0 − 6 = 21.0°C ✓.
Level +15 → 27.0 + 15 = 42.0°C ✓. The app's endpoint labels ("54°F"..."108°F")
look arbitrary only because they are rounded Fahrenheit renderings of whole
Celsius values — there is no independent Fahrenheit design underneath them.

Because the scale is uniform, it needs no lookup table at all: it's affine
arithmetic, `dc = 270 + 10*L`.

## Defects beyond the value mismatch

1. **Rails exceed the API's accepted range.** The current table's rails are
   `DIAL_REL_MIN_DC` 100 (10.0°C) and `DIAL_REL_MAX_DC` 450 (45.0°C). The
   Somnus API only accepts 12.0..42.3°C (`temp_range_t` seeded in `main.c`
   from the local_api spec). The pad silently clamps anything outside that,
   so the dial's extreme levels collapse onto the same real temperature —
   the knob keeps clicking, the number keeps changing, and the bed stops
   responding, with no error to explain why.
2. **Wrong default.** `dial_state.c:107` sets `s_state.rel_mode = true`, so a
   fresh device boots into relative mode without the user choosing it. This
   spec does not change that line — see Open questions.

## Target

Replace the two lookup tables (`DIAL_REL_DC[21]`, `DIAL_REL_LO_DC[20]`) with
arithmetic:

```c
#define DIAL_REL_MIN     (-15)
#define DIAL_REL_MAX     ( 15)
#define DIAL_REL_MIN_DC  120    // level -15 rail (12.0 C, the API minimum)
#define DIAL_REL_MAX_DC  420    // level +15 rail (42.0 C)
#define DIAL_REL_ZERO_DC 270    // level 0 (27.0 C)
```

- `dial_rel_to_dc(L)` = `DIAL_REL_ZERO_DC + 10 * L`, clamped to
  `[DIAL_REL_MIN, DIAL_REL_MAX]` before the multiply (equivalently, clamp the
  result to `[DIAL_REL_MIN_DC, DIAL_REL_MAX_DC]` — the two are equivalent
  since the mapping is affine and monotonic).
- `dial_rel_from_dc(dc)` = nearest level, clamped to
  `[DIAL_REL_MIN, DIAL_REL_MAX]`. Round to nearest, not truncate — truncating
  toward zero would bias every off-grid value warmer-of-center on the cool
  side and cooler-of-center on the warm side. Ties (exactly halfway between
  two levels, i.e. `dc` ending in `...5` relative to the grid) resolve toward
  the WARMER level, preserving the convention the old table's boundary
  values documented.
- `dial_rel_step(dc, detents)` — semantics unchanged from today: read the
  level currently DISPLAYED for `dc` (via `dial_rel_from_dc`), add `detents`,
  clamp to range, return that level's carrier. Returns `dc` unchanged only
  when already pinned at a rail. This is what makes an off-grid device value
  snap onto the grid in the direction turned on the very first detent — the
  numeral and the bed always move together, never separately.

Every level's carrier must land inside the API's accepted range:
`120 <= dial_rel_to_dc(L) <= 423` for all `L` in `[-15, 15]` (423 is the API's
actual upper bound per `main.c`'s seeded `temp_range_t`; the dial's own rail
at 420 stays strictly inside it, which is intentional headroom, not a bug).

## Out of scope / not changed by this spec

- **`dial_state.c:107` (`rel_mode` default = `true`).** **Decided 2026-09-02:
  relative stays the default. No code change.** The scale is now a verified
  1:1 match for the Somnus app (1.0 C per level, level 0 = 27.0 C, rails at
  -15/+15), so a new user's dial and their app show the same number out of
  the box; a dial that disagreed with the app on first boot would read as
  broken before it read as a design choice.

  Scope of the decision: **fresh devices only.** `dial_state.c:199-200`
  already handles the other two paths and is not affected — an explicitly
  persisted `relmode` is honored, and a device set up before this release
  (a "zone" key with no "relmode") stays on ABSOLUTE so an unattended OTA
  never changes what the big number means for someone who has read F every
  night.

  The argument for absolute is real and was weighed: a temperature is
  meaningful with no app open, and `design-spec.md`'s "the one big number is
  a fact" thesis was written around a 55-110 F numeral. It loses to the
  app-agreement argument for a first-boot default, and the Settings row makes
  it one tap away for anyone who prefers it.
- The absolute-mode temperature path, the Pad Address row / input model, the
  connect-loop phase behavior, and the °F/°C units toggle — none of these are
  touched by this spec.
- `scr_dial.c`'s arc-range wiring (`configure_arc_range`, lines ~216-225)
  reads `DIAL_REL_MIN_DC`/`DIAL_REL_MAX_DC` already, so the new rails apply
  automatically with no code change there. Two of its comments (the sweep
  cited as "100–450dc in relative" near line 199, and "relative mode's
  21-level table" near line 215) describe the old rails/table and go stale
  the moment this spec ships; fixing them is a follow-up, not part of this
  change.
