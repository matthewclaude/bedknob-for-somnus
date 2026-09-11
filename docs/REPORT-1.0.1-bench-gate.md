# REPORT: §7.1 bench gate — 1.0.1-beta.1 → 1.0.1 OTA through bedknob-for-somnus — 2026-09-11

**PASS** — the bench dial, running `1.0.1-beta.1` with Beta builds off, asked the new endpoint, was offered `1.0.1`, installed it in about 22 seconds, rebooted from `ota_0`, printed `App version: 1.0.1` in its boot banner, marked the new app valid (rollback cancelled) and then reported `latest 1.0.1, running 1.0.1 -- up to date`. No assert, panic, abort or Guru line. Three `E (` lines appear about 2.5 minutes after the reboot and are a single pad-poll HTTP connect timeout to the pad on the LAN, unrelated to the OTA. Serial captured with `cat`, not `idf.py monitor`. Nothing flashed, no pad writes, no tag. Owner confirms Menu > About shows **1.0.1** on the screen.

## Gate (all three passed)

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ gh release view somnus-v1.0.1 --repo matthewclaude/bedknob-for-somnus --json tagName,isPrerelease,publishedAt
{"isPrerelease":false,"publishedAt":"2026-09-11T18:34:09Z","tagName":"somnus-v1.0.1"}
$ git --no-optional-locks status --short --untracked-files=no
$
```

Check 1 listed exactly the expected path. Check 2 exists with `isPrerelease` false, published 18:34:09Z (window long closed; not re-waited, per instructions). Check 3 printed nothing.

## Capture — command sequence as run

Run as one background job from the repo root (the shell blocks on a foreground `sleep`), after telling the owner in chat to have the dial at Menu > Update with Beta builds OFF and not to tap Check for updates until told:

```
DEV=/dev/cu.usbmodem83401
stty -f $DEV 115200 raw
( cat $DEV & echo $! > /tmp/serial_cap_101.pid ) > bench-logs/1.0.1-ota.log 2>&1
echo "capture started $(date -u +%T) pid=$(cat /tmp/serial_cap_101.pid)"
sleep 300
kill $(cat /tmp/serial_cap_101.pid); echo "kill exit=$?"
echo "capture stopped $(date -u +%T)"
```

The job's own output:

```
capture started 18:41:10 pid=15806
kill exit=0
capture stopped 18:46:10
```

The owner was told "capture running, tap Check for updates now" immediately after launch. `bench-logs/` is gitignored (`.gitignore` line 28); the log stays local.

## (a) Log size

```
$ wc -c -l bench-logs/1.0.1-ota.log
     215   11670 bench-logs/1.0.1-ota.log
```

215 lines, 11670 bytes.

## (b) Every `ota:` line, in order (with log line numbers)

```
3:I (126647) ota: latest 1.0.1, running 1.0.1-beta.1 -- update available
18:I (154965) ota: OTA image written and verified; ready to reboot
120:I (1221) ota: boot pending-verify: true (rollback armed)
188:I (15382) ota: app marked valid; rollback cancelled
190:I (23009) ota: latest 1.0.1, running 1.0.1 -- up to date
```

Context around the install (not `ota:` lines, quoted for the timeline):

```
4:I (132297) esp_https_ota: Starting OTA...
5:I (132298) esp_https_ota: Writing to <ota_0> partition at offset 0x20000
19:I (154969) app: OTA image ready; rebooting into it
26:W (154998) net: Wi-Fi lost — reconnecting
```

Check at 126.6 s of the old uptime, write started at 132.3 s, image verified at 155.0 s: about 22.7 s to write and verify, matching the beta's install time.

## (c) Boot banner after the reboot

```
30:ESP-ROM:esp32s3-20210327
31:Build:Mar 27 2021
32:rst:0xc (RTC_SW_CPU_RST),boot:0x2b (SPI_FAST_FLASH_BOOT)
33:Saved PC:0x40386280
...
40:I (24) boot: ESP-IDF v6.0 2nd stage bootloader
41:I (24) boot: compile time Sep  5 2026 18:45:59
...
65:I (393) boot: Loaded app from partition at offset 0x20000
...
88:I (790) app_init: Application information:
89:I (794) app_init: Project name:     somnus-dial
90:I (798) app_init: App version:      1.0.1
91:I (802) app_init: Compile time:     Sep 11 2026 18:32:31
92:I (811) app_init: ELF file SHA256:  ed6679d6e...
93:I (811) app_init: ESP-IDF:          v6.0
```

Reset reason is a software CPU reset (the OTA reboot), the app loaded from `ota_0` at 0x20000 (the partition the OTA wrote), and the app version line reads `1.0.1`. The compile time 18:32:31 on Sep 11 is consistent with the release run that published `somnus-v1.0.1` (Release published 18:34:09Z).

## (d) assert / panic / abort / Guru / `E (` lines

assert, panic, abort, Guru: **none**.

Lines starting `E (`: three, all at the same tick, about 143 s after the reboot and about 135 s after the OTA client's "up to date" line:

```
204:E (157804) esp-tls: [sock=54] select() timeout
205:E (157804) transport_base: Failed to open a new connection: 32774
206:E (157804) HTTP_CLIENT: Connection failed, sock < 0
207:W (157807) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
```

The warning on line 207 is from the `dial_somnus` pad client, so this is one failed poll of the pad at `http://192.168.1.169:8080` (line 186), not the OTA path. The pad polls on either side of it (lines 203 and 208) succeeded and reported the same unchanged state (`side A: on=0 set=18.0C water=23.4C`). This does not affect the verdict; it is noted as unrelated.

## (e) Yes/no

- **Offered 1.0.1: yes.** Line 3: `I (126647) ota: latest 1.0.1, running 1.0.1-beta.1 -- update available`.
- **Installed: yes.** Line 18: `I (154965) ota: OTA image written and verified; ready to reboot`, followed by line 19 `app: OTA image ready; rebooting into it` and the reset on line 32.
- **Came back up on 1.0.1: yes.** Line 90: `I (798) app_init: App version:      1.0.1`; line 188: `ota: app marked valid; rollback cancelled`; line 190: `I (23009) ota: latest 1.0.1, running 1.0.1 -- up to date`.

## Pad state

Read-only on the pad throughout. The pad state lines before the OTA (lines 1–2) and after (lines 187–215) all read `side A: on=0 set=18.0C water=23.4C`: the bed's on/off and setpoint are unchanged by this test.

## Menu > About — screen-side confirmation

The owner, asked in chat after the capture what Menu > About shows on the dial, answered **1.0.1**. This matches the boot banner's `App version:      1.0.1` (log line 90) and the OTA client's `running 1.0.1` (log line 190).

## Deviations

No deviations.

## Still unverified

- The log does not name the host the OTA client queried (no URL is printed at INFO level), so the log alone does not show that the request went to `bedknob-for-somnus` rather than the old repo. Repo identity for this build was established at build time by the §7.1 strings check (3/0/0) recorded in `REPORT-1.0.1-commit.md`; and since stable is dual-published, §7.1 says no runtime discriminator exists and none should be sought.
- Whether the three `E (` lines at tick 157804 recur is not known from one 300 s window; they were a single pad-poll connect timeout with successful polls on both sides.
