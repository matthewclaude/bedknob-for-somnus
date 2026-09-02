# SPEC — Dial-side scheduling

**Status: DEFERRED to a future version (v1.5 / v2.0). Not current work.** Owner decision, Sep 1 2026 — recorded the day the idea came up so the reasoning isn't lost, not because it is queued. Do not start building this.

**Idea:** delete the schedule in the Somnus app entirely and let the dial own it — temperature stages through the night, plus turning the pad on and off.

---

## 1. The finding that started this — RESOLVED 2026-09-02

> **Resolution:** option 1 + option 2 below were taken. The "Adjustment mode"
> row is hidden from Settings; the pref, screen and wiring stay dormant in
> the tree (same treatment as the AWAY badge); the three stale comments were
> corrected. Recorded in `docs/HARDWARE-bringup-log.md` §9.6. The analysis is
> kept because §6 depends on it.


`sched_follow` is **fully built as a setting and read by nothing.**

- Persisted to NVS (`dial_state.c`, key `sched_follow`, fresh-device default `true`)
- A complete "Schedule vs Hold" choice screen with explanatory prose (`scr_adjust_mode.c`)
- A Settings row displaying it
- **The only readers outside the UI: none.** Every `sched_follow` reader is a screen showing its own state back to itself.

The functions meant to consume it — `main.c`'s `temp_write_phase()` / `sleep_phase_now()` — went out when `worker_task` was rewritten from scratch during the Somnus port. Several comments continued to describe them as the live write path long afterwards.

**This is the fourth instance of a pattern** also seen with the pad address, the timezone, and the Settings row that would have fixed the timezone: a capability whose entry point survived while the mechanism behind it was deleted, with comments still describing it as working.

**Resolution taken Sep 2 2026:** the "Adjustment mode" row is **hidden** from `SCR_SETTINGS`. The pref, the NVS key, `scr_adjust_mode.c` and all its wiring stay in the tree, dormant — the same treatment `app_state_t.away` gets — ready if this feature is built. The stale comments were corrected in the same pass.

## 2. Why the feature is possible now and wasn't before Sep 1 2026

Scheduling requires knowing the local time. Until the timezone fix (`docs/SPEC-timezone-source.md`), `dial_time_valid()` returned false permanently because nothing ever called `dial_time_set_iana_tz()`. The dial ran on UTC and had no way to know when "10pm" was.

That fix also brings DST handling for free — the zone resolves through the embedded posix_tz_db table, so a 10pm bedtime stays 10pm across the spring and autumn transitions.

## 3. Contention — resolved in principle

The pad has its own 3-stage overnight schedule that moves `target_t` autonomously. Two controllers writing the same field would fight, with whoever wrote last winning each round.

**Confirmed by the owner: the Somnus app supports either a multi-stage schedule or a single fixed temperature, and the schedule can be deleted.** With no schedule on the pad, `target_t` moves only when something writes it.

This is a configuration precondition the firmware cannot enforce or detect. The API exposes no schedule endpoint and no way to read whether one is active. **The dial cannot tell whether the pad's schedule is on.** If a user enables both they fight, and nothing reports it. See the heuristic in section 7.

## 4. Scope

The pad's power state is writable via `POST /api/power`, and the dial already has `CMD_TOGGLE_ON`. So a schedule could cover:

- **Pre-bed:** power on and pre-cool ahead of bedtime
- **Overnight stages:** temperature changes through the night
- **Wake:** warm before waking, and/or power off afterwards

Powering off in the morning is a real benefit the app's schedule does not obviously give — it stops the pad conditioning an empty bed all day.

## 5. Architecture — the scheduler must be a pure function

**Given the local time now, what should the setpoint and power state be?** Not a sequence of timers, not a counter advancing through stages.

This is not a style preference. It is what makes the two real failure modes harmless:

- **The dial reboots mid-night.** On boot it asks "what should it be at 3:14am?" and applies that. A timer-based design would restart the sequence from stage one, or skip the rest of the night.
- **Wi-Fi drops for an hour.** On reconnect it asks the same question and applies the current answer. No catch-up replay, no missed-transition bookkeeping.

Integration point: `worker_task`'s steady-state loop already polls on a cadence and already has `dial_time_now()`.

**Write only on transitions**, not every poll. A write every 10 seconds would fight the user's own adjustments continuously and hammer the pad. Compare the computed target against the last value the scheduler itself wrote.

## 6. Manual override — what `sched_follow` was always for

The crux: the user turns the knob at 2am. Does the schedule overwrite it at the next stage boundary?

The dormant toggle answers this exactly, and its existing labels still fit:

- **Schedule** — a manual adjustment stands until the next phase boundary, then the schedule resumes.
- **Hold** — a manual adjustment wins for the rest of the night; the schedule does not resume until the next cycle.

The semantics shift slightly (it used to mean "follow the pad's schedule"; now "follow mine") but the user-facing choice is the same and the screen's prose largely survives.

Decide and document: does "Hold" persist across a mid-night reboot, or reset? Stateless-scheduler discipline (section 5) argues it must be persisted with a timestamp or expiry, not held in RAM.

## 7. Open questions

- **Schedule definition UI.** Bedtime, wake time, stage temperatures. Set-once, so some tedium is acceptable — unlike the Pad Address screen, which was intolerable partly because it blocked first use.
- **How many stages?** The pad's own is three. Matching it is a reasonable start.
- **Contention heuristic.** The dial cannot read whether the pad's schedule is active, but it *can* notice `target_t` changing to a value it did not write — a reliable "something else is driving this bed" signal. Worth surfacing rather than silently fighting.
- **Invalid clock.** `dial_time_valid()` can be false on a fresh device or before SNTP syncs. The scheduler must do nothing rather than guess. An unscheduled bed is fine; a bed scheduled off a wrong clock is not.
- **Bounds.** The API clamps to 12-42.3C, but the dial should refuse to write a scheduled value outside the relative scale's rails (120-420 dc) rather than relying on the pad to catch it.

## 8. If and when this is picked up

Start small: **wire `sched_follow` to a real consumer with a single hardcoded two-stage schedule**, verify it fires correctly across one night, and only then build the definition UI. That validates the scheduler, transition detection, override semantics and reboot recovery before any effort goes into screens.

Un-hiding the "Adjustment mode" Settings row is part of that work.

## 9. Hard prerequisite — satisfied once, 2026-09-02

The original prerequisite: before the dial is given authority over the bed's temperature while the owner sleeps, it should first be watched doing the passive thing correctly — tracking the pad through an overnight cycle without dropping its connection or losing the plot.

**Done once.** The dial ran one full night unattended and tracked the pad's own 3-stage schedule correctly — no hang, crash, or Wi-Fi drop (`docs/HARDWARE-bringup-log.md` §14.1). Multi-night heap growth is still uninstrumented; treat that as the remaining bar before this feature is picked up.
