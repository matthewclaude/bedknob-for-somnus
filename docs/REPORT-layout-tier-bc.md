# REPORT — Layout audit Tier B + Tier C, stale comments, discovery log noise (0.1.6-beta.3 fix pass)

**Verdict: DONE WITH DEVIATIONS** — all items done; two deviations from the task block's wording (discovery lines muted per-tag rather than re-levelled to DEBUG, because they are IDF library ESP_LOGE calls; comment (a) was at dial_state.h:183-241 and :557-568, not line 501), one from the plan (netpick keeps `lv_obj_center` after the width/long-mode lines, the plan said "replaces"). Details in §7.

## 1. Verdict

DONE WITH DEVIATIONS (see §7). No PROJECT_VER bump, no CHANGELOG edit, no tag, no push. Nothing touches the Somnus API paths or the pad.

## 2. Gate check (raw)

```
$ git rev-parse --abbrev-ref HEAD
main
$ git --no-optional-locks status --short

$ git tag --list 'somnus-v*' --sort=-v:refname | head -1
somnus-v0.1.6-beta.2
$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.6-beta.2")
```

All four agree with the required values. Gate passed.

## 3. Item-by-item status

| Item | Status | Where |
|---|---|---|
| B1 Update row string | done — `"v%s available - tap to install"` → `"v%s - tap to install"`; the create() comment's example string updated to match | `firmware/dial-idf/components/dial_ui/scr_update.c:302` |
| B2 Netpick row width | done — `make_row()` label gets `LV_PCT(100)` width, `LV_LABEL_LONG_DOT`, `LV_TEXT_ALIGN_CENTER` (the existing `lv_obj_center` is kept for vertical centring, see §7) | `firmware/dial-idf/components/dial_ui/scr_netpick.c:58` |
| B3 Wrong-password copy | done — `"That password didn't work. Try again."` → `"Wrong password, try again"` | `firmware/dial-idf/components/dial_ui/scr_passkey.c:139` |
| B4 Adjust mode y-offsets | done — Back pill `310 - CY` → `300 - CY`; description `235 - CY` → `228 - CY`; header "y-offsets below" comment now lists the slots | `firmware/dial-idf/components/dial_ui/scr_adjust_mode.c:249`, `firmware/dial-idf/components/dial_ui/scr_adjust_mode.c:263` |
| B5 Night mode title/note | done — title `64 - CY` → `56 - CY`, note `84 - CY` → `74 - CY`, comment explains why this list leaves the shared 64 slot | `firmware/dial-idf/components/dial_ui/scr_night_mode.c:201`, `firmware/dial-idf/components/dial_ui/scr_night_mode.c:206` |
| B6 Wi-Fi confirm body | done — `126 - CY` → `116 - CY` | `firmware/dial-idf/components/dial_ui/scr_wifi.c:221` |
| B7 Standby clock block | done (optional item taken: it is three constants across seven `lv_obj_align` lines, no comment in the file marks the old placement as deliberate) — dots `104` → `112` (5 sites), clock `168` → `176`, date `232` → `240` | `firmware/dial-idf/components/dial_ui/scr_standby.c:172`, `firmware/dial-idf/components/dial_ui/scr_standby.c:177` |
| C1 Update row flex block | done — the "Check for updates" row is a flex column (`LV_FLEX_FLOW_COLUMN`, main axis CENTER, cross axis START, `pad_ver 0`, `pad_row 4`); the three `lv_obj_align` offsets (-20/+6/+26) are gone; value/error widths resolved via `lv_obj_update_layout` exactly as About's `make_info_row()`; the error line is `LV_OBJ_FLAG_HIDDEN` outside FAILED and shown in FAILED. No string or behaviour change. | `firmware/dial-idf/components/dial_ui/scr_update.c:367` (create), `firmware/dial-idf/components/dial_ui/scr_update.c:318` (render_ota_row) |
| C2 Connecting/error screen | done — new `apply_palette()` sets `pal->bg`, `ink_primary` (main), `ink_secondary` (sub), called from create() and on_state(); `main_color` default is `PAL()->ink_primary`; `lv_color_black()`, `0xe0e0e0` ×2 and `0x808080` removed; offsets `-12` → `-24`, `+28` → `+24` | `firmware/dial-idf/components/dial_ui/scr_connecting.c:19`, `firmware/dial-idf/components/dial_ui/scr_connecting.c:42`, `firmware/dial-idf/components/dial_ui/scr_connecting.c:54` |
| Comment (a) Screen timeout | done — "these five" / "five labels" / `"2m"` / "five choices'" / "of those five" → four / `"1m"` / four, matching `DIAL_SCR_TIMEOUT_CHOICES = {5,15,30,60}` and the 90 s default's nearest label | `firmware/dial-idf/components/dial_state/dial_state.h:191`, `:227`, `:563` |
| Comment (b) main.c auto-update | done — two-strikes block no longer says "overnight install" / "many nights" / "overnight-window OCCURRENCE"; now names `dial_auto_update_window`'s post-wake window (two hours after night ends, two hours wide, fixed 09:00-11:00 only when night is off); the CMD_OTA_APPLY comment's "unattended overnight path" → "post-wake path" | `firmware/dial-idf/main/main.c:586`, `:595`, `:913` |
| Comment (c) simulator scenario_update | done — row order now `Back(0) / Check for updates(1) / Installed(2) / Skip(3, AVAILABLE only) / Auto-update(4) / Beta builds(5)`, states the rotor settles on row 1 so +1 detent lands on Installed; "Auto-update set to Overnight" → "On (renders the derived post-wake window)" | `simulator/main.c:607` |
| Discovery log noise | done — per-host lines identified as IDF library `ESP_LOGE` from tags `esp-tls`, `transport_base`, `HTTP_CLIENT` (not this component's own calls, which had no per-host log at all); those three tags are set to `ESP_LOG_NONE` for the scan's duration via `esp_log_level_set` and restored with the levels read back by `esp_log_level_get` on every exit path; `"scan: no pad found"` WARN → INFO. Candidate list, pass count, timeouts, concurrency, found/not-found logic untouched. | `firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:51`, `:391`, `:401` |

Not touched, per the rules: Tier D (D1, D2), SCR_SIDEPICK / scr_sidepick.c / side_picked, PROJECT_VER, CHANGELOG.md, any Somnus API path, docs/screens (the simulator was run against a backup and the checked-in PNGs restored byte-for-byte; `git status docs/screens` empty).

## 4. Build output

### 4a. `idf.py build` (firmware/dial-idf), RAW, unfiltered

Exit code 0. The only warning is CMake's pre-existing `cmake_minimum_required` deprecation notice at `CMakeLists.txt:5`, unrelated to this pass.

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
-- App "somnus-dial" version: 0.1.6-beta.2
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
-- Configuring done (9.6s)
-- Generating done (1.3s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
[  0%] Built target partition_table_bin
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target blank_ota_data
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Performing build step for 'bootloader'
[  1%] Built target __idf_esp_https_ota
[  1%] Built target __idf_esp_http_server
[  2%] Built target _project_elf_src
[  2%] Built target bootloader_ld_in_preprocess
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
[  2%] Built target __idf_esp-tls
[ 47%] Built target __idf_bootloader_support
[  2%] Built target __idf_http_parser
[ 50%] Built target __idf_esp_security
[  2%] Built target __idf_esp_hal_i2c
[ 52%] Built target __idf_esp_hal_security
[  2%] Built target __idf_esp_gdbstub
[ 56%] Built target __idf_esp_hal_ana_conv
[  2%] Built target __idf_esp_wifi
[ 59%] Built target __idf_esp_hal_uart
[  2%] Built target __idf_esp_coex
[ 62%] Built target __idf_esp_hal_wdt
[ 64%] Built target __idf_esp_hal_timg
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[  7%] Built target __idf_wpa_supplicant
[ 71%] Built target __idf_esp_hal_gpspi
[  8%] Built target __idf_esp_netif
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 76%] Built target __idf_esp_hal_pmu
[ 79%] Built target __idf_esp_hal_usb
[ 84%] Built target __idf_esp_hal_gpio
[ 14%] Built target __idf_lwip
[ 88%] Built target __idf_hal
[ 14%] Built target __idf_vfs
[ 93%] Built target __idf_soc
[ 14%] Built target __idf_esp_driver_usb_serial_jtag
[ 95%] Built target __idf_xtensa
[ 14%] Built target __idf_esp_phy
[ 97%] Built target __idf_main
[ 98%] Built target bootloader.elf
[ 15%] Built target __idf_nvs_flash
[100%] Built target gen_bootloader_binary
[ 16%] Built target __idf_nvs_sec_provider
[100%] Built target gen_project_binary
[ 17%] Built target __idf_esp_event
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 18%] Built target __idf_esp_driver_uart
[100%] Built target bootloader_check_size
[ 18%] Built target __idf_esp_psram
[100%] Built target app
[ 18%] No install step for 'bootloader'
[ 18%] Built target __idf_esp_ringbuf
[ 18%] Completed 'bootloader'
[ 19%] Built target __idf_esp_timer
[ 20%] Built target bootloader
[ 21%] Built target __idf_cxx
[ 21%] Built target __idf_pthread
[ 22%] Built target __idf_esp_libc
[ 24%] Built target __idf_freertos
[ 27%] Built target __idf_esp_hw_support
[ 28%] Built target __idf_esp_hal_i2s
[ 28%] Built target __idf_esp_hal_touch_sens
[ 28%] Built target __idf_esp_hal_pmu
[ 28%] Built target __idf_esp_hal_usb
[ 29%] Built target __idf_esp_hal_gpio
[ 30%] Built target __idf_soc
[ 31%] Built target __idf_heap
[ 32%] Built target __idf_log
[ 32%] Built target __idf_hal
[ 33%] Built target __idf_esp_rom
[ 33%] Built target __idf_esp_common
[ 35%] Built target __idf_esp_system
[ 36%] Built target __idf_spi_flash
[ 37%] Built target __idf_bootloader_support
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
[ 51%] Built target __idf_efuse
[ 51%] Built target __idf_esp_partition
[ 51%] Built target __idf_app_update
[ 51%] Built target __idf_esp_bootloader_format
[ 51%] Building C object esp-idf/esp_app_format/CMakeFiles/__idf_esp_app_format.dir/esp_app_desc.c.obj
[ 51%] Linking C static library libesp_app_format.a
[ 51%] Built target __idf_esp_app_format
[ 52%] Built target __idf_esp_hal_security
[ 53%] Built target __idf_esp_hal_mspi
[ 53%] Built target __idf_esp_hal_clock
[ 53%] Built target __idf_esp_hal_gpspi
[ 53%] Built target __idf_esp_hal_dma
[ 54%] Built target __idf_esp_stdio
[ 55%] Built target __idf_xtensa
[ 55%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_espressif__cjson
[ 56%] Built target __idf_dial_knob
[ 56%] Built target __idf_dial_time
[ 56%] Built target __idf_esp_hal_lcd
[ 56%] Building C object esp-idf/dial_state/CMakeFiles/__idf_dial_state.dir/dial_state.c.obj
[ 57%] Built target __idf_esp_driver_spi
[ 57%] Built target __idf_unity
[ 58%] Built target __idf_esp_driver_gptimer
[ 58%] Built target __idf_esp_driver_i2c
[ 58%] Built target __idf_esp_hal_mcpwm
[ 58%] Built target __idf_esp_hal_pcnt
[ 58%] Built target __idf_esp_driver_sdm
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_hal_rmt
[ 58%] Built target __idf_esp_driver_tsens
[ 59%] Built target __idf_sdmmc
[ 59%] Built target __idf_esp_driver_twai
[ 59%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_console
[ 61%] Built target __idf_esp_hid
[ 61%] Built target __idf_esp_eth
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_perfmon
[ 61%] Built target __idf_protobuf-c
[ 61%] Built target __idf_rt
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_driver
[ 62%] Built target __idf_esp_driver_ledc
[ 63%] Built target __idf_spiffs
[ 63%] Built target __idf_dial_net
[ 63%] Built target __idf_cmock
[ 63%] Built target __idf_esp_driver_pcnt
[ 63%] Built target __idf_dial_somnus
[ 63%] Built target __idf_dial_ota
[ 64%] Built target __idf_esp_driver_cam
[ 65%] Built target __idf_esp_lcd
[ 65%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_esp_driver_mcpwm
[ 67%] Built target __idf_esp_driver_rmt
[ 68%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_espressif__esp_lcd_sh8601
[ 69%] Built target __idf_protocomm
[ 69%] Built target __idf_esp_driver_sdmmc
[ 70%] Built target __idf_esp_local_ctrl
[ 70%] Built target __idf_fatfs
[ 70%] Linking C static library libdial_state.a
[ 70%] Built target __idf_dial_state
[ 71%] Building C object esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj
[ 71%] Linking C static library libdial_pad_discovery.a
[ 71%] Built target __idf_dial_pad_discovery
[ 96%] Built target __idf_lvgl__lvgl
[ 96%] Built target __idf_dial_display
[ 96%] Built target __idf_lcd_bl_pwm_bsp
[ 96%] Built target __idf_lcd_touch_bsp
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Building C object esp-idf/dial_power/CMakeFiles/__idf_dial_power.dir/dial_power.c.obj
[ 96%] Linking C static library libdial_power.a
[ 96%] Built target __idf_dial_power
[ 96%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_router.c.obj
[ 96%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_screens.c.obj
[ 96%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_setup.c.obj
[ 96%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_connecting.c.obj
[ 96%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_discovery.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_wifi.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_netpick.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_about.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_menu.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_passkey.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_dial.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_updating.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update_prompt.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_welcome.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_sidepick.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_settings.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_address.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_adjust_mode.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness_menu.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_mode.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_face.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby_face.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_timezone.c.obj
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
somnus-dial.bin binary size 0x189930 bytes. Smallest app partition is 0x400000 bytes. 0x2766d0 bytes (62%) free.
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
EXIT=0
```

### 4b. Simulator build (`cmake --build build` from the repo root, per simulator/README.md), RAW

Built and linked `build/dial_sim`; zero warnings. Then run once (49 screens rendered) with docs/screens backed up beforehand and restored afterwards.

```
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
[ 48%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 86%] Built target lvgl_examples
[100%] Built target lvgl_demos
```

## 5. Commits

### Commit 1 — layout (Tier B + C)

`01c6f3ab9f2a0c6eec83c3ea06bd5055585a791e`

```
 .../dial-idf/components/dial_ui/scr_adjust_mode.c  | 14 ++++-
 .../dial-idf/components/dial_ui/scr_connecting.c   | 33 +++++++++--
 firmware/dial-idf/components/dial_ui/scr_netpick.c |  7 +++
 .../dial-idf/components/dial_ui/scr_night_mode.c   | 12 +++-
 firmware/dial-idf/components/dial_ui/scr_passkey.c |  6 +-
 firmware/dial-idf/components/dial_ui/scr_standby.c | 19 +++---
 firmware/dial-idf/components/dial_ui/scr_update.c  | 68 ++++++++++++++++------
 firmware/dial-idf/components/dial_ui/scr_wifi.c    |  6 +-
 8 files changed, 126 insertions(+), 39 deletions(-)
```

### Commit 2 — comments + discovery logging

`fe7c556cff7faa785d99a1ddd246a9fb0d7a8f49`

```
 .../dial_pad_discovery/dial_pad_discovery.c        | 53 +++++++++++++++++++---
 .../dial-idf/components/dial_state/dial_state.h    | 10 ++--
 firmware/dial-idf/main/main.c                      | 15 +++---
 simulator/main.c                                   | 19 ++++----
 4 files changed, 71 insertions(+), 26 deletions(-)
```

`git --no-optional-locks status --short` is empty after both commits. docs/REPORT-*.md is not in either commit (this file is uncommitted).

## 6. Key values shown working

Measured on fresh simulator renders (ink bands, y then x, from the PNGs), before = checked-in tree at f46a779, after = commit 1. Widths computed by summing the compiled Montserrat `adv_w` tables (calibrates exactly to the audit's 291 / 380 px figures).

| Item | Before | After |
|---|---|---|
| B1 string width (Mont 16) | `v1.2.0-beta.3 available - tap to install` = **291 px** (row content 288 → ellipsized); `v1.4.3 available…` = 234 | `v1.2.0-beta.3 - tap to install` = **215 px**; `v1.2.0-beta.10 - tap to install` = 223; `v10.20.30-beta.10 - tap to install` = 254. All < 288. Sim `update.png` value line ink x 53-191 (whole, no ellipsis; was 53-257 at the row edge). |
| B2 netpick | label width unset (content), no long mode | label `LV_PCT(100)` = 288 px, `LONG_DOT`, centred; `netpick.png` rows render identically to within 1 px in x (145-216 → 144-215) |
| B3 passkey copy (Mont 20) | 380 px in a 280 px `LONG_DOT` label | **276 px**, fits whole. Not rendered by any simulator scenario (passkey.png unchanged, as expected). |
| B4 adjust mode | Back pill centre 310, ink bottom **y 345** (chord tangent, 0 px); description 235 | pill centre 300, ink bottom **y 335**; description 228 (`adjust-mode.png` bottom bands 298-345 → 311-335) |
| B5 night mode | title ink 58-69, note 79-88, row-above label 96-111 (**8 px** ink gap) | title 50-61, note 69-78, label 96-111 (**18 px** ink gap) |
| B6 wifi confirm | body ink 93-161, Continue top 166 (**5 px**) | body 83-151, Continue 166 (**15 px**) |
| B7 standby (x 120-240 window) | dots 100-107, clock 137-200, date 226-237 → block 100-237, centre 168.5 | dots 108-115, clock 145-208, date 234-245 → block 108-245, **centre 176.5** |
| C1 update (two-line, `update.png`, row one above focus) | label ink 78-98, value 104-116 (9 px under the title's 58-69) | label 85-105, value 112-124 (**16 px** under the title); block centred in the row |
| C1 update (FAILED, `update-failed.png`, focused row 142-218) | label 151-173, value 180-194, error 201-213 | 150-172, 180-194, 201-213 (3-line block 68 of 76, unchanged within 1 px as the plan predicted) |
| C2 bg | `#000000` (`connecting.png`, `pad-unreachable.png`, `pad-degraded-real.png`) | `#101419` = pal->bg day |
| C2 offsets, 4-line degraded (`pad-degraded-real.png`) | main + sub line 1 **merged into one band 160-189** (box overlap), block 160-243, centre 201.5 | main 148-162, sub 171-185 / 189-203 / 207-221 / 225-239: no overlap, block 148-239, **centre 193.5** |
| C2 offsets, boot (`connecting.png`) | main 160-178, sub 202-213 | main 148-166, sub 198-209 |

### ESP_LOGE / ESP_LOGW call sites in `components/dial_pad_discovery/dial_pad_discovery.c`

Before (f46a779):

```
318:        ESP_LOGE(TAG, "scan: out of semaphores, aborting pass");
343:        ESP_LOGW(TAG, "scan: no STA IP, skipping");
354:        ESP_LOGW(TAG, "scan: subnet looser than /24 (or invalid), skipping");
371:    ESP_LOGW(TAG, "scan: no pad found");
```

After (fe7c556):

```
43: * INFO lines. They are the library's own ESP_LOGE calls, so they can't be
350:        ESP_LOGE(TAG, "scan: out of semaphores, aborting pass");
375:        ESP_LOGW(TAG, "scan: no STA IP, skipping");
386:        ESP_LOGW(TAG, "scan: subnet looser than /24 (or invalid), skipping");
```

Line 43 is a comment. The three remaining call sites are the not-a-normal-scan early-outs (semaphore exhaustion, no STA IP, subnet looser than /24), none per-host. The per-host noise never came from this file — it is IDF's own `ESP_LOGE` in `esp-tls` (`[sock=N] select() timeout` / `connect() error`), `transport_base` (`Failed to open a new connection`) and `HTTP_CLIENT` (`Connection failed, sock < 0`), three lines per failed probe; those tags are muted for the scan and restored after. `CONFIG_LOG_DYNAMIC_LEVEL_CONTROL=y` in sdkconfig, so `esp_log_level_set` per tag is live.

### Hex-literal check

`git diff f46a779 01c6f3a -- firmware/dial-idf/components/dial_ui | grep '^+' | grep -E '0x[0-9A-Fa-f]{6}'`:

```
(exit 1; 1 = no matches)
```

Zero new hex colour literals in touched UI files. The diff removes three (`0xe0e0e0` ×2, `0x808080`) plus `lv_color_black()`.

### Recurring-bug-shape check per touched screen

- **Update row (B1/C1)** — the state the device shows is OTA_AVAILABLE with a real version string, and FAILED with `ota.err`. Rendered both in the simulator (`update.png` at v1.4.3, `update-failed.png` with "check failed (HTTP -1)"); the beta-string case computed at 215/223 px against 288. The "Tap again to confirm" and "Starting install..." strings go through the same `s_val_ota` label with the same `LONG_DOT` width, unchanged. Row add/remove of Skip (on_state) and `dial_list`'s zoom/fade pass were exercised by the scenario's `sim_knob(1)`; About already runs flex rows inside `dial_list`, so the rotor transform on a flex row is a known-good path.
- **Connecting (C2)** — the pre-pad-connect loop shows PH_SOMNUS_CONNECTING (one line), PH_DEGRADED with a real error + `Retrying in Ns` + `Swipe left for menu` (four lines) and PH_WIFI_CONNECTING with an SSID (two lines). Rendered `connecting.png`, `pad-unreachable.png` (3-line) and `pad-degraded-real.png` (4-line): no overlap in any. Palette is re-read in `on_state`, so a night flip while stuck on this screen recolours on the next state commit exactly as every other screen does; `main_color` for degraded still takes `warning` / night `ink_secondary`.
- **Netpick (B2)** — the device state is N real SSIDs of arbitrary length; `netpick.png` (short names) unchanged to 1 px; a 32-char SSID now ellipsizes inside 288 px instead of overflowing both ends. Not renderable without a long-SSID scenario (none exists; adding one would be new simulator code, out of scope).
- **Passkey (B3)** — the wrong-password state is `s_last_failed` after a portal reject; copy is 276 px in the 280 px label. Not renderable in the simulator (no scenario sets `s_last_failed`).
- **Adjust mode, Night mode, Wi-Fi confirm (B4-B6)** — fixed-slot screens; the rendered states (`adjust-mode.png`, `night-mode.png` with the §7 note showing, `wifi-confirm.png`) are the states the device shows.
- **Standby clock (B7)** — shown only when Standby face = Clock (Temperature is the default since beta.2). `standby.png` / `standby-update.png` rendered; the "Update available" label at 330 and the battery glyph at 46 are untouched, and the date's chord at y 249 is 332 px.

## 7. Deviations

1. **Discovery lines are muted, not demoted to DEBUG.** The task block says "Demote them to DEBUG (or VERBOSE)". The three per-host lines are not this component's calls — they are IDF's own `ESP_LOGE` in three library tags, and `CONFIG_LOG_MAXIMUM_LEVEL=3` (INFO) means DEBUG is not compiled in anyway. The nearest thing that does what was asked: set those three tags to `ESP_LOG_NONE` for the scan's duration and restore afterwards. The scan blocks the worker task, which is the only `esp_http_client` user (dial_somnus, dial_ota, this component), so no other request's errors fall in the muted window. If a genuinely-unexpected library error during a scan is ever needed, it is now invisible for that window — the tradeoff is documented in the source comment.
2. **Comment (a) location.** The task said "dial_state.h near line 501" and "the comment says 30s/1m/2m/5m/10m". The stale comment is the Screen-timeout block at lines 183-241 (plus the field comment at ~557-568); it said "five" choices and that a fresh device reads "2m". Fixed those; there is no literal "30s/1m/2m/5m/10m" string anywhere in the file (`grep` confirms). Line 501 is the `tz_prompted` / `units_c` area, untouched.
3. **B2 keeps `lv_obj_center`.** The plan says the three lines "replace the bare `lv_obj_center`". With the label at `LV_PCT(100)` width, `lv_obj_center` is what still centres it vertically in the 76 px row (LVGL 8 labels are top-left by default); dropping it would have moved every row label to the row's top. Kept it after the width/long-mode/text-align lines. Rendered result is identical to before for short names.
4. **B7 taken.** The plan marks it optional ("skip if the current placement is deliberate") and the task block says "only if trivial". Nothing in scr_standby.c or its history marks the 12 px-high placement as deliberate, and the change is three constants, so it was done. Easy to revert in isolation if the owner wants the old placement.
5. **Comment (c) scope.** Besides the row order, the same paragraph said "Auto-update set to Overnight"; the value has rendered the derived window since docs/SPEC-night-window.md §6, so that clause was corrected in the same edit rather than left half-stale.
6. **docs/screens not regenerated.** The plan's verification section regenerates the PNGs; this task block does not ask for it and the commits were to contain only the code changes, so the checked-in screenshots were left as they are (they are now stale for update, update-failed, netpick (1 px), adjust-mode, night-mode, wifi-confirm, standby, standby-update, connecting, pad-unreachable, pad-degraded-real). Fresh renders used for §6 are in the session scratchpad only.

No pixel value in this task block disagreed with the plan; the plan's values were used throughout.

## 8. Not verifiable without hardware

Every visual item needs a bench check on the dial (the simulator's 180 px mask is a few px looser than the physical bezel):

- **Update** (SCR_MENU → Update): row shape with a real pending version (ideally a beta string, e.g. `-beta.N`), then a forced failure (`Update failed` + error line) — C1/B1.
- **Netpick** (Wi-Fi → Change network): a long SSID row ellipsizes with no clipping at either end — B2.
- **Passkey**: type a wrong password, confirm "Wrong password, try again" shows whole in red — B3.
- **Adjust mode**: Back pill fully inside the bezel with visible margin — B4.
- **Night mode picker** with no clock / no zone so the note renders under the title — B5.
- **Wi-Fi → Change network confirm view**: body-to-Continue gap — B6.
- **Standby, face = Clock**: block centred; dots/clock/date; with an update pending, the 330 label — B7.
- **Connecting / degraded**: boot screen background matches the chassis colour with no black flash into the dial face; unplug the pad for the 4-line degraded case; check it at night for the ember inks — C2.
- **Discovery log line count on a real scan**: pull the pad off the network (or point Pad Address at a dead IP) and capture serial through the scan; expect only `scan: N candidates, pass 1`, `scan: pass 1 found nothing, pass 2`, `scan: no pad found` (or `pad found at …`) from `pad_discovery`, and no `esp-tls` / `transport_base` / `HTTP_CLIENT` lines during it. Also confirm a subsequent OTA check or pad request after the scan still logs its own errors normally (levels restored).
- **Scan timing**: unchanged by design (no probe/timeout/concurrency edits), but a real scan's wall-clock is the only proof that muting the log calls did not alter it.
