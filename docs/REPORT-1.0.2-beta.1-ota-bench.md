# REPORT — 1.0.2-beta.1 beta-channel OTA bench verification

**Verdict: PASS.** The bench dial, running 1.0.1 with Beta builds On, found `somnus-v1.0.2-beta.1` on `matthewclaude/bedknob-for-somnus`, downloaded it, wrote it to `ota_1`, rebooted into it, cancelled the rollback and reported itself up to date. This is the first over-the-air install served from the new repo's Release, and the "finds an update and installs it" half of `SPEC-standby-poll.md` §6 item 7.

Date: 2026-09-13, capture 18:33:54Z onward. Serial capture: `bench-logs/1.0.2-beta1-ota.log` (gitignored, 209 lines). No build, no wire flash, no code change, no commit in this block.

## Gate (raw output, unedited)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2-beta.1")
$ git --no-optional-locks describe --tags --exact-match somnus-v1.0.2-beta.1 2>&1 || git --no-optional-locks rev-parse somnus-v1.0.2-beta.1^{}
somnus-v1.0.2-beta.1
$ git --no-optional-locks status --short --untracked-files=no
$ ls /dev/cu.usbmodem83401
/dev/cu.usbmodem83401
```

The `describe` half succeeded, so the `||` branch never ran and the commit was not printed. Resolved explicitly (addition, not a substitution):

```
$ git --no-optional-locks rev-parse somnus-v1.0.2-beta.1^{}
00bd94d263e36c643cfa27756f3d0463905bad86
$ git --no-optional-locks rev-parse HEAD
5cf284ed5e6cad9b415b347fb3e02425bb491f52
$ git diff --stat 39af8f5 00bd94d -- firmware/
 firmware/dial-idf/CMakeLists.txt | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

All four gate checks met their expected values. HEAD is one commit ahead of the tag (the tag-push report), which the gate allows.

Release object, fetched as an object rather than a summarized page:

```
$ gh release view somnus-v1.0.2-beta.1 --repo matthewclaude/bedknob-for-somnus --json tagName,isDraft,isPrerelease,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/560214835","contentType":"application/octet-stream","createdAt":"2026-09-13T00:16:05Z","digest":"sha256:a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d","downloadCount":0,"id":"RA_kwDOUCLPIM4hZDMz","label":"","name":"somnus-dial-merged.bin","size":1743392,"state":"uploaded","updatedAt":"2026-09-13T00:16:11Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2-beta.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/560214834","contentType":"application/octet-stream","createdAt":"2026-09-13T00:16:05Z","digest":"sha256:764f2228b8555a6b5591a722b959473ed97fd9b77898f24f0e67d3bcd6b973a0","downloadCount":0,"id":"RA_kwDOUCLPIM4hZDMy","label":"","name":"somnus-dial.bin","size":1612320,"state":"uploaded","updatedAt":"2026-09-13T00:16:05Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2-beta.1/somnus-dial.bin"},"isDraft":false,"isPrerelease":true,"publishedAt":"2026-09-13T00:16:03Z","tagName":"somnus-v1.0.2-beta.1"}
```

`downloadCount` was 0 for both assets before this run.

## Step 1 — log format strings (so the greps are auditable)

`firmware/dial-idf/components/dial_ota/dial_ota.c` (`TAG = "ota"`, line 28):

```
$ grep -n 'ESP_LOG[IWE]' firmware/dial-idf/components/dial_ota/dial_ota.c
285:        ESP_LOGI(TAG, "latest %s, running %s -- update available", latest, desc->version);
289:    ESP_LOGI(TAG, "latest %s, running %s -- up to date", latest, desc->version);
309:        ESP_LOGI(TAG, "no releases published yet (HTTP 404)");
316:        ESP_LOGW(TAG, "release check failed: %s (HTTP %d)", esp_err_to_name(err), status);
324:        ESP_LOGW(TAG, "release JSON exceeded %d bytes", CHECK_BUF_CAP);
367:        ESP_LOGW(TAG, "tag list check failed: %s (HTTP %d)", esp_err_to_name(err), status);
375:        ESP_LOGW(TAG, "tag list JSON exceeded %d bytes", CHECK_BUF_CAP);
409:        ESP_LOGI(TAG, "no somnus-v tags, running %s -- up to date", desc->version);
418:        ESP_LOGI(TAG, "latest %s, running %s -- up to date", highest, desc->version);
449:            ESP_LOGW(TAG, "release for %s not found (HTTP 404) -- trying next tag", full_tag);
451:            ESP_LOGW(TAG, "release fetch for %s failed: %s (HTTP %d) -- trying next tag",
454:            ESP_LOGW(TAG, "release JSON for %s exceeded %d bytes -- trying next tag",
459:                ESP_LOGW(TAG, "bad release JSON for %s -- trying next tag", full_tag);
461:                ESP_LOGW(TAG, "%s is a draft -- trying next tag", full_tag);
566:        ESP_LOGW(TAG, "esp_https_ota_begin: %s%s", esp_err_to_name(err),
590:        ESP_LOGE(TAG, "esp_https_ota_perform: %s", esp_err_to_name(err));
599:        ESP_LOGE(TAG, "incomplete image received");
610:        ESP_LOGE(TAG, "esp_https_ota_finish: %s", esp_err_to_name(err));
617:    ESP_LOGI(TAG, "OTA image written and verified; ready to reboot");
631:        ESP_LOGW(TAG, "esp_ota_get_state_partition failed at boot; assuming not pending");
638:    ESP_LOGI(TAG, "boot pending-verify: %s", pending ? "true (rollback armed)" : "false");
646:        ESP_LOGW(TAG, "esp_ota_get_state_partition failed; leaving rollback state as-is");
650:        ESP_LOGI(TAG, "boot not pending verification (state %d) -- nothing to do", ota_state);
654:        ESP_LOGI(TAG, "app marked valid; rollback cancelled");
663:        ESP_LOGE(TAG, "failed to mark app valid / cancel rollback");
```

Coverage by phase:

- Version comparison: line 285 (beta channel, update available) and lines 289 / 418 (up to date; 418 is the beta-channel tag-list path).
- Download: the component logs nothing of its own at download start. The download and write are logged by ESP-IDF's `esp_https_ota` (`~/esp/esp-idf/components/esp_https_ota/src/esp_https_ota.c` line 532 `"Starting OTA..."`, line 543 `"Writing to <%s> partition at offset 0x%" PRIx32`). Download failure would be lines 566 / 590 / 599.
- Write / verify: line 617 `"OTA image written and verified; ready to reboot"`; failure line 610.
- Post-boot rollback: lines 638 and 654 / 663.
- Up-to-date: lines 289 / 418.

The `App version:` banner is ESP-IDF's `app_init`, and the partition-load line is the second-stage bootloader (`boot:`).

## Step 2 — reader

`caffeinate -i -s -t 5400` (pid 57147) and the reattaching `stty` + `cat` loop (pid 57148, `cat` pid 57154) started as backgrounded jobs. The log was truncated to empty before the reader attached. First line of the capture:

```
=== reader attached 2026-09-13T18:33:54Z ===
```

## Step 3 — on-glass sequence (one action per message)

| Step | Instruction | Owner read back |
|---|---|---|
| 3a | Tap to wake; Menu > Update; read Installed | `1.0.1` |
| 3b | Set Beta builds to On; read row | `on` (first reply repeated `1.0.1`; asked again) |
| 3c | Tap Check for updates; read row | `2 beta 1` |
| 3d | Tap the row again, confirm through to Starting install; read row | `installing` |
| 3e | Watch only (no action) | `.2 beta 1` (Installed row after reboot) |
| 3e' | Menu > Update > Check for updates; read row | `up to date` |

The re-check in 3e' was a manual tap: `main.c` line 1120 schedules the first automatic check about 24 h after boot, so no automatic check would have landed within this session.

## Step 4 — extraction (raw)

```
$ wc -l "$LOG"
     209 bench-logs/1.0.2-beta1-ota.log
$ grep -nE "ota:|esp_https_ota:|App version|Partition|boot: |valid|reader (attached|lost)" "$LOG"
1:=== reader attached 2026-09-13T18:33:54Z ===
12:I (1608081) ota: latest 1.0.2-beta.1, running 1.0.1 -- update available
16:I (1630657) esp_https_ota: Starting OTA...
17:I (1630657) esp_https_ota: Writing to <ota_1> partition at offset 0x420000
30:I (1651098) ota: OTA image written and verified; ready to reboot
52:I (24) boot: ESP-IDF v6.0 2nd stage bootloader
53:I (24) boot: compile time Sep  5 2026 18:45:59
54:I (24) boot: Multicore bootloader
55:I (25) boot: chip revision: v0.2
56:I (27) boot: efuse block revision: v1.4
61:I (46) boot: Enabling RNG early entropy source...
62:I (51) boot: Partition Table:
63:I (53) boot: ## Label            Usage          Type ST Offset   Length
64:I (60) boot:  0 nvs              WiFi data        01 02 00009000 00010000
65:I (66) boot:  1 otadata          OTA data         01 00 00019000 00002000
66:I (73) boot:  2 phy_init         RF data          01 01 0001b000 00001000
67:I (79) boot:  3 ota_0            OTA app          00 10 00020000 00400000
68:I (86) boot:  4 ota_1            OTA app          00 11 00420000 00400000
69:I (92) boot:  5 assets           Unknown data     01 82 00820000 007c0000
70:I (99) boot: End of partition table
77:I (393) boot: Loaded app from partition at offset 0x420000
78:I (393) boot: Disabling RNG early entropy source...
102:I (798) app_init: App version:      1.0.2-beta.1
132:I (1222) ota: boot pending-verify: true (rollback armed)
194:I (2855) ota: app marked valid; rollback cancelled
206:I (149295) ota: latest 1.0.2-beta.1, running 1.0.2-beta.1 -- up to date
$ grep -cE "assert|panic|abort|Guru" "$LOG"
0
$ grep -nE "^E \(" "$LOG"
$ grep -c 'POST /api/' "$LOG"
0
```

The grep pattern was widened from the sketched `ota:` to `ota:|esp_https_ota:` because the download and write lines carry the IDF library's tag, not the component's (step 1).

## Items a–h

**a. Beta channel saw the beta — PASS.** Line 12:
`I (1608081) ota: latest 1.0.2-beta.1, running 1.0.1 -- update available`

**b. Download and write completed without error — PASS.** Lines 16, 17 and 30:
```
I (1630657) esp_https_ota: Starting OTA...
I (1630657) esp_https_ota: Writing to <ota_1> partition at offset 0x420000
I (1651098) ota: OTA image written and verified; ready to reboot
```
No `esp_https_ota_begin`, `esp_https_ota_perform`, `incomplete image received` or `esp_https_ota_finish` line anywhere in the capture. Line 31, the app's own follow-up: `I (1651101) app: OTA image ready; rebooting into it`.

**c. Rebooted and printed the new version — PASS.** Line 102:
`I (798) app_init: App version:      1.0.2-beta.1`
The tick counter reset from 1651181 (line 41) to 24 (line 52) is the reboot; the ROM banner at lines 42–51 shows `rst:0xc (RTC_SW_CPU_RST)`, a software reset, not a power loss.

**d. Booted from the other OTA slot and was marked valid — PASS.** Lines 17, 77, 132 and 194:
```
I (1630657) esp_https_ota: Writing to <ota_1> partition at offset 0x420000
I (393) boot: Loaded app from partition at offset 0x420000
I (1222) ota: boot pending-verify: true (rollback armed)
I (2855) ota: app marked valid; rollback cancelled
```
Offset 0x420000 is `ota_1` per the partition table at line 68. No `failed to mark app valid` line.

**e. A later check reports up to date — PASS.** Line 206:
`I (149295) ota: latest 1.0.2-beta.1, running 1.0.2-beta.1 -- up to date`

**f. No assert / panic / abort / Guru; E lines attributed — PASS.** The count is 0 and the `^E \(` grep returned nothing. There are three `W (` lines, none an error and none OTA-related:
```
38:W (1651131) net: Wi-Fi lost — reconnecting
127:W (1210) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
169:W (1409) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```
Line 38 is the Wi-Fi driver being torn down by the app's own restart (it follows `wifi:state: run -> init` at line 33). Lines 127 and 169 are the display driver and Wi-Fi driver boot-time notices seen on every boot.

**g. `POST /api/` count — PASS, 0.** The dial did not write to the pad. Every pad line in the capture is a read (`app: side A: on=0 set=18.0C water=23.x C`), and the pad's state is identical before and after: `on=0 set=18.0C` at line 2 (before) and line 209 (after).

**h. Reader markers — one attach, no loss.**
```
1:=== reader attached 2026-09-13T18:33:54Z ===
```
There is no `reader lost device` marker anywhere. The ESP32-S3's USB-Serial-JTAG device stayed enumerated across the software reset, so `cat` never lost the file and the reboot shows in the log as the ROM banner and tick reset at lines 42–52, not as a reader gap. Wall-clock timestamps therefore exist only for the attach; every other time in this report is the firmware's millisecond tick.

## Install duration

The capture has no per-line wall-clock; the two clocks are the firmware ticks before and after the reboot.

| Event | Line | Tick (ms) |
|---|---|---|
| First download line (`Starting OTA...`) | 16 | 1630657 |
| Image written and verified | 30 | 1651098 |
| Last line before reset (`wifi:lmac stop hw txq`) | 41 | 1651181 |
| Reset, tick counter restarts | 52 | 24 |
| `App version: 1.0.2-beta.1` | 102 | 798 |

- Download + write: 1651098 − 1630657 = 20441 ms = **20.4 s**.
- Download line to reset: 1651181 − 1630657 = 20524 ms.
- Reset to App version banner: 798 ms.
- Download line to App version: 20524 + 798 = **21322 ms ≈ 21.3 s**, plus the ROM/bootloader time between the last pre-reset line and tick 0 of the new image, which is not on either clock (typically well under a second).

For comparison, the 1.0.1 stable install on 2026-09-11 measured ~22.7 s for the write alone.

## OTA partition before and after

- After: `ota_1` at offset 0x420000, quoted at line 77 (`Loaded app from partition at offset 0x420000`) and line 17 (written there).
- Before: `ota_0`. The pre-run boot happened before the capture started, so there is no `Loaded app from partition` line for it in this log. The inference is that `esp_https_ota` always writes to the passive slot, and it wrote `ota_1`, so the running slot was `ota_0`. That matches the two events since: `docs/REPORT-1.0.1-bench-gate.md` line 65 (`esp_https_ota: Writing to <ota_0> partition at offset 0x20000`) and line 83 (`boot: Loaded app from partition at offset 0x20000`) for the 1.0.1 install, and the 2026-09-11 wire flash of 39af8f5, which `firmware/dial-idf/build/flash_args` writes at `0x20000 somnus-dial.bin` (`ota_0`) together with `0x19000 ota_data_initial.bin`.

## Build and diff

There was no build and no code change in this run, so there is no build output and no `git diff --stat` to show. The tracked tree was clean before and is clean after; the only new file on disk is the gitignored capture and this report.

## Deviations from the instructions

- The gate's tag check printed the tag name rather than the commit, because `describe --exact-match` succeeded and the `||` fallback never ran. An extra `rev-parse` was run to confirm the commit; the gate command itself was not altered.
- The step-4 grep pattern was widened to include `esp_https_ota:` so the download and write lines (IDF library tag) are captured. The sketched pattern would have missed items b's first two lines.
- The reader loop was launched under `nohup bash -c` rather than as a bare `( … ) &` subshell, and the reader was stopped by pid rather than `kill %2`, because job control in this tool's non-interactive shell does not carry between commands. The loop body was unchanged.
- The capture file was truncated to empty before the reader started, so the log contains only this run.
- Step 3b needed a second ask: the first reply repeated `1.0.1`.
- Item e needed an explicit extra ask (3e') for a manual Check for updates, since the automatic check is 24 h out. Step 3e as written asked for no further action; the instruction's "re-check" could not otherwise happen inside the session.

## What could not be verified

- Nothing here says anything about a stable-channel dial (Beta builds Off) staying on 1.0.1 and not being offered the prerelease.
- Nothing here says anything about whether the browser flasher at `matthewclaude.github.io/bedknob-for-somnus/` can flash a board from the new URL.
- The pre-run OTA slot is inferred, not quoted (see above).
- No wall-clock timestamps exist for anything but the reader attach, so the install duration is tick arithmetic across a reset.

## Closing state

- Dial: running **1.0.2-beta.1** from `ota_1`, image marked valid, last check `up to date`. Serial pad reads resumed normally after the reboot (`pad connected`, side A polls at the active cadence, then `poll: standby cadence 300s` at line 205).
- **Beta builds: left On** — the owner's choice when asked at the end of the run.
- Pad: not touched. No write was sent (`POST /api/` count 0); side A read `on=0 set=18.0C` before and after.
- Reader and `caffeinate` stopped (pids 57147, 57148, 57154 gone). `bench-logs/1.0.2-beta1-ota.log` kept on disk, gitignored.
- Nothing built, flashed by wire, committed, tagged or pushed. This report is untracked until the next docs commit.
