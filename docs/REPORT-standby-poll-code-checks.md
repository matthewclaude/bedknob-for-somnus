# REPORT: SPEC-standby-poll §5 — code checks before implementation

**Date:** 2026-09-11
**Spec:** `docs/SPEC-standby-poll.md` (amended 300 s revision)
**Tree read:** `main` at `6e61f6e` (docs: spec-standby-300s report and its REPORTS.md line)
**Firmware root:** `firmware/dial-idf/` — all paths below are relative to it unless prefixed `docs/`.

## Verdict

**CLEAR TO IMPLEMENT.**

- Question 1: the staleness dot is **failure-driven** (connection phase and the pad's online flag), not age-driven. No threshold moves with the cadence. No spec amendment.
- Question 2: `dial_power_level()` is a spinlock-guarded read designed for cross-task use; the worker is named as an intended reader in the component's own concurrency comment. Nothing extra needed.
- Question 3: nothing in `firmware/` assumes a successful poll within the last ~10 s. Two OTA housekeeping steps inherit the poll cadence and stretch harmlessly; the differential-port spec sizes on the fastest cadence as the spec says.

Two items are bench observations, not code questions (see the last section).

## Gate checks (raw output)

```
$ grep -c 'POLL_STANDBY_US 300000000' docs/SPEC-standby-poll.md
1
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

Both passed.

## Question 1 — the staleness dot

**Premise correction.** The spec points at `scr_dial.c` ~431. On the current tree line 431 is inside `render_value()` (numeral formatting). The staleness dot logic is at **`components/dial_ui/scr_dial.c:726-743`**, inside the dial face's `on_state` render. The object is created at lines 1211-1221. The line number in the spec is from an earlier revision; the mechanism is what was asked about, so this is recorded and investigation continued.

**Code, `components/dial_ui/scr_dial.c:726-743`:**

```c
    // Staleness dot. Night-quiet errors (design-spec.md's "silent staleness
    // at night"): shown at 40% instead of full opacity after dark, so a
    // routine offline blip doesn't glow at 3am.
    lv_obj_set_style_bg_color(s_stale_dot, pal->stale, 0);
    lv_opa_t stale_target = night ? LV_OPA_40 : LV_OPA_COVER;
    bool stale = (st->phase != PH_READY) || !st->device_online;
    if (stale != s_stale_shown) {
        s_stale_shown = stale;
        if (stale) anim_opa(s_stale_dot, LV_OPA_TRANSP, stale_target, 300);
        else       anim_opa(s_stale_dot, lv_obj_get_style_opa(s_stale_dot, 0), LV_OPA_TRANSP, 300);
    } else if (stale) {
        // Still stale, no transition this render — keep the level in sync
        // with a day/night flip (e.g. the worker's dusk palette swap) even
        // without a fresh fade.
        lv_anim_del(s_stale_dot, set_opa_cb);
        lv_obj_set_style_opa(s_stale_dot, stale_target, 0);
    }
```

The dot is `stale = (phase != PH_READY) || !device_online`. There is no timestamp, no elapsed-time comparison, and no threshold anywhere in this path. Both inputs were traced to their writers:

**`st->phase`** — written by the worker's poll result, `main/main.c:1384-1391`:

```c
        if (somnus_refresh_state()) {
            poll_failures = 0;
            dial_state_set_phase(PH_READY, NULL);
            ota_confirm_once();
        } else if (++poll_failures >= 3) {
            dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
        }
        last_poll_us = esp_timer_get_time();
```

`poll_failures` is a counter of **consecutive failed polls**; three in a row flips the phase to `PH_DEGRADED`, one success flips it back. The other non-READY phase reachable in steady state is `PH_WIFI_LOST`, set at `main/main.c:1376-1381` when `dial_wifi_is_connected()` is false at poll time — also a failure condition, not an age.

**`st->device_online`** — written only at `main/main.c:496` inside `mut_device_state()` (the commit applied after a *successful* `/api/state` round-trip):

```c
    st->device_online = d->online;
```

and `d.online` is unconditionally `true` on that path, `main/main.c:695-701`:

```c
static bool somnus_refresh_state(void)
{
    int64_t started_us = esp_timer_get_time();
    somnus_state_t s;
    if (!dial_somnus_get_state(&s)) return false;

    device_snapshot_t d = { .poll_started_us = started_us, .online = true };
```

A failed GET returns before any commit, so `device_online` is never written false by a poll failure; it holds the last-known-good value (consistent with `components/dial_somnus/dial_somnus.h:108`, "a failed poll can never clobber the last-known-good state"). `grep -n device_online main/main.c` finds no other writer. The standby clock face's presence dots (`components/dial_ui/scr_standby.c:122-129`) read the same `device_online` flag and are governed identically.

**Answer: failure-driven.** The dot is a function of the connection phase (a consecutive-failure counter) plus a pad-reported flag. Poll age is never computed.

**Consequence at 300 s:** none for the dot's logic. The dimmed face will not show the dot during a healthy five-minute window, in whole or in part. The only thing that changes is *detection latency*: three consecutive failures at 300 s is ~15 minutes to `PH_DEGRADED` while in STANDBY, versus ~30 s today. That is the "15-minute outage detection" the amended spec §4 already names as the thing to watch, so it is expected, not new. `dial_state_set_phase` on every successful poll is a no-op change from `PH_READY` to `PH_READY` and does not cause a render churn either way.

No spec amendment is forced by this question.

## Question 2 — `dial_power_level()` from `worker_task`

**Definition, `components/dial_power/dial_power.c:399-400, 420-425, 514`:**

```c
static portMUX_TYPE s_lvl_spin = portMUX_INITIALIZER_UNLOCKED;
static dial_power_level_t s_level = DPWR_ACTIVE;
...
static dial_power_level_t level_get(void)
{
    taskENTER_CRITICAL(&s_lvl_spin);
    dial_power_level_t l = s_level;
    taskEXIT_CRITICAL(&s_lvl_spin);
    return l;
}
...
dial_power_level_t dial_power_level(void) { return level_get(); }
```

**State read:** the single enum `s_level`, guarded by the spinlock `s_lvl_spin`.

**Writers of `s_level`, and the task each runs on:**

1. `power_task` (`dial_power.c:430-505`), a dedicated FreeRTOS task created by `dial_power_start()` at `dial_power.c:511` (`xTaskCreate(power_task, "power", 2560, NULL, 2, NULL)`). It recomputes the tier from idle time every 100 ms and writes it under the same spinlock (`dial_power.c:457-459`):
   ```c
           taskENTER_CRITICAL(&s_lvl_spin);
           s_level = want;
           taskEXIT_CRITICAL(&s_lvl_spin);
   ```
2. `dial_power_wake_consumes()` (`dial_power.c:521-531`), called from input handlers — the knob decoder's esp_timer callback and the LVGL task's touch filter — also under the spinlock.

**The component's stated concurrency model, `dial_power.c:390-398`:**

```c
/*
 * Concurrency model (deliberate, after review):
 *  - s_level (the DECIDED level) is guarded by a spinlock so it can be read
 *    and written from the knob decoder's esp_timer callback, the LVGL task's
 *    touch filter, and the worker — none of which may block on a mutex.
 *  - LEDC fades are issued ONLY by power_task. Everyone else just changes the
 *    decision; power_task notices (s_applied != decided, or a forced reapply)
 *    within one 100ms tick. ...
 */
```

"the worker" is listed explicitly as one of the readers the lock was designed for.

**Existing callers and their tasks:**

| Call site | Function | Task |
|---|---|---|
| `main/main.c:187` | `nav_policy()` OTA takeover block | LVGL task |
| `main/main.c:378` | `nav_policy()` passive-screen standby | LVGL task |
| `main/main.c:425` | `nav_policy()` steady-state face pick | LVGL task |
| `components/dial_ui/ui_router.c:176` | `dispatch_tick()` | LVGL task |
| `main/main.c:1201` | `worker_task` idle tick (update-prompt wake edge) | **worker task** |

Task attribution for the OTA nav block: `nav_policy` is declared under the banner at `main/main.c:149`, `/* ---- navigation policy (runs in the LVGL task) ---- */`; it is installed via `ui_router_set_nav_policy(nav_policy)` at `main/main.c:1534` under the comment "Router + screens live in the LVGL task from here on" (line 1531); and `ui_router.c:148-151` says `dispatch_tick` "Runs in the LVGL task every 50ms" and is the only caller of `s_nav_policy` (line 184). So the OTA nav block runs on the LVGL task, not the worker.

The decisive fact for this question is the fifth row: **`worker_task` already calls `dial_power_level()` today**, at `main/main.c:1201`, every idle tick, for the update-prompt wake edge (`ota_pwr_level`). `worker_task` is created at `main/main.c:1591` (`xTaskCreatePinnedToCore(worker_task, "worker", 16384, NULL, 3, NULL, 0)`). The spec's proposed read in the `due` computation is the same call from the same task, a few hundred lines later in the same loop.

**Answer: nothing extra is needed.** No mutex, no snapshot. The read is a spinlock-protected copy of one enum, the writer uses the same spinlock, the component documents the worker as an intended reader, and the worker already does it. The value is a point-in-time sample that can be up to 100 ms behind `power_task`'s next decision, which is irrelevant at a 300 s cadence. The spec §3 rule 3 ("the worker tracks the tier it last saw; on a transition out of STANDBY it treats the poll as overdue") can reuse the existing `ota_pwr_level` sample at line 1201 rather than calling twice per tick; that is an implementation choice, not a correctness requirement.

## Question 3 — anything else assuming a recent poll

All greps run from `firmware/dial-idf/`; `..` is `firmware/`. `firmware/` contains `backups/`, `dial-idf/`, `README.md`.

**Grep A — `POLL_INTERVAL_US` across `firmware/`:**

```
$ grep -rn 'POLL_INTERVAL_US' ..
../dial-idf/main/main.c:61:#define POLL_INTERVAL_US 10000000    // and at most every ~10s when idle
../dial-idf/main/main.c:1372:        int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
[exit 0]
```

One definition, one consumer: the `due` expression the spec itself replaces. Nothing in any component, nothing in `backups/`.

**Grep B — last-poll timestamp readers across `firmware/`:**

```
$ grep -rn 'last_poll\|last_ok\|last_success\|poll_ts\|last_seen\|last_state_us\|poll_age\|since_poll' .. --exclude-dir=build --exclude-dir=managed_components
../dial-idf/main/main.c:952:    // last_poll_us=0/poll_confirms=POLL_CONFIRM_N (below) forces a fast
../dial-idf/main/main.c:1110:    int64_t last_poll_us      = esp_timer_get_time();
../dial-idf/main/main.c:1123:                last_poll_us = 0;                  // read it back now, not in 10s
../dial-idf/main/main.c:1176:            // acts on the command. This used to set last_poll_us = now, which
../dial-idf/main/main.c:1181:            last_poll_us = 0;
../dial-idf/main/main.c:1373:        if (now - last_poll_us < due) continue;
../dial-idf/main/main.c:1380:            last_poll_us = esp_timer_get_time();
../dial-idf/main/main.c:1391:        last_poll_us = esp_timer_get_time();
[exit 0]
```

`last_poll_us` is a `worker_task` local (line 1110). It is never exported: it is not in `app_state_t` (`components/dial_state/dial_state.h:375-…` has no poll timestamp field; the only `_us` timestamp in dial_state is `last_input_us`, the quiet-period input gate), and no component reads it. Its sole reader is the `due` gate at line 1373. The writes at 1123 and 1181 zero it to force an immediate poll after a command; both are unchanged by the spec and continue to work at any cadence.

**Grep B′ — the other per-poll timestamp, `poll_started_us`:**

```
$ grep -rn 'poll_started_us' main components
main/main.c:466:    int64_t poll_started_us;   // when the /api/state round-trip began
main/main.c:478:    bool predates_input = dial_state_last_input_us() > d->poll_started_us;
main/main.c:701:    device_snapshot_t d = { .poll_started_us = started_us, .online = true };
```

Consumer: `mut_device_state()` line 478 compares the poll's start time against the last input time to decide whether to clear optimistic intent. It is an ordering comparison between two events, not an age-against-threshold check. Unaffected.

**Grep C — literal "~10 s" cadence assumptions in code and comments across `firmware/`:**

```
$ grep -rn '10s\b\|10 s\b\|~10\b\|ten seconds\|every 10' .. --exclude-dir=build --exclude-dir=managed_components   (POLL_INTERVAL_US's own comment block at main.c:60-69 filtered out)
../dial-idf/components/dial_power/dial_power.c:158:// is full (PWR_RING_N samples deep, ~10s after boot or after a gap) -- before
../dial-idf/components/dial_power/dial_power.c:286: * lock" warning before STANDBY, clamped to [10s, 30s] so it can never run
../dial-idf/components/dial_power/dial_power.c:288: * immediately, i.e. zero warning) and never drops below ~10s (still a
../dial-idf/components/dial_power/dial_power.c:291: * `want`'s idle comparison runs unconditionally every 100ms regardless of
../dial-idf/components/dial_power/dial_power.c:307:    // a third of 5s is under a second, and the old 10s floor would have
../dial-idf/components/dial_power/dial_power.c:497:        // Plugged-in / on-battery detector (§10.2): every 10th tick of this
../dial-idf/components/dial_state/dial_state.h:559:    // power_task, which reads this live every 100ms tick, same as the
../dial-idf/components/dial_state/dial_state.h:917:// power_task reads this getter fresh every 100ms tick (same as the
../dial-idf/components/dial_ui/scr_settings.c:349:// on every 100ms tick (see dial_power.h's dial_power_brightness_changed
../dial-idf/components/dial_ota/dial_ota.c:62:#define CHECK_BUF_CAP  (64 * 1024)   // release JSON is normally ~10-30KB
../dial-idf/components/dial_somnus/dial_somnus.h:69:                            // 20.0) and a GET ten seconds later still read 20.5 --
../dial-idf/main/main.c:627:// this guards against re-reading the OTA partition state on every ~10s poll
../dial-idf/main/main.c:1123:                last_poll_us = 0;                  // read it back now, not in 10s
../dial-idf/main/main.c:1178:            // zone on left the dial showing the old state for ~10s even though
../dial-idf/main/main.c:1238:            // [10s,30s), the only slice between "settled" and "display about
../dial-idf/main/main.c:1241:            // walked away by the time idle_us cleared 10s), it consumed the
[exit 0]
```

Per-hit judgement:
- `dial_power.c:158, 286-307, 497`; `dial_state.h:559, 917`; `scr_settings.c:349` — power_task's own 100 ms tick, its battery ring buffer, and the dim-before-standby warning window. Idle-time based, nothing to do with the poll.
- `dial_ota.c:62` — a buffer size in KB. Not a cadence.
- `dial_somnus.h:69` — a bench-test narrative ("a GET ten seconds later"). Comment only.
- `main.c:627` — comment on `ota_confirm_once()` guarding against re-reading partition state "every ~10s poll". The guard is a run-once latch; a slower cadence calls it less, never more. Unaffected.
- `main.c:1123, 1178` — comments describing the post-command `last_poll_us = 0` fast path. That path bypasses `due` entirely and keeps working.
- `main.c:1238, 1241` — the update-prompt wake edge's `[10s, 30s)` idle-time slice. Idle-input based, not poll based. Unaffected.

None of these is an age-of-poll assumption.

**Grep D — `poll` in the UI, somnus and state components (readers of the poll notion):**

```
$ grep -rn -i 'poll' components/dial_ui components/dial_somnus/dial_somnus.h components/dial_state/dial_state.h
components/dial_ui/scr_wifi.c:37/101/261/266   — an LVGL lv_timer that re-reads Wi-Fi RSSI every 2 s; local, unrelated
components/dial_ui/scr_netpick.c:13/15/28/120/149/154/189/194 — an lv_timer polling the scan result every 500 ms; local, unrelated
components/dial_ui/scr_connecting.c:117        — comment: phase leaves CONNECTING "on a poll or a 30s fallback timer"; failure/phase driven
components/dial_ui/scr_update.c:271            — comment: a background poll must not blow away the in-progress check; a commit-ordering concern, not an age
components/dial_ui/scr_dial.c:7/54/553/571/795/947/1350/1492 — comments: "the poll reconciles later", "the knob wins" over a commit; all event-driven on the commit itself, none timed
components/dial_ui/ui_router.c:18              — the router polls dial_power_level() (see Q2), not the pad
components/dial_somnus/dial_somnus.h:108/117   — "a failed poll can never clobber last-known-good state" — supports Q1
components/dial_state/dial_state.h:10/19/33/857 — generation counter re-render on commit; 2.5 s quiet gate; PH_READY definition; optimistic intent reverted "on the next poll"
[exit 0]
```

Every UI consumer reacts to the *commit* of a poll (via the generation counter, `dial_state.h:10`), not to its recency. A dial face that receives no commit for 300 s simply does not re-render the water reading; nothing times out.

**Cadence-inheriting code after the `due` gate.** The gate at `main/main.c:1371-1373` is a `continue`, so everything below it in the loop body runs only when a poll actually runs. Two blocks live there besides the poll:

1. **OTA_FAILED auto-clear**, `main/main.c:1393-1415` — "Re-checked at this same idle cadence" (its own comment). `dial_ota_clear_stale_failure(OTA_FAILED_AUTOCLEAR_US)` with a 25 s threshold. At 300 s in STANDBY the wedge-clearing latency stretches from ≤35 s to ≤325 s. It is a belt-and-suspenders path (the screen's own `CMD_OTA_CLEAR_FAILED` on teardown is the primary); the stretch is harmless and only applies while the screen is asleep. Does not assume a recent poll.
2. **OTA auto-check**, `main/main.c:1425-1470` — a 6 h timer (`OTA_AUTOCHECK_INTERVAL_US`, line 83) plus a per-device minute-of-hour offset gate. At 300 s granularity the gate `(now_min_local % 60) >= (offset % 60)` is still hit within the hour it becomes true. Does not assume a recent poll.

The blocks that *do* run every 300 ms tick regardless — night-window flip, update-prompt wake edge, auto-update install decision (`main/main.c:1216-1366`, all inside the `dial_time_now()` branch that closes before the gate) — are unaffected by definition. Note the auto-update install gate requires `st.phase == PH_READY` (line 1306); at 300 s an outage takes ~15 min instead of ~30 s to withdraw that eligibility, which is inside the design's 30-minute idle requirement (`auto_idle_us >= 30 min`, line 1307) and not a hazard.

**Differential port spec, `docs/SPEC-differential-firmware-port.md:156-165`:** §4.1 "Buffer capacity must be sized on the fastest cadence … Capacity is `ceil(600s / POLL_CONFIRM_US) + margin`". Sized on the 2 s confirm cadence, so a slower idle cadence only under-fills it. Confirmed unaffected, as the spec claims. Line 195 provisionally computes a `gapThreshold` against `POLL_INTERVAL_US`; that is an unported design note, not firmware, and the port's author will need to decide whether a gap threshold should be the STANDBY cadence — noted for that spec, out of scope here.

**Negative results worth recording:**
- No component outside `main/` references `POLL_INTERVAL_US`, `last_poll_us`, or any poll-age quantity.
- `app_state_t` carries no poll timestamp, so no screen *can* compute poll age.
- `firmware/backups/` contains no `POLL_INTERVAL_US` hit.

## No source file modified

Run after the report was written and before the index line was added:

```
$ git --no-optional-locks status --short
?? docs/REPORT-standby-poll-code-checks.md
$ git diff --stat
(no output — nothing tracked changed)
```

After the `docs/REPORTS.md` line was added the diff-stat is `docs/REPORTS.md | 1 +` plus the untracked report; both are committed together as this task's only commit.

## Deviations

1. **Spec line number.** The spec's `scr_dial.c` ~431 is stale; the dot lives at 726-743 on the current tree. The task said to verify rather than trust it, so this is a recorded correction, not a workaround. The spec's §5 bullet could be updated to say "`scr_dial.c` `on_state`, the `s_stale_dot` block" so it stops drifting; not done here (read-only).
2. **`--exclude-dir` on greps B and C.** Added `--exclude-dir=build --exclude-dir=managed_components` to keep vendored and generated trees out of the timestamp and comment sweeps. Grep A ran with no excludes, as specified, and its result stands for the whole of `firmware/`.
3. **Grep D.** Not requested by name; run to give the "readers of the poll notion" question a genuine negative result in the UI components rather than an argument from silence.

No other deviations. No build, no flash, no version bump, no tag, no push, no source edit.

## Needs the bench, not reading

- **Outage detection latency at ~15 min.** Reading confirms three consecutive failures at the STANDBY cadence is what raises the dot. Whether 15 minutes without a dot on a sleeping bedside dial is acceptable is a judgement the amended spec §4/§6 already assigns to the soak, not to this review.
- **Water reading age on the dimmed face.** Not a mechanism, but a user-visible consequence: with the Temperature standby face (the default, `standby_screen()` at `main/main.c:158`) the dial face stays on screen dimmed and its water numeral will be up to ~300 s old. Nothing in the code flags it stale. The spec's §3 rule 3 wake-poll (~302.5 s worst case) is the mitigation; confirm on the bench that a glance at the dimmed face after a bed state change does not read as "the dial is frozen".
- **`ota_pwr_level` reuse.** Whether the implementation samples `dial_power_level()` once per tick (reusing line 1201) or twice is invisible to reading and only matters for tidiness; note it in the implementation PR.
