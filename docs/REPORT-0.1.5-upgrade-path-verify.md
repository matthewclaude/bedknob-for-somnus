# REPORT: 0.1.4 → 0.1.5 OTA upgrade path, verified on hardware

**Verdict: PASS. The bench dial, wire-restored to 0.1.4 with NVS intact, found stable 0.1.5 on its first manual check (`latest 0.1.5, running 0.1.4 -- update available`), downloaded and verified it in ~21 s, rebooted into it, booted as `App version: 0.1.5`, reconnected to the pad and cancelled rollback within 4 s. Whole sequence completed inside the 240 s window. Main checkout untouched; worktree removed.**

Date: 2026-09-05 01:44–01:53 UTC (2026-09-04 evening local)
Hardware test only. No commits, tags, pushes, or edits to tracked files.

## Gate checks

| # | Check | Observed | Result |
|---|---|---|---|
| 1 | `git rev-parse HEAD` (main repo, `firmware/somnus-port`) | `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56` | PASS |
| 2 | `git cat-file -t somnus-v0.1.4` / `git rev-parse somnus-v0.1.4` | `commit` / `56aded3bf862b1bd2b6ab36c1d2c0e4f2ca86329` | PASS |
| 3 | `ls /dev/cu.usbmodem*` | `/dev/cu.usbmodem83401` | PASS |
| 4 | `git status --porcelain` baseline (recorded, non-blocking) | 17 `??` untracked lines (reports/plan/"Claude outputs/"), 0 tracked modifications | recorded |

## Worktree and build

```
git worktree add /tmp/somnus-0.1.4-wt somnus-v0.1.4
  -> Preparing worktree (detached HEAD 56aded3)
```

Worktree HEAD `56aded3bf862b1bd2b6ab36c1d2c0e4f2ca86329`; its `CMakeLists.txt` PROJECT_VER
= `0.1.4`.

ESP-IDF environment: `source ~/esp/esp-idf/export.sh` (ESP-IDF v6.0, `idf.py` at
`~/esp/esp-idf/tools/idf.py`) — same install as every prior build in this repo.

```
idf.py set-target esp32s3      -> exit 0 (configured, build dir written)
idf.py build                   -> exit 0
```

- Errors in build log: **0** (`grep -c -i 'error[: ]'` = 0).
- `somnus-dial.bin binary size 0x179c60 bytes` (1,547,360 bytes; 63% of the 4 MB slot free).
- Version, from the built binary's app descriptor
  (`esptool image-info build/somnus-dial.bin`): **`App version: 0.1.4`**, Project name
  `somnus-dial`, compiled Sep 4 2026 20:47:09. (`strings` on the binary also yields `0.1.4`.)

## Flash (wire)

Command run: `idf.py -p /dev/cu.usbmodem83401 flash` → exit 0 at 01:48:32Z.

esptool command as printed by idf.py:

```
esptool --chip esp32s3 -p /dev/cu.usbmodem83401 -b 460800 --before=default-reset --after=hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x19000 ota_data_initial.bin 0x20000 somnus-dial.bin
```

Writes performed (all `Hash of data verified`):

| Offset | Image | Bytes |
|---|---|---|
| 0x00000 | bootloader.bin | 22,576 |
| 0x08000 | partition-table.bin | 3,072 |
| 0x19000 | ota_data_initial.bin (otadata) | 8,192 |
| 0x20000 | somnus-dial.bin (ota_0) | 1,547,360 |

**NVS not written — confirmed.** The partition table (`partitions.csv`) places `nvs` (64K)
first after the table, i.e. 0x9000–0x19000; nothing in the flash args targets that
range. The only data partition written is `otadata` at 0x19000 (resets boot slot to
ota_0, which is what makes the freshly-flashed 0.1.4 boot). Wi-Fi, pad address and
timezone therefore survive — borne out below: the 0.1.4 boot reached the pad without
re-provisioning. Finished with `Hard resetting via RTS pin...`.

## Serial capture

```
sleep 10
stty -f /dev/cu.usbmodem83401 115200 raw
( cat /dev/cu.usbmodem83401 & echo $! > /tmp/serial_cap2.pid ) > /tmp/upgrade_path_capture.log 2>&1
sleep 240
kill $(cat /tmp/serial_cap2.pid)
```

(This shell blocks a foreground `sleep`, so the 10 s boot wait, capture start, 240 s
window and kill ran as one background job. `idf.py monitor` was not used.)

```
capture started 01:48:55 pid=25583
kill exit=0
capture stopped 01:52:55
bytes: 10734 lines: 198
```

Owner's actions on the dial during the window (as instructed): confirmed About = 0.1.4,
Settings → Update, Beta builds Off, Check for updates, accepted the 0.1.5 offer.

### Every line matching `ota` or `app:` (case-insensitive), verbatim, in order

Prefix = line number in `/tmp/upgrade_path_capture.log`. Timestamps are ms since boot;
they restart at line 54 because the dial rebooted into the new image.

```
1:I (23806) app: side A: on=1 set=20.0C water=20.1C
2:I (34139) app: side A: on=1 set=20.0C water=20.0C
4:I (56921) ota: latest 0.1.5, running 0.1.4 -- update available
5:I (67425) esp_https_ota: Starting OTA...
6:I (67426) esp_https_ota: Writing to <ota_1> partition at offset 0x420000
19:I (88196) ota: OTA image written and verified; ready to reboot
20:I (88199) app: OTA image ready; rebooting into it
22:I (88208) wifi:pm stop, total sleep time: 43029906 us / 86734846 us
54:I (66) boot:  1 otadata          OTA data         01 00 00019000 00002000
56:I (79) boot:  3 ota_0            OTA app          00 10 00020000 00400000
57:I (86) boot:  4 ota_1            OTA app          00 11 00420000 00400000
121:I (1220) ota: boot pending-verify: true (rollback armed)
149:I (1344) app: knob ready on GPIO8/7
179:I (3429) app: pad connected at http://192.168.1.169:8080
180:I (3838) app: side A: on=1 set=19.0C water=19.8C
181:I (3865) ota: app marked valid; rollback cancelled
184:I (24054) app: side A: on=1 set=19.0C water=19.7C
185:I (68669) app: side A: on=1 set=19.0C water=19.5C
188:I (75873) app: side A: on=1 set=19.0C water=19.5C
189:I (78394) app: side A: on=1 set=19.0C water=19.5C
190:I (80579) app: side A: on=1 set=19.0C water=19.5C
191:I (91401) app: side A: on=1 set=19.0C water=19.5C
192:I (101682) app: side A: on=1 set=19.0C water=19.5C
193:I (112287) app: side A: on=1 set=19.0C water=19.5C
194:I (122564) app: side A: on=1 set=19.0C water=19.4C
195:I (133280) app: side A: on=1 set=19.0C water=19.4C
196:I (143562) app: side A: on=1 set=19.0C water=19.4C
197:I (154274) app: side A: on=1 set=19.0C water=19.4C
198:I (164550) app: side A: on=1 set=19.0C water=19.4C
```

### The specific things asked about

- **`latest 0.1.5, running 0.1.4 -- update available`** — YES, line 4, at 56.9 s after the
  0.1.4 boot. This is the stable channel (`/releases/latest`) on the old 0.1.4 code.
- **Download / write** — `esp_https_ota: Starting OTA...` at 67.4 s (≈10 s after the
  offer; the owner tapping through), writing to `ota_1` at 0x420000. The intervening
  raw lines 7–18 are `esp_image` segment listings (two passes: write-time and
  verify-time) — no per-percent progress lines in this firmware's log. `ota: OTA image
  written and verified; ready to reboot` at 88.2 s. Download+verify ≈ 20.8 s for
  1,609,872 bytes (~77 KB/s; no repeat of the ~3 KB/s anomaly noted in §9.7).
- **Reboot** — line 20 `app: OTA image ready; rebooting into it`, Wi-Fi torn down (line 22),
  bootloader banner follows (lines 42+), bootloader partition listing shows `ota_1` (line 57).
- **New version actually booted** — raw lines 90–92 (not in the ota/app: filter, quoted
  here because they are the proof):

  ```
  90:I (793) app_init: Project name:     somnus-dial
  91:I (797) app_init: App version:      0.1.5
  92:I (801) app_init: Compile time:     Sep  5 2026 01:07:18
  ```

  Compile time matches the CI build for `somnus-v0.1.5` (run 33935053719, built
  01:04–01:10Z). The image installed is the published stable release, not a local build.
- **Rollback** — line 121 `boot pending-verify: true (rollback armed)`, then after Wi-Fi
  and pad came up (line 179, pad connected without re-provisioning — NVS survived), line
  181 `app marked valid; rollback cancelled` at 3.9 s. From there the dial ran normally
  for the remaining ~2.7 min of the window (lines 184–198, steady side-A telemetry).
- **Window sufficient?** Yes. Offer at 57 s, install done at 88 s, new image valid at
  ~92 s of a 240 s capture. Nothing was still in progress when the capture stopped.

## Worktree cleanup

```
git worktree remove /tmp/somnus-0.1.4-wt   -> exit 0
git worktree list
  /Users/matthew/Projects/somnus-waveshare-rotary-dial  1f527bb [firmware/somnus-port]
ls -d /tmp/somnus-0.1.4-wt                  -> No such file or directory
```

Post-cleanup, main repo:

- `git rev-parse HEAD` = `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56`, branch
  `firmware/somnus-port` — unchanged.
- `git status --porcelain`: identical to the gate-4 baseline (same 17 `??` lines, zero
  tracked modifications). This report file is the only addition, itself untracked.

## What this closes

- SPEC-ota-readiness §9.7's "still not exercised": a dial graduating from below onto
  stable 0.1.5 via `/releases/latest` — now observed. (Beta.N → 0.1.5 graduation still
  not observed on hardware; the bench unit came from 0.1.4, not a beta.)
- §9.9's practical consequence for stable users: a 0.1.4 dial with Beta builds **off**
  reaches 0.1.5 cleanly. The Beta-**on** case on 0.1.4 was not exercised here (toggle was
  Off per the test plan); `REPORT-0.1.5-ci-check.md` shows 0.1.5 sits first in the
  five-entry list, so it should also succeed, but that remains unobserved on hardware.
