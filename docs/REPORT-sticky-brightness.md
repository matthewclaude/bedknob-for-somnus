# REPORT — Brightness percent picker sticky against poll commits

Date: 2026-09-05 19:03 CDT (capture named `2026-09-06-…` per the task). Branch `main`. Toolchain ESP-IDF v6.0.

## Verdict

**DONE — `eef9257`: `SCR_BRIGHTNESS` added to `nav_policy()`'s have-state sticky set next to `SCR_BRIGHTNESS_MENU` with a one-line comment; incremental `idf.py build` recompiled `main.c` only with zero warning lines; wire-flashed; 40 s boot capture clean (App version 0.1.6-beta.1, no assert/panic/abort/Guru/`E (` lines, no reset).** Step 0 committed the sticky-night report as `9d0d785`. Re-check: every screen in the previous report's table except `SCR_SIDEPICK` is now in **both** sticky sets (statement below). No tag, no version bump, no push.

## Gate

```
$ git --no-optional-locks status --short
?? docs/REPORT-sticky-night-pickers.md
$ git log --oneline -1
d80f66e fix(nav): Night mode and Night face pickers are sticky against poll commits
```

Exactly the required entry and HEAD; proceeded.

## Step 0

```
$ git add docs/REPORT-sticky-night-pickers.md && git commit -m "docs: sticky night pickers report"
9d0d785 docs: sticky night pickers report
```

## The diff (`git diff 9d0d785 eef9257`)

```diff
diff --git a/firmware/dial-idf/main/main.c b/firmware/dial-idf/main/main.c
index 0ef4ef9..ab11c64 100644
--- a/firmware/dial-idf/main/main.c
+++ b/firmware/dial-idf/main/main.c
@@ -404,8 +404,12 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
             // the same category, never listed here or below — the same
             // class of gap the 2026-09-01 audit closed for Timezone and
             // Adjust mode.
+            // SCR_BRIGHTNESS (the percent picker under Brightness menu) was
+            // in the no-state set below but not here — same gap, closed
+            // 2026-09-05 (docs/REPORT-sticky-night-pickers.md's table).
             if (passive || cur == SCR_SETTINGS ||
-                cur == SCR_BRIGHTNESS_MENU || cur == SCR_ADJUST_MODE ||
+                cur == SCR_BRIGHTNESS_MENU || cur == SCR_BRIGHTNESS ||
+                cur == SCR_ADJUST_MODE ||
                 cur == SCR_PAD_ADDRESS || cur == SCR_TIMEZONE ||
                 cur == SCR_STANDBY_FACE ||
                 cur == SCR_NIGHT_MODE || cur == SCR_NIGHT_FACE) return cur;
```

One membership added, one comment. Nothing else.

## Membership re-check (post-fix, `main.c` line numbers)

have-state set, lines 410–415 (`if (passive || cur == …) return cur;`, `passive` = MENU / WIFI / ABOUT / UPDATE at 376–377):
`SCR_SETTINGS`, `SCR_BRIGHTNESS_MENU`, **`SCR_BRIGHTNESS`**, `SCR_ADJUST_MODE`, `SCR_PAD_ADDRESS`, `SCR_TIMEZONE`, `SCR_STANDBY_FACE`, `SCR_NIGHT_MODE`, `SCR_NIGHT_FACE`, plus the four passive screens.

no-state set, lines 442–448:
`SCR_MENU`, `SCR_SETTINGS`, `SCR_ABOUT`, `SCR_WIFI`, `SCR_BRIGHTNESS`, `SCR_BRIGHTNESS_MENU`, `SCR_UPDATE`, `SCR_PAD_ADDRESS`, `SCR_TIMEZONE`, `SCR_ADJUST_MODE`, `SCR_STANDBY_FACE`, `SCR_NIGHT_MODE`, `SCR_NIGHT_FACE`.

**Statement:** every screen in `docs/REPORT-sticky-night-pickers.md`'s membership table — `SCR_MENU`, `SCR_SETTINGS`, `SCR_BRIGHTNESS_MENU`, `SCR_BRIGHTNESS`, `SCR_NIGHT_MODE`, `SCR_NIGHT_FACE`, `SCR_STANDBY_FACE`, `SCR_TIMEZONE`, `SCR_PAD_ADDRESS`, `SCR_ADJUST_MODE`, `SCR_WIFI`, `SCR_ABOUT`, `SCR_UPDATE` — is now in both sets. The one exception is `SCR_SIDEPICK`, exactly as the task allows: it is the fresh-device flow, kept by its own clause in the have-state branch (`cur == SCR_SIDEPICK`, line ~421) and deliberately not sticky with no device state, where the connect/error screen is the right thing to show. The two sets are otherwise identical in content (the have-state set spells the four passive screens through the `passive` flag). No gap remains in this class.

## `idf.py build` — raw tail (last 40 lines)

Incremental. Exit 0. One object rebuilt: `main.c.obj` (log line 174), then re-link and re-image.

```
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Built target __idf_dial_power
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
somnus-dial.bin binary size 0x189770 bytes. Smallest app partition is 0x400000 bytes. 0x276890 bytes (62%) free.
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

Every line containing "warning" (case-insensitive): **0** — none (an incremental build does not re-run CMake, so even the usual cmake_minimum_required note is absent).

App descriptor: App version **0.1.6-beta.1**, Compile time **Sep  5 2026 18:46:18** — unchanged since commit 2's clean build for the reason recorded in the previous report (`__TIME__` lives in `esp_app_desc.c`, not rebuilt by a `main.c`-only build); the elf/bin were regenerated (tail above).

## Port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

Native USB-Serial/JTAG only — no "flip the plug".

## `idf.py -p /dev/cu.usbmodem83401 flash` — raw output

Exit 0. Nothing recompiled by the flash step. Warning lines: none. From `Executing action: flash`; esptool's terminal control sequences and in-place progress rewrites stripped.

```
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target blank_ota_data
[  0%] Built target _project_elf_src
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target partition_table_bin
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
[  2%] Built target __idf_esp_adc
[ 26%] Built target __idf_esp_system
[  2%] Built target __idf_esp-tls
[ 32%] Built target __idf_efuse
[  2%] Built target __idf_http_parser
[ 47%] Built target __idf_bootloader_support
[  2%] Built target __idf_esp_hal_i2c
[ 50%] Built target __idf_esp_security
[  2%] Built target __idf_esp_gdbstub
[ 52%] Built target __idf_esp_hal_security
[  2%] Built target __idf_esp_wifi
[ 56%] Built target __idf_esp_hal_ana_conv
[  2%] Built target __idf_esp_coex
[ 59%] Built target __idf_esp_hal_uart
[ 62%] Built target __idf_esp_hal_wdt
[  7%] Built target __idf_wpa_supplicant
[ 64%] Built target __idf_esp_hal_timg
[  8%] Built target __idf_esp_netif
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 67%] Built target __idf_esp_hal_clock
[ 14%] Built target __idf_lwip
[ 71%] Built target __idf_esp_hal_gpspi
[ 14%] Built target __idf_vfs
[ 14%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 14%] Built target __idf_esp_phy
[ 76%] Built target __idf_esp_hal_pmu
[ 15%] Built target __idf_nvs_flash
[ 79%] Built target __idf_esp_hal_usb
[ 16%] Built target __idf_nvs_sec_provider
[ 84%] Built target __idf_esp_hal_gpio
[ 17%] Built target __idf_esp_event
[ 88%] Built target __idf_hal
[ 18%] Built target __idf_esp_driver_uart
[ 93%] Built target __idf_soc
[ 18%] Built target __idf_esp_psram
[ 95%] Built target __idf_xtensa
[ 18%] Built target __idf_esp_ringbuf
[ 97%] Built target __idf_main
[ 19%] Built target __idf_esp_timer
[ 98%] Built target bootloader.elf
[ 20%] Built target __idf_cxx
[100%] Built target gen_bootloader_binary
[ 20%] Built target __idf_pthread
[100%] Built target gen_project_binary
[ 21%] Built target __idf_esp_libc
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 23%] Built target __idf_freertos
[100%] Built target bootloader_check_size
[100%] Built target app
[ 26%] Built target __idf_esp_hw_support
[ 26%] No install step for 'bootloader'
[ 27%] Built target __idf_esp_hal_i2s
[ 27%] Completed 'bootloader'
[ 27%] Built target __idf_esp_hal_touch_sens
[ 28%] Built target bootloader
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
[ 51%] Built target __idf_esp_app_format
[ 52%] Built target __idf_esp_hal_security
[ 53%] Built target __idf_esp_hal_mspi
[ 53%] Built target __idf_esp_hal_clock
[ 53%] Built target __idf_esp_hal_gpspi
[ 53%] Built target __idf_esp_hal_dma
[ 54%] Built target __idf_esp_stdio
[ 55%] Built target __idf_xtensa
[ 55%] Built target __idf_dial_knob
[ 55%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_espressif__cjson
[ 56%] Built target __idf_esp_hal_lcd
[ 56%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_dial_time
[ 56%] Built target __idf_esp_hal_ledc
[ 56%] Built target __idf_dial_state
[ 56%] Built target __idf_esp_driver_i2c
[ 57%] Built target __idf_esp_driver_gptimer
[ 58%] Built target __idf_unity
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_hal_cam
[ 59%] Built target __idf_esp_hal_mcpwm
[ 59%] Built target __idf_esp_hal_rmt
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_esp_hal_pcnt
[ 60%] Built target __idf_esp_driver_sdm
[ 60%] Built target __idf_esp_driver_twai
[ 60%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_esp_eth
[ 61%] Built target __idf_esp_hid
[ 61%] Built target __idf_perfmon
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_protobuf-c
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_rt
[ 62%] Built target __idf_driver
[ 62%] Built target __idf_dial_ota
[ 62%] Built target __idf_esp_driver_ledc
[ 63%] Built target __idf_spiffs
[ 64%] Built target __idf_esp_lcd
[ 64%] Built target __idf_dial_net
[ 65%] Built target __idf_esp_driver_cam
[ 65%] Built target __idf_cmock
[ 66%] Built target __idf_dial_somnus
[ 66%] Built target __idf_esp_driver_mcpwm
[ 66%] Built target __idf_esp_driver_pcnt
[ 66%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_esp_driver_rmt
[ 68%] Built target __idf_espressif__esp_lcd_sh8601
[ 69%] Built target __idf_protocomm
[ 70%] Built target __idf_dial_pad_discovery
[ 70%] Built target __idf_esp_driver_sdmmc
[ 71%] Built target __idf_esp_local_ctrl
[ 71%] Built target __idf_fatfs
[ 96%] Built target __idf_lvgl__lvgl
[ 96%] Built target __idf_dial_display
[ 96%] Built target __idf_lcd_bl_pwm_bsp
[ 96%] Built target __idf_lcd_touch_bsp
[ 96%] Built target __idf_i2c_bsp
[ 96%] Built target __idf_dial_haptics
[ 96%] Built target __idf_dial_power
[100%] Built target __idf_dial_ui
[100%] Built target __idf_main
[100%] Built target __ldgen_output_sections.ld
[100%] Built target somnus-dial.elf
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189770 bytes. Smallest app partition is 0x400000 bytes. 0x276890 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app
esptool --chip esp32s3 -p /dev/cu.usbmodem83401 -b 460800 --before=default-reset --after=hard-reset write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 bootloader/bootloader.bin 0x8000 partition_table/partition-table.bin 0x19000 ota_data_initial.bin 0x20000 somnus-dial.bin
esptool v5.3.1
Serial port /dev/cu.usbmodem83401:
Connecting...
Connected to ESP32-S3 on /dev/cu.usbmodem83401:
Chip type:          ESP32-S3 (QFN56) (revision v0.2)
Features:           Wi-Fi, BT 5 (LE), Dual Core + LP Core, 240MHz, Embedded PSRAM 8MB (AP_3v3)
Crystal frequency:  40MHz
USB mode:           USB-Serial/JTAG
MAC:                28:84:85:4b:d9:04

Uploading stub flasher...
Running stub flasher...
Stub flasher running.
Changing baud rate to 460800...
Changed.

Configuring flash size...

Writing 'bootloader/bootloader.bin' at 0x00000000...
SHA digest in image updated.
Flash will be erased from 0x00000000 to 0x00005fff...
Compressed 22576 bytes to 14461...

Writing at 0x00000000 [                              ]   0.0% 0/14461 bytes... 
Writing at 0x00005830 [==============================] 100.0% 14461/14461 bytes... 
Wrote 22576 bytes (14461 compressed) at 0x00000000 in 0.3 seconds (701.9 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (764.0 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.0 seconds (1508.4 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1611632 bytes to 983989...

Writing at 0x00020000 [                              ]   0.0% 0/983989 bytes... 
Writing at 0x0002b8f7 [                              ]   1.7% 16384/983989 bytes... 
Writing at 0x0003857d [                              ]   3.3% 32768/983989 bytes... 
Writing at 0x0004b6d0 [>                             ]   5.0% 49152/983989 bytes... 
Writing at 0x00053208 [>                             ]   6.7% 65536/983989 bytes... 
Writing at 0x0005ba1d [=>                            ]   8.3% 81920/983989 bytes... 
Writing at 0x00065284 [=>                            ]  10.0% 98304/983989 bytes... 
Writing at 0x00070d12 [==>                           ]  11.7% 114688/983989 bytes... 
Writing at 0x00083aa9 [==>                           ]  13.3% 131072/983989 bytes... 
Writing at 0x0008a7dc [===>                          ]  15.0% 147456/983989 bytes... 
Writing at 0x00093b97 [===>                          ]  16.7% 163840/983989 bytes... 
Writing at 0x0009c1d6 [====>                         ]  18.3% 180224/983989 bytes... 
Writing at 0x000a47ef [====>                         ]  20.0% 196608/983989 bytes... 
Writing at 0x000aa0fc [=====>                        ]  21.6% 212992/983989 bytes... 
Writing at 0x000af97d [=====>                        ]  23.3% 229376/983989 bytes... 
Writing at 0x000b4e3a [======>                       ]  25.0% 245760/983989 bytes... 
Writing at 0x000bacfd [======>                       ]  26.6% 262144/983989 bytes... 
Writing at 0x000c019f [=======>                      ]  28.3% 278528/983989 bytes... 
Writing at 0x000c5b9e [=======>                      ]  30.0% 294912/983989 bytes... 
Writing at 0x000cabdf [========>                     ]  31.6% 311296/983989 bytes... 
Writing at 0x000d0147 [========>                     ]  33.3% 327680/983989 bytes... 
Writing at 0x000d639c [=========>                    ]  35.0% 344064/983989 bytes... 
Writing at 0x000dba38 [=========>                    ]  36.6% 360448/983989 bytes... 
Writing at 0x000e1468 [==========>                   ]  38.3% 376832/983989 bytes... 
Writing at 0x000e6524 [==========>                   ]  40.0% 393216/983989 bytes... 
Writing at 0x000eb9a0 [===========>                  ]  41.6% 409600/983989 bytes... 
Writing at 0x000f1090 [===========>                  ]  43.3% 425984/983989 bytes... 
Writing at 0x000f6daf [============>                 ]  45.0% 442368/983989 bytes... 
Writing at 0x000fbf92 [============>                 ]  46.6% 458752/983989 bytes... 
Writing at 0x001012f7 [=============>                ]  48.3% 475136/983989 bytes... 
Writing at 0x00106298 [=============>                ]  50.0% 491520/983989 bytes... 
Writing at 0x0010b3fb [==============>               ]  51.6% 507904/983989 bytes... 
Writing at 0x00110d33 [==============>               ]  53.3% 524288/983989 bytes... 
Writing at 0x001163a9 [===============>              ]  54.9% 540672/983989 bytes... 
Writing at 0x0011baa2 [===============>              ]  56.6% 557056/983989 bytes... 
Writing at 0x00120f7b [================>             ]  58.3% 573440/983989 bytes... 
Writing at 0x0012645a [================>             ]  59.9% 589824/983989 bytes... 
Writing at 0x0012bf61 [=================>            ]  61.6% 606208/983989 bytes... 
Writing at 0x0013141c [=================>            ]  63.3% 622592/983989 bytes... 
Writing at 0x001365e9 [==================>           ]  64.9% 638976/983989 bytes... 
Writing at 0x0013b716 [==================>           ]  66.6% 655360/983989 bytes... 
Writing at 0x00140f5d [===================>          ]  68.3% 671744/983989 bytes... 
Writing at 0x001466f2 [===================>          ]  69.9% 688128/983989 bytes... 
Writing at 0x0014bc20 [====================>         ]  71.6% 704512/983989 bytes... 
Writing at 0x00151139 [====================>         ]  73.3% 720896/983989 bytes... 
Writing at 0x001569b7 [=====================>        ]  74.9% 737280/983989 bytes... 
Writing at 0x0015c379 [=====================>        ]  76.6% 753664/983989 bytes... 
Writing at 0x00161460 [======================>       ]  78.3% 770048/983989 bytes... 
Writing at 0x00166694 [======================>       ]  79.9% 786432/983989 bytes... 
Writing at 0x0016bf6f [=======================>      ]  81.6% 802816/983989 bytes... 
Writing at 0x00171200 [=======================>      ]  83.3% 819200/983989 bytes... 
Writing at 0x00176369 [========================>     ]  84.9% 835584/983989 bytes... 
Writing at 0x0017b6e9 [========================>     ]  86.6% 851968/983989 bytes... 
Writing at 0x001814c0 [=========================>    ]  88.2% 868352/983989 bytes... 
Writing at 0x00186abb [=========================>    ]  89.9% 884736/983989 bytes... 
Writing at 0x0018d6e5 [==========================>   ]  91.6% 901120/983989 bytes... 
Writing at 0x00192a25 [==========================>   ]  93.2% 917504/983989 bytes... 
Writing at 0x00197e0c [===========================>  ]  94.9% 933888/983989 bytes... 
Writing at 0x0019e3c4 [===========================>  ]  96.6% 950272/983989 bytes... 
Writing at 0x001a3871 [============================> ]  98.2% 966656/983989 bytes... 
Writing at 0x001a9289 [============================> ]  99.9% 983040/983989 bytes... 
Writing at 0x001a9770 [==============================] 100.0% 983989/983989 bytes... 
Wrote 1611632 bytes (983989 compressed) at 0x00020000 in 10.2 seconds (1259.0 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

## Boot capture

Raw pyserial read at 115200 for 40 s from 19:02:56 CDT, right after the flash returned. File **`bench-logs/2026-09-06-flash-sticky-brightness.log`** (6735 bytes, 126 lines; gitignored). Quiet boot, no input activity.

```
I (379) boot: Loaded app from partition at offset 0x20000
I (380) boot: Set actual ota_seq=1 in otadata[0]
I (785) app_init: Project name:     somnus-dial
I (789) app_init: App version:      0.1.6-beta.1
I (793) app_init: Compile time:     Sep  5 2026 18:46:18
I (803) app_init: ESP-IDF:          v6.0
I (1212) ota: boot pending-verify: false
I (1331) dial_state: sb_face: stored 0 -> 0
I (2645) dial_somnus: zone mode set to single (One Bed)
I (2645) app: pad connected at http://192.168.1.169:8080
I (2683) ota: boot not pending verification (state 2) -- nothing to do
```

- App version **0.1.6-beta.1**, compile time **Sep  5 2026 18:46:18**; booted from the app at 0x20000; pad connected at 2.6 s; `sb_face: stored 0 -> 0` (the owner's Clock choice, still persisted).
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (2645) dial_somnus: zone mode set to single (One Bed) (the only case-insensitive `e (` hit is "single (One Bed)").
- `rst:` lines: **0** — no reboot.
- `W (` lines, none in the requested pattern set:

```
W (1200) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1394) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

## Git

```
$ git diff --stat 9d0d785 eef9257
 firmware/dial-idf/main/main.c | 6 +++++-
 1 file changed, 5 insertions(+), 1 deletion(-)
$ git log --oneline -3
eef9257 fix(nav): brightness percent picker is sticky against poll commits
9d0d785 docs: sticky night pickers report
d80f66e fix(nav): Night mode and Night face pickers are sticky against poll commits
```

Fix commit: **`eef925764c98f762b656ac6793944ebb098b46fd`**. Step-0 commit: `9d0d785`. Working tree after: only this report untracked. PROJECT_VER untouched.

## Deviations from the spec

1. **Commit message trailers** appended after both verbatim titles.
2. **The comment is three lines, not one** — it names the file where the gap was found so the next reader can follow it. The code change is the single `cur == SCR_BRIGHTNESS ||` term.
3. **Capture filename date** `2026-09-06-…` as instructed; machine clock read 2026-09-05 19:03 CDT.

Everything else: none.

## Not verified without hardware

- **Settings → Brightness → Day, sit on the percent picker for 30 s** without touching it (three 10 s polls): the picker must stay, big numeral and rim arc in place; before this commit the first poll commit would have returned the dial face.
- **Same for Brightness → Night (standby)** — the dim live preview must persist through the polls too (the preview is applied by the picker and re-applied on its own state ticks; leaving the screen is what ends it, so a poll must not be the thing that leaves).
