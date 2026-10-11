# REPORT: Pad discovery on subnets wider than /24

**Verdict: PASS.** Commit `4e46454` on main. On a subnet wider than /24 the dial now scans two /24 blocks instead of skipping the scan. On eero's 192.168.4.0/22, 192.168.4.100 is candidate #1, whether the dial is at 192.168.4.37 or 192.168.5.20. On /24 and narrower, the candidate list is byte-identical to HEAD~1.

Date: 2026-10-04. Not tagged, not pushed, no release. This report is not part of the commit, because it contains the commit's SHA.

## Gate check

| Check | Result |
|---|---|
| `idf.py build` in `firmware/dial-idf` | **PASS** on the second build (exit 0, no warnings). The first build failed: `-Werror=maybe-uninitialized` on `blocks[0]`/`blocks[1]` in the new log call. I fixed it by zero-initialising `blocks[MAX_BLOCKS]` and rebuilt. Both logs are below. |
| Existing tests (`firmware/dial-idf/test/test_dial_rel.c`, the repo's only host test) | **PASS**: `all relative-scale table assertions passed`, exit 0 |
| Host-side unit tests for discovery | **None exist in the repo.** I didn't add one. I checked the cases from item 8 with a scratchpad harness instead (next section). |
| Scratchpad harness (not committed) | **PASS**: `all discovery candidate assertions passed`, exit 0 |

### About the harness

The repo has no discovery tests. To show "the candidates your code produces", the harness extracts the real `build_candidate_list()` code with awk. It takes everything from the `BLOCK_HOSTS`/`MAX_BLOCKS`/`MAX_HOSTS` defines and the first `typedef struct` down to `// Lenient best-effort`, from both the committed file and `git show HEAD~1:` of the same file, and compiles them side by side with `cc -Wall -Wextra`. It doesn't use a re-typed copy. For every case it asserts:

- no candidate is the dial's own IP, the real network address or the real broadcast address;
- every candidate is inside the real subnet and inside one of the scanned blocks;
- there are no duplicates and the count is ≤ 512;
- the expected pad address is in the list.

It also checks that the new builder's output is byte-identical to HEAD~1's for /24, /25, /26, /30 and /32. On top of that it runs an exhaustive /24 sweep: dial at .1–.254 × 16 stored-pad values (none, then .17, .34 … .255), with no differences.

## Raw output

### Final `idf.py build` (exit 0), unfiltered

```
$ cd firmware/dial-idf && source ~/esp/esp-idf/export.sh && idf.py build
Executing action: all (aliases: build)
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target blank_ota_data
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
[ 15%] Built target __idf_esp_phy
[ 75%] Built target __idf_micro-ecc
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
[ 24%] Built target __idf_freertos
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
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
[ 54%] Built target __idf_esp_hal_rmt
[ 54%] Built target __idf_esp_driver_spi
[ 54%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_esp_hal_cam
[ 55%] Built target __idf_esp_driver_i2c
[ 55%] Built target __idf_esp_hal_twai
[ 55%] Built target __idf_esp_hal_pcnt
[ 56%] Built target __idf_esp_driver_gptimer
[ 57%] Built target __idf_unity
[ 57%] Built target __idf_esp_hal_mcpwm
[ 59%] Built target __idf_sdmmc
[ 59%] Built target __idf_console
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_esp_hal_lcd
[ 59%] Built target __idf_esp_eth
[ 59%] Built target __idf_esp_hid
[ 59%] Built target __idf_esp_driver_tsens
[ 59%] Built target __idf_esp_driver_touch_sens
[ 59%] Built target __idf_perfmon
[ 60%] Built target __idf_wear_levelling
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_rt
[ 61%] Built target __idf_protobuf-c
[ 62%] Built target __idf_spiffs
[ 62%] Built target __idf_dial_state
[ 62%] Built target __idf_dial_knob
[ 63%] Built target __idf_espressif__cjson
[ 63%] Built target __idf_dial_time
[ 63%] Built target __idf_cmock
[ 64%] Built target __idf_driver
[ 64%] Built target __idf_esp_driver_ledc
[ 65%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_esp_driver_mcpwm
[ 66%] Built target __idf_esp_driver_pcnt
[ 67%] Built target __idf_esp_driver_rmt
[ 67%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_dial_ota
[ 67%] Built target __idf_esp_driver_twai
[ 68%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_dial_somnus
[ 70%] Built target __idf_protocomm
[ 70%] Built target __idf_esp_lcd
[ 70%] Built target __idf_dial_net
[ 70%] Built target __idf_esp_driver_sdmmc
[ 70%] Built target __idf_espressif__esp_lcd_sh8601
[ 71%] Built target __idf_esp_local_ctrl
[ 72%] Building C object esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj
[ 72%] Built target __idf_fatfs
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Built target __idf_dial_display
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Linking C static library libdial_pad_discovery.a
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Built target __idf_i2c_bsp
[ 97%] Built target __idf_dial_haptics
[ 97%] Built target __idf_dial_power
[ 97%] Built target __idf_dial_pad_discovery
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
```

### First `idf.py build` (failed, before the `= { 0 }` fix), unfiltered

```
Executing action: all (aliases: build)
Running make in directory /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
[  0%] Built target _project_elf_src
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target partition_table_bin
[  0%] Built target blank_ota_data
[  0%] Performing build step for 'bootloader'
[  0%] Built target __idf_esp_https_ota
[  0%] Built target __idf_esp_http_server
[  1%] Built target bootloader_ld_in_preprocess
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
[100%] Built target bootloader_check_size
[ 24%] Built target __idf_freertos
[100%] Built target app
[ 24%] No install step for 'bootloader'
[ 27%] Built target __idf_esp_hw_support
[ 27%] Completed 'bootloader'
[ 28%] Built target __idf_esp_hal_i2s
[ 29%] Built target bootloader
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
[ 54%] Built target __idf_esp_driver_i2c
[ 54%] Built target __idf_esp_hal_pcnt
[ 55%] Built target __idf_esp_hal_cam
[ 55%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_esp_hal_mcpwm
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_esp_hal_rmt
[ 57%] Built target __idf_sdmmc
[ 57%] Built target __idf_esp_hal_twai
[ 58%] Built target __idf_console
[ 59%] Built target __idf_unity
[ 59%] Built target __idf_esp_hal_lcd
[ 59%] Built target __idf_esp_driver_touch_sens
[ 59%] Built target __idf_esp_driver_tsens
[ 59%] Built target __idf_esp_driver_sdm
[ 59%] Built target __idf_perfmon
[ 59%] Built target __idf_rt
[ 59%] Built target __idf_esp_hid
[ 60%] Built target __idf_esp_https_server
[ 61%] Built target __idf_wear_levelling
[ 61%] Built target __idf_protobuf-c
[ 62%] Built target __idf_spiffs
[ 63%] Built target __idf_espressif__cjson
[ 63%] Built target __idf_dial_state
[ 63%] Built target __idf_cmock
[ 63%] Built target __idf_dial_time
[ 63%] Built target __idf_dial_knob
[ 64%] Built target __idf_driver
[ 64%] Built target __idf_esp_driver_pcnt
[ 64%] Built target __idf_esp_driver_ledc
[ 65%] Built target __idf_esp_driver_mcpwm
[ 66%] Built target __idf_esp_driver_cam
[ 67%] Built target __idf_esp_driver_rmt
[ 67%] Built target __idf_esp_driver_sd_intf
[ 67%] Built target __idf_esp_driver_twai
[ 68%] Built target __idf_esp_driver_sdspi
[ 68%] Built target __idf_esp_eth
[ 69%] Built target __idf_esp_lcd
[ 69%] Built target __idf_dial_somnus
[ 69%] Built target __idf_dial_ota
[ 69%] Built target __idf_dial_net
[ 70%] Built target __idf_protocomm
[ 70%] Built target __idf_esp_driver_sdmmc
[ 70%] Built target __idf_espressif__esp_lcd_sh8601
[ 71%] Building C object esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj
[ 72%] Built target __idf_esp_local_ctrl
[ 72%] Built target __idf_fatfs
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Built target __idf_dial_display
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Built target __idf_i2c_bsp
In file included from /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:9:
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c: In function 'dial_pad_discovery_scan':
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:462:39: error: 'blocks[0]' may be used uninitialized [-Werror=maybe-uninitialized]
  462 |                  (unsigned)(blocks[0] >> 8) & 0xFF);
      |                            ~~~~~~~~~~~^~~~~
/Users/matthew/esp/esp-idf/components/log/include/esp_log.h:224:167: note: in definition of macro 'ESP_LOG_LEVEL'
  224 |         else                                                  { esp_log(ESP_LOG_CONFIG_INIT(ESP_LOG_INFO),    tag, LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
      |                                                                                                                                                                       ^~~~~~~~~~~
/Users/matthew/esp/esp-idf/components/log/include/esp_log.h:127:41: note: in expansion of macro 'ESP_LOG_LEVEL_LOCAL'
  127 | #define ESP_LOGI(tag, format, ...) do { ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, tag, format, ##__VA_ARGS__); } while(0)
      |                                         ^~~~~~~~~~~~~~~~~~~
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:458:9: note: in expansion of macro 'ESP_LOGI'
  458 |         ESP_LOGI(TAG, "scan: subnet %u.%u.%u.%u/%d wider than /24, scanning block %u.%u.%u.0/24",
      |         ^~~~~~~~
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:442:14: note: 'blocks[0]' was declared here
  442 |     uint32_t blocks[MAX_BLOCKS];
      |              ^~~~~~
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:470:39: error: 'blocks[1]' may be used uninitialized [-Werror=maybe-uninitialized]
  470 |                  (unsigned)(blocks[1] >> 8) & 0xFF);
      |                            ~~~~~~~~~~~^~~~~
/Users/matthew/esp/esp-idf/components/log/include/esp_log.h:224:167: note: in definition of macro 'ESP_LOG_LEVEL'
  224 |         else                                                  { esp_log(ESP_LOG_CONFIG_INIT(ESP_LOG_INFO),    tag, LOG_FORMAT(I, format), esp_log_timestamp(), tag, ##__VA_ARGS__); } \
      |                                                                                                                                                                       ^~~~~~~~~~~
/Users/matthew/esp/esp-idf/components/log/include/esp_log.h:127:41: note: in expansion of macro 'ESP_LOG_LEVEL_LOCAL'
  127 | #define ESP_LOGI(tag, format, ...) do { ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO, tag, format, ##__VA_ARGS__); } while(0)
      |                                         ^~~~~~~~~~~~~~~~~~~
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:464:9: note: in expansion of macro 'ESP_LOGI'
  464 |         ESP_LOGI(TAG, "scan: subnet %u.%u.%u.%u/%d wider than /24, scanning blocks %u.%u.%u.0/24 and %u.%u.%u.0/24",
      |         ^~~~~~~~
/Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery/dial_pad_discovery.c:442:14: note: 'blocks[1]' was declared here
  442 |     uint32_t blocks[MAX_BLOCKS];
      |              ^~~~~~
[ 97%] Built target __idf_dial_haptics
cc1: all warnings being treated as errors
make[2]: *** [esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj] Error 1
make[1]: *** [esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/all] Error 2
make[1]: *** Waiting for unfinished jobs....
[ 97%] Built target __idf_dial_power
make: *** [all] Error 2
make failed with exit code 2, output of the command is in the /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/log/idf_py_stderr_output_85374 and /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/log/idf_py_stdout_output_85374
```

### Existing host test

```
$ cc -I components/dial_state -Wall -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
all relative-scale table assertions passed
exit 0
```

### Discovery harness (compile, then run), unfiltered

```
$ cc -Wall -Wextra -o harness harness.c && ./harness
harness cc exit 0

== A /22, dial 192.168.4.37: dial 192.168.4.37 mask 255.255.252.0 (/22), no stored pad
network 192.168.4.0 broadcast 192.168.7.255; blocks: 192.168.4.0/24; candidates 254
first 30:
   1 192.168.4.100
   2 192.168.4.101
   3 192.168.4.102
   4 192.168.4.103
   5 192.168.4.104
   6 192.168.4.105
   7 192.168.4.106
   8 192.168.4.107
   9 192.168.4.108
  10 192.168.4.109
  11 192.168.4.110
  12 192.168.4.111
  13 192.168.4.112
  14 192.168.4.113
  15 192.168.4.114
  16 192.168.4.115
  17 192.168.4.116
  18 192.168.4.117
  19 192.168.4.118
  20 192.168.4.119
  21 192.168.4.120
  22 192.168.4.121
  23 192.168.4.122
  24 192.168.4.123
  25 192.168.4.124
  26 192.168.4.125
  27 192.168.4.126
  28 192.168.4.127
  29 192.168.4.128
  30 192.168.4.129
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.4.27 .. 192.168.4.36
   82- 91: 192.168.4.38 .. 192.168.4.47
   92- 97: 192.168.4.21 .. 192.168.4.26
   98-149: 192.168.4.48 .. 192.168.4.99
  150-254: 192.168.4.151 .. 192.168.4.255
192.168.4.100 at position 1 of 254

== B /22, dial 192.168.5.20: dial 192.168.5.20 mask 255.255.252.0 (/22), no stored pad
network 192.168.4.0 broadcast 192.168.7.255; blocks: 192.168.4.0/24 192.168.5.0/24; candidates 510
first 30:
   1 192.168.4.100
   2 192.168.4.101
   3 192.168.4.102
   4 192.168.4.103
   5 192.168.4.104
   6 192.168.4.105
   7 192.168.4.106
   8 192.168.4.107
   9 192.168.4.108
  10 192.168.4.109
  11 192.168.4.110
  12 192.168.4.111
  13 192.168.4.112
  14 192.168.4.113
  15 192.168.4.114
  16 192.168.4.115
  17 192.168.4.116
  18 192.168.4.117
  19 192.168.4.118
  20 192.168.4.119
  21 192.168.4.120
  22 192.168.4.121
  23 192.168.4.122
  24 192.168.4.123
  25 192.168.4.124
  26 192.168.4.125
  27 192.168.4.126
  28 192.168.4.127
  29 192.168.4.128
  30 192.168.4.129
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.5.10 .. 192.168.5.19
   82- 91: 192.168.5.21 .. 192.168.5.30
   92-170: 192.168.4.21 .. 192.168.4.99
  171-285: 192.168.4.151 .. 192.168.5.9
  286-510: 192.168.5.31 .. 192.168.5.255
192.168.4.100 at position 1 of 510

== C /16, dial 10.0.37.5: dial 10.0.37.5 mask 255.255.0.0 (/16), no stored pad
network 10.0.0.0 broadcast 10.0.255.255; blocks: 10.0.0.0/24 10.0.37.0/24; candidates 510
first 30:
   1 10.0.0.100
   2 10.0.0.101
   3 10.0.0.102
   4 10.0.0.103
   5 10.0.0.104
   6 10.0.0.105
   7 10.0.0.106
   8 10.0.0.107
   9 10.0.0.108
  10 10.0.0.109
  11 10.0.0.110
  12 10.0.0.111
  13 10.0.0.112
  14 10.0.0.113
  15 10.0.0.114
  16 10.0.0.115
  17 10.0.0.116
  18 10.0.0.117
  19 10.0.0.118
  20 10.0.0.119
  21 10.0.0.120
  22 10.0.0.121
  23 10.0.0.122
  24 10.0.0.123
  25 10.0.0.124
  26 10.0.0.125
  27 10.0.0.126
  28 10.0.0.127
  29 10.0.0.128
  30 10.0.0.129
whole list as ascending runs (position: range):
    1- 51: 10.0.0.100 .. 10.0.0.150
   52- 71: 10.0.0.1 .. 10.0.0.20
   72- 76: 10.0.37.0 .. 10.0.37.4
   77- 86: 10.0.37.6 .. 10.0.37.15
   87-165: 10.0.0.21 .. 10.0.0.99
  166-270: 10.0.0.151 .. 10.0.0.255
  271-510: 10.0.37.16 .. 10.0.37.255
10.0.0.100 at position 1 of 510

== D /24, dial 192.168.1.37: dial 192.168.1.37 mask 255.255.255.0 (/24), no stored pad
network 192.168.1.0 broadcast 192.168.1.255; blocks: 192.168.1.0/24; candidates 253
first 30:
   1 192.168.1.27
   2 192.168.1.28
   3 192.168.1.29
   4 192.168.1.30
   5 192.168.1.31
   6 192.168.1.32
   7 192.168.1.33
   8 192.168.1.34
   9 192.168.1.35
  10 192.168.1.36
  11 192.168.1.38
  12 192.168.1.39
  13 192.168.1.40
  14 192.168.1.41
  15 192.168.1.42
  16 192.168.1.43
  17 192.168.1.44
  18 192.168.1.45
  19 192.168.1.46
  20 192.168.1.47
  21 192.168.1.1
  22 192.168.1.2
  23 192.168.1.3
  24 192.168.1.4
  25 192.168.1.5
  26 192.168.1.6
  27 192.168.1.7
  28 192.168.1.8
  29 192.168.1.9
  30 192.168.1.10
whole list as ascending runs (position: range):
    1- 10: 192.168.1.27 .. 192.168.1.36
   11- 20: 192.168.1.38 .. 192.168.1.47
   21- 40: 192.168.1.1 .. 192.168.1.20
   41- 91: 192.168.1.100 .. 192.168.1.150
   92- 97: 192.168.1.21 .. 192.168.1.26
   98-149: 192.168.1.48 .. 192.168.1.99
  150-253: 192.168.1.151 .. 192.168.1.254
192.168.1.100 at position 41 of 253

== E /22, dial 192.168.5.20, stored 192.168.4.100: dial 192.168.5.20 mask 255.255.252.0 (/22), stored pad 192.168.4.100
network 192.168.4.0 broadcast 192.168.7.255; blocks: 192.168.4.0/24 192.168.5.0/24; candidates 510
first 30:
   1 192.168.4.100
   2 192.168.4.95
   3 192.168.4.96
   4 192.168.4.97
   5 192.168.4.98
   6 192.168.4.99
   7 192.168.4.101
   8 192.168.4.102
   9 192.168.4.103
  10 192.168.4.104
  11 192.168.4.105
  12 192.168.4.106
  13 192.168.4.107
  14 192.168.4.108
  15 192.168.4.109
  16 192.168.4.110
  17 192.168.4.111
  18 192.168.4.112
  19 192.168.4.113
  20 192.168.4.114
  21 192.168.4.115
  22 192.168.4.116
  23 192.168.4.117
  24 192.168.4.118
  25 192.168.4.119
  26 192.168.4.120
  27 192.168.4.121
  28 192.168.4.122
  29 192.168.4.123
  30 192.168.4.124
whole list as ascending runs (position: range):
    1-  1: 192.168.4.100
    2-  6: 192.168.4.95 .. 192.168.4.99
    7- 56: 192.168.4.101 .. 192.168.4.150
   57- 76: 192.168.4.1 .. 192.168.4.20
   77- 86: 192.168.5.10 .. 192.168.5.19
   87- 96: 192.168.5.21 .. 192.168.5.30
   97-170: 192.168.4.21 .. 192.168.4.94
  171-285: 192.168.4.151 .. 192.168.5.9
  286-510: 192.168.5.31 .. 192.168.5.255
192.168.4.100 at position 1 of 510

== F /22, dial 192.168.7.250 (last block): dial 192.168.7.250 mask 255.255.252.0 (/22), no stored pad
network 192.168.4.0 broadcast 192.168.7.255; blocks: 192.168.4.0/24 192.168.7.0/24; candidates 509
first 30:
   1 192.168.4.100
   2 192.168.4.101
   3 192.168.4.102
   4 192.168.4.103
   5 192.168.4.104
   6 192.168.4.105
   7 192.168.4.106
   8 192.168.4.107
   9 192.168.4.108
  10 192.168.4.109
  11 192.168.4.110
  12 192.168.4.111
  13 192.168.4.112
  14 192.168.4.113
  15 192.168.4.114
  16 192.168.4.115
  17 192.168.4.116
  18 192.168.4.117
  19 192.168.4.118
  20 192.168.4.119
  21 192.168.4.120
  22 192.168.4.121
  23 192.168.4.122
  24 192.168.4.123
  25 192.168.4.124
  26 192.168.4.125
  27 192.168.4.126
  28 192.168.4.127
  29 192.168.4.128
  30 192.168.4.129
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.7.240 .. 192.168.7.249
   82- 85: 192.168.7.251 .. 192.168.7.254
   86-164: 192.168.4.21 .. 192.168.4.99
  165-269: 192.168.4.151 .. 192.168.4.255
  270-509: 192.168.7.0 .. 192.168.7.239
192.168.4.100 at position 1 of 509

== G /22, dial 192.168.5.20, stored 192.168.6.50 (outside blocks): dial 192.168.5.20 mask 255.255.252.0 (/22), stored pad 192.168.6.50
network 192.168.4.0 broadcast 192.168.7.255; blocks: 192.168.4.0/24 192.168.5.0/24; candidates 510
first 30:
   1 192.168.4.100
   2 192.168.4.101
   3 192.168.4.102
   4 192.168.4.103
   5 192.168.4.104
   6 192.168.4.105
   7 192.168.4.106
   8 192.168.4.107
   9 192.168.4.108
  10 192.168.4.109
  11 192.168.4.110
  12 192.168.4.111
  13 192.168.4.112
  14 192.168.4.113
  15 192.168.4.114
  16 192.168.4.115
  17 192.168.4.116
  18 192.168.4.117
  19 192.168.4.118
  20 192.168.4.119
  21 192.168.4.120
  22 192.168.4.121
  23 192.168.4.122
  24 192.168.4.123
  25 192.168.4.124
  26 192.168.4.125
  27 192.168.4.126
  28 192.168.4.127
  29 192.168.4.128
  30 192.168.4.129
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.5.10 .. 192.168.5.19
   82- 91: 192.168.5.21 .. 192.168.5.30
   92-170: 192.168.4.21 .. 192.168.4.99
  171-285: 192.168.4.151 .. 192.168.5.9
  286-510: 192.168.5.31 .. 192.168.5.255
192.168.4.100 at position 1 of 510

== H /23, dial 192.168.1.3: dial 192.168.1.3 mask 255.255.254.0 (/23), no stored pad
network 192.168.0.0 broadcast 192.168.1.255; blocks: 192.168.0.0/24 192.168.1.0/24; candidates 509
first 30:
   1 192.168.0.100
   2 192.168.0.101
   3 192.168.0.102
   4 192.168.0.103
   5 192.168.0.104
   6 192.168.0.105
   7 192.168.0.106
   8 192.168.0.107
   9 192.168.0.108
  10 192.168.0.109
  11 192.168.0.110
  12 192.168.0.111
  13 192.168.0.112
  14 192.168.0.113
  15 192.168.0.114
  16 192.168.0.115
  17 192.168.0.116
  18 192.168.0.117
  19 192.168.0.118
  20 192.168.0.119
  21 192.168.0.120
  22 192.168.0.121
  23 192.168.0.122
  24 192.168.0.123
  25 192.168.0.124
  26 192.168.0.125
  27 192.168.0.126
  28 192.168.0.127
  29 192.168.0.128
  30 192.168.0.129
whole list as ascending runs (position: range):
    1- 51: 192.168.0.100 .. 192.168.0.150
   52- 71: 192.168.0.1 .. 192.168.0.20
   72- 81: 192.168.0.249 .. 192.168.1.2
   82- 91: 192.168.1.4 .. 192.168.1.13
   92-170: 192.168.0.21 .. 192.168.0.99
  171-268: 192.168.0.151 .. 192.168.0.248
  269-509: 192.168.1.14 .. 192.168.1.254
192.168.0.100 at position 1 of 509

== I /8, dial 10.200.3.4: dial 10.200.3.4 mask 255.0.0.0 (/8), no stored pad
network 10.0.0.0 broadcast 10.255.255.255; blocks: 10.0.0.0/24 10.200.3.0/24; candidates 510
first 30:
   1 10.0.0.100
   2 10.0.0.101
   3 10.0.0.102
   4 10.0.0.103
   5 10.0.0.104
   6 10.0.0.105
   7 10.0.0.106
   8 10.0.0.107
   9 10.0.0.108
  10 10.0.0.109
  11 10.0.0.110
  12 10.0.0.111
  13 10.0.0.112
  14 10.0.0.113
  15 10.0.0.114
  16 10.0.0.115
  17 10.0.0.116
  18 10.0.0.117
  19 10.0.0.118
  20 10.0.0.119
  21 10.0.0.120
  22 10.0.0.121
  23 10.0.0.122
  24 10.0.0.123
  25 10.0.0.124
  26 10.0.0.125
  27 10.0.0.126
  28 10.0.0.127
  29 10.0.0.128
  30 10.0.0.129
whole list as ascending runs (position: range):
    1- 51: 10.0.0.100 .. 10.0.0.150
   52- 71: 10.0.0.1 .. 10.0.0.20
   72- 75: 10.200.3.0 .. 10.200.3.3
   76- 85: 10.200.3.5 .. 10.200.3.14
   86-164: 10.0.0.21 .. 10.0.0.99
  165-269: 10.0.0.151 .. 10.0.0.255
  270-510: 10.200.3.15 .. 10.200.3.255
10.0.0.100 at position 1 of 510

old-vs-new /24 dial .37                             old  253 new  253 IDENTICAL
old-vs-new /24 dial .37 stored .100                 old  253 new  253 IDENTICAL
old-vs-new /24 dial .2 stored .254                  old  253 new  253 IDENTICAL
old-vs-new /24 dial .254 stored .1                  old  253 new  253 IDENTICAL
old-vs-new /24 dial .120                            old  253 new  253 IDENTICAL
old-vs-new /25 dial .200                            old  125 new  125 IDENTICAL
old-vs-new /26 dial .70 stored .66                  old   61 new   61 IDENTICAL
old-vs-new /30 dial .2                              old    1 new    1 IDENTICAL
old-vs-new /32                                      old   -1 new   -1 IDENTICAL
/24 exhaustive sweep (self .1-.254 x 16 stored values) compared

all discovery candidate assertions passed
harness exit 0
```

## Diff stat and commit

```
$ git diff --stat HEAD~1 HEAD
 docs/ARCHITECTURE.md                               |   5 +-
 docs/SPEC-pad-discovery.md                         |  48 +++++--
 .../dial_pad_discovery/dial_pad_discovery.c        | 155 ++++++++++++++++-----
 .../dial_pad_discovery/dial_pad_discovery.h        |   3 +-
 4 files changed, 164 insertions(+), 47 deletions(-)
```

Commit SHA: `4e46454` (`dial: pad discovery scans two /24 blocks on subnets wider than /24`), parent `19fab5a`.

## Candidate lists, worked through

How the wide-subnet order is built (`build_candidate_list()`):

- **First block:** `network & 0xFFFFFF00`.
- **Dial's block:** `self & 0xFFFFFF00`, added only if it is different from the first block.
- **Tiers, in order:**
  1. Tier 0: stored pad ± 5, if the stored address is inside the real subnet.
  2. First block, .100–.150.
  3. First block, .1–.20.
  4. Dial ± 10.
  5. The rest of the first block, ascending.
  6. The rest of the dial's block, ascending.
- **Filtering:** each address goes through `add_ip`. It drops anything ≤ network or ≥ broadcast, anything outside both blocks, anything already added, and the dial itself.

### Case 1: /22, dial 192.168.4.37, no stored pad (eero, dial in the first block)

- Network 192.168.4.0, broadcast 192.168.7.255, mask 255.255.252.0.
- The dial's own /24 is the first block, so only **192.168.4.0/24** is scanned.
- 255 hosts (.1–.255; .255 is an ordinary host on a /22), minus the dial, gives **254 candidates**.
- **192.168.4.100 is candidate #1.**
- Positions 1–51 are .100–.150. Positions 52–71 are .1–.20. The dial ± 10 tier (.27–.47, minus .37) comes next at 72–91, then the rest ascending.

| # | candidate | tier |
|---|---|---|
| 1 | 192.168.4.100 | first block .100–.150 |
| 2 | 192.168.4.101 | first block .100–.150 |
| 3 | 192.168.4.102 | first block .100–.150 |
| 4 | 192.168.4.103 | first block .100–.150 |
| 5 | 192.168.4.104 | first block .100–.150 |
| 6 | 192.168.4.105 | first block .100–.150 |
| 7 | 192.168.4.106 | first block .100–.150 |
| 8 | 192.168.4.107 | first block .100–.150 |
| 9 | 192.168.4.108 | first block .100–.150 |
| 10 | 192.168.4.109 | first block .100–.150 |
| 11 | 192.168.4.110 | first block .100–.150 |
| 12 | 192.168.4.111 | first block .100–.150 |
| 13 | 192.168.4.112 | first block .100–.150 |
| 14 | 192.168.4.113 | first block .100–.150 |
| 15 | 192.168.4.114 | first block .100–.150 |
| 16 | 192.168.4.115 | first block .100–.150 |
| 17 | 192.168.4.116 | first block .100–.150 |
| 18 | 192.168.4.117 | first block .100–.150 |
| 19 | 192.168.4.118 | first block .100–.150 |
| 20 | 192.168.4.119 | first block .100–.150 |
| 21 | 192.168.4.120 | first block .100–.150 |
| 22 | 192.168.4.121 | first block .100–.150 |
| 23 | 192.168.4.122 | first block .100–.150 |
| 24 | 192.168.4.123 | first block .100–.150 |
| 25 | 192.168.4.124 | first block .100–.150 |
| 26 | 192.168.4.125 | first block .100–.150 |
| 27 | 192.168.4.126 | first block .100–.150 |
| 28 | 192.168.4.127 | first block .100–.150 |
| 29 | 192.168.4.128 | first block .100–.150 |
| 30 | 192.168.4.129 | first block .100–.150 |

```
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.4.27 .. 192.168.4.36
   82- 91: 192.168.4.38 .. 192.168.4.47
   92- 97: 192.168.4.21 .. 192.168.4.26
   98-149: 192.168.4.48 .. 192.168.4.99
  150-254: 192.168.4.151 .. 192.168.4.255
192.168.4.100 at position 1 of 254
```

### Case 2: /22, dial 192.168.5.20, no stored pad (eero, dial in the second block)

- Blocks scanned: **192.168.4.0/24** and **192.168.5.0/24**.
- Block 1 has 255 hosts. Block 2 has 256 (192.168.5.0 and .255 are ordinary hosts on a /22), minus the dial. Total **510 candidates**.
- **192.168.4.100 is still candidate #1.**
- Tiers: .100–.150 at 1–51, .1–.20 at 52–71, then the dial ± 10 at 72–91 (192.168.5.10–.30, minus .20). After that come 192.168.4.21–.99, then 192.168.4.151–.255, then 192.168.5.0–.9 (positions 171–285 are one ascending run, because 4.255 is followed by 5.0), then 192.168.5.31–.255.

| # | candidate | tier |
|---|---|---|
| 1 | 192.168.4.100 | first block .100–.150 |
| 2 | 192.168.4.101 | first block .100–.150 |
| 3 | 192.168.4.102 | first block .100–.150 |
| 4 | 192.168.4.103 | first block .100–.150 |
| 5 | 192.168.4.104 | first block .100–.150 |
| 6 | 192.168.4.105 | first block .100–.150 |
| 7 | 192.168.4.106 | first block .100–.150 |
| 8 | 192.168.4.107 | first block .100–.150 |
| 9 | 192.168.4.108 | first block .100–.150 |
| 10 | 192.168.4.109 | first block .100–.150 |
| 11 | 192.168.4.110 | first block .100–.150 |
| 12 | 192.168.4.111 | first block .100–.150 |
| 13 | 192.168.4.112 | first block .100–.150 |
| 14 | 192.168.4.113 | first block .100–.150 |
| 15 | 192.168.4.114 | first block .100–.150 |
| 16 | 192.168.4.115 | first block .100–.150 |
| 17 | 192.168.4.116 | first block .100–.150 |
| 18 | 192.168.4.117 | first block .100–.150 |
| 19 | 192.168.4.118 | first block .100–.150 |
| 20 | 192.168.4.119 | first block .100–.150 |
| 21 | 192.168.4.120 | first block .100–.150 |
| 22 | 192.168.4.121 | first block .100–.150 |
| 23 | 192.168.4.122 | first block .100–.150 |
| 24 | 192.168.4.123 | first block .100–.150 |
| 25 | 192.168.4.124 | first block .100–.150 |
| 26 | 192.168.4.125 | first block .100–.150 |
| 27 | 192.168.4.126 | first block .100–.150 |
| 28 | 192.168.4.127 | first block .100–.150 |
| 29 | 192.168.4.128 | first block .100–.150 |
| 30 | 192.168.4.129 | first block .100–.150 |

```
whole list as ascending runs (position: range):
    1- 51: 192.168.4.100 .. 192.168.4.150
   52- 71: 192.168.4.1 .. 192.168.4.20
   72- 81: 192.168.5.10 .. 192.168.5.19
   82- 91: 192.168.5.21 .. 192.168.5.30
   92-170: 192.168.4.21 .. 192.168.4.99
  171-285: 192.168.4.151 .. 192.168.5.9
  286-510: 192.168.5.31 .. 192.168.5.255
192.168.4.100 at position 1 of 510
```

### Case 3: /16, dial 10.0.37.5, no stored pad

- Network 10.0.0.0, broadcast 10.0.255.255.
- Only **10.0.0.0/24** and **10.0.37.0/24** are scanned, not the other 254 blocks.
- **510 candidates.** 10.0.0.100 is candidate #1.
- The dial ± 10 tier is 10.0.37.0–.15 minus .5 (positions 72–86). Low end: 10.0.37.0 is a valid host on a /16, and the clamp only stops at the real network address. High end: 37.15 = 37.5 + 10.

I also ran 192.168.x /16 and /8 variants. The /8 case is in the raw harness output (case I): it scans 10.0.0.0/24 and 10.200.3.0/24, 510 candidates.

| # | candidate | tier |
|---|---|---|
| 1 | 10.0.0.100 | first block .100–.150 |
| 2 | 10.0.0.101 | first block .100–.150 |
| 3 | 10.0.0.102 | first block .100–.150 |
| 4 | 10.0.0.103 | first block .100–.150 |
| 5 | 10.0.0.104 | first block .100–.150 |
| 6 | 10.0.0.105 | first block .100–.150 |
| 7 | 10.0.0.106 | first block .100–.150 |
| 8 | 10.0.0.107 | first block .100–.150 |
| 9 | 10.0.0.108 | first block .100–.150 |
| 10 | 10.0.0.109 | first block .100–.150 |
| 11 | 10.0.0.110 | first block .100–.150 |
| 12 | 10.0.0.111 | first block .100–.150 |
| 13 | 10.0.0.112 | first block .100–.150 |
| 14 | 10.0.0.113 | first block .100–.150 |
| 15 | 10.0.0.114 | first block .100–.150 |
| 16 | 10.0.0.115 | first block .100–.150 |
| 17 | 10.0.0.116 | first block .100–.150 |
| 18 | 10.0.0.117 | first block .100–.150 |
| 19 | 10.0.0.118 | first block .100–.150 |
| 20 | 10.0.0.119 | first block .100–.150 |
| 21 | 10.0.0.120 | first block .100–.150 |
| 22 | 10.0.0.121 | first block .100–.150 |
| 23 | 10.0.0.122 | first block .100–.150 |
| 24 | 10.0.0.123 | first block .100–.150 |
| 25 | 10.0.0.124 | first block .100–.150 |
| 26 | 10.0.0.125 | first block .100–.150 |
| 27 | 10.0.0.126 | first block .100–.150 |
| 28 | 10.0.0.127 | first block .100–.150 |
| 29 | 10.0.0.128 | first block .100–.150 |
| 30 | 10.0.0.129 | first block .100–.150 |

```
whole list as ascending runs (position: range):
    1- 51: 10.0.0.100 .. 10.0.0.150
   52- 71: 10.0.0.1 .. 10.0.0.20
   72- 76: 10.0.37.0 .. 10.0.37.4
   77- 86: 10.0.37.6 .. 10.0.37.15
   87-165: 10.0.0.21 .. 10.0.0.99
  166-270: 10.0.0.151 .. 10.0.0.255
  271-510: 10.0.37.16 .. 10.0.37.255
10.0.0.100 at position 1 of 510
```

### Case 4: /24, dial 192.168.1.37, no stored pad (unchanged)

- This is the old path, untouched. Tier 1 (dial ± 10) comes first, then .1–.20, then .100–.150, then the rest.
- **253 candidates**, byte-identical to HEAD~1. That holds for this case and for the exhaustive /24 sweep.
- 192.168.1.100 is candidate #41, the same as before.

| # | candidate | tier |
|---|---|---|
| 1 | 192.168.1.27 | tier 1, dial ± 10 |
| 2 | 192.168.1.28 | tier 1, dial ± 10 |
| 3 | 192.168.1.29 | tier 1, dial ± 10 |
| 4 | 192.168.1.30 | tier 1, dial ± 10 |
| 5 | 192.168.1.31 | tier 1, dial ± 10 |
| 6 | 192.168.1.32 | tier 1, dial ± 10 |
| 7 | 192.168.1.33 | tier 1, dial ± 10 |
| 8 | 192.168.1.34 | tier 1, dial ± 10 |
| 9 | 192.168.1.35 | tier 1, dial ± 10 |
| 10 | 192.168.1.36 | tier 1, dial ± 10 |
| 11 | 192.168.1.38 | tier 1, dial ± 10 |
| 12 | 192.168.1.39 | tier 1, dial ± 10 |
| 13 | 192.168.1.40 | tier 1, dial ± 10 |
| 14 | 192.168.1.41 | tier 1, dial ± 10 |
| 15 | 192.168.1.42 | tier 1, dial ± 10 |
| 16 | 192.168.1.43 | tier 1, dial ± 10 |
| 17 | 192.168.1.44 | tier 1, dial ± 10 |
| 18 | 192.168.1.45 | tier 1, dial ± 10 |
| 19 | 192.168.1.46 | tier 1, dial ± 10 |
| 20 | 192.168.1.47 | tier 1, dial ± 10 |
| 21 | 192.168.1.1 | tier 2, .1–.20 |
| 22 | 192.168.1.2 | tier 2, .1–.20 |
| 23 | 192.168.1.3 | tier 2, .1–.20 |
| 24 | 192.168.1.4 | tier 2, .1–.20 |
| 25 | 192.168.1.5 | tier 2, .1–.20 |
| 26 | 192.168.1.6 | tier 2, .1–.20 |
| 27 | 192.168.1.7 | tier 2, .1–.20 |
| 28 | 192.168.1.8 | tier 2, .1–.20 |
| 29 | 192.168.1.9 | tier 2, .1–.20 |
| 30 | 192.168.1.10 | tier 2, .1–.20 |

```
whole list as ascending runs (position: range):
    1- 10: 192.168.1.27 .. 192.168.1.36
   11- 20: 192.168.1.38 .. 192.168.1.47
   21- 40: 192.168.1.1 .. 192.168.1.20
   41- 91: 192.168.1.100 .. 192.168.1.150
   92- 97: 192.168.1.21 .. 192.168.1.26
   98-149: 192.168.1.48 .. 192.168.1.99
  150-253: 192.168.1.151 .. 192.168.1.254
192.168.1.100 at position 41 of 253
```

### Extra cases (full lists in the raw harness output above)

| Case | Blocks scanned | Count | Notes |
|---|---|---|---|
| E: /22, dial 192.168.5.20, stored pad 192.168.4.100 | 4.0/24, 5.0/24 | 510 | Tier 0 first: 4.100, then 4.95–.99, then 4.101–.105. 4.100 is #1. |
| F: /22, dial 192.168.7.250 | 4.0/24, 7.0/24 | 509 | 192.168.7.255 (real broadcast) is excluded. The dial ± 10 tier is 7.240–.254 minus .250. |
| G: /22, dial 192.168.5.20, stored pad 192.168.6.50 | 4.0/24, 5.0/24 | 510 | The stored address is in neither block, so tier 0 adds nothing (see deviation 3). |
| H: /23, dial 192.168.1.3 | 0.0/24, 1.0/24 | 509 | The dial ± 10 tier crosses into the first block (0.249–0.255, 1.0–1.2). |
| I: /8, dial 10.200.3.4 | 10.0.0.0/24, 10.200.3.0/24 | 510 | Never more than two blocks. |

## Deviations from the spec

1. **"Near-self tier of block (b)" when there is no block (b).** If the dial is in the first block (case 1), I still run the dial ± 10 tier, in the slot the spec gives block (b)'s near-self tier: after the first block's .100–.150 and .1–.20. The spec says each block keeps "the existing tiers", and near-self is one of them.
2. **The near-self tier is clamped to the real subnet, not to the dial's own block.** So it can take addresses from the other scanned block (case H: 192.168.0.249–.255 for a dial at 192.168.1.3). Addresses outside both blocks are dropped, e.g. 192.168.6.x for a dial at 192.168.5.250.
3. **Tier 0 is limited to the two scanned blocks.** The spec says tier 0 applies "if inside the real subnet". I kept that condition, but stored-pad neighbours outside both blocks are dropped (case G: a stored 192.168.6.50 on eero is not probed). Two reasons:
   - item 3 says never scan more than the two blocks;
   - two full blocks already use 510 of the 512 slots, so up to 11 extra tier-0 addresses would silently push the tail of the second block past the cap.
4. **A wide subnet's interior .0 and .255 addresses are probed** (e.g. 192.168.4.255, 192.168.5.0, 10.0.37.0). They are ordinary hosts of the wider subnet. Item 4 excludes only the real subnet's network and broadcast addresses, which are never probed.
5. **New INFO log on /24 and narrower.** The candidate list is unchanged, but the scan now logs `scan: subnet A.B.C.D/P, scanning all of it` on every subnet size, so the log always names what is scanned (item 6). Wide subnets log `scan: subnet 192.168.4.0/22 wider than /24, scanning blocks 192.168.4.0/24 and 192.168.5.0/24` (or `scanning block …` when there is one block). The old skip warning is now `scan: subnet has no other host addresses, skipping`, which fires only for /31 and /32.
6. **Files changed beyond the two named:**
   - `dial_pad_discovery.h`: the worst-case-time comment now gives ~115 s for wide subnets.
   - `docs/ARCHITECTURE.md`: one paragraph still said "`/24` or tighter only, 256 hosts max".
   - Not changed: the historical 2026-09-01 lines in SPEC-pad-discovery.md that mention the "256-host cap". They are the design record.
7. **Implementation details the spec didn't specify:**
   - `build_candidate_list()` takes two new output parameters (`blocks_out`, `nblocks_out`) so the caller can log the blocks.
   - The builder struct (now 512 bytes of seen table) is `static`, not on the worker task's stack, following the existing `s_candidates` pattern.
   - `add_range()` no longer loops on `ip <= hi`, so a block ending at 0xFFFFFFFF can't loop forever.
8. **Item 8:** no discovery host tests exist, so none were added to the repo (as the spec allows). The cases ran in the uncommitted scratchpad harness described above.
9. **`docs/REPORTS.md`:** I added a line for this report, left uncommitted along with the report, following the standing convention that reports are not committed with the change.

## What can't be verified without hardware (a real dial on an eero /22)

- **Netmask.** That `dial_net_subnet()` really reports 255.255.252.0 on an eero network. The bug report implies it, but this change hasn't been seen on one.
- **Where eero leases.** That eero actually puts the pad in 192.168.4.0/24, and in which part of it. The ordering assumes leases start at the bottom of the range; eero's real lease pattern hasn't been observed. A pad in 192.168.6.x or 192.168.7.x with the dial in another block will not be found (by design).
- **Real scan time on 510 candidates.** The worst case is ~115 s (`ceil(510/4) × (0.3 + 0.6)` s), roughly double the /24 worst case. The real figure, and how that blocking time interacts with the connect loop, the 5-minute cooldown and the 60 s screen standby, are not measured.
- **Socket and heap stability.** Whether a ~2× longer sweep of short-lived `esp_http_client` instances stays stable on the device.
- **Progress screen.** Whether "N/510" fits the discovery screen's layout. I haven't checked it in the simulator either.
- **eero mesh behaviour.** Whether probes reach a pad attached to a different mesh node than the dial (ARP or client isolation across nodes).
- **Logs on the device.** That the new log lines appear as written in serial output.
