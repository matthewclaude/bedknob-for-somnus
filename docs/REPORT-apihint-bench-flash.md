# 1.0.4-beta.1 bench, part 1: wire-flash of 5e9bc69 and serial capture

**Verdict: PASS.** All four gates passed; the build at 5e9bc69 was up to date and carries the Local API hint string once and the version string 1.0.3-beta.1 only; all four regions flashed with hash verification and a hard reset; the capture loop is running and attach1.log is growing; the pad answered the read-only GET.

Date: 2026-09-30. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch main.

## Gate

| # | Check | Raw output | Result |
|---|---|---|---|
| 1 | `sed -n 21p firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.3-beta.1")` | PASS |
| 2 | `git rev-parse HEAD` | `5e9bc69d6eba9ac271b24772ded918e80a97aab3` | PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | *(empty)* | PASS |
| 4 | `ls /dev/cu.usbmodem*` | `/dev/cu.usbmodem83401` (exactly one) | PASS |

No Xcode licence complaint from git, strings or python3; DEVELOPER_DIR was not set.

## Step 1: old capture

- `bench-logs/capture.pid` existed (dated Sep 30 11:56) and held PID 18095. `ps -p 18095` returned no process: not alive.
- No `cat` reading `/dev/cu.usbmodem*` and no `caffeinate` with `cu.usbmodem` in its command line.
- **Nothing running; nothing killed.**
- (An unrelated `caffeinate -i -t 300`, PID 24384, was seen later during step 4. It does not mention cu.usbmodem, so it is not ours and was left alone.)

## Step 2: build

`. ~/esp/esp-idf/export.sh`, then `idf.py build` in `firmware/dial-idf`. Last 10 lines:

```
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
or
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x19000 build/ota_data_initial.bin 0x20000 build/somnus-dial.bin
or from the "~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build" directory
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash "@flash_args"
```

Strings checks:

- `strings build/somnus-dial.bin | grep -c "Local API enabled"` → `1`
- `strings build/somnus-dial.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'` → `1.0.3-beta.1` (only line)

sha256 of `build/somnus-dial.bin`:

```
4626d913cf12a3aabf53049cd06eb0c4d873c5be831b4948a2511fb01ac79155  build/somnus-dial.bin
```

## Step 3: flash

`idf.py -p /dev/cu.usbmodem83401 flash` in `firmware/dial-idf`, exit code 0. Chip: ESP32-S3 (QFN56) revision v0.2, stub flasher, 460800 baud. Raw lines per region (progress bars omitted):

```
Writing 'bootloader/bootloader.bin' at 0x00000000...
Flash will be erased from 0x00000000 to 0x00005fff...
Compressed 22576 bytes to 14461...
Wrote 22576 bytes (14461 compressed) at 0x00000000 in 0.3 seconds (713.7 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (867.9 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.1 seconds (958.7 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1611568 bytes to 983969...
Wrote 1611568 bytes (983969 compressed) at 0x00020000 in 10.3 seconds (1253.0 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

NVS (0x9000 region) was not written. The otadata region at 0x19000 was rewritten with the initial image, so the dial boots the app at 0x20000; the capture below shows `ota: boot not pending verification (state 2)`.

## Step 4: capture

Started from the repo root with the command exactly as given in the task block, port `/dev/cu.usbmodem83401`. `$!` = **27264**, written to `bench-logs/capture.pid`.

Processes after start (`ps -axo pid,ppid,command`):

| PID | PPID | Role |
|---|---|---|
| 27264 | 1 | `sh -c 'DEV=…; while true; …'` (the loop; PID in capture.pid) |
| 27267 | 27264 | `caffeinate -i -s sh -c …` |
| 27271 | 27264 | `cat /dev/cu.usbmodem83401` (current reader) |

Growth check: `attach1.log` was 318 bytes, then 369 bytes 10 seconds later: **growing**. `bench-logs/capture.out` is empty. Only attach1.log exists (no USB drop so far).

First 20 lines of `bench-logs/2026-09-30-apihint-attach1.log` (7 lines at the time; the pad's address is replaced with `<the pad>`):

```
=== reader attached 2026-09-30T19:49:42Z ===
I (7426) dial_somnus: zone mode set to single (One Bed)
I (7426) app: pad connected at http://<the pad>
I (7490) app: side A: on=0 set=17.0C water=24.1C
I (7490) ota: boot not pending verification (state 2) -- nothing to do
I (10314) power: plugged (4704 mV)
I (18169) app: side A: on=0 set=17.0C water=24.1C
```

As expected, the boot banner is missing: the reader attached about 7 s into the post-flash boot. The dial was not reset; the owner presses RST next to capture a full boot.

## Step 5: pad baseline (one read-only GET)

`curl -s -m 5 …/api/state` answered (curl exit 0):

| Field | side0 | side1 |
|---|---|---|
| is_on | False | False |
| target_t | 17.0 | 17.0 |
| current_t | 24.081024 | 23.88446 |
| is_wl_low | False | False |

`error` = False.

## How to stop the capture

Stop the loop first so it does not start a new reader, then caffeinate, then the current cat:

```sh
cd ~/Projects/somnus-waveshare-rotary-dial
kill 27264                              # loop (bench-logs/capture.pid)
kill 27267                              # caffeinate
pkill -f 'cat /dev/cu.usbmodem83401'    # current reader (27271 at start; a new PID after any USB drop)
ps -axo pid,command | grep cu.usbmodem | grep -v grep   # should print nothing
```

## Deviations

- The full `idf.py flash` output was saved to `bench-logs/2026-09-30-apihint-flash.out` (gitignored via `bench-logs/`) so the write lines could be pulled from it. The block did not ask for that file. It contains the dial's MAC, so it must stay out of the repo.
- In the attach1.log excerpt above, the pad's address is redacted to follow the public-repo rule. The log file itself is unchanged.
- The pad state was parsed from a scratch copy of the GET response outside the repo. There was still only one GET.

## Not verifiable here

Everything the bench checks on the screen: the plain-language Local API hint text and layout when the pad does not answer, the normal faces once the pad answers, and the version shown on the About/version screens. A full boot banner also needs the owner's RST press.
