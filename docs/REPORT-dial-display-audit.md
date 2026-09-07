# Dial °F Setpoint Display Audit

## VERDICT

**H1 (benign) is what the source implements.** The absolute-face °F numeral
is a display-only `round(°C × 9/5 + 32)` of a canonical whole-°C setpoint
(1.0 °C per detent, 31 levels, 12.0–42.0 °C); the write path posts that same
whole-°C value straight to `POST /api/target_t`. There is no leftover
55–110 °F range, no surviving `f_to_c` helper, and no C→F→C round-trip
anywhere in the setpoint's life. The computed click table (Part B) shows
zero dead clicks, zero dial/app °F disagreements, and an exact match against
the Somnus app's own −15…+15 °F ladder given in the task. H2 is not
supported by the code as it stands today.

## GATE CHECK

- **Rule 1** (no firmware feature/behavior change) applied. No file under
  `components/`, `main/`, or `test/` was modified — confirmed by
  `git --no-optional-locks diff --stat` below (empty). This audit found no
  bug requiring a code change, so there is nothing to recommend fixing;
  see DEVIATIONS FROM SPEC.
- **Rule 2** (read-only against the pad) applied. Only `GET /api/state` was
  ever issued, once, via `tools/dial_display_audit.py --once`, to verify the
  logger starts and prints a good line. No `POST /api/power` or
  `POST /api/target_t` was sent at any point in this session. Nothing was
  blocked by this rule — a read-only verification was sufficient.
- **Rule 3** (no git write commands) applied. Only `git --no-optional-locks
  status`/`diff --stat`/`rev-parse`/`log`/`grep` were run; nothing was
  `git add`ed, committed, or pushed.
- **Rule 4** (never force-overwrite) applied. `docs/REPORT-dial-display-audit.md`
  did not previously exist (verified with `ls` before writing) —
  this write is a new file, not an overwrite. `tools/dial_display_audit.py`
  is also new.
- **Rule 5** (don't go looking for out-of-repo docs) applied. Nothing outside
  this repo was consulted; the Somnus app's °F ladder was taken verbatim
  from the task prompt, not looked up.

Nothing was blocked. This report is the full, complete audit.

## PART A — Code trace

**Where a detent changes the setpoint, and its internal unit.**

`firmware/dial-idf/components/dial_ui/scr_dial.c:1443-1447` (`on_knob`):

```c
int base = ((s_shown_dc + 5) / 10) * 10;   // nearest whole degree
nf = base + detents * 10;
if (nf < s_arc_min) nf = s_arc_min;
if (nf > s_arc_max) nf = s_arc_max;
```

One detent = `detents * 10` on `temp_dc`, the canonical unit defined at
`firmware/dial-idf/components/dial_state/dial_state.h:52-56` as **tenths of
a degree Celsius, integer** ("dc"). So one detent = 10 dc = exactly **1.0 °C**
per click, not a whole-°F step and not a level index directly (though in
absolute mode it happens to always land on a whole °C). The comment block
right above it, `scr_dial.c:1424-1431`, states the design intent explicitly:
*"Absolute: one detent = 10 tenths = exactly 1.0°C (the Q1 units fix's
design decision — matches the Somnus app's own whole-degree scale...)"*.

Relative mode (the other on-screen mode, not the absolute face this audit
is about) steps by whole **levels**, via `dial_rel_step()` —
`dial_state.h:125-136`:

```c
#define DIAL_REL_MIN     (-15)
#define DIAL_REL_MAX     ( 15)
#define DIAL_REL_MIN_DC  120   // level −15 rail (12.0°C, the API minimum)
#define DIAL_REL_MAX_DC  420   // level +15 rail (42.0°C)
#define DIAL_REL_ZERO_DC 270   // level 0 (27.0°C)
```
and `on_knob`'s relative branch, `scr_dial.c:1443`: `nf = dial_rel_step(s_shown_dc, detents);`
— a level is also 10 dc = 1.0 °C, same grid, same unit, different origin
label. Both modes share the exact same underlying `temp_dc` grid.

**Where the absolute-face number is computed/formatted, and the C→F conversion.**

`firmware/dial-idf/components/dial_state/dial_state.h:1024-1026`:

```c
static inline int dial_dc_to_f(int dc) { return (int)lroundf((float)dc * 0.18f + 32.0f); }
```
(`dc * 0.18` ≡ `dc/10 * 9/5`, i.e. `°C * 9/5 + 32`, round-half-up via `lroundf`.)

Called from `firmware/dial-idf/components/dial_ui/scr_dial.c:444-445`
(`render_value`, the absolute-face branch when the units toggle is set to °F):

```c
} else {
    snprintf(out, out_sz, "%d", dial_dc_to_f(temp_dc));
}
```

`render_value` is called from `render_numeral()` (`scr_dial.c:466-470`),
which is called with `s_shown_dc` — the same `temp_dc` the knob handler
just wrote — never a separately-tracked °F variable. The comment directly
above (`scr_dial.c:416-420`) names the exact expected irregularity:

> *"°F is display-only math with no bearing on what gets stored or posted:
> after the Q1 units fix, absolute mode steps in exact whole 1.0°C
> increments, so the °F numeral now steps IRREGULARLY (e.g. 68, 70, 72, 73,
> 75 — each is the nearest whole °F to a clean whole-°C value). That
> irregularity is the correct, expected result..."*

**Where the POSTed value is derived from the same internal state.**

Worker task, `firmware/dial-idf/main/main.c:1160-1169`:

```c
if (last_temp_dc[z] >= 0) {
    // int tenths -> double directly, no float intermediate: ...
    double c = last_temp_dc[z] / 10.0;
    zone_temp_t up = { z, last_temp_dc[z], issued_us };
    if (dial_somnus_set_temp((somnus_side_t)z, c))
        dial_state_commit(mut_zone_temp, &up);
}
```

`last_temp_dc[z]` is populated straight from `cmd.temp_dc`
(`main.c:1143`), which is the exact `temp_dc` `post_temp()` posted from
`scr_dial.c:827-835` — the same integer the knob handler computed and the
same integer the numeral was rendered from. There is no separate °F-derived
value anywhere on this path; `°C = dc / 10.0`, full stop.

`dial_somnus_set_temp()`'s own doc comment,
`firmware/dial-idf/components/dial_somnus/dial_somnus.h:114-131`, confirms
the pad is sent this value with no re-clamping on the dial's side (the pad
clamps itself to its accepted range).

**Min/max rails on the absolute face, and units.**

`main.c:1085-1095` (worker seeds `app_state_t.temp_min_dc/max_dc` once,
right after first connect):

```c
// Hardcoded directly in the canonical unit (tenths of °C): 12.0-42.0°C == 120-420dc,
// ... the pad accepts up to 42.3 ... this dial never sets it — seeding the
// API's 423 here put one detent above 42.0
temp_range_t range = { 120, 420 };
```

So the absolute face's operative rails are **120–420 dc = 12.0–42.0 °C**
(31 whole-degree steps inclusive), identical to the relative face's
`DIAL_REL_MIN_DC`/`DIAL_REL_MAX_DC`. The pad's own API accepts up to
42.3 °C (`dial_somnus.h:68`, `main.c:1091`), but the dial itself never
offers a detent past 42.0 °C, so 42.1–42.3 °C is reachable only by some
other client (e.g. the app), never by a dial click — see NOT VERIFIABLE
below for what happens if the pad is already sitting there.

**Fahrenheit range 55..110 / leftover f_to_c / c_to_f helpers.**

```
$ grep -rn "f_to_c\|dial_f_to_c" firmware/dial-idf/components firmware/dial-idf/main
firmware/dial-idf/components/dial_state/dial_state.h:79:// dial_f_to_c(int f) alongside dial_c_to_f: it was the write-path half of
```

That is the *only* hit, and it is a comment, not code — `dial_state.h:76-81`:

> *"There used to be a `dial_f_to_c(int f)` alongside `dial_c_to_f`: it was
> the write-path half of that round-trip and is deleted, not renamed —
> nothing should ever again convert a whole-°F value back into the
> canonical unit."*

No `55`/`110` Fahrenheit-range literal exists anywhere in
`components/`/`main/` (checked by grep across both trees). `dial_c_to_f()`
still exists (`dial_state.h:1017-1018`) but is explicitly scoped, by its own
comment (`dial_state.h:1006-1015`), to the **measured water reading**
display only (`actual_c`) — never the setpoint, never round-tripped into a
write.

**H1 vs H2 — which does the source support?**

**H1.** Every element of H1 matches the source exactly: `DIAL_REL_ZERO_DC =
270` (27.0 °C = level 0), `DIAL_REL_MIN_DC`/`MAX_DC` = 120/420 (12.0–42.0 °C,
31 levels), and `dial_dc_to_f()` is precisely `round(dc/10 * 9/5 + 32)`. H2
is not supported: there is no whole-°F internal unit anywhere, no surviving
`f_to_c`, and no 55–110 °F range literal in the tree. The commit history
this code itself narrates (the "Q1 units fix", `dial_state.h:44-81`)
describes H2 as the *pre-fix* design that was deliberately removed on
2026-08-30 for causing fractional, non-1.0°C POST bodies — i.e. H2 was a
real, already-fixed historical bug, not the current state.

## PART B — Computed click table

Formulas used, from Part A:
- `dc = 270 + 10 × level` (`DIAL_REL_ZERO_DC + 10*level`, `dial_state.h:136`)
- `C_dial = dc / 10.0` (exact; this is the canonical unit itself)
- `C_clamped = clamp(C_dial, 12.0, 42.3)` (pad's own accepted range,
  `dial_somnus.h:68`; the dial's own rails already sit inside this, so no
  row clamps)
- `F_dial = round(dc × 0.18 + 32.0)` (`dial_dc_to_f`, `dial_state.h:1024`)
- `somnus_level = C_clamped − 27.0`
- `F_app = round(C_clamped × 9/5 + 32)` (per task's own formula for "the °F
  the Somnus app should display")

| detent | dc (0.1°C) | C_dial | C_clamped | somnus_level | F_dial | F_app(calc) | F_app(given ladder) | match |
|---:|---:|---:|---:|---:|---:|---:|---:|:---:|
| 0  | 120 | 12.0 | 12.0 | −15 | 54  | 54  | 54  | OK |
| 1  | 130 | 13.0 | 13.0 | −14 | 55  | 55  | 55  | OK |
| 2  | 140 | 14.0 | 14.0 | −13 | 57  | 57  | 57  | OK |
| 3  | 150 | 15.0 | 15.0 | −12 | 59  | 59  | 59  | OK |
| 4  | 160 | 16.0 | 16.0 | −11 | 61  | 61  | 61  | OK |
| 5  | 170 | 17.0 | 17.0 | −10 | 63  | 63  | 63  | OK |
| 6  | 180 | 18.0 | 18.0 | −9  | 64  | 64  | 64  | OK |
| 7  | 190 | 19.0 | 19.0 | −8  | 66  | 66  | 66  | OK |
| 8  | 200 | 20.0 | 20.0 | −7  | 68  | 68  | 68  | OK |
| 9  | 210 | 21.0 | 21.0 | −6  | 70  | 70  | 70  | OK |
| 10 | 220 | 22.0 | 22.0 | −5  | 72  | 72  | 72  | OK |
| 11 | 230 | 23.0 | 23.0 | −4  | 73  | 73  | 73  | OK |
| 12 | 240 | 24.0 | 24.0 | −3  | 75  | 75  | 75  | OK |
| 13 | 250 | 25.0 | 25.0 | −2  | 77  | 77  | 77  | OK |
| 14 | 260 | 26.0 | 26.0 | −1  | 79  | 79  | 79  | OK |
| 15 | 270 | 27.0 | 27.0 | 0   | 81  | 81  | 81  | OK |
| 16 | 280 | 28.0 | 28.0 | 1   | 82  | 82  | 82  | OK |
| 17 | 290 | 29.0 | 29.0 | 2   | 84  | 84  | 84  | OK |
| 18 | 300 | 30.0 | 30.0 | 3   | 86  | 86  | 86  | OK |
| 19 | 310 | 31.0 | 31.0 | 4   | 88  | 88  | 88  | OK |
| 20 | 320 | 32.0 | 32.0 | 5   | 90  | 90  | 90  | OK |
| 21 | 330 | 33.0 | 33.0 | 6   | 91  | 91  | 91  | OK |
| 22 | 340 | 34.0 | 34.0 | 7   | 93  | 93  | 93  | OK |
| 23 | 350 | 35.0 | 35.0 | 8   | 95  | 95  | 95  | OK |
| 24 | 360 | 36.0 | 36.0 | 9   | 97  | 97  | 97  | OK |
| 25 | 370 | 37.0 | 37.0 | 10  | 99  | 99  | 99  | OK |
| 26 | 380 | 38.0 | 38.0 | 11  | 100 | 100 | 100 | OK |
| 27 | 390 | 39.0 | 39.0 | 12  | 102 | 102 | 102 | OK |
| 28 | 400 | 40.0 | 40.0 | 13  | 104 | 104 | 104 | OK |
| 29 | 410 | 41.0 | 41.0 | 14  | 106 | 106 | 106 | OK |
| 30 | 420 | 42.0 | 42.0 | 15  | 108 | 108 | 108 | OK |

Worked arithmetic for one row (detent 11, level −4, to show the method):
`dc = 270 + 10×(−4) = 230` → `C_dial = 230/10.0 = 23.0` → within
[12.0, 42.3], so `C_clamped = 23.0` → `F_dial = round(230 × 0.18 + 32.0) =
round(41.4 + 32.0) = round(73.4) = 73` → `somnus_level = 23.0 − 27.0 = −4`
→ `F_app = round(23.0 × 9/5 + 32) = round(41.4 + 32) = round(73.4) = 73`.

Table generated by a short throwaway Python script
(`/private/tmp/.../scratchpad/gen_table.py`, not part of the repo) applying
exactly these formulas over all 31 detents, cross-checked by hand for the
row above.

**Dead clicks (adjacent detents, identical post-clamp °C):** none. Every
detent's `C_dial` is already inside 12.0–42.3, so `C_clamped == C_dial`
everywhere and all 31 values are distinct by construction (`dc` steps by 10
every time).

**Dial °F vs. app °F disagreement (same °C, different displayed °F):**
none. `F_dial` and `F_app(calc)` are computed from the identical
`round(x × 9/5 + 32)` formula on the identical `C_clamped`, so they cannot
differ for any row in this table — confirmed above, all 31 rows match.

**Detents outside the pad's accepted 12.0–42.3 °C range:** none. The dial's
own rails (12.0–42.0 °C) sit entirely inside the pad's wider accepted range.

**Given app ladder (levels −15..+15) vs. computed `F_app(calc)`:** identical,
row for row, all 31 values (see table's rightmost two columns) — the task's
supplied reference ladder is reproduced exactly by `round(C×9/5+32)` on this
grid, no discrepancy to flag.

## RAW command output

```
$ git --no-optional-locks status
On branch main
Your branch is up to date with 'somnus/main'.

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	docs/REPORT-release-0.1.6-beta.3.md
	docs/SPEC-standby-poll.md

nothing added to commit but untracked files present (use "git add" to track)
```

```
$ grep -rn "f_to_c\|dial_f_to_c" firmware/dial-idf/components firmware/dial-idf/main
firmware/dial-idf/components/dial_state/dial_state.h:79:// dial_f_to_c(int f) alongside dial_c_to_f: it was the write-path half of
```

```
$ PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py --once
12:36:50  target_t=18C  level=-9  app_should_show=64F  current_t=23.97998C  on=False
exit=0
```

This is the ONLY network traffic to the pad this session generated: one
`GET /api/state`, to verify the new logger starts, parses a real response,
prints one correctly-formatted line, and exits cleanly on `--once`, per the
task's own instruction not to run a long capture. No firmware was built or
flashed — this task made no firmware change to build, so no `idf.py build`
was run.

```
$ git --no-optional-locks diff --stat
(no output — empty)
```

```
$ git rev-parse HEAD
53fc0c9dad016dac148a9887d91773b2a835c51c
```

## DEVIATIONS FROM SPEC

None. The absolute-face display, the write path, and the rails all match
the design the code's own comments describe (the 2026-08-30 "Q1 units fix"),
and that design produces zero dead clicks, zero dial/app disagreements, and
an exact match to the Somnus app's own ladder over the full 31-detent range.

## NOT VERIFIABLE WITHOUT HARDWARE

- This audit is a static trace plus one read-only `GET /api/state` sample
  (`target_t=18`, `on=false`) — it did not observe a live knob turn, so it
  cannot directly confirm the *running* firmware on the bench dial matches
  this source tree (vs. an older flashed build). `git rev-parse HEAD` above
  pins what the trace was done against; Matthew should confirm that's what's
  actually flashed, or reflash first, before trusting a live click session
  against this report.
- What the dial does with a setpoint the pad reports **between** grid points
  — e.g. 42.1–42.3 °C, reachable only by the app or another client, never by
  a dial detent (Part A). `render_value()`'s °C branch (`scr_dial.c:441-443`)
  would print a fractional value like "42.3" with no rounding; what
  `dial_dc_to_f()` shows for that same off-grid state, and what the very
  next detent does to it (Part A's own comment at `scr_dial.c:1450-1458`
  describes the *snap* behavior for this case, but it's untested here), is
  a real-hardware question, not a source-trace one.
- Whether the physical encoder ever produces `detents` values other than
  ±1 per debounce window (a fast spin coalescing into a multi-step jump) —
  `bidi_switch_knob.c`'s debounce/coalescing behavior was not traced in this
  audit (out of scope: this audit is about the absolute-face °F **display**
  math, not encoder debounce), so an unusually fast real turn is not ruled
  out as a source of an apparent "skip" that this report's table wouldn't
  show as a bug.
- Whether the Somnus app itself actually implements `round(C×9/5+32)`
  faithfully on its own live display — this audit took the app's ladder as
  given data (per the task) and confirmed the dial's arithmetic reproduces
  it; it did not, and per rule 5/task scope was not asked to, inspect the
  app's own code or a live app screen.

## Fill-in block — click-by-click hardware verification

Matthew: run `PAD_HOST=<pad-ip> python3 tools/dial_display_audit.py` (no
`--once`) while turning the knob on the absolute face, and fill in one row
per click. The logger appends every line to `docs/dial-audit-run.log` too,
so the last column can be copied from there after the fact instead of
transcribed live.

| click # | DIAL showed | SOMNUS APP showed | logger `target_t=` line (timestamp) |
|---|---|---|---|
| 1 |  |  |  |
| 2 |  |  |  |
| 3 |  |  |  |
| 4 |  |  |  |
| 5 |  |  |  |
| 6 |  |  |  |
| 7 |  |  |  |
| 8 |  |  |  |
| 9 |  |  |  |
| 10 |  |  |  |

---

# PART D — Live run results (2026-09-07, 12:41–12:42 local)

Appended after the static audit above. Source: `docs/dial-audit-run.log`,
39 change-events from a real click session on the absolute °F face,
`PAD_HOST=192.168.1.169`, 1 Hz `GET /api/state` only. No writes were issued
by this tooling; every setpoint change in the log came from Matthew's own
knob turns.

## D1. Verdict

**No defect.** Matthew reports the dial and the Somnus app lined up across
the session, and the log independently corroborates the write path:

- **Every** logged `target_t` is a whole integer °C. Zero fractional values
  across 39 events. That kills H2 on real hardware, not just in source —
  a whole-°F internal unit would have produced values like 22.8 or 36.1.
- **Every** logged value maps into the Somnus app's own −15…+15 ladder.
  Distinct values observed: 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 29, 31, 38, 39, 40, 42 °C — 20 of the 31 grid points, including
  both rails (12.0 and 42.0). No value landed off-grid, and no value landed
  outside the rails.
- The Part B table's predicted `F_app` for each observed °C is what the
  logger printed, row for row.

## D2. The irregularity Matthew noticed, reproduced and explained

The slow-click segment at 12:42:00–12:42:09 is the clean one — roughly one
detent every two seconds, well inside the 1 Hz sampler's resolution:

```
12:42:00  target_t=19C  level=-8  app_should_show=66F
12:42:02  target_t=20C  level=-7  app_should_show=68F
12:42:04  target_t=21C  level=-6  app_should_show=70F
12:42:05  target_t=22C  level=-5  app_should_show=72F
12:42:07  target_t=23C  level=-4  app_should_show=73F   <-- +1, not +2
12:42:09  target_t=24C  level=-3  app_should_show=75F
```

Six consecutive detents, each exactly +1.0 °C, rendering as
66, 68, 70, 72, **73**, 75 °F. The step sizes are +2, +2, +2, **+1**, +2.

That single +1 is the whole complaint, and it is correct. It is the same
sequence `scr_dial.c:416-420` names verbatim as the expected result of the
Q1 units fix. The +1 lands every fifth detent, at levels −14, −9, −4, +1,
+6, +11 (°F 55, 64, 73, 82, 91, 100), because 31 whole-Celsius steps span
only 54 whole Fahrenheit degrees. The Somnus app's own ladder has the
identical stutter at the identical points — it is not a dial artifact.

## D3. What this run could NOT establish

**The method cannot resolve individual detents.** Two independent reasons,
both structural:

1. The logger samples at 1 Hz. Fast spins alias badly — 12:41:26→12:41:28
   shows 25 → 39 → 42 °C in two seconds, and 12:42:29 shows a 15 → 40 °C
   jump between adjacent samples. Those are almost certainly many detents
   between samples, not one detent moving 25 levels, but the log cannot
   distinguish the two.
2. The dial debounces and coalesces before it posts (`post_temp()`,
   `scr_dial.c:827-835`). Even at infinite poll rate the API would show the
   *settled* setpoint, never the per-detent sequence the screen displayed.

So the encoder-coalescing hypothesis from the static audit's
"NOT VERIFIABLE" section remains open in principle. The segment at
12:41:53–12:41:56 moved +2 levels per one-second sample
(14 → 16 → 18 → 20 °C), which is equally consistent with two clicks per
second and with one detent producing two levels. Nothing in this run
separates them.

**It does not matter for the reported symptom.** Matthew confirmed the
dial's number and the app's number agreed throughout, and every posted value
was on-grid. If a coalescing question is ever worth settling, it needs
`idf.py monitor` on USB with per-detent logging, not the pad's API — the API
is the wrong instrument for it. Not pursued; no symptom currently justifies
it.

## D4. Pad state — RESTORE REQUIRED

The pad's state before the session (12:36:50 and 12:40:52 samples) was:

```
target_t = 18 C   (level -9, 64 F)   on = False
```

Its state at the last logged sample (12:42:33) was:

```
target_t = 24 C   (level -3, 75 F)   on = True
```

**The pad was left powered on at 24 °C and has not been restored.** Per the
project's own rule that the pad is a bed in active use and must be returned
to its prior state after any write session, it needs to go back to
`target_t = 18`, power off — by hand on the dial or in the app. No tooling
in this audit will do it; nothing here is permitted to write.
