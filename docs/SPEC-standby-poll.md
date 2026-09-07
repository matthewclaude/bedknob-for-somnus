# Spec: Standby poll cadence

Status: **Draft, 2026-09-06. Not built. Scheduled as `0.1.7-beta.1`, after `0.1.6` graduates to stable.** Owner's call on 2026-09-06 that the STANDBY-tier polling is unnecessary load; this spec is the "spec on disk before code" step. Owner marked it not urgent and delegated the timing (Sep 6) — so it waits behind beta.3, the 0.1.6 graduation and the 1.0.0 decision, and does not get folded into any 0.1.6 beta. Reasoning in §8. Supersedes the "future task, nicety not requirement" note of 2026-09-04.

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
#define POLL_STANDBY_US  60000000    // STANDBY cadence: once a minute
```

```c
int64_t due = poll_confirms > 0            ? POLL_CONFIRM_US
            : dial_power_level() == DPWR_STANDBY ? POLL_STANDBY_US
            :                                   POLL_INTERVAL_US;
```

Rules, in priority order:

1. **Confirm polls win.** After a write, the three 2 s confirm polls run regardless of tier. (A write from the dial implies input, which wakes the tier anyway; this rule exists so the ordering is explicit, not because the case is reachable.)
2. **STANDBY polls once a minute.** ACTIVE and DIMMED are unchanged at 10 s. DIMMED is a short transition on the way to STANDBY, not a resting state; giving it its own cadence is a third number for no benefit.
3. **Leaving STANDBY forces a poll due now.** The worker tracks the tier it last saw; on a transition out of `DPWR_STANDBY` it treats the poll as overdue. The existing `KNOB_SETTLE_US` gate still applies — the wake poll lands ~2.5 s after the last input, exactly as an idle poll does today. Worst-case number shown on wake: (60 s + 2.5 s) old, versus (10 s + 2.5 s) today. Deliberately **not** bypassing the settle gate for the wake poll: one gate, one rule, and the optimistic-state protection in `mut_device_state` was designed around it.
4. **No user setting.** A constant. There is nothing to change from any device state and nothing for the sticky-picker lesson of `0.1.6-beta.2` to bite. If the number is wrong, it is wrong for everyone and gets fixed in a beta.
5. **One INFO log line on each cadence change** (`poll: standby cadence 60s` / `poll: active cadence 10s`), so a serial capture proves the mechanism instead of someone counting request lines. Not per poll.

Nothing else moves. Confirm count, settle time, idle cadence, the failure counter, `PH_DEGRADED`, the OTA gates, the wake-consume rule, discovery — untouched.

## 4. Why 60 seconds

- **~85 % fewer requests overnight** (6/min → 1/min). Going from 10 s to 60 s captures nearly all of the win; 60 s → 300 s only takes another 13 % of the original and costs everything below.
- **The standby face stays truthful.** A setpoint changed from the app appears on the dimmed face within a minute. Water temperature moves on the order of a degree a minute at most, so the night face's alternating water number is never visibly wrong.
- **Outage detection stays sane.** Three failures at 60 s = 3 minutes to `PH_DEGRADED` in standby, versus 30 s today. Acceptable: nobody is looking, and the wake poll (§3.3) re-tests the pad within seconds of someone looking. At 300 s that would be 15 minutes and a wake into a stale face that hadn't noticed the pad was gone.
- **Schedule tracking.** The pad's own 3-stage night schedule steps land on the dial within a minute of the pad applying them. Item 7's verification is repeatable at this cadence.
- It is one constant. If a night on the bench says 30 s or 120 s, that is a one-line beta.

## 5. The two questions, and the shape to check

*Changeable from the state the device will be in when it needs changing?* Not applicable — no setting. *Read by anything?* The worker loop, which is the same code that owns the constant. The consumer cannot be dead because it is not separate.

The shape that **can** go wrong here is the reverse one: a mechanism this spec quietly changes without meaning to. The reviewer must confirm each of these still behaves at a 60 s cadence, by reading the code, before anything is flashed:

- The staleness dot (`scr_dial.c` ~431): is it driven by poll **failure** or by poll **age**? If it is age-based with a threshold under 60 s, the standby face would show the stale dot every minute. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so.
- `dial_power_level()` called from `worker_task`: confirm it is safe to read from a task other than the one that computes the tier (it already appears in `main.c`'s OTA nav block — establish which task that runs on). If it is not, the tier is read through the state snapshot instead.
- Anything else that assumes "a successful poll happened in the last ~10 s": grep for `POLL_INTERVAL_US` and for readers of the last-poll timestamp. The rolling-differential firmware port (`docs/SPEC-differential-firmware-port.md` §4) already sizes its buffer on the *fastest* cadence, so it is unaffected; anything sizing on the slowest is not.

## 6. Things to check on hardware, not assume

1. **Cadence, by count.** Serial capture: let the dial time out to STANDBY, count poll lines over 10 minutes. Expect ~10, not ~60. Then wake it and count over 2 minutes at ACTIVE: expect ~12. The §3.5 log lines bracket each phase.
2. **Wake poll.** With the dial in STANDBY, change the setpoint from the Somnus app, wait 5 s, touch the knob. The face must show the new setpoint within ~3 s of the touch (the settle gate), not up to a minute later.
3. **External change in STANDBY.** Change the setpoint from the app while the dial sits in STANDBY with Standby face = Temperature. The dimmed face updates within 60 s. Restore the setpoint afterwards — it is a real bed.
4. **Night face at STANDBY.** With the Tokyo-timezone trick, confirm the number/water alternation still runs at the dim floor and the water number tracks (it is keyed on heating/cooling state, which the poll supplies; a 60 s cadence must not freeze the alternation).
5. **Outage in STANDBY.** Unplug the pad (or block it) with the dial in STANDBY. Expect `PH_DEGRADED` after ~3 minutes, staleness dot shown, and — on wake — a fresh poll attempt within seconds. Restore the pad, confirm recovery.
6. **One unattended night**, repeating item 7's shape: the pad's 3-stage schedule tracked on the dial's face, an app write from bed propagated, morning log clean. This is the gate for the tag: the previous overnight proof ran at 10 s and does not carry over.
7. **Unattended OTA still fires** at STANDBY if an update is available — unrelated code, but it is the one mechanism that only runs in the tier this spec touches, so it gets watched once.

Simulator: not applicable — no worker task, no poll.

## 7. Commits

1. **The change.** `POLL_STANDBY_US`, the `due` branch, the last-seen-tier edge in the worker loop, the two log lines, the §5 code checks written up in the report. Build, flash by wire, §6 items 1–5 on the bench.
2. **Overnight.** §6 item 6, then 7 if an update is pending. No code.
3. **Release** as its own beta after the overnight passes: `CHANGELOG.md` section, `PROJECT_VER` bump, tag, push to `somnus` only. Hardware before tag.

## 8. Where it lands

**Decided 2026-09-06: `0.1.7-beta.1`**, after `0.1.6` graduates to stable. `0.1.6` is otherwise a pure fix release on top of the standby face and is the clean `1.0.0` candidate; a behaviour change to the overnight poll loop is exactly the kind of thing that should get its own soak rather than ride into a stable graduation. Owner delegated the timing and marked it not urgent; `0.1.6-beta.4` was the alternative and was not taken.

## 9. Not in this spec

- Any DIMMED-tier cadence.
- Wi-Fi power-save, modem sleep, light/deep sleep, screen-off. See §1.
- A user-facing setting or About-screen readout of the cadence.
- Bedknob for Mac and Bedknob Mini poll intervals — separate apps, separate specs if ever.
- Changing `KNOB_SETTLE_US`, `POLL_INTERVAL_US`, `POLL_CONFIRM_US` or `POLL_CONFIRM_N`.
