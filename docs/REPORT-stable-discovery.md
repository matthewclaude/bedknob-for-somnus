# REPORT: somnus-v1.0.4 stable release prep (wide-subnet pad discovery)

**Verdict: READY — 1.0.4.** The gate passes. PROJECT_VER is now "1.0.4", and CHANGELOG.md has a `## 1.0.4 — 2026-10-10` section. `idf.py build` and the host test both pass. Nothing is committed, tagged or pushed.

## Step 1: Beta tag

| | |
|---|---|
| Beta tag | `somnus-v1.0.4-beta.2` (lightweight; `git cat-file -t` = `commit`) |
| Commit SHA | `1598b27f4a5ffac8afd4172e4ebf60d3b0fef1bb` (`release: somnus-v1.0.4-beta.2 — pad discovery on networks wider than /24`) |
| Its CHANGELOG entry | `## 1.0.4-beta.2 — 2026-10-04 (beta)`: "The dial can now find your pad by itself on routers that use larger networks, such as eero mesh systems…" |
| On remote `somnus` | yes: `git ls-remote --tags somnus` lists `1598b27…  refs/tags/somnus-v1.0.4-beta.2` |

This is the newest `somnus-v*` tag (`git tag --list 'somnus-v*' --sort=-creatordate` puts it first). The discovery code itself is commit `4e46454`.

## Step 2: Gate check — PASS

```
$ git log --oneline somnus-v1.0.4-beta.2..main
94241fc docs: five version-bearing screens at 1.0.4-beta.2
```

```
$ git log --stat --format='%h %s' somnus-v1.0.4-beta.2..main
94241fc docs: five version-bearing screens at 1.0.4-beta.2

 docs/screens/about-wifi-real.png  | Bin 23571 -> 23661 bytes
 docs/screens/about-wifi-worst.png | Bin 24544 -> 24628 bytes
 docs/screens/about.png            | Bin 19348 -> 19608 bytes
 docs/screens/update-failed.png    | Bin 22343 -> 22488 bytes
 docs/screens/update.png           | Bin 23863 -> 24028 bytes
 5 files changed, 0 insertions(+), 0 deletions(-)
```

There is one commit after the tag, and it touches only simulator screenshots under docs/screens/. No firmware source changed. `somnus/main` is also `94241fc`, per `git ls-remote --heads somnus main`.

Stable tag check: `git tag --list 'somnus-v1.0.4'` printed nothing. `git ls-remote --tags somnus 'somnus-v1.0.4*'` lists only `-beta.1` and `-beta.2`. So `somnus-v1.0.4` does not exist locally or on the remote.

## Step 3: Changes

```
$ git diff -- CHANGELOG.md firmware/dial-idf/CMakeLists.txt
diff --git a/CHANGELOG.md b/CHANGELOG.md
index 902b6c5..a558c2a 100644
--- a/CHANGELOG.md
+++ b/CHANGELOG.md
@@ -22,6 +22,14 @@ Add the new section in the same commit that bumps `PROJECT_VER`.
 Releases marked **(beta)** are prereleases, visible only to dials with
 "Beta builds" turned on.
 
+## 1.0.4 — 2026-10-10
+
+The dial can now find your pad by itself on routers that use larger
+networks, such as eero mesh systems. Before, on those networks the dial
+gave up looking and you had to type the pad's address in by hand.
+
+Discovery on non-192.168.1.x subnets (e.g. eero 192.168.4.x) installed and ran on a field unit; the discovery path itself is not yet field-confirmed because OTA preserves the stored pad address.
+
 ## 1.0.4-beta.2 — 2026-10-04 (beta)
 
 The dial can now find your pad by itself on routers that use larger
diff --git a/firmware/dial-idf/CMakeLists.txt b/firmware/dial-idf/CMakeLists.txt
index c646b18..1e3fed1 100644
--- a/firmware/dial-idf/CMakeLists.txt
+++ b/firmware/dial-idf/CMakeLists.txt
@@ -18,5 +18,5 @@ add_compile_options("-Wno-format")
 # which would have made every device silently reject any 0.x release as a
 # downgrade (is_newer() has no concept of a renumbering, only "older").
 # Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
-set(PROJECT_VER "1.0.4-beta.2")
+set(PROJECT_VER "1.0.4")
 project(somnus-dial)
```

PROJECT_VER is read from one place only: `firmware/dial-idf/CMakeLists.txt:21`. There is no version header. `dial_ota.c` mentions PROJECT_VER only in comments, and at runtime it reads the app descriptor. The release.yml extraction awk, run locally with VER=1.0.4, returns exactly the new section and does not pick up `1.0.4-beta.*`.

## Step 4: Raw build output (unfiltered)

```
$ . ~/esp/esp-idf/export.sh && idf.py build
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
-- App "somnus-dial" version: 1.0.4
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
-- Configuring done (9.1s)
-- Generating done (1.3s)
-- Build files have been written to: /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
[  0%] Built target partition_table_bin
[  0%] Built target blank_ota_data
[  0%] Built target _project_elf_src
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target memory_ld_in_preprocess
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_https_ota
[  0%] Built target __idf_esp_http_server
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
[  8%] Built target __idf_wpa_supplicant
[ 67%] Built target __idf_esp_hal_clock
[ 71%] Built target __idf_esp_hal_gpspi
[  9%] Built target __idf_esp_netif
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 76%] Built target __idf_esp_hal_pmu
[ 79%] Built target __idf_esp_hal_usb
[ 15%] Built target __idf_lwip
[ 84%] Built target __idf_esp_hal_gpio
[ 15%] Built target __idf_vfs
[ 88%] Built target __idf_hal
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
[ 19%] Built target __idf_esp_driver_uart
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[100%] Built target bootloader_check_size
[ 19%] Built target __idf_esp_psram
[100%] Built target app
[ 19%] No install step for 'bootloader'
[ 19%] Built target __idf_esp_ringbuf
[ 19%] Completed 'bootloader'
[ 20%] Built target __idf_esp_timer
[ 21%] Built target bootloader
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
[ 54%] Built target __idf_dial_knob
[ 54%] Built target __idf_esp_hal_ledc
[ 54%] Built target __idf_esp_hal_twai
[ 54%] Built target __idf_dial_state
[ 54%] Built target __idf_esp_hal_lcd
[ 55%] Built target __idf_espressif__cjson
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_dial_time
[ 56%] Built target __idf_esp_driver_i2c
[ 56%] Built target __idf_esp_driver_spi
[ 57%] Built target __idf_unity
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_hal_pcnt
[ 58%] Built target __idf_esp_hal_rmt
[ 58%] Built target __idf_esp_driver_sdm
[ 58%] Built target __idf_esp_hal_mcpwm
[ 58%] Built target __idf_esp_driver_touch_sens
[ 58%] Built target __idf_esp_driver_twai
[ 59%] Built target __idf_sdmmc
[ 59%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_console
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_esp_hid
[ 60%] Built target __idf_protobuf-c
[ 60%] Built target __idf_rt
[ 61%] Built target __idf_wear_levelling
[ 61%] Built target __idf_perfmon
[ 62%] Built target __idf_esp_https_server
[ 63%] Built target __idf_spiffs
[ 63%] Built target __idf_esp_driver_ledc
[ 64%] Built target __idf_esp_lcd
[ 65%] Built target __idf_driver
[ 65%] Built target __idf_dial_ota
[ 65%] Built target __idf_dial_net
[ 65%] Built target __idf_dial_somnus
[ 65%] Built target __idf_cmock
[ 66%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_esp_driver_pcnt
[ 67%] Built target __idf_esp_driver_mcpwm
[ 67%] Built target __idf_esp_driver_sd_intf
[ 68%] Built target __idf_esp_driver_sdspi
[ 69%] Built target __idf_esp_driver_rmt
[ 70%] Built target __idf_protocomm
[ 70%] Built target __idf_espressif__esp_lcd_sh8601
[ 71%] Built target __idf_dial_pad_discovery
[ 71%] Built target __idf_esp_driver_sdmmc
[ 72%] Built target __idf_esp_local_ctrl
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
exit: 0
```

## Step 4: Raw host test output (unfiltered)

The only host test under firmware/ is `firmware/dial-idf/test/test_dial_rel.c`. I ran it from `firmware/dial-idf`, with the binary in the session scratchpad instead of /tmp:

```
$ cc -I components/dial_state -Wall -o /private/tmp/claude-501/-Users-matthew-Projects-somnus-waveshare-rotary-dial/39cc3bfa-37bf-445a-883b-e3d1f2853d02/scratchpad/test_dial_rel test/test_dial_rel.c -lm && /private/tmp/claude-501/-Users-matthew-Projects-somnus-waveshare-rotary-dial/39cc3bfa-37bf-445a-883b-e3d1f2853d02/scratchpad/test_dial_rel
all relative-scale table assertions passed
exit: 0
```

## git diff --stat

```
$ git diff --stat
 CHANGELOG.md                     | 8 ++++++++
 docs/REPORTS.md                  | 2 ++
 firmware/dial-idf/CMakeLists.txt | 2 +-
 3 files changed, 11 insertions(+), 1 deletion(-)
```

`docs/REPORTS.md` was already modified before this task (two 2026-10-04 index lines), and I did not touch it. My changes are CHANGELOG.md and firmware/dial-idf/CMakeLists.txt, plus this report, which is untracked. **There is no commit SHA because this task does not commit.**

## Key values

| | |
|---|---|
| Old PROJECT_VER | `1.0.4-beta.2` |
| New PROJECT_VER | `1.0.4` (confirmed in build log: `-- App "somnus-dial" version: 1.0.4`) |
| Stable tag name | `somnus-v1.0.4` |
| Files changed | `CHANGELOG.md` (+8), `firmware/dial-idf/CMakeLists.txt` (line 21) |
| Image size | `somnus-dial.bin` 0x189a40 bytes, 62 % of the app partition free |

## Deviations from this task

- The host test binary went to the session scratchpad instead of the `/tmp` path in the test file's header comment. The command is otherwise the same.
- For the gate, I also ran `git log --stat` alongside the required `git log --oneline` to show which files the commit touched.

Nothing else deviates from the task.

## Points for Matthew (no action taken)

1. **The stable notes cover only discovery.** The last stable release is 1.0.2 (`5512537`). Since then, three firmware changes have gone out only as betas: `fe5b139` (SCR_SIDEPICK removal and the Scale fix, 1.0.3-beta.1), `5e9bc69` (Local API hint, 1.0.4-beta.1) and `4e46454` (discovery, 1.0.4-beta.2). All three ship in 1.0.4. As instructed, the `## 1.0.4` section carries only the beta.2 entry, so a stable-channel user going from 1.0.2 to 1.0.4 won't read about the other two changes. The 1.0.2 stable entry also ended with an "Internal: … no code change beyond the version string" line, and this one doesn't have that line.
2. **The field-test line is in the public notes.** The added sentence will appear in the GitHub Release body, because release.yml publishes the section verbatim. It is also one long unwrapped line, while the rest of the file wraps at about 72 columns.
3. **Version screenshots are still at 1.0.4-beta.2.** The five version-bearing PNGs in docs/screens still say 1.0.4-beta.2. Earlier releases regenerated them in a follow-up docs commit, and this task didn't ask for that.
4. **Tag type.** The beta.2 tag is lightweight. Every tag from 0.1.6-beta.1 through 1.0.4-beta.1 was annotated. The commands below use an annotated tag; change them if you want a lightweight one.

## Not verifiable without hardware

- An OTA install of 1.0.4 on a real dial (the stable channel from 1.0.2, and beta.2 to 1.0.4).
- Pad discovery on a 192.168.4.x (eero /22) network. OTA keeps the stored pad address, so the discovery path only runs on a fresh setup or after the address is cleared.

## Commands for Matthew to run himself

    cd ~/Projects/somnus-waveshare-rotary-dial
    git add CHANGELOG.md
    git add firmware/dial-idf/CMakeLists.txt
    git diff --cached --stat
    git commit -m "release: somnus-v1.0.4 — wide-subnet pad discovery goes stable"
    git tag -a somnus-v1.0.4 -m "somnus-v1.0.4"
    git push somnus main
    git push somnus somnus-v1.0.4
    gh run list --repo matthewclaude/bedknob-for-somnus --limit 3

`git diff --cached --stat` should list exactly those two files. Push to `somnus` only, never to `origin`.
