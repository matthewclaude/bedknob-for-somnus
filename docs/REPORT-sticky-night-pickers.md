# REPORT — Night mode and Night face pickers sticky against poll commits

Date: 2026-09-05 18:58 CDT (capture named `2026-09-06-…` per the task). Branch `main`. Toolchain ESP-IDF v6.0.

## Verdict

**DONE — `d80f66e`: `SCR_NIGHT_MODE` and `SCR_NIGHT_FACE` added to both of `nav_policy()`'s sticky sets next to `SCR_STANDBY_FACE`; incremental `idf.py build` recompiled `main.c` only with zero warning lines; wire-flashed; 40 s boot capture clean (App version 0.1.6-beta.1, no assert/panic/abort/Guru/`E (` lines, no reset).** Step 0 committed the spec status and the commit-2 report as `cdd7a90`. No tag, no version bump, no push. Membership table below shows every Settings target is now in both sets except `SCR_BRIGHTNESS`, which is in the no-state set only — the next visible gap, left alone as instructed. Bonus from the capture: the boot log reads `sb_face: stored 0 -> 0`, so the Standby face choice the owner made during the commit-2 eyes-on (Clock) survived the reflash — persistence proven on hardware.

## Gate

```
$ git --no-optional-locks status --short
 M docs/SPEC-standby-face.md
?? docs/REPORT-standby-face-c2.md
$ git log --oneline -1
9dad0b2 feat(standby): Standby face setting (Temperature/Clock, default Temperature); Night clock row renamed Night standby
```

Exactly the two required entries and the required HEAD; proceeded.

## Step 0

```
$ git add docs/SPEC-standby-face.md docs/REPORT-standby-face-c2.md && git commit -m "docs: standby face verified on hardware; commit 2 report"
cdd7a90 docs: standby face verified on hardware; commit 2 report
$ git --no-optional-locks status --short
(empty)
```

## The diff (`git diff cdd7a90 d80f66e`)

```diff
diff --git a/firmware/dial-idf/main/main.c b/firmware/dial-idf/main/main.c
index b68b789..0ef4ef9 100644
--- a/firmware/dial-idf/main/main.c
+++ b/firmware/dial-idf/main/main.c
@@ -399,11 +399,16 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
             // SCR_STANDBY_FACE (docs/SPEC-standby-face.md §4): a Settings
             // sub-screen reached by a deliberate tap, same category as
             // Pad Address / Timezone — a poll landing mid-choice must not
-            // yank the user off the picker.
+            // yank the user off the picker. SCR_NIGHT_MODE / SCR_NIGHT_FACE
+            // added 2026-09-05 (docs/REPORT-standby-face-c2.md, Findings 1):
+            // the same category, never listed here or below — the same
+            // class of gap the 2026-09-01 audit closed for Timezone and
+            // Adjust mode.
             if (passive || cur == SCR_SETTINGS ||
                 cur == SCR_BRIGHTNESS_MENU || cur == SCR_ADJUST_MODE ||
                 cur == SCR_PAD_ADDRESS || cur == SCR_TIMEZONE ||
-                cur == SCR_STANDBY_FACE) return cur;
+                cur == SCR_STANDBY_FACE ||
+                cur == SCR_NIGHT_MODE || cur == SCR_NIGHT_FACE) return cur;
             // First link on a fresh device: pick a default side before showing
             // the dial (SCR_SIDEPICK). Nothing to pick on a single-zone topper,
             // so that device goes straight to its one face. The `cur` half of
@@ -434,7 +439,8 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
                 cur == SCR_WIFI || cur == SCR_BRIGHTNESS ||
                 cur == SCR_BRIGHTNESS_MENU || cur == SCR_UPDATE ||
                 cur == SCR_PAD_ADDRESS || cur == SCR_TIMEZONE ||
-                cur == SCR_ADJUST_MODE || cur == SCR_STANDBY_FACE)
+                cur == SCR_ADJUST_MODE || cur == SCR_STANDBY_FACE ||
+                cur == SCR_NIGHT_MODE || cur == SCR_NIGHT_FACE)
                 return cur;
         }
         // PH_PAD_DISCOVERY gets its own screen (live scan progress); every
```

Two lines of code (one per set) plus the comment recording where the gap was found. Nothing else touched.

## Sticky-set membership of every screen Settings navigates to

Sources: `scr_settings.c` `ui_router_go(SCR_…)` calls (lines 194, 279, 290, 301, 339, 372, 382, 640) and, one level down, `scr_brightness_menu.c` (128, 135, 146 → `SCR_BRIGHTNESS` with arg 0/1/2). `SCR_ADJUST_MODE` and `SCR_SIDEPICK` are listed too because `ui_router.h` documents them as Settings-origin screens even though the Settings rows that opened them are hidden/removed today. The two sets are `main.c` have-state (line 403–410, `if (passive || cur == …) return cur;` where `passive` = MENU / WIFI / ABOUT / UPDATE) and no-state (433–439).

| Screen | Reached from Settings via | have-state set | no-state set | Note |
|---|---|---|---|---|
| `SCR_MENU` | Back (194, 640) | yes (`passive`) | yes | |
| `SCR_SETTINGS` | (itself) | yes | yes | |
| `SCR_BRIGHTNESS_MENU` | Brightness row (279) | yes | yes | |
| `SCR_BRIGHTNESS` | Brightness menu → Day / Night (in use) / Night (standby) | **no** | yes | **the next gap**: a poll landing while the percent picker is open returns SCR_DIAL; the picker's own detents stamp input so the tier stays ACTIVE, but a routine 10 s poll commit still re-runs nav_policy. Not fixed here (task: fix nothing else). |
| `SCR_NIGHT_MODE` | Night mode row (290) | **yes — this commit** | **yes — this commit** | was in neither |
| `SCR_NIGHT_FACE` | Night face row (301) | **yes — this commit** | **yes — this commit** | was in neither |
| `SCR_STANDBY_FACE` | Standby face row (339) | yes (`9dad0b2`) | yes (`9dad0b2`) | |
| `SCR_TIMEZONE` | Timezone row (372) | yes | yes | Settings-raised visit only; the gate-raised visit is protected upstream (F7) |
| `SCR_PAD_ADDRESS` | Pad Address row (382) | yes | yes | |
| `SCR_ADJUST_MODE` | (row hidden; dial long-press) | yes | yes | |
| `SCR_SIDEPICK` | (no Settings row any more) | own clause (413–415: `cur == SCR_SIDEPICK` keeps it) | **no** | fresh-device flow only; with no state the connect screen wins, which is the original design |
| `SCR_WIFI`, `SCR_ABOUT`, `SCR_UPDATE` | Menu, not Settings | yes (`passive`) | yes | listed for completeness; the passive set is also idle-dismissed at STANDBY (378–381) |

So after this commit the only Settings-reachable screen missing from a set is `SCR_BRIGHTNESS` in the have-state set.

## `idf.py build` — raw tail (last 40 lines)

Incremental, as the task allowed. Exit 0. One object rebuilt: `main.c.obj` (log line 174), then re-link and re-image.

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
somnus-dial.bin binary size 0x1897a0 bytes. Smallest app partition is 0x400000 bytes. 0x276860 bytes (62%) free.
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

App descriptor after the build: App version **0.1.6-beta.1**, Compile time **Sep  5 2026 18:46:18** — unchanged from commit 2's clean build, because `__DATE__`/`__TIME__` live in `esp_app_desc.c`, which an incremental `main.c`-only build does not recompile. The image is nevertheless new: `main.c.obj` was rebuilt and `somnus-dial.elf`/`.bin` regenerated (tail above), and the flash log shows nothing further to build.

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
[  0%] Built target _project_elf_src
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target blank_ota_data
[  0%] Built target memory_ld_in_preprocess
[  0%] Performing build step for 'bootloader'
[  1%] Built target __idf_esp_https_ota
[  1%] Built target __idf_esp_http_server
[  2%] Built target bootloader_ld_in_preprocess
[  2%] Built target _project_elf_src
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
[ 74%] Built target __idf_esp_hal_dma
[ 14%] Built target __idf_esp_driver_usb_serial_jtag
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
[ 26%] Completed 'bootloader'
[ 27%] Built target __idf_esp_hal_i2s
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
[ 55%] Built target __idf_esp_hal_twai
[ 55%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_esp_driver_i2c
[ 56%] Built target __idf_dial_state
[ 57%] Built target __idf_unity
[ 58%] Built target __idf_esp_hal_ledc
[ 58%] Built target __idf_espressif__cjson
[ 58%] Built target __idf_dial_time
[ 58%] Built target __idf_esp_hal_lcd
[ 58%] Built target __idf_dial_knob
[ 59%] Built target __idf_sdmmc
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_esp_hal_pcnt
[ 60%] Built target __idf_console
[ 60%] Built target __idf_esp_hal_cam
[ 60%] Built target __idf_esp_hal_rmt
[ 60%] Built target __idf_esp_hal_mcpwm
[ 60%] Built target __idf_esp_driver_tsens
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_esp_driver_twai
[ 60%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_protobuf-c
[ 61%] Built target __idf_wear_levelling
[ 62%] Built target __idf_esp_hid
[ 62%] Built target __idf_esp_https_server
[ 62%] Built target __idf_perfmon
[ 63%] Built target __idf_esp_driver_ledc
[ 63%] Built target __idf_spiffs
[ 63%] Built target __idf_rt
[ 63%] Built target __idf_driver
[ 63%] Built target __idf_dial_ota
[ 64%] Built target __idf_esp_lcd
[ 64%] Built target __idf_dial_somnus
[ 64%] Built target __idf_dial_net
[ 64%] Built target __idf_cmock
[ 65%] Built target __idf_esp_driver_cam
[ 65%] Built target __idf_esp_driver_pcnt
[ 66%] Built target __idf_esp_driver_sdspi
[ 67%] Built target __idf_esp_driver_mcpwm
[ 68%] Built target __idf_esp_driver_rmt
[ 68%] Built target __idf_esp_driver_sd_intf
[ 68%] Built target __idf_espressif__esp_lcd_sh8601
[ 69%] Built target __idf_protocomm
[ 70%] Built target __idf_dial_pad_discovery
[ 70%] Built target __idf_esp_driver_sdmmc
[ 71%] Built target __idf_esp_local_ctrl
[ 85%] Built target __idf_fatfs
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
somnus-dial.bin binary size 0x1897a0 bytes. Smallest app partition is 0x400000 bytes. 0x276860 bytes (62%) free.
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
Wrote 22576 bytes (14461 compressed) at 0x00000000 in 0.3 seconds (699.5 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (754.2 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.0 seconds (1558.3 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1611680 bytes to 984041...

Writing at 0x00020000 [                              ]   0.0% 0/984041 bytes... 
Writing at 0x0002b90f [                              ]   1.7% 16384/984041 bytes... 
Writing at 0x00038591 [                              ]   3.3% 32768/984041 bytes... 
Writing at 0x0004b6e4 [>                             ]   5.0% 49152/984041 bytes... 
Writing at 0x00053211 [>                             ]   6.7% 65536/984041 bytes... 
Writing at 0x0005ba28 [=>                            ]   8.3% 81920/984041 bytes... 
Writing at 0x0006528e [=>                            ]  10.0% 98304/984041 bytes... 
Writing at 0x00070d28 [==>                           ]  11.7% 114688/984041 bytes... 
Writing at 0x00083aad [==>                           ]  13.3% 131072/984041 bytes... 
Writing at 0x0008a7e7 [===>                          ]  15.0% 147456/984041 bytes... 
Writing at 0x00093ba7 [===>                          ]  16.6% 163840/984041 bytes... 
Writing at 0x0009c1ef [====>                         ]  18.3% 180224/984041 bytes... 
Writing at 0x000a4821 [====>                         ]  20.0% 196608/984041 bytes... 
Writing at 0x000aa12c [=====>                        ]  21.6% 212992/984041 bytes... 
Writing at 0x000af9b1 [=====>                        ]  23.3% 229376/984041 bytes... 
Writing at 0x000b4e55 [======>                       ]  25.0% 245760/984041 bytes... 
Writing at 0x000bad1f [======>                       ]  26.6% 262144/984041 bytes... 
Writing at 0x000c01b1 [=======>                      ]  28.3% 278528/984041 bytes... 
Writing at 0x000c5bbd [=======>                      ]  30.0% 294912/984041 bytes... 
Writing at 0x000cabf3 [========>                     ]  31.6% 311296/984041 bytes... 
Writing at 0x000d0164 [========>                     ]  33.3% 327680/984041 bytes... 
Writing at 0x000d63bd [=========>                    ]  35.0% 344064/984041 bytes... 
Writing at 0x000dba50 [=========>                    ]  36.6% 360448/984041 bytes... 
Writing at 0x000e1491 [==========>                   ]  38.3% 376832/984041 bytes... 
Writing at 0x000e6540 [==========>                   ]  40.0% 393216/984041 bytes... 
Writing at 0x000eb9b3 [===========>                  ]  41.6% 409600/984041 bytes... 
Writing at 0x000f10a5 [===========>                  ]  43.3% 425984/984041 bytes... 
Writing at 0x000f6dc9 [============>                 ]  45.0% 442368/984041 bytes... 
Writing at 0x000fbfa3 [============>                 ]  46.6% 458752/984041 bytes... 
Writing at 0x0010130a [=============>                ]  48.3% 475136/984041 bytes... 
Writing at 0x001062ac [=============>                ]  49.9% 491520/984041 bytes... 
Writing at 0x0010b3fe [==============>               ]  51.6% 507904/984041 bytes... 
Writing at 0x00110d3c [==============>               ]  53.3% 524288/984041 bytes... 
Writing at 0x001163a9 [===============>              ]  54.9% 540672/984041 bytes... 
Writing at 0x0011ba98 [===============>              ]  56.6% 557056/984041 bytes... 
Writing at 0x00120f71 [================>             ]  58.3% 573440/984041 bytes... 
Writing at 0x0012644e [================>             ]  59.9% 589824/984041 bytes... 
Writing at 0x0012bf4c [=================>            ]  61.6% 606208/984041 bytes... 
Writing at 0x00131405 [=================>            ]  63.3% 622592/984041 bytes... 
Writing at 0x001365d7 [==================>           ]  64.9% 638976/984041 bytes... 
Writing at 0x0013b708 [==================>           ]  66.6% 655360/984041 bytes... 
Writing at 0x00140f4b [===================>          ]  68.3% 671744/984041 bytes... 
Writing at 0x001466ed [===================>          ]  69.9% 688128/984041 bytes... 
Writing at 0x0014bc19 [====================>         ]  71.6% 704512/984041 bytes... 
Writing at 0x0015112a [====================>         ]  73.3% 720896/984041 bytes... 
Writing at 0x00156991 [=====================>        ]  74.9% 737280/984041 bytes... 
Writing at 0x0015c350 [=====================>        ]  76.6% 753664/984041 bytes... 
Writing at 0x00161440 [======================>       ]  78.3% 770048/984041 bytes... 
Writing at 0x0016667a [======================>       ]  79.9% 786432/984041 bytes... 
Writing at 0x0016bf49 [=======================>      ]  81.6% 802816/984041 bytes... 
Writing at 0x001711e5 [=======================>      ]  83.2% 819200/984041 bytes... 
Writing at 0x00176343 [========================>     ]  84.9% 835584/984041 bytes... 
Writing at 0x0017b6cc [========================>     ]  86.6% 851968/984041 bytes... 
Writing at 0x00181470 [=========================>    ]  88.2% 868352/984041 bytes... 
Writing at 0x00186a8f [=========================>    ]  89.9% 884736/984041 bytes... 
Writing at 0x0018d6be [==========================>   ]  91.6% 901120/984041 bytes... 
Writing at 0x001929f8 [==========================>   ]  93.2% 917504/984041 bytes... 
Writing at 0x00197de7 [===========================>  ]  94.9% 933888/984041 bytes... 
Writing at 0x0019e3a4 [===========================>  ]  96.6% 950272/984041 bytes... 
Writing at 0x001a3857 [============================> ]  98.2% 966656/984041 bytes... 
Writing at 0x001a926f [============================> ]  99.9% 983040/984041 bytes... 
Writing at 0x001a97a0 [==============================] 100.0% 984041/984041 bytes... 
Wrote 1611680 bytes (984041 compressed) at 0x00020000 in 10.2 seconds (1267.4 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

## Boot capture

Raw pyserial read at 115200 for 40 s from 18:58:03 CDT, right after the flash returned. File **`bench-logs/2026-09-06-flash-sticky-night.log`** (6736 bytes, 126 lines; gitignored). Quiet boot, no input activity.

```
I (379) boot: Loaded app from partition at offset 0x20000
I (380) boot: Set actual ota_seq=1 in otadata[0]
I (785) app_init: Project name:     somnus-dial
I (789) app_init: App version:      0.1.6-beta.1
I (793) app_init: Compile time:     Sep  5 2026 18:46:18
I (803) app_init: ESP-IDF:          v6.0
I (1212) ota: boot pending-verify: false
I (1331) dial_state: sb_face: stored 0 -> 0
I (3027) dial_somnus: zone mode set to single (One Bed)
I (3027) app: pad connected at http://192.168.1.169:8080
I (3070) ota: boot not pending verification (state 2) -- nothing to do
```

- App version **0.1.6-beta.1**, compile time **Sep  5 2026 18:46:18** (see the build note above); booted from the app at 0x20000; pad connected at 3.0 s.
- `I (1331) dial_state: sb_face: stored 0 -> 0` — the owner's Clock choice from the commit-2 eyes-on came back from NVS through a wire flash (NVS is not written by `idf.py flash`), so the "choice survives a reboot" item from that report is now observed.
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (3027) dial_somnus: zone mode set to single (One Bed) (the only case-insensitive `e (` hit is "single (One Bed)").
- `rst:` lines: **0** — no reboot.
- `W (` lines, none in the requested pattern set:

```
W (1200) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1393) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

## Git

```
$ git diff --stat cdd7a90 d80f66e
 firmware/dial-idf/main/main.c | 12 +++++++++---
 1 file changed, 9 insertions(+), 3 deletions(-)
$ git log --oneline -3
d80f66e fix(nav): Night mode and Night face pickers are sticky against poll commits
cdd7a90 docs: standby face verified on hardware; commit 2 report
9dad0b2 feat(standby): Standby face setting (Temperature/Clock, default Temperature); Night clock row renamed Night standby
```

Fix commit: **`d80f66e5e4021914d86946f621ba555b40e4cbb3`**. Step-0 commit: `cdd7a90`. Working tree after: only this report untracked. PROJECT_VER untouched.

## Deviations from the spec

1. **Commit message trailers** appended after both verbatim titles.
2. **Capture filename date** `2026-09-06-…` as instructed; machine clock read 2026-09-05 18:58 CDT.

Everything else: none. The build was incremental as the task permitted; the flash was to the native port; nothing beyond the two sets changed.

## Not verified without hardware

- **Settings → Night mode, sit on it for 30 s** (three 10 s polls): the picker must stay on screen with its checkmark in place; before this commit the first poll commit would have returned the dial face.
- **Settings → Night face, same 30 s** — same expectation.
- Optionally the same on **Brightness → Day** to see the remaining have-state gap in the flesh (it should yank, by the table above); a one-line fix when you want it.
