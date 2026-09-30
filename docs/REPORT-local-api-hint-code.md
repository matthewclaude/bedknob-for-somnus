# REPORT: 1.0.4-beta.1 code — Local API hint on transport failure

**Verdict: PASS.** All six gates passed; the one change is committed as 5e9bc69 (firmware hint + simulator scenario string + regenerated pad-degraded-real.png). Host test passed, firmware build clean, binary identity/content checks as expected, only the intended screen changed. Not pushed, not tagged, PROJECT_VER still 1.0.3-beta.1.

Date: 2026-09-30. Repo: ~/Projects/somnus-waveshare-rotary-dial, branch main.

## Gate

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.3-beta.1")

$ git rev-parse somnus-v1.0.3-beta.1^{commit}
5a6e6923120416e9d0adf5d154c47febce8f504b

$ git rev-parse HEAD
ea76e170c0e6f10a00400c78cdcd9f981a0cd2c7
$ git ls-remote --heads somnus main
ea76e170c0e6f10a00400c78cdcd9f981a0cd2c7	refs/heads/main

$ git --no-optional-locks status --short --untracked-files=no
(no output)

$ sed -n 116,120p firmware/dial-idf/components/dial_somnus/dial_somnus.c
    if (err != ESP_OK) {
        set_error("http error: %s", esp_err_to_name(err));
        free(acc.buf);
        return false;
    }

$ sed -n 853,854p simulator/main.c
    snprintf(st->phase_err, sizeof(st->phase_err),
             "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)");
```

1 PASS, 2 PASS (5a6e692…), 3 PASS (HEAD = somnus/main = ea76e17…), 4 PASS, 5 PASS, 6 PASS. No Xcode-licence complaint from git or strings; DEVELOPER_DIR was not set.

## Steps 1–2: the diff (source files only)

```
$ git show --format= HEAD -- firmware/dial-idf/components/dial_somnus/dial_somnus.c simulator/main.c
diff --git a/firmware/dial-idf/components/dial_somnus/dial_somnus.c b/firmware/dial-idf/components/dial_somnus/dial_somnus.c
index 81c076e..69c6869 100644
--- a/firmware/dial-idf/components/dial_somnus/dial_somnus.c
+++ b/firmware/dial-idf/components/dial_somnus/dial_somnus.c
@@ -114,7 +114,9 @@ static bool do_request(const char *path, esp_http_client_method_t method,
     esp_http_client_cleanup(client);
 
     if (err != ESP_OK) {
-        set_error("http error: %s", esp_err_to_name(err));
+        ESP_LOGW(TAG, "http error: %s", esp_err_to_name(err));
+        // User-facing hint for the Pad unreachable screen; the raw esp_err name goes to the serial log only.
+        set_error("%s", "Is the pad's Local API enabled?\nSomnus support can turn it on");
         free(acc.buf);
         return false;
     }
diff --git a/simulator/main.c b/simulator/main.c
index a0fd9d2..5a4a260 100644
--- a/simulator/main.c
+++ b/simulator/main.c
@@ -851,7 +851,7 @@ static void scenario_pad_degraded_real(void)
     dial_state_set_pad_url("http://unreachable.invalid:8080");
     app_state_t *st = sim_state_ptr();
     snprintf(st->phase_err, sizeof(st->phase_err),
-             "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)");
+             "Is the pad's Local API enabled?\nSomnus support can turn it on");
     st->retry_in_s = 27;
     st->generation++;
     ui_router_go(SCR_CONNECTING, NULL, LV_SCR_LOAD_ANIM_NONE);
```

Apostrophe check (both files):

```
== firmware/dial-idf/components/dial_somnus/dial_somnus.c
$ grep -n "pad's Local API"
119:        set_error("%s", "Is the pad's Local API enabled?\nSomnus support can turn it on");
$ LC_ALL=C grep -c $'\xe2\x80\x99'
0
== simulator/main.c
$ grep -n "pad's Local API"
854:             "Is the pad's Local API enabled?\nSomnus support can turn it on");
$ LC_ALL=C grep -c $'\xe2\x80\x99'
0
```

The hint is 63 bytes including the `\n`; it fits `s_last_error[160]` and `app_state_t.phase_err[128]` (dial_state.h:378) without truncation. `retry_in_s = 27` unchanged; sim_state.c untouched.

## Step 3: host test

```
$ cc -I components/dial_state -Wall -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
all relative-scale table assertions passed
exit=0
```

Compiled with no warnings. PASS.

## Step 4: firmware build

`. ~/esp/esp-idf/export.sh`, then `idf.py build` in firmware/dial-idf. Exit 0, 0 errors. The build was incremental on the existing build directory (CMake did not reconfigure), so the known `cmake_minimum_required(VERSION 3.5)` deprecation warning did not print this time; dial_somnus.c recompiled with no compiler warnings. See Findings for the ESP-IDF fatfs Kconfig NOTE lines.

Last 25 lines:

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

App size: **1,611,568 B** (0x189730) vs 1.0.3-beta.1's 1,611,440 B → **+128 B**. 62% of the smallest app partition free.

## Step 5: binary identity and content (build/somnus-dial.bin)

```
$ strings | grep -c "repos/matthewclaude/bedknob-for-somnus"
3
$ strings | grep -c -E "somnus-dial-releases|orion-waveshare-rotary-dial"
0
$ strings | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'
1.0.3-beta.1
$ strings | grep -c "Local API enabled"
1
$ strings | grep -c "Somnus support can turn it on"
1
$ strings | grep -c "not responding (HTTP"
0
```

All six as expected (3 / 0 / 1.0.3-beta.1 only / 1 / 1 / 0).

## Step 6: simulator screens

docs/screens/ (48 PNGs) backed up to the session scratch directory before the run.

Configure (`cmake -B build -S simulator`):

```
-- dial_sim: firmware version 1.0.3-beta.1, OTA scenarios advertise 1.0.4
-- dial_sim: using local LVGL checkout at ~/Projects/somnus-waveshare-rotary-dial/simulator/../firmware/dial-idf/managed_components/lvgl__lvgl
CMake Warning (policy) at ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/env_support/cmake/custom.cmake:66 (install):
  Policy CMP0177 is not set: install() DESTINATION paths are normalized.  Run
  "cmake --help-policy CMP0177" for policy details.  Use the cmake_policy
  command to set the policy and suppress this warning.
Call Stack (most recent call first):
  ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl/CMakeLists.txt:16 (include)
This warning is for project developers.  Use -Wno-author or -Wno-policy to
suppress it.

-- Configuring done (0.1s)
-- Generating done (0.1s)
-- Build files have been written to: ~/Projects/somnus-waveshare-rotary-dial/build
```

Build (`cmake --build build`) exit 0, 0 warnings in the build output. Run (`./build/dial_sim`) exit 0:

```
I (ui_router) router up, screen 0
wrote welcome          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water ~/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   ~/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             ~/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (7 distinct colors sampled)
wrote update-prompt    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (5 distinct colors sampled)
wrote settings         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face ~/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      ~/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off ~/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     ~/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb ~/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         ~/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   ~/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to ~/Projects/somnus-waveshare-rotary-dial/docs/screens
```

```
$ git --no-optional-locks status --short docs/screens
 M docs/screens/pad-degraded-real.png
```

Only pad-degraded-real.png changed; pad-unreachable.png unchanged; no PNG added or removed (48 before and after). Nothing needed restoring.

pad-degraded-real.png, inspected: headline "Pad unreachable" in red; subtitle exactly four grey lines, in order: "Is the pad's Local API enabled?", "Somnus support can turn it on", "Retrying in 27s", "Swipe left for menu". Neither hint line wraps. The block is centred and well inside the round bezel; nothing clipped. The apostrophe in "pad's" renders as a proper glyph (no box, no gap). The block is one line shorter than before (the old string wrapped), so it sits a little lower/centred than the old four-line-from-wrap layout, as expected.

## Step 7: commit

```
$ git diff --cached --name-only
docs/screens/pad-degraded-real.png
firmware/dial-idf/components/dial_somnus/dial_somnus.c
simulator/main.c

$ git log --oneline -2
5e9bc69 dial: plain-language Local API hint when the pad does not answer (1.0.4-beta.1 change)
ea76e17 docs: 1.0.3-beta.1 publish verification, six version-bearing screens at 1.0.3-beta.1, four REPORTS.md lines

$ git --no-optional-locks status --short --untracked-files=no
(no output)

$ git diff --stat HEAD~1
 docs/screens/pad-degraded-real.png                  | Bin 19561 -> 19100 bytes
 .../dial-idf/components/dial_somnus/dial_somnus.c   |   4 +++-
 simulator/main.c                                    |   2 +-
 3 files changed, 4 insertions(+), 2 deletions(-)
```

Commit SHA: **5e9bc69d6eba9ac271b24772ded918e80a97aab3**. Not pushed, not tagged. This report is untracked; no REPORTS.md line added.

## Findings (noticed, not changed)

1. **Hint shows for every transport failure, not just Local API off.** `err != ESP_OK` also covers a pad that is powered off/unplugged, a wrong or stale IP, and Wi-Fi drops. In those cases "Is the pad's Local API enabled?" may point the user the wrong way. This is by design for this beta but worth a look once the bench shows which esp_err values each case produces (the raw name is still in the serial log).
2. **Stale simulator comment.** The comment above `scenario_pad_degraded_real` (simulator/main.c ~line 841) still says the subtitle "wraps to four lines (the echoed dial_somnus error, …)" and describes the old IP-bearing reason. It now renders four lines without wrapping (two hint lines + retry + swipe), and the "tallest case" claim for scr_connecting.c's block may no longer hold. Left as is per scope.
3. **The old sim string was not a real firmware string.** "Somnus pad at … not responding (HTTP -1)" did not exist in firmware (the binary count was already a check for 0); the firmware previously showed "http error: ESP_ERR_…". So the old screenshot never matched the device.
4. **Double log line on failure.** The new `ESP_LOGW(TAG, "http error: …")` is followed by set_error()'s own `ESP_LOGW(TAG, "%s", s_last_error)`, so each failure now logs two W lines, the second being the hint whose second half prints as an untagged serial line. Expected per the task; noted for anyone grepping logs.
5. **ESP-IDF fatfs Kconfig NOTEs** in the build output (`~/esp/esp-idf/components/fatfs/Kconfig:230/235: FATFS_PRINT_LLI / FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value … treated as 'n'`). They come from ESP-IDF's own Kconfig, not this project, and are labelled NOTE rather than warning; recorded because the task allows only the one CMake deprecation warning.
6. **Simulator configure CMake policy warning** (CMP0177, from LVGL's env_support/cmake/custom.cmake:66). Third-party, pre-existing, not part of the firmware build.
7. main.c:1055 (NULL to PH_PAD_DISCOVERY, one-tick stale headline) seen and deliberately not touched, per scope.

## Deviations

Deviations: none. (Note only: the firmware build was incremental, so the known CMake deprecation warning did not appear rather than appearing as the one allowed warning.)

## Not verifiable without hardware

- The real screen on the dial (font metrics and bezel on the device vs the simulator).
- The serial log form on the device: the hint's second line ("Somnus support can turn it on") prints as an untagged serial line after the tagged W line; this is expected.
- The unplugged-pad bench: owner's job, needs the real pad and the bed empty.
- The untested assumption that a pad with its Local API turned off refuses the connection (transport error, `err != ESP_OK`) rather than answering with an HTTP status. If it answers with a status, the user sees "pad returned HTTP %d" instead and this hint never shows.
