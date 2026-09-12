# REPORT: SPEC-standby-poll §6 item 6 — unattended overnight capture, START (somnus-v1.0.2 pre-release build)

**Verdict: STARTED.** caffeinate pid 25166, reader pid 25176 (its `cat` child is pid 25181), log `bench-logs/1.0.2-overnight.log`, reader attached 2026-09-12T01:41:48Z, alive check passed at 2026-09-12T01:42:08Z with the log at 3 lines / 153 bytes and growing (5 lines / 261 bytes one poll later). Nothing flashed, no pad write, no version bump, no tag, no push. The morning task in §6 below reads the night and decides the gate.

Date: 2026-09-11 (owner's clock, CDT, UTC−5); capture start 2026-09-12 01:41Z. Branch `main` at `9c31813`, `PROJECT_VER` `1.0.1`, firmware commit `39af8f5` as flashed by wire in `REPORT-1.0.2-bench-cadence.md` and still running (tick 21101430 ms = 5 h 51 m 41 s of uptime; boot ≈ 19:50:08Z on 2026-09-11 from that report's anchor, which puts the first captured poll at ≈ 01:41:49Z, matching the attach marker to within a second). Items 1, 2 and 3 of §6 passed on the bench (`REPORT-1.0.2-bench-cadence.md`, `REPORT-1.0.2-bench-items-2-3.md`). Item 6 is the gate for `somnus-v1.0.2-beta.1`: the earlier overnight proof ran at the 10 s cadence and does not carry over to 300 s.

## 1. Gate — raw output of all four checks

```
$ grep -c 'POLL_STANDBY_US 300000000' firmware/dial-idf/main/main.c
1
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls bench-logs/1.0.2-overnight.log 2>/dev/null
(no output, exit 1 — file did not exist)
$ git --no-optional-locks status --short --untracked-files=no
(no output, exit 0)
```

All four passed. `bench-logs/` is in `.gitignore` (line 28), so the log stays out of the tree.

## 2. Commands as run and their output

### Step 1 — keep the Mac awake

```
$ ls -d bench-logs && nohup caffeinate -i -s >/dev/null 2>&1 &
$ echo $! > /tmp/caffeinate_overnight.pid
$ echo "caffeinate pid=$(cat /tmp/caffeinate_overnight.pid)"
caffeinate pid=25166
$ ps -p $(cat /tmp/caffeinate_overnight.pid) -o pid,command
  PID COMMAND
25166 caffeinate -i -s
```

### Step 2 — detached, appending, reattaching reader

```
$ cd /Users/matthew/Projects/somnus-waveshare-rotary-dial
$ DEV=/dev/cu.usbmodem83401
$ LOG=$PWD/bench-logs/1.0.2-overnight.log
$ nohup bash -c '
    while true; do
      stty -f '"$DEV"' 115200 raw 2>/dev/null
      echo "=== reader attached $(date -u +%FT%TZ) ===" >> '"$LOG"'
      cat '"$DEV"' >> '"$LOG"'
      echo "=== reader lost device $(date -u +%FT%TZ) ===" >> '"$LOG"'
      sleep 5
    done
  ' >/dev/null 2>&1 &
$ echo $! > /tmp/serial_cap_overnight.pid
$ sleep 20
$ echo "reader pid=$(cat /tmp/serial_cap_overnight.pid) started $(date -u +%FT%TZ)"
reader pid=25176 started 2026-09-12T01:42:08Z
$ ps -p $(cat /tmp/serial_cap_overnight.pid) >/dev/null && echo "reader alive" || echo "READER DIED"
reader alive
$ wc -l -c "$LOG"
       3     153 /Users/matthew/Projects/somnus-waveshare-rotary-dial/bench-logs/1.0.2-overnight.log
$ tail -3 "$LOG"
=== reader attached 2026-09-12T01:41:48Z ===
I (21101430) app: side A: on=1 set=18.0C water=19.5C
I (21112059) app: side A: on=1 set=18.0C water=19.4C
```

### Confirmation (extra, read-only)

```
$ ps -p 25176,25166 -o pid,ppid,stat,command
  PID  PPID STAT COMMAND
25166     1 SN   caffeinate -i -s
25176     1 SN   bash -c   while true; do ... cat /dev/cu.usbmodem83401 >> .../bench-logs/1.0.2-overnight.log ... done
$ pgrep -fl "cat /dev/cu.usbmodem83401"
25181 cat /dev/cu.usbmodem83401
$ wc -l -c bench-logs/1.0.2-overnight.log        (≈ 30 s later)
       5     261 bench-logs/1.0.2-overnight.log
$ tail -2 bench-logs/1.0.2-overnight.log
I (21122709) app: side A: on=1 set=18.0C water=19.4C
I (21132972) app: side A: on=1 set=18.0C water=19.3C
```

Both processes are reparented to launchd (ppid 1), so they survive this session ending.

## 3. Dial state at start

Last `side A:` line inside the first 20 s:

```
I (21112059) app: side A: on=1 set=18.0C water=19.4C
```

The pad reads **on**, setpoint **18.0 °C**, water 19.4 °C and falling toward it. Four polls landed in the first ~30 s at ticks 21101430, 21112059, 21122709, 21132972 — gaps of 10.6, 10.7, 10.3 s — so the dial was on the **ACTIVE 10 s cadence** at attach time, i.e. the display was not in STANDBY. No `poll:` cadence line fell inside the window (the tier had not changed during it); the first `poll: standby cadence 300s` of the night should appear once the screen times out and the morning task will find it. The pad's previous recorded state (`REPORT-1.0.2-bench-items-2-3.md`) was off at 17.8 °C; it is now on at 18.0 °C, so the owner or the pad's schedule changed it between the bench runs and this start. No write came from this task.

## 4. Deviations

1. **Step 1 was run as `ls -d bench-logs && nohup caffeinate -i -s ... &`** rather than the bare `nohup caffeinate` line, to confirm the log directory existed before anything was started. `$!` still resolved to the caffeinate process itself (ps shows pid 25166 as `caffeinate -i -s`, ppid 1), so the recorded PID is the right one to kill in the morning.
2. **One extra read-only confirmation** (`ps`/`pgrep`/second `wc`/`tail`) was run after the prescribed alive check, to record the `cat` child pid and show the log still growing. It touched nothing.

No other deviations.

## 5. Owner's instructions for the night

Leave the dial and the USB cable alone; use the bed normally. If the Mac reboots, rerun the step 1 and step 2 blocks exactly as above — the log appends and the `=== reader attached ... ===` marker is the seam. In the morning, ask for the overnight analysis (§6).

## 6. THE MORNING TASK (to be run, not remembered)

Gate first: `git --no-optional-locks status --short --untracked-files=no` prints nothing; `grep -c 'POLL_STANDBY_US 300000000' firmware/dial-idf/main/main.c` prints 1; `bench-logs/1.0.2-overnight.log` exists. Then, in order, with all output quoted in `docs/REPORT-1.0.2-overnight.md`:

1. **Stop the capture.** `kill $(cat /tmp/serial_cap_overnight.pid)`; then `pkill -f "cat /dev/cu.usbmodem83401"` for the `cat` child (pid 25181 tonight, but find it, do not assume); `kill $(cat /tmp/caffeinate_overnight.pid)`. Confirm with `ps` that 25166, 25176 and the `cat` are gone. Do not delete the log or the pid files.
2. **Size and span.** `wc -l -c bench-logs/1.0.2-overnight.log`; the wall-clock span from the first `=== reader attached ===` marker to the time of the kill, and the tick span from the first to the last `side A:` line (the two must agree to within a few seconds if coverage was continuous).
3. **Coverage.** `grep -n '=== reader' bench-logs/1.0.2-overnight.log` — quote every `=== reader attached ... ===` and `=== reader lost device ... ===` line. State plainly: continuous (exactly one attach marker, no lost marker), or not, with the length of each gap between a lost and the following attach.
4. **Cadence lines.** `grep -n 'poll: ' bench-logs/1.0.2-overnight.log` — quote every `poll: standby cadence 300s` and `poll: active cadence 10s` with its tick. Each STANDBY entry must be followed by ACTIVE only when a wake happened; cadence lines must appear only at real tier changes.
5. **STANDBY gaps.** For every run of polls between a `standby cadence 300s` line and the next `active cadence 10s` (or end of log), compute the gaps between consecutive `side A:` ticks and report min / max / count. They must sit at ~300 s (the bench saw 300.4–300.8 s). Any gap far above 300 s inside a STANDBY run is a wedge or a lost reader — cross-check against the markers in step 3. Also report the ACTIVE gaps (~10.3–10.9 s expected).
6. **Setpoint tracking.** List every change of `set=` (and of `on=`) in the `side A:` lines: the previous value, the new value, the tick of the first poll that showed it, and the tick of the poll before it. State for each whether the pickup latency was within 300 s (the gap between those two ticks is the upper bound on how late the dial saw it). These are the pad's own schedule steps; the owner made no dial input if he followed the instructions, so any `POST /api/` or knob line is to be quoted and explained.
7. **Errors.** `grep -nE 'assert|panic|abort|Guru|E \(' bench-logs/1.0.2-overnight.log` — quote every hit, or state there were none.
8. **Resets.** `grep -nE 'ESP-ROM|rst:0x' bench-logs/1.0.2-overnight.log` — quote every hit. Any hit voids the night: a reset means the dial rebooted, so the run is not one unattended night of the same process, and the tick counter restarted, which also breaks steps 2, 5 and 6. Say why it is void and whether the cause was bus power (a `=== reader lost device ===` at the same moment) or the firmware (no marker, reset alone).
9. **Bookends.** Quote the first and the last `side A:` line of the log.
10. **Verdict.** PASS only if coverage was continuous (or every gap is explained and shorter than one STANDBY interval), STANDBY gaps sit at ~300 s, every pad schedule step was picked up within 300 s, and there are no errors or resets. Otherwise FAIL or VOID with the failing step named in the first line. Add the report's line to `docs/REPORTS.md`, commit both as their own commit, do not push, and do not tag — the tag is the owner's gated step.

Log file to keep: `bench-logs/1.0.2-overnight.log` (ignored by git; do not commit it).
