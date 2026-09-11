# REPORT: SPEC-standby-poll §6 item 1 — STANDBY cadence on the bench (somnus-v1.0.2 pre-release build)

**Verdict: PASS.** The STANDBY cadence measured ~300 s: six consecutive inter-poll gaps of 300.4–300.8 s across the two captures, the `poll: standby cadence 300s` line on each entry into STANDBY, the `poll: active cadence 10s` line on the one wake, and ~10.5 s gaps at ACTIVE. Qualified on one sub-item: the ACTIVE half comes from the first capture, not the second, and the 2-minute ~12-poll count was not collected (see §8). No errors, asserts or panics. Nothing written to the pad by this task, but the pad's setpoint was changed through the dial by a hands-on touch during the run and is **not restored** (see §7h).

Date: 2026-09-11. Branch `main` at `9ca2698`, `PROJECT_VER` `1.0.1`, firmware commit `39af8f5`. No bump, no tag, no push.

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

All four passed; the dial was at the expected port.

## 2. Flash — raw tail

Command: `. ~/esp/esp-idf/export.sh ; cd firmware/dial-idf ; idf.py -p /dev/cu.usbmodem83401 flash` — exit 0, finished 19:50:11Z. Progress-bar lines elided to their final state; everything else verbatim.

```
Uploading stub flasher...
Running stub flasher...
Stub flasher running.
Changing baud rate to 460800...
Changed.

Configuring flash size...

Writing 'bootloader/bootloader.bin' at 0x00000000...
SHA digest in image updated.
Flash will be erased from 0x00000000 to 0x00005fff...
Compressed 22576 bytes to 14461...
Writing at 0x00005830 [==============================] 100.0% 14461/14461 bytes...
Wrote 22576 bytes (14461 compressed) at 0x00000000 in 0.3 seconds (693.9 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes...
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (787.4 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes...
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.1 seconds (1002.0 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1612320 bytes to 984584...
Writing at 0x001a9a20 [==============================] 100.0% 984584/984584 bytes...
Wrote 1612320 bytes (984584 compressed) at 0x00020000 in 10.4 seconds (1241.8 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

All four segments written and hash-verified; the flash reported success. NVS not written: the log below shows the timezone restored from NVS (`CST6CDT`), Wi-Fi joined and the pad address found without discovery, so credentials, timezone and pad address survived. `idf.py monitor` was not used at any point.

## 3. Captures — exact commands and job output

First capture, one background job (STANDBY half):

```
DEV=/dev/cu.usbmodem83401
stty -f $DEV 115200 raw
( cat $DEV & echo $! > /tmp/serial_cap_102.pid ) > bench-logs/1.0.2-standby-cadence.log 2>&1
echo "capture started $(date -u +%T) pid=$(cat /tmp/serial_cap_102.pid)"
sleep 1800
kill $(cat /tmp/serial_cap_102.pid); echo "kill exit=$?"
echo "capture stopped $(date -u +%T)"
```

```
capture started 19:50:18 pid=20633
kill exit=0
capture stopped 20:20:18
```

Second capture (ACTIVE half), started after the owner was told the dial could be touched and asked to wake it with one knob touch:

```
( cat $DEV & echo $! > /tmp/serial_cap_102b.pid ) > bench-logs/1.0.2-active-cadence.log 2>&1
echo "capture started $(date -u +%T) pid=$(cat /tmp/serial_cap_102b.pid)"
sleep 180
kill $(cat /tmp/serial_cap_102b.pid); echo "kill exit=$?"
echo "capture stopped $(date -u +%T)"
```

```
capture started 20:20:52 pid=21096
kill exit=0
capture stopped 20:23:52
```

Both logs live in the gitignored `bench-logs/`; the quoted lines below are the evidence of record. Wall-clock anchor: the first line captured is at tick 10311 ms and the capture began 19:50:18Z, so boot ≈ 19:50:08Z; each tick below converts as boot + tick.

## 4. (a) Sizes

| Log | Bytes | Lines |
|---|---|---|
| `bench-logs/1.0.2-standby-cadence.log` | 3669 | 63 |
| `bench-logs/1.0.2-active-cadence.log` | 53 | 1 |

## 5. (b) The STANDBY transition

Verbatim, first capture line 37:

```
I (60944) app: poll: standby cadence 300s
```

Tick 60.9 s ≈ 19:51:09Z, about 58 s after the flash command finished (19:50:11Z) and 61 s after boot. The dial idled down on its own from the post-flash boot; the owner's screen timeout is evidently 60 s. A second entry into STANDBY, after the touch described in (h), is line 58:

```
I (178129) app: poll: standby cadence 300s
```

That is 60.6 s after the last write (`POST /api/power` at 117538), again the 60 s timeout, and 0.3 s after the last ACTIVE poll at 177829.

## 6. (c) The cadence itself — every poll after the transition, with gaps

Poll lines are the worker's `side A: on=… set=… water=…` INFO line, one per successful `GET /api/state` (single-zone pad, so one line per poll). Every poll after the line-58 transition, verbatim, then the one poll in the second capture:

```
L57: I (177829) app: side A: on=0 set=24.0C water=23.7C     (last ACTIVE poll, 0.3 s before the transition)
L58: I (178129) app: poll: standby cadence 300s
L59: I (478205) app: side A: on=0 set=24.0C water=23.9C
L60: I (778939) app: side A: on=0 set=24.0C water=24.0C
L61: I (1079319) app: side A: on=0 set=24.0C water=24.0C
L62: I (1380150) app: side A: on=0 set=24.0C water=24.0C
L63: I (1680534) app: side A: on=0 set=24.0C water=24.0C
--- second capture ---
L1:  I (1981261) app: side A: on=0 set=24.0C water=24.0C
```

| From tick | To tick | Gap |
|---|---|---|
| 177829 | 478205 | 300.376 s |
| 478205 | 778939 | 300.734 s |
| 778939 | 1079319 | 300.380 s |
| 1079319 | 1380150 | 300.831 s |
| 1380150 | 1680534 | 300.384 s |
| 1680534 | 1981261 | 300.727 s |

Six consecutive gaps, all between 300.4 s and 300.8 s. **Yes, they are ~300 s.** The 0.4–0.8 s over 300 is the worker's 300 ms command-queue wait plus the HTTP round-trip, the same overhead the 10 s cadence shows (10.3–10.9 s, below). This is a measurement of the constant, not a count. The 30-minute window's last poll fell at 1680.5 s of uptime and the capture ended at ~1810 s; the sixth gap crosses into the second capture, which is a continuous view of the same uptime.

## 7. (d) through (h)

### (d) The wake

Verbatim, first capture line 38:

```
I (102044) app: poll: active cadence 10s
```

Tick 102.0 s ≈ 19:51:50Z, 41 s after entering STANDBY. It was followed by seven writes from the dial (lines 39–45, quoted in (h)) and then the first poll at:

```
I (109793) app: side A: on=1 set=24.0C water=23.5C
```

7.7 s after the cadence line and 2.63 s after the last knob input (`POST /api/target_t … 24` at 107163). That is the `KNOB_SETTLE_US` gate (2.5 s) doing exactly what the spec describes: the poll came within seconds of the wake, held only until the knob stopped, not minutes later. The three 2 s confirm polls then ran (109793 → 112293 → 115362; 2.5 s and 3.1 s, the second stretched by the queue wait), a power-off write at 117538 restarted them (120201 → 122350 → 124498; 2.1 s, 2.1 s), and idle cadence resumed.

### (e) The ACTIVE cadence

The second capture contains no wake: the owner did not touch the dial during it (line count 1, the standby poll at 1981261). The ACTIVE cadence is therefore taken from the first capture, which has two ACTIVE stretches:

Before the first STANDBY (boot to 60.9 s):

```
I (15933) app: side A: on=0 set=18.0C water=23.6C
I (26681) app: side A: on=0 set=18.0C water=23.6C
I (36957) app: side A: on=0 set=18.0C water=23.6C
I (47676) app: side A: on=0 set=18.0C water=23.6C
I (57944) app: side A: on=0 set=18.0C water=23.6C
```

After the wake and the confirm polls (124.5 s to the second STANDBY at 178.1 s):

```
I (124498) app: side A: on=0 set=24.0C water=23.6C
I (135229) app: side A: on=0 set=24.0C water=23.6C
I (145491) app: side A: on=0 set=24.0C water=23.6C
I (156241) app: side A: on=0 set=24.0C water=23.6C
I (166977) app: side A: on=0 set=24.0C water=23.7C
I (177829) app: side A: on=0 set=24.0C water=23.7C
```

| Stretch | Gaps (s) |
|---|---|
| boot → STANDBY | 10.748, 10.276, 10.719, 10.268 |
| wake → STANDBY | 10.731, 10.262, 10.750, 10.736, 10.852 |

Nine idle gaps at ACTIVE/DIMMED, all 10.3–10.9 s: the 10 s cadence is unchanged. The spec's "count over 2 minutes: expect ~12" was not performed: with a 60 s screen timeout the dial cannot stay ACTIVE for two minutes without input, so the longest input-free ACTIVE stretch available is ~58 s (five or six polls), and that is what both stretches show (5 and 6 polls). A 2-minute count would need a longer timeout setting or a touch at the 1-minute mark, either of which changes what is being measured.

### (f) Cadence lines only at real transitions

Every line containing `cadence` in both logs:

```
first capture  L37: I (60944) app: poll: standby cadence 300s
first capture  L38: I (102044) app: poll: active cadence 10s
first capture  L58: I (178129) app: poll: standby cadence 300s
second capture: (none)
```

Three lines, each at a genuine STANDBY entry or exit. None at boot: the capture starts at 10.3 s of uptime, the worker's first poll is at 15.9 s (after `pad connected` at 15.5 s), and there is no cadence line between there and the 60.9 s entry. None on ACTIVE→DIMMED: dimming precedes each STANDBY entry (before 60.9 s and again between 117.5 s and 178.1 s) and neither stretch has a line. None per poll: eleven STANDBY-tier polls, three cadence lines.

### (g) Errors, asserts, panics

Lines starting `E (`, or containing assert / panic / abort / Guru, in either log: **none.** No boot markers (`ESP-ROM`, `rst:0x`) in either log, so the dial did not reset during the 33 minutes captured. The only `W` line is none; the only non-`I` lines are none.

### (h) Pad state through both captures

First poll of the run and last poll of the run:

```
I (15933) app: side A: on=0 set=18.0C water=23.6C
I (1981261) app: side A: on=0 set=24.0C water=24.0C
```

**Not the same.** On/off is back where it started (off), but the setpoint went from 18.0 °C to 24.0 °C. The cause is in the log and it was not this task: between the wake at 102.0 s and 117.5 s the dial itself issued writes, i.e. someone had hands on the dial at ≈19:51:52–19:52:08Z, about a minute and a half after the "leave it alone" instruction went out. Verbatim:

```
I (104009) dial_somnus: POST /api/power {"side0":{"is_on":true}}
I (104918) dial_somnus: POST /api/target_t {"side0":{"target_t":19}}
I (105150) dial_somnus: POST /api/target_t {"side0":{"target_t":20}}
I (105367) dial_somnus: POST /api/target_t {"side0":{"target_t":21}}
I (105563) dial_somnus: POST /api/target_t {"side0":{"target_t":22}}
I (106045) dial_somnus: POST /api/target_t {"side0":{"target_t":23}}
I (107163) dial_somnus: POST /api/target_t {"side0":{"target_t":24}}
I (117538) dial_somnus: POST /api/power {"side0":{"is_on":false}}
```

A press (on), six detents up (19→24 °C), a press (off). The pad's target is now 24.0 °C and it is off. This task made no writes and did not restore it: **the setpoint needs putting back to 18.0 °C by the owner** (dial, app, or web page). Water temperature drifted 23.6 → 24.0 °C over the half hour with the pad off, which is room-temperature settling, not heating.

The touch did no harm to the measurement — the dial re-entered STANDBY 60 s after the last write and the six ~300 s gaps all follow it — and it incidentally supplied the wake evidence in (d).

## 8. Deviations

1. **Second capture empty of a wake.** Step 4's ACTIVE capture holds one line (a STANDBY poll); nobody touched the dial in its 3-minute window. Per the task, no blind retry. The ACTIVE cadence (e) and the wake (d) are reported from the first capture instead, which has nine ~10 s gaps and one wake with a poll 2.6 s after the last input. The literal "2 minutes, ~12 polls" count was not made and, at a 60 s screen timeout, cannot be made without input (§7e).
2. **The dial was touched during the first capture** (19:51:52–19:52:08Z), against step 2. The writes were the dial's, not this task's; the pad setpoint is left at 24.0 °C instead of 18.0 °C and is flagged for restore, not restored (no pad writes).
3. **Boot's first ~10 s not captured.** The flash's hard reset came ~7 s before the capture attached; the first captured line is at tick 10311. "None on boot" for (f) is therefore established from 10.3 s onward, which still precedes the worker's first poll (15.9 s) and the seed read.
4. Flash output progress-bar lines are collapsed to their 100 % state in §2 (the raw log has one bar per 16 KiB chunk); the surrounding lines are verbatim.

## 9. What remains — §6 items 2 through 7, the next tasks

Item 1 is done by this report. Still on the bench, each needing a pad write or a special condition this task was forbidden:

2. **Wake poll** — setpoint changed from the app while in STANDBY, knob touched 5 s later, face shows the new value within ~3 s.
3. **External change in STANDBY** — app change with Standby face = Temperature; dimmed face updates within five minutes; restore the setpoint afterwards.
4. **Night face at STANDBY** — Tokyo-timezone trick; number/water alternation runs at the dim floor and the water number tracks.
5. **Outage in STANDBY** — pad unplugged or blocked; `PH_DEGRADED` after ~15 minutes, staleness dot, fresh poll within seconds of wake, recovery on restore. The item carrying the most weight.
6. **One unattended night** — the gate for the tag; the earlier overnight proof ran at 10 s and does not carry over.
7. **Unattended OTA still fires** at STANDBY if an update is available — watched once.

Before any of them: put the pad's setpoint back to 18.0 °C. Also open from `REPORT-standby-poll-code-checks.md`: the water-reading age on the dimmed Temperature face at 300 s (item 4 is the natural place to look). The version bump to `somnus-v1.0.2-beta.1` and the tag wait on item 6.
