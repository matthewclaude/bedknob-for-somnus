# REPORT: SPEC-standby-poll §6 items 4 and 5 — night face at STANDBY, outage in STANDBY (somnus-v1.0.2 pre-release build)

**Verdict: item 4 PASS. Item 5 PASS.** Item 4: with night forced through the on-device Timezone picker (UK) and the pad held at HEATING (set 42.2 °C, water 26–36 °C), the owner saw the number/WATER alternation at roughly 2 s, WATER legible, the whole face at the dim floor, and the WATER number step from 83 °F to 94 °F exactly at the next 300 s poll (300.4 s after the previous one), matching the app's 94 °F read 30 s later; worst-case staleness is one cadence, 300.4 s. Item 5: with the pad unplugged at T0 = 23:28:29Z, three consecutive polls failed at T0+202 s, T0+507 s and T0+812 s, each attempt returning after 5.0–5.3 s (the esp-tls connect timeout); the third failure is the `PH_DEGRADED` transition (`main.c:1425`, no log line by design) at ~812 s, inside the 600–900 s band; the owner confirmed the staleness dot on the dimmed face; a single tap produced a wake poll attempt ~2.4 s after the wake line (failure returned 7.7 s after it), not at the 300 s boundary; after the pad was plugged back in the next STANDBY-cadence poll succeeded and the owner confirmed the dial back in its correct state. The pad itself came back OFF at 27.0 °C, not the ON / 42.2 °C set before the unplug — the pad's own power-cycle behaviour, noted, not corrected.

Date: 2026-09-12. Branch `main`; spec commit `7fda070`; `PROJECT_VER` `1.0.1`; the build under test is the one already on the bench dial (no flash this session). Capture `bench-logs/1.0.2-items-4-5.log`, reader PID 43144, started 23:10:57Z, killed 23:54:26Z; 71 lines / 3853 bytes; the serial content spans device ticks 18704213–21019918 ms (38 m 36 s of device time between first and last line) inside a 43 m 29 s wall-clock capture window. The log carries device-tick timestamps only (`cat` adds nothing); UTC values below are derived with the anchor "tick 19398 s ↔ 23:25:45–23:26:00Z" (the TZ-restore line, bracketed by my request and my next read) and are good to about ±8 s.

## 1. Gate — raw output

```
$ grep -n 'define POLL_STANDBY_US' firmware/dial-idf/main/main.c
79:#define POLL_STANDBY_US 300000000    // STANDBY cadence: once every five minutes
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1")
$ ls -l /dev/cu.usbmodem83401
crw-rw-rw-  1 root  wheel  0x9000005 Sep 12 13:02 /dev/cu.usbmodem83401
$ ls bench-logs/ 2>/dev/null | grep -c 'items-4-5'
0
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All five passed.

## 2. Commit 1 — spec reword

`docs/SPEC-standby-poll.md` §6 item 4 replaced with the prescribed text (bold lead-in and number kept; written as one line to match the one-line-per-item form of items 1–7 in that list). Other mentions of the Tokyo trick in `docs/` untouched.

```
$ git --no-optional-locks show --stat HEAD
7fda070690f4f5ebeb4c25884a39074bba7b6934
docs: SPEC-standby-poll §6 item 4 — night face via the on-device Timezone picker, not the Tokyo trick

 docs/SPEC-standby-poll.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

`git --no-optional-locks diff --stat` after the commit and at the time of writing this report: empty (working tree clean; `bench-logs/` is gitignored at `.gitignore:28`).

## 3. Capture

```
DEV=/dev/cu.usbmodem83401
mkdir -p bench-logs
stty -f $DEV 115200 raw
nohup cat $DEV > bench-logs/1.0.2-items-4-5.log 2>&1 &
PID=43144
START_UTC=2026-09-12T23:10:57Z
```

The file stayed at 0 bytes for the first ~3.5 minutes (dial in STANDBY, nothing to log between polls); first line at 23:14:2xZ, after which it was confirmed growing before any owner action was requested. Killed at the end:

```
END_UTC=23:54:26Z
$ kill 43144; ps -p 43144 -o pid=
reader 43144 gone
$ wc -lc bench-logs/1.0.2-items-4-5.log
      71    3853 bench-logs/1.0.2-items-4-5.log
```

## 4. Item 4 — night face at STANDBY

### 4.1 Owner steps and his words

| Step | Request | Owner's reply (verbatim) | UTC of reply |
|---|---|---|---|
| a | Bed ON, target ≥ 3 °C from water 23.6 °C | "set 27 c at this time" | 23:15:59Z |
| a′ (added, see §7) | Raise target to the app's maximum | "42 c" | 23:18:38Z |
| b | Settings > Night mode window as shown | "9 to 7 pm" | 23:16:5xZ |
| c | Settings > Night face | "Number only" | 23:17:xxZ |
| d | Settings > Timezone → UK | "ok" | 23:18:17Z |
| e | Time out to STANDBY, watch 60 s: alternation ~2 s? WATER legible? whole face at dim floor? | "yes to all" | 23:20:31Z |
| f | Dial's WATER number and app water now | "108 f , 83 f water" | 23:21:21Z |
| f | At the change: new number, time, app water | "108 , 94 f" | 23:25:23Z |
| f | App water now | "94" | 23:25:45Z |
| g | Timezone → Central; face left night? | "94 in non-night mode" | 23:26:10Z |

The night window "9 to 7 pm" is read as 21:00–07:00. UK local time during the observation was 00:18–00:26 BST, inside that window.

### 4.2 Raw serial excerpt

```
I (18704213) app: side A: on=0 set=18.0C water=23.6C
I (18833814) app: poll: active cadence 10s
I (18841456) app: side A: on=1 set=27.0C water=24.1C
...
I (18909118) app: side A: on=1 set=27.0C water=25.5C
I (18926857) dial_time: TZ Europe/London -> GMT0BST,M3.5.0/1,M10.5.0
I (18927158) power: night mode on
I (18931242) app: side A: on=1 set=27.0C water=26.0C
...
I (18957457) app: side A: on=1 set=27.0C water=26.4C
I (18968717) app: side A: on=1 set=42.2C water=26.5C
I (18979574) app: side A: on=1 set=42.2C water=26.7C
W (18988206) ledc: LEDC FADE TOO SLOW
I (18988274) app: poll: standby cadence 300s
I (18993674) app: poll: active cadence 10s
I (18996680) app: side A: on=1 set=42.2C water=26.8C
...
I (19050537) app: side A: on=1 set=42.2C water=28.1C
W (19053606) ledc: LEDC FADE TOO SLOW
I (19053837) app: poll: standby cadence 300s
I (19350918) app: side A: on=1 set=42.2C water=34.2C
I (19386318) app: poll: active cadence 10s
I (19398117) dial_time: TZ America/Chicago -> CST6CDT,M3.2.0,M11.1.0
I (19398418) power: night mode off
I (19403116) app: side A: on=1 set=42.2C water=35.0C
```

### 4.3 Numbers

- Night entered at tick 18927.2 s on the TZ change (`power: night mode on`), left at tick 19398.4 s on the restore (`power: night mode off`). The log confirms the picker route works without the web page.
- HEATING held throughout: set 42.2 °C vs water 26.5–35.7 °C, always more than 0.5 °C apart, so `alt_want` stayed true and the alternation timer was never deleted.
- STANDBY for the observation: `poll: standby cadence 300s` at tick 19053.8 s (~23:20:19Z). Last ACTIVE poll 19050.5 s read water 28.1 °C = 82.6 °F; the owner read the dimmed face as "83 f water".
- Next STANDBY poll at tick 19350.9 s, **300.4 s** after the previous poll, read water 34.2 °C = 93.6 °F; the owner reported the face changing to "94 f" and the app reading "94" 30 s later. The dial's WATER number reached the app's value at the poll boundary, i.e. within 300 s.
- **Worst-case WATER staleness: 300.4 s**, one cadence. During that interval the water rose 28.1 → 34.2 °C (11 °F) while the face showed 83 °F; that gap is the cost the spec item names, and it is bounded by the cadence, not the alternation.
- Owner's words on the glass: "yes to all" to alternation roughly every 2 s, WATER legible, whole face at the night dim floor. Restore: "94 in non-night mode".

## 5. Item 5 — outage in STANDBY

### 5.1 Owner steps and his words

| Step | Request | Owner's reply (verbatim) | UTC of reply |
|---|---|---|---|
| h | (mine) wait for STANDBY | — log: `poll: standby cadence 300s` at tick 19460.1 s, ~23:27:00Z | 23:27:03Z |
| i | Make the pad unreachable, give UTC | "unplugged" | **T0 = 23:28:29Z** |
| (unsolicited, during j) | — | "it's still showing 108 and it's powered on" | 23:38:xxZ |
| k | Staleness dot on the dimmed face? | "staleness dot is showing, and tapped" | 23:47:13Z |
| l | Tap once to wake, give UTC | (same reply as k) | 23:47:13Z |
| m | Restore the pad, give UTC | "plugged back in" | 23:48:49Z |
| n | Bed state in the app | "app is reconnected and it's off" / "Target says OFF in the app because it does not display a temperature in the off setting." | 23:53:xxZ |
| (dot gone?) | Is the staleness dot gone now? | "the bedknob returned to its correct state." | 23:54:2xZ |

The owner gave no UTC for i or m; T0 and the restore time are the receipt times of his replies.

### 5.2 Raw serial excerpt — the whole outage, unfiltered

```
I (19450528) app: side A: on=1 set=42.2C water=35.7C
I (19460129) app: poll: standby cadence 300s
E (19755532) esp-tls: [sock=54] select() timeout
E (19755532) transport_base: Failed to open a new connection: 32774
E (19755532) HTTP_CLIENT: Connection failed, sock < 0
W (19755536) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
E (20060844) esp-tls: [sock=54] select() timeout
E (20060844) transport_base: Failed to open a new connection: 32774
E (20060844) HTTP_CLIENT: Connection failed, sock < 0
W (20060848) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
E (20366156) esp-tls: [sock=54] select() timeout
E (20366156) transport_base: Failed to open a new connection: 32774
E (20366156) HTTP_CLIENT: Connection failed, sock < 0
W (20366160) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
I (20666165) app: poll: active cadence 10s
E (20673868) esp-tls: [sock=54] select() timeout
E (20673868) transport_base: Failed to open a new connection: 32774
E (20673868) HTTP_CLIENT: Connection failed, sock < 0
W (20673872) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
E (20689080) esp-tls: [sock=54] select() timeout
E (20689080) transport_base: Failed to open a new connection: 32774
E (20689080) HTTP_CLIENT: Connection failed, sock < 0
W (20689084) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
E (20704292) esp-tls: [sock=54] select() timeout
E (20704292) transport_base: Failed to open a new connection: 32774
E (20704292) HTTP_CLIENT: Connection failed, sock < 0
W (20704296) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
E (20719504) esp-tls: [sock=54] select() timeout
E (20719504) transport_base: Failed to open a new connection: 32774
E (20719504) HTTP_CLIENT: Connection failed, sock < 0
W (20719508) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
I (20726413) app: poll: standby cadence 300s
I (21019918) app: side A: on=0 set=27.0C water=31.8C
```

There is no `PH_DEGRADED` line and no `PH_READY` / `device_online` line: `dial_state_set_phase()` (`components/dial_state/dial_state.c`) logs nothing, and `main.c` logs nothing around the calls at lines 1421–1426. The phase evidence is the code path plus the glass (§5.4).

### 5.3 Numbers

`last_poll_us` is stamped when an attempt returns (`main.c:1427`), so each due time is the previous return + 300 s.

| Event | Tick (s) | UTC (±8 s) | From T0 | Attempt duration |
|---|---|---|---|---|
| Last good poll (ACTIVE) | 19450.5 | 23:27:03Z | −86 s | — |
| STANDBY cadence line | 19460.1 | 23:27:13Z | −76 s | — |
| **T0** (owner: "unplugged") | ~19554 | 23:28:29Z | 0 | — |
| Failed poll 1 (due 19750.5) | 19755.5 | 23:31:51Z | **+202 s** | 5.0 s |
| Failed poll 2 (due 20055.5) | 20060.8 | 23:36:56Z | **+507 s** | 5.3 s |
| Failed poll 3 (due 20360.8) = `PH_DEGRADED` | 20366.2 | 23:42:01Z | **+812 s** | 5.3 s |
| Wake line (`active cadence 10s`) | 20666.2 | 23:47:01Z | +1112 s | — |
| Wake poll attempt began / returned | ~20668.6 / 20673.9 | 23:47:03Z / 23:47:09Z | — | 5.3 s |
| ACTIVE retries (failed) | 20689.1, 20704.3, 20719.5 | | | 5.2–5.3 s each, 15.2 s apart |
| Back to STANDBY | 20726.4 | 23:48:01Z | | 60.2 s after wake |
| Restore (owner: "plugged back in") | ~20774 (T0 + 1220 s) | 23:48:49Z | +1220 s | — |
| First successful poll after restore (due 21019.5) | 21019.9 | 23:52:55Z | | 0.4 s; 246 s after restore |

- Gaps between failure lines: 305.3 s and 305.3 s = 300 s cadence + the 5.3 s connect timeout; the cadence held through the outage.
- `PH_DEGRADED` at **T0 + 812 s**, inside the 600–900 s band, toward the late end because the unplug landed 104 s after the previous poll return (19450.5 + 104 ≈ 19554), i.e. 196 s before the next due poll.
- **Wake line to wake poll: ~2.4 s** to the attempt starting, 7.7 s to its (failed) return, followed by retries at the 10 s ACTIVE cadence. The wake path ran; note that the next scheduled STANDBY poll happened to be due at the same tick as the tap (§7 item 8).
- Recovery: the dial had returned to STANDBY before the pad was restored, so the first successful poll was the next 300 s-cadence poll, 246 s after the restore reply and 300.4 s after the last failed attempt; `somnus_refresh_state()` succeeding resets `poll_failures` and sets `PH_READY` (`main.c:1421–1423`).

### 5.4 Owner's words on the glass

- Before degrade (~T0+10 min): "it's still showing 108 and it's powered on" — the face held the last known state (set 42.2 °C = 108 °F) through the first two failures, as designed.
- After the third failure: "staleness dot is showing".
- After recovery: "the bedknob returned to its correct state." (asked whether the staleness dot was gone; taken as yes).
- Pad state after restore, in the app: "app is reconnected and it's off" and "Target says OFF in the app because it does not display a temperature in the off setting." The log agrees: `side A: on=0 set=27.0C water=31.8C`. This is not the ON / 42.2 °C state set in (a): the pad controller came back off with its earlier 27.0 °C target after the power cycle. Noted per (n); nothing written to the pad.

## 6. Verdict

- **Item 4: PASS.** Night forced via the on-device picker; alternation, legibility and dim floor confirmed by the owner at STANDBY under HEATING; WATER number tracked the pad at the next poll, 300.4 s after the previous; worst-case staleness one cadence.
- **Item 5: PASS.** Three consecutive failed polls at the 300 s cadence, `PH_DEGRADED` at T0+812 s (in band), staleness dot confirmed, wake poll within ~2.4 s of the wake line, clean recovery on the first poll after restore.

## 7. Deviations from the instructions

1. **Added owner action (a′).** After (a) the water rose 23.6 → 26.1 °C in three minutes toward a 27 °C target, which would have ended HEATING (and correctly deleted the alternation timer) before the observation. I asked the owner to raise the target to the app's maximum; he set 42 °C (dial reads 42.2 °C). This is within (a)'s intent but was not in the script.
2. **Step (b)** was answered "9 to 7 pm"; I recorded the words and read the window as 21:00–07:00 rather than asking again.
3. **Step (f), first reading.** The owner's reply "108 f , 83 f water" gave the dial's two numbers, not the app's water value; a second request for the app value was answered after the poll had already landed ("108 , 94 f", then "94"). The app's water at the *start* of the stale interval was therefore never captured; the staleness figure rests on the log's two poll values (28.1 → 34.2 °C) and the app matching the dial at the end of the interval.
4. **T0 and the restore time** are the receipt times of the owner's replies ("unplugged", "plugged back in"); he gave no UTC.
5. **One action per message** was bent three times: (e)'s three questions in one message (one observation), (k)+(l) in one message ordered dot-then-tap, and (m)+(n) in one message ordered restore-then-read-app. Each pair was ordered so the second could not precede the first.
6. **No `PH_DEGRADED`, `PH_READY` or `device_online` log lines exist** in this firmware; those items are shown by the third failed poll's position in the code path (`main.c:1424–1426`) and the owner's dot reports, not by a log string.
7. **UTC in the log.** The capture is raw `cat`, so lines carry device ticks only; UTC values are derived from a request/read bracket and are ±8 s.
8. **Wake timing coincidence.** The wake line (20666.2 s) happens to sit exactly 300.0 s after the third failure (20366.2 s), so the next scheduled STANDBY poll was due at the same instant the owner tapped. The attempt that followed cannot be told apart from the scheduled one by timing alone. What does separate them: the cadence line switched to ACTIVE first, and three further attempts followed at 10 s cadence, so the wake path ran; and the 2.4 s gap from the wake line to the attempt start is the wake-consume delay, not a 300 s boundary. The "within seconds of the wake line" expectation is met, but this run does not isolate the wake poll from a coincident scheduled poll.
9. **Spec reword layout.** The prescribed text was given wrapped; it was written as a single line so item 4 keeps the one-line-per-item form of the list. Words, number and bold lead-in are verbatim.

Nothing else deviated: no push, no tag, no `PROJECT_VER` change, no flash, no `idf.py monitor`, no pad writes, no dial touches by me, `git status` only ever with `--no-optional-locks`.

## 8. What could not be verified

- The alternation period, WATER legibility, the dim floor, the staleness dot appearing and disappearing, and the face leaving night: glass-only, on the owner's word as quoted above. The final "returned to its correct state" is his answer to "is the dot gone", not a literal "dot gone".
- App water at the start of the stale interval (deviation 3), so the staleness is bounded by the poll-to-poll interval (300.4 s) rather than measured against the app at both ends.
- `PH_DEGRADED` / `PH_READY` transitions as such (deviation 6).
- Whether the wake poll was the wake-consume path or the coincident scheduled poll (deviation 8); the ACTIVE cadence line and the 10 s retries make the wake path certain to have run, but the specific attempt is ambiguous.
- Wall-clock UTC of log lines beyond ±8 s (deviation 7).
