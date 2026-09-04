# Spec: Night face — number only

Status: **BUILT (commits `a14462c`, `27a82e5`, `5c42aad`, `e31761a`) and VERIFIED ON HARDWARE 2026-09-04 — all eleven §8 checks passed on the bench dial, wire-flashed over a restored beta.2 NVS. Ships as `0.1.5-beta.3`.** Originally a proposal written 2026-09-03. Owner chose the shape the same day: the dial face, while the night window is active, shows the **setpoint** and little else, in a **new ~140 px digits font**, alternating with the **water temperature** while the bed is heating or cooling (§3a). Standby (the clock) is untouched. Revision 3 (WATER word moves above the numeral; two review fixes in §6).

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md`, `LICENSING.md` and `somnus-dial-project-summary.md` are **not in this repo** — they live only in the Claude Project. Everything this spec needs is restated here.

## 1. Why

The dial is used in the dark, often without glasses. The 88 px numeral competes with a unit, a caption, a status pill, a side label, an arc and a handle. At night the palette dims all of it together, which makes the number *harder* to pick out, not easier. This is the display-side half of the accessibility problem that `SPEC-voice.md` §1 concluded should be solved on the screen and the haptics before anyone reaches for audio.

## 2. What exists today (read before touching)

`components/dial_ui/scr_dial.c`:

- The numeral is `s_temp_lbl` in `s_num_box` (210×92, centered at y=150), font **`dial_font_num_88`** — Montserrat SemiBold, 4 bpp, glyphs `-` `.` `0-9` `:` plus a spliced `+` (header comment in `dial_font_num_88.c`; the splice exists only because the standby clock shares the font and had to stay pixel-identical). `render_numeral()` writes `"68"`, `"20.5"`, `"+3"`, `"-3"` or `"0"` depending on `s_units_c` / `s_rel`.
- `apply_palette_and_state()` is the single place every render decision is made; it already reads `bool night = dial_palette_is_night()` and uses it for the chevron period, the staleness-dot opacity and the OTA line. **Every visibility and font change in this spec goes in that function**, so a night transition (the worker's `dial_state_commit` on the palette flip) and a settings change both land through the existing path with no new hook.
- Off-side rule (`NUM_STANDBY_OPA`, the power-button breathe) and the knob/handle guards are upstream of rendering and are not touched.
- Night itself is `dial_night_active()` in `main.c` → `dial_palette_set_night()`; `dial_palette_is_night()` is the read side. This spec adds no second definition of night.

## 3. The night face

While `dial_palette_is_night()` **and** the new setting is Number only (§4):

**Shown:** the setpoint numeral in `dial_font_num_140`, centered on the face (`LV_ALIGN_CENTER, 0, 0`, box enlarged to 340×160 so `"20.5"` and `"-15"` fit; digits share advance width so there is no reflow jitter, same as today). The power button, unchanged — it is the only way to switch a side on/off and the off-side breathe must still have something to aim at. The staleness dot at its night opacity — it is the only "this number may be stale" signal. The arc ring, as the palette already draws it at night.

**Hidden** (`LV_OBJ_FLAG_HIDDEN`, toggled per render exactly the way `s_pill` already is): the unit / `LEVEL` label, the `WATER` caption, the status pill (and `chevron_stop()` — a pulse on a hidden glyph is wasted work), the side label and its identity underline, the AWAY badge, page dots if present. The OTA line is already hidden at night.

**The drag handle stays.** It is the only touch target for the temperature, it is already the palette's business how bright it is, and removing a control because the *labels* went away is the "control that looks live" pattern in reverse. It sits on the arc at the edge and does not compete with the number.

**Off side:** the numeral dims to `NUM_STANDBY_OPA` as today. Nothing else changes; the breathe on the power button is now the brightest thing on the face by construction, which is what §14.6 of the hardware log wanted anyway.

**Relative mode:** `"+3"` / `"-3"` / `"0"` with no `LEVEL` label. The sign disambiguates against °F (which never reads below 54) and °C (which always carries a decimal). Acceptable; if it ever confuses, the fix is a small `LEVEL` under the number, not the label back at its old place.

### 3a. Water temperature — alternation, not a second number (owner, 2026-09-03)

The water reading is shown **in the same place and the same 140 px font**, by alternating with the setpoint, so there is never more than one number on the face.

**When.** Only while `kind` (the value `apply_palette_and_state()` already derives for the pill: `ZK_HEATING` / `ZK_COOLING` / holding, ±0.5 °C deadband) is heating or cooling, and the zone is on. When holding, the setpoint sits still. A still number means "you are there"; a flipping number means "not yet" — the alternation carries the state the hidden pill used to carry.

**Cadence.** 2 s setpoint, 2 s water, one `lv_timer` owned by `scr_dial.c`, created only while alternation is active and deleted otherwise (and in `destroy()`). The setpoint-change zoom bump (`anim_zoom_bump`) must not fire on an alternation tick — only on a real setpoint change, as today. If the bench says the setpoint should hold longer than the water reading (it is the value the user can act on), change the two constants; do not add a setting for it.

**The knob wins.** While `s_dragging`, and for 3 s after the last detent or handle release, the face is locked to the setpoint and the timer is paused. A number that flips to water mid-turn reads the wrong value at the moment the user is acting on it. Record the last-interaction tick in `on_knob` and the handle's RELEASED path; the timer callback checks it.

**Telling them apart.** Not a second font weight — another 140 px face is ~250 KB for a cue that is the least legible one in the dark. The setpoint renders in `ink_primary` as today. The water reading renders in `accent` — the heating/cooling hue already computed for the pill — with the word **`WATER`** in `lv_font_montserrat_28` centered **above** it, in the slot the day face's `WATER` caption occupies (y = 98), shown only on the water phase. *Revision 3 correction:* the first build put the word beneath the numeral, where it lands on the power button (72 px square centered at y = 280; the 160 px numeral box already ends at y = 260). Above is the only free slot at night — the side label and caption that lived there are hidden. Color plus a label; the label is the guarantee for anyone who cannot see hue at night.

**Formatting.** Water uses the same unit rules as the setpoint: whole °F via `dial_c_to_f()`, one decimal in °C. In relative mode water is shown as a **level** via the same `dial_rel_from_dc` path the setpoint uses (rounded), so the two numbers are comparable; a raw °C next to a level is not. `actual_c < 0` (no reading) → no alternation.

**Off side / no reading / day:** no timer, no water phase. The day face is unchanged — the `WATER` caption stays where it is.

**Leaving night:** the same function runs on the palette flip and restores every widget and the 88 px font. No state is kept between the two layouts beyond the widgets' own flags.

## 4. The setting (consumer before control)

One Settings row, **Night face**, values **Number only** (default) / **Full**, inserted directly under **Night mode**. Picker is `scr_night_mode.c`'s shape: Back, two rows, checkmark. Pref `ui/night_face` (u8, 0 = full, 1 = number only), clamp-on-read to `{0,1}` → 1, getter/setter copied from `night_on`, no changed-hook (the face re-renders on the next `on_state`, and the setter may call `dial_state_commit` to make it immediate, the way the timeout row does — check which of the two the tree uses and copy it).

**Hidden while `night_on` is false**, same as the Brightness night rows: a night-face row on a dial that never enters night is `sched_follow`'s grave.

**Default Number only, deliberately,** which is a departure from the night window's "an OTA changes nothing" rule. Reason: this is presentation, not behavior — no write path, no timing, no brightness changes — and it is the whole point of the beta. The Full row is the one-tap way back. Stated here so nobody reads the default as an accident.

**The two questions.** Changeable from the state it needs changing in: yes — Settings, at steady state and inside the connect loop. Read by anything: `apply_palette_and_state()`, the single consumer, and commit 1 (§6) proves the consumer works before the row exists.

## 5. The font

`dial_font_num_140.c`, generated in **one** run — no splice, this font has no pixel-identity obligation:

```
lv_font_conv --font Montserrat-SemiBold.ttf --size 140 --bpp 4 \
  --range 0x2B,0x2D,0x2E,0x30-0x39 --no-compress --format lvgl \
  --lv-font-name dial_font_num_140 -o dial_font_num_140.c
```

Thirteen glyphs: `+ - . 0-9`. No colon — the clock does not use this font. Expect roughly 2.5× the 88 px font's bitmap (~250 KB of flash, in a 1.55 MB app with 4 MB slots). `LV_FONT_DECLARE` in `scr_dial.c`, add to `dial_ui`'s CMakeLists next to the 88. Width check at 140 px, Montserrat SemiBold digits ≈ 0.6 em advance: `"20.5"` ≈ 290 px, `"108"` ≈ 250 px, `"-15"` ≈ 230 px — all inside 340. If the generated advance is wider than that estimate, drop to 128 px rather than clip.

## 6. Commits

1. **Font + night rendering, no setting.** Add the font; in `apply_palette_and_state()` compute `bool minimal = night;` and apply §3. Build, flash, force night by the Tokyo-timezone trick, look at it, turn the knob, switch the side off, come back to day. This commit is the feature; it can ship alone.
2. **The setting.** Pref, getter/setter, `SCR_NIGHT_FACE`, the Settings row with the hidden-when-night-off rule, and `minimal = night && st->night_face_min`.
3. **Review fixes, one commit** (found reading commits 1–3, 2026-09-03): `dial_dots_layout()` never clears `HIDDEN` on `dot_menu`, so the menu dot hidden at night is never restored at dawn — add the `else` clear. A drag that starts during a water phase shows the live drag value in the water accent until the next tick — on the handle's `PRESSED`, if the water phase is showing, switch to the setpoint phase immediately, the way `on_knob` already does. And the `WATER` word moves above the numeral (§3a).
4. **Release** as `0.1.5-beta.3`: `PROJECT_VER`, CHANGELOG section `## 0.1.5-beta.3`, tag `somnus-v0.1.5-beta.3`, push to `somnus` only. The bench dial (Beta builds on) picks it up by the tags path; that is the third exercise of that path.

## 7. Not in scope

- The standby clock. It stays a clock (design-spec §5). A setpoint that rests on screen all night was considered and declined 2026-09-03.
- A big-number overlay on knob turn during the day. Considered; the night face covers the real case. Revisit if daytime legibility comes up.
- Two numbers on the night face at once. The water reading alternates with the setpoint (§3a) rather than sitting beside it.
- Haptic level count (`SPEC-voice.md` §1). Separate, still recommended, not this beta.

## 8. Verification on hardware

Force night (timezone to a zone where it is night, then back), and for each of: °F absolute, °C absolute, relative +3, relative −12, relative 0 — read the face from across a dark room without glasses. With the bed heating or cooling, watch the alternation: 2 s / 2 s, WATER word only on the water phase, accent color on water; set a temperature the bed already holds and confirm the number stops flipping. Turn the knob during a water phase and confirm it snaps to the setpoint and stays for ~3 s after. Turn the knob through a range stop. Switch the side off and on. Let the timeout put it on standby and wake it. Set Night face to Full and confirm the old face returns, then Night mode to Off and confirm the Night face row disappears. Record in the report which of those were done and which were not.
