# SPEC — Power sensing (USB vs battery)

**Status: measured, specified, NOT implemented. Not v1 scope. Sep 3–4: §9.5 — the pin reads the cell on battery; owner's decision is a plugged-in / on-battery icon only. §10 is the design, target beta.4.** Recorded Sep 2 2026 so the measurement never has to be redone. **See §8 — upstream tried this independently on identical hardware and declined to ship it.**

**Question that started it:** the Waveshare factory demo firmware showed a green USB icon when plugged in. Can this firmware do the same?

**Answer: yes.** `GPIO1` sees a genuinely different voltage depending on power source, measured on real hardware.

**Scope of the intended feature:** a plugged-in versus on-battery indicator. **Not** a battery percentage — see §3.

---

## 1. The pin

- **`GPIO1` / `ADC1_CH0`**, carrying the net **`BATT_ADC`**
- A **10K/10K divider** (R62/R63, schematic sheet 4 `4_OTHER.png`) off the board's `5V` net, so the ADC reads **half** the rail
- **Unused anywhere in this firmware.** No conflict with the knob encoder (GPIO8/7), display QSPI (GPIO13–18, 21), touch (GPIO9–12), backlight (GPIO47), the companion-MCU UART (GPIO48/38), PDM mic (GPIO45/46), SDMMC (GPIO2–6, 42) or I2S (GPIO39–41)

## 2. Measured on hardware, Sep 2 2026

| State | Rail voltage |
|---|---|
| USB connected | **~4.73V** average |
| USB unplugged (battery) | **~4.08V** average |
| Step | **~650mV (~14%)** |

Recovery is within **one 3-second sample** of replugging. Eight consecutive unplugged samples over 24 seconds sat in a **4032–4114mV** band.

Method: a temporary ADC read on `power_task`'s existing 100ms tick, sampling every 3s into a RAM ring buffer, with a separate low-priority task replaying the window to the log. The instrumentation was removed afterwards.

## 3. What the numbers mean

**The unplugged rail is regulated, not a raw cell.** 82mV of spread across 24 seconds is a boost converter holding a node — a battery discharging under load wanders considerably more. So the topology is almost certainly a boost from the cell onto this `5V` net, diode-OR'd (or ideal-diode switched) with USB, and USB's higher voltage simply dominates when present.

**Consequences:**

- **USB-vs-battery detection: viable.** A rail threshold near **4.4V** (≈2.2V at the ADC, after the divider) separates the two states with roughly 300mV of margin either side.
- **State-of-charge: not established, and unlikely in this form.** A regulated boost output stays flat across most of a cell's discharge and then falls off a cliff near its dropout. So this pin plausibly gives a reliable *on-battery flag* and a *late-stage low-battery warning*, not a smooth 0–100% curve. Settling that needs a logged reading across a full real discharge — hours, not seconds. **Open question, not a conclusion.** The intended feature does not depend on resolving it.

## 4. Design constraint worth respecting

**The USB baseline is 4.73V, not 5.0V** — about 270mV lost in the cable and connector. That leaves only ~330mV between a healthy USB rail and the 4.4V threshold.

**A bad cable could therefore read as "on battery."** Any implementation needs **hysteresis** and several consecutive samples before flipping state. A dial that flickers between plugged-in and on-battery because of a marginal cable is worse than one that never tries.

The same reading doubles as **brownout early warning** — a supply sagging toward the buck's headroom and one sagging toward the detection threshold are the same measurement, and on a device that would otherwise reboot silently at 3am that is arguably the more valuable of the two uses.

## 5. The schematic is incomplete for this hardware

**Waveshare's published schematic shows no battery connector and no charging IC at all.** All five sheets were read directly (as images — they are PNGs, which is why a text-only pass over them is unreliable). The only power path drawn is USB-C → TLV62569 buck → 3.3V.

The owner's board is the SKU that **ships with the battery installed** (about $2 more; most ship without and you fit your own), and it demonstrably runs with USB unplugged. So the battery variant's socket, charger and boost are simply absent from the public document.

**Treat the published schematic as the base-SKU document.** Where it and the board disagree, the board wins — as it did here.

## 6. If this gets built

Small: no new component needed. One-time `adc_oneshot_new_unit` + `adc_oneshot_config_channel` on `ADC1_CH0`, sampled from `dial_power.c`'s existing periodic tick, with calibration via `adc_cali` and a linear fallback.

Frame it as **power source + supply health**, not as a battery gauge, until §3's open question is settled.

Any icon must follow `design-spec.md` §2: RGB565-quantized values, a night variant with blue ≤ `0x18`, and a shape channel so the state does not depend on hue. A green USB icon copied from the factory demo satisfies none of those.

## 7. Technique notes from the experiment

Two things learned that generalize beyond this:

- **`esp_console`'s native-USB write returns success even with no physical link** — it neither blocks nor fails, it silently discards. So any logging that must survive a USB disconnect has to buffer in RAM and **unconditionally replay a window on reconnect**, rather than printing each sample once. A print-once design silently loses exactly the samples taken while disconnected.
- **`idf.py monitor` resets the chip on attach** (`rst:0x15 USB_UART_CHIP_RESET`), which destroys any RAM buffer you were trying to read. Reattach with `screen /dev/cu.usbmodem<n> 115200` instead when the data lives in RAM.

## 8. Upstream tried this and declined — Sep 2 2026

These findings were **sent to Chris Meyer (upstream, `chris023/orion-waveshare-rotary-dial`)** as part of a first introduction, after he was seen working on the same charging-icon feature independently.

**He declined to implement it, judging the reading unreliable.**

Weight this properly: **Chris has the same battery-equipped SKU** (most of these boards ship without a cell; both his and this one have it). So his conclusion is not an inference from a base-SKU board without a battery to measure — it is an independent attempt on comparable hardware, by someone with more time in this codebase, ending in a decision not to ship. That is meaningful evidence that the feature is harder than §2's clean 650mV step makes it look.

**Not yet known, and worth asking him:**

- Whether he measured the same ~4.73V / ~4.08V split, or saw something worse.
- **Whether he tried it with hysteresis and consecutive-sample debounce, or hit flapping on a bare threshold comparison and stopped.** These are different claims. §4 identifies the exact failure mode (a marginal cable eating the ~330mV margin) and the standard fix; a conclusion drawn from a naive comparison would not transfer to a debounced implementation.
- What his observed failure mode actually was — flapping, a wrong steady state, or something else.

**Standing position:** the measurements in §2 stay valid — they were taken on this hardware and are reproducible. Chris's decline is a strong argument for keeping this out of v1, which this document already does, and for treating §4's hysteresis requirement as mandatory rather than advisory if it is ever built. It is not a reason to discard the measurement.

## 9. Battery charging management — researched Sep 3 2026

**Question:** what is the "battery recharge management module" Waveshare lists, and can the firmware see anything from it?

**Answer: it is undocumented, and the ESP32-S3 has no signal from it.** Nothing to build on, one cheap experiment worth running, one hazard worth knowing.

### 9.1 What the public documents say

- **Wiki and product page:** "battery recharge management module" and "PH1.25 lithium battery header" as onboard resources; the battery SKU ships a **3.7 V 102035 800 mAh** cell assembled. No charger part number, no charge current, no status LED, no PMIC. The only power ICs named anywhere are the CH445P (the USB orientation switch) and, on the schematic, the TLV62569 3V3 buck.
- **The schematic** (five sheets, re-read Sep 3; the five PNGs are now in `reference/waveshare-schematic/`, copied from Sandjab/Waveshare-Knob on GitHub because Waveshare's own download was unreachable): the same five sheets §5 describes. Sheet 1 has USB-C, the TLV62569 buck from the `5V` net to 3V3, the backlight FET and the two encoder switches. Sheet 4 has the `BATT_ADC` divider (R62/R63, 10K/10K, off `5V`), the PDM mic (MSM261D4030H1CPM), the TF socket. **No battery socket, no charger, no boost, no power button, no charge-status net on any sheet.** Sheet 2's ESP32-S3 net list confirms it: the S3's only power-related pin is `GPIO1 = BATT_ADC`. There is no `CHRG`, `PGOOD`, `PWR_KEY` or PMIC I2C. The battery section of the board is simply not in the published schematic, exactly as §5 concluded.

### 9.2 What that means for firmware

- **Charging state is unobservable.** Whether the cell is charging, done, or absent cannot be read. Only the `5V` net via GPIO1, which §2 already measured.
- **Cell voltage may or may not be observable — §3's "regulated rail" inference is weaker than it reads.** A freshly charged Li-ion cell sits at ~4.1 V and moves only millivolts over 24 s at the dial's load, so the 4032–4114 mV band is **equally consistent with the raw cell** switched straight onto the `5V` net by the charger's load-sharing path. That topology is plausible: the TLV62569 runs from 2.5 V, so the board needs no boost at all to make 3V3 from a cell. If that is what it is, the pin is a real (if coarse) fuel gauge, which reverses §3's pessimism.
- **The experiment that settles it, cheap:** log GPIO1 every minute on battery from full until the board dies, using the §7 technique (RAM ring buffer, replay on reconnect, or write to the TF card). A trace that falls steadily from ~4.1 V toward ~3.5 V is the cell; one that holds flat and then cliffs is a boost. Hours of unattended bench time, no code in the product.
- **Identifying the charger IC needs the case opened.** Its markings are the only source. Worth ten minutes if the discharge trace is ambiguous, otherwise not.

### 9.3 The hazard: a power-bank-class IC

A board with a hard power button, a `5V` net, a charger and no PMIC on the S3 side is the signature of a single "power bank" SoC (IP5306 class: charger + boost + button in one part). If that is what Waveshare fitted, it **auto-powers-off when load current stays below a threshold (tens of mA) for tens of seconds.** The dial never trips it today because the LCD and radio draw far more than that continuously. **Any future "screen off on battery" or deep-sleep feature must be tested on battery for at least a minute before anyone believes it works** — the symptom would be the dial powering itself off, not sleeping. Unverified; recorded so the first person to try deep sleep does not spend an evening on it.

### 9.4 Nothing to build (superseded by 9.5)

Written before the factory-firmware check. No firmware feature was expected from this. The §6 plan (power source + supply health from GPIO1, with §4's hysteresis) stands unchanged, and 9.2's discharge log is the only step that could upgrade it to a gauge.

### 9.5 Resolved Sep 3 2026 — the pin reads the cell

Factory firmware restored from the §6 backup and its status screen read. It shows the same pin in millivolts: **~4614 mV on USB, ~3800–3900 mV on battery, declining over time**, with a segmented battery icon that drops segments as the number falls.

3.8–3.9 V is a lithium cell somewhere around half charge. No regulator holds a rail there, and the Sep 2 reading of ~4.08 V was a cell that had just come off USB. So **§3's "regulated boost" inference is withdrawn: on battery, the `5V` net is the cell**, reached through the charger's load-sharing path, and the TLV62569 makes 3V3 straight from it. A real, if coarse, state-of-charge reading is available from GPIO1 with nothing but the existing divider.

What follows for §6 if it is ever built:

- **Two regimes, one pin.** Above ~4.4 V the net is the USB rail and says nothing about the cell; below it, the net is the cell. So the firmware can show *charging* (plugged in) or a *percentage* (unplugged), never charge progress while plugged in. That is a hardware fact, not a software gap.
- **SoC from voltage, with the usual caveats.** Map roughly 4.15 V → full, 3.7 V → half, 3.4 V → empty for this chemistry, and expect sag under load (backlight full, radio transmitting) to read low by tens of mV. Average over many samples; §4's hysteresis and consecutive-sample rules still apply, and are the likely reason upstream found a naive version "unreliable" (§8).
- **The low-battery warning is now the valuable feature.** A dial that reads 3.5 V unplugged at bedtime is going to die before morning. That is a one-threshold check on a pin the firmware already owns, and it is worth more than a percentage.
- **Factory firmware's plugged-in reading was ~4614 mV against this project's ~4730 mV** — different cable, calibration, or sampling; the ~300 mV gap to the 4.4 V threshold in §4 is the number to respect, whichever rail figure is used.

**Factory firmware's own thresholds, observed on a full discharge Sep 3–4:** at **~3450 mV** its battery icon turns red and blinks; at **~3200 mV** a full-screen LOW BATT takes over and hides the reading; the board was still running below that, so the actual cutoff is somewhere under 3200 mV and was not seen. Use 3450 mV for the warning — measured, not guessed — and treat 3200 mV as the conservative floor.

**Design rule for our warning, from watching theirs:** never take over the face. A bedside dial whose temperature disappears behind a battery screen has stopped doing its job at the moment it is needed; the warning is a small persistent mark on the face (and, at night, on the night face), not a screen.

Still unknown: the charger IC and whether it is power-bank class (§9.3). Not needed for any of the above.

**Owner's decision, revised Sep 4 2026:** a **plugged-in / on-battery icon only.** No percentage, no low-battery warning — the user finds out when it shuts off. Queued behind `1.0.0`; not v1.

**Plug-in detection has to survive a depleted cell.** Observed Sep 4: plugging in at ~3175 mV, the pin read **~4350 mV and climbing** — on USB the `5V` net is the charger's system node riding just above the cell, not a fixed rail, and it only reaches the ~4.6–4.7 V seen on a charged board once the cell is well up. So §4's flat 4.4 V threshold would call a just-plugged-in dead dial "on battery" for as long as that climb takes, and hysteresis cannot fix a reading that is slowly and correctly low. Detect with either a **~4.25 V threshold** (an unplugged cell never exceeds ~4.2 V; the plugged-in floor seen so far is 4.35 V — a narrow gap) or, better, **the slope**: rising means charging, falling means on battery, whatever the level. Time-to-4.4 V and time-to-4.6 V from plug-in were not recorded; worth one measurement before this is built.

## 10. The indicator — BUILT (`e7ff49b`, `8245021`, `6f76af1`) and VERIFIED ON HARDWARE Sep 4 2026; shipped as `0.1.5-beta.4`

*Bench, Sep 4: boot classifies plugged at ~10 s (`plugged (4772 mV)`); unplug → battery glyph within ~5 s, About `Battery 4.10 V`; plug in → lightning glyph 3 s then nothing, About `USB 4.65 V`; cable wiggle did not flap; night face shows the glyph dim and clear of the WATER word; standby clock carries it. The glyphs live on the dial face and standby only — a transition seen while About is open draws nothing on return, by design. The ESP_LOGD sample line is compiled out (`CONFIG_LOG_MAXIMUM_LEVEL=3`); About is the bench instrument. §10.2's open measurement (time-to-4280 mV on a depleted cell) is still unmeasured; the slope tiebreak stands as designed.*

**Scope, per §9.5's revised decision:** two states, **plugged in** or **on battery**. No percentage, no low warning, no setting. One new reading, one new field, two glyphs, one diagnostic row.

**Sequencing:** not in the same beta as the night face. One feature per beta keeps a regression attributable; the night face is beta.3, this is beta.4.

### 10.1 What exists (read before touching)

- `components/dial_power/dial_power.c` — `power_task`, a 100 ms tick that already owns the brightness tiers (`DPWR_ACTIVE / DIMMED / STANDBY`) and the night duty. The ADC read goes here; nothing else needs a new task.
- `GPIO1 / ADC1_CH0`, 10K/10K divider, unused (§1). The factory `01_ADC_Test` demo is the reference: `adc_oneshot` unit 1, `ADC_ATTEN_DB_12`, 12-bit, `adc_cali_create_scheme_curve_fitting`, `adc_cali_raw_to_voltage`, ×2 for the divider. Copy that, not a hand-rolled `raw * 3.3 / 4096`.
- `app_state_t` in `dial_state.h` is the only way screens learn anything (`on_state` after `dial_state_commit`). `clock_valid` is the model: a worker-owned fact published into the snapshot.
- Screens: `scr_dial.c`'s `apply_palette_and_state()` (every render decision, including the night face's `minimal`), `scr_standby.c`'s `apply_palette_and_state()`, `scr_about.c`'s rows. The staleness dot sits at `(180, 26)`, top center.
- LVGL's built-in symbol set is already linked (the pill uses `LV_SYMBOL_UP/DOWN/MINUS`); `LV_SYMBOL_CHARGE` (0xF0E7) and `LV_SYMBOL_BATTERY_EMPTY` (0xF244) are in the same set. **Confirm they render in `lv_font_montserrat_16` on the bench** — the compiled fonts have already surprised this project once (§18.6 of the hardware log: no dash glyph).

### 10.2 Detection — what the pin means (§2, §9.5)

| Situation | Pin (after ×2) |
|---|---|
| USB, cell charged | ~4.6–4.7 V |
| USB, cell depleted, just plugged in | **~4.35 V and rising** |
| Battery, cell full | ~4.1–4.2 V |
| Battery, half | ~3.8–3.9 V |
| Battery, near end | 3.2 V and below |

Two regimes with a gap of only ~150 mV at the worst case (4.2 vs 4.35), and the plugged-in floor was seen once, not characterized. So:

- **Threshold with hysteresis:** enter PLUGGED at **≥ 4.28 V**, leave PLUGGED at **≤ 4.18 V**. 100 mV band, centered in the gap.
- **Debounce:** the state changes only after **5 consecutive** 1-second samples agree. A marginal cable (§4) sagging across the band flaps at most once per five seconds and, with the slope term below, not at all.
- **Slope as a tiebreak inside the band:** keep the last 10 samples; if the mean of the newest 5 exceeds the mean of the oldest 5 by **> 20 mV**, that is charging regardless of level; if it is lower by > 20 mV, that is discharging. Inside 4.18–4.28 V the slope decides; outside it the threshold does. Rising while unplugged is physically impossible, so the slope never lies in the direction that matters.
- **Boot:** state is UNKNOWN until five samples exist (~5 s). UNKNOWN renders as nothing.
- Sample once per second (every tenth tick of `power_task`), not every 100 ms — the ADC is noisy and the number moves in minutes.

**Open measurement, before building:** time from plug-in on a depleted cell until the pin passes 4.28 V. If that is more than a few seconds the threshold is wrong and the slope term becomes primary. One run with `screen` open and the log line below is enough.

### 10.3 State

One field in `app_state_t`, published by `power_task` through `dial_state_commit` **only on a change** (a commit per second would wake every screen for nothing):

```c
typedef enum { PWR_UNKNOWN = 0, PWR_BATTERY, PWR_PLUGGED } dial_power_src_t;
dial_power_src_t power_src;   // §10; UNKNOWN until 5 samples exist
uint16_t         power_mv;    // last calibrated reading ×2; diagnostic only, About row
```

`power_mv` updates every sample but does **not** by itself trigger a commit; it rides along with whatever commit happens next. About reads it on `on_state`, so the row refreshes whenever anything else changes, and About can run its own 1 s `lv_timer` calling `dial_state_get()` while open. Log one line per transition: `power: plugged (4612 mV)` / `power: battery (4085 mV)`. That line is the bench instrument.

### 10.4 What the user sees

**Nothing, most of the time.** A bedside dial plugged in at its normal spot shows no indicator; an always-on glyph is noise on an instrument face (design-spec §1's "one big number is a fact"). Two exceptions:

- **On battery:** a small **`LV_SYMBOL_BATTERY_EMPTY`** glyph, `lv_font_montserrat_16`, `ink_secondary`, at **`(180, 46)`** — under the staleness dot's slot, top center, inside the arc. Present on the dial face (both layouts — it is one of the things the night face keeps, at night opacity `LV_OPA_40`, same as the stale dot), and on the standby clock at the clock's night ink. It means exactly one thing: *you are running down.*
- **Plug-in confirmation:** on the transition BATTERY → PLUGGED, **`LV_SYMBOL_CHARGE`** in the same slot, `accent`-free (`ink_secondary`), for **3 s**, then gone. The factory firmware's green USB icon is what the owner remembered; this is its short-lived cousin. No confirmation on UNPLUG — the battery glyph appearing is the confirmation.

Two glyphs, one slot, never both. Neither has a setting.

**About screen:** a **Power** row after Serial: `USB  4.61 V` or `Battery  3.92 V` or `--` while UNKNOWN. This is where the number lives for anyone who wants it, and the only place. It is what the owner used to characterize the factory firmware, so the Bedknob should be at least that inspectable.

### 10.5 The two questions

*Changeable from the state the device is in?* There is nothing to change: no setting. *Read by anything?* Three consumers (dial face, standby, About), all reading `power_src` through `on_state`. Commit order, consumer before control: **commit 1** ADC read + `power_src` + `power_mv` + the log line + the About row (the diagnostic proves the detector on the bench with no face art at risk); **commit 2** the two glyphs on the dial face and standby.

### 10.6 Not in scope

- Percentage or curve (§9.5, decided).
- Low-battery warning (§9.5, decided Sep 4; the battery glyph is the whole story).
- Any screen that takes over the face (§9.5's design rule).
- Deep sleep / screen-off on battery — §9.3's power-bank hazard applies the day that is attempted.
- Brownout early warning (§4's second use) — could come later from the same sample; not this.

### 10.7 Bench

With `screen` open: plug/unplug five times and watch the transition lines and the glyph; count seconds from plug-in to the CHARGE glyph on a charged cell and on a ~3.5 V cell. Wiggle the cable. Leave it on battery through a night window and confirm the glyph is visible but dim on the night face and on the standby clock. Read the About row against the factory firmware's number if you ever swap back.

## 11. Revised Sep 4 2026 (later) — percentage + low-battery styling + About redesign, adopted from upstream PR #4

**Supersedes §9.5's "no percentage, no low warning, no setting" and §10.6's matching exclusions.** Those were the right call with only this project's own measurements in hand. Overtaken by finding [chris023/orion-waveshare-rotary-dial#4](https://github.com/chris023/orion-waveshare-rotary-dial/pull/4) (author: borski, unmerged as of this writing) — an independent battery implementation on the same board, same pin, same divider, whose measured thresholds land within noise of this project's own (§9.5, §10.2). Full PR content fetched read-only and kept for reference at `reference/upstream-pr4-battery-diag/` (not tracked, not built against directly — see that folder's own README). Owner's decision: adopt the percentage curve and the low-battery visual treatment; do not adopt their detector, their separate diagnostics screen, or their new dial_battery component wholesale.

### 11.1 What carries over from upstream, and what doesn't

**Keep this port's own detector as-is (`dial_power.c`, §10.2).** It is already more robust than PR #4's: hysteresis (4280/4180 mV vs. their 4400/4300), a 5-sample debounce, and a slope tiebreak inside the band that theirs lacks entirely. Both were arrived at independently and land within ~100 mV of each other, which is good cross-confirmation, but this port's version has been bench-verified on this exact board (§10, beta.4) and theirs hasn't been verified against this fork's hardware at all. **Do not replace `dial_power.c`'s classifier with `dial_battery.c`'s.**

**Adopt upstream's percentage curve.** Their `BATT_CURVE` table is LiPo-chemistry data (the 3.9–3.75 V knee holding most of the usable capacity), not detector logic, and it's expressed in the same units this port already uses — rail mV, i.e. the calibrated pin reading ×2, matching `power_mv`'s existing convention exactly. Use it as-is:

```c
// Ascending by mV. Full is a resting LiPo off the charger; empty is where
// this board's TLV62569 buck gives up, not where the cell is flat (§9.5).
// Source: chris023/orion-waveshare-rotary-dial PR #4, dial_battery.c —
// reused verbatim as curve data, not as a component.
static const struct { int mv, pct; } BATT_CURVE[] = {
    { 3500,   0 }, { 3550,   5 }, { 3650,  10 }, { 3700,  15 },
    { 3750,  20 }, { 3790,  30 }, { 3820,  40 }, { 3850,  50 },
    { 3900,  60 }, { 3950,  70 }, { 4000,  80 }, { 4100,  90 },
    { 4200, 100 },
};
```

**Cross-check against this project's own §9.5 observations before trusting the bottom of the curve on the bench.** Factory firmware's low-batt blink at ~3450 mV is already below this table's 3500 mV floor (clamps to 0%, so this port's 15% warning at ~3700 mV fires about a quarter volt earlier than the factory one), and its full-screen takeover at ~3200 mV is further below it still. Plausible, not yet confirmed on this port's own hardware — bench-verify per §11.6 before shipping, same standard every other number in this document was held to.

**LOW threshold: 15%**, upstream's `DIAL_BATTERY_PCT_LOW`. Matches this project's own curve position for the ~3700 mV region, comfortably above the factory-firmware blink point (~3%) so the warning fires with real runway left, not at the last minute.

**Do not adopt:** their separate `SCR_DIAG` swipe-triggered screen (dead entry point on this port too — same `touch_filter`-swallows-the-standby-gesture problem their own commit message names, and this port doesn't want a new screen anyway, see §11.3), their `dial_battery` component structure, or their exact USB-detect thresholds.

### 11.2 State — extends §10.3, does not replace it

```c
dial_power_src_t power_src;   // unchanged (§10.3)
uint16_t         power_mv;    // unchanged (§10.3)
int8_t            power_pct;   // NEW. 0..100 while power_src == PWR_BATTERY;
                                // -1 (UNKNOWN) while PLUGGED or PWR_UNKNOWN —
                                // there is no cell reading to give while the
                                // rail is the charger (§9.5's "two regimes,
                                // one pin"), and inventing one is worse than
                                // admitting there isn't one (PR #4's own
                                // framing, and correct).
```

Computed in `dial_power.c`'s existing `pwr_sample_and_classify()`, from the curve above, on every classified BATTERY sample. Rides the same commit as `power_src`/`power_mv` — no new commit path, no new task.

### 11.3 UI — About redesign, no new screen

**Decision: the diagnostics content (Wi-Fi, battery, build) becomes new/changed rows on the existing `SCR_ABOUT`, not a new screen.** `SCR_ABOUT` is already in `nav_policy`'s sticky set, already reached from Menu, already the scrollable-row-list pattern (`scr_settings.c`'s row factory, ported verbatim per `scr_about.c`'s own comment) that every other read-only info screen in this port uses. Upstream's separate tap-to-dismiss visual face is a different UI paradigm for the same job; adopting it means a second navigation pattern in a UI that has exactly one everywhere else. Not adopting their swipe-down entry point either — it doesn't work on this port for the same reason it didn't on theirs (`touch_filter` consumes the standby wake before the gesture reaches the screen), and this port doesn't want a new gesture to solve that when Menu → About already exists.

**Rows, in order (extends the existing Back / Firmware / IDF / Serial / Power list):**

- **Firmware, IDF** — unchanged. Already "build" per the user's ask; no new row needed.
- **Serial → Pad** — this is `START-HERE.md`'s already-queued beta.5 item (`app_state_t.serial` is an unwritten Orion fossil; replace with the pad address, scheme stripped). Do in the same pass since this touches the same screen — don't leave two half-migrated rows.
- **Wi-Fi** *(new)* — reuse `scr_wifi.c`'s existing pattern verbatim rather than inventing a second one: `esp_wifi_sta_get_ap_info()` + its `signal_word()` helper (`Strong` ≥ -60 dBm, `Good` ≥ -70 dBm, else weaker). Row value e.g. `MyNetwork · Good (-58 dBm)`. `esp_wifi_sta_get_ap_info()` is documented thread-safe (`scr_wifi.c`'s own comment already establishes this), so About can call it directly from `on_state` the same way `scr_wifi.c` does — no new plumbing through `dial_state`.
- **Power → Battery** *(renamed from upstream's plain "Power", extended)* — `USB  4.61 V` unchanged while plugged; while on battery, `Battery  78%  (4.05 V)` — percentage **and** the raw voltage this port already shows, per the owner's call to add rather than replace. `--` while UNKNOWN, unchanged.

**Dial-face / standby badge — upgraded, not replaced.** Same slot as §10.4 (`(180, 46)`, top center inside the arc, night-face-visible at `LV_OPA_40`), same two-states-only rule (nothing shown while plugged past the 3 s CHARGE confirmation — an always-on percentage on an instrument face is exactly what §10.4 already argued against). What changes:

- **Shared code location, confirmed by reading the actual source (not guessed):** the glyph is NOT duplicated per-screen. `scr_dial.c` and `scr_standby.c` both call into one shared implementation in `ui_screens_internal.h` — `power_glyph_t` / `power_glyph_create()` / `power_glyph_apply()` / `power_glyph_destroy()` — specifically so the two screens can't drift out of agreement on the transition logic. The badge upgrade extends THAT shared component (swap its `LV_SYMBOL_BATTERY_EMPTY` label for a custom-drawn fill, add the breathing behavior inside `power_glyph_apply()`), not two separate per-screen implementations.
- The badge is **custom-drawn** rather than an `LV_SYMBOL_BATTERY_EMPTY` glyph, so the fill actually tracks `power_pct` instead of LVGL's five-bucket rounding (PR #4's own reasoning for drawing it, and correct — a five-bucket icon at 78%, 65%, and 52% all draw identically).
- **At or below `DIAL_BATTERY_PCT_LOW` (15%): breathes red.** Resolved Sep 4 2026 (owner, after a scoping question — this was underspecified in the first pass of this section):
  - **Color:** `pal->warning` (`dial_palette.h`'s existing "faults only — never thermal" token — low battery qualifies, and it's already night-safe: day `0xE82818`, night `0xC83010`, blue channel `0x10` satisfies this project's own checkable night rule of blue ≤ `0x18`). Not a new/invented red.
  - **Animation mechanism:** ping-pong opacity on the label, same primitive as `scr_dial.c`'s `chevron_start()` (`lv_anim_path_ease_in_out`, infinite repeat) — reuse that exact curve, don't write a third animation implementation. Continuous while the condition holds (not finite like `power_hint_pulse()`'s two-breathe acknowledgment — this is a standing state, not a one-shot response to input).
  - **Opacity range — day vs. night are DIFFERENT, deliberately:** day `LV_OPA_60 ↔ LV_OPA_100` (identical to the chevron pulse's own range). Night `LV_OPA_20 ↔ LV_OPA_50` — NOT the same range as day with only the period changed. This badge already has its own separate, lower night ceiling today (`LV_OPA_40` steady, vs `LV_OPA_COVER` by day, `scr_dial.c` ~line 719) specifically because this is a bedside device; breathing a red warning glyph up to full brightness inches from someone's face at 2am would contradict that existing decision and the palette's whole blue-capped/dim night design. Period stays the existing 1.2s day / 2.4s night split either way.
- **Fill keeps a visible floor even near-empty** (PR #4's 2px-minimum reasoning) — an empty outline reads as a broken widget, not a warning.
- **Check the AWAY badge slot before building.** `somnus-dial-project-summary.md` records the AWAY badge as "kept dormant, not deleted — wired but hidden," and PR #4's own commit message notes their badge shares AWAY's slot on their layout, splitting the row when both show. This port's AWAY badge is currently hidden, so there is no live conflict today — but if AWAY is ever revived, this is exactly the recurring "does the new setting collide with a dormant one" shape this project checks for every time (`HARDWARE-bringup-log.md` §12). Note it in the commit, don't silently ignore it.

### 11.4 What stays out, still

- Upstream's separate `SCR_DIAG` screen and its swipe-down entry point (§11.3).
- Their `dial_battery` component and detector (§11.1) — this port's `dial_power.c` classifier stays authoritative.
- Anything that takes over the full face — §9.5's design rule is untouched by this revision. The badge stays a badge.
- Build info beyond Firmware/IDF (upstream's "build" row is redundant with what About already shows).

### 11.5 Sequencing

One feature per beta (owner's standing rule, `START-HERE.md`). This is a new beta on top of beta.4, not a re-spin of it. Reads and writes: `dial_power.c` (curve + `power_pct`), `dial_state.h`/`.c` (new field), `scr_about.c` (Wi-Fi row, Power→Battery row rework, Serial→Pad in the same pass since it's the same screen), `ui_screens_internal.h` (the shared `power_glyph_t`/`power_glyph_apply()` — badge draw + breathing, same slot as beta.4's glyph, replacing it not adding a second one; `scr_dial.c` and `scr_standby.c` only own each call site's position/color pass-through, unchanged).

### 11.6 Bench, before tag

Everything in §10.7 still applies (plug/unplug cycling, cable wiggle, night-face visibility). Added: percentage reads sanely across a full discharge if one is available, or at minimum spot-checked against the factory-firmware comparison points (~3450 mV ≈ low-batt blink, ~3200 mV ≈ factory takeover) per §11.1's caveat; the red breathing state is reachable and stops breathing once back above 15% (recharge or a fresh cell); the About Wi-Fi row matches what `scr_wifi.c`'s own screen reports for the same network at the same time (two independent readers of `esp_wifi_sta_get_ap_info()` should never disagree); Serial→Pad row shows the pad address correctly (beta.5 item, done here instead of separately).

### 11.7 Revised Sep 4 2026 (evening) — badge resolution and percentage smoothing, after review

Built and flashed the same evening; full write-up in `docs/REPORT-about-layout-battery-glyph.md`. Owner asked for an independent review of the badge and the discharge curve, approved every finding, then approved the changes.

- **Badge body 20 px wide (was 16), fill rounds to nearest pixel (was floor), fill floor 1 px (was 2).** At 16 px the usable span was 12 px and everything from 0 % to 24 % drew as the same 2 px bar — the §11.1 15 % threshold sat inside a band the bar could not move in. Now ~6 % per pixel; 5/15/20/50/100 % → 1/2/3/8/16 px.
- **Percentage reads the median of the newest 5 samples**, not the single 1 s reading. On BATT_CURVE's 3.75–3.85 V plateau one percent is 3–5 mV, so a lone reading's ADC noise plus a Wi-Fi TX sag was a 5–10 point swing. Median (not mean) so a single sag sample is discarded rather than averaged in. The 5-sample window is the same set that just voted BATTERY through the debounce, so no plugged-in reading leaks in after an unplug.
- **Held non-increasing while on battery.** A cell only discharges while unplugged; any upward tick is noise, a sag ending, or surface charge relaxing. Reset to −1 (unknown) whenever `power_src != PWR_BATTERY`, i.e. on plug-in. Consequence: the ≤ 15 % breathe cannot flap at the boundary, so no separate hysteresis on the low flag was added.
- **Detector untouched** — `pwr_classify()`, hysteresis, debounce, slope tiebreak are byte-identical to §10.2.
- **§11.1 correction:** the factory firmware's ~3450 mV blink is *below* this table's 3500 mV floor (0 %), not "roughly 3 %". This port's warning at ~3700 mV therefore fires about a quarter volt earlier than the factory one — deliberate runway.
- **USB-only boards (no cell):** the pin reads the USB rail, PLUGGED for the unit's whole life, battery branch unreachable; no error path. The firmware cannot tell "charging a cell" from "no cell" — hardware fact.

**Bench still owed (§11.6 stands):** the hold/median on a real discharge — About's number should sit still and only step down while unplugged, and the badge should visibly narrow between 25 % and 5 % before turning red. Serial cannot capture this (§7); it is an eyes-on check.
