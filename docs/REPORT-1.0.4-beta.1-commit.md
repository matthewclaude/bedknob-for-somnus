# somnus-v1.0.4-beta.1 release commit

**Verdict: PASS.** Release commit `1d01f3b664e6a217bbf10bb34152f683d1a4d91e` (`1d01f3b`) on main, parent `452df8e`. It changes exactly two files: `firmware/dial-idf/CMakeLists.txt` line 21 and a new `## 1.0.4-beta.1` section in `CHANGELOG.md`. Nothing is pushed or tagged.

## Gate

| # | Check | Raw output | Result |
|---|---|---|---|
| 1 | `sed -n 21p firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.3-beta.1")` | PASS |
| 2 | `git rev-parse HEAD` / `git log --oneline -2` | `452df8e6a282b08e32db21d0ffb2e21ff7eee76d` / see below | PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | *(empty)* | PASS |
| 4 | `git tag -l somnus-v1.0.4-beta.1` ; `git ls-remote --tags somnus somnus-v1.0.4-beta.1` | *(empty)* ; *(empty, rc=0)* | PASS |
| 5 | `grep -c '^## 1.0.4-beta.1' CHANGELOG.md` ; first `## ` heading | `0` ; `## 1.0.3-beta.1 — 2026-09-30 (beta)` | PASS |

Gate 2 log:

```
452df8e docs: 1.0.4-beta.1 bench PASS (hint screen, reconnect), code and flash reports, SPEC-local-api-hint steady-state correction
5e9bc69 dial: plain-language Local API hint when the pad does not answer (1.0.4-beta.1 change)
```

No command reported a problem with the Xcode licence. `DEVELOPER_DIR` was not set.

## Step 1: PROJECT_VER

After the edit, `sed -n 21p` prints `set(PROJECT_VER "1.0.4-beta.1")`, and `git diff --numstat` showed `1	1	firmware/dial-idf/CMakeLists.txt` (one line changed).

## Step 2: CHANGELOG

- The section is inserted directly above `## 1.0.3-beta.1`, with one blank line before that heading, matching the existing spacing. The heading uses U+2014.
- `LC_ALL=C grep -c $'\xe2\x80\x99' CHANGELOG.md` gave `0` before the edit and `0` after it. The apostrophe in "pad's" is ASCII 0x27.
- `git diff --numstat` showed `10	0	CHANGELOG.md`, so only lines were added.

The extraction below uses the awk range from `.github/workflows/release.yml` with `ver=1.0.4-beta.1`. It is shown verbatim, including the leading and trailing blank lines, which release.yml then trims:

```

When the dial cannot reach your pad, the "Pad unreachable" screen now asks
"Is the pad's Local API enabled?" and says Somnus support can turn it on,
instead of showing a technical error code. The Local API has to be on for
the dial to work, and today it is turned on by asking Somnus support.
Nothing else about connecting, retrying or controlling the pad changes.

Internal: the raw network error still goes to the serial log.

```

After release.yml trims those blank lines, this is the published Release body:

```
When the dial cannot reach your pad, the "Pad unreachable" screen now asks
"Is the pad's Local API enabled?" and says Somnus support can turn it on,
instead of showing a technical error code. The Local API has to be on for
the dial to work, and today it is turned on by asking Somnus support.
Nothing else about connecting, retrying or controlling the pad changes.

Internal: the raw network error still goes to the serial log.
```

## Commit diff (`git show --format= HEAD`)

```diff
diff --git a/CHANGELOG.md b/CHANGELOG.md
index bf6d6ae..843e984 100644
--- a/CHANGELOG.md
+++ b/CHANGELOG.md
@@ -22,6 +22,16 @@ Add the new section in the same commit that bumps `PROJECT_VER`.
 Releases marked **(beta)** are prereleases, visible only to dials with
 "Beta builds" turned on.
 
+## 1.0.4-beta.1 — 2026-09-30 (beta)
+
+When the dial cannot reach your pad, the "Pad unreachable" screen now asks
+"Is the pad's Local API enabled?" and says Somnus support can turn it on,
+instead of showing a technical error code. The Local API has to be on for
+the dial to work, and today it is turned on by asking Somnus support.
+Nothing else about connecting, retrying or controlling the pad changes.
+
+Internal: the raw network error still goes to the serial log.
+
 ## 1.0.3-beta.1 — 2026-09-30 (beta)
 
 A new dial in Dual Sides mode no longer stops to ask "Which side of the
diff --git a/firmware/dial-idf/CMakeLists.txt b/firmware/dial-idf/CMakeLists.txt
index 0f811f0..afe9e38 100644
--- a/firmware/dial-idf/CMakeLists.txt
+++ b/firmware/dial-idf/CMakeLists.txt
@@ -18,5 +18,5 @@ add_compile_options("-Wno-format")
 # which would have made every device silently reject any 0.x release as a
 # downgrade (is_newer() has no concept of a renumbering, only "older").
 # Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
-set(PROJECT_VER "1.0.3-beta.1")
+set(PROJECT_VER "1.0.4-beta.1")
 project(somnus-dial)
```

## Step 3: build and checks

The build ran `. ~/esp/esp-idf/export.sh` and then `idf.py build` in `firmware/dial-idf`. It reconfigured because PROJECT_VER changed and exited with rc=0.

- `grep -ci error` on the build log gave `0`.
- The build log has exactly one warning: `CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required)`, the known one.
- ESP-IDF's usual Kconfig `NOTE:` lines also appear (for example `FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value`). They come from IDF component Kconfig files, not this project, and earlier reports record the same lines. They are notes, not warnings.

Last 25 lines of the build output:

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
Generated ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189730 bytes. Smallest app partition is 0x400000 bytes. 0x2768d0 bytes (62%) free.
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

**App size:** `build/somnus-dial.bin` is 1,611,568 B (0x189730). 5e9bc69 was also 1,611,568 B, a difference of 0 B. That is expected: `1.0.3-beta.1` and `1.0.4-beta.1` are the same length (12 characters), so the only change is the version string's content.

strings checks on `build/somnus-dial.bin`:

| Check | Expected | Got |
|---|---|---|
| `grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'` | `1.0.4-beta.1` only | `1.0.4-beta.1` |
| `grep -c "repos/matthewclaude/bedknob-for-somnus"` | 3 | 3 |
| `grep -c -E "somnus-dial-releases\|orion-waveshare-rotary-dial"` | 0 | 0 |
| `grep -c "Local API enabled"` | 1 | 1 |

The simulator was not run, as the block instructs.

## Step 4: commit

`git diff --cached --name-only` before the commit:

```
CHANGELOG.md
firmware/dial-idf/CMakeLists.txt
```

`git log --oneline -2`:

```
1d01f3b release: somnus-v1.0.4-beta.1 — plain-language Local API hint on the Pad unreachable screen
452df8e docs: 1.0.4-beta.1 bench PASS (hint screen, reconnect), code and flash reports, SPEC-local-api-hint steady-state correction
```

`git --no-optional-locks status --short --untracked-files=no`: *(empty)*

`git show --stat --format= HEAD`:

```
 CHANGELOG.md                     | 10 ++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 11 insertions(+), 1 deletion(-)
```

Commit SHA: `1d01f3b664e6a217bbf10bb34152f683d1a4d91e`

## Deviations

- The commit message has the given subject line plus a `Co-Authored-By: Claude Opus 5.5` trailer after a blank line. The block gave only the subject line. The trailer does not affect the tag, release.yml or the Release body.
- The CHANGELOG insertion was done with a short Python script that reads and rewrites the file. It inserts text at the exact heading and uses no force-write or overwrite flags.
- Otherwise none.

## Next step for the owner

Read the `## 1.0.4-beta.1` section in CHANGELOG.md (quoted above), because release.yml publishes it verbatim as the Release body. Then run block 2: tag, push, verify.
