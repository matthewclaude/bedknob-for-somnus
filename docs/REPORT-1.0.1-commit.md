# REPORT: 1.0.1 commit — the 1.0.1-beta.1 build renumbered; stable release notes — 2026-09-11

**DONE** — the three release edits (`PROJECT_VER` 1.0.1, CHANGELOG `## 1.0.1 — 2026-09-11` section, README current-release line) committed as `3afea98bf6248e44ce885c6175a2e5eafbfceb72` (parent `800f0cd`). `idf.py build` exit 0; binary shows 3 / 0 / 0 for the §7.1 repo-identity check and 0 for `1\.0\.1-beta`; the workflow's extractor selects exactly the new section. **Not tagged, not pushed, no release run started.** One deviation to read before tagging: a pre-existing, configure-time CMake deprecation warning from an unchanged line (Deviations, item 1).

## Gate (all four passed)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1-beta.1")
$ git tag -l 'somnus-v1.0.1'
$ grep -c '^## 1\.0\.1 ' CHANGELOG.md
0
$ git --no-optional-locks status --short --untracked-files=no
$
```

Checks 2 and 4 printed nothing.

Premises verified before editing: `project(somnus-dial)` is line 22 of the CMakeLists; README line 5 is `> \`somnus-v1.0.0\`.` with the disclaimer on lines 3–4; README lines 53 and 212 and `docs/ARCHITECTURE.md` line 198 name `somnus-dial-releases` and were left alone; `## 1.0.1-beta.1 — 2026-09-10 (beta)` is the first release heading in CHANGELOG.md (line 25); the extractor awk in `.github/workflows/release.yml` lines 63–67 has the quoted shape.

## The three edits

### 1. `firmware/dial-idf/CMakeLists.txt` line 21

```
-set(PROJECT_VER "1.0.1-beta.1")
+set(PROJECT_VER "1.0.1")
```

Line 22 `project(somnus-dial)` unchanged; nothing else in the file touched.

### 2. `CHANGELOG.md` — new section above `## 1.0.1-beta.1`

Before: no `## 1.0.1 ` section; line 25 was the `## 1.0.1-beta.1 — 2026-09-10 (beta)` heading, directly after the "Releases marked **(beta)** are prereleases…" paragraph.

After (inserted at line 25; the beta heading now at line 43, unchanged):

```
## 1.0.1 — 2026-09-11

The dial now checks for firmware updates at the project's own repository,
`matthewclaude/bedknob-for-somnus`, where the source code also lives.
Nothing else changes — same screens, same behaviour, same pad control.
This is `1.0.1-beta.1` graduated to a stable release.

A dial on 1.0.0 is offered this update as usual; once it is installed,
updates come from the new location automatically. Flashing a dial from
the browser flasher continues to work exactly as before.

Releases are being published to both the old and the new locations during
this changeover, and a dial that never installs 1.0.1 will keep looking at
the old location — so update it while both are still being published.

Internal: the `1.0.1-beta.1` build renumbered; no code change beyond the
version string.
```

### 3. `README.md` line 5

```
-> `somnus-v1.0.0`.
+> `somnus-v1.0.1`.
```

## (a) Build

`. ~/esp/esp-idf/export.sh; cd firmware/dial-idf; idf.py build`, exit 0. CMake re-configured (the CMakeLists changed). Last 40 lines, verbatim except that the home directory is written as `~`:

```
[ 96%] Built target __idf_lcd_bl_pwm_bsp
[ 96%] Built target __idf_lcd_touch_bsp
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
NOTE: ~/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: 
BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 
'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 
'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is 
treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:235: 
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
Generated ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189930 bytes. Smallest app partition is 0x400000 bytes. 0x2766d0 bytes (62%) free.
[100%] Built target app_check_size
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

No errors. No warnings from the compile or link. The three `NOTE:` lines (eleven with their wrapped continuations) are ESP-IDF's own Kconfig complaints about upstream components, as on every build. The very first lines of the log, however, are a CMake deprecation warning — see Deviations, item 1:

```
CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.
This warning is for project developers.  Use -Wno-author or -Wno-deprecated
```

## (b) Repo-identity check and (d) beta version string

Run from `firmware/dial-idf` after the build above:

```
$ strings build/somnus-dial.bin | grep -c 'repos/matthewclaude/bedknob-for-somnus'
3
$ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
0
$ strings build/somnus-dial.bin | grep -c 'somnus-waveshare-rotary-dial'
0
$ strings build/somnus-dial.bin | grep -c '1\.0\.1-beta'
0
```

3, 0, 0 as §7.1 requires; (d) is 0. For completeness, `strings build/somnus-dial.bin | grep -c -x '1.0.1'` is 1.

## (c) Release-notes extractor, `ver=1.0.1`

The awk from `.github/workflows/release.yml` lines 63–67, followed by the workflow's blank-line trim (line 69), run locally against the committed CHANGELOG.md. Full extracted text:

```
The dial now checks for firmware updates at the project's own repository,
`matthewclaude/bedknob-for-somnus`, where the source code also lives.
Nothing else changes — same screens, same behaviour, same pad control.
This is `1.0.1-beta.1` graduated to a stable release.

A dial on 1.0.0 is offered this update as usual; once it is installed,
updates come from the new location automatically. Flashing a dial from
the browser flasher continues to work exactly as before.

Releases are being published to both the old and the new locations during
this changeover, and a dial that never installs 1.0.1 will keep looking at
the old location — so update it while both are still being published.

Internal: the `1.0.1-beta.1` build renumbered; no code change beyond the
version string.
```

It is the new section only: nothing from the `1.0.1-beta.1` section (no "Beta builds turned on", no `dial_ota.c`, no 2026-09-10) appears. The exact-match clause is what excludes the beta: `## 1.0.1-beta.1 — …` neither equals `## 1.0.1` nor starts with `## 1.0.1 ` (space).

## Diff and commit

```
$ git --no-optional-locks diff --stat HEAD~1 HEAD
 CHANGELOG.md                     | 18 ++++++++++++++++++
 README.md                        |  2 +-
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 20 insertions(+), 2 deletions(-)
```

Commit `3afea98bf6248e44ce885c6175a2e5eafbfceb72` — `release: somnus-v1.0.1 — the 1.0.1-beta.1 build renumbered; OTA client polls bedknob-for-somnus`. After it, `git --no-optional-locks status --short --untracked-files=no` printed nothing and `git tag -l 'somnus-v1.0.1'` printed nothing.

## Deviations

1. **The build is not literally warning-free.** The first eight lines of the log are `CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required): Compatibility with CMake < 3.10 will be removed from a future version of CMake.` Line 5 is `cmake_minimum_required(VERSION 3.5)`, unchanged since upstream wrote it on 2026-07-08 (`d3ef41a1`), and the warning is emitted by the local CMake 4.4.2 at configure time only, which is why no earlier release report records it (`docs/REPORT-1.0.1-beta.1-commit.md` quotes only the last 40 lines and says "no warnings from project code"). It is not caused by this commit and cannot be fixed without touching a CMakeLists line outside the `PROJECT_VER` scope, so the commit was made and the warning is recorded here instead. Fixing line 5 belongs to a separate change.
2. **The CHANGELOG's "graduates" sentence is phrased as** "This is `1.0.1-beta.1` graduated to a stable release." rather than "This graduates 1.0.1-beta.1 to stable", to keep the paragraph's register (each sentence says what the reader has). Same content.
3. **The migration sentence is three lines, not one**, wrapped at the file's ~72-column width like its neighbours. It is one sentence. It names no removal date.

No other deviations. Nothing under `.github/`, `web-flasher/` or `docs/` (other than this report and its index line) changed; `firmware/` changed only on the `PROJECT_VER` line.

## Not verifiable without hardware

- The §7.1 bench pass: a dial on `1.0.1-beta.1` with Beta builds off being offered `1.0.1` from `bedknob-for-somnus`, installing it and rebooting to `1.0.1`, with the cat-based serial capture. That belongs to the tag block, after both Releases exist and outside the ~6-minute window.
- That the published Release body on both repos equals the extracted text above; the extractor was run locally with the workflow's awk and trim, but the run itself has not happened.
- Not flashed. The binary was built and inspected with `strings` only.
