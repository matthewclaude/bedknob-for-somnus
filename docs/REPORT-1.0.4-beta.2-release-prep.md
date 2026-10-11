# REPORT: somnus-v1.0.4-beta.2 release prep

**Verdict: PASS.** Two commits on main, not tagged or pushed. The release commit `1598b27` has PROJECT_VER "1.0.4-beta.2" and the matching CHANGELOG section. The screens commit `94241fc` has the version-bearing screens at 1.0.4-beta.2. "510/510" and "254/510" fit on the pad discovery screen, with about 50 px of clearance to the ring.

Date: 2026-10-04. Starting HEAD: `4e46454` (the discovery change). No new behaviour.

## Gate check

| Gate | Result |
|---|---|
| `idf.py build` in `firmware/dial-idf` | **PASS**, exit 0. Image `somnus-dial.bin` is 0x189a40 bytes, 62% of the app partition free. The only warning is the known `cmake_minimum_required` deprecation, printed twice. |
| Existing test `firmware/dial-idf/test/test_dial_rel.c` | **PASS**: `all relative-scale table assertions passed`, exit 0 |
| Step 4: "510/510" and "254/510" fit the discovery screen | **PASS**. No clipping and no overlap; measurements below. |
| Extra (not in the gate): release.yml's tag == PROJECT_VER check and CHANGELOG extraction, run locally against HEAD | **PASS**: the tag version matches, and the section extracts to the three-line body |

## PROJECT_VER and CHANGELOG as written

`firmware/dial-idf/CMakeLists.txt` line 21:

```
set(PROJECT_VER "1.0.4-beta.2")
```

`CHANGELOG.md` lines 25–30, above `## 1.0.4-beta.1`:

```
## 1.0.4-beta.2 — 2026-10-04 (beta)

The dial can now find your pad by itself on routers that use larger
networks, such as eero mesh systems. Before, on those networks the dial
gave up looking and you had to type the pad's address in by hand.

```

## Commits and diff stat

| Commit | Contents |
|---|---|
| `1598b27` `release: somnus-v1.0.4-beta.2 — pad discovery on networks wider than /24` | PROJECT_VER and CHANGELOG, in one commit (parent `4e46454`) |
| `94241fc` `docs: five version-bearing screens at 1.0.4-beta.2` | the regenerated screens (parent `1598b27`) |

```
$ git diff --stat 4e46454 1598b27
 CHANGELOG.md                     | 6 ++++++
 firmware/dial-idf/CMakeLists.txt | 2 +-
 2 files changed, 7 insertions(+), 1 deletion(-)

$ git diff --stat 1598b27 94241fc
 docs/screens/about-wifi-real.png  | Bin 23571 -> 23661 bytes
 docs/screens/about-wifi-worst.png | Bin 24544 -> 24628 bytes
 docs/screens/about.png            | Bin 19348 -> 19608 bytes
 docs/screens/update-failed.png    | Bin 22343 -> 22488 bytes
 docs/screens/update.png           | Bin 23863 -> 24028 bytes
 5 files changed, 0 insertions(+), 0 deletions(-)

$ git diff --stat 4e46454 94241fc
 CHANGELOG.md                      |   6 ++++++
 docs/screens/about-wifi-real.png  | Bin 23571 -> 23661 bytes
 docs/screens/about-wifi-worst.png | Bin 24544 -> 24628 bytes
 docs/screens/about.png            | Bin 19348 -> 19608 bytes
 docs/screens/update-failed.png    | Bin 22343 -> 22488 bytes
 docs/screens/update.png           | Bin 23863 -> 24028 bytes
 firmware/dial-idf/CMakeLists.txt  |   2 +-
 7 files changed, 7 insertions(+), 1 deletion(-)
```

Each commit was staged path by path, and `git diff --cached --stat` showed exactly the list above before each commit.

## Step 3: version-bearing screens

The simulator reads PROJECT_VER at configure time (`dial_sim: firmware version 1.0.4-beta.2, OTA scenarios advertise 1.0.5`). After rendering, I compared every PNG with a pre-run backup:

- **Changed (5):** about, about-wifi-real, about-wifi-worst, update, update-failed.
- **Unchanged (43),** including:
  - `update-prompt.png`, which shows only the offered version "1.0.5" (deviation 1);
  - the screens that carry no version text: standby-update, updating, about-battery-*.

No non-version screen changed. I checked `about.png` by eye: "v1.0.4-beta.2" is on one line, the same length as "v1.0.4-beta.1".

## Step 4: discovery progress screen at 510

**How the renders were made.** The simulator has one discovery scenario, `scenario_pad_discovery` in `simulator/main.c`, with the fixed text `"Still looking (checking more slowly)...\n137/254"`. For each variant I swapped that string in a temporary copy of `main.c`, forced a rebuild, ran `dial_sim` and copied `docs/screens/pad-discovery.png` out. Afterwards I restored `simulator/main.c` (git shows it unchanged) and restored `docs/screens/` to the step-3 state, so no layout or simulator code was changed or committed.

**Where they are.** The renders are under `bench-logs/` (git-ignored), next to the measuring script and its output:

| Screenshot | Headline (phase_err line 1) | Fraction |
|---|---|---|
| `bench-logs/release-prep-1.0.4b2/pad-discovery-pass2-510-of-510.png` | Still looking (checking more slowly)... | 510/510 |
| `bench-logs/release-prep-1.0.4b2/pad-discovery-pass2-254-of-510.png` | Still looking (checking more slowly)... | 254/510 |
| `bench-logs/release-prep-1.0.4b2/pad-discovery-pass1-510-of-510.png` | Looking for your Somnus pad... | 510/510 |
| `bench-logs/release-prep-1.0.4b2/pad-discovery-pass1-254-of-510.png` | Looking for your Somnus pad... | 254/510 |
| `bench-logs/release-prep-1.0.4b2/pad-discovery-pass2-137-of-254-baseline.png` | Still looking (checking more slowly)... | 137/254 |

The 137/254 baseline render is byte-identical to the committed `docs/screens/pad-discovery.png`. That confirms the patched-scenario method reproduces the real scenario.

**What they show** (I looked at each render):

- The fraction is centred in the ring, on one line, in Montserrat 24, the same as at 137/254.
- The ring's fill matches the arc value: full at 510/510, about half at 254/510 (49%).
- The headline is unchanged by this release. The pass-2 headline wraps to two lines and the pass-1 headline fits on one, the same as at 1.0.4-beta.1.

**How fit was measured.** Ink is any pixel that differs from the background #101418 by more than 40 in any channel, inside the round mask. Fraction ink is ink within 90 px of the ring centre (180, 210). Ring ink is ink 90–118 px from that centre. Headline ink is ink above y = 95.

```
$ python3 measure.py   (in bench-logs/release-prep-1.0.4b2/)
pad-discovery-pass1-254-of-510.png
  fraction ink box x 134-224 (w 91) y 198-220 (h 23); farthest fraction pixel from ring centre 45.8px, ring inner edge 95.6px -> clearance 49.8px
  headline ink box x 55-304 y 54-68; ring top y 100 -> vertical gap 31px; farthest headline pixel from screen centre 176.8px of 180 -> margin 3.2px
  ink pixels on the 360x360 frame edge rows/cols: 0
pad-discovery-pass1-510-of-510.png
  fraction ink box x 137-221 (w 85) y 198-220 (h 23); farthest fraction pixel from ring centre 43.0px, ring inner edge 95.6px -> clearance 52.6px
  headline ink box x 55-304 y 54-68; ring top y 100 -> vertical gap 31px; farthest headline pixel from screen centre 176.8px of 180 -> margin 3.2px
  ink pixels on the 360x360 frame edge rows/cols: 0
pad-discovery-pass2-137-of-254-baseline.png
  fraction ink box x 137-223 (w 87) y 198-220 (h 23); farthest fraction pixel from ring centre 43.7px, ring inner edge 95.6px -> clearance 51.9px
  headline ink box x 67-288 y 45-77; ring top y 100 -> vertical gap 22px; farthest headline pixel from screen centre 174.7px of 180 -> margin 5.3px
  ink pixels on the 360x360 frame edge rows/cols: 0
pad-discovery-pass2-254-of-510.png
  fraction ink box x 134-224 (w 91) y 198-220 (h 23); farthest fraction pixel from ring centre 45.8px, ring inner edge 95.6px -> clearance 49.8px
  headline ink box x 67-288 y 45-77; ring top y 100 -> vertical gap 22px; farthest headline pixel from screen centre 174.7px of 180 -> margin 5.3px
  ink pixels on the 360x360 frame edge rows/cols: 0
pad-discovery-pass2-510-of-510.png
  fraction ink box x 137-221 (w 85) y 198-220 (h 23); farthest fraction pixel from ring centre 43.0px, ring inner edge 95.6px -> clearance 52.6px
  headline ink box x 67-288 y 45-77; ring top y 100 -> vertical gap 22px; farthest headline pixel from screen centre 174.7px of 180 -> margin 5.3px
  ink pixels on the 360x360 frame edge rows/cols: 0
```

- The widest fraction ink is 91 px wide ("254/510"), against 87 px for the shipped "137/254". Its farthest pixel is 45.8 px from the ring centre, and the ring's inner edge is at 95.6 px, leaving **49.8 px of clearance**.
- The fraction (y 198–220) and the headline (bottom at y 77 or 68) never meet: the ring top at y 100 lies between them.
- No ink touches the frame edge.
- The pass-1 headline's corners are 3.2 px inside the round mask. That headline text is unchanged in this release (and from 1.0.4-beta.1), so it is not something this step introduces.

## Raw output

### `idf.py build` (exit 0), unfiltered

```
$ cd firmware/dial-idf && source ~/esp/esp-idf/export.sh && idf.py build
CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.
This warning is for project developers.  Use -Wno-author or -Wno-deprecated
to suppress it.

Executing action: all (aliases: build)
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
-- Component directory /Users/matthew/esp/esp-idf/components/mqtt does not contain a CMakeLists.txt file. No component will be added
-- Minimal build - OFF
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
-- App "somnus-dial" version: 1.0.4-beta.2
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
-- Configuring done (8.6s)
-- Generating done (1.3s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
[  0%] Built target blank_ota_data
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target _project_elf_src
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_https_ota
[  0%] Built target __idf_esp_http_server
[  2%] Built target bootloader_ld_in_preprocess
[  2%] Built target _project_elf_src
[  8%] Built target __idf_log
[  1%] Built target __idf_esp_http_client
[ 16%] Built target __idf_esp_rom
[  1%] Built target __idf_tcp_transport
[ 18%] Built target __idf_esp_common
[  1%] Built target __idf_esp_driver_i2s
[ 25%] Built target __idf_esp_hw_support
[ 26%] Built target __idf_esp_system
[  2%] Built target __idf_esp_adc
[ 32%] Built target __idf_efuse
[  3%] Built target __idf_esp-tls
[ 47%] Built target __idf_bootloader_support
[  3%] Built target __idf_http_parser
[ 50%] Built target __idf_esp_security
[  3%] Built target __idf_esp_hal_i2c
[ 52%] Built target __idf_esp_hal_security
[  3%] Built target __idf_esp_gdbstub
[ 56%] Built target __idf_esp_hal_ana_conv
[  3%] Built target __idf_esp_wifi
[ 59%] Built target __idf_esp_hal_uart
[  3%] Built target __idf_esp_coex
[ 62%] Built target __idf_esp_hal_wdt
[ 64%] Built target __idf_esp_hal_timg
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[  8%] Built target __idf_wpa_supplicant
[ 71%] Built target __idf_esp_hal_gpspi
[  9%] Built target __idf_esp_netif
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 76%] Built target __idf_esp_hal_pmu
[ 79%] Built target __idf_esp_hal_usb
[ 84%] Built target __idf_esp_hal_gpio
[ 15%] Built target __idf_lwip
[ 88%] Built target __idf_hal
[ 15%] Built target __idf_vfs
[ 93%] Built target __idf_soc
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 95%] Built target __idf_xtensa
[ 15%] Built target __idf_esp_phy
[ 97%] Built target __idf_main
[ 98%] Built target bootloader.elf
[ 16%] Built target __idf_nvs_flash
[100%] Built target gen_bootloader_binary
[ 17%] Built target __idf_nvs_sec_provider
[100%] Built target gen_project_binary
[ 18%] Built target __idf_esp_event
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 19%] Built target __idf_esp_driver_uart
[100%] Built target bootloader_check_size
[100%] Built target app
[ 19%] Built target __idf_esp_psram
[ 19%] No install step for 'bootloader'
[ 19%] Built target __idf_esp_ringbuf
[ 19%] Completed 'bootloader'
[ 20%] Built target bootloader
[ 21%] Built target __idf_esp_timer
[ 22%] Built target __idf_cxx
[ 22%] Built target __idf_pthread
[ 23%] Built target __idf_esp_libc
[ 25%] Built target __idf_freertos
[ 28%] Built target __idf_esp_hw_support
[ 29%] Built target __idf_esp_hal_i2s
[ 29%] Built target __idf_esp_hal_touch_sens
[ 29%] Built target __idf_esp_hal_pmu
[ 29%] Built target __idf_esp_hal_usb
[ 30%] Built target __idf_esp_hal_gpio
[ 31%] Built target __idf_soc
[ 32%] Built target __idf_heap
[ 33%] Built target __idf_log
[ 33%] Built target __idf_hal
[ 34%] Built target __idf_esp_rom
[ 34%] Built target __idf_esp_common
[ 36%] Built target __idf_esp_system
[ 37%] Built target __idf_spi_flash
[ 38%] Built target __idf_bootloader_support
[ 38%] Built target __idf_esp_hal_ana_conv
[ 38%] Built target __idf_esp_hal_wdt
[ 38%] Built target __idf_esp_hal_timg
[ 41%] Built target tfpsacrypto
[ 42%] Built target mbedtls
[ 43%] Built target mbedx509
[ 47%] Built target builtin
[ 47%] Built target p256m
[ 48%] Built target everest
[ 48%] Built target __idf_esp_driver_dma
[ 48%] Built target __idf_esp_mm
[ 49%] Built target __idf_esp_pm
[ 50%] Built target __idf_esp_hal_uart
[ 50%] Built target __idf_esp_driver_gpio
[ 50%] Built target __idf_esp_security
[ 50%] Built target __idf_efuse
[ 50%] Built target __idf_esp_partition
[ 50%] Built target __idf_app_update
[ 50%] Built target __idf_esp_bootloader_format
[ 50%] Building C object esp-idf/esp_app_format/CMakeFiles/__idf_esp_app_format.dir/esp_app_desc.c.obj
[ 50%] Linking C static library libesp_app_format.a
[ 50%] Built target __idf_esp_app_format
[ 51%] Built target __idf_esp_hal_security
[ 52%] Built target __idf_esp_hal_mspi
[ 52%] Built target __idf_esp_hal_clock
[ 52%] Built target __idf_esp_hal_gpspi
[ 52%] Built target __idf_esp_hal_dma
[ 53%] Built target __idf_esp_stdio
[ 54%] Built target __idf_xtensa
[ 54%] Built target __idf_esp_hal_mcpwm
[ 54%] Built target __idf_esp_hal_rmt
[ 54%] Built target __idf_esp_hal_pcnt
[ 54%] Built target __idf_esp_driver_sdm
[ 54%] Built target __idf_esp_hal_lcd
[ 54%] Built target __idf_esp_hal_twai
[ 55%] Built target __idf_esp_https_server
[ 55%] Built target __idf_esp_driver_touch_sens
[ 55%] Built target __idf_esp_driver_tsens
[ 55%] Built target __idf_esp_hid
[ 55%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_sdmmc
[ 56%] Built target __idf_protobuf-c
[ 56%] Built target __idf_esp_driver_i2c
[ 56%] Built target __idf_perfmon
[ 57%] Built target __idf_wear_levelling
[ 57%] Built target __idf_rt
[ 57%] Built target __idf_esp_hal_ledc
[ 58%] Built target __idf_console
[ 59%] Built target __idf_spiffs
[ 60%] Built target __idf_espressif__cjson
[ 60%] Built target __idf_dial_state
[ 61%] Built target __idf_driver
[ 61%] Built target __idf_dial_knob
[ 61%] Built target __idf_dial_time
[ 62%] Built target __idf_esp_hal_cam
[ 63%] Built target __idf_esp_driver_gptimer
[ 63%] Built target __idf_esp_driver_pcnt
[ 63%] Built target __idf_esp_driver_sd_intf
[ 65%] Built target __idf_unity
[ 66%] Built target __idf_esp_driver_mcpwm
[ 66%] Built target __idf_esp_driver_sdspi
[ 66%] Built target __idf_esp_driver_twai
[ 67%] Built target __idf_esp_driver_rmt
[ 67%] Built target __idf_esp_eth
[ 68%] Built target __idf_esp_lcd
[ 68%] Built target __idf_esp_driver_ledc
[ 69%] Built target __idf_protocomm
[ 69%] Built target __idf_dial_ota
[ 69%] Built target __idf_cmock
[ 69%] Built target __idf_dial_net
[ 69%] Built target __idf_dial_somnus
[ 70%] Built target __idf_esp_driver_cam
[ 70%] Built target __idf_esp_driver_sdmmc
[ 70%] Built target __idf_espressif__esp_lcd_sh8601
[ 71%] Built target __idf_esp_local_ctrl
[ 72%] Built target __idf_dial_pad_discovery
[ 72%] Built target __idf_fatfs
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Built target __idf_dial_display
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Built target __idf_i2c_bsp
[ 97%] Built target __idf_dial_haptics
[ 97%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
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
somnus-dial.bin binary size 0x189a40 bytes. Smallest app partition is 0x400000 bytes. 0x2765c0 bytes (62%) free.
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
build exit 0
```

### Existing host test

```
$ cc -I components/dial_state -Wall -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
all relative-scale table assertions passed
exit 0
```

### Simulator configure + build (step 3), unfiltered

```
$ cmake -B build -S simulator && cmake --build build
-- dial_sim: firmware version 1.0.4-beta.2, OTA scenarios advertise 1.0.5
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
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_settings.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_timezone.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_discovery.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_address.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_adjust_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness_menu.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_palette.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_list.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_88.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_16.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_20.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_140.c.o
[ 49%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

### Simulator run (step 3), unfiltered

```
$ cd build && ./dial_sim
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

### Simulator builds and runs for the step-4 variants, unfiltered

#### pass2-510-of-510: `cmake --build build` (after deleting `main.c.o` and `dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

#### pass2-510-of-510: `./dial_sim`

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (2 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

#### pass2-254-of-510: `cmake --build build` (after deleting `main.c.o` and `dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

#### pass2-254-of-510: `./dial_sim`

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (4 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

#### pass1-510-of-510: `cmake --build build` (after deleting `main.c.o` and `dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

#### pass1-510-of-510: `./dial_sim`

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (5 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

#### pass1-254-of-510: `cmake --build build` (after deleting `main.c.o` and `dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

#### pass1-254-of-510: `./dial_sim`

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (7 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

#### pass2-137-of-254-baseline: `cmake --build build` (after deleting `main.c.o` and `dial_sim`)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

#### pass2-137-of-254-baseline: `./dial_sim`

```
I (ui_router) router up, screen 0
wrote welcome          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/welcome.png (6 distinct colors sampled)
wrote wifi-portal      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-portal.png (5 distinct colors sampled)
wrote netpick          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/netpick.png (6 distinct colors sampled)
wrote passkey          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/passkey.png (4 distinct colors sampled)
wrote connecting       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/connecting.png (1 distinct colors sampled)
wrote dial             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial.png (10 distinct colors sampled)
wrote dial-update      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-update.png (10 distinct colors sampled)
wrote dial-relative    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative.png (8 distinct colors sampled)
wrote dial-celsius     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-celsius.png (10 distinct colors sampled)
wrote dial-relative-max /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-relative-max.png (9 distinct colors sampled)
wrote dial-night-water /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/dial-night-water.png (7 distinct colors sampled)
wrote rails-420-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=410
wrote rails-420-up-down /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-420-up-down.png (10 distinct colors sampled)
wrote rails-423        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=420
wrote rails-423-up     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-423-up.png (10 distinct colors sampled)
wrote rails-drag-337-live /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337-live.png (10 distinct colors sampled)
[cmd] SET_TEMP zone=0 a=0 b=0 temp_dc=340
wrote rails-drag-337   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/rails-drag-337.png (10 distinct colors sampled)
wrote menu             /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/menu.png (3 distinct colors sampled)
wrote update           /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
wrote update-prompt    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-prompt.png (3 distinct colors sampled)
wrote update-failed    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/update-failed.png (6 distinct colors sampled)
wrote settings         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings.png (4 distinct colors sampled)
wrote settings-pad     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-pad.png (2 distinct colors sampled)
wrote settings-timezone-raw /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-timezone-raw.png (2 distinct colors sampled)
wrote timezone         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/timezone.png (5 distinct colors sampled)
wrote night-mode       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-mode.png (4 distinct colors sampled)
wrote night-face       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/night-face.png (6 distinct colors sampled)
wrote standby-face     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-face.png (5 distinct colors sampled)
wrote settings-standby-face /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/settings-standby-face.png (9 distinct colors sampled)
wrote pad-address      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-address.png (5 distinct colors sampled)
wrote pad-unreachable  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-unreachable.png (4 distinct colors sampled)
wrote pad-discovery    /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-discovery.png (3 distinct colors sampled)
wrote pad-degraded-real /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (3 distinct colors sampled)
wrote adjust-mode      /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/adjust-mode.png (9 distinct colors sampled)
wrote brightness-menu  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-menu.png (5 distinct colors sampled)
wrote brightness       /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness.png (6 distinct colors sampled)
wrote brightness-clock /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock.png (5 distinct colors sampled)
wrote brightness-clock-off /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/brightness-clock-off.png (4 distinct colors sampled)
wrote wifi-info        /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-info.png (6 distinct colors sampled)
wrote wifi-confirm     /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/wifi-confirm.png (6 distinct colors sampled)
wrote about            /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about.png (4 distinct colors sampled)
wrote about-wifi-worst /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-worst.png (5 distinct colors sampled)
wrote about-wifi-real  /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-wifi-real.png (5 distinct colors sampled)
wrote about-battery-pct /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-pct.png (2 distinct colors sampled)
wrote about-battery-usb /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/about-battery-usb.png (2 distinct colors sampled)
wrote updating         /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/updating.png (8 distinct colors sampled)
wrote standby          /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby.png (4 distinct colors sampled)
wrote standby-update   /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens/standby-update.png (4 distinct colors sampled)
done: 48 screens rendered to /Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/screens
```

#### Rebuild after restoring `simulator/main.c` (so `build/dial_sim` matches the committed source)

```
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

### release.yml checks, run locally against HEAD

```
$ (release.yml checks, run locally against HEAD with GITHUB_REF_NAME=somnus-v1.0.4-beta.2; grep -P replaced by sed because BSD grep has no -P)
Tag version 1.0.4-beta.2 matches PROJECT_VER 1.0.4-beta.2
--- extracted body ---
The dial can now find your pad by itself on routers that use larger
networks, such as eero mesh systems. Before, on those networks the dial
gave up looking and you had to type the pad's address in by hand.
--- end ---
```

## Deviations from the spec

1. **Five screens changed, not six.** The 1.0.4-beta.1 screens commit changed six. `update-prompt.png` shows only the offered OTA version. The simulator derives that as the patch number plus one with the beta suffix removed, which is "1.0.5" for both 1.0.4-beta.1 and 1.0.4-beta.2, so its render is byte-identical. This is expected, not a rendering problem.
2. **Two commits, not one.** The spec calls 19fab5a "the 1.0.4-beta.1 release commit". 19fab5a is actually the docs commit that followed the release commit 1d01f3b (which held only PROJECT_VER and CHANGELOG). I followed that pair:
   - `1598b27` holds the bump and the changelog together, as step 2 requires;
   - `94241fc` holds the screens.

   Both commits carry PROJECT_VER 1.0.4-beta.2, so release.yml would accept a tag on either. The owner picks which one the tag goes on. At 1.0.4-beta.1 the tag was on the release commit.
3. **Step 4 renders are not in `docs/screens/` and not committed.** They are in the git-ignored `bench-logs/release-prep-1.0.4b2/`. Rendering needed a temporary edit to the simulator scenario string, which I reverted. I also rendered two variants the spec didn't ask for: the pass-1 headline, and the 137/254 baseline for comparison.
4. **The first step-4 attempt rendered the wrong images, and I discarded it.** I restored `main.c` with `cp -p`, which kept its old mtime, so make didn't rebuild and later variants showed earlier strings. The final set was made by deleting `main.c.o` and `dial_sim` before every build. I checked each image's fraction by eye, and the baseline's byte-identity to the committed PNG confirms this method.
5. **Uncommitted files.** This report and its new `docs/REPORTS.md` line are not committed, following the standing convention that reports are not committed with a change. The previous task's `docs/REPORT-pad-discovery-wide-subnet.md` and its REPORTS.md line are also still uncommitted.
6. **Extra check:** I ran release.yml's tag/PROJECT_VER and CHANGELOG-extraction logic locally. The workflow uses `grep -oP`; I replaced it with an equivalent `sed`, because macOS grep has no `-P`.

## What can't be verified without hardware

- **On the device itself:**
  - the panel shows "v1.0.4-beta.2" under About;
  - the discovery screen's "N/510" text looks right on the real 360×360 round display (the simulator uses the same LVGL 8.4.0 and fonts, but not the panel itself);
  - the real scan actually reports a total of 510, which needs a dial on a network wider than /24.
- **The discovery change on an eero /22:** see `docs/REPORT-pad-discovery-wide-subnet.md`.
- **Release mechanics:** an OTA update from 1.0.4-beta.1 to 1.0.4-beta.2 on a dial with Beta builds on, and the browser-flasher install of the beta image. These also depend on the tag, push and CI release, which this task does not do.
- **CI:** that release.yml passes in GitHub Actions. I only ran its two checks locally.
