# REPORT: SPEC-standby-poll §6 item 6 — unattended overnight capture, analysis (somnus-v1.0.2 pre-release build)

**Verdict: PASS.** The pad's own schedule was tracked on the dial through an unattended night at the 300 s cadence: four pad-originated changes (18 → 20 → 19 → 20 °C, then off / 18 °C), each on the dial at the very next poll, 300.4–300.7 s after the last poll that showed the old state; 126 STANDBY-tier gaps at 300.0–301.6 s (mean 300.5 s); one reader attach, no loss, 10 h 35 m 43 s continuous; no assert / panic / abort / Guru / `E (`; no `ESP-ROM` / `rst:0x`, uptime monotonic from a boot that matches the 2026-09-11 19:50Z flash. Morning log clean. Items 4 and 5 of §6 remain before the tag; bump, tag and push are the owner's.

Date: 2026-09-12. Branch `main` at `0abf47e`, `PROJECT_VER` `1.0.1`, firmware commit `39af8f5` as flashed by wire in `REPORT-1.0.2-bench-cadence.md`. Capture started by `REPORT-1.0.2-overnight-start.md`. No code change, no flash, no pad write, no bump, no tag, no push. Owner's clock is CDT (UTC−5). This report supersedes `REPORT-1.0.2-overnight.md` (committed at `0abf47e` from the start report's recorded morning task before this fuller task arrived; same log, same conclusion, see deviation 1).

## 1. Gate — raw output

```
$ ls bench-logs/1.0.2-overnight.log
bench-logs/1.0.2-overnight.log
$ git --no-optional-locks status --short --untracked-files=no
(no output, exit 0)
```

Both passed.

## 2. Step 1 — stopping the capture

The capture had already been stopped at 12:17:31Z by the start report's morning task (reader loop 25176 first, then its `cat` child 25181 found by `pgrep`, then caffeinate 25166; `ps` confirmed all three gone). The prescribed block was then run as written and reports the processes already gone:

```
$ ps -p 25176,25166 -o pid,comm
  PID COMM
(exit 1 — none running)
$ kill $(cat /tmp/serial_cap_overnight.pid) 2>&1; echo "reader kill exit=$?"
kill 25176 failed: no such process
reader kill exit=1
$ kill $(cat /tmp/caffeinate_overnight.pid) 2>&1; echo "caffeinate kill exit=$?"
kill 25166 failed: no such process
caffeinate kill exit=1
$ sleep 2
$ ps -p $(cat /tmp/serial_cap_overnight.pid) >/dev/null && echo "READER STILL ALIVE" || echo "reader stopped"
reader stopped
$ wc -l -c bench-logs/1.0.2-overnight.log
     161    8469 bench-logs/1.0.2-overnight.log
```

Killed, in order: reader loop 25176, its `cat /dev/cu.usbmodem83401` child 25181, caffeinate 25166. Nothing was forced. The log has not grown since (still 161 lines / 8469 bytes; mtime 07:14 CDT = 12:14Z, the last poll).

## 3. Step 2 — analysis of `bench-logs/1.0.2-overnight.log`

Every figure below was recomputed from the file by script in this task.

### (a) Size, span, boot time

| | |
|---|---|
| Lines / bytes | 161 / 8469 |
| Reader attached (marker) | 2026-09-12T01:41:48Z |
| Reader killed | 2026-09-12T12:17:31Z |
| Wall-clock span covered | 10 h 35 m 43 s (38 143 s) |
| First `side A:` tick | 21101430 |
| Last `side A:` tick | 59074573 |
| Tick span, first → last poll | 37 973.1 s = 10 h 32 m 53 s |
| Kill − last poll | 2 m 48 s (< one STANDBY interval; the next poll was not yet due) |

**Boot time derived from the file:** the first poll (tick 21101430 = 5 h 51 m 41.4 s of uptime) landed between the attach marker at 01:41:48Z and the second poll, which was already in the log when `wc` ran 20 s after launch, so it arrived no later than ≈ +9.4 s. Boot is therefore between **2026-09-11T19:50:06.6Z and 19:50:16.0Z**. That is consistent with the 2026-09-11 19:50Z wire flash and with the 19:50:08Z anchor the two bench reports used. Uptime at the last poll is 16 h 24 m 35 s, so the dial ran the bench items of 2026-09-11 and this entire night on one boot; a reboot would have restarted the tick.

### (b) Reader markers

```
1:=== reader attached 2026-09-12T01:41:48Z ===
```

That is the only marker in the file: one attach, no `=== reader lost device ===`. **Coverage was continuous** — no gaps. Independently, wall-clock span minus tick span is 169.9 s and the kill fell 168.4 s after the last poll: they agree within 2 s, which is only possible if no polls were lost.

### (c) Cadence lines

```
7:I (21148306) app: poll: standby cadence 300s
22:I (23855811) app: poll: active cadence 10s
30:I (23916004) app: poll: standby cadence 300s
```

Three tier changes all night, at 01:42:36Z (screen timeout 48 s after attach — the dial was awake when the capture began), 02:27:43Z (one wake, see (e)) and 02:28:44Z (back to STANDBY 60.2 s later, the owner's 60 s screen timeout). No cadence line appears anywhere else.

### (d) STANDBY-tier cadence

Gaps between consecutive `side A:` lines, excluding the stretch from the `active cadence 10s` line to the next `standby cadence 300s` line (and the pre-timeout ACTIVE stretch at the top of the log):

| Run | Lines | Gaps | Min | Max | Mean |
|---|---|---|---|---|---|
| STANDBY run 1 | 8–21 | 9 | 300.1 s | 301.1 s | 300.5 s |
| STANDBY run 2 | 31–161 | 117 | 300.0 s | 301.6 s | 300.5 s |
| **All STANDBY** | | **126** | **300.0 s** | **301.6 s** | **300.5 s** |

None outside 299–302 s. **They sit at ~300 s**: the timer is 300 s and the 0.5 s mean excess is the poll's HTTP round-trip, the same figure `REPORT-1.0.2-bench-cadence.md` measured over six gaps (300.4–300.8 s). No wedge, no runaway, no drift over ten and a half hours.

### (e) The ACTIVE stretch

Two ACTIVE stretches: lines 2–6 (the awake dial at attach, 4 gaps of 10.3–10.8 s, until the 01:42:36Z timeout) and the one wake:

```
21:I (23848611) app: side A: on=1 set=18.0C water=17.6C      ← last STANDBY poll before the wake
22:I (23855811) app: poll: active cadence 10s                 ← 02:27:43Z (21:27:43 CDT)
23:I (23858769) app: side A: on=1 set=18.0C water=17.6C      ← wake poll, 2.96 s after the cadence line
24:I (23869637) app: side A: on=1 set=18.0C water=17.6C
25:I (23880744) app: side A: on=1 set=18.0C water=17.6C
26:I (23891008) app: side A: on=1 set=18.0C water=17.7C
27:I (23901839) app: side A: on=1 set=18.0C water=17.6C
28:I (23912104) app: side A: on=1 set=18.0C water=17.7C
29:W (23915712) ledc: LEDC FADE TOO SLOW
30:I (23916004) app: poll: standby cadence 300s               ← 60.2 s after the wake
```

Six ACTIVE gaps of 10.2–11.1 s (mean 10.6 s). **What woke it:** the log does not say — the firmware logs the tier change, not the touch or knob event behind it, and no knob or write line accompanies it. The pad state is identical on both sides (`on=1 set=18.0C`, water 17.6–17.7 °C), and

```
$ grep -c 'POST /api/' bench-logs/1.0.2-overnight.log
0
```

so **the dial wrote nothing** all night. A touch at 21:27 CDT with the display returning to STANDBY after the timeout is consistent with the owner going to bed and looking at the dial.

### (f) Pad schedule changes tracked on the dial — the core of item 6

Every change of `on=` or `set=` between consecutive polls. All four occurred in STANDBY, none was written by the dial ((e)), so they are the pad's own schedule.

| # | Change | Last poll, OLD state | First poll, NEW state | Elapsed | Within one 300 s interval |
|---|---|---|---|---|---|
| 1 | 18.0 → 20.0 °C, on | 41944710 `on=1 set=18.0C water=17.9C` (07:29:12Z / 02:29 CDT) | 42245087 `on=1 set=20.0C water=17.9C` (07:34:13Z / 02:34 CDT) | 300.4 s | **yes** |
| 2 | 20.0 → 19.0 °C, on | 45851340 `on=1 set=20.0C water=19.9C` (08:34:19Z / 03:34 CDT) | 46152074 `on=1 set=19.0C water=19.9C` (08:39:20Z / 03:39 CDT) | 300.7 s | **yes** |
| 3 | 19.0 → 20.0 °C, on | 56369378 `on=1 set=19.0C water=19.0C` (11:29:37Z / 06:29 CDT) | 56669770 `on=1 set=20.0C water=19.7C` (11:34:37Z / 06:34 CDT) | 300.4 s | **yes** |
| 4 | on 20.0 °C → **off**, 18.0 °C | 58172739 `on=1 set=20.0C water=20.6C` (11:59:40Z / 06:59 CDT) | 58473462 `on=0 set=18.0C water=21.2C` (12:04:41Z / 07:04 CDT) | 300.7 s | **yes** |

Each change happened somewhere inside the 300.4–300.7 s window between the two quoted polls, so each was on the dial no later than one poll interval after the pad made it; no step waited for a second poll. The interval exceeds 300.0 s only by the poll round-trip already accepted in item 1. Water temperature follows each step (to 19.9 °C after step 1, 19.0–19.1 °C after step 2, 20.6 °C after step 3, then drifting up to 21.9 °C as the bed idles off), which is the pad acting on the same setpoints the dial displayed. Step 4 is the schedule's end-of-night off, four minutes after the dial's own `power: night mode off` at 07:00 CDT — two independent schedules both landing as expected.

### (g) Errors

```
$ grep -nE 'assert|panic|abort|Guru|E \(' bench-logs/1.0.2-overnight.log
(no output)
```

**None.**

### (h) Resets

```
$ grep -nE 'ESP-ROM|rst:0x' bench-logs/1.0.2-overnight.log
(no output)
```

**None.** Ticks are monotonic across all 161 lines ((a)). The night is not void.

### (i) Warnings

```
15:W (22189712) ledc: LEDC FADE TOO SLOW
29:W (23915712) ledc: LEDC FADE TOO SLOW
158:W (58190812) ledc: LEDC FADE TOO SLOW
```

All three are the same ESP-IDF `ledc` driver warning, each 40–300 ms after a backlight transition: line 15 at `power: night mode on` (21:00 CDT), line 29 as the wake's 60 s timeout dimmed the screen back to STANDBY, line 158 at `power: night mode off` (07:00 CDT). It means the requested fade duration was longer than the LEDC hardware fade can express at that step count, so the driver clamps it; the fade completes, nothing is skipped. **Benign** — pre-existing behaviour of the 1.0.1 backlight code, not touched by `39af8f5`, no effect on polling (the neighbouring gaps are 300.4 s, 300.3 s / 10.3 s and 300.7 s). No other `W ` lines.

### (j) OTA auto-check

```
11:I (21744608) app: OTA auto-check window offset: +56min past 10:00
13:I (22046868) ota: latest 1.0.1, running 1.0.1 -- up to date
103:I (43749839) ota: latest 1.0.1, running 1.0.1 -- up to date
```

Two unattended checks fired, at 01:57:35Z (20:57 CDT) and 07:59:17Z (02:59 CDT), ≈ 6 h 1 m apart, and both found 1.0.1 as expected (1.0.1 is the newest published release; the running build reports `PROJECT_VER` 1.0.1). **Both ran while the tier was STANDBY** — the first 15 min after the 01:42:36Z timeout with no ACTIVE stretch in between, the second in the middle of STANDBY run 2 — and neither disturbed the poll rhythm (gaps of 300.1 s and 300.4 s around them). That is the observable half of §6 item 7: the check does fire in STANDBY on the 1.0.2 build. The offset line (a per-device stagger derived from the Wi-Fi MAC, per `main.c`) is printed once at the first check.

### (k) Wi-Fi events

```
133:I (52162753) app: side A: on=1 set=19.0C water=19.0C
134:I (52406820) wifi:bcn_timeout,ap_probe_send_start
135:I (52463161) app: side A: on=1 set=19.0C water=19.0C
136:I (52763964) app: side A: on=1 set=19.0C water=19.0C
```

One beacon timeout at 10:23:34Z (05:23 CDT); the station probed the AP and stayed associated (no disconnect / reconnect / got-IP line follows). Polls straddle it at 300.4 s and 300.8 s. **Poll rhythm not interrupted.**

### (l) Bookends

```
2:I (21101430) app: side A: on=1 set=18.0C water=19.5C
161:I (59074573) app: side A: on=0 set=18.0C water=21.9C
```

Start (01:41:49Z): pad **on, 18.0 °C**, water 19.5 °C cooling toward it. End (12:14:42Z): pad **off, 18.0 °C**, water 21.9 °C — left there by the pad's own schedule at step 4; nothing to restore.

Remaining non-poll lines, for completeness: eleven `dial_time: SNTP time synced` at exactly 3600 s intervals, `power: night mode on` at 01:59:57Z and `power: night mode off` at 11:59:58Z (the dial's 21:00–07:00 CDT night palette).

## 4. Step 3 — verdict on §6 item 6

**PASS.** Against what the spec asks: the pad's own multi-stage schedule was tracked on the dial through an unattended night at the 300 s STANDBY cadence — four pad-originated changes, each displayed at the next poll ((f)); the cadence held at 300.0–301.6 s over 126 intervals with no wedge ((d)); the dial did not reset ((h)) and did not write to the pad ((e)); the morning log is clean ((g), (i)). Coverage was continuous ((b)), so the log is the whole night, not a sample of it.

**What remains before the tag:** §6 items 4 and 5 of `docs/SPEC-standby-poll.md`. Item 7's OTA half was observed incidentally here ((j)) but was not the subject of this task. The 1.0.2 version bump, the `somnus-v1.0.2-beta.1` tag and the push to `somnus` are the owner's gated steps and were not done.

## 5. Deviations

1. **The analysis had already been run once before this task's text arrived.** The start report's §6 recorded a morning task; that was executed at 12:17Z, the processes killed in the order reader → `cat` child → caffeinate, and its result committed as `docs/REPORT-1.0.2-overnight.md` at `0abf47e`. This task's prescribed kill block was then run as written and found nothing to kill (§2). Every number in this report was recomputed from the file, not carried over; the two reports agree. The earlier report carries a one-line pointer to this one as the report of record.
2. **The pickup bound is 300.4–300.7 s, not ≤ 300.0 s** ((f)). The excess is the poll round-trip on top of the 300 s timer, identical to what item 1 accepted as "~300 s". Stated so the reader can disagree; the verdict reads "within one cadence interval" as within one poll-to-poll gap.
3. **One wake at 21:27 CDT** ((e)), presumably the owner's touch, against the start report's "leave the dial alone". It wrote nothing and changed no pad state; a single real-use wake is within what an unattended night is meant to exercise. Recorded, not counted against the night.

## 6. What the log cannot settle

- **What caused the 02:27:43Z wake.** The firmware logs the cadence change, not the touch/knob event. Pad state unchanged and no `POST /api/`, so it was not a write; the actor is inferred, not recorded.
- **Exactly when within each 300 s window the pad made each change.** Only the poll-to-poll bound is observable ((f)); the true latency is somewhere in 0–300.7 s per step.
- **The OTA auto-check period.** Two checks ≈ 6 h apart is what the file shows; the schedule behind it is in `main.c`, not in the log, and is item 7's business.
- **The boot instant to better than ±5 s** ((a)); it is bounded to 19:50:06.6–19:50:16.0Z and that suffices for the reset question.
