# REPORT — Phase 1 layout build + wire flash (bench dial)

Date: 2026-09-04 22:15 CDT (machine clock; the capture file is named per the task, `2026-09-05-…`).
Branch `main`. Toolchain ESP-IDF v6.0 via `~/esp/esp-idf/export.sh` (the `get-idf` alias body).

## Verdict

**DONE — flashed and booting clean.** The tree at `5bf68a5` + the report commit was built from clean (zero warnings, 31 `dial_ui` objects compiled), wire-flashed to `/dev/cu.usbmodem83401` (all four segments hash-verified), and the 40 s boot capture shows App version 0.1.5, compile time one minute before the flash, boot from the app at 0x20000, pad connected, no panic / assert / abort / Guru / `E (` lines, no reboot. PROJECT_VER untouched, no tag, no push, no source edits. The eyes-on list is now the owner's.

## Step 0 — report commit

```
$ git add docs/REPORT-layout-phase1.md && git commit -m "docs: phase 1 layout report"
report commit: 4b114c52e19d8b38dbe7c9cd383e0c1140a13e94
```

## Gate

```
$ git --no-optional-locks status --short
(empty)
$ git log --oneline -3
4b114c5 docs: phase 1 layout report
5bf68a5 ui: layout fixes A1-A5 from the screen audit
a009840 sim: scenarios for the layout audit (S1-S9, timezone, night-face)
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")
```

## Step 1 — `idf.py build`

First attempt was a plain `idf.py build` on the existing `build/`: exit 0, but it recompiled nothing (the tree had been built at `5bf68a5` earlier), so its log could not show `dial_ui` warnings and the app descriptor still carried the earlier compile time (22:03:52). Re-ran as `idf.py fullclean && idf.py build` so both checks are real. Output below is from that clean build.

App version / size (from the build tail and `esptool image-info build/somnus-dial.bin`):

```
Project name: somnus-dial
App version: 0.1.5
Compile time: Sep  4 2026 22:15:24
ESP-IDF: v6.0
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
```

Last 40 lines:

```
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
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
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

Every warning line in the full build log (`grep -i warning`, CMake policy lines excluded): **none**. `dial_ui` warning lines: **none**. The three `NOTE:` lines about Kconfig bool defaults are ESP-IDF's own.

## Step 2 — port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

esptool identified it as ESP32-S3 (QFN56) rev v0.2, USB mode USB-Serial/JTAG, MAC 28:84:85:4b:d9:04 — the native port, not the CH340.

## Step 3 — `idf.py -p /dev/cu.usbmodem83401 flash` (raw, unfiltered)

```
Adding "flash"'s dependency "all" to list of commands with default set of options.
Executing action: all (aliases: build)
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
[  0%] Built target _project_elf_src
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target blank_ota_data
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
[  3%] Built target __idf_http_parser
[ 47%] Built target __idf_bootloader_support
[  3%] Built target __idf_esp_hal_i2c
[ 50%] Built target __idf_esp_security
[  3%] Built target __idf_esp_gdbstub
[ 52%] Built target __idf_esp_hal_security
[  3%] Built target __idf_esp_wifi
[ 56%] Built target __idf_esp_hal_ana_conv
[  3%] Built target __idf_esp_coex
[ 59%] Built target __idf_esp_hal_uart
[ 62%] Built target __idf_esp_hal_wdt
[ 64%] Built target __idf_esp_hal_timg
[ 65%] Built target __idf_esp_bootloader_format
[  8%] Built target __idf_wpa_supplicant
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[  9%] Built target __idf_esp_netif
[ 71%] Built target __idf_esp_hal_gpspi
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 76%] Built target __idf_esp_hal_pmu
[ 15%] Built target __idf_lwip
[ 79%] Built target __idf_esp_hal_usb
[ 15%] Built target __idf_vfs
[ 84%] Built target __idf_esp_hal_gpio
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 88%] Built target __idf_hal
[ 15%] Built target __idf_esp_phy
[ 93%] Built target __idf_soc
[ 95%] Built target __idf_xtensa
[ 16%] Built target __idf_nvs_flash
[ 97%] Built target __idf_main
[ 17%] Built target __idf_nvs_sec_provider
[ 98%] Built target bootloader.elf
[ 18%] Built target __idf_esp_event
[100%] Built target gen_bootloader_binary
[ 19%] Built target __idf_esp_driver_uart
[100%] Built target gen_project_binary
[ 19%] Built target __idf_esp_psram
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 19%] Built target __idf_esp_ringbuf
[100%] Built target bootloader_check_size
[ 20%] Built target __idf_esp_timer
[100%] Built target app
[ 20%] No install step for 'bootloader'
[ 21%] Built target __idf_cxx
[ 21%] Completed 'bootloader'
[ 21%] Built target __idf_pthread
[ 22%] Built target bootloader
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
[ 50%] Built target __idf_esp_app_format
[ 51%] Built target __idf_esp_hal_security
[ 52%] Built target __idf_esp_hal_mspi
[ 52%] Built target __idf_esp_hal_clock
[ 52%] Built target __idf_esp_hal_gpspi
[ 52%] Built target __idf_esp_hal_dma
[ 53%] Built target __idf_esp_stdio
[ 54%] Built target __idf_xtensa
[ 54%] Built target __idf_dial_time
[ 54%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_espressif__cjson
[ 55%] Built target __idf_dial_knob
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_esp_hal_lcd
[ 56%] Built target __idf_dial_state
[ 56%] Built target __idf_esp_hal_twai
[ 57%] Built target __idf_unity
[ 57%] Built target __idf_esp_driver_i2c
[ 57%] Built target __idf_esp_driver_spi
[ 57%] Built target __idf_esp_hal_mcpwm
[ 57%] Built target __idf_esp_hal_rmt
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_driver_tsens
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_hid
[ 59%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_hal_pcnt
[ 60%] Built target __idf_esp_driver_twai
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_protobuf-c
[ 61%] Built target __idf_perfmon
[ 61%] Built target __idf_rt
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_esp_driver_ledc
[ 62%] Built target __idf_dial_net
[ 62%] Built target __idf_dial_ota
[ 63%] Built target __idf_spiffs
[ 64%] Built target __idf_driver
[ 65%] Built target __idf_esp_lcd
[ 65%] Built target __idf_dial_somnus
[ 65%] Built target __idf_cmock
[ 66%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_esp_driver_pcnt
[ 67%] Built target __idf_esp_driver_mcpwm
[ 67%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_esp_eth
[ 68%] Built target __idf_esp_driver_rmt
[ 68%] Built target __idf_espressif__esp_lcd_sh8601
[ 69%] Built target __idf_esp_driver_sdspi
[ 70%] Built target __idf_protocomm
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
[100%] Built target __ldgen_output_sections.ld
[100%] Built target somnus-dial.elf
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target blank_ota_data
[  0%] Built target partition_table_bin
[  0%] Built target __idf_esp_https_ota
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_http_server
[  1%] Built target __idf_esp_http_client
[  1%] Built target bootloader_ld_in_preprocess
[  2%] Built target _project_elf_src
[  8%] Built target __idf_log
[  1%] Built target __idf_tcp_transport
[ 16%] Built target __idf_esp_rom
[  1%] Built target __idf_esp_driver_i2s
[ 18%] Built target __idf_esp_common
[  2%] Built target __idf_esp_adc
[ 25%] Built target __idf_esp_hw_support
[  3%] Built target __idf_esp-tls
[ 26%] Built target __idf_esp_system
[  3%] Built target __idf_http_parser
[ 32%] Built target __idf_efuse
[  3%] Built target __idf_esp_hal_i2c
[ 47%] Built target __idf_bootloader_support
[  3%] Built target __idf_esp_gdbstub
[ 50%] Built target __idf_esp_security
[  3%] Built target __idf_esp_wifi
[ 52%] Built target __idf_esp_hal_security
[  3%] Built target __idf_esp_coex
[ 56%] Built target __idf_esp_hal_ana_conv
[ 59%] Built target __idf_esp_hal_uart
[  8%] Built target __idf_wpa_supplicant
[ 62%] Built target __idf_esp_hal_wdt
[  9%] Built target __idf_esp_netif
[ 64%] Built target __idf_esp_hal_timg
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 15%] Built target __idf_lwip
[ 67%] Built target __idf_esp_hal_clock
[ 15%] Built target __idf_vfs
[ 71%] Built target __idf_esp_hal_gpspi
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Built target __idf_esp_hal_dma
[ 15%] Built target __idf_esp_phy
[ 75%] Built target __idf_micro-ecc
[ 16%] Built target __idf_nvs_flash
[ 76%] Built target __idf_esp_hal_pmu
[ 17%] Built target __idf_nvs_sec_provider
[ 79%] Built target __idf_esp_hal_usb
[ 18%] Built target __idf_esp_event
[ 84%] Built target __idf_esp_hal_gpio
[ 19%] Built target __idf_esp_driver_uart
[ 88%] Built target __idf_hal
[ 19%] Built target __idf_esp_psram
[ 93%] Built target __idf_soc
[ 19%] Built target __idf_esp_ringbuf
[ 95%] Built target __idf_xtensa
[ 20%] Built target __idf_esp_timer
[ 97%] Built target __idf_main
[ 21%] Built target __idf_cxx
[ 98%] Built target bootloader.elf
[ 21%] Built target __idf_pthread
[100%] Built target gen_bootloader_binary
[ 22%] Built target __idf_esp_libc
[100%] Built target gen_project_binary
[ 24%] Built target __idf_freertos
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[100%] Built target bootloader_check_size
[ 27%] Built target __idf_esp_hw_support
[100%] Built target app
[ 28%] Built target __idf_esp_hal_i2s
[ 28%] No install step for 'bootloader'
[ 28%] Built target __idf_esp_hal_touch_sens
[ 28%] Completed 'bootloader'
[ 28%] Built target __idf_esp_hal_pmu
[ 29%] Built target bootloader
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
[ 50%] Built target __idf_esp_app_format
[ 51%] Built target __idf_esp_hal_security
[ 52%] Built target __idf_esp_hal_mspi
[ 52%] Built target __idf_esp_hal_clock
[ 52%] Built target __idf_esp_hal_gpspi
[ 52%] Built target __idf_esp_hal_dma
[ 53%] Built target __idf_esp_stdio
[ 54%] Built target __idf_xtensa
[ 54%] Built target __idf_esp_hal_ledc
[ 54%] Built target __idf_esp_hal_twai
[ 55%] Built target __idf_esp_driver_i2c
[ 55%] Built target __idf_dial_time
[ 55%] Built target __idf_espressif__cjson
[ 55%] Built target __idf_dial_state
[ 55%] Built target __idf_esp_hal_lcd
[ 55%] Built target __idf_dial_knob
[ 56%] Built target __idf_unity
[ 56%] Built target __idf_esp_driver_spi
[ 57%] Built target __idf_esp_driver_gptimer
[ 58%] Built target __idf_console
[ 59%] Built target __idf_esp_hal_cam
[ 59%] Built target __idf_esp_hal_rmt
[ 59%] Built target __idf_esp_hal_pcnt
[ 59%] Built target __idf_esp_driver_touch_sens
[ 59%] Built target __idf_esp_driver_twai
[ 59%] Built target __idf_esp_hal_mcpwm
[ 59%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_esp_driver_sdm
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_protobuf-c
[ 61%] Built target __idf_esp_hid
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_rt
[ 63%] Built target __idf_spiffs
[ 63%] Built target __idf_perfmon
[ 64%] Built target __idf_driver
[ 64%] Built target __idf_dial_ota
[ 64%] Built target __idf_esp_driver_ledc
[ 65%] Built target __idf_esp_lcd
[ 65%] Built target __idf_cmock
[ 65%] Built target __idf_dial_net
[ 65%] Built target __idf_dial_somnus
[ 66%] Built target __idf_esp_driver_cam
[ 67%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_esp_driver_mcpwm
[ 68%] Built target __idf_esp_driver_sd_intf
[ 68%] Built target __idf_esp_driver_pcnt
[ 69%] Built target __idf_esp_driver_rmt
[ 69%] Built target __idf_espressif__esp_lcd_sh8601
[ 70%] Built target __idf_protocomm
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
[100%] Built target __ldgen_output_sections.ld
[100%] Built target somnus-dial.elf
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189150 bytes. Smallest app partition is 0x400000 bytes. 0x276eb0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app
esptool --chip esp32s3 -p /dev/cu.usbmodem83401 -b 460800 --before=default-reset --after=hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x19000 ota_data_initial.bin 0x20000 somnus-dial.bin
esptool v5.3.1
Serial port /dev/cu.usbmodem83401:
Connecting...
[1A[2K[1A[2KConnected to ESP32-S3 on /dev/cu.usbmodem83401:
Chip type:          ESP32-S3 (QFN56) (revision v0.2)
Features:           Wi-Fi, BT 5 (LE), Dual Core + LP Core, 240MHz, Embedded PSRAM 8MB (AP_3v3)
Crystal frequency:  40MHz
USB mode:           USB-Serial/JTAG
MAC:                28:84:85:4b:d9:04

Uploading stub flasher...
Running stub flasher...
[1A[2K[1A[2KStub flasher running.
Changing baud rate to 460800...
Changed.

Configuring flash size...

Writing 'bootloader/bootloader.bin' at 0x00000000...
SHA digest in image updated.
Flash will be erased from 0x00000000 to 0x00005fff...
Compressed 22576 bytes to 14462...
[KWriting at 0x00000000 [                              ]   0.0% 0/14462 bytes... [KWriting at 0x00005830 [==============================] 100.0% 14462/14462 bytes... 
[1A[2K[1A[2KWrote 22576 bytes (14462 compressed) at 0x00000000 in 0.2 seconds (729.8 kbit/s).
Verifying written data...
[1A[2KHash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...
[KWriting at 0x00008000 [                              ]   0.0% 0/141 bytes... [KWriting at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
[1A[2K[1A[2KWrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (691.9 kbit/s).
Verifying written data...
[1A[2KHash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...
[KWriting at 0x00019000 [                              ]   0.0% 0/31 bytes... [KWriting at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
[1A[2K[1A[2KWrote 8192 bytes (31 compressed) at 0x00019000 in 0.1 seconds (980.5 kbit/s).
Verifying written data...
[1A[2KHash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1610064 bytes to 982952...
[KWriting at 0x00020000 [                              ]   0.0% 0/982952 bytes... [KWriting at 0x0002b919 [                              ]   1.7% 16384/982952 bytes... [KWriting at 0x00038592 [>                             ]   3.3% 32768/982952 bytes... [KWriting at 0x0004b764 [>                             ]   5.0% 49152/982952 bytes... [KWriting at 0x000531eb [=>                            ]   6.7% 65536/982952 bytes... [KWriting at 0x0005ba4b [=>                            ]   8.3% 81920/982952 bytes... [KWriting at 0x000652f5 [==>                           ]  10.0% 98304/982952 bytes... [KWriting at 0x00070dab [==>                           ]  11.7% 114688/982952 bytes... [KWriting at 0x00083bb1 [===>                          ]  13.3% 131072/982952 bytes... [KWriting at 0x0008a7aa [===>                          ]  15.0% 147456/982952 bytes... [KWriting at 0x00093b60 [====>                         ]  16.7% 163840/982952 bytes... [KWriting at 0x0009c1bc [====>                         ]  18.3% 180224/982952 bytes... [KWriting at 0x000a4804 [=====>                        ]  20.0% 196608/982952 bytes... [KWriting at 0x000aa103 [=====>                        ]  21.7% 212992/982952 bytes... [KWriting at 0x000af98e [======>                       ]  23.3% 229376/982952 bytes... [KWriting at 0x000b4e55 [======>                       ]  25.0% 245760/982952 bytes... [KWriting at 0x000bac35 [=======>                      ]  26.7% 262144/982952 bytes... [KWriting at 0x000c00e3 [=======>                      ]  28.3% 278528/982952 bytes... [KWriting at 0x000c5ac2 [========>                     ]  30.0% 294912/982952 bytes... [KWriting at 0x000caabc [========>                     ]  31.7% 311296/982952 bytes... [KWriting at 0x000d0049 [=========>                    ]  33.3% 327680/982952 bytes... [KWriting at 0x000d65b5 [=========>                    ]  35.0% 344064/982952 bytes... [KWriting at 0x000db95e [==========>                   ]  36.7% 360448/982952 bytes... [KWriting at 0x000e138f [==========>                   ]  38.3% 376832/982952 bytes... [KWriting at 0x000e643c [===========>                  ]  40.0% 393216/982952 bytes... [KWriting at 0x000eb91b [===========>                  ]  41.7% 409600/982952 bytes... [KWriting at 0x000f0fe2 [============>                 ]  43.3% 425984/982952 bytes... [KWriting at 0x000f6cba [============>                 ]  45.0% 442368/982952 bytes... [KWriting at 0x000fbe6c [=============>                ]  46.7% 458752/982952 bytes... [KWriting at 0x0010126c [=============>                ]  48.3% 475136/982952 bytes... [KWriting at 0x0010620a [==============>               ]  50.0% 491520/982952 bytes... [KWriting at 0x0010b321 [==============>               ]  51.7% 507904/982952 bytes... [KWriting at 0x00110cf2 [===============>              ]  53.3% 524288/982952 bytes... [KWriting at 0x00116267 [===============>              ]  55.0% 540672/982952 bytes... [KWriting at 0x0011b9c1 [================>             ]  56.7% 557056/982952 bytes... [KWriting at 0x00120e8b [================>             ]  58.3% 573440/982952 bytes... [KWriting at 0x0012646b [=================>            ]  60.0% 589824/982952 bytes... [KWriting at 0x0012beda [=================>            ]  61.7% 606208/982952 bytes... [KWriting at 0x001313a8 [==================>           ]  63.3% 622592/982952 bytes... [KWriting at 0x001364be [==================>           ]  65.0% 638976/982952 bytes... [KWriting at 0x0013b714 [===================>          ]  66.7% 655360/982952 bytes... [KWriting at 0x001410ff [===================>          ]  68.3% 671744/982952 bytes... [KWriting at 0x0014664d [====================>         ]  70.0% 688128/982952 bytes... [KWriting at 0x0014bb49 [====================>         ]  71.7% 704512/982952 bytes... [KWriting at 0x0015108b [=====================>        ]  73.3% 720896/982952 bytes... [KWriting at 0x00156b0e [=====================>        ]  75.0% 737280/982952 bytes... [KWriting at 0x0015c2d9 [======================>       ]  76.7% 753664/982952 bytes... [KWriting at 0x001613d6 [======================>       ]  78.3% 770048/982952 bytes... [KWriting at 0x001665c9 [=======================>      ]  80.0% 786432/982952 bytes... [KWriting at 0x0016bf31 [=======================>      ]  81.7% 802816/982952 bytes... [KWriting at 0x00171127 [========================>     ]  83.3% 819200/982952 bytes... [KWriting at 0x001762c3 [========================>     ]  85.0% 835584/982952 bytes... [KWriting at 0x0017b693 [=========================>    ]  86.7% 851968/982952 bytes... [KWriting at 0x001814ee [=========================>    ]  88.3% 868352/982952 bytes... [KWriting at 0x00186c0b [==========================>   ]  90.0% 884736/982952 bytes... [KWriting at 0x0018d66c [==========================>   ]  91.7% 901120/982952 bytes... [KWriting at 0x001928d1 [===========================>  ]  93.3% 917504/982952 bytes... [KWriting at 0x00197f58 [===========================>  ]  95.0% 933888/982952 bytes... [KWriting at 0x0019e39c [============================> ]  96.7% 950272/982952 bytes... [KWriting at 0x001a376e [============================> ]  98.3% 966656/982952 bytes... [KWriting at 0x001a9150 [==============================] 100.0% 982952/982952 bytes... 
[1A[2K[1A[2KWrote 1610064 bytes (982952 compressed) at 0x00020000 in 10.3 seconds (1248.9 kbit/s).
Verifying written data...
[1A[2KHash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

Segments written and verified: bootloader @0x0, partition table @0x8000, otadata @0x19000, app @0x20000 — **4 × "Hash of data verified."** (NVS at 0x9000 not written; Wi-Fi/pad/timezone survived, see the boot log.)

## Step 4 — boot capture

Raw pyserial read on `/dev/cu.usbmodem83401` at 115200 for 40 s, started right after the flash returned (the port drops for a moment on the post-flash reset; the reader reopens it). Full capture: **`bench-logs/2026-09-05-flash-layout-phase1.log`** (6785 bytes, 128 lines; gitignored, not committed).

Quoted lines:

```
I (379) boot: Loaded app from partition at offset 0x20000
I (379) boot: Set actual ota_seq=1 in otadata[0]
I (785) app_init: Project name:     somnus-dial
I (789) app_init: App version:      0.1.5
I (793) app_init: Compile time:     Sep  4 2026 22:15:24
I (802) app_init: ESP-IDF:          v6.0
I (1212) ota: boot pending-verify: false
I (3085) dial_somnus: zone mode set to single (One Bed)
I (3085) app: pad connected at http://192.168.1.169:8080
I (3907) ota: boot not pending verification (state 2) -- nothing to do
```

- Partition booted: the app at offset **0x20000** (ota_0, `ota_seq=1`).
- First `dial_somnus` line: `I (3085) dial_somnus: zone mode set to single (One Bed)`, followed on the same tick by the pad-connected line.
- Rollback: a wire flash never boots pending-verify, so there is no "app marked valid" line; the equivalent is `boot pending-verify: false` + `boot not pending verification (state 2) -- nothing to do`.
- Lines containing `assert`, `panic`, `abort`, `Guru`, `E (`, `rollback` (case-insensitive): **none** (the only case-insensitive `e (` hit is "single (One Bed)").
- Reset lines (`rst:`) in the capture: **0** — no reboot loop. The capture starts mid-bootloader (segment 3 of the image load) because the reader attached ~0.3 s into the boot.
- `W` lines present, none in the requested pattern set: `sh8601: The 36h command has been used…` (the panel driver's usual init note), `wifi:Password length matches WPA2 standards…`, `ledc: LEDC FADE TOO SLOW` ×2 (brightness fade, seen on every boot).
- Also seen: Wi-Fi up at 1451 ms, IP at 2505 ms, TZ restored from NVS, SNTP synced at 4166 ms, `power: night mode on`, `power: plugged (4798 mV)`, side A polls every ~10 s.

## Step 5

No panic, no reboot loop; nothing to do.

## Git

```
$ git log --oneline -3
4b114c5 docs: phase 1 layout report
5bf68a5 ui: layout fixes A1-A5 from the screen audit
a009840 sim: scenarios for the layout audit (S1-S9, timezone, night-face)
```

Report commit: `4b114c52e19d8b38dbe7c9cd383e0c1140a13e94`. Working tree after this task: only this file (`docs/REPORT-layout-phase1-flash.md`) untracked; `bench-logs/` is gitignored. No tag, no push.

## Deviations from the spec

1. **`idf.py fullclean` before the build.** The spec says `idf.py build`; the incremental build compiled zero objects, which made "zero warnings from components/dial_ui" unprovable and left a stale compile time in the app descriptor. The clean rebuild is the same source, same version, and gives the spec's "compile time within the last few minutes" (22:15:24 vs flash at ~22:16).
2. **Commit message trailers.** The report commit carries the session's two attribution trailer lines after the spec's title; the title is verbatim.
3. **"app marked valid" line absent** — not a deviation in what was done, but the expected line does not exist for a wire flash; the state-2 line above is the equivalent.
4. **Capture filename date.** Named `2026-09-05-…` as instructed although the machine clock read 2026-09-04 22:16 CDT.

Everything else: none.

## Not verified (the eyes-on list is the owner's)

- Settings → Units → °C, then turn the knob across the range (A1: unit follows the numeral; "20.0 °C" clear).
- Settings with Night mode on and no timezone set (A2: two-line Night mode row with the "set timezone" annotation) — note this bench dial has a TZ persisted (`CST6CDT…` restored from NVS) and a synced clock, so the annotation will not appear on it without clearing the zone.
- Settings and Wi-Fi lists scrolled to their far ends (A5: far row inside the bezel at zoom 140; the physical bezel covers a few px more than the simulator's mask).
- Side pick via Settings (A4: title visible at y 72). Reminder from the phase-1 report: Settings has no "My side" row any more, so reaching SCR_SIDEPICK on hardware needs a fresh-device path (side_picked cleared) rather than a Settings tap.
- An available update on the face (A3: "Update available" 2 px clear of the dots) — needs a newer release to exist, or the beta channel with a newer pre-release.
- The +15 LEVEL vs chassis-ring residual recorded in the phase-1 report (Settings → Scale → Relative, knob to the +15 rail).
