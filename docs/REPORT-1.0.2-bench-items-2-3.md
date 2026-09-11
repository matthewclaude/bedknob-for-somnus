# REPORT: SPEC-standby-poll §6 items 2 and 3 — wake poll and external change in STANDBY (somnus-v1.0.2 pre-release build)

**Verdict: PASS on both items, but the pad ends at 17.8 °C (64 °F), not 18.0 °C.** The Somnus app is Fahrenheit-only and has no value that maps to 18.0 °C; the owner chose 64 °F (17.8 °C) for the restore, the pad is off, and the last poll of the capture reads `on=0 set=17.8C`. Item 2: the tap woke the dial with no pad write and the wake poll landed 2.48 s after the wake line, showing the app's change. Item 3: the app change made in STANDBY reached the dimmed face on the next 300 s poll, 3 min 35 s to 4 min 16 s after the change. No errors, asserts or panics. The run took two attempts; the first is kept as evidence and contains two dial-originated pad writes from the owner's touches, none from this task.

Date: 2026-09-11. Branch `main` at `459d435`, `PROJECT_VER` `1.0.1`, firmware commit `39af8f5` as flashed by `REPORT-1.0.2-bench-cadence.md`. No flash, no bump, no tag, no push. Wall-clock: dial boot ≈ 19:50:08Z (from the previous report's anchor); tick → UTC is boot + tick, good to ±2 s. Owner's clock is CDT (UTC−5).

## 1. Gate — raw output of all four checks

```
$ grep -c 'POLL_STANDBY_US 300000000' firmware/dial-idf/main/main.c
1
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1")
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All four passed.

## 2. Captures — commands and job output

Both attempts used the task's command verbatim (attempt 2 differs only in that the attempt-1 log had been renamed first):

```
DEV=/dev/cu.usbmodem83401
stty -f $DEV 115200 raw
( cat $DEV & echo $! > /tmp/serial_cap_102c.pid ) > bench-logs/1.0.2-items-2-3.log 2>&1
echo "capture started $(date -u +%T) pid=$(cat /tmp/serial_cap_102c.pid)"
sleep 900
kill $(cat /tmp/serial_cap_102c.pid); echo "kill exit=$?"
echo "capture stopped $(date -u +%T)"
```

Attempt 1 (stopped early at the owner's request, see §5 deviation 2; its job was ended with TaskStop and the reader killed by hand, so it printed no `kill exit`/`stopped` lines; the log was renamed to `bench-logs/1.0.2-items-2-3-attempt1.log`):

```
capture started 20:32:06 pid=21380
(stopped 20:38:16Z by hand)
```

Attempt 2, the capture of record, `bench-logs/1.0.2-items-2-3.log`:

```
capture started 20:38:21 pid=21538
kill exit=0
capture stopped 20:53:21
```

Both logs are in the gitignored `bench-logs/`; the quoted lines are the evidence of record.

## 3. Analysis

### (a) Sizes

| Log | Bytes | Lines |
|---|---|---|
| `bench-logs/1.0.2-items-2-3.log` (attempt 2, of record) | 1577 | 31 |
| `bench-logs/1.0.2-items-2-3-attempt1.log` | 1212 | 23 |

### (b) Every `cadence` line, verbatim

Attempt 2:

```
L1:  I (2916839) app: poll: active cadence 10s       20:38:45Z  step-B first try: tap wake (clean)
L13: I (3035393) app: poll: standby cadence 300s     20:40:43Z
L14: I (3072593) app: poll: active cadence 10s       20:41:21Z  step-B second try: tap wake (clean)
L21: I (3132564) app: poll: standby cadence 300s     20:42:21Z
L23: I (3447330) app: poll: active cadence 10s       20:47:35Z  owner touched the dial after item 3 landed
L30: I (3507369) app: poll: standby cadence 300s     20:48:35Z
```

Attempt 1:

```
L2:  I (2602378) app: poll: active cadence 10s       20:33:30Z  tap wake, followed 1 s later by a power-on write
L12: I (2663466) app: poll: standby cadence 300s     20:34:31Z
L13: I (2779266) app: poll: active cadence 10s       20:36:27Z  touch wake, followed 1 s later by a power-off write
L23: I (2840339) app: poll: standby cadence 300s     20:37:28Z
```

Every line is a real STANDBY entry or exit; each STANDBY entry is 60–61 s after the last input (the owner's 60 s screen timeout). No line at boot (no boot in either capture), none per poll.

Step A: the dial had been in STANDBY since 19:53:06Z (previous report) when attempt 1 started, so no transition line could appear; STANDBY was confirmed instead from the first poll, `I (2582877) app: side A: on=0 set=24.0C water=24.0C`, which sits exactly two 300 s intervals (601.6 s) after the previous report's last poll at 1981261, with no activity line before it. For attempt 2, the transition into STANDBY at 2840339 is in the attempt-1 log (L23), 60.6 s after the last write.

### (c) Item 2 — the wake poll

Second try of step B in attempt 2 (the first try is deviation 4). Owner's actions: bed ON and target 68 °F from the app, 5 s wait, one fingertip tap on the glass.

```
L13: I (3035393) app: poll: standby cadence 300s
L14: I (3072593) app: poll: active cadence 10s
L15: I (3075076) app: side A: on=1 set=20.6C water=22.5C
L16: I (3085682) app: side A: on=1 set=20.6C water=22.4C
```

- Last input before the poll: the tap itself. It is the input that flipped the tier (`dial_power_wake_consumes()` in the touch filter), so the worker's wake line at 3072593 is at most one 300 ms tick after the touch.
- First poll after the wake line: 3075076, showing the bed on and the new setpoint.
- Settle gap: 3075076 − 3072593 = **2.483 s** from the wake line, i.e. 2.5–2.8 s from the touch. That is `KNOB_SETTLE_US` (2.5 s) and nothing else: no confirm polls, no forced-poll shortcut past the gate.
- No `POST /api/` line: the tap was swallowed end to end and the dial wrote nothing.
- Owner: "68 within ~3 s".

**Item 2: PASS** against "within ~3 s of the tap". Note the pad reports the app's 68 °F as `set=20.6C` (= 69.1 °F), not 20.0 °C; see deviation 5.

### (d) Item 3 — the external change in STANDBY (the restore)

Owner's action: target 64 °F and bed OFF from the app, dial untouched; owner's stated time "3:43" pm CDT = 20:43Z, to the minute. My own clock bounds the change to between the Step D prompt being answered (20:42:27Z) and my next log check (20:43:40Z), so the change was made in **20:43:00–20:43:40Z**.

```
L20: I (3127463) app: side A: on=1 set=20.6C water=22.6C     last ACTIVE poll, 20:42:16Z
L21: I (3132564) app: poll: standby cadence 300s             20:42:21Z
L22: I (3427529) app: side A: on=0 set=17.8C water=23.5C     20:47:16Z  ← first poll showing the change
```

- First poll showing `set=17.8C` (and `on=0`): tick 3427529 ≈ 20:47:16Z, 300.07 s after the previous poll: the first STANDBY-cadence poll after the change, exactly as the spec describes.
- Elapsed from the owner's change: **3 min 35 s to 4 min 16 s** (215–256 s), under 300 s.
- No `POST /api/` line: the dial wrote nothing.
- Owner, asked at ~20:44Z while waiting: "Still shows the old value". Asked after the poll: "Yes, 64 and off". Asked whether the dim face read as stale or frozen while waiting: **"Looked normal, not stale."**

**Item 3: PASS** against "within five minutes". The change was in the pad within the window and the dimmed face showed it.

### (e) Every `POST /api/` line

Attempt 2 (of record): **none.** The dial made no pad writes in the capture of record.

Attempt 1: two, both from the dial and both 1 s after a wake:

```
L2:  I (2602378) app: poll: active cadence 10s
L3:  I (2603345) dial_somnus: POST /api/power {"side0":{"is_on":true}}
L13: I (2779266) app: poll: active cadence 10s
L14: I (2780305) dial_somnus: POST /api/power {"side0":{"is_on":false}}
```

Cause: the owner's touches on the dial. The first was the step-B wake (owner: "Single tap on the screen"); the second was the owner turning the bed back off, done on the dial rather than from the app. In this build the press that wakes a STANDBY screen is swallowed until release by `touch_filter()` in `main.c`, so a single contact cannot reach the dial face's power button; a toggle 1 s after the wake means a second contact landed on the power button after the face had come up. Attempt 2's three tap wakes (L1, L14, L23) all produced no write, which is consistent with that reading and with the owner's "i'll make sure to pay attention". The two attempt-1 writes cancelled each other on power (on, then off) and did not touch the setpoint. Neither was this task's, and the owner's instructions were followed as written.

### (f) Errors, asserts, panics

Lines starting `E (`, or containing assert / panic / abort / Guru, in either log: **none.** No reset markers; the dial's uptime ran continuously from the previous report's boot (tick 3619505 ≈ 60.3 min at the end).

### (g) First and last `side A:` lines

Attempt 2:

```
first L2:  I (2919614) app: side A: on=0 set=24.0C water=23.2C
last  L29: I (3502569) app: side A: on=0 set=17.8C water=23.6C
```

The pad ends **off at 17.8 °C (64 °F)**. That is not 18.0 °C; it is the closest value the Fahrenheit-only Somnus app offers (64 °F = 17.78 °C; 65 °F = 18.33 °C) and the one the owner chose. The pad was on for the 6 minutes between the item-2 stimulus (20:41Z) and the item-3 restore (20:43Z) at 20.6 °C, a heat the water temperature shows (22.4 → 23.6 °C is room settling; it never reached the setpoint). Restored to the owner's intent, not to the literal 18.0 °C.

### (h) The owner's answers, quoted

- Step A: "Yes, dark and untouched".
- Step B, attempt 1: "Something else", then in chat "its in F" (the app is Fahrenheit-only; 20.0 °C was not enterable). Asked which input woke the dial: "Single tap on the screen". Asked to turn the pad off from the app and whether the app could switch to Celsius: "Pad turned off; app stays in F". Then in chat: "lets start over and i'll make sure to pay attention :)".
- Step B, attempt 2, first try (68 °F set with the bed off): "Still showed the old 75". Asked what the app showed: "the somnus app shows it as OFF and the Bedknob shows 75 and the bed is off."
- Asked whether to turn the bed on from the app for the stimulus: "Yes, turn on + set 68 from the app". Asked which value to restore to: "64 F (17.8 C)".
- Step B, attempt 2, second try: **"68 within ~3 s"**.
- Step C: "Yes, hands off".
- Step D, first phrasing (three parts in one message): "i do not understand your instructions". Re-issued as one action: "Done: 64 and off". Change time, in chat: **"3:43"**. While waiting: "Still shows the old value". After the poll: **"Yes, 64 and off"**. Stale or frozen during the wait: **"Looked normal, not stale"**.

## 4. Step E — end state

The last `side A:` line of the capture of record reads `on=0 set=17.8C`. **The pad is restored to off at 64 °F / 17.8 °C, the Fahrenheit-only app's nearest value to 18.0 °C, as the owner chose. It is not at 18.0 °C.** The owner was told in chat.

## 5. Deviations

1. **Restore is 17.8 °C, not 18.0 °C.** The Somnus app is Fahrenheit-only; no whole-°F value is 18.0 °C. The owner chose 64 °F. The dial could have set exactly 18 °C but the task forbids dial writes. Stated in the verdict line.
2. **Two attempts.** In attempt 1 the step-B tap was followed by a dial power-on write and the app change never happened (Fahrenheit); the owner then turned the bed off on the dial (a second write) and asked to start over. Attempt 1 was stopped at 20:38:16Z, its log kept as `bench-logs/1.0.2-items-2-3-attempt1.log`, and attempt 2 started at 20:38:21Z. The dial was in STANDBY (since 20:37:28Z) when attempt 2 began. Not a blind retry: the cause was identified and the owner requested it.
3. **Step A had no transition line to wait for** in either attempt: the dial was already in STANDBY. Confirmed from the 300 s grid (attempt 1) and from the attempt-1 log's transition line (attempt 2).
4. **Step B took two tries in attempt 2.** The first tap wake was clean (L1, poll 2.78 s after the wake line, no write) but the pad still read 24.0 °C: the app had not sent the target because the bed was off (the app only sets a target while on). With the owner's agreement the stimulus became "bed ON + 68 °F" from the app, so item 2's stimulus included an on/off change as well as a setpoint change. The bed was on for ~6 minutes as a result.
5. **68 °F arrived as 20.6 °C**, i.e. 69.1 °F, not the 20.0 °C the task named. Either the app sent 69 or the pad rounds; not investigated, outside this task. The owner read "68" on the face; the face shows whole °F and the dial's own °F rendering of 20.6 °C would be 69. The measurement (a change, seen within 3 s) does not depend on the value.
6. **Step D's three parts were sent in one message** on the first try, against "do not batch"; the owner did not understand. Re-issued one action per message. The owner's change time is to the minute ("3:43") and is bounded by my clock to 20:43:00–20:43:40Z; the item-3 elapsed time is therefore a 41 s range, all of it under 300 s.
7. **The owner touched the dial twice when asked not to**, both harmless to the measurements: once around tick 2975 (the ACTIVE stretch after the first tap ran 118 s instead of 60), and once at 3447330 (20:47:35Z), 20 s after the item-3 poll had already landed. Both were clean wakes with no write.
8. **Wake input premise.** The task states a tap wakes without acting. That held for all three attempt-2 wakes; the two attempt-1 power toggles are explained above as a second contact after the wake, not as a hole in the wake-consume rule, but that explanation is inferred from the code and the timing, not observed.

## 6. What remains — §6 items 4 through 7

4. **Night face at STANDBY** — Tokyo-timezone trick; number/water alternation runs at the dim floor and the water number tracks. Also the open code-checks item: water-reading age on the dimmed Temperature face at 300 s.
5. **Outage in STANDBY** — pad unplugged or blocked; `PH_DEGRADED` after ~15 minutes, staleness dot, fresh poll within seconds of wake, recovery on restore. The item carrying the most weight.
6. **One unattended night** — the gate for the tag; the earlier overnight proof ran at 10 s and does not carry over.
7. **Unattended OTA still fires** at STANDBY if an update is available — watched once.

Bench notes for those tasks, learned here: the Somnus app is Fahrenheit-only and sets a target only while the bed is on; wake the dial with one brief fingertip tap and no second contact; one action per message to the owner.
