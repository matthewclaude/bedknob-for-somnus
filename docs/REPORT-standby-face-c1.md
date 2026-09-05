# REPORT — Standby face, commit 1: `standby_screen()` consumer, hard-wired to Temperature

Date: 2026-09-05 18:20 CDT (capture file named `2026-09-06-…` per the task). Branch `main`. Spec: `docs/SPEC-standby-face.md` §6 commit 1. Toolchain ESP-IDF v6.0.

## Verdict

**DONE — `03e6259`: `standby_screen()` added to `main.c`, both `nav_policy` STANDBY sites route through it, hard-wired to `SCR_DIAL`; `idf.py fullclean && idf.py build` with zero compiler warnings (33 `main` + `dial_ui` objects), wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.6-beta.1, compile time 18:18:37, no assert/panic/abort/Guru/`E (` lines, no reset).** `main.c` is the only file in the commit. No `scr_dial.c` change was needed or made. No tag, no version bump, no push. Two things for the owner: the spec on disk changed mid-task (default is now Temperature; the helper's comment follows the spec, not the task text — Deviations 1), and one §5-class finding about the night standby duty being the "Night clock" brightness (Findings, below).

## Gate

```
$ git --no-optional-locks status --short -uall
?? docs/REPORT-release-0.1.6-beta.1.md
$ git add docs/REPORT-release-0.1.6-beta.1.md && git commit -m "docs: report"
[main 1f04499] docs: report
$ git --no-optional-locks status --short -uall
(empty)
$ test -f docs/SPEC-standby-face.md && echo "spec present"
spec present
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.6-beta.1")
$ git log --oneline -3
c559b8e docs: SPEC-standby-face — Clock/Temperature standby face, replaces the screen-timeout Off idea
46f65b2 release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and absolute-rails fix
baa7f72 docs: rails fix report
```

Gate passed; proceeded. (`1f04499` is the gate's report commit; the feature commit sits on top of it.)

## The full diff (`git diff 1f04499 03e6259`)

```diff
diff --git a/firmware/dial-idf/main/main.c b/firmware/dial-idf/main/main.c
index e61e93e..d7c17be 100644
--- a/firmware/dial-idf/main/main.c
+++ b/firmware/dial-idf/main/main.c
@@ -148,6 +148,19 @@ static void knob_init(void)
 
 /* ---- navigation policy (runs in the LVGL task) ------------------------ */
 
+// Standby face (docs/SPEC-standby-face.md): the screen the router shows
+// while dial_power_level() == DPWR_STANDBY. Both STANDBY sites in nav_policy
+// read this one rule. Commit 1 (§6) hard-wires Temperature — the dial face,
+// which dial_power dims to the STANDBY duty like any other screen, so a wake
+// is a brightness change and not a screen transition. Commit 2 makes this
+// read the ui/sb_face pref — default Temperature (SCR_DIAL) per the spec's
+// 2026-09-05 ruling, with SCR_STANDBY (the clock) as the other value.
+static screen_id_t standby_screen(const app_state_t *st)
+{
+    (void)st;   // commit 2 reads the pref off st here
+    return SCR_DIAL;
+}
+
 static screen_id_t nav_policy(const app_state_t *st, void **arg)
 {
     // OTA install takeover (M6 UX hardening): once the confirmed install on
@@ -365,7 +378,7 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
                            cur == SCR_WIFI || cur == SCR_ABOUT || cur == SCR_UPDATE;
             if (passive && dial_power_level() == DPWR_STANDBY) {
                 *arg = (void *)(uintptr_t)st->ui_zone;
-                return SCR_STANDBY;
+                return standby_screen(st);
             }
             // ADJUST_MODE joins BRIGHTNESS_MENU here (not the idle-dismissed
             // passive set above): both are Settings sub-screens reached by a
@@ -396,7 +409,7 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
                 ((st->fresh_device && !st->side_picked) || cur == SCR_SIDEPICK))
                 return SCR_SIDEPICK;
             *arg = (void *)(uintptr_t)st->ui_zone;
-            return dial_power_level() == DPWR_STANDBY ? SCR_STANDBY : SCR_DIAL;
+            return dial_power_level() == DPWR_STANDBY ? standby_screen(st) : SCR_DIAL;
         }
         // Never trap the user (field incident 2026-07-28): with no device
         // state the connect/error screen used to own the display outright,
```

`standby_screen()` takes `const app_state_t *st` and returns `screen_id_t` (what `nav_policy` returns); `(void)st` keeps `-Wunused-parameter` quiet until commit 2 reads the pref. The READY-branch ternary keeps its `*arg = ui_zone` assignment on the line above; the passive-screen site keeps its `*arg` assignment too, so `SCR_DIAL` arrives with the zone it needs either way. With `SCR_DIAL` returned at both tiers, the router's re-run of `nav_policy` on a power-level change (`ui_router.c`, `power_changed`) resolves to the same `(id, arg)` pair and `ui_router_go` no-ops — a wake is a brightness change only, as §3 says.

## Step 2 — every other reference to `SCR_STANDBY` (post-edit line numbers)

| Where | Line | What | Route through `standby_screen()`? |
|---|---|---|---|
| `main.c` | 157 | the new helper's comment naming SCR_STANDBY as the clock value | n/a (the helper itself) |
| `main.c` | 353 | comment in the update-prompt block: "lands back on SCR_DIAL/SCR_STANDBY same as any other abandoned sub-screen" | No — describes the fallback below it, which is the ternary that now calls the helper (line 412); the comment is still literally true (either can result) |
| `main.c` | 381 | passive-screen site (Menu / Wi-Fi / About / Update at STANDBY) | **Yes — done** |
| `main.c` | 412 | READY-branch ternary | **Yes — done** |
| `main.c` | 188 (`ota.unattended` block), 1185 (`ota_prompt_woke`), 1272 (prompt exit) | OTA logic keyed on `DPWR_STANDBY` — the tier, not the screen | No — these never mention the screen; they keep working with the dial face showing (§1's point). Not a finding |
| `components/dial_ui/ui_screens.c` | 14 | `ui_router_register(SCR_STANDBY, &scr_standby)` | No — registration |
| `components/dial_ui/ui_router.h` | 26 | the enum entry's comment "always-on clock face" | No — the screen is still that; the comment is about the screen, not about when it shows |
| `components/dial_ui/scr_standby.c` | 2, 9 | the clock face's own header | No |
| `components/dial_ui/scr_updating.c` | 10 | header comment: nav_policy forces SCR_UPDATING "the same way it would force SCR_STANDBY or SCR_ERROR" | No — an analogy in a comment; not a behavioural assumption. Slightly stale wording now that STANDBY may show SCR_DIAL; harmless |
| `components/dial_ui/scr_brightness.c` | 17, 102 | the Night **clock** brightness picker previews `DPWR_STANDBY` duty | No code route — but see Findings 1: this pref names the night standby duty "clock" |
| `simulator/main.c` | 1065, 1085 | `standby` / `standby-update` scenarios call `ui_router_go(SCR_STANDBY, …)` directly | No — they render the clock face on purpose; the simulator has no `nav_policy` and cannot force the tier (spec §6 commit 2 already says `standby-temperature` is hardware-only unless the harness grows that) |

Nothing else in `main.c` or `components/` assumes "standby means the clock is showing": every tier-keyed gate reads `dial_power_level()`, and no code compares `ui_router_current()` against `SCR_STANDBY`.

## `idf.py fullclean && idf.py build` — raw tail (last 40 lines)

Exit 0.

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
somnus-dial.bin binary size 0x189190 bytes. Smallest app partition is 0x400000 bytes. 0x276e70 bytes (62%) free.
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

Every line of the build log containing "warning" (case-insensitive):

```
4: CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
11: This warning is for project developers.  Use -Wno-author or -Wno-deprecated
```

That is CMake's pre-existing deprecation note about `cmake_minimum_required` in the project `CMakeLists.txt`. Compiler warnings from `main`: **none**; from `dial_ui`: **none**.

App descriptor (`esptool image-info`): App version **0.1.6-beta.1**, Compile time **Sep  5 2026 18:18:37**.

## Port

```
$ ls /dev/cu.usbmodem*
/dev/cu.usbmodem83401
$ ls /dev/cu.usbserial*
(no matches)
```

Native USB-Serial/JTAG port only — no "flip the plug".

## `idf.py -p /dev/cu.usbmodem83401 flash` — raw output

Exit 0. Nothing recompiled by the flash step (no `Building C object` lines; the clean build is what went on the wire). Warning lines in the flash log beyond the same CMake note: none. From `Executing action: flash`; esptool's terminal control sequences and in-place progress rewrites stripped.

```
Executing action: flash
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 flash"...
[  0%] Built target blank_ota_data
[  0%] Built target partition_table_bin
[  0%] Built target sections_ld_in_preprocess
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
[  2%] Built target __idf_esp_adc
[ 26%] Built target __idf_esp_system
[  3%] Built target __idf_esp-tls
[ 32%] Built target __idf_efuse
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
[  8%] Built target __idf_wpa_supplicant
[ 64%] Built target __idf_esp_hal_timg
[  9%] Built target __idf_esp_netif
[ 65%] Built target __idf_esp_bootloader_format
[ 66%] Built target __idf_spi_flash
[ 15%] Built target __idf_lwip
[ 67%] Built target __idf_esp_hal_clock
[ 15%] Built target __idf_vfs
[ 71%] Built target __idf_esp_hal_gpspi
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 15%] Built target __idf_esp_phy
[ 76%] Built target __idf_esp_hal_pmu
[ 16%] Built target __idf_nvs_flash
[ 79%] Built target __idf_esp_hal_usb
[ 17%] Built target __idf_nvs_sec_provider
[ 84%] Built target __idf_esp_hal_gpio
[ 18%] Built target __idf_esp_event
[ 88%] Built target __idf_hal
[ 19%] Built target __idf_esp_driver_uart
[ 93%] Built target __idf_soc
[ 19%] Built target __idf_esp_psram
[ 95%] Built target __idf_xtensa
[ 19%] Built target __idf_esp_ringbuf
[ 97%] Built target __idf_main
[ 20%] Built target __idf_esp_timer
[ 98%] Built target bootloader.elf
[ 21%] Built target __idf_cxx
[100%] Built target gen_bootloader_binary
[ 21%] Built target __idf_pthread
[100%] Built target gen_project_binary
[ 22%] Built target __idf_esp_libc
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 24%] Built target __idf_freertos
[100%] Built target bootloader_check_size
[100%] Built target app
[ 27%] Built target __idf_esp_hw_support
[ 27%] No install step for 'bootloader'
[ 28%] Built target __idf_esp_hal_i2s
[ 28%] Completed 'bootloader'
[ 28%] Built target __idf_esp_hal_touch_sens
[ 29%] Built target bootloader
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
[ 54%] Built target __idf_esp_hal_lcd
[ 55%] Built target __idf_espressif__cjson
[ 55%] Built target __idf_dial_state
[ 55%] Built target __idf_esp_driver_i2c
[ 55%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_esp_hal_twai
[ 56%] Built target __idf_esp_https_server
[ 56%] Built target __idf_dial_knob
[ 56%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_dial_time
[ 56%] Built target __idf_esp_hid
[ 57%] Built target __idf_esp_driver_gptimer
[ 59%] Built target __idf_esp_hal_cam
[ 59%] Built target __idf_wear_levelling
[ 59%] Built target __idf_perfmon
[ 60%] Built target __idf_sdmmc
[ 60%] Built target __idf_protobuf-c
[ 60%] Built target __idf_esp_hal_pcnt
[ 60%] Built target __idf_esp_hal_rmt
[ 62%] Built target __idf_unity
[ 62%] Built target __idf_console
[ 62%] Built target __idf_esp_hal_mcpwm
[ 62%] Built target __idf_esp_driver_touch_sens
[ 62%] Built target __idf_esp_driver_sdm
[ 62%] Built target __idf_rt
[ 62%] Built target __idf_esp_driver_tsens
[ 63%] Built target __idf_spiffs
[ 63%] Built target __idf_dial_somnus
[ 63%] Built target __idf_dial_ota
[ 63%] Built target __idf_esp_driver_ledc
[ 63%] Built target __idf_dial_net
[ 64%] Built target __idf_esp_lcd
[ 65%] Built target __idf_driver
[ 65%] Built target __idf_esp_driver_twai
[ 65%] Built target __idf_esp_eth
[ 66%] Built target __idf_protocomm
[ 66%] Built target __idf_esp_driver_sd_intf
[ 66%] Built target __idf_cmock
[ 67%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_esp_driver_cam
[ 68%] Built target __idf_esp_driver_pcnt
[ 69%] Built target __idf_esp_driver_rmt
[ 70%] Built target __idf_esp_driver_mcpwm
[ 71%] Built target __idf_dial_pad_discovery
[ 71%] Built target __idf_espressif__esp_lcd_sh8601
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
somnus-dial.bin binary size 0x189190 bytes. Smallest app partition is 0x400000 bytes. 0x276e70 bytes (62%) free.
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
Compressed 22576 bytes to 14459...

Writing at 0x00000000 [                              ]   0.0% 0/14459 bytes... 
Writing at 0x00005830 [==============================] 100.0% 14459/14459 bytes... 
Wrote 22576 bytes (14459 compressed) at 0x00000000 in 0.2 seconds (724.6 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'partition_table/partition-table.bin' at 0x00008000...
Flash will be erased from 0x00008000 to 0x00008fff...
Compressed 3072 bytes to 141...

Writing at 0x00008000 [                              ]   0.0% 0/141 bytes... 
Writing at 0x00008c00 [==============================] 100.0% 141/141 bytes... 
Wrote 3072 bytes (141 compressed) at 0x00008000 in 0.0 seconds (749.6 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'ota_data_initial.bin' at 0x00019000...
Flash will be erased from 0x00019000 to 0x0001afff...
Compressed 8192 bytes to 31...

Writing at 0x00019000 [                              ]   0.0% 0/31 bytes... 
Writing at 0x0001b000 [==============================] 100.0% 31/31 bytes... 
Wrote 8192 bytes (31 compressed) at 0x00019000 in 0.1 seconds (991.2 kbit/s).
Verifying written data...
Hash of data verified.

Writing 'somnus-dial.bin' at 0x00020000...
Flash will be erased from 0x00020000 to 0x001a9fff...
Compressed 1610128 bytes to 983040...

Writing at 0x00020000 [                              ]   0.0% 0/983040 bytes... 
Writing at 0x0002b912 [                              ]   1.7% 16384/983040 bytes... 
Writing at 0x0003858b [>                             ]   3.3% 32768/983040 bytes... 
Writing at 0x0004b740 [>                             ]   5.0% 49152/983040 bytes... 
Writing at 0x000531dc [=>                            ]   6.7% 65536/983040 bytes... 
Writing at 0x0005ba38 [=>                            ]   8.3% 81920/983040 bytes... 
Writing at 0x000652dc [==>                           ]  10.0% 98304/983040 bytes... 
Writing at 0x00070d8b [==>                           ]  11.7% 114688/983040 bytes... 
Writing at 0x00083bbc [===>                          ]  13.3% 131072/983040 bytes... 
Writing at 0x0008a795 [===>                          ]  15.0% 147456/983040 bytes... 
Writing at 0x00093b47 [====>                         ]  16.7% 163840/983040 bytes... 
Writing at 0x0009c1b9 [====>                         ]  18.3% 180224/983040 bytes... 
Writing at 0x000a47da [=====>                        ]  20.0% 196608/983040 bytes... 
Writing at 0x000aa0e3 [=====>                        ]  21.7% 212992/983040 bytes... 
Writing at 0x000af969 [======>                       ]  23.3% 229376/983040 bytes... 
Writing at 0x000b4e3c [======>                       ]  25.0% 245760/983040 bytes... 
Writing at 0x000bac08 [=======>                      ]  26.7% 262144/983040 bytes... 
Writing at 0x000c00bb [=======>                      ]  28.3% 278528/983040 bytes... 
Writing at 0x000c5aa7 [========>                     ]  30.0% 294912/983040 bytes... 
Writing at 0x000caa97 [========>                     ]  31.7% 311296/983040 bytes... 
Writing at 0x000d002b [=========>                    ]  33.3% 327680/983040 bytes... 
Writing at 0x000d6587 [=========>                    ]  35.0% 344064/983040 bytes... 
Writing at 0x000db95d [==========>                   ]  36.7% 360448/983040 bytes... 
Writing at 0x000e137d [==========>                   ]  38.3% 376832/983040 bytes... 
Writing at 0x000e6425 [===========>                  ]  40.0% 393216/983040 bytes... 
Writing at 0x000eb8e5 [===========>                  ]  41.7% 409600/983040 bytes... 
Writing at 0x000f0fbf [============>                 ]  43.3% 425984/983040 bytes... 
Writing at 0x000f6c9b [============>                 ]  45.0% 442368/983040 bytes... 
Writing at 0x000fbe5c [=============>                ]  46.7% 458752/983040 bytes... 
Writing at 0x00101256 [=============>                ]  48.3% 475136/983040 bytes... 
Writing at 0x001061fe [==============>               ]  50.0% 491520/983040 bytes... 
Writing at 0x0010b30e [==============>               ]  51.7% 507904/983040 bytes... 
Writing at 0x00110cc3 [===============>              ]  53.3% 524288/983040 bytes... 
Writing at 0x0011624d [===============>              ]  55.0% 540672/983040 bytes... 
Writing at 0x0011b995 [================>             ]  56.7% 557056/983040 bytes... 
Writing at 0x00120e7b [================>             ]  58.3% 573440/983040 bytes... 
Writing at 0x00126455 [=================>            ]  60.0% 589824/983040 bytes... 
Writing at 0x0012bec2 [=================>            ]  61.7% 606208/983040 bytes... 
Writing at 0x00131386 [==================>           ]  63.3% 622592/983040 bytes... 
Writing at 0x001364a8 [==================>           ]  65.0% 638976/983040 bytes... 
Writing at 0x0013b6e8 [===================>          ]  66.7% 655360/983040 bytes... 
Writing at 0x001410e7 [===================>          ]  68.3% 671744/983040 bytes... 
Writing at 0x00146627 [====================>         ]  70.0% 688128/983040 bytes... 
Writing at 0x0014bb39 [====================>         ]  71.7% 704512/983040 bytes... 
Writing at 0x0015107c [=====================>        ]  73.3% 720896/983040 bytes... 
Writing at 0x00156ae9 [=====================>        ]  75.0% 737280/983040 bytes... 
Writing at 0x0015c2ad [======================>       ]  76.7% 753664/983040 bytes... 
Writing at 0x001613a5 [======================>       ]  78.3% 770048/983040 bytes... 
Writing at 0x001665bb [=======================>      ]  80.0% 786432/983040 bytes... 
Writing at 0x0016bf16 [=======================>      ]  81.7% 802816/983040 bytes... 
Writing at 0x00171104 [========================>     ]  83.3% 819200/983040 bytes... 
Writing at 0x00176296 [========================>     ]  85.0% 835584/983040 bytes... 
Writing at 0x0017b67b [=========================>    ]  86.7% 851968/983040 bytes... 
Writing at 0x001814cb [=========================>    ]  88.3% 868352/983040 bytes... 
Writing at 0x00186be3 [==========================>   ]  90.0% 884736/983040 bytes... 
Writing at 0x0018d62d [==========================>   ]  91.7% 901120/983040 bytes... 
Writing at 0x001928a4 [===========================>  ]  93.3% 917504/983040 bytes... 
Writing at 0x00197efc [===========================>  ]  95.0% 933888/983040 bytes... 
Writing at 0x0019e36a [============================> ]  96.7% 950272/983040 bytes... 
Writing at 0x001a373d [============================> ]  98.3% 966656/983040 bytes... 
Writing at 0x001a9190 [==============================] 100.0% 983040/983040 bytes... 
Wrote 1610128 bytes (983040 compressed) at 0x00020000 in 10.4 seconds (1241.8 kbit/s).
Verifying written data...
Hash of data verified.

Hard resetting via RTS pin...
[100%] Built target flash
Done
```

## Boot capture

Raw pyserial read at 115200 for 40 s from 18:19:40 CDT, right after the flash returned. File **`bench-logs/2026-09-06-flash-standby-face-c1.log`** (6987 bytes, 130 lines; gitignored). Starts mid-bootloader (segment 3) as before. Quiet boot — no knob or touch activity in the window.

```
I (379) boot: Loaded app from partition at offset 0x20000
I (379) boot: Set actual ota_seq=1 in otadata[0]
I (785) app_init: Project name:     somnus-dial
I (789) app_init: App version:      0.1.6-beta.1
I (793) app_init: Compile time:     Sep  5 2026 18:18:37
I (803) app_init: ESP-IDF:          v6.0
I (1212) ota: boot pending-verify: false
I (4178) dial_somnus: zone mode set to single (One Bed)
I (4178) app: pad connected at http://192.168.1.169:8080
I (4306) ota: boot not pending verification (state 2) -- nothing to do
I (10311) power: plugged (4620 mV)
```

- App version **0.1.6-beta.1**, compile time **Sep  5 2026 18:18:37**; booted from the app at 0x20000 (`ota_seq=1`); pad connected at 4.2 s; `power: plugged (4620 mV)` at 10.3 s.
- Lines matching `assert|panic|abort|Guru|E \(|rollback` (case-insensitive): **1** — I (4178) dial_somnus: zone mode set to single (One Bed) (the only case-insensitive `e (` hit is "single (One Bed)").
- `rst:` lines: **0** — no reboot.
- `W (` lines, none in the requested pattern set:

```
W (1200) sh8601: The 36h command has been used and will be overwritten by external initialization sequence
W (1390) wifi:Password length matches WPA2 standards, authmode threshold changes from OPEN to WPA2
```

The capture cannot show the standby face itself: the screen timeout is longer than 40 s and nothing about the face is logged.

## Git

```
$ git diff --stat 1f04499 03e6259
 firmware/dial-idf/main/main.c | 17 +++++++++++++++--
 1 file changed, 15 insertions(+), 2 deletions(-)
$ git log --oneline -3
03e6259 feat(standby): standby_screen() helper; dial face at STANDBY (hard-wired, spec §6 commit 1)
1f04499 docs: report
c559b8e docs: SPEC-standby-face — Clock/Temperature standby face, replaces the screen-timeout Off idea
```

Commit: **`03e625973006a76fbd8821ecb24239115497e4c3`**. Working tree after this task: `docs/SPEC-standby-face.md` **modified, uncommitted — the owner's own edit, made while this task ran** (5 lines: default Temperature; see Deviations 1), plus this report untracked. Neither was staged. No tag, no push, PROJECT_VER untouched.

## Deviations from the spec

1. **The helper's comment says default Temperature, not "default SCR_STANDBY".** The task text asked for a one-line comment that commit 2 "makes it read the ui/sb_face pref, default SCR_STANDBY". While this task was running the owner edited `docs/SPEC-standby-face.md` on disk (uncommitted): §3 now reads "Default Temperature, deliberately — owner's ruling 2026-09-05", §4 clamps to 1 (Temperature), §6 commit 2 says default Temperature. "Spec before code" makes the spec the authority, so the comment follows it and names SCR_STANDBY as the other value. The code is unaffected (commit 1 is hard-wired either way). The spec edit is left for the owner to commit; it is not in `03e6259` (files allowed: `main.c` only).
2. **The comment is three lines, not one.** It also records why a wake is not a transition. The code is a one-line body.
3. **Commit message trailers.** Both commits carry the session's two attribution trailer lines after the spec's title; the titles are verbatim.
4. **Capture filename date.** Named `2026-09-06-…` as instructed although the machine clock read 2026-09-05 18:19 CDT.

Everything else: none. No `scr_dial.c` change was needed for the standby-dimmed render — `dial_power` applies the STANDBY duty to the backlight regardless of screen, and `scr_dial.c` has no tier-dependent code path to add (its only "standby" is `NUM_STANDBY_OPA`, the zone-off opacity, unrelated to the power tier).

## Findings for the owner (§5-class, not code changes here)

1. **The night standby duty is the "Night clock" brightness, and it can be 0 = off.** `dial_power_night_clock_duty()` / `bri_night_clock_pct` "governs ONLY the night standby/screensaver duty" (`dial_power.h`), and the Settings picker for it is named for the clock (`scr_brightness.c` previews `DPWR_STANDBY` duty as "Night clock"). With the dial face showing at STANDBY, that same pref now sets how bright the **temperature** is at night — and the picker's "Off" (0 %) would blank the face the owner wants to glance at. Behaviour is consistent, the name is not. Worth a row rename ("Night standby" / "Night face brightness") or at least a note in commit 2's spec text; check on hardware that the current bench value (20 % per the sim default) reads well.
2. **The `ui_router.h` and `scr_updating.c` comments** still describe SCR_STANDBY as what STANDBY shows. Cosmetic; touch them in commit 2 alongside the pref.

## Not verified without hardware (spec §5 items 1, 2, 3, 5)

- **Day timeout:** let the dial idle past the Screen timeout in daylight — expect the full dial face (ring, setpoint, pill, power button) dimmed to the STANDBY duty, no clock.
- **Wake:** touch or turn — expect the face to brighten in place with no screen transition, and the first detent (or press) swallowed, not applied to the setpoint.
- **Night (Tokyo-timezone trick), Night face = Number only, side heating or cooling:** let it time out — expect the number-only face alternating with the water temperature every 2 s at the night dim floor (§5.1: confirm the swap is visible and the WATER word legible), the ambient "Update available" line at y=326 if an update is pending (§5.2), and the battery glyph on the face (§5.3).
- **Update prompt on first wake (§5.5):** with an update available (a newer beta than 0.1.6-beta.1 would have to exist), sleep past the timeout, wake — expect the update-prompt sheet on the first wake, since `ota_prompt_woke` compares consecutive tiers, not screens.
- **Finding 1 above** on the panel: the night standby brightness value that suits a temperature readout rather than a clock.
