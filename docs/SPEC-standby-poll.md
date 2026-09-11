# Spec: Standby poll cadence

Status: **Draft, 2026-09-06; amended 2026-09-11. Not built. Scheduled as `somnus-v1.0.2-beta.1`, queued behind the repo consolidation (`docs/SPEC-repo-consolidation.md`).** Owner's call on 2026-09-06 that the STANDBY-tier polling is unnecessary load; this spec is the "spec on disk before code" step. Owner marked it not urgent and delegated the timing (Sep 6), so it waited behind the `1.0.0` graduation and now waits behind the consolidation, which has a closing window and this does not. The `0.1.7-beta.1` slot it was first given no longer exists: the project renumbered on 2026-09-09, `0.1.6` shipped as `1.0.0`, and current stable is `1.0.1`. The cadence was changed from 60 s to 300 s on 2026-09-11 (§4). Reasoning on placement in §8. Supersedes the "future task, nicety not requirement" note of 2026-09-04.

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md`, `LICENSING.md` and `somnus-dial-project-summary.md` are **not in this repo** — they live only in the Claude Project. Everything this spec needs is restated here.

## 1. The problem, stated honestly

`worker_task`'s steady-state poll loop reads `GET /api/state` on a cadence that is completely decoupled from the display power tier. It polls at the same rate whether the dial is ACTIVE with a hand on the knob or has been sitting in STANDBY at the dim floor for eight hours. Overnight that is roughly six requests a minute, ~3,000 requests a night, against a pad whose only job at 3 am is running its own schedule.

What this costs is **not bandwidth** — six few-hundred-byte JSON bodies a minute on a home LAN is nothing, and the spec must not be sold on that. What it costs is the pad's HTTP server answering requests nobody reads, the dial's radio never going quiet, and log noise on both ends. Those are real and worth a constant's worth of code. That is all this is.

What this is **not**: a battery feature. The dial was never designed to run on battery as a normal mode; battery is an unplugged-briefly nicety (`docs/SPEC-power-sensing.md` §9.3 records a possible power-bank-class charger IC that cuts power under light load). Nothing in this spec touches Wi-Fi power-save, screen-off, light sleep or deep sleep, and nothing in it may be justified by battery life.

## 2. What exists today (read before touching)

The cadence lives in four constants in `main.c` and one `due` computation in the steady-state loop:

```c
#define KNOB_SETTLE_US    2500000    // 2.5s of no input before the bed is read back
#define POLL_INTERVAL_US 10000000    // idle cadence
#define POLL_CONFIRM_US   2000000    // 2s between confirm polls after a write
#define POLL_CONFIRM_N          3    // how many, before returning to idle cadence
```

```c
if (now - dial_state_last_input_us() < KNOB_SETTLE_US) continue;
int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
```

So the real cadence is: 10 s at idle; 2 s for three polls after any write; **zero** (polls suppressed) while the user is interacting. `dial_cmd_post()` calls `dial_state_stamp_input()`, so every posted command counts as input for the settle gate.

Other facts the change leans on:

- `dial_power_level()` returns `DPWR_ACTIVE` / `DPWR_DIMMED` / `DPWR_STANDBY`. The tiers are keyed on idle time and the Screen timeout pref (`components/dial_power/dial_power.c`), not on which screen is showing.
- Wake rule: `main.c`'s touch filter and knob step call `dial_power_wake_consumes()` first; the first input while STANDBY/DIMMED flips the tier to ACTIVE and is swallowed before any screen handler sees it.
- `mut_device_state` keeps the optimistic on/temp if input arrived after the poll began, and clears `ui_temp_dc` only on a successful poll with no newer input. A poll landing near a knob turn cannot roll the number back. This protection already exists and is what makes an immediate wake poll safe.
- Three consecutive steady-state poll failures set `PH_DEGRADED`; `nav_policy()` keeps the user on the dial face with the staleness dot (`scr_dial.c`, around line 431) through the outage.
- With Standby face = Temperature (the default since `0.1.6-beta.2`, `docs/SPEC-standby-face.md`), the dial face **is** the STANDBY screen: the setpoint, and at night the number-only face alternating with the water temperature while heating/cooling (`docs/SPEC-night-face.md` §3). STANDBY is no longer "a clock nobody is reading" — pad state is on screen, dimmed. The poll must keep it truthful.
- Unattended overnight OTA runs only at `DPWR_STANDBY` (`main.c`, the OTA nav block); the update prompt fires on the wake edge out of STANDBY/DIMMED (`ota_prompt_woke`). Both read the power tier, neither reads the poll timer.
- The overnight verification that matters (item 7, 2026-09-02, `HARDWARE-bringup-log.md` §14.1 — project-only): external-change tracking against the pad's own 3-stage schedule, and writes from the Somnus app and Bedknob for Mac, all propagated to the dial. That was proven at the 10 s cadence. This spec changes the cadence that verification ran on, so it has to be re-run (§6).

## 3. The change

One new constant and one extra branch in `due`:

```c
#define POLL_STANDBY_US 300000000    // STANDBY cadence: once every five minutes
```

```c
int64_t due = poll_confirms > 0            ? POLL_CONFIRM_US
            : dial_power_level() == DPWR_STANDBY ? POLL_STANDBY_US
            :                                   POLL_INTERVAL_US;
```

Rules, in priority order:

1. **Confirm polls win.** After a write, the three 2 s confirm polls run regardless of tier. (A write from the dial implies input, which wakes the tier anyway; this rule exists so the ordering is explicit, not because the case is reachable.)
2. **STANDBY polls once every five minutes.** ACTIVE and DIMMED are unchanged at 10 s. DIMMED is a short transition on the way to STANDBY, not a resting state; giving it its own cadence is a third number for no benefit.
3. **Leaving STANDBY forces a poll due now.** The worker tracks the tier it last saw; on a transition out of `DPWR_STANDBY` it treats the poll as overdue. The existing `KNOB_SETTLE_US` gate still applies — the wake poll lands ~2.5 s after the last input, exactly as an idle poll does today. Worst-case number shown on wake: (300 s + 2.5 s) old, versus (10 s + 2.5 s) today. Deliberately **not** bypassing the settle gate for the wake poll: one gate, one rule, and the optimistic-state protection in `mut_device_state` was designed around it.
4. **No user setting.** A constant. There is nothing to change from any device state and nothing for the sticky-picker lesson of `0.1.6-beta.2` to bite. If the number is wrong, it is wrong for everyone and gets fixed in a beta.
5. **One INFO log line on each cadence change** (`poll: standby cadence 300s` / `poll: active cadence 10s`), so a serial capture proves the mechanism instead of someone counting request lines. Not per poll.

Nothing else moves. Confirm count, settle time, idle cadence, the failure counter, `PH_DEGRADED`, the OTA gates, the wake-consume rule, discovery — untouched.

## 4. Why 300 seconds, for this beta

The first draft of this section argued for 60 s from an assumed rate — "water moves on the order of a degree a minute at most" — and against 300 s on outage-detection grounds. The owner has since read an overnight history chart from the pad itself, and the case below is grounded in that instead. 300 s is the owner's choice for this beta, a deliberate try-it-and-see.

**What the pad actually did overnight.** Pad state changed exactly twice in the window: the setpoint stepped from 64.4 °F to 68.0 °F at roughly 3:20–3:30, and dropped back at about 7:00 when the schedule ended. Between about 4:15 and 6:55 the water sat on 68.0 °F with ripples of roughly ±0.2 °F — the control loop hunting. At the whole-degree °F the dial renders, that stretch does not change at all. Fast movement is confined to the minutes after a step: the 3:30 heat-up is near-vertical at chart scale, consistent with about a degree a minute for that burst, and the drift after the pad goes off at 7:00 is about 68 → 72 °F over half an hour, roughly 0.13 °F per minute, flattening after that.

*Caveat, recorded honestly:* the chart's own sampling interval is unknown, so a transient shorter than its resolution would not appear. The near-vertical segment at the 3:30 step is where such a transient would hide.

- **~97 % fewer requests overnight** (6/min → 1 per 5 min: ~2,880 → ~96 over an eight-hour night), against the ~85 % that 60 s would have given (~480).
- **Even 300 s oversamples heavily.** The night's information content is two state changes. ~96 polls against two changes is roughly 50 samples per change; 60 s would have been roughly 240 per change. Neither cadence is tuned to the signal; 300 s is merely less untuned.
- **Where staleness actually lands.** Almost entirely in the few minutes after each schedule step: a step reaches the dimmed face up to five minutes late instead of up to one, and during the heat-up burst the water number can lag by a few degrees for those minutes. The rest of the night the water is within a fraction of a degree of where the last poll left it — ±0.2 °F on the plateau, well under a degree across any five-minute window of the post-off drift — which the whole-degree °F display cannot show. The plateau from 4:15 to 6:55 renders identically at 10 s, 60 s or 300 s.
- **A passive glance wakes nothing.** Looking at the dimmed face does not touch the dial, so only touch triggers the wake poll (§3.3). The glance sees whatever the last standby poll left, up to five minutes old. This is the case Standby face = Temperature exists to serve, and the plateau observation above is why five minutes is tolerable there.
- **Outage detection is the one cost this evidence does not address.** Three failures at 300 s is fifteen minutes to `PH_DEGRADED` and the staleness dot, versus three minutes at 60 s and 30 s today. The wake poll re-tests the pad within seconds of a touch, so the exposure is a passive glance at a face that has not yet noticed the pad is gone. The chart says nothing about this. It is the thing to watch on the bench night (§6 item 5).

300 s is being tried for one beta. It is one constant. If the stale glance after a schedule step or the outage delay is noticeable, 120 s or 60 s is the expected fallback — a one-line change in the next beta.

## 5. The two questions, and the shape to check

*Changeable from the state the device will be in when it needs changing?* Not applicable — no setting. *Read by anything?* The worker loop, which is the same code that owns the constant. The consumer cannot be dead because it is not separate.

The shape that **can** go wrong here is the reverse one: a mechanism this spec quietly changes without meaning to. The reviewer must confirm each of these still behaves at a 300 s cadence, by reading the code, before anything is flashed:

- The staleness dot (`scr_dial.c` ~431): is it driven by poll **failure** or by poll **age**? This check matters more at 300 s than it did at 60 s: if it is age-based with a threshold under 300 s, the dimmed face would show the stale dot for most of every five-minute window. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so.
- `dial_power_level()` called from `worker_task`: confirm it is safe to read from a task other than the one that computes the tier (it already appears in `main.c`'s OTA nav block — establish which task that runs on). If it is not, the tier is read through the state snapshot instead.
- Anything else that assumes "a successful poll happened in the last ~10 s": grep for `POLL_INTERVAL_US` and for readers of the last-poll timestamp. The rolling-differential firmware port (`docs/SPEC-differential-firmware-port.md` §4) already sizes its buffer on the *fastest* cadence, so it is unaffected; anything sizing on the slowest is not.

## 6. Things to check on hardware, not assume

1. **Cadence, by log line and by gap.** Serial capture with timestamps: let the dial time out to STANDBY and confirm the `poll: standby cadence 300s` line (§3.5) appears on the transition, then confirm the next three poll lines are ~300 s apart — a 15-minute window, which is also item 5's timescale. Counting polls over 10 minutes gives 2 samples at this cadence and cannot tell 300 s from 200 or 600; the gap between consecutive poll lines measures the constant directly, and the §3.5 lines exist so that nobody has to count. Then wake it, confirm `poll: active cadence 10s`, and count over 2 minutes at ACTIVE: expect ~12.
2. **Wake poll.** With the dial in STANDBY, change the setpoint from the Somnus app, wait 5 s, touch the knob. The face must show the new setpoint within ~3 s of the touch (the settle gate), not up to five minutes later.
3. **External change in STANDBY.** Change the setpoint from the app while the dial sits in STANDBY with Standby face = Temperature. The dimmed face updates within five minutes. Restore the setpoint afterwards — it is a real bed.
4. **Night face at STANDBY.** With the Tokyo-timezone trick, confirm the number/water alternation still runs at the dim floor and the water number tracks (it is keyed on heating/cooling state, which the poll supplies; a 300 s cadence must not freeze the alternation).
5. **Outage in STANDBY.** Unplug the pad (or block it) with the dial in STANDBY. Expect `PH_DEGRADED` after ~15 minutes, staleness dot shown, and — on wake — a fresh poll attempt within seconds. Restore the pad, confirm recovery. This is now the item carrying the most weight: §4's evidence covers the truthfulness of the face and says nothing about outage detection, so the fifteen-minute window is the one cost of 300 s that only the bench can size.
6. **One unattended night**, repeating item 7's shape: the pad's 3-stage schedule tracked on the dial's face, an app write from bed propagated, morning log clean. This is the gate for the tag: the previous overnight proof ran at 10 s and does not carry over.
7. **Unattended OTA still fires** at STANDBY if an update is available — unrelated code, but it is the one mechanism that only runs in the tier this spec touches, so it gets watched once.

Simulator: not applicable — no worker task, no poll.

## 7. Commits

1. **The change.** `POLL_STANDBY_US`, the `due` branch, the last-seen-tier edge in the worker loop, the two log lines, the §5 code checks written up in the report. Build, flash by wire, §6 items 1–5 on the bench.
2. **Overnight.** §6 item 6, then 7 if an update is pending. No code.
3. **Release** as its own beta after the overnight passes: `CHANGELOG.md` section, `PROJECT_VER` bump, tag, push to `somnus` only. Hardware before tag.

## 8. Where it lands

**Decided 2026-09-06, renumbered 2026-09-11: `somnus-v1.0.2-beta.1`**, queued behind the repo consolidation (`docs/SPEC-repo-consolidation.md` §5, whose Phase 6a lands with the same beta). The original decision was `0.1.7-beta.1`, after `0.1.6` graduated to stable; the project renumbered on 2026-09-09, `0.1.6` shipped as `1.0.0`, the `0.1.7` line no longer exists, and current stable is `1.0.1`. The reasoning behind the placement is unchanged: `0.1.6` was a pure fix release on top of the standby face and the clean `1.0.0` candidate, and a behaviour change to the overnight poll loop is exactly the kind of thing that should get its own soak rather than ride into a stable graduation. Owner delegated the timing and marked it not urgent (2026-09-06); `0.1.6-beta.4` was the alternative and was not taken.

## 9. Not in this spec

- Any DIMMED-tier cadence.
- Wi-Fi power-save, modem sleep, light/deep sleep, screen-off. See §1.
- A user-facing setting or About-screen readout of the cadence.
- Bedknob for Mac and Bedknob Mini poll intervals — separate apps, separate specs if ever.
- Changing `KNOB_SETTLE_US`, `POLL_INTERVAL_US`, `POLL_CONFIRM_US` or `POLL_CONFIRM_N`.
