# REPORT: SPEC-standby-poll — implementation (STANDBY cadence 300 s), build only

**Verdict: DONE.** Two commits on `main`, not pushed, no tag, `PROJECT_VER` still `1.0.1`, nothing flashed, the dial untouched. Commit 1 (docs) `e4967d4`; commit 2 (firmware, `main/main.c` only) `39af8f5`. `idf.py build` exited 0 with no warnings from project code.

Date: 2026-09-11. Branch `main`, base `217a896`.

## 1. Gate — raw output of all four checks

```
$ grep -c 'POLL_STANDBY_US 300000000' docs/SPEC-standby-poll.md
1
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1")
$ sed -n '1372p' firmware/dial-idf/main/main.c
        int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All four passed.

## 2. Commit 1 — `e4967d4` — docs/SPEC-standby-poll.md §5, first bullet

Before (line 89):

```
- The staleness dot (`scr_dial.c` ~431): is it driven by poll **failure** or by poll **age**? This check matters more at 300 s than it did at 60 s: if it is age-based with a threshold under 300 s, the dimmed face would show the stale dot for most of every five-minute window. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so.
```

After (line 89):

```
- The staleness dot (the `s_stale_dot` block in `scr_dial.c`'s on_state render, `apply_palette_and_state()`): is it driven by poll **failure** or by poll **age**? This check matters more at 300 s than it did at 60 s: if it is age-based with a threshold under 300 s, the dimmed face would show the stale dot for most of every five-minute window. If so, the threshold moves with the cadence (e.g. `2 × current due`), and the spec is amended to say so. *Answered 2026-09-11, `docs/REPORT-standby-poll-code-checks.md`: failure-driven — `stale = (phase != PH_READY) || !device_online`, no poll age is computed — so no threshold moves with the cadence and nothing in this bullet needs re-opening.*
```

The enclosing function was verified on disk: the `s_stale_dot` block (`scr_dial.c` 726–743 on the current tree) is inside `apply_palette_and_state()` (declared at line 527), which the file's own comments describe as re-applied from `on_state` on every render. `git diff --stat`: `docs/SPEC-standby-poll.md | 2 +-`.

## 3. Commit 2 — `39af8f5` — firmware/dial-idf/main/main.c, every hunk

### 3.1 Constant (landed at lines 72–79)

Before (lines 70–71, unchanged, shown for position):

```c
#define POLL_CONFIRM_US   2000000    // 2s between the confirm polls after a write
#define POLL_CONFIRM_N          3    // how many of them before returning to idle cadence
```

After (lines 70–79; 72–79 new):

```c
#define POLL_CONFIRM_US   2000000    // 2s between the confirm polls after a write
#define POLL_CONFIRM_N          3    // how many of them before returning to idle cadence
/*
 * ...and idle cadence itself follows the display tier. Once the face has gone
 * to STANDBY nobody is reading what the poll fetches, so back off to once
 * every five minutes and let the radio (and the pad's HTTP server) go quiet
 * overnight; ACTIVE and DIMMED keep the 10s above. Leaving STANDBY forces the
 * next read immediately (through the same quiet gate) — docs/SPEC-standby-poll.md.
 */
#define POLL_STANDBY_US 300000000    // STANDBY cadence: once every five minutes
```

`KNOB_SETTLE_US`, `POLL_INTERVAL_US`, `POLL_CONFIRM_US`, `POLL_CONFIRM_N` untouched.

### 3.2 Last-seen tier, a `worker_task` local (landed at lines 1122–1125)

Before (lines 1113–1114):

```c
    int poll_failures = 0;
    for (;;) {
```

After (lines 1121–1126):

```c
    int poll_failures = 0;
    // Display tier as of the previous tick — the STANDBY-exit edge below needs
    // a transition, which a single dial_power_level() read can't give. Seeded
    // from a real read so the first tick is a level, never an edge.
    dial_power_level_t last_pwr_level = dial_power_level();
    for (;;) {
```

Seeded from a real read, so boot produces no cadence log line: the first tick is a level, not a change.

### 3.3 Tier sample, edge detection, both log lines, and the due expression (landed at lines 1380–1409)

Before (lines 1368–1372):

```c
        // No command this tick. Resync only when quiet AND due — "due" being
        // sooner while we're still confirming a write the user just made.
        int64_t now = esp_timer_get_time();
        if (now - dial_state_last_input_us() < KNOB_SETTLE_US) continue;
        int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
```

After (lines 1380–1409):

```c
        // No command this tick. Resync only when quiet AND due — "due" being
        // sooner while we're still confirming a write the user just made,
        // and much later while the face is in STANDBY (POLL_STANDBY_US).
        //
        // One fresh tier sample per tick, used for both the cadence choice and
        // the STANDBY-exit edge (deliberately not the update-prompt block's
        // ota_pwr_level: this gate owns its own read). A spinlock-guarded read
        // of one enum, documented safe from this task.
        dial_power_level_t pwr_level = dial_power_level();
        if (pwr_level != last_pwr_level) {
            bool was_standby = (last_pwr_level == DPWR_STANDBY);
            bool is_standby  = (pwr_level == DPWR_STANDBY);
            if (was_standby && !is_standby) {
                // Leaving STANDBY: the face is about to be looked at and the
                // last read may be minutes old. Same idiom as the command
                // paths above; the quiet gate below still applies, so this
                // lands ~KNOB_SETTLE_US after the input that woke it.
                last_poll_us = 0;                  // read it back now, not in 10s
                ESP_LOGI(TAG, "poll: active cadence 10s");
            } else if (is_standby && !was_standby) {
                ESP_LOGI(TAG, "poll: standby cadence 300s");
            }
            // ACTIVE <-> DIMMED is not a cadence change (both 10s): no log.
            last_pwr_level = pwr_level;
        }
        int64_t now = esp_timer_get_time();
        if (now - dial_state_last_input_us() < KNOB_SETTLE_US) continue;
        int64_t due = poll_confirms > 0          ? POLL_CONFIRM_US
                    : pwr_level == DPWR_STANDBY  ? POLL_STANDBY_US
                    :                              POLL_INTERVAL_US;
```

Where each required piece landed:

| Piece | Line(s) |
|---|---|
| `#define POLL_STANDBY_US 300000000` | 79 |
| `last_pwr_level` declaration | 1125 |
| fresh sample `pwr_level = dial_power_level()` | 1388 |
| edge test `pwr_level != last_pwr_level` | 1389 |
| `last_poll_us = 0;` on STANDBY exit | 1397 |
| `ESP_LOGI(TAG, "poll: active cadence 10s")` | 1398 |
| `ESP_LOGI(TAG, "poll: standby cadence 300s")` | 1400 |
| `last_pwr_level = pwr_level` | 1403 |
| `KNOB_SETTLE_US` gate (unchanged text) | 1406 |
| `due` expression | 1407–1409 |
| `if (now - last_poll_us < due) continue;` (unchanged) | 1410 |

Priority in `due`: confirm polls, then STANDBY, then idle — as specified. Log lines fire only when the STANDBY membership of the tier changes; ACTIVE↔DIMMED updates `last_pwr_level` silently. The `TAG` is the file's existing `"app"`.

## 4. How the tier was sampled, and `ota_pwr_level`

One call to `dial_power_level()` per loop iteration, at line 1388, at the top of the due-computation block. The same `pwr_level` value feeds the edge test (1389) and the `due` expression (1408). It is a new local, not `ota_pwr_level`; `ota_pwr_level` (line 1213 after the shift, 1201 before) and `s_ota_prev_pwr_level` are unchanged and unreferenced by the new code. `grep -n 'pwr_level' main.c` shows the only uses of `ota_pwr_level` are the two pre-existing ones (its assignment and the OTA entry gate).

Premise correction, recorded for the reviewer: the task said `ota_pwr_level` "lives inside the `dial_time_now()` branch and does not exist when time is not yet valid." On disk that sample is at loop scope, and its own comment (lines 1205–1212) says it is "sampled every idle tick regardless of clock validity". The instruction not to reuse it was followed regardless; the correction changes nothing about the work, only the stated reason.

## 5. Build — raw tail, unfiltered

Command: `. ~/esp/esp-idf/export.sh ; cd firmware/dial-idf ; idf.py build` — exit status 0. Full log kept in the session scratchpad; last 40 lines:

```
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
[100%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
[100%] Linking C static library libmain.a
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
NOTE: /Users/matthew/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: 
BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 
'n' are allowed). Value is treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 
'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is 
treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:235: 
FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value (only 'y' and 'n' are 
allowed). Value is treated as 'n'.
[100%] Built target __ldgen_output_sections.ld
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189a20 bytes. Smallest app partition is 0x400000 bytes. 0x2765e0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
or
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x19000 build/ota_data_initial.bin 0x20000 build/somnus-dial.bin
or from the "/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build" directory
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash "@flash_args"
```

Warnings: `grep -n warning build.log` (excluding the CMake deprecation) returned nothing — no warnings from project code. The three `NOTE:` lines are ESP-IDF's own Kconfig files (`bt`, `fatfs`), pre-existing and not project code. The `cmake_minimum_required(VERSION 3.5)` deprecation warning did **not** appear this run: the build directory was already configured, so CMake did not re-run the configure step; it will reappear on the next reconfigure and is not caused by this change. Nothing was flashed; `idf.py monitor` was not run.

The build modified no tracked file: `git --no-optional-locks status --short --untracked-files=no` after the build showed only ` M firmware/dial-idf/main/main.c`, the pending commit-2 edit.

## 6. `git diff --stat` and SHAs

Commit 1 `e4967d4` — `docs: SPEC-standby-poll §5 — staleness-dot pointer names the code, not a line; record the answer`

```
 docs/SPEC-standby-poll.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

Commit 2 `39af8f5` — `firmware: STANDBY poll cadence 300 s (docs/SPEC-standby-poll.md)`

```
 firmware/dial-idf/main/main.c | 41 +++++++++++++++++++++++++++++++++++++++--
 1 file changed, 39 insertions(+), 2 deletions(-)
```

Not pushed. No tag. `PROJECT_VER` remains `1.0.1`.

## 7. Deviations

1. **Sample placement relative to the settle gate.** The task said to take the sample "at the due computation". It sits at the top of that block (line 1388), *above* the `KNOB_SETTLE_US` `continue` (1406) rather than between it and the `due` line. Reason: a wake is caused by input, so the first few ticks after a STANDBY exit hit the settle-gate `continue`; sampling above it means the edge is seen on the first tick and the `poll: active cadence 10s` line lands at the wake, not 2.5 s later. The gate itself is unchanged and still governs when the forced poll fires, so the wake poll lands ~2.5 s after the last input exactly as specified. If the reviewer would rather the sample sit below the gate, it is a two-line move with the same behaviour for the poll and a 2.5 s later log line.
2. **Premise correction** (§4 above): `ota_pwr_level` is at loop scope, not inside the `dial_time_now()` branch. Followed the instruction as written; recorded the mismatch rather than stopping, since it did not affect any edit, line number or quoted line the task depends on.
3. **Build ran before commit 2 was made**, not after, so a failed build would not have been committed. Commit content is identical either way.

Observations, not deviations:

- `docs/SPEC-standby-poll.md` line 38 (§2) carries the same stale pointer, "`scr_dial.c`, around line 431". The task scoped commit 1 to the §5 bullet, so line 38 is untouched. Worth the same reword in a later docs pass.
- The forced `last_poll_us = 0` on STANDBY exit is belt-and-braces on the current constants: after more than 10 s in STANDBY the poll is already overdue at the 10 s idle cadence, so the zeroing only changes behaviour when the last STANDBY poll happened within 10 s of the wake. It is still the right shape (it does not depend on the two intervals' ratio) and is what the spec asks for.
- Boot logs neither cadence line, because `last_pwr_level` is seeded from a real read. §6 item 1 looks for the STANDBY line on the transition, which is unaffected.

## 8. What needs the bench — the next task

Flash by wire (`/dev/cu.usbmodem83401`, pyserial capture, never `idf.py monitor`), then `docs/SPEC-standby-poll.md` §6, by name:

1. **Cadence, by log line and by gap** — `poll: standby cadence 300s` on the transition, next three poll lines ~300 s apart over 15 minutes; then wake, `poll: active cadence 10s`, ~12 polls over 2 minutes at ACTIVE.
2. **Wake poll** — setpoint changed from the app while in STANDBY, knob touched 5 s later, face shows the new setpoint within ~3 s.
3. **External change in STANDBY** — dimmed Temperature face updates within five minutes; restore the setpoint afterwards.
4. **Night face at STANDBY** — number/water alternation still runs at the dim floor and the water number tracks.
5. **Outage in STANDBY** — `PH_DEGRADED` after ~15 minutes, staleness dot, fresh poll attempt within seconds of wake, recovery on restore. The item carrying the most weight.
6. **One unattended night** — the gate for the tag; the earlier overnight proof ran at 10 s and does not carry over.
7. **Unattended OTA still fires** at STANDBY if an update is available — watched once.

Also to confirm on the bench, from `REPORT-standby-poll-code-checks.md`: the water-reading age on the dimmed Temperature face at 300 s. The version bump to `somnus-v1.0.2-beta.1` and the tag belong to the release commit after the bench passes, per `docs/SPEC-standby-poll.md` §8 and `docs/SPEC-repo-consolidation.md` §5.
