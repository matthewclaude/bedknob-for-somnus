# PLAN — Implementing the screen layout audit (for review)

> **Status (2026-09-04):** Section S and Tier A are done — Section S in commit `a009840` ("sim: scenarios for the layout audit"), Tier A in the commit immediately after it ("ui: layout fixes A1-A5 from the screen audit"); see `docs/REPORT-layout-phase1.md` and the audit report's "Resolution — phase 1". Tiers B, C, D untouched.

Companion to `docs/REPORT-screen-layout-audit.md`, which has the
measurements behind every item. This is the proposed change list, grouped
by risk, with the exact edit for each so the review can approve, drop, or
amend line by line. Nothing here has been done.

Scope at a glance: 12 firmware files, ~200 lines of layout change, ~150
lines of simulator scenarios, all 30 screenshots regenerated, one
`idf.py build` + flash + eyes-on pass. No state, power, networking, or
worker code is touched.

## Proposed order

1. Simulator scenarios first (section S), so the computed findings render
   and every fix gets a before/after pair.
2. Tiers A → C in one pass, one build, one flash.
3. Tier D as a separate decision — those two change what a screen looks
   like, not just where things sit.

## Tier A — fix before beta.5 (5 files, ~30 lines)

| # | File | Change | Verifies as |
|---|---|---|---|
| A1 | `scr_dial.c` | Delete the fixed `lv_obj_align(s_unit_lbl, CENTER, 266−CX, 122−CY)`. In `render_numeral()` and `alt_show_water()`, after the `lv_label_set_text`, add `lv_obj_align_to(s_unit_lbl, s_temp_lbl, LV_ALIGN_OUT_RIGHT_TOP, 22, −6)`. Update the file-header's "−10…+10" to the real ±15. | `dial.png` unchanged within 2 px; new `dial-celsius.png` shows "20.0 °C" with no overlap; new `dial-relative-max.png` shows "+15 LEVEL". |
| A2 | `scr_settings.c` | Give the Night mode row the Pad Address stacked shape: label `LEFT_MID 0,−16`; value `width LV_PCT(100)`, `LONG_DOT`, `LEFT_MID 0,16`. Same four lines for the Timezone row. Unconditional, so the row shape never reflows. | Fresh `settings.png` (already renders the annotation) shows two clean lines; new `settings-timezone-raw.png` with a raw IANA value. |
| A3 | `scr_dial.c` | `s_ota_lbl` y: `330 − CY` → `326 − CY`. Adjust the create() comment's budget arithmetic (316 → 337, label centred). `scr_standby.c` untouched. | `dial-update.png`: ink 321–332, 2 px above dots, 5 px below disc. |
| A4 | `scr_sidepick.c` | Move the title's `lv_label_create` block to after the halves-and-divider loop (or add `lv_obj_move_foreground(s_title)`); y `TOP_MID 36` → `72`. | `sidepick.png` finally shows the title, inside the 288 px chord. |
| A5 | `dial_list.c` | `#define ZOOM_MIN 168` → `140`. Update the comment with the chord arithmetic (far row content 101–259 vs chord 96–264). | `settings.png` "Absolute" and `wifi-info.png` "dBm" fully inside the mask; every other list screenshot eyeballed for the stronger curvature. |

Decision needed on A5: the alternative is `OPA_MIN 100 → 50` (fade instead
of shrink). It hides the clip rather than removing it, but leaves the
rotor's look unchanged. Recommend the zoom change; flagging the choice.

## Tier B — constants and copy (6 files, ~15 lines)

| # | File | Change |
|---|---|---|
| B1 | `scr_update.c` | `OTA_AVAILABLE` string: `"v%s available - tap to install"` → `"v%s - tap to install"` (beta strings were 291 px in a 288 px row). |
| B2 | `scr_netpick.c` | In `make_row()`: `lv_obj_set_width(lbl, LV_PCT(100))`, `lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT)`, `text_align CENTER` — replaces the bare `lv_obj_center`. |
| B3 | `scr_passkey.c` | `"That password didn't work. Try again."` → `"Wrong password, try again"` (276 px, fits the 280 px `LONG_DOT` label). |
| B4 | `scr_adjust_mode.c` | Back pill `310 − CY` → `300 − CY`; description `235 − CY` → `228 − CY`. Update the "y-offsets below" comment. |
| B5 | `scr_night_mode.c` | Title `64 − CY` → `56 − CY`; note `84 − CY` → `74 − CY`. This is the one list screen whose title leaves the shared 64 slot; comment says why (the note needs the seam). |
| B6 | `scr_wifi.c` | Confirm body `126 − CY` → `116 − CY` (14 px above Continue instead of 4). |
| B7 | `scr_standby.c` (optional) | Dots `104` → `112`, clock `168` → `176`, date `232` → `240`: centres the block on the panel. Skip if the current placement is deliberate. |

## Tier C — small rewrites (2 files, ~50 lines)

| # | File | Change |
|---|---|---|
| C1 | `scr_update.c` | Replace the three hand-aligned lines of the "Check for updates" row with a flex-column block: port About's `make_info_row()` shape (main-axis `CENTER`, `pad_row 4`) with cross-axis `LV_FLEX_ALIGN_START` so lines stay left-aligned. Label / value / error become flex children; the error line keeps `LONG_DOT` and is hidden (not empty) when not FAILED so the block re-centres. Two-line state centres at 13.5 px slack; FAILED state is 68 of 76. |
| C2 | `scr_connecting.c` | `create()`/`on_state()`: `pal->bg`, `ink_primary` for the main label, `ink_secondary` for the sub (warning already comes from `PAL()`). Block offsets: main `−12` → `−24`, sub `+28` → `+24`, so the 4-line degraded case (main 145–167, sub 168–240) no longer overlaps at the box level and centres at 192. Adds `#include "dial_palette.h"` via the existing internal header — no new dependency. |

## Tier D — redesigns, separate approval (2 files, ~70 lines)

| # | File | Change | Why it is a judgment call |
|---|---|---|---|
| D1 | `scr_brightness.c` | Replace the "%"-below-the-numeral slot with `scr_updating.c`'s flex row (digits + "%" glued on the baseline, `pad_column 4`, row centred at 150). Keep `s_unit_lbl` at 214 as a caption used only for the clock row's "Off" at 0; the flex-row "%" hides in that state. | Changes the picker's silhouette. Three screens currently place the unit three ways; this picks Updating's as the standard. Alternative is to leave it and accept the 26 px gap as the screen's own vocabulary. |
| D2 | `scr_pad_discovery.c` | Rebuild `create()` on the `scr_updating.c` layout: r=165 chassis ring as the progress indicator, title "LOOKING FOR PAD" at 64, fraction as the hero at 150 in `lv_font_montserrat_48` (`num_88` has no "/"), headline as the 220 caption (`width 260`, wrap). Drop the off-centre r=110 arc. | A full relayout of a screen with no checked-in render and no easy hardware repro (needs a real discovery scan). Simulator scenario S6 is the only verification short of pulling the pad off the network. |

## Section S — simulator scenarios (`simulator/main.c`, ~150 lines)

All follow the existing `scenario_*` pattern (apply_baseline, poke fields,
`generation++`, navigate, pump, snapshot). Names are the PNG names.

| # | Scenario | State it pokes | Proves |
|---|---|---|---|
| S1 | `dial-celsius` | `units_c = true`, zone A `temp_dc = 200` | A1 in °C ("20.0 °C") |
| S2 | `dial-relative-max` | `rel_mode = true`, `temp_dc = 420` (level +15) | A1 at the widest relative numeral |
| S3 | `settings-timezone-raw` | Needs a stub hook so `dial_time_get_iana_tz()` returns "America/Los_Angeles" (today `stubs.c` returns false); knob-walk onto Timezone | A2's Timezone row |
| S4 | `wifi-confirm` | Navigate to `SCR_WIFI`, `sim_tap` the "Change network" row (the harness already exists for passkey) | B6, and the confirm view gets its first render |
| S5 | `night-mode` | Navigate to `SCR_NIGHT_MODE` with night on (stub clock invalid → note shows) | B5 |
| S6 | `pad-discovery` | `phase = PH_PAD_DISCOVERY`, `phase_err = "Still looking (checking more slowly)...\n137/254"` | D2 (and today's layout for the before shot) |
| S7 | `update-failed` | `ota.status = OTA_FAILED`, `ota.err = "check failed (HTTP -1)"` | C1's three-line state |
| S8 | `pad-degraded-real` | Pad URL containing "unreachable" plus a realistic `phase_err` and `retry_in_s` | C2's four-line case |
| S9 | fix `scenario_settings_pad` | Comment describes the pre-2026-09-02 row order; +7 detents now lands on Rotation. Re-aim (+9 from Brightness reaches Pad Address in the current list) and rewrite the comment. | Existing screenshot becomes what it claims to be |
| — | `timezone`, `night-face` | Plain navigate-and-snapshot | Two list screens with no render at all |

Then regenerate all 30 (now ~40) PNGs. Five checked-in files are already
stale against the tree (`settings`, `settings-pad`, `update`, and the two
`about-wifi-*`), so the churn is expected and belongs in the beta.5
commit.

## Verification plan (one pass, same as the About pass)

1. Simulator: rebuild, run, inspect every PNG that a tier touched plus
   every list screen for A5. Ink-box check on the four measured items
   (A1 unit gap, A3 label/dots, A5 far-row edges, C2 block centre).
2. `idf.py build` zero warnings.
3. Flash to `/dev/cu.usbmodem83401`, confirm boot over pyserial.
4. Eyes-on on the dial: Settings → Units → °C and turn the knob across the
   range (A1); Settings with night on and no zone set (A2); Settings and
   Wi-Fi lists scrolled to their ends (A5); side pick via Settings (A4);
   an available update on the face (A3).

## Docs and bookkeeping

- `CHANGELOG.md` beta.5: one "Layout pass across all screens" bullet
  listing the user-visible items (A1–A5, B3, B4).
- `docs/REPORT-screen-layout-audit.md`: append a "Resolution" section
  recording what shipped and what was declined, same as the About report.
- `docs/screens/`: regenerated set; `simulator/main.c` comments updated
  where the row order drifted.

## Open questions for the review

1. A5: zoom floor (recommended) or opacity fade?
2. B7: is the standby block's 12 px-high placement deliberate?
3. D1 and D2: do either, both, or neither in this pass?
4. S3 needs a small `stubs.c` hook for a fake IANA zone — acceptable to
   grow the stub, or skip that scenario and verify A2's Timezone row on
   hardware only?
