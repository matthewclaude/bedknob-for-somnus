# REPORT: SPEC-standby-poll §6 item 6 — unattended overnight capture, ANALYSIS (somnus-v1.0.2 pre-release build)

**Verdict: PASS.** One reader attach, no loss, 10 h 35 m 43 s of continuous coverage; 137 polls, of which 126 STANDBY-tier gaps sit at 300.0–301.6 s (mean 300.5 s) with no outlier; the pad's own schedule moved the setpoint four times (18 → 20 → 19 → 20 °C, then off / 18 °C) and each step was on the dial at the next poll, at most 300.4–300.7 s after the previous one; no assert / panic / abort / Guru / `E (` line; no `ESP-ROM` or `rst:0x`, so the dial ran the whole night on one boot. The tag gate for `somnus-v1.0.2-beta.1` is met on the evidence in `bench-logs/1.0.2-overnight.log`. Tagging is the owner's gated step and was not done.

Date: 2026-09-12. Branch `main` at `397419e`, `PROJECT_VER` `1.0.1`, firmware commit `39af8f5` as flashed by wire in `REPORT-1.0.2-bench-cadence.md`, still running from its 2026-09-11 boot (≈ 19:50:08Z; tick → UTC is boot + tick, good to ±2 s; owner's clock is CDT, UTC−5). Capture started by `REPORT-1.0.2-overnight-start.md`. No flash, no pad write, no bump, no tag, no push.

## 1. Gate

```
$ git --no-optional-locks status --short --untracked-files=no
(no output, exit 0)
$ grep -c 'POLL_STANDBY_US 300000000' firmware/dial-idf/main/main.c
1
$ ls -l bench-logs/1.0.2-overnight.log
-rw-r--r--  1 matthew  staff  8469 Sep 12 07:14 bench-logs/1.0.2-overnight.log
$ ps -p 25176,25166 -o pid,stat,etime,comm          (before the kill, 12:17:17Z)
  PID STAT  ELAPSED COMM
25166 SN   10:35:41 caffeinate
25176 SN   10:35:29 bash
$ pgrep -fl "cat /dev/cu.usbmodem83401"
25181 cat /dev/cu.usbmodem83401
```

All three processes from the start report were still alive at analysis time; the log's mtime (07:14 CDT = 12:14Z) matches the last poll.

## 2. Step 1 — stop the capture

```
$ kill 25176            # reader loop        at 2026-09-12T12:17:31Z
$ kill 25181            # its cat child (found by pgrep, not assumed)
$ kill 25166            # caffeinate
$ ps -p 25166,25176,25181 -o pid,comm
  PID COMM
(exit 1 — none running)
$ pgrep -fl "cat /dev/cu.usbmodem83401"
no cat reader
```

Log and pid files left in place. `bench-logs/` is git-ignored; the log is not committed.

## 3. Step 2 — size and span

```
$ wc -l -c bench-logs/1.0.2-overnight.log
     161    8469 bench-logs/1.0.2-overnight.log
```

| | |
|---|---|
| Reader attached | 2026-09-12T01:41:48Z |
| Reader killed | 2026-09-12T12:17:31Z |
| Wall-clock span | 10 h 35 m 43 s (38 143 s) |
| First `side A:` tick | 21101430 (≈ 01:41:49Z) |
| Last `side A:` tick | 59074573 (≈ 12:14:42Z) |
| Tick span, first → last poll | 37 973.1 s = 10 h 32 m 53 s |
| Kill − last poll | 2 m 48 s, less than one STANDBY interval |

Wall-clock span minus tick span is 169.9 s, and the kill fell 168 s after the last poll: the two agree to within 2 s, as they must when coverage is continuous.

## 4. Step 3 — coverage

```
$ grep -n '=== reader' bench-logs/1.0.2-overnight.log
1:=== reader attached 2026-09-12T01:41:48Z ===
```

**Continuous.** Exactly one attach marker, no `=== reader lost device ===` marker, and no gap in the poll ticks (§6). The Mac did not sleep or reboot and the dial never dropped off the bus.

## 5. Step 4 — cadence lines

```
$ grep -n 'poll: ' bench-logs/1.0.2-overnight.log
7:I (21148306) app: poll: standby cadence 300s
22:I (23855811) app: poll: active cadence 10s
30:I (23916004) app: poll: standby cadence 300s
```

Three tier changes all night, each a real one:

- Line 7, 01:42:36Z: the display timed out 48 s after the capture attached (it was awake at attach, §3 of the start report); STANDBY.
- Line 22, 02:27:43Z (21:27 CDT): one wake. No knob, `POST /api/` or other input line accompanies it, and the pad state is identical on both sides of it (`on=1 set=18.0C water=17.6C`), so it was a touch wake with no write — consistent with the owner going to bed. The wake poll landed 2.96 s after the cadence line (23855811 → 23858769), the same shape as item 2 on the bench.
- Line 30, 02:28:44Z: back to STANDBY 60.2 s later (owner's 60 s screen timeout), followed by no further wake until the kill.

No cadence line appears anywhere except at these entries and exits.

## 6. Step 5 — inter-poll gaps

Gaps between consecutive `side A:` ticks, grouped by the tier in force when the later poll was due:

| Segment | Lines | Tier | n | min | max | mean |
|---|---|---|---|---|---|---|
| attach → timeout | 2–6 | ACTIVE | 4 | 10.3 s | 10.8 s | 10.6 s |
| first STANDBY run | 8–21 | STANDBY | 9 | 300.1 s | 301.1 s | 300.5 s |
| wake | 23–28 | ACTIVE | 6 | 10.2 s | 11.1 s | 10.6 s |
| second STANDBY run → kill | 31–161 | STANDBY | 117 | 300.0 s | 301.6 s | 300.5 s |

All 126 STANDBY gaps lie in 300.0–301.6 s; none falls outside 299–302 s. The bench saw 300.4–300.8 s over six gaps; over 126 the spread widens by under a second and the mean is the same. The 0.5 s over 300.0 is the poll's own round-trip, as in `REPORT-1.0.2-bench-cadence.md`. No wedge, no runaway, no drift.

## 7. Step 6 — setpoint tracking

Every change of `on=` or `set=` between consecutive polls. All four happened in STANDBY; none was made by the dial (no `POST /api/`, no knob line in the log).

| # | Change | Last poll showing the old value | First poll showing the new value | Gap | Picked up within one 300 s interval |
|---|---|---|---|---|---|
| 1 | on, 18.0 → 20.0 °C | 41944710 (07:29:12Z / 02:29 CDT) | 42245087 (07:34:13Z / 02:34 CDT) | 300.4 s | yes |
| 2 | on, 20.0 → 19.0 °C | 45851340 (08:34:19Z / 03:34 CDT) | 46152074 (08:39:20Z / 03:39 CDT) | 300.7 s | yes |
| 3 | on, 19.0 → 20.0 °C | 56369378 (11:29:37Z / 06:29 CDT) | 56669770 (11:34:37Z / 06:34 CDT) | 300.4 s | yes |
| 4 | on 20.0 °C → **off** 18.0 °C | 58172739 (11:59:40Z / 06:59 CDT) | 58473462 (12:04:41Z / 07:04 CDT) | 300.7 s | yes |

```
96:I (42245087) app: side A: on=1 set=20.0C water=17.9C
111:I (46152074) app: side A: on=1 set=19.0C water=19.9C
150:I (56669770) app: side A: on=1 set=20.0C water=19.7C
159:I (58473462) app: side A: on=0 set=18.0C water=21.2C
```

Each change occurred somewhere inside the 300.4–300.7 s window between the two quoted polls, so the dial's latency on each step is bounded above by that gap. The bound exceeds 300.0 s by the same 0.4–0.7 s poll round-trip that §6 item 1 already accepted as "~300 s"; there is no case where a step waited for a second poll. Water temperature tracks each step (rising to 19.9 °C after step 1, settling at 19.0–19.1 °C after step 2, 20.6 °C after step 3, then 21.2 → 21.9 °C as the bed idles off). Steps 1–3 are the pad's multi-stage schedule; step 4 is its end-of-schedule off, four minutes after the dial's own `power: night mode off` at 07:00 CDT — two different schedules, both landing where expected.

## 8. Step 7 — errors

```
$ grep -nE 'assert|panic|abort|Guru|E \(' bench-logs/1.0.2-overnight.log
(no output)
```

None. For completeness, every non-poll, non-marker, non-cadence line of the night:

```
9:I (21620389) dial_time: SNTP time synced
11:I (21744608) app: OTA auto-check window offset: +56min past 10:00
13:I (22046868) ota: latest 1.0.1, running 1.0.1 -- up to date
14:I (22189669) power: night mode on
15:W (22189712) ledc: LEDC FADE TOO SLOW
29:W (23915712) ledc: LEDC FADE TOO SLOW
35:I (25220702) dial_time: SNTP time synced
48:I (28820759) dial_time: SNTP time synced
61:I (32421005) dial_time: SNTP time synced
74:I (36021082) dial_time: SNTP time synced
87:I (39621127) dial_time: SNTP time synced
100:I (43221196) dial_time: SNTP time synced
103:I (43749839) ota: latest 1.0.1, running 1.0.1 -- up to date
114:I (46821282) dial_time: SNTP time synced
127:I (50421383) dial_time: SNTP time synced
134:I (52406820) wifi:bcn_timeout,ap_probe_send_start
141:I (54021583) dial_time: SNTP time synced
154:I (57621874) dial_time: SNTP time synced
157:I (58190739) power: night mode off
158:W (58190812) ledc: LEDC FADE TOO SLOW
```

Hourly SNTP syncs, two OTA checks (both "up to date" against the published 1.0.1 — the running build is 1.0.2 code at `PROJECT_VER` 1.0.1, so this is expected), the dial's night mode on at 21:00 and off at 07:00 CDT, three `LEDC FADE TOO SLOW` warnings at the backlight fades for night-mode on, wake → STANDBY, and night-mode off (a pre-existing warning, unchanged by 1.0.2, not an error), and one Wi-Fi beacon timeout at 05:23 CDT that the polls straddle without a missed or late poll (gaps 300.4 s on either side).

## 9. Step 8 — resets

```
$ grep -nE 'ESP-ROM|rst:0x' bench-logs/1.0.2-overnight.log
(no output)
```

None. The tick counter runs monotonically from 21101430 to 59074573; the dial's uptime at the last poll is 16 h 24 m 35 s, spanning the 2026-09-11 bench work and this whole night on one boot. The night is not void.

## 10. Step 9 — bookends

```
2:I (21101430) app: side A: on=1 set=18.0C water=19.5C
161:I (59074573) app: side A: on=0 set=18.0C water=21.9C
```

## 11. Deviations

1. **The dial was woken once, at 21:27 CDT**, by a touch with no pad write (§5). The start report asked the owner to leave the dial alone; a single wake in real use is within what item 6 is meant to exercise (the ACTIVE ↔ STANDBY edge under a real night) and it did not alter the pad. Recorded, not counted against the night.
2. **The pickup bound is 300.4–300.7 s, not ≤ 300.0 s** (§7). This is the poll round-trip on top of the 300 s timer, identical to the cadence measurement item 1 accepted. Stated so the reader can disagree; the verdict treats "within the cadence" as within one interval.

No other deviations. Pad end state: **off, 18.0 °C**, water 21.9 °C — the pad's own schedule left it there; nothing to restore.

## 12. What remains before the tag

§6 items 4, 5 and 7 of `docs/SPEC-standby-poll.md` are outside this task and were not run here. Item 6 is PASS. The version bump to 1.0.2, the tag `somnus-v1.0.2-beta.1` and the push to `somnus` are the owner's gated steps per `somnus-release-procedure`.
