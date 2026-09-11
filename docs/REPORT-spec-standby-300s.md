# REPORT: SPEC-standby-poll — STANDBY cadence 60 s → 300 s, §4 grounded in pad history, renumbered to `somnus-v1.0.2-beta.1`

**DONE.** Spec amended and committed alone as `7b7bc54` on `main`, not pushed. Documentation only: nothing under `firmware/`, `.github/` or `web-flasher/` was touched, no version bump.

Date: 2026-09-11. Operator: Claude Code (Fable 5.1), on the owner's instruction.

## 1. Gate checks, raw output

Run before any edit, in this order.

```
$ grep -c 'POLL_STANDBY_US  60000000' docs/SPEC-standby-poll.md
1
$ grep -c '0.1.7-beta.1' docs/SPEC-standby-poll.md
2
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All three passed: the constant was at 60 s exactly once, `0.1.7-beta.1` appeared twice (Status line and §8, as expected), and the tree was clean.

Note for a re-run: after this change, check 1 prints `0` and the constant reads `POLL_STANDBY_US 300000000` (one space fewer, so the columns still align). Check 2 still prints `2`, because the Status line and §8 now *mention* `0.1.7-beta.1` as the slot that no longer exists; it is no longer the scheduled version anywhere in the file.

## 2. BEFORE and AFTER

### Status line

**BEFORE**

```
Status: **Draft, 2026-09-06. Not built. Scheduled as `0.1.7-beta.1`, after `0.1.6` graduates to stable.** Owner's call on 2026-09-06 that the STANDBY-tier polling is unnecessary load; this spec is the "spec on disk before code" step. Owner marked it not urgent and delegated the timing (Sep 6) — so it waits behind beta.3, the 0.1.6 graduation and the 1.0.0 decision, and does not get folded into any 0.1.6 beta. Reasoning in §8. Supersedes the "future task, nicety not requirement" note of 2026-09-04.
```

**AFTER**

```
Status: **Draft, 2026-09-06; amended 2026-09-11. Not built. Scheduled as `somnus-v1.0.2-beta.1`, queued behind the repo consolidation (`docs/SPEC-repo-consolidation.md`).** Owner's call on 2026-09-06 that the STANDBY-tier polling is unnecessary load; this spec is the "spec on disk before code" step. Owner marked it not urgent and delegated the timing (Sep 6), so it waited behind the `1.0.0` graduation and now waits behind the consolidation, which has a closing window and this does not. The `0.1.7-beta.1` slot it was first given no longer exists: the project renumbered on 2026-09-09, `0.1.6` shipped as `1.0.0`, and current stable is `1.0.1`. The cadence was changed from 60 s to 300 s on 2026-09-11 (§4). Reasoning on placement in §8. Supersedes the "future task, nicety not requirement" note of 2026-09-04.
```

### §8 Where it lands

**BEFORE**

```
## 8. Where it lands

**Decided 2026-09-06: `0.1.7-beta.1`**, after `0.1.6` graduates to stable. `0.1.6` is otherwise a pure fix release on top of the standby face and is the clean `1.0.0` candidate; a behaviour change to the overnight poll loop is exactly the kind of thing that should get its own soak rather than ride into a stable graduation. Owner delegated the timing and marked it not urgent; `0.1.6-beta.4` was the alternative and was not taken.
```

**AFTER**

```
## 8. Where it lands

**Decided 2026-09-06, renumbered 2026-09-11: `somnus-v1.0.2-beta.1`**, queued behind the repo consolidation (`docs/SPEC-repo-consolidation.md` §5, whose Phase 6a lands with the same beta). The original decision was `0.1.7-beta.1`, after `0.1.6` graduated to stable; the project renumbered on 2026-09-09, `0.1.6` shipped as `1.0.0`, the `0.1.7` line no longer exists, and current stable is `1.0.1`. The reasoning behind the placement is unchanged: `0.1.6` was a pure fix release on top of the standby face and the clean `1.0.0` candidate, and a behaviour change to the overnight poll loop is exactly the kind of thing that should get its own soak rather than ride into a stable graduation. Owner delegated the timing and marked it not urgent (2026-09-06); `0.1.6-beta.4` was the alternative and was not taken.
```

### §3 constant and rules

**BEFORE**

```
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
```

**AFTER**

```
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
```

### §4 (whole section)

**BEFORE**

```
## 4. Why 60 seconds

- **~85 % fewer requests overnight** (6/min → 1/min). Going from 10 s to 60 s captures nearly all of the win; 60 s → 300 s only takes another 13 % of the original and costs everything below.
- **The standby face stays truthful.** A setpoint changed from the app appears on the dimmed face within a minute. Water temperature moves on the order of a degree a minute at most, so the night face's alternating water number is never visibly wrong.
- **Outage detection stays sane.** Three failures at 60 s = 3 minutes to `PH_DEGRADED` in standby, versus 30 s today. Acceptable: nobody is looking, and the wake poll (§3.3) re-tests the pad within seconds of someone looking. At 300 s that would be 15 minutes and a wake into a stale face that hadn't noticed the pad was gone.
- **Schedule tracking.** The pad's own 3-stage night schedule steps land on the dial within a minute of the pad applying them. Item 7's verification is repeatable at this cadence.
- It is one constant. If a night on the bench says 30 s or 120 s, that is a one-line beta.
```

**AFTER**

```
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
```

### §5 intro sentence and first bullet

**BEFORE**

```
The shape that **can** go wrong here is the reverse one: a mechanism this spec quietly changes without meaning to. The reviewer must confirm each of these still behaves at a 60 s cadence, by reading the code, before anything is flashed:

- The staleness dot (`scr_dial.c` ~431): is it driven by poll **failure** or by poll **age**? If it is age-based with a threshold under 60 s, the standby face would show the stale dot every minute. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so.
```

**AFTER**

```
The shape that **can** go wrong here is the reverse one: a mechanism this spec quietly changes without meaning to. The reviewer must confirm each of these still behaves at a 300 s cadence, by reading the code, before anything is flashed:

- The staleness dot (`scr_dial.c` ~431): is it driven by poll **failure** or by poll **age**? This check matters more at 300 s than it did at 60 s: if it is age-based with a threshold under 300 s, the dimmed face would show the stale dot for most of every five-minute window. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so.
```

### §6 item 1

**BEFORE**

```
1. **Cadence, by count.** Serial capture: let the dial time out to STANDBY, count poll lines over 10 minutes. Expect ~10, not ~60. Then wake it and count over 2 minutes at ACTIVE: expect ~12. The §3.5 log lines bracket each phase.
```

**AFTER**

```
1. **Cadence, by log line and by gap.** Serial capture with timestamps: let the dial time out to STANDBY and confirm the `poll: standby cadence 300s` line (§3.5) appears on the transition, then confirm the next three poll lines are ~300 s apart — a 15-minute window, which is also item 5's timescale. Counting polls over 10 minutes gives 2 samples at this cadence and cannot tell 300 s from 200 or 600; the gap between consecutive poll lines measures the constant directly, and the §3.5 lines exist so that nobody has to count. Then wake it, confirm `poll: active cadence 10s`, and count over 2 minutes at ACTIVE: expect ~12.
```

### §6 item 2

**BEFORE**

```
2. **Wake poll.** With the dial in STANDBY, change the setpoint from the Somnus app, wait 5 s, touch the knob. The face must show the new setpoint within ~3 s of the touch (the settle gate), not up to a minute later.
```

**AFTER**

```
2. **Wake poll.** With the dial in STANDBY, change the setpoint from the Somnus app, wait 5 s, touch the knob. The face must show the new setpoint within ~3 s of the touch (the settle gate), not up to five minutes later.
```

### §6 item 3

**BEFORE**

```
3. **External change in STANDBY.** Change the setpoint from the app while the dial sits in STANDBY with Standby face = Temperature. The dimmed face updates within 60 s. Restore the setpoint afterwards — it is a real bed.
```

**AFTER**

```
3. **External change in STANDBY.** Change the setpoint from the app while the dial sits in STANDBY with Standby face = Temperature. The dimmed face updates within five minutes. Restore the setpoint afterwards — it is a real bed.
```

### §6 item 4

**BEFORE**

```
4. **Night face at STANDBY.** With the Tokyo-timezone trick, confirm the number/water alternation still runs at the dim floor and the water number tracks (it is keyed on heating/cooling state, which the poll supplies; a 60 s cadence must not freeze the alternation).
```

**AFTER**

```
4. **Night face at STANDBY.** With the Tokyo-timezone trick, confirm the number/water alternation still runs at the dim floor and the water number tracks (it is keyed on heating/cooling state, which the poll supplies; a 300 s cadence must not freeze the alternation).
```

### §6 item 5

**BEFORE**

```
5. **Outage in STANDBY.** Unplug the pad (or block it) with the dial in STANDBY. Expect `PH_DEGRADED` after ~3 minutes, staleness dot shown, and — on wake — a fresh poll attempt within seconds. Restore the pad, confirm recovery.
```

**AFTER**

```
5. **Outage in STANDBY.** Unplug the pad (or block it) with the dial in STANDBY. Expect `PH_DEGRADED` after ~15 minutes, staleness dot shown, and — on wake — a fresh poll attempt within seconds. Restore the pad, confirm recovery. This is now the item carrying the most weight: §4's evidence covers the truthfulness of the face and says nothing about outage detection, so the fifteen-minute window is the one cost of 300 s that only the bench can size.
```

## 3. §6 item 1 — approach taken and why

Chosen: **lean on the §3.5 cadence log lines, and measure the gap between consecutive poll lines, rather than counting polls.** The item now asks for the `poll: standby cadence 300s` line on the transition, then three consecutive poll lines ~300 s apart (a 15-minute window), then the `poll: active cadence 10s` line on wake followed by the original 2-minute ACTIVE count (~12).

Why: at 300 s a 10-minute count yields 2 samples, which cannot distinguish 300 s from 200 s or 600 s, and a count long enough to be discriminating (an hour for ~12 samples) is a poor use of a bench session. The interval between consecutive timestamped poll lines measures the constant directly with two lines, and the §3.5 log lines were specified precisely so that a serial capture proves the mechanism without anyone counting request lines. The 15-minute window was chosen because it coincides with item 5's `PH_DEGRADED` timescale, so the same capture length serves both.

## 4. §6 items 2, 4, 6 and 7 — genuinely unaffected?

- **Item 2 (wake poll): NOT unaffected as written.** Its closing clause read "not up to a minute later", which is the 60 s cadence stated in words. The test itself (change from app, wait 5 s, touch, expect the new setpoint within ~3 s) is cadence-independent and unchanged. The clause was reworded to "not up to five minutes later". Recorded as a deviation in §6 below.
- **Item 4 (night face at STANDBY): NOT unaffected as written.** Its parenthetical read "a 60 s cadence must not freeze the alternation". The test is cadence-independent (alternation is keyed on heating/cooling state, which any successful poll supplies); only the number was stale. Reworded to "a 300 s cadence". Recorded as a deviation.
- **Item 6 (one unattended night): unaffected.** It names no cadence; its point is that the 10 s overnight proof does not carry over, which is truer at 300 s than at 60 s. Untouched.
- **Item 7 (unattended OTA at STANDBY): unaffected.** OTA gates read the power tier, not the poll timer (§2), so the cadence does not enter. Untouched.

## 5. Diff stat and commit

```
docs/SPEC-standby-poll.md | 46 +++++++++++++++++++++++++++-------------------
 1 file changed, 27 insertions(+), 19 deletions(-)
```

Spec commit: `7b7bc54ba325117c04e9f94596cece9b888ebc83`
`docs: SPEC-standby-poll — STANDBY cadence 60 s → 300 s, §4 grounded in pad history, renumbered to somnus-v1.0.2-beta.1`

Not pushed. This report and its `docs/REPORTS.md` line are a second, following commit.

## 6. Deviations

Four edits were made beyond the lines the task enumerated. Each is a literal 60 s reference that the task's own grep list did not name; leaving any of them would have left the spec contradicting its own constant. Each is a number-only change, shown in full in §2 above, and reversible in isolation.

1. **§3 rule 5, log-line text:** `poll: standby cadence 60s` → `poll: standby cadence 300s`. The task said rule 3's wake behaviour is unchanged and did not mention rule 5, but the log line states the cadence, and §6 item 1 now cites it by its new text.
2. **§5 intro sentence:** "still behaves at a 60 s cadence" → "at a 300 s cadence". The task asked only for the first bullet to be reworded.
3. **§6 item 2:** "not up to a minute later" → "not up to five minutes later" (see §4 above).
4. **§6 item 4:** "a 60 s cadence must not freeze the alternation" → "a 300 s cadence" (see §4 above).

Also worth stating, though not deviations: the §4 rewrite quotes the old assumed-rate sentence once in its opening paragraph so a reader can see what was replaced; §1 was not touched and still carries its "not bandwidth, not battery" framing verbatim; §8 gained a clause noting that the consolidation spec's Phase 6a lands with the same beta, which `docs/SPEC-repo-consolidation.md` §5 already states, so this spec agrees with it rather than only pointing at it.

## 7. Not verifiable without hardware

- Every figure in §4 that comes from the overnight chart (64.4 → 68.0 °F at ~3:20–3:30, the ±0.2 °F plateau, the ~1 °F/min burst, the 68 → 72 °F post-off drift) was supplied by the owner from a chart that is not in this repo. It was transcribed, not re-read.
- Whether the staleness dot is failure-driven or age-driven (§5 first bullet) was not checked in `scr_dial.c`; that is a code-review step the spec assigns to the implementer, and this task was spec-only.
- The request counts (~2,880 / ~480 / ~96 over eight hours) and the reduction percentages are arithmetic from the stated cadences, not measurements.
- Whether 15 minutes to `PH_DEGRADED` is noticeable in practice is exactly what §6 item 5 now exists to find out.
