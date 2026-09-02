# SPEC — Power sensing (USB vs battery)

**Status: measured, specified, NOT implemented. Not v1 scope.** Recorded Sep 2 2026 so the measurement never has to be redone. **See §8 — upstream tried this independently on identical hardware and declined to ship it.**

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
