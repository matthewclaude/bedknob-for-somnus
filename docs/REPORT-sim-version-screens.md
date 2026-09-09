# REPORT — simulator reads PROJECT_VER at build time; screens regenerated at 1.0.0

Date: 2026-09-09. Session: https://claude.ai/code/session_01T2F1kBEuT13CM3rWm5tnLD

## Verdict

**DONE** — commit `effb9c6` on `main` (on top of `7c106fb` = `somnus-v1.0.0`): the simulator now derives the installed version (1.0.0) and the advertised OTA version (1.0.1) from `firmware/dial-idf/CMakeLists.txt` at configure time, the two hardcoded literals are gone behind `#error` guards, and 6 of 49 screens regenerated with the new strings while the other 43 are byte-identical. Nothing under `firmware/` changed. Not tagged, not pushed.

## Gate

```
$ git rev-parse --short HEAD
7c106fb

$ git describe --tags --exact-match HEAD
somnus-v1.0.0

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
20:# Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
21:set(PROJECT_VER "1.0.0")

$ git --no-optional-locks status --short
?? docs/REPORT-1.0.0-tag-push.md

$ grep -n '"0.1.4"' simulator/stubs.c
223:    static const esp_app_desc_t desc = { .version = "0.1.4", .idf_ver = "v6.0" };
$ grep -n 'SIM_OTA_LATEST "1.4.3"' simulator/main.c
46:#define SIM_OTA_LATEST "1.4.3"
```

Gate 3 passed on content; the `set(PROJECT_VER "1.0.0")` line is **line 21**, not 22, since the previous task's housekeeping commit (7c106fb) collapsed the two-line comment above it into one. Recorded under Deviations. All other checks matched exactly.

## Edits

### A. simulator/CMakeLists.txt — exact block added (git diff)

```diff
diff --git a/simulator/CMakeLists.txt b/simulator/CMakeLists.txt
index 58a165d..0f25f82 100644
--- a/simulator/CMakeLists.txt
+++ b/simulator/CMakeLists.txt
@@ -7,6 +7,36 @@ if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
 endif()
 
+# ---- firmware version, read from the firmware's own CMakeLists ----------
+# scr_about.c shows esp_app_get_description()->version and the Update
+# screens compare it against dial_ota's "latest"; the simulator stubs both
+# (stubs.c, main.c). Read PROJECT_VER from firmware/dial-idf/CMakeLists.txt
+# at configure time instead of hardcoding it here, so docs/screens can never
+# show a version the firmware doesn't ship. No fallback on purpose: a silent
+# default is exactly the stale-screenshot bug this replaces.
+set(DIAL_FW_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/../firmware/dial-idf/CMakeLists.txt")
+set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${DIAL_FW_CMAKE}")
+file(STRINGS "${DIAL_FW_CMAKE}" DIAL_FW_VER_LINES REGEX "^set\\(PROJECT_VER")
+list(LENGTH DIAL_FW_VER_LINES DIAL_FW_VER_LINE_COUNT)
+if(NOT DIAL_FW_VER_LINE_COUNT EQUAL 1)
+  message(FATAL_ERROR "dial_sim: expected exactly one set(PROJECT_VER ...) line in ${DIAL_FW_CMAKE}, found ${DIAL_FW_VER_LINE_COUNT}")
+endif()
+string(REGEX REPLACE "^set\\(PROJECT_VER +\"([^\"]+)\"\\).*$" "\\1" DIAL_FW_VERSION "${DIAL_FW_VER_LINES}")
+if(NOT DIAL_FW_VERSION OR DIAL_FW_VERSION STREQUAL DIAL_FW_VER_LINES)
+  message(FATAL_ERROR "dial_sim: could not extract PROJECT_VER from '${DIAL_FW_VER_LINES}'")
+endif()
+
+# OTA scenarios advertise the next patch release -- same major.minor, patch+1
+# -- so the Update screens always offer something newer than what is
+# installed. A -beta.N suffix is dropped before the arithmetic.
+string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" DIAL_FW_VERSION_CORE "${DIAL_FW_VERSION}")
+if(NOT DIAL_FW_VERSION_CORE)
+  message(FATAL_ERROR "dial_sim: PROJECT_VER '${DIAL_FW_VERSION}' is not MAJOR.MINOR.PATCH[-suffix]")
+endif()
+math(EXPR DIAL_FW_PATCH_NEXT "${CMAKE_MATCH_3} + 1")
+set(DIAL_SIM_OTA_LATEST "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${DIAL_FW_PATCH_NEXT}")
+message(STATUS "dial_sim: firmware version ${DIAL_FW_VERSION}, OTA scenarios advertise ${DIAL_SIM_OTA_LATEST}")
+
 # ---- LVGL 8.4.0 --------------------------------------------------------
 # firmware/dial-idf pins lvgl/lvgl at exactly 8.4.0 (managed_components/ is
 # gitignored, so a fresh clone won't have it). Use that local checkout
@@ -104,6 +134,11 @@ target_link_libraries(dial_sim PRIVATE lvgl::lvgl m)
 
 target_compile_options(dial_sim PRIVATE -Wall -Wno-unused-parameter)
 
+# Installed / advertised versions, from the block at the top of this file.
+target_compile_definitions(dial_sim PRIVATE
+  SIM_APP_VERSION="${DIAL_FW_VERSION}"
+  SIM_OTA_LATEST="${DIAL_SIM_OTA_LATEST}")
+
 # Where the PNGs land — always the repo's docs/screens/, regardless of the
 # build directory's location or the binary's working directory at run time.
 get_filename_component(DIAL_SIM_SCREENS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../docs/screens" ABSOLUTE)
```

Notes on the block:
- `file(STRINGS ... REGEX "^set\\(PROJECT_VER")` is anchored at line start so a comment that merely mentions `set(PROJECT_VER` can never match; a `list(LENGTH)` check then requires exactly one hit. `FATAL_ERROR` on zero or many hits and on a failed extraction — no fallback.
- `string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)")` takes the numeric core, so a `-beta.N` suffix is dropped before `math(EXPR patch+1)`; `1.0.0` → `1.0.1`, and e.g. `1.0.1-beta.2` would advertise `1.0.2`.
- `SIM_APP_VERSION` carries the full `PROJECT_VER` string (suffix included), matching what `esp_app_get_description()->version` reports on the device.
- `CMAKE_CONFIGURE_DEPENDS` on the firmware CMakeLists makes an incremental `cmake --build build` re-run configure when `PROJECT_VER` changes, so a version bump is picked up without a manual reconfigure.

### B. simulator/stubs.c

Before (lines 216-225):

```c
/* ---- esp_app_desc --------------------------------------------------------
 * Fixed "v1.0.1 / v6.0" identity for scr_about.c's Firmware/IDF rows —
 * kept in step with firmware/dial-idf/CMakeLists.txt's PROJECT_VER so the
 * simulator's about.png never shows a version the real firmware doesn't. */

const esp_app_desc_t *esp_app_get_description(void)
{
    static const esp_app_desc_t desc = { .version = "0.1.4", .idf_ver = "v6.0" };
    return &desc;
}
```

After:

```c
/* ---- esp_app_desc --------------------------------------------------------
 * Identity for scr_about.c's Firmware/IDF rows. The version is NOT written
 * here: simulator/CMakeLists.txt reads PROJECT_VER out of
 * firmware/dial-idf/CMakeLists.txt at configure time and passes it in as
 * SIM_APP_VERSION, so about.png shows whatever the firmware would report and
 * cannot drift from it. The IDF string is cosmetic and stays fixed. */

#ifndef SIM_APP_VERSION
#error "SIM_APP_VERSION must come from simulator/CMakeLists.txt (read from firmware/dial-idf/CMakeLists.txt PROJECT_VER)"
#endif

const esp_app_desc_t *esp_app_get_description(void)
{
    static const esp_app_desc_t desc = { .version = SIM_APP_VERSION, .idf_ver = "v6.0" };
    return &desc;
}
```

`.idf_ver = "v6.0"` untouched.

### C. simulator/main.c

Before (lines 41-46):

```c
// The "available" version every OTA scenario advertises. One constant rather
// than a literal per scenario, because it has to stay AHEAD of the version the
// simulator reports as installed (stubs.c's esp_app_desc_t, which tracks
// PROJECT_VER) — otherwise the screenshots show a dial offering to update
// itself to something it already runs. Bump it with each release.
#define SIM_OTA_LATEST "1.4.3"
```

After:

```c
// The "available" version every OTA scenario advertises. One constant rather
// than a literal per scenario, because it has to stay AHEAD of the version the
// simulator reports as installed (stubs.c's esp_app_desc_t, which tracks
// PROJECT_VER) — otherwise the screenshots show a dial offering to update
// itself to something it already runs. It is derived from PROJECT_VER by
// simulator/CMakeLists.txt at build time (patch+1), so it is always ahead of
// the installed version without anyone remembering to bump it.
#ifndef SIM_OTA_LATEST
#error "SIM_OTA_LATEST must come from simulator/CMakeLists.txt (PROJECT_VER with patch+1)"
#endif
```

All five existing `snprintf(st->ota.latest, ..., SIM_OTA_LATEST)` uses (now at main.c lines 367, 588, 606, 641, 1122) are unchanged.

### D. Literal-version sweep

```
$ grep -rn -E '"0\.1\.4"|"1\.4\.3"' simulator/
(empty, exit 1)

$ grep -n -E '"[0-9]+\.[0-9]+\.[0-9]+' simulator/*.c simulator/*.h
simulator/stubs.c:10: *   dial_net_ip -> "192.168.1.23"
simulator/stubs.c:121:    snprintf(out, sz, "192.168.1.23");
```

The two remaining hits are the simulated dial's own IP address (the `dial_net_ip` stub for the About screen's network rows), not a firmware version. Left alone.

## Build and regenerate

Build directory: repo-root `build/` — the one `simulator/README.md` names (`cmake -B build -S simulator` from the repo root; gitignored via `.gitignore:42 /build/`), not `simulator/build`. Backup: `cp -r docs/screens /tmp/screens-before` (49 files).

### Configure (raw)

```
$ cmake -B build -S simulator
-- dial_sim: firmware version 1.0.0, OTA scenarios advertise 1.0.1
-- dial_sim: using local LVGL checkout at /Users/matthew/Projects/somnus-waveshare-rotary-dial/simulator/../firmware/dial-idf/managed_components/lvgl__lvgl
CMake Warning (policy) at /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/env_support/cmake/custom.cmake:66 (install):
  Policy CMP0177 is not set: install() DESTINATION paths are normalized.  Run
  "cmake --help-policy CMP0177" for policy details.  Use the cmake_policy
  command to set the policy and suppress this warning.
Call Stack (most recent call first):
  /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/CMakeLists.txt:16 (include)
This warning is for project developers.  Use -Wno-author or -Wno-policy to
suppress it.

-- Configuring done (0.1s)
-- Generating done (0.1s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/build
(exit 0)
```

The `message(STATUS)` line reads `firmware version 1.0.0, OTA scenarios advertise 1.0.1`. The one CMake warning is LVGL's own `install()` policy CMP0177 notice from the vendored `managed_components/lvgl__lvgl` checkout under CMake 4.4.2 — pre-existing, not from `simulator/`.

Compile definitions as generated (`build/CMakeFiles/dial_sim.dir/flags.make`):

```
SIM_APP_VERSION=\"1.0.0\"
SIM_OTA_LATEST=\"1.0.1\"
```

### Build (raw, unfiltered)

```
$ cmake --build build
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/sim_state.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/stubs.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_router.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_screens.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_connecting.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_setup.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_dial.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_netpick.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_passkey.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_menu.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_wifi.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_about.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_updating.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update_prompt.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_welcome.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_sidepick.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_settings.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_timezone.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_discovery.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_address.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_adjust_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness_menu.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_mode.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_palette.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_list.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_88.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_16.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_20.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_140.c.o
[ 49%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 86%] Built target lvgl_examples
[100%] Built target lvgl_demos
(exit 0)
```

`grep -i -E 'warning|error'` over that output: no hits. Zero warnings in `simulator/*.c` (and none in the firmware UI sources either).

### Run

```
$ ./build/dial_sim
I (ui_router) router up, screen 0
wrote welcome          .../docs/screens/welcome.png (6 distinct colors sampled)
...
done: 49 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

Exit 0, 49 screens rendered (same count as the last regen).

## Screens diff

```
$ git --no-optional-locks status --short docs/screens
 M docs/screens/about-wifi-real.png
 M docs/screens/about-wifi-worst.png
 M docs/screens/about.png
 M docs/screens/update-failed.png
 M docs/screens/update-prompt.png
 M docs/screens/update.png
```

`cmp` of every PNG against `/tmp/screens-before`: **6 changed, 43 byte-identical, 49 total**; no PNGs added or removed.

Each changed PNG, viewed after the run, and the version text it now shows:

| PNG | Version text rendered now (was) |
|---|---|
| `about.png` | Firmware row **v1.0.0** (was v0.1.4); IDF row v6.0 unchanged |
| `about-wifi-real.png` | Scrolled to the Wi-Fi row; the Firmware row is still partly visible at the top edge reading **v1.0.0** (was v0.1.4) |
| `about-wifi-worst.png` | Same layout as above; top-edge Firmware row **v1.0.0** (was v0.1.4) |
| `update.png` | "**v1.0.1** - tap to install", Installed **v1.0.0**, "Skip **1.0.1**" (were v1.4.3 / v0.1.4 / 1.4.3) |
| `update-prompt.png` | "Update available **1.0.1**" (was 1.4.3) |
| `update-failed.png` | Installed **v1.0.0** (was v0.1.4) |

Every changed PNG renders a version string; **no non-version PNG changed**, so the build environment matches the Sep 6 regen (fonts, LVGL 8.4.0 local checkout, same 49 scenarios).

Screens the spec expected to change but which are identical, and why:

- `standby-update.png` — `scr_standby.c` contains no `version`/`latest` reference; the standby update cue is a glyph/badge with no version text, so a different `ota.latest` cannot alter its pixels.
- `updating.png` — `scr_updating.c` renders progress only, no version text.
- `about-battery-pct.png`, `about-battery-usb.png` — scrolled far enough that the Firmware row is fully off-screen (unlike the `about-wifi-*` shots where it peeks in at the top).

## Commit

```
$ git diff --stat HEAD~1 HEAD
 docs/REPORT-1.0.0-tag-push.md     | 176 ++++++++++++++++++++++++++++++++++++++
 docs/screens/about-wifi-real.png  | Bin 23213 -> 23232 bytes
 docs/screens/about-wifi-worst.png | Bin 24177 -> 24196 bytes
 docs/screens/about.png            | Bin 18520 -> 18358 bytes
 docs/screens/update-failed.png    | Bin 21750 -> 21747 bytes
 docs/screens/update-prompt.png    | Bin 22101 -> 21987 bytes
 docs/screens/update.png           | Bin 23110 -> 22790 bytes
 simulator/CMakeLists.txt          |  35 ++++++++
 simulator/main.c                  |   8 +-
 simulator/stubs.c                 |  14 ++-
 10 files changed, 227 insertions(+), 6 deletions(-)

$ git rev-parse --short HEAD
effb9c6

$ git diff --name-only HEAD~1 HEAD -- firmware/
(empty)
```

`docs/REPORT-1.0.0-tag-push.md` was untracked and is included, per the spec. This task's own report is left untracked.

## Deviations from this spec

1. **Gate 3 line number.** The spec says line 22; the line is 21 (content exact). Cause is the previous task's own edit, already reported in docs/REPORT-1.0.0-tag-push.md. Proceeded on content.
2. **Build directory is repo-root `build/`**, as the README names, rather than the spec's first suggestion `simulator/build`; the spec allowed this ("or whatever build dir simulator/README.md names").
3. **Additions to the CMake block beyond the spec's list:** the `^` anchor on the `file(STRINGS REGEX)`, the exactly-one-line count check, and `CMAKE_CONFIGURE_DEPENDS` on the firmware CMakeLists. All tighten the "no silent fallback" intent; none change the derived values.
4. **Commit trailers.** The commit carries the session's required `Co-Authored-By` / `Claude-Session` lines in addition to the spec's two `-m` paragraphs.
5. **The spec's expected-changed list** named `standby-update` and "update-*" generally; only the six screens above actually carry version text (see the identical-screens explanation). Not an error, recorded so the expectation can be corrected.

## Could not verify

Nothing in this task touches hardware; no firmware was built, flashed, or run. The simulator compiles the unmodified firmware UI sources on the host, and the verification here is the host build plus the rendered PNGs.
