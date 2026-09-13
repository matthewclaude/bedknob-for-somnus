# REPORT: somnus-v1.0.2-beta.1 release commit — block 1 of 2 (commit only)

**Verdict: DONE.** One commit, `c6261fa`, pairs the `PROJECT_VER` bump to `1.0.2-beta.1` (`firmware/dial-idf/CMakeLists.txt` line 21) with a new `## 1.0.2-beta.1 — 2026-09-13 (beta)` section at the top of `CHANGELOG.md`; `idf.py build` exit 0 with only the known CMake 3.5 deprecation warning; the binary carries the version string once, `bedknob-for-somnus` three times and `somnus-dial-releases` zero times; the workflow's extractor returns exactly the new section. No tag, no push, no flash, no dial or pad contact. README lines 4–5 left at `somnus-v1.0.1`. Block 2 (tag + push) is the owner's.

Date: 2026-09-12 (evening, local; the CHANGELOG section is dated 2026-09-13 as instructed). Branch `main`; parent `6cd1cc3` (items 4/5 bench report); main is now five commits ahead of `somnus/main` (`3da2dcd`) before the report commit.

## 1. Gate — raw output

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1")
$ grep -n 'define POLL_STANDBY_US' firmware/dial-idf/main/main.c
79:#define POLL_STANDBY_US 300000000    // STANDBY cadence: once every five minutes
$ grep -n '^## ' CHANGELOG.md | head -3
25:## 1.0.1 — 2026-09-11
43:## 1.0.1-beta.1 — 2026-09-10 (beta)
58:## 1.0.0 — 2026-09-09
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All four passed.

## 2. The change

`firmware/dial-idf/CMakeLists.txt` line 21: `set(PROJECT_VER "1.0.1")` → `set(PROJECT_VER "1.0.2-beta.1")`.

`CHANGELOG.md`, new section inserted immediately above `## 1.0.1 — 2026-09-11`, quoted in full:

```
## 1.0.2-beta.1 — 2026-09-13 (beta)

While the screen is off and the dial has gone to standby, it now asks the
pad for its state once every five minutes instead of every ten seconds.
Touching the knob still reads the pad immediately, so the face is current
the moment you look at it. Nothing about the screens or pad control
changes.

Internal: `POLL_STANDBY_US` at 300 s in the worker loop's due computation;
the cadence is logged when crossing into or out of standby; a pad outage
in standby now shows as stale after about fifteen minutes instead of three.
```

`README.md` lines 4–5 untouched (still name `somnus-v1.0.1` as the current stable).

## 3. Build — `idf.py build` from `firmware/dial-idf`

Exit code: **0**. Configure-time warning, raw (the known, expected one; not fixed):

```
CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.
This warning is for project developers.  Use -Wno-author or -Wno-deprecated
to suppress it.
```

Also at configure time, three Kconfig `NOTE:` lines from ESP-IDF's own components (fatfs `FATFS_PRINT_LLI` / `FATFS_PRINT_FLOAT` "'default 0' is not a valid bool value", and a bt `sdkconfig.rename.esp32s3` duplicate mapping) — ESP-IDF-side, pre-existing, not from this project. No compiler warnings in the project sources.

Tail of the output, raw:

```
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

## 4. Binary checks

```
$ strings build/somnus-dial.bin | grep -c '1\.0\.2-beta\.1'
1
$ strings build/somnus-dial.bin | grep -c 'bedknob-for-somnus'
3
$ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
0
```

Expected non-zero / 3 / 0: met. `build/somnus-dial.bin` is 1,612,320 bytes (1.0.1 was 1,612,080; the longer version string accounts for the difference in kind, not verified byte-for-byte).

## 5. Extractor check

```
$ awk '/^## 1\.0\.2-beta\.1 /{f=1;next} /^## /{f=0} f' CHANGELOG.md

While the screen is off and the dial has gone to standby, it now asks the
pad for its state once every five minutes instead of every ten seconds.
Touching the knob still reads the pad immediately, so the face is current
the moment you look at it. Nothing about the screens or pad control
changes.

Internal: `POLL_STANDBY_US` at 300 s in the worker loop's due computation;
the cadence is logged when crossing into or out of standby; a pad outage
in standby now shows as stale after about fifteen minutes instead of three.

```

Exactly the new section (leading and trailing blank lines are the section's own spacing), nothing from `## 1.0.1` below it.

## 6. Commit

```
$ git --no-optional-locks show --stat HEAD
c6261fa6e289e6ae94abaacf547b5e4708f6643a
release: 1.0.2-beta.1 — STANDBY poll cadence 300 s (PROJECT_VER bump + CHANGELOG section)

 CHANGELOG.md                     | 12 ++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 13 insertions(+), 1 deletion(-)
```

`git --no-optional-locks status --short --untracked-files=no` after the commit: no output (the build touched no tracked file).

## 7. Deviations

None. The gate ran first and passed; one commit carrying both files; no tag, no push, no flash, no `idf.py monitor`, no dial or pad contact; `git status` only with `--no-optional-locks`; the CMake deprecation warning recorded and left alone; README untouched.

## 8. Not verifiable without hardware or the tag

- That a dial on 1.0.1 with Beta builds ON is offered and installs `1.0.2-beta.1` over the air, and that Menu → About then shows `1.0.2-beta.1` (SPEC-repo-consolidation §7 shape; needs the tag, the workflow run and the bench dial).
- That the release workflow's own extractor (run on GitHub, not this local `awk`) accepts the heading and publishes the body — the local command is the same expression, but the real check is the run at tag time.
- That Phase 6a's trimmed `release.yml` (`8ee93c4`) parses and shows exactly two publish steps — PyYAML was unavailable locally when 6a landed; this tag is its first run.
- The bench dial still runs the unbumped `1.0.1` build of the same code (`39af8f5`); the `1.0.2-beta.1` binary built here has not been flashed anywhere.
