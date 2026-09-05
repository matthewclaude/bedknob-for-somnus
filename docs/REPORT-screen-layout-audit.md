# REPORT — Layout audit of every dial screen (Sep 4 2026)

Owner-directed follow-up to `docs/REPORT-about-layout-battery-glyph.md`
pass 1: apply the same geometry-based scrutiny the About screen got to every
other screen, and write up recommendations. **No code was changed.** The
working tree is exactly as `docs/REPORT-handoff.md` left it.

## Method

Same as the About pass, plus two things that make the numbers exact rather
than eyeballed:

- **Fresh renders, not just the checked-in PNGs.** `build/dial_sim` was
  rebuilt from the current tree and run. Its output path is compile-time
  (`docs/screens/`), so the checked-in files were backed up first and put
  back byte-for-byte afterwards (`git status` on `docs/screens` unchanged
  before/after). Five checked-in screenshots do **not** match what the tree
  renders today — see "Stale screenshots" at the end. Everything below was
  measured against the fresh renders.
- **Font metrics from the compiled tables.** A scratch script parses the
  `adv_w`/kerning tables of the Montserrat sizes this UI uses and of
  `dial_font_num_88`/`_140`, and sums per-glyph advances the way
  `lv_txt_get_width` does. Calibrated against the render: it predicts "72"
  in `num_88` at x 127.5–232.5; the render's ink is 131–229 (the difference
  is side bearing). So width claims below for states the simulator never
  renders (°C mode, relative ±15, the Settings annotation) are good to
  ±3 px. Where a claim is computed rather than rendered, it says so.
- **Round-panel chord.** Half-width at height y is
  `sqrt(180² − (y−180)²)`. Worth keeping in mind that the physical bezel
  covers a few more pixels than the simulator's 180 px mask, so every
  "0 px margin" below is a real clip on the device.

Ink boxes quoted below (`x0–x1, y0–y1`) are measured from the fresh PNGs.

## Ranked summary

| # | Screen | Finding | Status |
|---|---|---|---|
| 1 | Dial face | Setpoint numeral overprints the unit label in °C mode and at relative levels ≥ ±10 | Computed, reachable via Settings → Units / Scale |
| 2 | Settings | "Night mode" value overprints its own label whenever the §7 clock annotation is on | **Rendered** (fresh `settings.png`) |
| 3 | Dial face | "Update available" descender overlaps the page dots by 2 px | **Rendered** |
| 4 | Side pick | "Which side of the bed?" title never renders (covered by the two halves) | **Rendered** |
| 5 | Every rotor list | The row two below focus is clipped by the panel edge (Settings "Absolu", Wi-Fi "dB") | **Rendered** |
| 6 | Update | "Check for updates" row is top-anchored (About's pre-pass defect #2), 9 px ink from the title | Rendered |
| 7 | Settings | Timezone raw-IANA / raw-POSIX values collide with the label | Computed, reachable |
| 8 | Netpick | Long SSIDs clip at both ends with no ellipsis | Computed |
| 9 | Passkey | The wrong-password message is ellipsized mid-word | Computed |
| 10 | Brightness picker | "%" floats 26 px under the numeral; three different unit placements across three screens | Rendered |
| 11 | Adjust mode | Back pill's bottom edge is tangent to the panel edge | Rendered |
| 12 | Night mode picker | Clock note at y=84 sits < 1 px (box) above the row-above-focus label | Computed |
| 13 | Connecting / error | Hard-coded pure black + fixed greys instead of palette tokens; block sits 20 px low when degraded | Rendered |
| 14 | Wi-Fi confirm view | Body text ends 4 px above the Continue button | Computed, no scenario renders it |
| 15 | Pad discovery | Off-chassis ring, headline corners at the chord, no scenario | Computed |
| 16 | Standby | Clock block centred 12 px above the panel centre | Rendered, cosmetic |

Items 1–5 are what I would fix before tagging beta.5. 6–12 are one-constant
or one-row changes. 13–16 are consistency.

## 1. Dial face (`scr_dial.c`) — `dial.png`, `dial-relative.png`, `dial-update.png`

### 1a. Numeral vs unit — collision in °C and at |level| ≥ 10

The unit label is a fixed slot: `lv_obj_align(s_unit_lbl, CENTER, 266−CX, 122−CY)`,
i.e. its ink is at x 257–275 (°F) or 236–297 ("LEVEL"), y 114–128. The
numeral is centred in a 210 px box; its ink band is y 119–181, so any
horizontal overlap is a visible overprint.

`dial_font_num_88` advances (px): 0=59 1=34 2=51 3=51 4=60 5=51 6=55 7=54
8=57 9=55 .=22 :=22 −=34 +=52.

| Mode | Numeral | Width | Ink span | Unit ink starts | Result |
|---|---|---|---|---|---|
| °F | "72" | 105 | 128–232 | 257 | 25 px clear (render: 131–229 vs 257) |
| Relative | "+3" | 103 | 129–231 | 236 | 5 px clear (render: 135–227 vs 236) |
| Relative | "+10" | 145 | 108–252 | 236 | **16 px overprint** |
| Relative | "−15" | 119 | 121–239 | 236 | 3 px overprint |
| °C | "22.5" | 175 | 93–267 | 257 | **10 px overprint** |
| °C | "20.0" | 191 | 85–275 | 257 | **18 px overprint** (unit almost fully covered) |
| °C | "45.0" (rail) | 192 | 84–276 | 257 | 19 px overprint |
| °C | "11.1" | 124 | 118–242 | 257 | clear (only 1-heavy values fit) |

Relative rails are ±15 (`DIAL_REL_MIN/MAX_DC` 120/420; the file-header's
"−10…+10" is out of date). °C mode is a plain Settings toggle. Both are
ordinary user states, not edge cases.

**Fix.** Anchor the unit to the numeral instead of to the screen: after
every `lv_label_set_text(s_temp_lbl, …)` (i.e. inside `render_numeral()` and
`alt_show_water()`), `lv_obj_align_to(s_unit_lbl, s_temp_lbl,
LV_ALIGN_OUT_RIGHT_TOP, 22, −6)`. With a 22 px gap the °F case lands where
it is today (unit box x 254–275 vs 255–277 now, same y), so `dial.png`
barely changes, and the wide cases push the unit right instead of under
the digits. Widest cases against the chord at the unit's y (≈112–135,
half-width ≥ 167): "20.0" box ends 275.5, + 22 + "°C" 22 → 320; "+15" box
ends 248.5, + 22 + "LEVEL" 64 → 335. Both inside x ≤ 347. The range-stop
nudge animates `s_num_box`'s x, not the label, so the unit stays put
during a nudge exactly as now. (Alternative: the
`scr_updating.c` flex-row idiom, digits + unit in one row; that changes the
unit from a superscript to a baseline suffix, which is a bigger visual
change than the problem warrants.)

### 1b. "Update available" vs page dots — 2 px overlap

Measured: label ink y 325–336 ("p" descender), dots ink y 334–342. The
label is centred at 330 in a slot whose budget is disc bottom 316 → dots top
337 (21 px) for a 15 px line. Centring it in that budget means y = 326:
ink 321–332, 2 px above the dots, 5 px below the disc. `s_ota_lbl`'s
`ext_click_area(14)` reaches 14 px past the box, so the touch target is
unaffected. Same slot is used by "Finalizing update…" (no descender, but
same fix applies). `scr_standby.c`'s copy at 330 has no dots to hit and can
stay.

### 1c. Everything else on the face checks out

Name 64 / underline 82 / water 98 / numeral 150 / pill 214 / disc 280 /
dots 340 — the §4 spec positions. Battery glyph at 46 shares the stale-dot
column (26) with 20 px clearance. Night-face minimal: "20.0" in `num_140`
is 304 px inside a 340 px box; "+15" 218 px; unit hidden, so no 1a issue
there. The `WATER` word at 92 (revision-3 fix) still holds.

## 2. Settings (`scr_settings.c`) — fresh `settings.png`, `settings-pad.png`

### 2a. Night mode row — value overprints the label (rendered)

Fresh `settings.png` shows "Night mode" with "9 pm - 7 am - set timezone"
drawn straight through it. Numbers: label "Night mode" (Mont 24) is 147 px
from x=36 → ends 183; the value (Mont 16) is 217 px right-aligned to 324 →
starts 107. **76 px of overlap.** "10 pm - 6 am - set timezone" is 224 px
(83 px overlap); "10 pm - 6 am - no clock" 185 px (44 px). Only the
un-annotated forms ("9 pm - 7 am" 97 px, "Off" and "Off - set timezone"
145 px) fit.

The simulator reaches it because its `dial_time_valid()` stub returns
false, but that is the honest state of a real dial with no zone persisted
("set timezone") or Wi-Fi up / SNTP down ("no clock") — exactly the §7
states the annotation exists for, and the timezone setup gate exists
because fresh devices sit there.

**Fix.** Give the row the Pad Address treatment (label nudged −16, value on
a second line at +16, full width, `LONG_DOT`): every form fits in 288 px,
and the two-line shape doesn't reflow when the annotation comes and goes.
Do it unconditionally, not only when annotated, so the row's shape is
stable. (Shortening the annotation to fit one line is the other option; the
longest that fits beside the label is ≈128 px, which rules out anything
that still says "set timezone".)

### 2b. Timezone row — raw values collide (computed)

Label "Timezone" 120 px → ends 156. Values the on_state can show:
curated labels (≤ "Central Europe" 183 px at Mont 24 on the picker; at
Mont 16 here they're all < 120 px, fine), **raw IANA** when the portal
applied a zone outside the 11 ("America/Los_Angeles" 177 px → starts 147,
9 px overlap), **raw POSIX** for a dial-v1.4.x-flashed unit
("PST8PDT,M3.2.0,M11.1.0" 187 px → 19 px overlap). Both are documented,
reachable states (`docs/REVIEW-2026-09-02.md` F2). Same stacked-row fix as
2a, or a `LONG_DOT` max-width of `288 − 120 − 12 = 156` on this value only.

### 2c. Factory reset row

"Tap again to confirm" (169 px) on its stacked +26 line ends at y 73 of 76,
3 px above the border, with the label centred at 38. Works; reads
bottom-heavy while armed. Optional: −8/+18 like Pad Address's −16/+16.

### 2d. The rest

Every other label + value pair fits: widest is "Screen timeout" 189 +
"30s" ≈ 35 → 224 of 288. "Night face / Number only" 159 + 107 = 266
(22 px spare). Pad Address's stacked layout is already balanced (content
8.5–63 of 76, centre 35.8). Title at the 64 seam is correct for these
centred rows.

## 3. Rotor list (`dial_list.c`) — clipping of the row two below focus

Rendered in `settings.png` ("Scale … Absolu" — the "te" is cut) and
`wifi-info.png` ("Strong -48 dB" — the "m" is cut). Geometry, for
`ROW_H 76`:

- The row two below focus rests at y 332, zoomed to 168/256 = 0.656 about
  its own centre (`ZOOM_MIN`, reached at `d = 2·row_h`).
- Row content is 288 px wide (`pad_hor 36`); a right-aligned value's right
  edge at 324 maps to 180 + 144·0.656 = **274.5**; the label's left edge
  at 36 maps to **85.5**.
- The value's text band is y 323–341; the panel half-width at y 341 is
  80.4 → x ≤ 260.4. At y 332 it is 96.4 → x ≤ 276.4. So the lower half of
  the last ~14 px of any value is masked; symmetric on the label's first
  glyph (half-width at the 24 px band's bottom, y 341, is 80 → x ≥ 100 vs
  the label at 85.5).

It is not a per-screen inset problem: to clear the chord at the current
zoom the insets would have to be ≥ 60 px, which starves the focused row.
The lever is the rotor's own falloff.

**Fix (one constant).** `ZOOM_MIN 168 → 140` (0.547). Far-row content then
spans 101–259 while the chord at the band's bottom (y 339.4) is 96–264:
≈5 px margin both sides. Side effects, all benign: neighbours (1 row away)
are untouched at their quadratic-eased ~0.91; the Menu's "About" row at the
far position gains a little clearance from the page dots (ink gap 8 → ≈9 px);
About's 3-line blocks shrink the same way. If the stronger curvature is
unwelcome, the alternative is `OPA_MIN 100 → ~50` so the far row reads as a
fade into the bezel rather than a cut — but that hides the clip rather than
removing it. `ROW_H 72` for the two-column lists (the menu's pitch) does
*not* fix it on its own: the margin at that pitch is 0.3 px.

## 4. Side pick (`scr_sidepick.c`) — `sidepick.png`

The "Which side of the bed?" title (Mont 20, `TOP_MID` y 36) is created
before the two half-panels, which are full-height opaque `lv_obj`s
(`bg_opa` default COVER, colour from `apply_highlight`). LVGL draws later
siblings on top, so **the title is never visible** — the render has no ink
anywhere in y 30–62. It has been dead since the halves were added.

Two things to fix together: create the title after the halves (or
`lv_obj_move_foreground`), *and* move it down — at y 36 its 235 px width
exceeds the 216 px chord there. y 72 gives a 288 px chord (26 px margin per
side) and still clears the LEFT/RIGHT labels at 180. Or delete the label
and its `apply_highlight` colour line, if the two words alone are judged
enough; either way the current state is a silent no-op.

## 5. Update (`scr_update.c`) — fresh `update.png`

### 5a. "Check for updates" row is top-anchored

Label at −20 (box 4.5–31.5 of 76), value at +6 (35–53), error line at +26
(57–71, FAILED only). In the normal two-line state the content spans
4.5–53: 23 px of dead band at the bottom, block centre 11 px above the row
centre — the same defect About's info rows had before the pass. In the
render the row sits one above focus, and the "UPDATE" title ink (58–69) is
9 px from the "Check for updates" ink (78–97): not a collision, but the
title-to-content gap that About's pass established as the seam is gone.

**Fix.** Port About's `make_info_row()` shape (flex column, main-axis
CENTER) with cross-axis `LV_FLEX_ALIGN_START` so the lines stay
left-aligned. Two lines centre themselves (49 px content → 13.5 px slack
each side); the FAILED three-line state (27+18+15 plus two 4 px gaps = 68) still fits 76.
Nothing else on the screen needs the stacked shape.

### 5b. Long version strings

"v1.4.3 available - tap to install" is 234 px (fits 288). A beta string
"v1.2.0-beta.3 available - tap to install" is 291 px and ellipsizes to
"…tap to inst…". The row's own label already says what this is; "v1.2.0-
beta.3 - tap to install" would fit with room. Copy change only.

## 6. Wi-Fi (`scr_wifi.c`) — `wifi-info.png`, confirm view unrendered

- List view: row 3 ("Signal … Strong -48 dBm", 131 px) is the §3 clip
  case; nothing else to do here once `ZOOM_MIN` moves.
- Confirm view (no simulator scenario; computed): body wraps at 240 px to
  **4 lines** ("The dial restarts into setup," / "where you can choose a
  new" / "network on the dial or from" / "your phone.", widest 235 px),
  72 px tall centred at 126 → y 90–162. Continue button is 166–254.
  **4 px gap.** One more word wraps to 5 lines and overlaps the button by
  14 px. Move the body to 116 (80–152; chord at y 80 is 300 px, fine) for a
  14 px gap. Widening the label to 260 does not help — the first line
  would need 271 px to take one more word, so it still wraps to 4. Cancel
  pill's outer corner is 174 px from centre (6 px inside the mask) — fine.
- Add a `scenario_wifi_confirm` (tap "Change network") so this view gets a
  checked-in render like everything else.

## 7. Menu (`scr_menu.c`) — `menu.png`

Fine. "About" at the far position: ink 318–329 vs dots 337–342, 8 px. The
notification dot at `OUT_LEFT_MID −8` of a centred "Update" label reads
right. Rows are centred so the seam/chord issues don't apply. The only
cross-screen note: Back is centred here and on Netpick, left-aligned on
every other list; not a defect, but the two families should stay that way
deliberately rather than by accident.

## 8. Netpick (`scr_netpick.c`) — `netpick.png`

Row labels are `lv_obj_center`ed with no width and no long-mode. An SSID
can be 32 bytes; at Mont 24 a 32-character name is ≈400 px against the
288 px content width, so it overflows both ends and is hard-clipped by the
row (no ellipsis, no hint that it was cut). "Bluebird Cottage" (209 px)
already uses 73 % of the width. Fix: `lv_obj_set_width(lbl, LV_PCT(100))`,
`LV_LABEL_LONG_DOT`, `text_align CENTER` in this file's `make_row()`. Same
for Passkey's title (`s_title_lbl` already does this, 260 wide — good).

## 9. Passkey and Pad Address (`scr_passkey.c`, `scr_pad_address.c`)

The two wheels are laid out identically (title 56, readout 92/96, wheel
170, hairline 200, discs 240, captions 292) and the disc/caption geometry
in the source comment checks out (caption "Delete" at y 285–299, x 65–103;
chord there ≥ 45). Two small things:

- **Wrong-password copy is ellipsized (computed).** "That password didn't
  work. Try again." is 380 px at Mont 20 in a 280 px `LONG_DOT` label →
  it renders as "That password didn't work. T…". This is the one message
  a stuck user most needs whole. "Wrong password, try again" is 276 px
  and fits on one line; or switch the label to `LONG_WRAP` for the
  message state (two lines, 44 px, would push into the wheel's 170 slot —
  so prefer the shorter copy).
- Pad Address opens with the wheel at "0" and the readout pre-filled; the
  candidate's 100 px hit box (120–220) overlaps the Add disc (196–284) by
  24 px. Both fire `cand_event_cb`, and LVGL hit-tests the later-created
  disc first, so no behaviour issue — noting it so nobody "fixes" it into
  two different actions.

## 10. Brightness picker (`scr_brightness.c`) — `brightness*.png`

Rendered: numeral ink 119–181, "%" ink 207–220 → a 26 px void between a
number and its unit, and the "%" sits alone on the pill slot (214). Three
screens now place the same "big number + unit" vocabulary three ways: Dial
(fixed superscript top-right), Updating (flex row, unit on the baseline),
Brightness (unit centred below). Recommend the Updating idiom here — it is
the same "big percent" job, and its flex row already handles "5" → "100"
without the gap moving. The clock row's "Off" at 0 can stay in the 214
slot as a caption (`s_unit_lbl` becomes caption-only; the flex row's unit
label hides at 0). Title at 84 (not the shared 64) is documented and still
justified: "DAY BRIGHTNESS" is 146 px and the ring's inner chord at y 55 is
162 px.

## 11. Adjust mode (`scr_adjust_mode.c`) — `adjust-mode.png`

Back pill 140×72 at 310 → bottom edge y 346, where the panel chord is
110–250 — exactly the pill's x extent. Only its 36 px corner radius keeps
the ink inside the mask, with 0 px to spare at the tangents; on the device
the bezel will sit on it. Move to 300 (bottom 336; chord 90–270, 20 px
margin) and the description from 235 to 228 so the gaps stay even (pills
end 186; description 30 px → 213–243; Back starts 264). Option pills at
y 114–186 clear the chord by 19 px at their outer corners; title at 64
("ADJUSTMENT MODE" 169 px vs a 242 px chord at y 55) is fine.

## 12. Night mode picker (`scr_night_mode.c`) — no scenario (computed)

Title at 64 plus a §7 note (Mont 12) at 84 → note box 76.5–91.5. The row
above focus is centred at 104 and zoomed 0.914; its Mont 24 label box is
91.7–116.3. **0.2 px** between boxes; ~7 px between cap-heights. Same
seam logic the About pass fixed: anything at 84 lives inside the upper
row's box. Move the pair up — title 56 (box 47–65; chord at 47 is 242 px
vs "NIGHT MODE" 108), note 74 (box 66.5–81.5) — for 10 px clear of the
scaled label. Night face (`scr_night_face.c`) has no note and is fine.

## 13. Connecting / error (`scr_connecting.c`) — `connecting.png`, `pad-unreachable.png`

- Background is `lv_color_black()` and the inks are literal `0xe0e0e0` /
  `0x808080`; every other screen paints `pal->bg` (#101418 day, #100C08
  night) and palette inks. Boot (this screen → dial) and every
  PH_DEGRADED transition therefore flash from pure black to the chassis
  colour, and at night the fixed light-grey text ignores the ember palette
  whose whole point is the blue-channel rule. `main_color` already uses
  `PAL()->warning` for degraded, so the plumbing is half there: use
  `pal->bg`, `ink_primary`, `ink_secondary`, set from `on_state`.
- Degraded layout: main label at −12 (Mont 20, 22 px) and a subtitle at
  +28 that wraps to **4 lines** with a realistic error ("Somnus pad at
  192.168.1.100:8080 not responding (HTTP -1)" / "Retrying in 27s" /
  "Swipe left for menu" → 72 px). The block then spans 157–244, centre
  200: 20 px low, and the main label's bottom (179) touches the sub's top
  (172) — 7 px of overlap at the box level (the 20 px font's descender
  band). With the simulator's shorter 3-line sub (54 px) the gap is 2 px. Fix:
  compute the pair as one block — sub at `+30` with the main at
  `−(sub_height/2 + 14)` — or simply main −24 / sub +24 for the 4-line
  case (main 145–167, sub 168–240, centre 192).
- The 320 px sub width is justified (first line 303 px) and clears the
  chord at y 168–240 (half-width ≥ 166).

## 14. Pad discovery (`scr_pad_discovery.c`) — no scenario (computed)

The only screen whose ring is not the r=165 chassis: an r=110 arc offset
30 px down, fraction "n/N" (Mont 24) at its centre (210), headline (Mont
16, 280 wide) at 60. The pass-2 headline wraps to two lines ("Still
looking (checking more" 225 px / "slowly)…"), box 42–78; the chord at y 42
is 231 px, so the first line's corners sit 3 px inside the simulator mask
and under the physical bezel. Recommend adopting `scr_updating.c`'s layout
wholesale — it is the same job (progress ring, one big number, one
caption): chassis ring r=165 as the progress indicator, title at 64
("LOOKING FOR PAD"), the fraction as the hero at 150 (Mont 48 — `num_88`
has no "/"), headline as the 220 caption. Then add a
`scenario_pad_discovery` so it has a checked-in render.

## 15. Update prompt (`scr_update_prompt.c`) — `update-prompt.png`

Checks out end to end: grab bar at 10 (chord 118 px, bar 40), title 172 px
at y 24–47 (chord at cap-height y 29 is 196 px), "Update now" 300×88 at
122–210 (chord at 122 is 340 px), "Later" 240×72 at 218–290 (chord at 290
is 284 px), "Update options" 127 px at 296–336 (chord at 336 is 180 px).
Body wraps to exactly the two lines the string's own `\n` asks for (229 /
221 px in 300). Nothing to change.

## 16. Standby (`scr_standby.c`) — `standby.png`, `standby-update.png`

Presence dots ink 100–107, clock 137–199, date 226–237: block 100–237,
centre 168.5, i.e. 11.5 px above the panel centre. Cosmetic; the face
reads fine because the ring's bottom gap is empty. If it is ever nudged,
+8 on all three (dots 112, clock 176, date 240) centres it at 176 and the
date's chord at y 249 is still 332 px. "Update available" at 330 has no
dots to collide with here; leave it.

## 17. Screens with nothing to report

- **Welcome:** block 130–225, centre 177.5; tagline 241 px in 260. Fine.
- **Wi-Fi portal (`scr_setup.c`):** step text 3 lines 69–123 (widest
  172 px), SSID at 166 (`LONG_DOT` 300), hint at 200, pill 240×72 at
  236–308 (chord at 308 is 253 px, pill corners rounded 36). Fine.
- **Updating:** ring, "62 %" flex row, caption at 220 (200 px in 260).
  Fine — and it is the reference layout §10 and §14 should copy.
- **Timezone, Night face, Brightness menu:** stock 76 px rows, widest
  label "Central Europe" 183 px / "Night (in use)" 164 px + "20%" — fit.
  Only the §3 far-row clip applies. Timezone and Night face have no
  simulator scenario.
- **About:** as left by the previous pass; the fresh render matches
  `about.png` byte-for-byte. Only the two Wi-Fi variants are stale (below).

## Stale screenshots and missing scenarios

Fresh renders that differ from the checked-in file (copies saved under
`Claude outputs/screens-fresh-2026-09-04/` for viewing; `docs/screens/`
itself was left untouched):

| File | Why it differs |
|---|---|
| `settings.png` | Current tree shows the Night face row and the §2a overprint; checked-in file predates both |
| `settings-pad.png` | Row order changed (Night mode/Night face/Screen timeout added; Adjustment mode hidden) — the checked-in shot is 7 detents into an older list |
| `update.png` | "Installed" row value and the Auto-update window string changed |
| `about-wifi-real.png`, `about-wifi-worst.png` | Rendered by an earlier pass with a different fake AP; untracked, so nothing to preserve |

The `scenario_settings_pad` comment still describes the pre-2026-09-02 row
order (Adjustment mode at index 1); +7 from Brightness now lands on
Rotation, not Pad Address. Worth re-aiming when the screenshots are
regenerated for the beta.5 commit.

States with no render at all today, in the order they'd earn their keep:
dial in °C (§1a), dial relative at ±15 (§1a), Settings with the clock
annotation and a raw-IANA timezone (§2a/2b), Wi-Fi confirm view (§6), night
mode picker with a note (§12), pad discovery (§14), update FAILED (§5a's
three-line state), degraded with a real pad error (§13), timezone and
night-face pickers, dial with the zone off.

## What this pass did not do

No code, no screenshots, no simulator scenarios were changed. Every number
above is from the current tree's own render or its own font tables; the
"computed" items are the ones the simulator cannot currently show, and
those are the first scenarios to add before fixing so the fix has a
before/after like the About pass did.

## Resolution — phase 1 (2026-09-04)

Section S and Tier A of `docs/PLAN-screen-layout-fixes.md` shipped in two
commits: `a009840` (scenarios + the BEFORE renders) and the commit after it
(`ui: layout fixes A1-A5 from the screen audit`, code + the AFTER renders +
these doc edits). Tiers B–D were not touched. Numbers below are ink boxes
from the fresh PNGs, measured the same way as above (bg-diff threshold 40
inside the 180 px mask; the far-row check at threshold 12 because that row
is faded to opa 100/255); "before" is the `a009840` render.

| # | Fix | Before (a009840) | After | Verifies as |
|---|---|---|---|---|
| A1 | `scr_dial.c`: unit anchored to the numeral (`place_unit()`: `OUT_RIGHT_TOP`, +22, −6) | `dial.png` °F ink 257–275 × 114–128. `dial-celsius.png` "20.0" 87–276 with °C drawn **under** the digits (no separate unit run). `dial-relative-max.png` "+15" 117–~250 with "LEVEL" starting at 236 (overprint). | `dial.png` °F 256–274 × 115–129 — **1 px** shift, 92 px differ in the whole panel. `dial-celsius.png` "20.0" 87–270, °C 297–317: **27 px clear**. `dial-relative-max.png` "+15" 117–246, "LEVEL" 273–334: 27 px clear of the digits. | pass / pass / pass — see the residual below |
| A2 | `scr_settings.c`: Night mode + Timezone rows stacked (label −16, value full-width `LONG_DOT` +16), unconditional | `settings.png` Night mode row: one ink band 248–268 with the 224 px value through the label. `settings-timezone-raw.png` (new): "America/Mexico_City" 60–322 through "Timezone" 37–154. | `settings.png` Night mode row: two bands, label 234–253 and value 265–277. `settings-timezone-raw.png`: label 155–172, value 188–204. | pass — two clean lines on both |
| A3 | `scr_dial.c`: `s_ota_lbl` y 330 → 326 | `dial-update.png` label ink 325–337 vs dots 337–342: **1 row shared** (the "p" descender). | label 321–333, dots 337–342: 3 empty rows between, 4 below the disc (316). | pass — plan said 321–332; the 1 px difference is the measurement threshold (before measured 325–337 the same way where the audit said 325–336) |
| A4 | `scr_sidepick.c`: title created after the halves, y 36 → 72 | `sidepick.png`: **no ink** in y 20–110 (title covered). | title ink 63–296 × 75–89; chord at y 89 is 24–336 (39 px margin each side); LEFT/RIGHT at 180 untouched. | pass |
| A5 | `dial_list.c`: `ZOOM_MIN` 168 → 140 | `settings.png` / `wifi-info.png` far row: max ink radius **178.3** (on the mask edge = clipped; "Absolu", "-48 dB"). | max radius 174.2 / 173.8; far-row content 103–~250, chord at the band's bottom 96–264. "Number only" and "Strong -48 dBm" complete. | pass |

Every list screenshot changed under A5 (about*, brightness-menu, menu,
netpick, night-face, night-mode, settings*, timezone, update*, wifi-info) —
the far rows are smaller and dimmer; neighbours one row away are unchanged.
The simulator was rebuilt and `idf.py build` finished with zero warnings.
Not done here: the flash + eyes-on pass (plan verification steps 3–4).

**Residual worth a look (A1, relative +15 only):** with the unit riding the
numeral, "LEVEL" at level +15 ends at x = 334 while the r = 165 chassis
ring's inner edge at that height (y 116–129) is at x ≈ 315–322, so the
final "L" sits on the ring by ~12–17 px — visible in
`dial-relative-max.png` where the accent arc is at full extent. The plan's
chord check (≤ 347) was against the mask, not the ring. Levels ≤ +9 and
every absolute value are clear of it ("+3 LEVEL" ends at 317). Options if
it bothers: a smaller gap (16 px puts the "L" at 328), or a smaller unit at
|level| ≥ 10. Left as specified.

**Scenario notes:** S3 uses `America/Mexico_City`, not the audit's
`America/Los_Angeles` — that one is in `DIAL_TZ_IANA[]` and renders as
"Pacific", so it never exercised the raw-IANA path; Mexico_City is the same
19 characters. S7 needs no knob-walk: `scr_update.c` opens on "Check for
updates" already (row 1), so `scenario_update`'s "Installed(1)/Check for
updates(2)" comment is stale (left alone — not a Section S row).
`settings-pad.png` at +9 came out byte-identical to the checked-in file, so
only four of the five "stale" shots actually changed in `a009840`.

**A1b (2026-09-05, follow-up to A1):** the residual above turned out to be
the general case, not a +15 corner. On hardware in °C every setpoint
rendered as four glyphs ("34.0") because the setpoint always lands on a
whole degree (1.0 °C per level, level 0 = 27.0 °C, rails 12 and 42), so the
".0" carried nothing and pushed the anchored unit onto the chassis ring and
under the handle at every °C value; in relative mode "+15 LEVEL" overlapped
the ring for |level| ≥ 10. Two changes in `scr_dial.c`: `render_value()`'s
°C branch drops a zero tenth ("34", not "34.0") and keeps a non-zero one
(the branch is shared with the night-face water alternation, whose reading
is fractional — "23.4" still reads "23.4"), and `apply_palette_and_state()`
hides the unit label in relative mode instead of writing "LEVEL" into it
(hidden if minimal OR relative; otherwise °C/°F). Measured on the fresh
renders: `dial-celsius.png` now reads "20 °C" with the °C ink box at
x 258–278 × y 115–129 (right edge 278, inside the °F slot's 280 and 37 px
short of the ring's inner edge at ≥ 315; was 297–317). `dial-relative-max.png`
shows a bare "+15" — the 364 px that differ from the A1 render are exactly
the old "LEVEL" area (x 273–334); `dial-relative.png` likewise loses its
"LEVEL" (x 256–317). `dial.png` and `dial-update.png` (°F) are pixel-identical
to the A1 renders. A new scenario, `dial-night-water.png`, renders the
Number-only night face mid water-phase in °C with the water at 23.4 °C and
the setpoint at 34 °C: "23.4" in the accent with WATER above it, proving
the tenth survives. Report: `docs/REPORT-layout-a1b.md`.

**Range-stop / drag grid bug (2026-09-05, found on hardware during the
phase-1 eyes-on; the drag half in the A1b bench log):** in absolute mode the
setpoint could walk off the whole-degree grid by two routes. (a) Knob:
`main.c` seeded the dial's rails from the API's accept range, `{ 120, 423 }`,
and `on_knob()`'s absolute branch stepped `s_shown_dc ± 10` and clamped to
that 423, so one detent above 42.0 landed on 42.3 and every later value
carried the 3 until the 12.0 rail reset it. (b) Drag: the handle's release
handler in `scr_dial.c` snapped to the grid only in relative mode and posted
the raw `value_from_point()` tenth otherwise (bench log: 42.3, then 41.3 …
13.3 by knob). Fix, in one commit: the rails are now `{ 120, 420 }` — the
Somnus app's whole-degree scale, identical to `DIAL_REL_MIN_DC/MAX_DC`; the
pad accepts up to 42.3 but the dial never sets it (dial_state.h comments
updated to match, dial_somnus.h left as the API description). `on_knob()`
snaps the displayed value to its nearest whole degree before stepping
(round-half-up, then the existing clamp), so an off-grid start such as 42.3
or an app-set 33.5 lands on the grid on the first detent and the range-stop
test still fires at both rails from on-grid values. The release handler
snaps to the nearest whole degree in absolute mode exactly as it snaps to a
level in relative mode, before the render and the post. Six simulator
scenarios drive it through the real `on_knob` and pointer-indev paths:
`rails-420-up.png` "42" (pinned, nothing posted), `rails-420-up-down.png`
"41" (posts 410), `rails-423.png` "42.3" with the handle pinned at the rail
end, `rails-423-up.png` "42" (posts 420 — not pinned, by design),
`rails-drag-337-live.png` "33.7" mid-drag, `rails-drag-337.png` "34" on
release (posts 340). Report: `docs/REPORT-rails-fix.md`.
