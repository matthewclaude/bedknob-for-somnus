# REPORT: 1.0.3-beta.1 commit — bench docs and the release commit

**Verdict: DONE with one flagged deviation. Both commits are made (A `4e4f504`, B `5a6e692`), not tagged, not pushed. The build has 0 compiler errors and 0 compiler warnings. The only warning is the old CMake deprecation notice for `cmake_minimum_required(VERSION 3.5)` at `firmware/dial-idf/CMakeLists.txt:5`. It was already there in the fe5b139 build. See Deviations.**

Date: 2026-09-30. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch `main`.

## Gate

| # | Check | Result |
|---|---|---|
| 1 | HEAD is 042840b and matches somnus/main | PASS. `git rev-parse HEAD` = `042840b3dd557cd24369cec6322903d53dfb5aee`; `git ls-remote --heads somnus main` = `042840b3dd557cd24369cec6322903d53dfb5aee refs/heads/main`; `git rev-parse somnus/main` gives the same SHA. |
| 2 | Only uncommitted changes are the three named files | PASS. `git --no-optional-locks status --porcelain`: ` M docs/REPORTS.md`, `?? docs/REPORT-sidepick-bench-flash.md`, `?? docs/REPORT-sidepick-bench.md`. Nothing else. |
| 3 | `docs/REPORT-sidepick-bench.md` has a line starting `**Verdict: PASS` | PASS. Line 5: `**Verdict: PASS (T1, T1-r1, T1-r2, T4, T4-r1, T4-r2, T2, T2-a, T2-r1, T2-r2, T2-z, Close), with the deviations listed below. T3 (OTA from 1.0.2) is still open and can only run after 1.0.3-beta.1 is published.**` |

## Commit A — docs

Message: `docs: sidepick bench reports (PASS) and the section 7 sb_face note`

SHA: `4e4f504b8b4d030824e1d92a752ccf479dba4c42`

```
 docs/REPORT-sidepick-bench-flash.md | 705 ++++++++++++++++++++++++++++++++++++
 docs/REPORT-sidepick-bench.md       |  40 ++
 docs/REPORTS.md                     |   1 +
 docs/SPEC-sidepick-deletion.md      |   8 +
 4 files changed, 754 insertions(+)
```

**Early return, confirmed.** `firmware/dial-idf/components/dial_state/dial_state.c:13` has `#define NVS_NS "ui"`. Lines 183–186:

```c
void dial_state_restore_prefs(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return;
```

The `sb_face` log lines are at 245–246, after this return. With `NVS_READONLY`, `nvs_open` fails (ESP_ERR_NVS_NOT_FOUND) when the namespace does not exist, which is the case after an NVS erase. So neither line is logged. The erase line the note cites is `firmware/dial-idf/main/main.c:844`: `ESP_LOGW(TAG, "settings: factory reset requested — erasing NVS");`.

Note added under the §7 table in `docs/SPEC-sidepick-deletion.md`. The table itself is not changed:

```
@@ -468,6 +468,14 @@ nightly dial, so the steps run in this order:
 | T3 (upgrade) | After publication, with the dial back on 1.0.2 as set out in step 4 of the order above: OTA to the beta. | No side picker. Same side and same Scale as before the update. | Optional: the OTA and boot lines. |
 | Close | Set the one dial, Bedknob #1, back to Bed Mode → One Bed. | BOTH SIDES label. | Diff the pad's state at the start and end of the session. |
 
+**Note (2026-09-30, after the bench).** The T1 and T2 rows expect
+`sb_face: no key -> default` after a factory reset. That line cannot
+appear. After an NVS erase the `"ui"` namespace does not exist, so
+`dial_state_restore_prefs` returns at its `nvs_open(NVS_NS, NVS_READONLY,
+&h)` check (`dial_state.c:186`) before it logs anything. The proof of an
+empty namespace is the `factory reset requested — erasing NVS` line
+followed by a boot with no `sb_face` line at all.
+
 T1 and T2 both reboot twice, as the task requires. T4 reuses the T1 device
 because a device that was factory-reset and has never had Scale tapped is
 exactly the state that exposes the latent path.
```

## Commit B — release

Message: `release: somnus-v1.0.3-beta.1 — delete SCR_SIDEPICK; Scale no longer flips to Absolute after Dual Sides + swipe + reboot`

SHA: `5a6e6923120416e9d0adf5d154c47febce8f504b`

```
 CHANGELOG.md                     | 16 ++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 17 insertions(+), 1 deletion(-)
```

Exactly two files. The CHANGELOG section is §8's draft, copied by script from the fenced block in `docs/SPEC-sidepick-deletion.md` with only `YYYY-MM-DD` replaced by `2026-09-30`. It is inserted directly above `## 1.0.2 — 2026-09-17`. The diff shows only additions in CHANGELOG.md, so no existing section changed.

```
diff --git a/CHANGELOG.md b/CHANGELOG.md
index 4b4c195..bf6d6ae 100644
--- a/CHANGELOG.md
+++ b/CHANGELOG.md
@@ -22,6 +22,22 @@ Add the new section in the same commit that bumps `PROJECT_VER`.
 Releases marked **(beta)** are prereleases, visible only to dials with
 "Beta builds" turned on.
 
+## 1.0.3-beta.1 — 2026-09-30 (beta)
+
+A new dial in Dual Sides mode no longer stops to ask "Which side of the
+bed?" after it first reaches the pad. It opens on the right side, as it
+already did for anyone who never saw that question, and one swipe shows
+the left. The dial still remembers the last side you looked at.
+
+Also fixed: switching an existing dial from One Bed to Dual Sides and then
+swiping to the other side could change Scale from Relative to Absolute on
+the next restart, even though you never changed it. Scale now stays where
+you left it.
+
+Internal: SCR_SIDEPICK and the side_picked flag are removed; the "relmode"
+key is now seeded alongside the first "zone" write in
+dial_state_set_ui_zone instead of by the side picker.
+
 ## 1.0.2 — 2026-09-17
 
 While the screen is off and the dial has gone to standby, it now asks the
diff --git a/firmware/dial-idf/CMakeLists.txt b/firmware/dial-idf/CMakeLists.txt
index ae9f20d..0f811f0 100644
--- a/firmware/dial-idf/CMakeLists.txt
+++ b/firmware/dial-idf/CMakeLists.txt
@@ -18,5 +18,5 @@ add_compile_options("-Wno-format")
 # which would have made every device silently reject any 0.x release as a
 # downgrade (is_newer() has no concept of a renumbering, only "older").
 # Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
-set(PROJECT_VER "1.0.2")
+set(PROJECT_VER "1.0.3-beta.1")
 project(somnus-dial)
```

## Release-notes extractor

This is the `Extract release notes from CHANGELOG.md` step of `.github/workflows/release.yml` (the awk block plus its two trim seds), run locally with `GITHUB_REF_NAME=somnus-v1.0.3-beta.1` on the committed CHANGELOG.md. Output:

```
A new dial in Dual Sides mode no longer stops to ask "Which side of the
bed?" after it first reaches the pad. It opens on the right side, as it
already did for anyone who never saw that question, and one swipe shows
the left. The dial still remembers the last side you looked at.

Also fixed: switching an existing dial from One Bed to Dual Sides and then
swiping to the other side could change Scale from Relative to Absolute on
the next restart, even though you never changed it. Scale now stays where
you left it.

Internal: SCR_SIDEPICK and the side_picked flag are removed; the "relmode"
key is now seeded alongside the first "zone" write in
dial_state_set_ui_zone instead of by the side picker.
```

This is exactly the new section: the body from the line after `## 1.0.3-beta.1 — 2026-09-30 (beta)` up to the line before `## 1.0.2`, trimmed. A `diff` against a sed slice of that range showed no difference. The heading line is not in the body because the awk skips it (`next`). That is how the workflow handles every release. It is not empty, so the `::error::` path is not taken.

## Build gate

`cd firmware/dial-idf && idf.py fullclean && idf.py build`, run after `. $HOME/esp/esp-idf/export.sh`, with the Commit B tree in place (built before the commit, so the tree matched what was committed).

- fullclean exit code: 0
- build exit code: 0
- Compiler `error:` lines: **0**. Compiler `warning:` lines: **0**.
- Lines that contain the word "warning": 2. Both belong to the one CMake deprecation notice (output lines 1 and 8). It is printed during CMake configure about `cmake_minimum_required(VERSION 3.5)` at `firmware/dial-idf/CMakeLists.txt:5`. That is ESP-IDF boilerplate, and Commit B does not change that line. REPORT-sidepick-code.md recorded the same notice for fe5b139.
- Lines that contain the word "error": 1. It is the object name `mbedtls/library/.../error.c.obj`, not an error.
- **Counts: 0 errors, 0 compiler warnings, 1 CMake deprecation warning.**

### Raw fullclean output (unfiltered)

```
Executing action: fullclean
Executing action: remove_managed_components
Done
```

### Raw build output (unfiltered)

```
CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.
This warning is for project developers.  Use -Wno-author or -Wno-deprecated
to suppress it.

Executing action: all (aliases: build)
Running cmake in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "cmake -G 'Unix Makefiles' -B /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build -DPYTHON_DEPS_CHECKED=1 -DPYTHON=/Users/matthew/.espressif/python_env/idf6.0_py3.12_env/bin/python -DESP_PLATFORM=1 -DCCACHE_ENABLE=False /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf"...
-- IDF_TARGET is not set, guessed 'esp32s3' from sdkconfig '/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig'
-- Found Git: /usr/bin/git (found version "2.54.0 (Apple Git-157)")
-- Component directory /Users/matthew/esp/esp-idf/components/mqtt does not contain a CMakeLists.txt file. No component will be added
-- Minimal build - OFF
-- The C compiler identification is GNU 15.2.0
-- The CXX compiler identification is GNU 15.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-gcc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Building ESP-IDF components for target esp32s3
NOTICE: Processing 5 dependencies:
NOTICE: [1/5] espressif/cjson (1.7.19)
NOTICE: [2/5] espressif/cmake_utilities (0.5.3)
NOTICE: [3/5] espressif/esp_lcd_sh8601 (2.0.1)
NOTICE: [4/5] lvgl/lvgl (8.4.0)
NOTICE: [5/5] idf (6.0.0)
-- ESP-TEE is currently supported only on the esp32c6;esp32h2;esp32c5;esp32c61 SoCs
-- KCONFIG_REPORT_VERBOSITY not set, using default
-- Project sdkconfig file /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig
NOTE: /Users/matthew/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/fatfs/Kconfig:235: FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: /Users/matthew/esp/esp-idf/components/bt/sdkconfig.rename.esp32s3:4: duplicate rename mapping for CONFIG_BT_NIMBLE_COEX_PHY_CODED_TX_RX_TLIM_EN: previous target CONFIG_BT_LE_COEX_PHY_CODED_TX_RX_TLIM_EN, new target CONFIG_BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM_EN - last mapping is used
NOTE: /Users/matthew/esp/esp-idf/components/bt/sdkconfig.rename.esp32s3:5: duplicate rename mapping for CONFIG_BT_NIMBLE_COEX_PHY_CODED_TX_RX_TLIM_DIS: previous target CONFIG_BT_LE_COEX_PHY_CODED_TX_RX_TLIM_DIS, new target CONFIG_BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM_DIS - last mapping is used
Loading defaults file /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig.defaults...
NOTE: Kconfig parser version: 1
NOTE: Kconfig defaults policy: use sdkconfig
NOTE: Status: Finished successfully
-- Compiler supported targets: xtensa-esp-elf
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- App "somnus-dial" version: 1.0.3-beta.1
-- Found Python3: /Users/matthew/.espressif/python_env/idf6.0_py3.12_env/bin/python (found version "3.12.13") found components: Interpreter
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE
-- Performing Test C_COMPILER_SUPPORTS_WFORMAT_SIGNEDNESS
-- Performing Test C_COMPILER_SUPPORTS_WFORMAT_SIGNEDNESS - Success
-- Setting up mbedtls configuration
-- Linkage type is INTERFACE
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_hal_wdt/esp32s3/rom.wdt.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/memory.ld.in
--   -> Preprocessing .in script: /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/memory.ld.in
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/sections.ld.in
--   -> Preprocessing .in script: /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/sections.ld.in
--   -> Applying ldgen processing: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/esp-idf/esp_system/ld/sections.ld.in
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.api.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.bt_funcs.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libgcc.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.version.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_master.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_50.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_smp.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_dtm.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_test.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_scan.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libc.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/soc/esp32s3/ld/esp32s3.peripherals.ld
-- ESP_LCD_SH8601: 2.0.1
-- Component idf::esp_trace will be linked with -Wl,--whole-archive
-- Components: app_trace app_update bootloader bootloader_support bt cmock console cxx dial_display dial_haptics dial_knob dial_net dial_ota dial_pad_discovery dial_power dial_somnus dial_state dial_time dial_ui driver efuse esp-tls esp_adc esp_app_format esp_blockdev esp_bootloader_format esp_coex esp_common esp_driver_ana_cmpr esp_driver_bitscrambler esp_driver_cam esp_driver_dac esp_driver_dma esp_driver_gpio esp_driver_gptimer esp_driver_i2c esp_driver_i2s esp_driver_i3c esp_driver_isp esp_driver_jpeg esp_driver_ledc esp_driver_mcpwm esp_driver_parlio esp_driver_pcnt esp_driver_ppa esp_driver_rmt esp_driver_sd_intf esp_driver_sdio esp_driver_sdm esp_driver_sdmmc esp_driver_sdspi esp_driver_spi esp_driver_touch_sens esp_driver_tsens esp_driver_twai esp_driver_uart esp_driver_usb_serial_jtag esp_eth esp_event esp_gdbstub esp_hal_ana_cmpr esp_hal_ana_conv esp_hal_cam esp_hal_clock esp_hal_dma esp_hal_gpio esp_hal_gpspi esp_hal_i2c esp_hal_i2s esp_hal_ieee802154 esp_hal_jpeg esp_hal_lcd esp_hal_ledc esp_hal_mcpwm esp_hal_mspi esp_hal_parlio esp_hal_pcnt esp_hal_pmu esp_hal_ppa esp_hal_rmt esp_hal_rtc_timer esp_hal_security esp_hal_timg esp_hal_touch_sens esp_hal_twai esp_hal_uart esp_hal_usb esp_hal_wdt esp_hid esp_http_client esp_http_server esp_https_ota esp_https_server esp_hw_support esp_lcd esp_libc esp_local_ctrl esp_mm esp_netif esp_netif_stack esp_partition esp_phy esp_pm esp_psram esp_ringbuf esp_rom esp_security esp_stdio esp_system esp_timer esp_trace esp_usb_cdc_rom_console esp_wifi espcoredump espressif__cjson espressif__cmake_utilities espressif__esp_lcd_sh8601 esptool_py fatfs freertos hal heap http_parser i2c_bsp idf_test ieee802154 lcd_bl_pwm_bsp lcd_touch_bsp log lvgl__lvgl lwip main mbedtls nvs_flash nvs_sec_provider openthread partition_table perfmon protobuf-c protocomm pthread rt sdmmc soc spi_flash spiffs tcp_transport ulp unity vfs wear_levelling wpa_supplicant xtensa
-- Component paths: /Users/matthew/esp/esp-idf/components/app_trace /Users/matthew/esp/esp-idf/components/app_update /Users/matthew/esp/esp-idf/components/bootloader /Users/matthew/esp/esp-idf/components/bootloader_support /Users/matthew/esp/esp-idf/components/bt /Users/matthew/esp/esp-idf/components/cmock /Users/matthew/esp/esp-idf/components/console /Users/matthew/esp/esp-idf/components/cxx /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_display /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_haptics /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_knob /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_net /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ota /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_power /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_somnus /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_state /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_time /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui /Users/matthew/esp/esp-idf/components/driver /Users/matthew/esp/esp-idf/components/efuse /Users/matthew/esp/esp-idf/components/esp-tls /Users/matthew/esp/esp-idf/components/esp_adc /Users/matthew/esp/esp-idf/components/esp_app_format /Users/matthew/esp/esp-idf/components/esp_blockdev /Users/matthew/esp/esp-idf/components/esp_bootloader_format /Users/matthew/esp/esp-idf/components/esp_coex /Users/matthew/esp/esp-idf/components/esp_common /Users/matthew/esp/esp-idf/components/esp_driver_ana_cmpr /Users/matthew/esp/esp-idf/components/esp_driver_bitscrambler /Users/matthew/esp/esp-idf/components/esp_driver_cam /Users/matthew/esp/esp-idf/components/esp_driver_dac /Users/matthew/esp/esp-idf/components/esp_driver_dma /Users/matthew/esp/esp-idf/components/esp_driver_gpio /Users/matthew/esp/esp-idf/components/esp_driver_gptimer /Users/matthew/esp/esp-idf/components/esp_driver_i2c /Users/matthew/esp/esp-idf/components/esp_driver_i2s /Users/matthew/esp/esp-idf/components/esp_driver_i3c /Users/matthew/esp/esp-idf/components/esp_driver_isp /Users/matthew/esp/esp-idf/components/esp_driver_jpeg /Users/matthew/esp/esp-idf/components/esp_driver_ledc /Users/matthew/esp/esp-idf/components/esp_driver_mcpwm /Users/matthew/esp/esp-idf/components/esp_driver_parlio /Users/matthew/esp/esp-idf/components/esp_driver_pcnt /Users/matthew/esp/esp-idf/components/esp_driver_ppa /Users/matthew/esp/esp-idf/components/esp_driver_rmt /Users/matthew/esp/esp-idf/components/esp_driver_sd_intf /Users/matthew/esp/esp-idf/components/esp_driver_sdio /Users/matthew/esp/esp-idf/components/esp_driver_sdm /Users/matthew/esp/esp-idf/components/esp_driver_sdmmc /Users/matthew/esp/esp-idf/components/esp_driver_sdspi /Users/matthew/esp/esp-idf/components/esp_driver_spi /Users/matthew/esp/esp-idf/components/esp_driver_touch_sens /Users/matthew/esp/esp-idf/components/esp_driver_tsens /Users/matthew/esp/esp-idf/components/esp_driver_twai /Users/matthew/esp/esp-idf/components/esp_driver_uart /Users/matthew/esp/esp-idf/components/esp_driver_usb_serial_jtag /Users/matthew/esp/esp-idf/components/esp_eth /Users/matthew/esp/esp-idf/components/esp_event /Users/matthew/esp/esp-idf/components/esp_gdbstub /Users/matthew/esp/esp-idf/components/esp_hal_ana_cmpr /Users/matthew/esp/esp-idf/components/esp_hal_ana_conv /Users/matthew/esp/esp-idf/components/esp_hal_cam /Users/matthew/esp/esp-idf/components/esp_hal_clock /Users/matthew/esp/esp-idf/components/esp_hal_dma /Users/matthew/esp/esp-idf/components/esp_hal_gpio /Users/matthew/esp/esp-idf/components/esp_hal_gpspi /Users/matthew/esp/esp-idf/components/esp_hal_i2c /Users/matthew/esp/esp-idf/components/esp_hal_i2s /Users/matthew/esp/esp-idf/components/esp_hal_ieee802154 /Users/matthew/esp/esp-idf/components/esp_hal_jpeg /Users/matthew/esp/esp-idf/components/esp_hal_lcd /Users/matthew/esp/esp-idf/components/esp_hal_ledc /Users/matthew/esp/esp-idf/components/esp_hal_mcpwm /Users/matthew/esp/esp-idf/components/esp_hal_mspi /Users/matthew/esp/esp-idf/components/esp_hal_parlio /Users/matthew/esp/esp-idf/components/esp_hal_pcnt /Users/matthew/esp/esp-idf/components/esp_hal_pmu /Users/matthew/esp/esp-idf/components/esp_hal_ppa /Users/matthew/esp/esp-idf/components/esp_hal_rmt /Users/matthew/esp/esp-idf/components/esp_hal_rtc_timer /Users/matthew/esp/esp-idf/components/esp_hal_security /Users/matthew/esp/esp-idf/components/esp_hal_timg /Users/matthew/esp/esp-idf/components/esp_hal_touch_sens /Users/matthew/esp/esp-idf/components/esp_hal_twai /Users/matthew/esp/esp-idf/components/esp_hal_uart /Users/matthew/esp/esp-idf/components/esp_hal_usb /Users/matthew/esp/esp-idf/components/esp_hal_wdt /Users/matthew/esp/esp-idf/components/esp_hid /Users/matthew/esp/esp-idf/components/esp_http_client /Users/matthew/esp/esp-idf/components/esp_http_server /Users/matthew/esp/esp-idf/components/esp_https_ota /Users/matthew/esp/esp-idf/components/esp_https_server /Users/matthew/esp/esp-idf/components/esp_hw_support /Users/matthew/esp/esp-idf/components/esp_lcd /Users/matthew/esp/esp-idf/components/esp_libc /Users/matthew/esp/esp-idf/components/esp_local_ctrl /Users/matthew/esp/esp-idf/components/esp_mm /Users/matthew/esp/esp-idf/components/esp_netif /Users/matthew/esp/esp-idf/components/esp_netif_stack /Users/matthew/esp/esp-idf/components/esp_partition /Users/matthew/esp/esp-idf/components/esp_phy /Users/matthew/esp/esp-idf/components/esp_pm /Users/matthew/esp/esp-idf/components/esp_psram /Users/matthew/esp/esp-idf/components/esp_ringbuf /Users/matthew/esp/esp-idf/components/esp_rom /Users/matthew/esp/esp-idf/components/esp_security /Users/matthew/esp/esp-idf/components/esp_stdio /Users/matthew/esp/esp-idf/components/esp_system /Users/matthew/esp/esp-idf/components/esp_timer /Users/matthew/esp/esp-idf/components/esp_trace /Users/matthew/esp/esp-idf/components/esp_usb_cdc_rom_console /Users/matthew/esp/esp-idf/components/esp_wifi /Users/matthew/esp/esp-idf/components/espcoredump /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__cjson /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__cmake_utilities /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__esp_lcd_sh8601 /Users/matthew/esp/esp-idf/components/esptool_py /Users/matthew/esp/esp-idf/components/fatfs /Users/matthew/esp/esp-idf/components/freertos /Users/matthew/esp/esp-idf/components/hal /Users/matthew/esp/esp-idf/components/heap /Users/matthew/esp/esp-idf/components/http_parser /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/i2c_bsp /Users/matthew/esp/esp-idf/components/idf_test /Users/matthew/esp/esp-idf/components/ieee802154 /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/lcd_bl_pwm_bsp /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/lcd_touch_bsp /Users/matthew/esp/esp-idf/components/log /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl /Users/matthew/esp/esp-idf/components/lwip /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/main /Users/matthew/esp/esp-idf/components/mbedtls /Users/matthew/esp/esp-idf/components/nvs_flash /Users/matthew/esp/esp-idf/components/nvs_sec_provider /Users/matthew/esp/esp-idf/components/openthread /Users/matthew/esp/esp-idf/components/partition_table /Users/matthew/esp/esp-idf/components/perfmon /Users/matthew/esp/esp-idf/components/protobuf-c /Users/matthew/esp/esp-idf/components/protocomm /Users/matthew/esp/esp-idf/components/pthread /Users/matthew/esp/esp-idf/components/rt /Users/matthew/esp/esp-idf/components/sdmmc /Users/matthew/esp/esp-idf/components/soc /Users/matthew/esp/esp-idf/components/spi_flash /Users/matthew/esp/esp-idf/components/spiffs /Users/matthew/esp/esp-idf/components/tcp_transport /Users/matthew/esp/esp-idf/components/ulp /Users/matthew/esp/esp-idf/components/unity /Users/matthew/esp/esp-idf/components/vfs /Users/matthew/esp/esp-idf/components/wear_levelling /Users/matthew/esp/esp-idf/components/wpa_supplicant /Users/matthew/esp/esp-idf/components/xtensa
-- Configuring done (12.3s)
-- Generating done (1.2s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
[  0%] Preprocessing linker script /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/sections.ld.in -> /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/esp-idf/esp_system/ld/sections.ld.in
[  0%] Generating ../../ota_data_initial.bin
[  0%] Generating ../../partition_table/partition-table.bin
[  0%] Generating project_elf_src_esp32s3.c
[  0%] Preprocessing linker script /Users/matthew/esp/esp-idf/components/esp_system/ld/esp32s3/memory.ld.in -> /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/esp-idf/esp_system/ld/memory.ld
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target blank_ota_data
Partition table binary generated. Contents:
*******************************************************************************
[  0%] Building C object esp-idf/esp_https_ota/CMakeFiles/__idf_esp_https_ota.dir/src/esp_https_ota.c.obj
# ESP-IDF Partition Table
# Name, Type, SubType, Offset, Size, Flags
nvs,data,nvs,0x9000,64K,
otadata,data,ota,0x19000,8K,
phy_init,data,phy,0x1b000,4K,
ota_0,app,ota_0,0x20000,4M,
ota_1,app,ota_1,0x420000,4M,
assets,data,spiffs,0x820000,7936K,
*******************************************************************************
[  0%] Built target partition_table_bin
[  0%] Creating directories for 'bootloader'
[  0%] No download step for 'bootloader'
[  1%] No update step for 'bootloader'
[  1%] No patch step for 'bootloader'
[  1%] Performing configure step for 'bootloader'
-- Found Git: /usr/bin/git (found version "2.54.0 (Apple Git-157)")
[  1%] Linking C static library libesp_https_ota.a
-- Component directory /Users/matthew/esp/esp-idf/components/mqtt does not contain a CMakeLists.txt file. No component will be added
[  1%] Built target __idf_esp_https_ota
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_parse.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_uri.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_txrx.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/util/ctrl_sock.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_main.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_ws.c.obj
[  1%] Building C object esp-idf/esp_http_server/CMakeFiles/__idf_esp_http_server.dir/src/httpd_sess.c.obj
[  1%] Linking C static library libesp_http_server.a
[  1%] Built target __idf_esp_http_server
[  1%] Building C object esp-idf/esp_http_client/CMakeFiles/__idf_esp_http_client.dir/lib/http_auth.c.obj
[  2%] Building C object esp-idf/esp_http_client/CMakeFiles/__idf_esp_http_client.dir/lib/http_utils.c.obj
[  2%] Building C object esp-idf/esp_http_client/CMakeFiles/__idf_esp_http_client.dir/esp_http_client.c.obj
[  2%] Building C object esp-idf/esp_http_client/CMakeFiles/__idf_esp_http_client.dir/lib/http_header.c.obj
[  2%] Linking C static library libesp_http_client.a
[  2%] Built target __idf_esp_http_client
[  2%] Building C object esp-idf/tcp_transport/CMakeFiles/__idf_tcp_transport.dir/transport.c.obj
[  2%] Building C object esp-idf/tcp_transport/CMakeFiles/__idf_tcp_transport.dir/transport_internal.c.obj
[  2%] Building C object esp-idf/tcp_transport/CMakeFiles/__idf_tcp_transport.dir/transport_ssl.c.obj
[  2%] Building C object esp-idf/tcp_transport/CMakeFiles/__idf_tcp_transport.dir/transport_socks_proxy.c.obj
[  2%] Building C object esp-idf/tcp_transport/CMakeFiles/__idf_tcp_transport.dir/transport_ws.c.obj
-- Minimal build - OFF
-- The C compiler identification is GNU 15.2.0
-- The CXX compiler identification is GNU 15.2.0
-- The ASM compiler identification is GNU
-- Found assembler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-gcc
-- Detecting C compiler ABI info
[  2%] Linking C static library libtcp_transport.a
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-gcc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
[  2%] Built target __idf_tcp_transport
[  2%] Building C object esp-idf/esp_driver_i2s/CMakeFiles/__idf_esp_driver_i2s.dir/i2s_common.c.obj
[  2%] Building C object esp-idf/esp_driver_i2s/CMakeFiles/__idf_esp_driver_i2s.dir/i2s_tdm.c.obj
[  2%] Building C object esp-idf/esp_driver_i2s/CMakeFiles/__idf_esp_driver_i2s.dir/i2s_pdm.c.obj
[  2%] Building C object esp-idf/esp_driver_i2s/CMakeFiles/__idf_esp_driver_i2s.dir/i2s_platform.c.obj
[  2%] Building C object esp-idf/esp_driver_i2s/CMakeFiles/__idf_esp_driver_i2s.dir/i2s_std.c.obj
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /Users/matthew/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin/xtensa-esp32s3-elf-g++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
[  2%] Linking C static library libesp_driver_i2s.a
-- Building ESP-IDF components for target esp32s3
[  2%] Built target __idf_esp_driver_i2s
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_common.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_monitor.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/gdma/adc_dma.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_cali.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_oneshot.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_cali_curve_fitting.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_continuous.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/esp32s3/curve_fitting_coefficients.c.obj
[  2%] Building C object esp-idf/esp_adc/CMakeFiles/__idf_esp_adc.dir/adc_filter.c.obj
[  3%] Linking C static library libesp_adc.a
[  3%] Built target __idf_esp_adc
-- ESP-TEE is currently supported only on the esp32c6;esp32h2;esp32c5;esp32c61 SoCs
[  3%] Building C object esp-idf/esp-tls/CMakeFiles/__idf_esp-tls.dir/esp_tls_error_capture.c.obj
[  4%] Building C object esp-idf/esp-tls/CMakeFiles/__idf_esp-tls.dir/esp-tls-crypto/esp_tls_crypto.c.obj
[  4%] Building C object esp-idf/esp-tls/CMakeFiles/__idf_esp-tls.dir/esp_tls.c.obj
[  4%] Building C object esp-idf/esp-tls/CMakeFiles/__idf_esp-tls.dir/esp_tls_platform_port.c.obj
[  4%] Building C object esp-idf/esp-tls/CMakeFiles/__idf_esp-tls.dir/esp_tls_mbedtls.c.obj
[  4%] Linking C static library libesp-tls.a
[  4%] Built target __idf_esp-tls
[  4%] Building C object esp-idf/http_parser/CMakeFiles/__idf_http_parser.dir/http_parser.c.obj
-- KCONFIG_REPORT_VERBOSITY not set, using default
-- Project sdkconfig file /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig
NOTE: Kconfig parser version: 1
NOTE: Kconfig defaults policy: use sdkconfig
NOTE: Status: Finished successfully
-- Compiler supported targets: xtensa-esp-elf
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting CXX compiler ABI info
[  4%] Linking C static library libhttp_parser.a
-- Detecting CXX compiler ABI info - done
[  4%] Built target __idf_http_parser
[  4%] Building C object esp-idf/esp_hal_i2c/CMakeFiles/__idf_esp_hal_i2c.dir/i2c_hal_iram.c.obj
[  4%] Building C object esp-idf/esp_hal_i2c/CMakeFiles/__idf_esp_hal_i2c.dir/esp32s3/i2c_periph.c.obj
[  4%] Building C object esp-idf/esp_hal_i2c/CMakeFiles/__idf_esp_hal_i2c.dir/i2c_hal.c.obj
-- Detecting C compiler ABI info
[  4%] Linking C static library libesp_hal_i2c.a
[  4%] Built target __idf_esp_hal_i2c
-- Detecting C compiler ABI info - done
-- Detecting CXX compiler ABI info
[  4%] Building C object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/gdbstub.c.obj
[  4%] Building C object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/packet.c.obj
[  4%] Building C object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/gdbstub_transport.c.obj
[  4%] Building C object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/port/xtensa/gdbstub_xtensa.c.obj
[  4%] Building ASM object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/port/xtensa/gdbstub-entry.S.obj
[  4%] Building ASM object esp-idf/esp_gdbstub/CMakeFiles/__idf_esp_gdbstub.dir/src/port/xtensa/xt_debugexception.S.obj
-- Detecting CXX compiler ABI info - done
-- Adding linker script /Users/matthew/esp/esp-idf/components/soc/esp32s3/ld/esp32s3.peripherals.ld
-- Bootloader project name: "bootloader" version: 1
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_hal_wdt/esp32s3/rom.wdt.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.api.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.bt_funcs.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libgcc.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.version.ld
-- Adding linker script /Users/matthew/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libc.ld
[  4%] Linking C static library libesp_gdbstub.a
[  4%] Built target __idf_esp_gdbstub
-- Components: bootloader bootloader_support efuse esp_app_format esp_blockdev esp_bootloader_format esp_common esp_hal_ana_conv esp_hal_clock esp_hal_dma esp_hal_gpio esp_hal_gpspi esp_hal_mspi esp_hal_pmu esp_hal_rtc_timer esp_hal_security esp_hal_timg esp_hal_uart esp_hal_usb esp_hal_wdt esp_hw_support esp_libc esp_rom esp_security esp_stdio esp_system esptool_py freertos hal log main micro-ecc partition_table soc spi_flash xtensa
-- Component paths: /Users/matthew/esp/esp-idf/components/bootloader /Users/matthew/esp/esp-idf/components/bootloader_support /Users/matthew/esp/esp-idf/components/efuse /Users/matthew/esp/esp-idf/components/esp_app_format /Users/matthew/esp/esp-idf/components/esp_blockdev /Users/matthew/esp/esp-idf/components/esp_bootloader_format /Users/matthew/esp/esp-idf/components/esp_common /Users/matthew/esp/esp-idf/components/esp_hal_ana_conv /Users/matthew/esp/esp-idf/components/esp_hal_clock /Users/matthew/esp/esp-idf/components/esp_hal_dma /Users/matthew/esp/esp-idf/components/esp_hal_gpio /Users/matthew/esp/esp-idf/components/esp_hal_gpspi /Users/matthew/esp/esp-idf/components/esp_hal_mspi /Users/matthew/esp/esp-idf/components/esp_hal_pmu /Users/matthew/esp/esp-idf/components/esp_hal_rtc_timer /Users/matthew/esp/esp-idf/components/esp_hal_security /Users/matthew/esp/esp-idf/components/esp_hal_timg /Users/matthew/esp/esp-idf/components/esp_hal_uart /Users/matthew/esp/esp-idf/components/esp_hal_usb /Users/matthew/esp/esp-idf/components/esp_hal_wdt /Users/matthew/esp/esp-idf/components/esp_hw_support /Users/matthew/esp/esp-idf/components/esp_libc /Users/matthew/esp/esp-idf/components/esp_rom /Users/matthew/esp/esp-idf/components/esp_security /Users/matthew/esp/esp-idf/components/esp_stdio /Users/matthew/esp/esp-idf/components/esp_system /Users/matthew/esp/esp-idf/components/esptool_py /Users/matthew/esp/esp-idf/components/freertos /Users/matthew/esp/esp-idf/components/hal /Users/matthew/esp/esp-idf/components/log /Users/matthew/esp/esp-idf/components/bootloader/subproject/main /Users/matthew/esp/esp-idf/components/bootloader/subproject/components/micro-ecc /Users/matthew/esp/esp-idf/components/partition_table /Users/matthew/esp/esp-idf/components/soc /Users/matthew/esp/esp-idf/components/spi_flash /Users/matthew/esp/esp-idf/components/xtensa
-- Adding linker script /Users/matthew/esp/esp-idf/components/bootloader/subproject/main/ld/esp32s3/bootloader.ld.in
--   -> Preprocessing .in script: /Users/matthew/esp/esp-idf/components/bootloader/subproject/main/ld/esp32s3/bootloader.ld.in
-- Configuring done (3.5s)
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/smartconfig.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/wifi_init.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/esp32s3/esp_adapter.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/mesh_event.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/wifi_default.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/smartconfig_ack.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/wifi_default_ap.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/lib_printf.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/src/wifi_netif.c.obj
[  4%] Building C object esp-idf/esp_wifi/CMakeFiles/__idf_esp_wifi.dir/regulatory/esp_wifi_regulatory.c.obj
[  4%] Linking C static library libesp_wifi.a
-- Generating done (0.3s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/bootloader
[  4%] Performing build step for 'bootloader'
[  4%] Built target __idf_esp_wifi
[  4%] Building C object esp-idf/esp_coex/CMakeFiles/__idf_esp_coex.dir/src/coexist_debug_diagram.c.obj
[  4%] Building C object esp-idf/esp_coex/CMakeFiles/__idf_esp_coex.dir/esp32s3/esp_coex_adapter.c.obj
[  4%] Building C object esp-idf/esp_coex/CMakeFiles/__idf_esp_coex.dir/src/coexist_debug.c.obj
[  3%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_timestamp_common.c.obj
[  3%] Generating project_elf_src_esp32s3.c
[  3%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/buffer/log_buffers.c.obj
[  3%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/noos/log_timestamp.c.obj
[  3%] Preprocessing linker script /Users/matthew/esp/esp-idf/components/bootloader/subproject/main/ld/esp32s3/bootloader.ld.in -> /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/bootloader/ld/bootloader.ld
[  4%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/noos/log_lock.c.obj
[  5%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/noos/util.c.obj
[  6%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/util.c.obj
[  6%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_format_text.c.obj
[  6%] Built target _project_elf_src
[  7%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_print.c.obj
[  7%] Built target bootloader_ld_in_preprocess
[  8%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log.c.obj
[  8%] Linking C static library liblog.a
[  4%] Linking C static library libesp_coex.a
[  4%] Built target __idf_esp_coex
[  8%] Built target __idf_log
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/ieee802_1x.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/port/os_xtensa.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/ap_config.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/port/eloop.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/pmksa_cache_auth.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/wpa_auth.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/comeback_token.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/wpa_auth_ie.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/sta_info.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/common/sae.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/ap/ieee802_11.c.obj
[  8%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_sys.c.obj
[  9%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_print.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/common/dragonfly.c.obj
[  4%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/common/wpa_common.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/bitfield.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/aes-siv.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha256-kdf.c.obj
[ 10%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_crc.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/ccmp.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/aes-gcm.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/dh_group5.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/crypto_ops.c.obj
[ 10%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_serial_output.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/dh_groups.c.obj
[ 11%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_spiflash.c.obj
[ 12%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_efuse.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/ms_funcs.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha1-tlsprf.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha256-tlsprf.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha384-tlsprf.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha384-prf.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha256-prf.c.obj
[  5%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha1-prf.c.obj
[ 12%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_gpio.c.obj
[ 13%] Building ASM object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_longjmp.S.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/md4-internal.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/sha1-tprf.c.obj
[ 14%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_systimer.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_common/eap_wsc_common.c.obj
[ 14%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_cache_esp32s2_esp32s3.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/common/ieee802_11_common.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/chap.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_common.c.obj
[ 15%] Building ASM object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_cache_writeback_esp32s3.S.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_peap.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_mschapv2.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_peap_common.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_tls.c.obj
[ 16%] Linking C static library libesp_rom.a
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_tls_common.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_ttls.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/mschapv2.c.obj
[  6%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_fast.c.obj
[ 16%] Built target __idf_esp_rom
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_fast_common.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/eap_peer/eap_fast_pac.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/rsn_supp/pmksa_cache.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/rsn_supp/wpa.c.obj
[ 17%] Building C object esp-idf/esp_common/CMakeFiles/__idf_esp_common.dir/src/esp_err_to_name.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/rsn_supp/wpa_ie.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/base64.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/common.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/ext_password.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/uuid.c.obj
[ 18%] Linking C static library libesp_common.a
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/wpabuf.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/wpa_debug.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/utils/json.c.obj
[ 18%] Built target __idf_esp_common
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_attr_build.c.obj
[  7%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_attr_parse.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_attr_process.c.obj
[ 18%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/cpu.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_common.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_dev_attr.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/wps/wps_enrollee.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/common/sae_pk.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_eap_client.c.obj
[ 19%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/esp_cpu_intr.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_wpa2_api_port.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_wpa_main.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_wpas_glue.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_common.c.obj
[ 20%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/esp_memory_utils.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_wps.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_wpa3.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_owe.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/esp_hostap.c.obj
[  8%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/tls_mbedtls.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/crypto_mbedtls.c.obj
[ 20%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/cpu_region_protect.c.obj
[ 21%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_clk.c.obj
[ 22%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_clk_init.c.obj
[ 22%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_init.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/crypto_mbedtls-bignum.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/crypto_mbedtls-rsa.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/crypto_mbedtls-ec.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/fastpsk.c.obj
[ 23%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_sleep.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/rc4.c.obj
[ 24%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_time.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/des-internal.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/esp_supplicant/src/crypto/fastpbkdf2.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/aes-wrap.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/aes-unwrap.c.obj
[  9%] Building C object esp-idf/wpa_supplicant/CMakeFiles/__idf_wpa_supplicant.dir/src/crypto/aes-ccm.c.obj
[ 24%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/chip_info.c.obj
[ 25%] Linking C static library libesp_hw_support.a
[ 25%] Built target __idf_esp_hw_support
[ 25%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/esp_err.c.obj
[  9%] Linking C static library libwpa_supplicant.a
[ 26%] Linking C static library libesp_system.a
[ 26%] Built target __idf_esp_system
[ 26%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_fields.c.obj
[ 28%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_utility.c.obj
[ 28%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_table.c.obj
[ 29%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_rtc_calib.c.obj
[ 30%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_fields.c.obj
[ 30%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_api.c.obj
[ 31%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_utility.c.obj
[ 31%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/efuse_controller/keys/with_key_purposes/esp_efuse_api_key.c.obj
[  9%] Built target __idf_wpa_supplicant
[  9%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/esp_netif_objects.c.obj
[  9%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/esp_netif_lwip.c.obj
[  9%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/esp_netif_handlers.c.obj
[  9%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/esp_netif_sntp.c.obj
[ 10%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/esp_netif_defaults.c.obj
[ 10%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/netif/ethernetif.c.obj
[ 10%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/netif/esp_pbuf_ref.c.obj
[ 10%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/esp_netif_lwip_defaults.c.obj
[ 10%] Building C object esp-idf/esp_netif/CMakeFiles/__idf_esp_netif.dir/lwip/netif/wlanif.c.obj
[ 32%] Linking C static library libefuse.a
[ 32%] Built target __idf_efuse
[ 32%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_common.c.obj
[ 33%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_mem.c.obj
[ 34%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_common_loader.c.obj
[ 35%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_random.c.obj
[ 35%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_clock_init.c.obj
[ 35%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_efuse.c.obj
[ 36%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/flash_encrypt.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/secure_boot.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_random_esp32s3.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/bootloader_flash.c.obj
[ 39%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/flash_qio_mode.c.obj
[ 39%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/bootloader_flash_config_esp32s3.c.obj
[ 40%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_utility.c.obj
[ 41%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/flash_partitions.c.obj
[ 41%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/esp_image_format.c.obj
[ 42%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_sha.c.obj
[ 43%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_init.c.obj
[ 43%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_clock_loader.c.obj
[ 44%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_console.c.obj
[ 44%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_console_loader.c.obj
[ 45%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/esp32s3/bootloader_soc.c.obj
[ 46%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/esp32s3/bootloader_esp32s3.c.obj
[ 46%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_panic.c.obj
[ 47%] Linking C static library libbootloader_support.a
[ 47%] Built target __idf_bootloader_support
[ 48%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_crypto_lock.c.obj
[ 48%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_crypto_periph_clk.c.obj
[ 10%] Linking C static library libesp_netif.a
[ 10%] Built target __idf_esp_netif
[ 50%] Linking C static library libesp_security.a
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/apps/sntp/sntp.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/netdb.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/if_api.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/netbuf.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/api_lib.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/err.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/api_msg.c.obj
[ 10%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/apps/sntp/sntp.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/tcpip.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/netifapi.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/api/sockets.c.obj
[ 50%] Built target __idf_esp_security
[ 51%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/mpu_hal.c.obj
[ 52%] Linking C static library libesp_hal_security.a
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/apps/netbiosns/netbiosns.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/def.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/dns.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/inet_chksum.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/init.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ip.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/mem.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/memp.c.obj
[ 52%] Built target __idf_esp_hal_security
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/netif.c.obj
[ 52%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/esp32s3/adc_periph.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/raw.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/pbuf.c.obj
[ 11%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/stats.c.obj
[ 53%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_hal_common.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/sys.c.obj
[ 53%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_oneshot_hal.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/tcp.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/tcp_in.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/tcp_out.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/timeouts.c.obj
[ 54%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_hal.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/udp.c.obj
[ 55%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/temperature_sensor_hal.c.obj
[ 55%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/esp32s3/temperature_sensor_periph.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/autoip.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/dhcp.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/etharp.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/icmp.c.obj
[ 56%] Linking C static library libesp_hal_ana_conv.a
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/igmp.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/ip4.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/ip4_napt.c.obj
[ 56%] Built target __idf_esp_hal_ana_conv
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/ip4_addr.c.obj
[ 12%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv4/ip4_frag.c.obj
[ 56%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uart_hal.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/dhcp6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/inet6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/icmp6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/ethip6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/ip6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/ip6_addr.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/ip6_frag.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/mld6.c.obj
[ 57%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uart_hal_iram.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/core/ipv6/nd6.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ethernet.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/bridgeif.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/bridgeif_fdb.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/auth.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/slipif.c.obj
[ 58%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/esp32s3/uart_periph.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/ccp.c.obj
[ 13%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/chap-new.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/chap-md5.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/chap_ms.c.obj
[ 58%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uhci_hal.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/demand.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/eap.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/ecp.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/eui64.c.obj
[ 59%] Linking C static library libesp_hal_uart.a
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/fsm.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/ipcp.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/ipv6cp.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/lcp.c.obj
[ 59%] Built target __idf_esp_hal_uart
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/magic.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/mppe.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/multilink.c.obj
[ 14%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/ppp.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/pppapi.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/pppcrypt.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/pppoe.c.obj
[ 60%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/esp32s3/mwdt_periph.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/pppol2tp.c.obj
[ 60%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/rom_patch.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/pppos.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/upap.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/utils.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/vj.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/hooks/tcp_isn_default.c.obj
[ 61%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/xt_wdt_hal.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/hooks/lwip_default_hooks.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/sockets_ext.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/debug/lwip_debug.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/freertos/sys_arch.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/if_index.c.obj
[ 15%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/acd_dhcp_check.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/port/esp32xx/vfs_lwip.c.obj
[ 62%] Linking C static library libesp_hal_wdt.a
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/apps/ping/ping_sock.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/polarssl/arc4.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/polarssl/des.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/polarssl/md4.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/polarssl/md5.c.obj
[ 62%] Built target __idf_esp_hal_wdt
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/lwip/src/netif/ppp/polarssl/sha1.c.obj
[ 16%] Building C object esp-idf/lwip/CMakeFiles/__idf_lwip.dir/apps/dhcpserver/dhcpserver.c.obj
[ 63%] Building C object esp-idf/esp_hal_timg/CMakeFiles/__idf_esp_hal_timg.dir/esp32s3/timer_periph.c.obj
[ 63%] Building C object esp-idf/esp_hal_timg/CMakeFiles/__idf_esp_hal_timg.dir/timer_hal.c.obj
[ 64%] Linking C static library libesp_hal_timg.a
[ 64%] Built target __idf_esp_hal_timg
[ 65%] Building C object esp-idf/esp_bootloader_format/CMakeFiles/__idf_esp_bootloader_format.dir/esp_bootloader_desc.c.obj
[ 65%] Linking C static library libesp_bootloader_format.a
[ 16%] Linking C static library liblwip.a
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_wrap.c.obj
[ 66%] Linking C static library libspi_flash.a
[ 66%] Built target __idf_spi_flash
[ 16%] Built target __idf_lwip
[ 67%] Building C object esp-idf/esp_hal_clock/CMakeFiles/__idf_esp_hal_clock.dir/esp32s3/clk_tree_hal.c.obj
[ 16%] Building C object esp-idf/vfs/CMakeFiles/__idf_vfs.dir/vfs_calls.c.obj
[ 16%] Building C object esp-idf/vfs/CMakeFiles/__idf_vfs.dir/vfs_eventfd.c.obj
[ 16%] Building C object esp-idf/vfs/CMakeFiles/__idf_vfs.dir/vfs.c.obj
[ 16%] Building C object esp-idf/vfs/CMakeFiles/__idf_vfs.dir/nullfs.c.obj
[ 16%] Building C object esp-idf/vfs/CMakeFiles/__idf_vfs.dir/vfs_semihost.c.obj
[ 67%] Linking C static library libesp_hal_clock.a
[ 67%] Built target __idf_esp_hal_clock
[ 69%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_hal_iram.c.obj
[ 69%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_hal.c.obj
[ 69%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hal.c.obj
[ 69%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hd_hal.c.obj
[ 70%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hal_iram.c.obj
[ 70%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/esp32s3/spi_periph.c.obj
[ 16%] Linking C static library libvfs.a
[ 71%] Linking C static library libesp_hal_gpspi.a
[ 16%] Built target __idf_vfs
[ 16%] Building C object esp-idf/esp_driver_usb_serial_jtag/CMakeFiles/__idf_esp_driver_usb_serial_jtag.dir/src/usb_serial_jtag_connection_monitor.c.obj
[ 16%] Building C object esp-idf/esp_driver_usb_serial_jtag/CMakeFiles/__idf_esp_driver_usb_serial_jtag.dir/src/usb_serial_jtag.c.obj
[ 16%] Building C object esp-idf/esp_driver_usb_serial_jtag/CMakeFiles/__idf_esp_driver_usb_serial_jtag.dir/src/usb_serial_jtag_vfs.c.obj
[ 71%] Built target __idf_esp_hal_gpspi
[ 72%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/gdma_hal_ahb_v1.c.obj
[ 73%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/esp32s3/gdma_periph.c.obj
[ 73%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/gdma_hal_top.c.obj
[ 74%] Linking C static library libesp_hal_dma.a
[ 16%] Linking C static library libesp_driver_usb_serial_jtag.a
[ 74%] Built target __idf_esp_hal_dma
[ 16%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Building C object esp-idf/micro-ecc/CMakeFiles/__idf_micro-ecc.dir/uECC_verify_antifault.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/src/lib_printf.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/src/phy_override.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/src/phy_init.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/esp32s3/phy_init_data.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/src/phy_common.c.obj
[ 16%] Building C object esp-idf/esp_phy/CMakeFiles/__idf_esp_phy.dir/src/btbb_init.c.obj
[ 16%] Linking C static library libesp_phy.a
[ 16%] Built target __idf_esp_phy
[ 16%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_cxx_api.cpp.obj
[ 16%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_page.cpp.obj
[ 16%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_item_hash_list.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_api.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_pagemanager.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_storage.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_partition_manager.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_handle_simple.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_partition_lookup.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_partition.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_handle_locked.cpp.obj
[ 75%] Linking C static library libmicro-ecc.a
[ 75%] Built target __idf_micro-ecc
[ 76%] Building C object esp-idf/esp_hal_pmu/CMakeFiles/__idf_esp_hal_pmu.dir/esp32s3/rtc_cntl_hal.c.obj
[ 76%] Linking C static library libesp_hal_pmu.a
[ 76%] Built target __idf_esp_hal_pmu
[ 77%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_dwc_hal.c.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_types.cpp.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_platform.cpp.obj
[ 17%] Building C object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_bootloader.c.obj
[ 77%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_wrap_hal.c.obj
[ 78%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/esp32s3/usb_dwc_periph.c.obj
[ 17%] Building CXX object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_encrypted_partition.cpp.obj
[ 78%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_serial_jtag_hal.c.obj
[ 79%] Linking C static library libesp_hal_usb.a
[ 79%] Built target __idf_esp_hal_usb
[ 80%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/gpio_hal.c.obj
[ 80%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/rtc_io_hal.c.obj
[ 17%] Building C object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_bootloader_aes.c.obj
[ 81%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/rtc_io_periph.c.obj
[ 82%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/dedic_gpio_periph.c.obj
[ 17%] Building C object esp-idf/nvs_flash/CMakeFiles/__idf_nvs_flash.dir/src/nvs_bootloader_xts_aes.c.obj
[ 82%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/sdm_hal.c.obj
[ 83%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/sdm_periph.c.obj
[ 84%] Linking C static library libesp_hal_gpio.a
[ 84%] Built target __idf_esp_hal_gpio
[ 17%] Linking C static library libnvs_flash.a
[ 85%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/hal_utils.c.obj
[ 85%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/efuse_hal.c.obj
[ 86%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/mmu_hal.c.obj
[ 87%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/esp32s3/efuse_hal.c.obj
[ 87%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/cache_hal.c.obj
[ 17%] Built target __idf_nvs_flash
[ 88%] Linking C static library libhal.a
[ 18%] Building C object esp-idf/nvs_sec_provider/CMakeFiles/__idf_nvs_sec_provider.dir/nvs_sec_provider.c.obj
[ 88%] Built target __idf_hal
[ 89%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/lldesc.c.obj
[ 89%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/dport_access_common.c.obj
[ 89%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/gpio_periph.c.obj
[ 90%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/sdmmc_periph.c.obj
[ 92%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/power_supply_periph.c.obj
[ 92%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/mpi_periph.c.obj
[ 92%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/interrupts.c.obj
[ 18%] Linking C static library libnvs_sec_provider.a
[ 93%] Linking C static library libsoc.a
[ 18%] Built target __idf_nvs_sec_provider
[ 18%] Building C object esp-idf/esp_event/CMakeFiles/__idf_esp_event.dir/esp_event.c.obj
[ 18%] Building C object esp-idf/esp_event/CMakeFiles/__idf_esp_event.dir/esp_event_private.c.obj
[ 18%] Building C object esp-idf/esp_event/CMakeFiles/__idf_esp_event.dir/default_event_loop.c.obj
[ 93%] Built target __idf_soc
[ 95%] Building C object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/eri.c.obj
[ 95%] Building C object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xt_trax.c.obj
[ 95%] Linking C static library libxtensa.a
[ 95%] Built target __idf_xtensa
[ 96%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/bootloader_start.c.obj
[ 19%] Linking C static library libesp_event.a
[ 97%] Linking C static library libmain.a
[ 19%] Built target __idf_esp_event
[ 97%] Built target __idf_main
[ 19%] Building C object esp-idf/esp_driver_uart/CMakeFiles/__idf_esp_driver_uart.dir/src/uart.c.obj
[ 20%] Building C object esp-idf/esp_driver_uart/CMakeFiles/__idf_esp_driver_uart.dir/src/uart_wakeup.c.obj
[ 20%] Building C object esp-idf/esp_driver_uart/CMakeFiles/__idf_esp_driver_uart.dir/src/uhci.c.obj
[ 20%] Building C object esp-idf/esp_driver_uart/CMakeFiles/__idf_esp_driver_uart.dir/src/uart_vfs.c.obj
[ 98%] Building C object CMakeFiles/bootloader.elf.dir/project_elf_src_esp32s3.c.obj
[ 98%] Linking C executable bootloader.elf
[ 98%] Built target bootloader.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/bootloader/bootloader.bin
[100%] Built target gen_bootloader_binary
[100%] Built target gen_project_binary
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[100%] Built target bootloader_check_size
[100%] Built target app
[ 20%] No install step for 'bootloader'
[ 20%] Completed 'bootloader'
[ 20%] Built target bootloader
[ 20%] Linking C static library libesp_driver_uart.a
[ 20%] Built target __idf_esp_driver_uart
[ 20%] Building C object esp-idf/esp_psram/CMakeFiles/__idf_esp_psram.dir/esp32s3/esp_psram_impl_octal.c.obj
[ 20%] Building C object esp-idf/esp_psram/CMakeFiles/__idf_esp_psram.dir/system_layer/esp_psram_mspi.c.obj
[ 20%] Building C object esp-idf/esp_psram/CMakeFiles/__idf_esp_psram.dir/system_layer/esp_psram.c.obj
[ 20%] Building C object esp-idf/esp_psram/CMakeFiles/__idf_esp_psram.dir/xip_impl/mmu_psram_flash.c.obj
[ 20%] Linking C static library libesp_psram.a
[ 20%] Built target __idf_esp_psram
[ 20%] Building C object esp-idf/esp_ringbuf/CMakeFiles/__idf_esp_ringbuf.dir/ringbuf.c.obj
[ 20%] Linking C static library libesp_ringbuf.a
[ 20%] Built target __idf_esp_ringbuf
[ 20%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/esp_timer_init.c.obj
[ 20%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/system_time.c.obj
[ 20%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/esp_timer.c.obj
[ 21%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/esp_timer_impl_systimer.c.obj
[ 21%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/ets_timer_legacy.c.obj
[ 21%] Building C object esp-idf/esp_timer/CMakeFiles/__idf_esp_timer.dir/src/esp_timer_impl_common.c.obj
[ 21%] Linking C static library libesp_timer.a
[ 21%] Built target __idf_esp_timer
[ 22%] Building CXX object esp-idf/cxx/CMakeFiles/__idf_cxx.dir/cxx_init.cpp.obj
[ 22%] Building CXX object esp-idf/cxx/CMakeFiles/__idf_cxx.dir/cxx_exception_stubs.cpp.obj
[ 22%] Building CXX object esp-idf/cxx/CMakeFiles/__idf_cxx.dir/cxx_guards.cpp.obj
[ 22%] Linking C static library libcxx.a
[ 22%] Built target __idf_cxx
[ 22%] Building C object esp-idf/pthread/CMakeFiles/__idf_pthread.dir/pthread.c.obj
[ 22%] Building C object esp-idf/pthread/CMakeFiles/__idf_pthread.dir/pthread_local_storage.c.obj
[ 22%] Building C object esp-idf/pthread/CMakeFiles/__idf_pthread.dir/pthread_cond_var.c.obj
[ 22%] Building C object esp-idf/pthread/CMakeFiles/__idf_pthread.dir/pthread_rwlock.c.obj
[ 22%] Building C object esp-idf/pthread/CMakeFiles/__idf_pthread.dir/pthread_semaphore.c.obj
[ 22%] Linking C static library libpthread.a
[ 22%] Built target __idf_pthread
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/assert.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/heap.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/abort.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/getentropy.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/pthread.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/locks.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/random.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/init.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/stdatomic.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/poll.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/time.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/termios.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/sysconf.c.obj
[ 22%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/scandir.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/realpath.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/syscalls.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/reent_syscalls.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/picolibc/picolibc_init.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/port/esp_time_impl.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/port/xtensa/stdatomic_s32c1i.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/picolibc/rand.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/picolibc/open_memstream.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/picolibc/errno.c.obj
[ 23%] Building C object esp-idf/esp_libc/CMakeFiles/__idf_esp_libc.dir/src/picolibc/getreent.c.obj
[ 23%] Linking C static library libesp_libc.a
[ 23%] Built target __idf_esp_libc
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/app_startup.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/heap_idf.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/port_systick.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/port_common.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/list.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/queue.c.obj
[ 24%] Building ASM object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/portable/xtensa/portasm.S.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/tasks.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/timers.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/portable/xtensa/port.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/event_groups.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/stream_buffer.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/portable/xtensa/xtensa_init.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/FreeRTOS-Kernel/portable/xtensa/xtensa_overlay_os_hook.c.obj
[ 24%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/esp_additions/idf_additions_event_groups.c.obj
[ 25%] Building C object esp-idf/freertos/CMakeFiles/__idf_freertos.dir/esp_additions/idf_additions.c.obj
[ 25%] Linking C static library libfreertos.a
[ 25%] Built target __idf_freertos
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/cpu.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/hw_random.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/esp_clk.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/clk_ctrl_os.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/esp_cpu_intr.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/periph_ctrl.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/intr_alloc.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/esp_memory_utils.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/cpu_region_protect.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/rtc_module.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/mac_addr.c.obj
[ 25%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/revision.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/regi2c_ctrl.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/esp_gpio_reserve.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sar_tsens_ctrl.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/io_mux.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/esp_clk_tree.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/spi_bus_lock.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/clk_utils.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/usb_phy/usb_phy.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp_clk_tree_common.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/spi_share_hw_ctrl.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/adc_share_hw_ctrl.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_modem.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_modes.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_uart.c.obj
[ 26%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_console.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_mspi.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_usb.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_gpio.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_event.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/systimer.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/sleep_wake_stub.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/power_supply/brownout.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/mspi_timing_tuning/mspi_timing_tuning.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/esp_clock_output.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_clk.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_clk_init.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_init.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_sleep.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/rtc_time.c.obj
[ 27%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/chip_info.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/sar_periph_ctrl.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp32s3/esp_memprot.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/port/esp_memprot_conv.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/mspi_timing_tuning/port/esp32s3/mspi_timing_config.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/mspi_timing_tuning/port/esp32s3/mspi_timing_by_mspi_delay.c.obj
[ 28%] Building C object esp-idf/esp_hw_support/CMakeFiles/__idf_esp_hw_support.dir/lowpower/port/esp32s3/sleep_cpu.c.obj
[ 28%] Linking C static library libesp_hw_support.a
[ 28%] Built target __idf_esp_hw_support
[ 28%] Building C object esp-idf/esp_hal_i2s/CMakeFiles/__idf_esp_hal_i2s.dir/esp32s3/i2s_periph.c.obj
[ 28%] Building C object esp-idf/esp_hal_i2s/CMakeFiles/__idf_esp_hal_i2s.dir/i2s_hal.c.obj
[ 29%] Linking C static library libesp_hal_i2s.a
[ 29%] Built target __idf_esp_hal_i2s
[ 29%] Building C object esp-idf/esp_hal_touch_sens/CMakeFiles/__idf_esp_hal_touch_sens.dir/esp32s3/touch_sensor_legacy_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_touch_sens/CMakeFiles/__idf_esp_hal_touch_sens.dir/touch_sensor_legacy_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_touch_sens/CMakeFiles/__idf_esp_hal_touch_sens.dir/touch_sens_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_touch_sens/CMakeFiles/__idf_esp_hal_touch_sens.dir/esp32s3/touch_sensor_periph.c.obj
[ 29%] Linking C static library libesp_hal_touch_sens.a
[ 29%] Built target __idf_esp_hal_touch_sens
[ 29%] Building C object esp-idf/esp_hal_pmu/CMakeFiles/__idf_esp_hal_pmu.dir/esp32s3/rtc_cntl_hal.c.obj
[ 29%] Linking C static library libesp_hal_pmu.a
[ 29%] Built target __idf_esp_hal_pmu
[ 29%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_dwc_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_wrap_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/usb_serial_jtag_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_usb/CMakeFiles/__idf_esp_hal_usb.dir/esp32s3/usb_dwc_periph.c.obj
[ 29%] Linking C static library libesp_hal_usb.a
[ 29%] Built target __idf_esp_hal_usb
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/gpio_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/rtc_io_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/rtc_io_periph.c.obj
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/sdm_periph.c.obj
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/sdm_hal.c.obj
[ 29%] Building C object esp-idf/esp_hal_gpio/CMakeFiles/__idf_esp_hal_gpio.dir/esp32s3/dedic_gpio_periph.c.obj
[ 30%] Linking C static library libesp_hal_gpio.a
[ 30%] Built target __idf_esp_hal_gpio
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/lldesc.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/interrupts.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/dport_access_common.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/gpio_periph.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/mpi_periph.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/power_supply_periph.c.obj
[ 30%] Building C object esp-idf/soc/CMakeFiles/__idf_soc.dir/esp32s3/sdmmc_periph.c.obj
[ 31%] Linking C static library libsoc.a
[ 31%] Built target __idf_soc
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/heap_caps.c.obj
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/heap_caps_base.c.obj
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/heap_caps_init.c.obj
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/port/esp32s3/memory_layout.c.obj
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/port/memory_layout_utils.c.obj
[ 31%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/tlsf/tlsf.c.obj
[ 32%] Building C object esp-idf/heap/CMakeFiles/__idf_heap.dir/multi_heap.c.obj
[ 32%] Linking C static library libheap.a
[ 32%] Built target __idf_heap
[ 32%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/os/log_timestamp.c.obj
[ 32%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/buffer/log_buffers.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/os/util.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/os/log_lock.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_print.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_timestamp_common.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/util.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_format_text.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/os/log_write.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_level/tag_log_level/tag_log_level.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_level/log_level.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_level/tag_log_level/linked_list/log_linked_list.c.obj
[ 33%] Building C object esp-idf/log/CMakeFiles/__idf_log.dir/src/log_level/tag_log_level/cache/log_binary_heap.c.obj
[ 33%] Linking C static library liblog.a
[ 33%] Built target __idf_log
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/hal_utils.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/efuse_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/mmu_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/sdmmc_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/esp32s3/efuse_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/brownout_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/color_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/systimer_hal.c.obj
[ 33%] Building C object esp-idf/hal/CMakeFiles/__idf_hal.dir/cache_hal.c.obj
[ 33%] Linking C static library libhal.a
[ 33%] Built target __idf_hal
[ 33%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_sys.c.obj
[ 33%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_spiflash.c.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_efuse.c.obj
[ 34%] Building ASM object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_longjmp.S.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_print.c.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_cache_esp32s2_esp32s3.c.obj
[ 34%] Building ASM object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_cache_writeback_esp32s3.S.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_gpio.c.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_systimer.c.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_crc.c.obj
[ 34%] Building C object esp-idf/esp_rom/CMakeFiles/__idf_esp_rom.dir/patches/esp_rom_serial_output.c.obj
[ 34%] Linking C static library libesp_rom.a
[ 34%] Built target __idf_esp_rom
[ 34%] Building C object esp-idf/esp_common/CMakeFiles/__idf_esp_common.dir/src/esp_err_to_name.c.obj
[ 34%] Linking C static library libesp_common.a
[ 34%] Built target __idf_esp_common
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/crosscore_int.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/esp_ipc.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/panic.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/system_time.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/stack_check.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/int_wdt.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/esp_system.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/esp_err.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/ubsan.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/freertos_hooks.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/startup_funcs.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/startup.c.obj
[ 34%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/xt_wdt.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/task_wdt/task_wdt.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/panic_handler.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/cpu_start.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/task_wdt/task_wdt_impl_timergroup.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/image_process.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/esp_system_chip.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/esp_ipc_isr.c.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/esp_ipc_isr_port.c.obj
[ 35%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/esp_ipc_isr_handler.S.obj
[ 35%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/esp_ipc_isr_routines.S.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/panic_arch.c.obj
[ 35%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/panic_handler_asm.S.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/expression_with_stack.c.obj
[ 35%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/expression_with_stack_asm.S.obj
[ 35%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/debug_helpers.c.obj
[ 36%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/debug_helpers_asm.S.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/debug_stubs.c.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/arch/xtensa/trax.c.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/clk.c.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/reset_reason.c.obj
[ 36%] Building ASM object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/highint_hdl.S.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/system_internal.c.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/cache_err_int.c.obj
[ 36%] Building C object esp-idf/esp_system/CMakeFiles/__idf_esp_system.dir/port/soc/esp32s3/apb_backup_dma.c.obj
[ 36%] Linking C static library libesp_system.a
[ 36%] Built target __idf_esp_system
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_hpm_enable.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_gd.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_winbond.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_issi.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_mxic.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_generic.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_boya.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/flash_brownout_hook.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/esp32s3/spi_flash_oct_flash_init.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_th.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_drivers.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_chip_mxic_opi.c.obj
[ 36%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/memspi_host_driver.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/cache_utils.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_blockdev.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/flash_mmap.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/flash_ops.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_wrap.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/esp_flash_api.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/esp_flash_spi_init.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_os_func_app.c.obj
[ 37%] Building C object esp-idf/spi_flash/CMakeFiles/__idf_spi_flash.dir/spi_flash_os_func_noos.c.obj
[ 37%] Linking C static library libspi_flash.a
[ 37%] Built target __idf_spi_flash
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_common_loader.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_common.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_random.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_mem.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_clock_init.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/secure_boot.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/flash_encrypt.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/bootloader_flash_config_esp32s3.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/bootloader_flash.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_random_esp32s3.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/bootloader_flash/src/flash_qio_mode.c.obj
[ 37%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_efuse.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_utility.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/esp_image_format.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/bootloader_sha.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/flash_partitions.c.obj
[ 38%] Building C object esp-idf/bootloader_support/CMakeFiles/__idf_bootloader_support.dir/src/esp32s3/secure_boot_secure_features.c.obj
[ 38%] Linking C static library libbootloader_support.a
[ 38%] Built target __idf_bootloader_support
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/esp32s3/adc_periph.c.obj
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_hal_common.c.obj
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_hal.c.obj
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/esp32s3/temperature_sensor_periph.c.obj
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/temperature_sensor_hal.c.obj
[ 38%] Building C object esp-idf/esp_hal_ana_conv/CMakeFiles/__idf_esp_hal_ana_conv.dir/adc_oneshot_hal.c.obj
[ 38%] Linking C static library libesp_hal_ana_conv.a
[ 38%] Built target __idf_esp_hal_ana_conv
[ 38%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/esp32s3/mwdt_periph.c.obj
[ 38%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/xt_wdt_hal.c.obj
[ 38%] Building C object esp-idf/esp_hal_wdt/CMakeFiles/__idf_esp_hal_wdt.dir/rom_patch.c.obj
[ 38%] Linking C static library libesp_hal_wdt.a
[ 38%] Built target __idf_esp_hal_wdt
[ 38%] Building C object esp-idf/esp_hal_timg/CMakeFiles/__idf_esp_hal_timg.dir/esp32s3/timer_periph.c.obj
[ 38%] Building C object esp-idf/esp_hal_timg/CMakeFiles/__idf_esp_hal_timg.dir/timer_hal.c.obj
[ 38%] Linking C static library libesp_hal_timg.a
[ 38%] Built target __idf_esp_hal_timg
[ 38%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_crypto.c.obj
[ 38%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_its_file.c.obj
[ 38%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_crypto_client.c.obj
[ 38%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_crypto_driver_wrappers_no_static.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/tf_psa_crypto_version.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_crypto_slot_management.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/dma/esp_aes_gdma_impl.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/psa_crypto_storage.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/tf_psa_crypto_config.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_crypto_storage/esp_psa_its.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/dma/esp_aes_dma_core.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/sha/core/esp_sha_gdma_impl.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/crypto_shared_gdma/esp_crypto_shared_gdma.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_hardware.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_mem.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/esp_aes_xts.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/esp_aes_common.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/esp_aes.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/aes/esp_aes_gcm.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_sha/core/psa_crypto_driver_esp_sha256.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_sha/psa_crypto_driver_esp_sha.c.obj
[ 39%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_sha/core/psa_crypto_driver_esp_sha512.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/sha/esp_sha.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_mac/psa_crypto_driver_esp_hmac_transparent.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_sha/core/psa_crypto_driver_esp_sha1.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/sha/core/sha.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_md/psa_crypto_driver_esp_md5.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_mac/psa_crypto_driver_esp_hmac_opaque.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_rsa_ds/psa_crypto_driver_esp_rsa_ds.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_rsa_ds/psa_crypto_driver_esp_rsa_ds_utilities.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_hmac_pbkdf2.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/bignum/bignum_alt.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/bignum/esp_bignum.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_aes/psa_crypto_driver_esp_aes.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_aes/psa_crypto_driver_esp_aes_gcm.c.obj
[ 40%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/core/CMakeFiles/tfpsacrypto.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/psa_driver/esp_mac/psa_crypto_driver_esp_cmac.c.obj
[ 41%] Linking CXX static library libtfpsacrypto.a
[ 41%] Built target tfpsacrypto
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/mps_trace.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/debug.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_msg.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_debug_helpers_generated.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/mps_reader.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_cookie.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_cache.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_ciphersuites.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls12_client.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_client.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_ticket.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls12_server.c.obj
[ 41%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls13_server.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls13_keys.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls13_client.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/ssl_tls13_generic.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/timing.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/version.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/version_features.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/mbedtls_debug.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_platform_time.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_timing.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/esp_psa_crypto_init.c.obj
[ 42%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedtls.dir/Users/matthew/esp/esp-idf/components/mbedtls/port/net_sockets.c.obj
[ 42%] Linking CXX static library libmbedtls.a
[ 42%] Built target mbedtls
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/error.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/pkcs7.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509_create.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509_crl.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509_crt.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509_oid.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/mbedtls_config.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509_csr.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509write.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509write_crt.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/library/CMakeFiles/mbedx509.dir/x509write_csr.c.obj
[ 43%] Linking CXX static library libmbedx509.a
[ 43%] Built target mbedx509
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/aria.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/asn1write.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/aesce.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/base64.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/bignum_core.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/bignum.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/bignum_mod.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/asn1parse.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/aesni.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/aes.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/bignum_mod_raw.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/block_cipher.c.obj
[ 43%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/camellia.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/chacha20.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/chachapoly.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ccm.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/cipher_wrap.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/cipher.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/constant_time.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/cmac.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ctr_drbg.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecdh.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecdsa.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecjpake.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecp.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecp_curves.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ecp_curves_new.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/entropy.c.obj
[ 44%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/entropy_poll.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/gcm.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/hmac_drbg.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/lmots.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/lms.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/md.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/md5.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/nist_kw.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/memory_buffer_alloc.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/oid.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pem.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pk.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pk_ecc.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pk_rsa.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pk_wrap.c.obj
[ 45%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pkcs5.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pkparse.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/pkwrite.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/platform.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/platform_util.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/poly1305.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_aead.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_cipher.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_ecp.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_ffdh.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_hash.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_pake.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_mac.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_crypto_rsa.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/psa_util.c.obj
[ 46%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/ripemd160.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/rsa.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/rsa_alt_helpers.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/sha1.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/sha256.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/sha512.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/sha3.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/builtin/CMakeFiles/builtin.dir/src/threading.c.obj
[ 47%] Linking CXX static library libmbed-builtin.a
[ 47%] Built target builtin
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/p256-m/CMakeFiles/p256m.dir/p256-m/p256-m.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/p256-m/CMakeFiles/p256m.dir/p256-m_driver_entrypoints.c.obj
[ 47%] Linking CXX static library libp256m.a
[ 47%] Built target p256m
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/everest/CMakeFiles/everest.dir/library/everest.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/everest/CMakeFiles/everest.dir/library/x25519.c.obj
[ 47%] Building C object esp-idf/mbedtls/mbedtls/tf-psa-crypto/drivers/everest/CMakeFiles/everest.dir/library/Hacl_Curve25519_joined.c.obj
[ 48%] Linking CXX static library libeverest.a
[ 48%] Built target everest
[ 48%] Building C object esp-idf/esp_driver_dma/CMakeFiles/__idf_esp_driver_dma.dir/src/esp_dma_utils.c.obj
[ 48%] Building C object esp-idf/esp_driver_dma/CMakeFiles/__idf_esp_driver_dma.dir/src/gdma_link.c.obj
[ 48%] Building C object esp-idf/esp_driver_dma/CMakeFiles/__idf_esp_driver_dma.dir/src/esp_async_memcpy.c.obj
[ 48%] Building C object esp-idf/esp_driver_dma/CMakeFiles/__idf_esp_driver_dma.dir/src/async_memcpy_gdma.c.obj
[ 48%] Building C object esp-idf/esp_driver_dma/CMakeFiles/__idf_esp_driver_dma.dir/src/gdma.c.obj
[ 48%] Linking C static library libesp_driver_dma.a
[ 48%] Built target __idf_esp_driver_dma
[ 48%] Building C object esp-idf/esp_mm/CMakeFiles/__idf_esp_mm.dir/heap_align_hw.c.obj
[ 48%] Building C object esp-idf/esp_mm/CMakeFiles/__idf_esp_mm.dir/esp_cache_msync.c.obj
[ 48%] Building C object esp-idf/esp_mm/CMakeFiles/__idf_esp_mm.dir/esp_mmu_map.c.obj
[ 48%] Building C object esp-idf/esp_mm/CMakeFiles/__idf_esp_mm.dir/port/esp32s3/ext_mem_layout.c.obj
[ 48%] Building C object esp-idf/esp_mm/CMakeFiles/__idf_esp_mm.dir/esp_cache_utils.c.obj
[ 48%] Linking C static library libesp_mm.a
[ 48%] Built target __idf_esp_mm
[ 49%] Building C object esp-idf/esp_pm/CMakeFiles/__idf_esp_pm.dir/pm_locks.c.obj
[ 49%] Building C object esp-idf/esp_pm/CMakeFiles/__idf_esp_pm.dir/pm_impl.c.obj
[ 49%] Building C object esp-idf/esp_pm/CMakeFiles/__idf_esp_pm.dir/pm_trace.c.obj
[ 49%] Linking C static library libesp_pm.a
[ 49%] Built target __idf_esp_pm
[ 49%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uart_hal.c.obj
[ 49%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uart_hal_iram.c.obj
[ 49%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/esp32s3/uart_periph.c.obj
[ 50%] Building C object esp-idf/esp_hal_uart/CMakeFiles/__idf_esp_hal_uart.dir/uhci_hal.c.obj
[ 50%] Linking C static library libesp_hal_uart.a
[ 50%] Built target __idf_esp_hal_uart
[ 50%] Building C object esp-idf/esp_driver_gpio/CMakeFiles/__idf_esp_driver_gpio.dir/src/gpio_pin_glitch_filter.c.obj
[ 50%] Building C object esp-idf/esp_driver_gpio/CMakeFiles/__idf_esp_driver_gpio.dir/src/rtc_io.c.obj
[ 50%] Building C object esp-idf/esp_driver_gpio/CMakeFiles/__idf_esp_driver_gpio.dir/src/gpio.c.obj
[ 50%] Building C object esp-idf/esp_driver_gpio/CMakeFiles/__idf_esp_driver_gpio.dir/src/dedic_gpio.c.obj
[ 50%] Building C object esp-idf/esp_driver_gpio/CMakeFiles/__idf_esp_driver_gpio.dir/src/gpio_glitch_filter_ops.c.obj
[ 50%] Linking C static library libesp_driver_gpio.a
[ 50%] Built target __idf_esp_driver_gpio
[ 50%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/init.c.obj
[ 50%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_ds.c.obj
[ 50%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_hmac.c.obj
[ 50%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_crypto_lock.c.obj
[ 50%] Building C object esp-idf/esp_security/CMakeFiles/__idf_esp_security.dir/src/esp_crypto_periph_clk.c.obj
[ 50%] Linking C static library libesp_security.a
[ 50%] Built target __idf_esp_security
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_rtc_calib.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_fields.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_table.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_utility.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_api.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_fields.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/esp32s3/esp_efuse_utility.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/efuse_controller/keys/with_key_purposes/esp_efuse_api_key.c.obj
[ 50%] Building C object esp-idf/efuse/CMakeFiles/__idf_efuse.dir/src/esp_efuse_startup.c.obj
[ 50%] Linking C static library libefuse.a
[ 50%] Built target __idf_efuse
[ 50%] Building C object esp-idf/esp_partition/CMakeFiles/__idf_esp_partition.dir/partition_target.c.obj
[ 50%] Building C object esp-idf/esp_partition/CMakeFiles/__idf_esp_partition.dir/partition.c.obj
[ 50%] Linking C static library libesp_partition.a
[ 50%] Built target __idf_esp_partition
[ 50%] Building C object esp-idf/app_update/CMakeFiles/__idf_app_update.dir/esp_ota_ops.c.obj
[ 50%] Linking C static library libapp_update.a
[ 50%] Built target __idf_app_update
[ 50%] Building C object esp-idf/esp_bootloader_format/CMakeFiles/__idf_esp_bootloader_format.dir/esp_bootloader_desc.c.obj
[ 50%] Linking C static library libesp_bootloader_format.a
[ 50%] Built target __idf_esp_bootloader_format
[ 50%] Building C object esp-idf/esp_app_format/CMakeFiles/__idf_esp_app_format.dir/esp_app_desc.c.obj
[ 50%] Linking C static library libesp_app_format.a
[ 50%] Built target __idf_esp_app_format
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/mpi_hal.c.obj
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/sha_hal.c.obj
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/aes_hal.c.obj
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/mpu_hal.c.obj
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/hmac_hal.c.obj
[ 50%] Building C object esp-idf/esp_hal_security/CMakeFiles/__idf_esp_hal_security.dir/ds_hal.c.obj
[ 51%] Linking C static library libesp_hal_security.a
[ 51%] Built target __idf_esp_hal_security
[ 51%] Building C object esp-idf/esp_hal_mspi/CMakeFiles/__idf_esp_hal_mspi.dir/spi_flash_hal.c.obj
[ 51%] Building C object esp-idf/esp_hal_mspi/CMakeFiles/__idf_esp_hal_mspi.dir/spi_flash_hal_iram.c.obj
[ 51%] Building C object esp-idf/esp_hal_mspi/CMakeFiles/__idf_esp_hal_mspi.dir/spi_flash_encrypt_hal_iram.c.obj
[ 51%] Building C object esp-idf/esp_hal_mspi/CMakeFiles/__idf_esp_hal_mspi.dir/spi_flash_hal_gpspi.c.obj
[ 52%] Linking C static library libesp_hal_mspi.a
[ 52%] Built target __idf_esp_hal_mspi
[ 52%] Building C object esp-idf/esp_hal_clock/CMakeFiles/__idf_esp_hal_clock.dir/esp32s3/clk_tree_hal.c.obj
[ 52%] Linking C static library libesp_hal_clock.a
[ 52%] Built target __idf_esp_hal_clock
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hal_iram.c.obj
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_hal.c.obj
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hd_hal.c.obj
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_hal_iram.c.obj
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/spi_slave_hal.c.obj
[ 52%] Building C object esp-idf/esp_hal_gpspi/CMakeFiles/__idf_esp_hal_gpspi.dir/esp32s3/spi_periph.c.obj
[ 52%] Linking C static library libesp_hal_gpspi.a
[ 52%] Built target __idf_esp_hal_gpspi
[ 52%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/esp32s3/gdma_periph.c.obj
[ 52%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/gdma_hal_top.c.obj
[ 52%] Building C object esp-idf/esp_hal_dma/CMakeFiles/__idf_esp_hal_dma.dir/gdma_hal_ahb_v1.c.obj
[ 52%] Linking C static library libesp_hal_dma.a
[ 52%] Built target __idf_esp_hal_dma
[ 52%] Building C object esp-idf/esp_stdio/CMakeFiles/__idf_esp_stdio.dir/stdio_simple.c.obj
[ 53%] Building C object esp-idf/esp_stdio/CMakeFiles/__idf_esp_stdio.dir/stdio_vfs.c.obj
[ 53%] Building C object esp-idf/esp_stdio/CMakeFiles/__idf_esp_stdio.dir/stdio_syscalls_simple.c.obj
[ 53%] Linking C static library libesp_stdio.a
[ 53%] Built target __idf_esp_stdio
[ 53%] Building C object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/eri.c.obj
[ 53%] Building ASM object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xtensa_intr_asm.S.obj
[ 53%] Building C object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xt_trax.c.obj
[ 53%] Building ASM object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xtensa_vectors.S.obj
[ 54%] Building ASM object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xtensa_context.S.obj
[ 54%] Building C object esp-idf/xtensa/CMakeFiles/__idf_xtensa.dir/xtensa_intr.c.obj
[ 54%] Linking C static library libxtensa.a
[ 54%] Built target __idf_xtensa
[ 54%] Building C object esp-idf/rt/CMakeFiles/__idf_rt.dir/FreeRTOS_POSIX_mqueue.c.obj
[ 54%] Building C object esp-idf/esp_hid/CMakeFiles/__idf_esp_hid.dir/src/esp_hidd.c.obj
[ 54%] Building C object esp-idf/esp_driver_spi/CMakeFiles/__idf_esp_driver_spi.dir/src/gpspi/spi_common.c.obj
[ 54%] Building C object esp-idf/protobuf-c/CMakeFiles/__idf_protobuf-c.dir/protobuf-c/protobuf-c/protobuf-c.c.obj
[ 54%] Building C object esp-idf/esp_hal_lcd/CMakeFiles/__idf_esp_hal_lcd.dir/lcd_hal.c.obj
[ 54%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/Partition.cpp.obj
[ 54%] Building C object esp-idf/esp_driver_i2c/CMakeFiles/__idf_esp_driver_i2c.dir/i2c_master.c.obj
[ 54%] Building C object esp-idf/perfmon/CMakeFiles/__idf_perfmon.dir/xtensa_perfmon_access.c.obj
[ 55%] Building C object esp-idf/esp_https_server/CMakeFiles/__idf_esp_https_server.dir/src/https_server.c.obj
[ 55%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/commands.c.obj
[ 55%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs_api.c.obj
[ 55%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_cmd.c.obj
[ 55%] Building C object esp-idf/perfmon/CMakeFiles/__idf_perfmon.dir/xtensa_perfmon_apis.c.obj
[ 55%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/SPI_Flash.cpp.obj
[ 55%] Building C object esp-idf/esp_hal_lcd/CMakeFiles/__idf_esp_hal_lcd.dir/esp32s3/lcd_periph.c.obj
[ 55%] Building C object esp-idf/esp_hid/CMakeFiles/__idf_esp_hid.dir/src/esp_hidh.c.obj
[ 55%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs/src/spiffs_cache.c.obj
[ 55%] Building C object esp-idf/rt/CMakeFiles/__idf_rt.dir/FreeRTOS_POSIX_utils.c.obj
[ 55%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/esp_console_common.c.obj
[ 55%] Linking C static library libesp_hal_lcd.a
[ 55%] Building C object esp-idf/perfmon/CMakeFiles/__idf_perfmon.dir/xtensa_perfmon_masks.c.obj
[ 55%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/WL_Ext_Perf.cpp.obj
[ 55%] Linking C static library libperfmon.a
[ 55%] Built target __idf_esp_hal_lcd
[ 55%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs/src/spiffs_check.c.obj
[ 55%] Linking C static library librt.a
[ 55%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/esp_console_repl_internal.c.obj
[ 55%] Linking C static library libesp_https_server.a
[ 55%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_disp.c.obj
[ 56%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_common.c.obj
[ 56%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/WL_Ext_Safe.cpp.obj
[ 56%] Building C object esp-idf/esp_driver_spi/CMakeFiles/__idf_esp_driver_spi.dir/src/gpspi/spi_master.c.obj
[ 56%] Built target __idf_perfmon
[ 56%] Built target __idf_rt
[ 56%] Building C object esp-idf/esp_hal_twai/CMakeFiles/__idf_esp_hal_twai.dir/esp32s3/twai_periph.c.obj
[ 56%] Building C object esp-idf/esp_hid/CMakeFiles/__idf_esp_hid.dir/src/esp_hid_common.c.obj
[ 56%] Built target __idf_esp_https_server
[ 56%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/split_argv.c.obj
[ 56%] Building C object esp-idf/esp_hal_ledc/CMakeFiles/__idf_esp_hal_ledc.dir/ledc_hal.c.obj
[ 56%] Building C object esp-idf/dial_state/CMakeFiles/__idf_dial_state.dir/dial_state.c.obj
[ 56%] Building C object esp-idf/esp_hal_twai/CMakeFiles/__idf_esp_hal_twai.dir/twai_hal_v1.c.obj
[ 56%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/WL_Flash.cpp.obj
[ 56%] Building C object esp-idf/esp_driver_i2c/CMakeFiles/__idf_esp_driver_i2c.dir/i2c_common.c.obj
[ 56%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/linenoise/linenoise.c.obj
[ 56%] Building C object esp-idf/esp_hal_ledc/CMakeFiles/__idf_esp_hal_ledc.dir/ledc_hal_iram.c.obj
[ 56%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_init.c.obj
[ 56%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_event.c.obj
[ 56%] Linking C static library libesp_hid.a
[ 56%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs/src/spiffs_gc.c.obj
[ 56%] Building C object esp-idf/esp_hal_ledc/CMakeFiles/__idf_esp_hal_ledc.dir/esp32s3/ledc_periph.c.obj
[ 56%] Built target __idf_esp_hid
[ 56%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_mmc.c.obj
[ 56%] Linking C static library libesp_hal_twai.a
[ 56%] Linking C static library libdial_state.a
[ 56%] Building C object esp-idf/esp_driver_i2c/CMakeFiles/__idf_esp_driver_i2c.dir/i2c_slave.c.obj
[ 57%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_group.c.obj
[ 57%] Linking C static library libesp_hal_ledc.a
[ 57%] Building C object esp-idf/espressif__cjson/CMakeFiles/__idf_espressif__cjson.dir/cJSON/cJSON.c.obj
[ 57%] Built target __idf_esp_hal_twai
[ 57%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/crc32.cpp.obj
[ 57%] Built target __idf_dial_state
[ 57%] Building C object esp-idf/esp_driver_spi/CMakeFiles/__idf_esp_driver_spi.dir/src/gpspi/spi_slave.c.obj
[ 57%] Generating ../../zones.csv.S
[ 57%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs/src/spiffs_hydrogen.c.obj
[ 57%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/esp_console_repl_chip.c.obj
[ 57%] Built target __idf_esp_hal_ledc
[ 57%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_sd.c.obj
[ 58%] Building CXX object esp-idf/wear_levelling/CMakeFiles/__idf_wear_levelling.dir/wear_levelling.cpp.obj
[ 58%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_cmd.c.obj
[ 58%] Building C object esp-idf/dial_knob/CMakeFiles/__idf_dial_knob.dir/bidi_switch_knob.c.obj
[ 58%] Building C object esp-idf/dial_time/CMakeFiles/__idf_dial_time.dir/dial_time.c.obj
[ 58%] Linking C static library libprotobuf-c.a
[ 58%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_indev.c.obj
[ 58%] Built target __idf_protobuf-c
[ 58%] Linking C static library libesp_driver_i2c.a
[ 59%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_date.c.obj
[ 59%] Building ASM object esp-idf/dial_time/CMakeFiles/__idf_dial_time.dir/__/__/zones.csv.S.obj
[ 59%] Building C object esp-idf/esp_driver_gptimer/CMakeFiles/__idf_esp_driver_gptimer.dir/src/gptimer.c.obj
[ 59%] Building C object esp-idf/esp_driver_spi/CMakeFiles/__idf_esp_driver_spi.dir/src/gpspi/spi_slave_hd.c.obj
[ 59%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_dbl.c.obj
[ 59%] Linking C static library libdial_knob.a
[ 59%] Linking C static library libwear_levelling.a
[ 59%] Built target __idf_esp_driver_i2c
[ 59%] Linking C static library libdial_time.a
[ 59%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity/src/unity.c.obj
[ 59%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/spiffs/src/spiffs_nucleus.c.obj
[ 59%] Built target __idf_dial_knob
[ 59%] Built target __idf_wear_levelling
[ 59%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_dstr.c.obj
[ 59%] Built target __idf_dial_time
[ 59%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_blockdev.c.obj
[ 60%] Building C object esp-idf/esp_hal_cam/CMakeFiles/__idf_esp_hal_cam.dir/cam_hal.c.obj
[ 60%] Building C object esp-idf/esp_hal_mcpwm/CMakeFiles/__idf_esp_hal_mcpwm.dir/mcpwm_hal.c.obj
[ 60%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_end.c.obj
[ 60%] Building C object esp-idf/esp_hal_pcnt/CMakeFiles/__idf_esp_hal_pcnt.dir/pcnt_hal.c.obj
[ 60%] Building C object esp-idf/esp_driver_gptimer/CMakeFiles/__idf_esp_driver_gptimer.dir/src/gptimer_common.c.obj
[ 60%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_indev_scroll.c.obj
[ 60%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sd_pwr_ctrl/sd_pwr_ctrl.c.obj
[ 60%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_file.c.obj
[ 60%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_hashtable.c.obj
[ 60%] Building C object esp-idf/esp_hal_mcpwm/CMakeFiles/__idf_esp_hal_mcpwm.dir/esp32s3/mcpwm_periph.c.obj
[ 60%] Building C object esp-idf/esp_hal_cam/CMakeFiles/__idf_esp_hal_cam.dir/esp32s3/cam_periph.c.obj
[ 60%] Building C object esp-idf/esp_hal_pcnt/CMakeFiles/__idf_esp_hal_pcnt.dir/esp32s3/pcnt_periph.c.obj
[ 60%] Linking C static library libesp_driver_spi.a
[ 60%] Building C object esp-idf/sdmmc/CMakeFiles/__idf_sdmmc.dir/sdmmc_io.c.obj
[ 60%] Linking C static library libesp_hal_mcpwm.a
[ 60%] Linking C static library libesp_hal_cam.a
[ 60%] Linking C static library libesp_hal_pcnt.a
[ 60%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_int.c.obj
[ 61%] Linking C static library libesp_driver_gptimer.a
[ 61%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_compat.c.obj
[ 61%] Built target __idf_esp_driver_spi
[ 61%] Built target __idf_esp_hal_pcnt
[ 61%] Built target __idf_esp_hal_mcpwm
[ 62%] Building C object esp-idf/espressif__cjson/CMakeFiles/__idf_espressif__cjson.dir/cJSON/cJSON_Utils.c.obj
[ 62%] Building C object esp-idf/esp_hal_rmt/CMakeFiles/__idf_esp_hal_rmt.dir/rmt_hal.c.obj
[ 62%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj.c.obj
[ 62%] Built target __idf_esp_driver_gptimer
[ 62%] Built target __idf_esp_hal_cam
[ 62%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_class.c.obj
[ 62%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_lit.c.obj
[ 62%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_runner.c.obj
[ 62%] Building C object esp-idf/esp_driver_sdm/CMakeFiles/__idf_esp_driver_sdm.dir/src/sdm.c.obj
[ 62%] Building C object esp-idf/esp_driver_touch_sens/CMakeFiles/__idf_esp_driver_touch_sens.dir/common/touch_sens_common.c.obj
[ 62%] Building C object esp-idf/esp_driver_tsens/CMakeFiles/__idf_esp_driver_tsens.dir/src/temperature_sensor.c.obj
[ 62%] Building C object esp-idf/esp_hal_rmt/CMakeFiles/__idf_esp_hal_rmt.dir/esp32s3/rmt_periph.c.obj
[ 62%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_rem.c.obj
[ 62%] Linking C static library libsdmmc.a
[ 62%] Building C object esp-idf/esp_driver_twai/CMakeFiles/__idf_esp_driver_twai.dir/esp_twai.c.obj
[ 62%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_utils_freertos.c.obj
[ 62%] Linking C static library libesp_hal_rmt.a
[ 62%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_utils_cache.c.obj
[ 62%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_rex.c.obj
[ 62%] Built target __idf_sdmmc
[ 62%] Linking C static library libesp_driver_sdm.a
[ 62%] Built target __idf_esp_hal_rmt
[ 63%] Building C object esp-idf/spiffs/CMakeFiles/__idf_spiffs.dir/esp_spiffs.c.obj
[ 63%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_utils_memory.c.obj
[ 63%] Linking C static library libespressif__cjson.a
[ 63%] Building C object esp-idf/esp_driver_touch_sens/CMakeFiles/__idf_esp_driver_touch_sens.dir/hw_ver2/touch_version_specific.c.obj
[ 63%] Building C object esp-idf/esp_eth/CMakeFiles/__idf_esp_eth.dir/src/esp_eth.c.obj
[ 63%] Linking C static library libesp_driver_tsens.a
[ 63%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_draw.c.obj
[ 63%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/unity_port_esp32.c.obj
[ 63%] Built target __idf_esp_driver_sdm
[ 63%] Building C object esp-idf/esp_driver_twai/CMakeFiles/__idf_esp_driver_twai.dir/esp_twai_onchip.c.obj
[ 63%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_str.c.obj
[ 63%] Built target __idf_espressif__cjson
[ 63%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/src/esp_lcd_common.c.obj
[ 63%] Built target __idf_esp_driver_tsens
[ 64%] Building C object esp-idf/unity/CMakeFiles/__idf_unity.dir/port/esp/unity_utils_memory_esp.c.obj
[ 64%] Building C object esp-idf/esp_eth/CMakeFiles/__idf_esp_eth.dir/src/phy/esp_eth_phy_802_3.c.obj
[ 64%] Building C object esp-idf/esp_driver_sd_intf/CMakeFiles/__idf_esp_driver_sd_intf.dir/sd_host.c.obj
[ 64%] Building C object esp-idf/esp_driver_sdspi/CMakeFiles/__idf_esp_driver_sdspi.dir/src/sdspi_crc.c.obj
[ 64%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/arg_utils.c.obj
[ 64%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_pos.c.obj
[ 64%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/src/esp_lcd_panel_io.c.obj
[ 64%] Building C object esp-idf/console/CMakeFiles/__idf_console.dir/argtable3/argtable3.c.obj
[ 64%] Building C object esp-idf/esp_driver_sdspi/CMakeFiles/__idf_esp_driver_sdspi.dir/src/sdspi_host.c.obj
[ 64%] Linking C static library libunity.a
[ 64%] Linking C static library libesp_driver_sd_intf.a
[ 64%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/src/esp_lcd_panel_ssd1306.c.obj
[ 64%] Linking C static library libesp_driver_touch_sens.a
[ 65%] Building C object esp-idf/driver/CMakeFiles/__idf_driver.dir/i2c/i2c.c.obj
[ 65%] Built target __idf_unity
[ 65%] Built target __idf_esp_driver_sd_intf
[ 65%] Building C object esp-idf/esp_eth/CMakeFiles/__idf_esp_eth.dir/src/esp_eth_netif_glue.c.obj
[ 65%] Generating ../../trust_roots.pem.S
[ 65%] Building C object esp-idf/esp_driver_ledc/CMakeFiles/__idf_esp_driver_ledc.dir/src/ledc.c.obj
[ 65%] Linking C static library libspiffs.a
[ 65%] Built target __idf_esp_driver_touch_sens
[ 65%] Linking C static library libesp_driver_twai.a
[ 65%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_scroll.c.obj
[ 65%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/src/esp_lcd_panel_st7789.c.obj
[ 65%] Building C object esp-idf/esp_driver_sdspi/CMakeFiles/__idf_esp_driver_sdspi.dir/src/sdspi_transaction.c.obj
[ 65%] Building C object esp-idf/driver/CMakeFiles/__idf_driver.dir/touch_sensor/touch_sensor_common.c.obj
[ 65%] Building C object esp-idf/dial_ota/CMakeFiles/__idf_dial_ota.dir/dial_ota.c.obj
[ 65%] Linking C static library libesp_eth.a
[ 65%] Built target __idf_esp_driver_twai
[ 65%] Built target __idf_spiffs
[ 65%] Linking C static library libconsole.a
[ 65%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_style.c.obj
[ 65%] Building C object esp-idf/dial_net/CMakeFiles/__idf_dial_net.dir/dial_wifi.c.obj
[ 65%] Built target __idf_esp_eth
[ 66%] Linking C static library libesp_driver_sdspi.a
[ 66%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_style_gen.c.obj
[ 66%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/src/esp_lcd_panel_ops.c.obj
[ 66%] Building C object esp-idf/driver/CMakeFiles/__idf_driver.dir/touch_sensor/esp32s3/touch_sensor.c.obj
[ 66%] Built target __idf_console
[ 66%] Building C object esp-idf/dial_somnus/CMakeFiles/__idf_dial_somnus.dir/dial_somnus.c.obj
[ 66%] Built target __idf_esp_driver_sdspi
[ 66%] Building C object esp-idf/cmock/CMakeFiles/__idf_cmock.dir/CMock/src/cmock.c.obj
[ 66%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_obj_tree.c.obj
[ 66%] Building C object esp-idf/esp_driver_cam/CMakeFiles/__idf_esp_driver_cam.dir/esp_cam_ctlr.c.obj
[ 66%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_refr.c.obj
[ 66%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/i2c/esp_lcd_panel_io_i2c.c.obj
[ 66%] Building ASM object esp-idf/dial_ota/CMakeFiles/__idf_dial_ota.dir/__/__/trust_roots.pem.S.obj
[ 66%] Linking C static library libdial_somnus.a
[ 66%] Linking C static library libcmock.a
[ 66%] Building C object esp-idf/driver/CMakeFiles/__idf_driver.dir/twai/twai.c.obj
[ 66%] Linking C static library libdial_ota.a
[ 66%] Linking C static library libdial_net.a
[ 66%] Built target __idf_cmock
[ 66%] Built target __idf_dial_somnus
[ 66%] Building C object esp-idf/esp_driver_cam/CMakeFiles/__idf_esp_driver_cam.dir/dvp_share_ctrl.c.obj
[ 66%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/spi/esp_lcd_panel_io_spi.c.obj
[ 66%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/core/lv_theme.c.obj
[ 66%] Linking C static library libesp_driver_ledc.a
[ 66%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/arm2d/lv_gpu_arm2d.c.obj
[ 66%] Built target __idf_dial_ota
[ 66%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_cap.c.obj
[ 66%] Built target __idf_dial_net
[ 66%] Building C object esp-idf/esp_driver_pcnt/CMakeFiles/__idf_esp_driver_pcnt.dir/src/pulse_cnt.c.obj
[ 66%] Building C object esp-idf/esp_driver_cam/CMakeFiles/__idf_esp_driver_cam.dir/dvp/src/esp_cam_ctlr_dvp_gdma.c.obj
[ 66%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_common.c.obj
[ 66%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_cmpr.c.obj
[ 66%] Built target __idf_esp_driver_ledc
[ 66%] Building C object esp-idf/esp_driver_cam/CMakeFiles/__idf_esp_driver_cam.dir/dvp/src/esp_cam_ctlr_dvp_cam.c.obj
[ 67%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw.c.obj
[ 67%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/common/protocomm.c.obj
[ 67%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_arc.c.obj
[ 67%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_img.c.obj
[ 67%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_label.c.obj
[ 67%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/i80/esp_lcd_panel_io_i80.c.obj
[ 67%] Linking C static library libdriver.a
[ 68%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_com.c.obj
[ 68%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_layer.c.obj
[ 68%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_fault.c.obj
[ 68%] Built target __idf_driver
[ 68%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_line.c.obj
[ 68%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/proto-c/constants.pb-c.c.obj
[ 68%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_encoder.c.obj
[ 69%] Building C object esp-idf/esp_lcd/CMakeFiles/__idf_esp_lcd.dir/rgb/esp_lcd_panel_rgb.c.obj
[ 69%] Building C object esp-idf/esp_driver_sdmmc/CMakeFiles/__idf_esp_driver_sdmmc.dir/legacy/src/sdmmc_transaction.c.obj
[ 69%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_mask.c.obj
[ 69%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_rect.c.obj
[ 69%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/proto-c/sec0.pb-c.c.obj
[ 69%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_transform.c.obj
[ 69%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_gen.c.obj
[ 70%] Linking C static library libesp_driver_cam.a
[ 70%] Linking C static library libesp_driver_pcnt.a
[ 70%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/proto-c/sec1.pb-c.c.obj
[ 70%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_draw_triangle.c.obj
[ 70%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_encoder_bytes.c.obj
[ 70%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_img_buf.c.obj
[ 70%] Building C object esp-idf/esp_driver_sdmmc/CMakeFiles/__idf_esp_driver_sdmmc.dir/legacy/src/sdmmc_host.c.obj
[ 70%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_oper.c.obj
[ 70%] Built target __idf_esp_driver_cam
[ 70%] Built target __idf_esp_driver_pcnt
[ 70%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_img_cache.c.obj
[ 70%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/proto-c/sec2.pb-c.c.obj
[ 70%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/proto-c/session.pb-c.c.obj
[ 71%] Building C object esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj
[ 71%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_encoder_copy.c.obj
[ 71%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/lv_img_decoder.c.obj
[ 71%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_encoder_simple.c.obj
[ 71%] Building C object esp-idf/esp_driver_sdmmc/CMakeFiles/__idf_esp_driver_sdmmc.dir/src/sd_host_sdmmc.c.obj
[ 71%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/transports/protocomm_console.c.obj
[ 71%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/pxp/lv_draw_pxp.c.obj
[ 72%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/transports/protocomm_httpd.c.obj
[ 72%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/pxp/lv_draw_pxp_blend.c.obj
[ 72%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_sync.c.obj
[ 72%] Building C object esp-idf/esp_driver_mcpwm/CMakeFiles/__idf_esp_driver_mcpwm.dir/src/mcpwm_timer.c.obj
[ 73%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/pxp/lv_gpu_nxp_pxp.c.obj
[ 73%] Linking C static library libdial_pad_discovery.a
[ 74%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_rx.c.obj
[ 74%] Building C object esp-idf/esp_driver_rmt/CMakeFiles/__idf_esp_driver_rmt.dir/src/rmt_tx.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/pxp/lv_gpu_nxp_pxp_osa.c.obj
[ 74%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/security/security2.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_draw_vglite.c.obj
[ 74%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/crypto/srp6a/esp_srp.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_draw_vglite_arc.c.obj
[ 74%] Built target __idf_dial_pad_discovery
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_draw_vglite_line.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_draw_vglite_blend.c.obj
[ 74%] Building C object esp-idf/esp_driver_sdmmc/CMakeFiles/__idf_esp_driver_sdmmc.dir/src/sd_trans_sdmmc.c.obj
[ 74%] Building C object esp-idf/protocomm/CMakeFiles/__idf_protocomm.dir/src/crypto/srp6a/esp_srp_mpi.c.obj
[ 74%] Linking C static library libesp_lcd.a
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_draw_vglite_rect.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_vglite_buf.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/nxp/vglite/lv_vglite_utils.c.obj
[ 74%] Linking C static library libesp_driver_mcpwm.a
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/renesas/lv_gpu_d2_draw_label.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/renesas/lv_gpu_d2_ra6m3.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl.c.obj
[ 74%] Built target __idf_esp_lcd
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_arc.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_composite.c.obj
[ 74%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_bg.c.obj
[ 74%] Building C object esp-idf/espressif__esp_lcd_sh8601/CMakeFiles/__idf_espressif__esp_lcd_sh8601.dir/esp_lcd_sh8601.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_img.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_label.c.obj
[ 75%] Linking C static library libesp_driver_sdmmc.a
[ 75%] Built target __idf_esp_driver_mcpwm
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_layer.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_line.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_mask.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_polygon.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_rect.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_stack_blur.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_texture_cache.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sdl/lv_draw_sdl_utils.c.obj
[ 75%] Linking C static library libprotocomm.a
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.c.obj
[ 75%] Built target __idf_esp_driver_sdmmc
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_arc.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_blend.c.obj
[ 75%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_dither.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_gradient.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_img.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_layer.c.obj
[ 76%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/diskio/diskio.c.obj
[ 76%] Built target __idf_protocomm
[ 76%] Linking C static library libespressif__esp_lcd_sh8601.a
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_letter.c.obj
[ 76%] Building C object esp-idf/esp_local_ctrl/CMakeFiles/__idf_esp_local_ctrl.dir/src/esp_local_ctrl.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_line.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_polygon.c.obj
[ 76%] Linking C static library libesp_driver_rmt.a
[ 76%] Built target __idf_espressif__esp_lcd_sh8601
[ 76%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/diskio/diskio_rawflash.c.obj
[ 76%] Building C object esp-idf/esp_local_ctrl/CMakeFiles/__idf_esp_local_ctrl.dir/src/esp_local_ctrl_handler.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_transform.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/sw/lv_draw_sw_rect.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/draw/swm341_dma2d/lv_gpu_swm341_dma2d.c.obj
[ 76%] Built target __idf_esp_driver_rmt
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/layouts/flex/lv_flex.c.obj
[ 76%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/diskio/diskio_wl.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/layouts/grid/lv_grid.c.obj
[ 76%] Building C object esp-idf/esp_local_ctrl/CMakeFiles/__idf_esp_local_ctrl.dir/proto-c/esp_local_ctrl.pb-c.c.obj
[ 76%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/src/ff.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/ffmpeg/lv_ffmpeg.c.obj
[ 76%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/bmp/lv_bmp.c.obj
[ 77%] Building C object esp-idf/esp_local_ctrl/CMakeFiles/__idf_esp_local_ctrl.dir/src/esp_local_ctrl_transport_httpd.c.obj
[ 77%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/src/ffunicode.c.obj
[ 77%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/port/freertos/ffsystem.c.obj
[ 77%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/diskio/diskio_sdmmc.c.obj
[ 77%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/freetype/lv_freetype.c.obj
[ 77%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/fsdrv/lv_fs_fatfs.c.obj
[ 77%] Linking C static library libesp_local_ctrl.a
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/fsdrv/lv_fs_littlefs.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/fsdrv/lv_fs_posix.c.obj
[ 78%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/vfs/vfs_fat.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/fsdrv/lv_fs_stdio.c.obj
[ 78%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/vfs/vfs_fat_sdmmc.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/fsdrv/lv_fs_win32.c.obj
[ 78%] Building C object esp-idf/fatfs/CMakeFiles/__idf_fatfs.dir/vfs/vfs_fat_spiflash.c.obj
[ 78%] Built target __idf_esp_local_ctrl
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/gif/gifdec.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/gif/lv_gif.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/png/lodepng.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/png/lv_png.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/qrcode/lv_qrcode.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/qrcode/qrcodegen.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/rlottie/lv_rlottie.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/sjpg/lv_sjpg.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/sjpg/tjpgd.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/libs/tiny_ttf/lv_tiny_ttf.c.obj
[ 78%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/lv_extra.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/fragment/lv_fragment.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/fragment/lv_fragment_manager.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/ime/lv_ime_pinyin.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/gridnav/lv_gridnav.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/imgfont/lv_imgfont.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/monkey/lv_monkey.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/msg/lv_msg.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/others/snapshot/lv_snapshot.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/themes/basic/lv_theme_basic.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/themes/mono/lv_theme_mono.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/calendar/lv_calendar.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/themes/default/lv_theme_default.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/animimg/lv_animimg.c.obj
[ 79%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/calendar/lv_calendar_header_arrow.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/calendar/lv_calendar_header_dropdown.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/chart/lv_chart.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/colorwheel/lv_colorwheel.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/imgbtn/lv_imgbtn.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/keyboard/lv_keyboard.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/led/lv_led.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/list/lv_list.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/menu/lv_menu.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/meter/lv_meter.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/msgbox/lv_msgbox.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/span/lv_span.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/spinner/lv_spinner.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/spinbox/lv_spinbox.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/tabview/lv_tabview.c.obj
[ 80%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/tileview/lv_tileview.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/extra/widgets/win/lv_win.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_dejavu_16_persian_hebrew.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_fmt_txt.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_loader.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_10.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_12.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_12_subpx.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_14.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_16.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_18.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_20.c.obj
[ 81%] Linking C static library libfatfs.a
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_22.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_26.c.obj
[ 81%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_24.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_28.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_28_compressed.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_30.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_32.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_34.c.obj
[ 82%] Built target __idf_fatfs
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_36.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_38.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_40.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_44.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_42.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_46.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_48.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_simsun_16_cjk.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_montserrat_8.c.obj
[ 82%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_unscii_16.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/font/lv_font_unscii_8.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/hal/lv_hal_indev.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/hal/lv_hal_disp.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/hal/lv_hal_tick.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_anim_timeline.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_anim.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_area.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_async.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_bidi.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_color.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_fs.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_gc.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_log.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_lru.c.obj
[ 83%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_ll.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_math.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_mem.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_style.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_printf.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_templ.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_style_gen.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_timer.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_tlsf.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_txt.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_txt_ap.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/misc/lv_utils.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_arc.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_btn.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_bar.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_btnmatrix.c.obj
[ 84%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_checkbox.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_canvas.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_dropdown.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_img.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_label.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_line.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_objx_templ.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_roller.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_slider.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_table.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_switch.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/src/widgets/lv_textarea.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/anim/lv_example_anim_1.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/anim/lv_example_anim_2.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/anim/lv_example_anim_3.c.obj
[ 85%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/animimg001.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/anim/lv_example_anim_timeline_1.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/animimg002.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/animimg003.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/emoji/img_emoji_F617.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_caret_down.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_cogwheel_alpha16.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_cogwheel_argb.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_cogwheel_chroma_keyed.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_cogwheel_rgb.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_cogwheel_indexed16.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_skew_strip.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_hand.c.obj
[ 86%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/img_star.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/imgbtn_left.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/imgbtn_mid.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/assets/imgbtn_right.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/event/lv_example_event_1.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/event/lv_example_event_2.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/event/lv_example_event_3.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/event/lv_example_event_4.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/get_started/lv_example_get_started_1.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/get_started/lv_example_get_started_2.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/get_started/lv_example_get_started_3.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_1.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_2.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_3.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_4.c.obj
[ 87%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_5.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/flex/lv_example_flex_6.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_1.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_2.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_3.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_5.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/ffmpeg/lv_example_ffmpeg_1.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/bmp/lv_example_bmp_1.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_4.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/layouts/grid/lv_example_grid_6.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/ffmpeg/lv_example_ffmpeg_2.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/freetype/lv_example_freetype_1.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/gif/img_bulb_gif.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/gif/lv_example_gif_1.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/png/img_wink_png.c.obj
[ 88%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/png/lv_example_png_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/qrcode/lv_example_qrcode_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/rlottie/lv_example_rlottie_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/rlottie/lv_example_rlottie_2.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/sjpg/lv_example_sjpg_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/libs/rlottie/lv_example_rlottie_approve.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/gridnav/lv_example_gridnav_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/fragment/lv_example_fragment_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/fragment/lv_example_fragment_2.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/gridnav/lv_example_gridnav_2.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/gridnav/lv_example_gridnav_3.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/gridnav/lv_example_gridnav_4.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/ime/lv_example_ime_pinyin_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/ime/lv_example_ime_pinyin_2.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/monkey/lv_example_monkey_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/imgfont/lv_example_imgfont_1.c.obj
[ 89%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/monkey/lv_example_monkey_3.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/monkey/lv_example_monkey_2.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/msg/lv_example_msg_2.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/msg/lv_example_msg_1.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/msg/lv_example_msg_3.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/others/snapshot/lv_example_snapshot_1.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/porting/lv_port_disp_template.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/porting/lv_port_fs_template.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/porting/lv_port_indev_template.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_1.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_2.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_3.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_4.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_5.c.obj
[ 90%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/scroll/lv_example_scroll_6.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_1.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_10.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_11.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_12.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_13.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_14.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_15.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_2.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_3.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_4.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_7.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_6.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_5.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_8.c.obj
[ 91%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/styles/lv_example_style_9.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/animimg/lv_example_animimg_1.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/arc/lv_example_arc_2.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/arc/lv_example_arc_1.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_1.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_2.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_4.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_3.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_5.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/bar/lv_example_bar_6.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btn/lv_example_btn_2.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btn/lv_example_btn_1.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btn/lv_example_btn_3.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btnmatrix/lv_example_btnmatrix_1.c.obj
[ 92%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btnmatrix/lv_example_btnmatrix_2.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/btnmatrix/lv_example_btnmatrix_3.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/calendar/lv_example_calendar_1.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/canvas/lv_example_canvas_1.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/canvas/lv_example_canvas_2.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_1.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_2.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_3.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_5.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_4.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_6.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_7.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_8.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/chart/lv_example_chart_9.c.obj
[ 93%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/checkbox/lv_example_checkbox_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/checkbox/lv_example_checkbox_2.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/colorwheel/lv_example_colorwheel_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/dropdown/lv_example_dropdown_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/dropdown/lv_example_dropdown_2.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/dropdown/lv_example_dropdown_3.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/img/lv_example_img_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/img/lv_example_img_2.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/img/lv_example_img_3.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/img/lv_example_img_4.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/imgbtn/lv_example_imgbtn_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/keyboard/lv_example_keyboard_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/label/lv_example_label_1.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/label/lv_example_label_2.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/label/lv_example_label_3.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/label/lv_example_label_4.c.obj
[ 94%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/label/lv_example_label_5.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/led/lv_example_led_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/line/lv_example_line_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/list/lv_example_list_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/list/lv_example_list_2.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/menu/lv_example_menu_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/menu/lv_example_menu_2.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/menu/lv_example_menu_3.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/menu/lv_example_menu_4.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/menu/lv_example_menu_5.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/meter/lv_example_meter_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/meter/lv_example_meter_2.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/meter/lv_example_meter_3.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/meter/lv_example_meter_4.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/msgbox/lv_example_msgbox_1.c.obj
[ 95%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/obj/lv_example_obj_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/obj/lv_example_obj_2.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/roller/lv_example_roller_2.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/roller/lv_example_roller_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/roller/lv_example_roller_3.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/slider/lv_example_slider_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/slider/lv_example_slider_2.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/slider/lv_example_slider_3.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/span/lv_example_span_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/spinbox/lv_example_spinbox_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/spinner/lv_example_spinner_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/switch/lv_example_switch_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/table/lv_example_table_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/table/lv_example_table_2.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/tabview/lv_example_tabview_1.c.obj
[ 96%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/tabview/lv_example_tabview_2.c.obj
[ 97%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/textarea/lv_example_textarea_1.c.obj
[ 97%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/textarea/lv_example_textarea_2.c.obj
[ 97%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/textarea/lv_example_textarea_3.c.obj
[ 97%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/tileview/lv_example_tileview_1.c.obj
[ 97%] Building C object esp-idf/lvgl__lvgl/CMakeFiles/__idf_lvgl__lvgl.dir/examples/widgets/win/lv_example_win_1.c.obj
[ 97%] Linking C static library liblvgl__lvgl.a
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Building C object esp-idf/dial_display/CMakeFiles/__idf_dial_display.dir/dial_display.c.obj
[ 97%] Linking C static library libdial_display.a
[ 97%] Built target __idf_dial_display
[ 97%] Building C object esp-idf/lcd_bl_pwm_bsp/CMakeFiles/__idf_lcd_bl_pwm_bsp.dir/lcd_bl_pwm_bsp.c.obj
[ 97%] Linking C static library liblcd_bl_pwm_bsp.a
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Building C object esp-idf/lcd_touch_bsp/CMakeFiles/__idf_lcd_touch_bsp.dir/lcd_touch_bsp.c.obj
[ 97%] Linking C static library liblcd_touch_bsp.a
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Building C object esp-idf/i2c_bsp/CMakeFiles/__idf_i2c_bsp.dir/i2c_bsp.c.obj
[ 97%] Linking C static library libi2c_bsp.a
[ 97%] Built target __idf_i2c_bsp
[ 97%] Building C object esp-idf/dial_haptics/CMakeFiles/__idf_dial_haptics.dir/dial_haptics.c.obj
[ 97%] Linking C static library libdial_haptics.a
[ 97%] Built target __idf_dial_haptics
[ 97%] Building C object esp-idf/dial_power/CMakeFiles/__idf_dial_power.dir/dial_power.c.obj
[ 97%] Linking C static library libdial_power.a
[ 97%] Built target __idf_dial_power
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_screens.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_dial.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_discovery.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_passkey.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_router.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_about.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_setup.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_menu.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_wifi.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_netpick.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_connecting.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_updating.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update_prompt.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_welcome.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_address.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_settings.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_adjust_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness_menu.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_face.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby_face.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_timezone.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_list.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_palette.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_num_88.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_16.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_num_140.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/dial_font_icons_20.c.obj
[100%] Linking C static library libdial_ui.a
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
[100%] Building C object CMakeFiles/somnus-dial.elf.dir/project_elf_src_esp32s3.c.obj
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
somnus-dial.bin binary size 0x1896b0 bytes. Smallest app partition is 0x400000 bytes. 0x276950 bytes (62%) free.
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

## Binary string checks

`firmware/dial-idf/build/somnus-dial.bin`, 1,611,440 bytes, built 2026-09-30 11:46.

- `strings build/somnus-dial.bin | grep -c '1\.0\.3-beta\.1'` → `1` (the match is `1.0.3-beta.1`). **PASS.**
- `grep -ac 'Which side' build/somnus-dial.bin` → `0`. **PASS**: the side picker's title is gone.

## Deviations

1. **Build warning count.** The task requires "0 errors and 0 warnings". The result is 0 errors and 0 compiler warnings, plus the CMake deprecation notice described above. It is old, it comes from configure and not from the compiler, and it is on a line this task did not change. I counted it as not failing the gate and made Commit B, the same way REPORT-sidepick-code.md handled fe5b139. If you count it as a failure, undo with `git reset --soft HEAD~1`. That leaves the two files staged and Commit A in place.
2. **The note covers T2 as well as T1.** The task names the T1 row. The T2 row in the §7 table expects the same `sb_face: no key -> default` line, which cannot appear for the same reason. So the note says "The T1 and T2 rows". The table is not changed. The prose paragraph under the table that says the `sb_face` line proves an empty namespace ("that restore_prefs saw an empty namespace (the `sb_face` line at `dial_state.c:245-246`)") is also not changed. The new note corrects it.
3. **This report is not in REPORTS.md.** Adding a line would have left `docs/REPORTS.md` modified after the commits. The task names no such edit. `docs/REPORT-sidepick-bench-flash.md` (committed in A) also has no line of its own in REPORTS.md, because the REPORTS.md change handed to me only adds the `REPORT-sidepick-bench.md` line. I left both as they were.
4. The awk extractor was run in a local `bash -c` with the workflow's exact awk program and trim seds. The real workflow step was not run on a runner.

No other deviations. Only the PROJECT_VER line changed in firmware sources. Nothing tagged or pushed. Working tree after Commit B: clean except for this report, which is untracked.

## What can't be verified without hardware

- **T3 (OTA upgrade from 1.0.2).** This can run only after `somnus-v1.0.3-beta.1` is tagged, pushed to `somnus` and published by the release workflow. The steps: put Bedknob #1 back on 1.0.2 with settings kept (wire-flash a `somnus-v1.0.2` build with `idf.py flash`, never the browser flasher), confirm About shows 1.0.2, note the side and the Scale, turn Beta builds On, then Check for updates. Expected: no side picker, and the same side and Scale as before.
- The bench PASS in REPORT-sidepick-bench.md was run on an image that reported 1.0.2 (HEAD 042840b code). The only firmware change since is the PROJECT_VER string. The 1.0.3-beta.1-labelled binary itself has not been flashed.
- Whether the GitHub release workflow passes (tag/PROJECT_VER match, notes extraction on the runner, CI build) is only known after the tag is pushed.
