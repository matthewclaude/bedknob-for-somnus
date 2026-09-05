# REPORT — About layout pass + battery badge/percentage review (Sep 4 2026, evening)

Two owner-directed passes on top of the uncommitted `0.1.5-beta.5` working
tree (`docs/REPORT-battery-pct-about-redesign.md` and its block-redesign
addendum). Both built clean, both flashed to the dial and confirmed booting
over serial. **Nothing is committed** — see `docs/REPORT-handoff.md`.

## Pass 1 — About screen layout ("optimize the layout but retain the way it's organized")

Organization unchanged: Back, then Firmware / IDF / Wi-Fi / Pad / Battery as
centred title / value / detail blocks, `dial_list` rotor, `ROW_H` 76.

### Defects found in the checked-in screenshots

1. **Title overprint.** The "ABOUT" heading had been pulled down to
   `84 - CY`, which is *inside* the box of the row above the focused one.
   While Back was that row it looked fine; once the list scrolled, the info
   rows' own titles ("IDF", "Pad") rendered straight through "ABOUT"
   (`about-battery-pct.png` before this pass showed it).
2. **Uneven vertical rhythm.** Blocks were top-anchored with a zero-slack
   fit (`PAD_TOP 3`, `PAD_ROW 5`, 3 + 63 + 10 = 76). Wi-Fi/Battery hugged the
   bottom border while Firmware/IDF/Pad left a 23 px empty band, and the
   focused block sat ~12 px above the rotor's centre line.

### Fix (`firmware/dial-idf/components/dial_ui/scr_about.c`)

- `make_info_row()`: main-axis `LV_FLEX_ALIGN_CENTER` (was `START`),
  `pad_ver 0`, `INFO_ROW_PAD_ROW 4` (was 5), `INFO_ROW_PAD_TOP` removed.
  3-line rows now have 5 px slack split top/bottom; the focused block is on
  the screen's centre. The old comment's reason for top-anchoring did not
  hold: the text-to-text gap between two consecutive 2-line rows is 26 px
  either way — only the border's position inside that gap moves, and it now
  falls mid-gap where a divider belongs.
- Title back to the shared `64 - CY` slot (`scr_settings.c`, `scr_wifi.c`).
  With `pad_top = 180 - ROW_H/2`, row boundaries park at y = 66 and 142, so
  64 is the seam between the two rows above focus, where the rotor's
  zoom-down keeps content clear.
- Back row and `make_row()` untouched.

### Verification

Simulator rebuilt, all five About screenshots regenerated and inspected
(`about.png`, `about-wifi-real.png`, `about-wifi-worst.png`,
`about-battery-pct.png`, `about-battery-usb.png`): no overlap in any
scrolled state. `idf.py build` zero warnings. Flashed; owner viewed it on the
dial: "that is perfect - the way it should be done."

## Pass 2 — battery badge + discharge curve ("analyze ... and see if you agree")

### Findings (assessment delivered first, owner approved all changes)

- **Badge structure is right** (drawn outline/nub/fill inside a transparent
  wrapper whose opa dims/breathes all three; hidden while plugged; 3 s
  CHARGE flash on the old label). **Its resolution was not:** 16 px body,
  2 px inset each side → 12 px usable, floor-rounded with a 2 px minimum.
  0–24 % all drew as 2 px; 78 % drew as 75 %. The 15 % threshold sat inside
  a band the bar could not move in — colour was the only signal. Confirmed
  by rendering the dial face at 100/78/50/30/20/10 % and magnifying.
- **Curve shape is fine** (resting-LiPo table from upstream PR #4; the
  3.75–3.85 V plateau at 3–5 mV/% is the real knee; 0 % at 3.5 V is
  conservative in the safe direction since §9.5 shows the board running
  below 3.2 V). Consequence: the 15 % warning at ~3.70 V fires ~250 mV
  earlier than the factory firmware's red blink at ~3.45 V — long runway,
  by design. §11.1's "3450 mV ≈ 3 %" claim was wrong (clamps to 0 %); fixed
  in the spec.
- **Sampling was the real problem:** `power_pct` came from each single 1 s
  ADC reading. On the plateau, ADC noise plus a Wi-Fi TX sag (tens of mV,
  §9.5's own caveat) becomes a 5–10 point swing sample to sample — a
  flickering About number, a fill pixel toggling, and the red breathe
  starting/stopping around 3.70 V.

### Changes

| Where | What |
|---|---|
| `dial_power.c` | `PWR_PCT_WINDOW_N 5`; new `pwr_ring_median_mv(n)` (insertion sort, n ≤ 10) over the newest ring samples; `s_pwr_pct_held` — the published percentage is the curve of that median, **held non-increasing while `s_power_src == PWR_BATTERY`**, reset to −1 otherwise (i.e. on plug-in). Median rather than mean so a lone sag sample is discarded outright. Classifier, hysteresis, debounce, slope tiebreak: untouched. |
| `ui_screens_internal.h` | `POWER_GLYPH_BODY_W 20` (was 16) → 16 px usable, ~6 %/px; fill width rounds to nearest (`(usable*pct + 50)/100`); `POWER_GLYPH_FILL_MIN_W 1` (was 2). Comment at the `low` check records why no hysteresis is needed. |
| `dial_state.h` | `power_pct` field comment updated. |
| `docs/SPEC-power-sensing.md` | §11.1 factory-blink sentence corrected; §11.7 added recording this pass. |
| `CHANGELOG.md` | beta.5: About bullet rewritten for the stacked layout; new "Steadier battery reading" bullet. |

**Low-flag hysteresis (approved) was subsumed, not added:** with the hold,
the percentage cannot rise while on battery, so the ≤ 15 % boundary cannot
flap; the flag stays a plain comparison with a comment saying why.

### Verification

- Host test of the median + hold on a synthetic 3.83 V plateau with ±12 mV
  noise and a 50 mV sag every 7th sample: single-sample path swung 26–47 %;
  new path settled 41–43 % and only ever stepped down.
- Simulator: badge re-rendered at the six levels; 10/20/30 % now visibly
  distinct; 78 % no longer reads as 75 %; badge still inside the caption
  above the arc. Temporary scenarios removed; `simulator/main.c` restored
  byte-identical (checked with `cmp`); incidental screenshot churn
  (`settings*`, `update`, `dial-relative`, `dial-update`) reverted.
- `idf.py build` zero warnings; flashed; serial showed clean boot, pad
  connected, `power: plugged (4638 mV)` at ~10 s.
- **Not verified on hardware:** the hold/median on a real discharge (serial
  dies the instant USB is unplugged — the standing §7 limitation). What to
  watch on the dial: About's percentage sits still and only steps down
  while unplugged; the badge visibly loses width between 25 % and 5 %
  before turning red.

## Owner question — board with no battery, USB only

No error path. The pin reads the USB rail (~4.6–4.7 V) → PLUGGED for the
unit's whole life; `power_pct` −1; badge hidden; About "On USB / 4.6x V".
The battery branch (curve, median, hold), the CHARGE flash
(BATTERY→PLUGGED only), and the breathe are unreachable because the board
dies the instant USB is removed. Only visible effect: the same ~10 s
UNKNOWN window at boot ("--", badge hidden) every board has. The firmware
cannot distinguish "USB with a charging cell" from "USB with no cell" —
hardware fact, not a gap. Theoretical worst case if a batteryless charger
IC's output wandered into the 4.18–4.28 V band: a cosmetic flip to the
battery badge, never a fault. No measurement has shown that.
