# Spec — Rolling Differential, Firmware Port

**Status:** Draft. Findings from a read of the firmware tree on 2026-09-01. **No firmware
code has been written or changed** — this closes two of `SPEC-rolling-differential.md`'s
open questions from source, and raises one the earlier specs did not anticipate.
**Date:** 2026-09-01 (§4.3 and the §5 color correction added 2026-09-02)
**Scope:** the v2 firmware port. Companion to `SPEC-rolling-differential.md` (referred to
as `RD§`) and `SPEC-differential-beta-fixes.md` (`BF§`), both of which are implemented and
green in Bedknob for Mac (renamed from SomnusDialPreview on 2026-09-02; its own repo,
where `SPEC-rolling-differential.md` and `SPEC-differential-beta-fixes.md` also live —
they are **not** in this repo).

Everything below is from reading `dial_somnus.c`, `dial_state.c/.h` and `main.c`. Nothing
here has been compiled, flashed, or measured. The board had not arrived at time of writing.

---

## 1. RD§5 is answered: the freshness signal already exists

RD§5 closes with *"Open: whether `dial_somnus.c` currently carries this distinction or
whether it needs to be added. Check before implementing."*

**It carries it. No change needed.**

```c
bool dial_somnus_get_state(somnus_state_t *out);
```

`true` means a genuine HTTP 200 was received, parsed, and written into `*out`. `false`
means transport or parse failure, and `*out` is left completely unmodified. The header
states the contract outright: *"out is left unmodified on failure so a failed poll can
never clobber the last-known-good state."*

That boolean **is** the explicit signal RD§5 requires. It is not implicit freshness
inferred from a changed value — the thing RD§5 rejects — it is the transport layer
reporting whether a response actually arrived.

**Requirement for the port.** Append to the buffer from the success path of
`somnus_refresh_state()` (`main.c`, ~line 546), which already opens with:

```c
if (!dial_somnus_get_state(&s)) return false;
```

The append goes after that guard, against the freshly-populated `somnus_state_t s`.

**Never** feed the buffer from `app_state_t` via `dial_state_get()`. That store is
last-known-good by design and survives failed polls unchanged; sampling it on a timer is
precisely the fabrication RD§5 exists to prevent. This is the same distinction the Swift
beta draws between `decoded` and `side0`, and it maps onto the firmware cleanly.

---

## 2. New: `parse_side()` can return a stale setpoint on a fresh poll

Narrower than RD§5's trap, same family, and not covered by RD§4.

```c
out->target_c = cJSON_IsNumber(target) ? (float)target->valuedouble : out->target_c;
```

If a 200 response carries a `side0` object that is missing `target_t`, `parse_side()`
still returns true, `dial_somnus_get_state()` still returns true, and `target_c` silently
retains **the previous poll's value**. The differential for that sample would then be
computed as `current − <stale target>` while every freshness check says the sample is good.

`current_t` does not have this problem — a non-number sets `has_current = false`, which
RD§4 condition 2 already rejects. Only the setpoint is exposed.

**Decision.** RD§4's validity list grows a fourth condition:

> 4. `target_t` was present as a number in *this* response.

Two ways to implement, either acceptable:

- `parse_side()` reports field presence (e.g. a `bool has_target` alongside
  `has_current`), and the differential requires it. Preferred — it makes the gap visible
  at the layer that creates it, and costs one bool.
- The append site re-checks the raw JSON. Works, but duplicates parsing knowledge outside
  the parser.

**Do not** "fix" this by making `parse_side()` fail on a missing `target_t`. The
last-known-good fallback is correct for the *main display*, which should keep showing the
last known setpoint rather than blanking on a partial body. This is the same split RD§5
draws: a pattern that is right for the dial face and wrong for the differential.

Probability is low — the pad has always sent `target_t` — but the code explicitly tolerates
its absence, so the differential must not.

---

## 3. `dial_state` synchronization, and where the buffer lives

RD§12's second open question: *"What synchronization does `dial_state` use between
`worker_task` and the UI task, and does appending from the poll path need a lock?"*

**The store uses a FreeRTOS mutex.** `s_mux = xSemaphoreCreateMutex()` (`dial_state.c:67`),
taken and released around every accessor. Readers take a full snapshot with
`dial_state_get(app_state_t *out)`; writers mutate inside
`dial_state_commit(mutate, arg)` with the mutex held for the callback's duration, after
which a `generation` counter bumps and the LVGL dispatcher re-renders on change. The
header's rules are explicit: never hold pointers into the store, and *"keep mutators tiny
and never block in them."*

Separately, `s_input_mux` (`dial_state.c:62`) is a `portMUX_TYPE` spinlock rather than a
mutex, deliberately — the knob decoder runs in the shared `esp_timer` task and must never
block on a held mutex.

**Answer: the append needs no lock, because the buffer must not live in `dial_state`.**

`worker_task` is the only caller of `dial_somnus_get_state()` — `dial_somnus.h` states the
rule: *"call only from the worker task, never from the LVGL task."* A tracker held as a
file-static in the worker's translation unit is therefore single-writer, single-task, and
needs no synchronization at all.

**Decision — compute in the worker, commit the result:**

- The ring buffer and the weighted-mean computation stay private to `worker_task`.
- After each successful poll the worker computes the display state and commits a small
  fixed-size result through `dial_state_commit`: state enum, `delta_dc`, coverage seconds,
  `min_dc`, `max_dc`, `setpoint_moved`. Roughly ten bytes.
- The UI reads it out of its normal `dial_state_get()` snapshot and renders. It performs no
  arithmetic over samples.

**Rejected:** putting the sample buffer in `app_state_t` and computing UI-side. That runs
an O(n) loop over up to a few hundred samples under the store mutex — directly against the
"keep mutators tiny" rule, on a store whose lock ordering the knob decoder's spinlock
already constrains. The worker-computes/commits-small shape is what every other field in
this codebase already does.

---

## 4. The firmware has no fixed poll interval

**This is the finding neither earlier spec anticipated, and it invalidates two of their
numbers.** The Swift beta polls at a flat 7 seconds. The firmware does not have a flat
anything:

```c
#define KNOB_SETTLE_US    2500000    // 2.5s of no input before the bed is read back
#define POLL_INTERVAL_US 10000000    // idle cadence
#define POLL_CONFIRM_US   2000000    // 2s between confirm polls after a write
#define POLL_CONFIRM_N          3    // how many, before returning to idle cadence
```

and, in the worker loop:

```c
if (now - dial_state_last_input_us() < KNOB_SETTLE_US) continue;
int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
```

So the real cadence is 10s at idle, 2s for three polls after any write, and **zero** — polls
suppressed outright — while the user is interacting.

### 4.1 Buffer capacity must be sized on the fastest cadence

RD§3's `ceil(600s / poll_interval) + margin` has no single answer here. Sized on the idle
10s cadence it gives 60 entries and **will overflow** during a post-write burst; sized on
the 2s confirm cadence it gives 300.

**Decision.** Capacity is `ceil(600s / POLL_CONFIRM_US) + margin` — the fastest cadence the
worker can produce, currently `ceil(600/2) + margin` ≈ 304. At six bytes per sample
(`int16 delta_dc`, `int16 target_dc`, `uint32 t_ms` — see §5) that is ~1.8KB statically
allocated. Acceptable on the ESP32-S3, and 5× what a naive reading of the idle cadence
would have reserved.

If `POLL_CONFIRM_US` ever changes, the capacity expression must follow it. Derive it from
the macro; do not hardcode the number.

### 4.2 The quiet gate manufactures gaps that are not dropouts

RD§6 defines a dropout as a gap longer than `3 × poll_interval` and requires it not be
bridged. On the dial, a user spinning the knob for thirty seconds suppresses every poll in
that window via `KNOB_SETTLE_US`. That is not a network dropout — it is the device working
as designed — but it produces an identical-looking gap.

Excluding it is still the **right** behavior: no samples were taken, so no time was
observed, and bridging across it would invent data. RD§6's rationale holds unchanged.

The consequence is a UX one, and it is not small: coverage drops right after any
interaction, which is exactly when someone is looking at the screen. A user who spins the
knob and immediately taps to see the trend gets reduced coverage, or falls under the
minimum and sees a placeholder.

**Decision.** Keep the exclusion. Do not special-case input-suppressed gaps as "observed."
But the dial's differential readout must make the resulting coverage legible rather than
mysterious — the coverage line is already mandatory per RD§6, and this is the case that
justifies it most.

**Open, and a genuine judgment call for first flash:** whether `3 ×` should be computed
against the idle interval (30s) or the confirm interval (6s). Against the confirm interval,
every ordinary idle 10s poll gap exceeds the threshold and *every* sample looks like a
dropout — clearly wrong. Against the idle interval, a 30s threshold is coarse relative to
the 2s confirm bursts. Provisionally: compute `gapThreshold` against `POLL_INTERVAL_US`
(30s), and revisit with real failure-rate data per RD§7. This must not be tuned by
reasoning; it is listed in §6 below.

### 4.3 A cold boot starts with an empty window — and boots are routine

Added 2026-09-02.

The buffer is RAM-only and file-static in the worker's translation unit (§3). **Every power
cycle starts it empty**, and the ten-minute window then takes ten minutes to fill.

This matters more here than it would on the Mac beta, because of a hardware fact the earlier
specs did not account for: **the dial has a physical power switch, and the owner's board is
the battery-equipped SKU** (`SPEC-power-sensing.md` §5). Powering the dial off is a normal
user action on this device, not an exceptional one. A Mac app runs for days; this thing gets
switched off and on.

**Consequences:**

- For roughly the first ten minutes after every power-on, coverage is partial. With RD§7's
  50% placeholder minimum, that is ~5 minutes of placeholder on **every** boot — and the
  first thing a user does after switching the dial on is look at it.
- Filling-after-boot and dropout-riddled look **identical** to a coverage percentage, and
  they mean completely different things. "Still gathering, back shortly" is fine;
  "unreliable readings" is alarming. RD§6's coverage line cannot distinguish them on its
  own.

**Decision — do not persist the buffer to NVS.** Three reasons, in order of weight: a dial
that was switched off observed nothing, so bridging across the off period would be exactly
the fabrication RD§5 exists to prevent; the off period is arbitrary and unknowable in
advance, so stale samples could be hours or weeks old; and it would put a write-heavy
workload on flash for a cosmetic gain.

**Requirement instead:** the readout must distinguish *warming up* from *degraded*. The
worker knows its own uptime, so a window that has never been full since boot is
distinguishable from one that has lost samples — track it explicitly rather than inferring
it from coverage alone. Wording and placement fall under RD§8, to be decided against the
real display.

---

## 5. Carried forward from the beta

Three things the Swift beta established that the port inherits:

- **BF§2, one meaning per color — the principle carries, the hues do not.** Corrected
  2026-09-02. BF§2 states the rule as *"coral = below setpoint, cyan = above, violet =
  in-band, everywhere."* Those are **Bedknob for Mac's** Swift colors and **none of them
  exist in the firmware.** The dial's equivalents come from `dial_palette.c`:
  `accent-heat` `#E86018` (below setpoint, warming), `accent-cool` `#3888C8` (above
  setpoint, cooling), `neutral-holding` `#587868` (in band). Porting the beta's hues
  literally would inject cyan — blue channel 219 — into a firmware whose night palette caps
  blue at `0x18`, and none of the four Swift values quantize to RGB565. See
  `docs/SPEC-brand-palette.md` §1 and `firmware/dial-idf/docs/design-spec.md` §2.

  The collision the rule exists to prevent is real and unchanged: `scr_dial.c`'s status
  pill computes `target − current` while the differential is `current − target` (RD§1);
  both signs are correct, and the colors must still agree on a screen showing both.

  **New requirement the beta could not have known:** the firmware carries every state in a
  **parallel shape channel** — `LV_SYMBOL_UP` ▲, `DOWN` ▼, `MINUS` ▬, `STOP` ○, `CLOSE` ×
  (design-spec.md §2) — so that hue never works alone, and specifically so night cooling
  reads without blue. The differential readout must adopt a glyph on the same grammar
  rather than relying on color. A differential that is color-only is a night-mode
  regression, not a style choice.

- **BF§5, the setpoint-change marker.** The tracker stores `target_t` per sample and flags
  a window spanning more than one distinct value. On the dial this matters far more than it
  did on the Mac: the pad's 3-stage overnight schedule moves the setpoint unprompted, so a
  transient-dominated window is the normal overnight case, not an edge case. Note this is
  the same stored `target_dc` §2 requires for validity — one field, two purposes.
- **RD§10, unit safety.** Storage and computation stay in tenths of °C. The firmware's
  canonical unit is already `dc` per `dial_state.h`, and `dial_f_to_c()` was deliberately
  deleted rather than renamed. A differential converts for display as `×1.8` with **no +32**
  — and there is currently no `dial_dc_to_f`-style helper for deltas, so one must be added
  rather than reusing `dial_dc_to_f()`, which carries the offset. Note the **face displays
  °F** (`DIAL_TEMP_MIN_F` 55, `DIAL_TEMP_MAX_F` 110) while the API and every spec here are
  °C; the differential inherits that split.

---

## 6. Deferred to first flash — unchanged discipline

RD§7's list stands, with §4 sharpening two entries. Do not tune by reasoning:

- Real poll failure rate under the dial's own Wi-Fi, on a nightstand rather than a Mac.
- Whether `gapThreshold` computes against the idle or confirm interval (§4.2).
- Whether `3 ×` is the right multiplier at all.
- Minimum coverage threshold. Still the beta's 50% placeholder; the dial's suppression
  behavior (§4.2) and its empty-at-boot window (§4.3) may both argue for a lower one.
- Actual pad round-trip latency.
- RD§8's placement decision, against the real 1.8" display in a dark bedroom — including
  §4.3's warming-up wording and §5's state glyph.

---

## 7. Summary of changes to the earlier specs

| Item | Status |
|---|---|
| RD§5 open question — does `dial_somnus.c` carry a freshness signal? | **Closed. Yes** — `dial_somnus_get_state()`'s return value (§1). |
| RD§12 Q2 — `dial_state` synchronization, does the append need a lock? | **Closed.** Mutex-guarded store; buffer lives in the worker, needs no lock (§3). |
| RD§4 validity conditions | **Amended** — a fourth condition, `target_t` present this response (§2). |
| RD§3 buffer capacity | **Amended** — size on the fastest cadence, not the idle one (§4.1). |
| RD§6 gap threshold | **Provisional** — exclusion behavior confirmed correct; the interval it derives from is now an open question (§4.2). |
| Cold-boot behavior | **New** — buffer is RAM-only and power cycling is routine on this hardware; do not persist, but distinguish warming-up from degraded (§4.3). |
| BF§2 color rule | **Corrected** — the beta's coral/cyan/violet do not exist in the firmware and violate its RGB565 and night-blue rules; use `dial_palette.c` tokens, and add a state glyph (§5). |
