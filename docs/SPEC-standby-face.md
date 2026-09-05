# Spec: Standby face — Clock or Temperature

Status: **BUILT (`03e6259` consumer, `9dad0b2` setting) and VERIFIED ON HARDWARE 2026-09-05 — owner: "works exactly as designed, °C and °F, night and day, Clock and Temperature." Ships as `0.1.6-beta.2`.** Default Temperature per owner. Target: its own beta after `0.1.6-beta.1` (one feature per beta). Replaces the earlier "screen-timeout Off" idea, which was rejected on 2026-09-05 for the reasons in §1.

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md`, `LICENSING.md` and `somnus-dial-project-summary.md` are **not in this repo** — they live only in the Claude Project. Everything this spec needs is restated here.

## 1. Why this and not "Off"

The owner's want: at night, glance at the dial and see the temperature, not a clock. The obvious feature — a "Never" entry in the Screen timeout menu — was tried once and removed (`dial_state.h`, the `DIAL_SCR_TIMEOUT_CHOICES` block comment), and re-checking it on 2026-09-05 found three reasons to leave it out:

- **It silently disables two other features.** Unattended overnight OTA runs only when `dial_power_level() == DPWR_STANDBY` (`main.c`, the OTA nav block), and the update-available prompt fires on the wake edge out of STANDBY/DIMMED (`main.c`, the `ota_prompt_woke` edge). A dial that never reaches STANDBY never auto-installs and never prompts — the recurring "setting that quietly kills a mechanism" shape.
- **The panel is an AMOLED (SH8601).** A static face at day brightness around the clock is a burn-in risk the standby dimming and clock face currently avoid.
- **The night dimming exists to stop the dial glowing at 3 am**; an always-on face defeats it.

So the timeout stays exactly as it is, the power tiers stay exactly as they are, and only **what is shown while in STANDBY** becomes a choice.

## 2. What exists today (read before touching)

- `main.c` `nav_policy()`: in the READY branch, `return dial_power_level() == DPWR_STANDBY ? SCR_STANDBY : SCR_DIAL;` — this line is the whole feature's consumer. A second site returns `SCR_STANDBY` when a passive screen (Menu / Wi-Fi / About / Update) is open at STANDBY.
- `components/dial_ui/scr_standby.c`: the clock face. Bare r=165 hairline ring, clock, date, zone dots, ambient "Update available" line, battery glyph. Unchanged by this spec.
- `components/dial_ui/scr_dial.c` `apply_palette_and_state()`: computes `bool minimal = night && st->night_face_min;` and renders the night face (number only, `dial_font_num_140`, alternating with the water temperature while heating/cooling — `docs/SPEC-night-face.md` §3) when minimal, the full dial otherwise. It already reads the night palette and the OTA line.
- `components/dial_power/dial_power.c`: the ACTIVE / DIMMED / STANDBY tiers and their backlight duties are keyed on idle time and the Screen timeout pref, not on which screen is showing. STANDBY's duty is the dim floor (night-aware).
- Wake rule: `main.c`'s touch filter and knob step call `dial_power_wake_consumes()` first; the first input while STANDBY/DIMMED flips the tier to ACTIVE and is swallowed before any screen handler sees it. Screen-independent — holds whichever face is showing.
- Settings pattern to copy: **Night face** row (`scr_settings.c`, `scr_night_face.c`, pref `ui/night_face`, `SPEC-night-face.md` §4): a two-value picker screen (Back + two rows + checkmark), a u8 pref with clamp-on-read, getter/setter, `dial_state_commit` on change.

## 3. The feature

One Settings row, **Standby face**, values **Temperature** (default) / **Clock**, placed directly under **Screen timeout** (it describes what the timeout leads to).

While `dial_power_level() == DPWR_STANDBY` and Standby face is **Temperature**, `nav_policy()` returns `SCR_DIAL` instead of `SCR_STANDBY`. That is the entire behavioural change. Consequences, all free:

- **Night, Night face = Number only:** the dial face is minimal — the big setpoint, alternating with the water temperature while heating/cooling, night palette, dimmed to the STANDBY duty. This is the owner's "set temperature / water temperature alternate in night mode".
- **Day (or Night face = Full):** the full dial face — ring, setpoint, pill, power button — dimmed to the STANDBY duty. The owner's "dimmed version of the progress dial in day mode".
- Wake: first input swallowed as today; the tier goes ACTIVE; `nav_policy` re-runs and returns `SCR_DIAL` again — no screen transition at all, the face just brightens. (With Clock, today's clock→dial transition is unchanged.)
- Unattended OTA, the update prompt, the auto-update window, night dimming, haptic trim: untouched — they read the power tier.
- The passive-screen site (Menu / Wi-Fi / About / Update at STANDBY) returns the same choice: `SCR_DIAL` under Temperature, `SCR_STANDBY` under Clock. Factor a `standby_screen(st)` helper so both sites read one rule.

**Default Temperature, deliberately — owner's ruling 2026-09-05: "why would I want yet another clock on my nightstand?"** This knowingly departs from the "an OTA changes nothing" rule: the clock face is Orion's design (`design-spec.md` §5, "a clock, not a dashboard"), inherited by the port, and the only existing dials are the owner's. Clock stays as the other value because `scr_standby.c` already exists and a future user who wants one is one tap away; deleting the clock face outright is a possible later cleanup, not this beta.

## 4. The setting

Pref `ui/sb_face` (u8, 0 = clock, 1 = temperature), clamp-on-read to `{0,1}` → **1 (Temperature)**, getter/setter copied from `night_face`, applied via `dial_state_commit` so the next `nav_policy` run sees it. Picker `scr_standby_face.c` in `scr_night_face.c`'s shape, Temperature listed first. Row label "Standby face", value "Temperature" / "Clock". Always visible (unlike Night face, it does not depend on night_on).

**The two questions.** Changeable from the state it needs changing in: yes — Settings, at steady state and inside the connect loop. Read by anything: `nav_policy()`'s two STANDBY sites, and commit 1 (§6) proves the consumer before the row exists.

## 4b. The "Night clock" brightness row (found by commit 1, `docs/REPORT-standby-face-c1.md`)

The night STANDBY duty is `bri_night_clock_pct` — the Settings → Brightness row labelled **Night clock**, whose picker previews the STANDBY duty and allows **0 % = off**. With Standby face = Temperature that pref now sets how bright the *temperature* is at night. Behaviour is right; the name is not. Commit 2 renames the row to **Night standby** (label and preview caption only — pref key, range and default unchanged, so an OTA changes nothing). 0 % stays allowed: a dark room is a legitimate choice, and it means the standby face is off whichever face is chosen — say so in the row's own comment. Also fix the two stale comments the report lists (`ui_router.h` SCR_STANDBY entry, `scr_updating.c` header) that still describe STANDBY as "the clock is showing".

## 5. Things to check on hardware, not assume

1. `scr_dial.c` under STANDBY duty: the night-face alternation timer keeps running while the tier is STANDBY (it is keyed on heating/cooling, not on the tier) — confirm the swap is visible at the dim floor and the water word is legible.
2. The ambient "Update available" line: `scr_dial.c` has its own (`s_ota_lbl`, moved to y=326 in the layout pass) — confirm it shows on the standby-dimmed dial the way the clock's does.
3. The battery glyph on the dial face at STANDBY duty.
4. **AMOLED:** the day-mode standby dial is a static image at the dim floor. Standby duty is already the floor the clock runs at, and the ring/number change with the setpoint, so this is judged acceptable — but note it, and if a Tier-D style "drift a few px every N minutes" ever seems warranted, it goes in a later spec, not this one.
5. Wake edge for the update prompt: `ota_prompt_woke` compares consecutive tiers, not screens — confirm the sheet still appears on the first wake after an update is found, with Temperature selected.

## 6. Commits

1. **The consumer, no setting.** `standby_screen()` helper in `main.c`, both call sites use it, hard-wired to Temperature for this commit only. Build, flash, let it time out by day and (Tokyo-timezone trick) at night; wake it; confirm §5 items 1–3 and 5.
2. **The setting.** Pref, getter/setter, clamp, `SCR_STANDBY_FACE` + `scr_standby_face.c`, the Settings row under Screen timeout, `standby_screen()` reads the pref (default Temperature), the Night clock → Night standby rename (§4b) and the two comment fixes. Simulator scenario `settings` re-rendered (new row) plus `standby-temperature` if the harness can force the tier; otherwise hardware only.
3. **Release** as the next beta after hardware verification of commit 2: `CHANGELOG.md` section, `PROJECT_VER` bump, tag, push to `somnus` only.

## 7. Not in this spec

- Any change to Screen timeout's choices. "Off"/"Never" stays out for §1's reasons.
- A new standby layout. The dial face is the face; the night face setting already decides what it looks like at night.
- Burn-in mitigation beyond the existing dimming (§5.4).
