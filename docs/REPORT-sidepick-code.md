**CODE COMMITTED** — commit A `e14e671` (spec touch-up), commit B `fe5b139` (the change). Gate passed; host test, firmware build, identity checks and simulator all passed. Not pushed; no version bump, no CHANGELOG edit, no tag.

# REPORT: sidepick-code — SPEC-sidepick-deletion implemented (1.0.3-beta.1 change)

Date: 2026-09-23. Starting HEAD `a05e47a`. Task: implement
`docs/SPEC-sidepick-deletion.md` in two commits (A: spec touch-up, B: code).

## Gate

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2")
$ git log --oneline -1
a05e47a docs: SPEC-sidepick-deletion owner decisions, and the spec report with its REPORTS.md line
$ git --no-optional-locks status --short --untracked-files=no
$ grep -c "^## 9. Owner decisions (2026-09-23)" docs/SPEC-sidepick-deletion.md
1
```

All four passed (the status command printed nothing).

## Commit A — `e14e671`

`docs: SPEC-sidepick-deletion - one-dial bench order and the section 8 note`

- §8: the parenthetical now records that the owner kept the latent fix in
  scope (§9, Q1), so the "Also fixed" paragraph stays.
- §7: new "Order of the session" paragraph above the table (one dial, the
  owner's nightly dial; wire-flash with `idf.py flash`; T1, T1-r, T4, T4-r,
  T2, T2-a, T2-r, T2-z, Close; set up again for nightly use; T3 only after
  publication, after wire-flashing a `somnus-v1.0.2` build with settings
  kept). The T3 row now points at step 4 of that order.

```
$ git show --stat --format= e14e671
 docs/SPEC-sidepick-deletion.md | 23 +++++++++++++++++++----
 1 file changed, 19 insertions(+), 4 deletions(-)
```

## Commit B — `fe5b139`

`dial: delete SCR_SIDEPICK; seed relmode at the first zone write (1.0.3-beta.1 change)`

```
$ git show --stat --format= fe5b139
 docs/ARCHITECTURE.md                               |  11 +-
 docs/screens/sidepick.png                          | Bin 15393 -> 0 bytes
 firmware/dial-idf/README.md                        |   5 +-
 .../dial-idf/components/dial_state/dial_state.c    |  41 ++----
 .../dial-idf/components/dial_state/dial_state.h    |  20 ++-
 .../dial-idf/components/dial_ui/CMakeLists.txt     |   2 +-
 .../dial-idf/components/dial_ui/scr_settings.c     |   5 +-
 .../dial-idf/components/dial_ui/scr_sidepick.c     | 140 ---------------------
 firmware/dial-idf/components/dial_ui/ui_router.c   |   1 -
 firmware/dial-idf/components/dial_ui/ui_router.h   |   1 -
 firmware/dial-idf/components/dial_ui/ui_screens.c  |   1 -
 .../components/dial_ui/ui_screens_internal.h       |   1 -
 firmware/dial-idf/main/main.c                      |   8 --
 simulator/CMakeLists.txt                           |   1 -
 simulator/main.c                                   |  10 --
 simulator/sim_state.c                              |   8 +-
 16 files changed, 29 insertions(+), 226 deletions(-)
```

`dial_somnus.c` is not in the diff; no path that writes side1 was added or
changed. SCR_WELCOME, `fresh_device` and `welcomed` are untouched. The
upgrade-to-absolute rule in `dial_state_restore_prefs` is untouched.

### Final text of `dial_state_set_ui_zone`

```c
void dial_state_set_ui_zone(zone_idx_t zone)
{
    xSemaphoreTake(s_mux, portMAX_DELAY);
    bool changed = (s_state.ui_zone != zone);
    s_state.ui_zone = zone;
    bool rel = s_state.rel_mode;
    xSemaphoreGive(s_mux);
    // No generation bump: the caller is the screen that already navigated —
    // re-rendering here would race the transition it just started.

    // Last side chosen survives reboot (owner D3). Writes are rare (one per
    // swipe) and this runs in the LVGL task, where a ~ms NVS write is fine.
    if (changed) {
        nvs_handle_t h;
        if (nvs_open(NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
            // Seed "relmode" (if absent) before the first "zone": this is the
            // only writer of "zone", and dial_state_restore_prefs reads "zone"
            // without "relmode" as a device set up before the relative scale
            // and forces Absolute. Seeding the RAM value keeps a never-touched
            // Scale where it is across the reboot; writing it first means a
            // power loss between the two sets can't leave "zone" alone.
            uint8_t existing;
            if (nvs_get_u8(h, "relmode", &existing) != ESP_OK)
                nvs_set_u8(h, "relmode", rel ? 1 : 0);
            nvs_set_u8(h, "zone", (uint8_t)zone);
            nvs_commit(h);
            nvs_close(h);
        }
    }
}
```

### `git show` of the main.c hunk

(Shown with `--format=` so the commit header, which carries the author's
e-mail address, is left out of this public report.)

```diff
$ git show --format= fe5b139 -- firmware/dial-idf/main/main.c
diff --git a/firmware/dial-idf/main/main.c b/firmware/dial-idf/main/main.c
index 807cafd..603109e 100644
--- a/firmware/dial-idf/main/main.c
+++ b/firmware/dial-idf/main/main.c
@@ -421,14 +421,6 @@ static screen_id_t nav_policy(const app_state_t *st, void **arg)
                 cur == SCR_PAD_ADDRESS || cur == SCR_TIMEZONE ||
                 cur == SCR_STANDBY_FACE ||
                 cur == SCR_NIGHT_MODE || cur == SCR_NIGHT_FACE) return cur;
-            // First link on a fresh device: pick a default side before showing
-            // the dial (SCR_SIDEPICK). Nothing to pick on a single-zone topper,
-            // so that device goes straight to its one face. The `cur` half of
-            // the OR keeps a poll from yanking the user off the picker
-            // mid-decision.
-            if (dial_state_is_dual(st) &&
-                ((st->fresh_device && !st->side_picked) || cur == SCR_SIDEPICK))
-                return SCR_SIDEPICK;
             *arg = (void *)(uintptr_t)st->ui_zone;
             return dial_power_level() == DPWR_STANDBY ? standby_screen(st) : SCR_DIAL;
         }
```

## Step 3 — proof of absence

```
$ git grep -n -E "side_picked|SCR_SIDEPICK|scr_sidepick|set_side_picked|scenario_sidepick" -- firmware/ simulator/ README.md docs/ ':!docs/REPORT-*' ':!CHANGELOG*' ':!docs/SPEC-sidepick-deletion.md'
docs/PLAN-screen-layout-fixes.md:30:| A4 | `scr_sidepick.c` | Move the title's `lv_label_create` block to after the halves-and-divider loop (or add `lv_obj_move_foreground(s_title)`); y `TOP_MID 36` → `72`. | `sidepick.png` finally shows the title, inside the 288 px chord. |
docs/REVIEW-2026-09-02.md:39:     PAD_ADDRESS/TIMEZONE) returns `cur`, fresh dual device → `SCR_SIDEPICK`,
(exit 0)
```

Two hits remain, both in docs that spec §6 says to leave as written:
- `docs/PLAN-screen-layout-fixes.md:30` — an executed plan (layout fix A4),
  listed under "Checked and left alone".
- `docs/REVIEW-2026-09-02.md:39` — a historical review. §6 leaves all
  `docs/REVIEW-*.md` as written; the task's grep excludes `docs/REPORT-*`
  but not `docs/REVIEW-*`, which is why it shows up.

Nothing in `firmware/`, `simulator/` or `README.md` hits. For completeness,
spec §2's own grep now hits only the `fresh_device` lines it predicted
(line numbers in main.c moved up by 8):

```
$ git grep -n -E "side_picked|fresh_device|SCR_SIDEPICK|scr_sidepick|sidepick" -- firmware/ simulator/
firmware/dial-idf/components/dial_state/dial_state.h:487:    bool fresh_device;
firmware/dial-idf/components/dial_ui/scr_welcome.c:3: * Wi-Fi credentials at boot; dial_state's fresh_device flag, set once in
firmware/dial-idf/main/main.c:213:    if (st->fresh_device && !st->welcomed &&
firmware/dial-idf/main/main.c:551:static void mut_fresh_device(app_state_t *st, void *arg) { st->fresh_device = *(bool *)arg; }
firmware/dial-idf/main/main.c:579:// fresh_device (set here) / welcomed (cleared by scr_welcome.c).
firmware/dial-idf/main/main.c:1578:    dial_state_commit(mut_fresh_device, &fresh);
```

## Step 4 — host test

```
$ cd firmware/dial-idf
$ cc -I components/dial_state -Wall -o /tmp/test_dial_rel test/test_dial_rel.c -lm && /tmp/test_dial_rel
all relative-scale table assertions passed
(exit 0)
```

## Step 5 — firmware build (ESP-IDF v6.0)

Environment: `. ~/esp/esp-idf/export.sh` (what the `get-idf` alias points
to); `idf.py --version` printed `ESP-IDF v6.0`. The output below is the
complete build log with one substitution: the home-directory prefix is
written as `~` so no home path lands in this public repo. Nothing else was
removed or changed.

```
$ idf.py build
CMake Warning (deprecated) at CMakeLists.txt:5 (cmake_minimum_required):
  Compatibility with CMake < 3.10 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value.  Or, use the <min>...<max> syntax
  to tell CMake that the project requires at least <min> but has been updated
  to work with policies introduced by <max> or earlier.
This warning is for project developers.  Use -Wno-author or -Wno-deprecated
to suppress it.

Executing action: all (aliases: build)
Running make in directory ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
Executing "make -j 12 all"...
-- Found Git: /usr/bin/git (found version "2.54.0 (Apple Git-157)")
-- Component directory ~/esp/esp-idf/components/mqtt does not contain a CMakeLists.txt file. No component will be added
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
-- Project sdkconfig file ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig
NOTE: ~/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:235: FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/bt/sdkconfig.rename.esp32s3:4: duplicate rename mapping for CONFIG_BT_NIMBLE_COEX_PHY_CODED_TX_RX_TLIM_EN: previous target CONFIG_BT_LE_COEX_PHY_CODED_TX_RX_TLIM_EN, new target CONFIG_BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM_EN - last mapping is used
NOTE: ~/esp/esp-idf/components/bt/sdkconfig.rename.esp32s3:5: duplicate rename mapping for CONFIG_BT_NIMBLE_COEX_PHY_CODED_TX_RX_TLIM_DIS: previous target CONFIG_BT_LE_COEX_PHY_CODED_TX_RX_TLIM_DIS, new target CONFIG_BT_CTRL_COEX_PHY_CODED_TX_RX_TLIM_DIS - last mapping is used
Loading defaults file ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/sdkconfig.defaults...
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
-- App "somnus-dial" version: 1.0.2
-- Setting up mbedtls configuration
-- Linkage type is INTERFACE
-- Adding linker script ~/esp/esp-idf/components/esp_hal_wdt/esp32s3/rom.wdt.ld
-- Adding linker script ~/esp/esp-idf/components/esp_system/ld/esp32s3/memory.ld.in
--   -> Preprocessing .in script: ~/esp/esp-idf/components/esp_system/ld/esp32s3/memory.ld.in
-- Adding linker script ~/esp/esp-idf/components/esp_system/ld/esp32s3/sections.ld.in
--   -> Preprocessing .in script: ~/esp/esp-idf/components/esp_system/ld/esp32s3/sections.ld.in
--   -> Applying ldgen processing: ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/esp-idf/esp_system/ld/sections.ld.in
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.api.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.bt_funcs.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libgcc.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.version.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_master.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_50.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_smp.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_dtm.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_test.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.ble_scan.ld
-- Adding linker script ~/esp/esp-idf/components/esp_rom/esp32s3/ld/esp32s3.rom.libc.ld
-- Adding linker script ~/esp/esp-idf/components/soc/esp32s3/ld/esp32s3.peripherals.ld
-- ESP_LCD_SH8601: 2.0.1
-- Component idf::esp_trace will be linked with -Wl,--whole-archive
-- Components: app_trace app_update bootloader bootloader_support bt cmock console cxx dial_display dial_haptics dial_knob dial_net dial_ota dial_pad_discovery dial_power dial_somnus dial_state dial_time dial_ui driver efuse esp-tls esp_adc esp_app_format esp_blockdev esp_bootloader_format esp_coex esp_common esp_driver_ana_cmpr esp_driver_bitscrambler esp_driver_cam esp_driver_dac esp_driver_dma esp_driver_gpio esp_driver_gptimer esp_driver_i2c esp_driver_i2s esp_driver_i3c esp_driver_isp esp_driver_jpeg esp_driver_ledc esp_driver_mcpwm esp_driver_parlio esp_driver_pcnt esp_driver_ppa esp_driver_rmt esp_driver_sd_intf esp_driver_sdio esp_driver_sdm esp_driver_sdmmc esp_driver_sdspi esp_driver_spi esp_driver_touch_sens esp_driver_tsens esp_driver_twai esp_driver_uart esp_driver_usb_serial_jtag esp_eth esp_event esp_gdbstub esp_hal_ana_cmpr esp_hal_ana_conv esp_hal_cam esp_hal_clock esp_hal_dma esp_hal_gpio esp_hal_gpspi esp_hal_i2c esp_hal_i2s esp_hal_ieee802154 esp_hal_jpeg esp_hal_lcd esp_hal_ledc esp_hal_mcpwm esp_hal_mspi esp_hal_parlio esp_hal_pcnt esp_hal_pmu esp_hal_ppa esp_hal_rmt esp_hal_rtc_timer esp_hal_security esp_hal_timg esp_hal_touch_sens esp_hal_twai esp_hal_uart esp_hal_usb esp_hal_wdt esp_hid esp_http_client esp_http_server esp_https_ota esp_https_server esp_hw_support esp_lcd esp_libc esp_local_ctrl esp_mm esp_netif esp_netif_stack esp_partition esp_phy esp_pm esp_psram esp_ringbuf esp_rom esp_security esp_stdio esp_system esp_timer esp_trace esp_usb_cdc_rom_console esp_wifi espcoredump espressif__cjson espressif__cmake_utilities espressif__esp_lcd_sh8601 esptool_py fatfs freertos hal heap http_parser i2c_bsp idf_test ieee802154 lcd_bl_pwm_bsp lcd_touch_bsp log lvgl__lvgl lwip main mbedtls nvs_flash nvs_sec_provider openthread partition_table perfmon protobuf-c protocomm pthread rt sdmmc soc spi_flash spiffs tcp_transport ulp unity vfs wear_levelling wpa_supplicant xtensa
-- Component paths: ~/esp/esp-idf/components/app_trace ~/esp/esp-idf/components/app_update ~/esp/esp-idf/components/bootloader ~/esp/esp-idf/components/bootloader_support ~/esp/esp-idf/components/bt ~/esp/esp-idf/components/cmock ~/esp/esp-idf/components/console ~/esp/esp-idf/components/cxx ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_display ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_haptics ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_knob ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_net ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ota ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_pad_discovery ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_power ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_somnus ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_state ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_time ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui ~/esp/esp-idf/components/driver ~/esp/esp-idf/components/efuse ~/esp/esp-idf/components/esp-tls ~/esp/esp-idf/components/esp_adc ~/esp/esp-idf/components/esp_app_format ~/esp/esp-idf/components/esp_blockdev ~/esp/esp-idf/components/esp_bootloader_format ~/esp/esp-idf/components/esp_coex ~/esp/esp-idf/components/esp_common ~/esp/esp-idf/components/esp_driver_ana_cmpr ~/esp/esp-idf/components/esp_driver_bitscrambler ~/esp/esp-idf/components/esp_driver_cam ~/esp/esp-idf/components/esp_driver_dac ~/esp/esp-idf/components/esp_driver_dma ~/esp/esp-idf/components/esp_driver_gpio ~/esp/esp-idf/components/esp_driver_gptimer ~/esp/esp-idf/components/esp_driver_i2c ~/esp/esp-idf/components/esp_driver_i2s ~/esp/esp-idf/components/esp_driver_i3c ~/esp/esp-idf/components/esp_driver_isp ~/esp/esp-idf/components/esp_driver_jpeg ~/esp/esp-idf/components/esp_driver_ledc ~/esp/esp-idf/components/esp_driver_mcpwm ~/esp/esp-idf/components/esp_driver_parlio ~/esp/esp-idf/components/esp_driver_pcnt ~/esp/esp-idf/components/esp_driver_ppa ~/esp/esp-idf/components/esp_driver_rmt ~/esp/esp-idf/components/esp_driver_sd_intf ~/esp/esp-idf/components/esp_driver_sdio ~/esp/esp-idf/components/esp_driver_sdm ~/esp/esp-idf/components/esp_driver_sdmmc ~/esp/esp-idf/components/esp_driver_sdspi ~/esp/esp-idf/components/esp_driver_spi ~/esp/esp-idf/components/esp_driver_touch_sens ~/esp/esp-idf/components/esp_driver_tsens ~/esp/esp-idf/components/esp_driver_twai ~/esp/esp-idf/components/esp_driver_uart ~/esp/esp-idf/components/esp_driver_usb_serial_jtag ~/esp/esp-idf/components/esp_eth ~/esp/esp-idf/components/esp_event ~/esp/esp-idf/components/esp_gdbstub ~/esp/esp-idf/components/esp_hal_ana_cmpr ~/esp/esp-idf/components/esp_hal_ana_conv ~/esp/esp-idf/components/esp_hal_cam ~/esp/esp-idf/components/esp_hal_clock ~/esp/esp-idf/components/esp_hal_dma ~/esp/esp-idf/components/esp_hal_gpio ~/esp/esp-idf/components/esp_hal_gpspi ~/esp/esp-idf/components/esp_hal_i2c ~/esp/esp-idf/components/esp_hal_i2s ~/esp/esp-idf/components/esp_hal_ieee802154 ~/esp/esp-idf/components/esp_hal_jpeg ~/esp/esp-idf/components/esp_hal_lcd ~/esp/esp-idf/components/esp_hal_ledc ~/esp/esp-idf/components/esp_hal_mcpwm ~/esp/esp-idf/components/esp_hal_mspi ~/esp/esp-idf/components/esp_hal_parlio ~/esp/esp-idf/components/esp_hal_pcnt ~/esp/esp-idf/components/esp_hal_pmu ~/esp/esp-idf/components/esp_hal_ppa ~/esp/esp-idf/components/esp_hal_rmt ~/esp/esp-idf/components/esp_hal_rtc_timer ~/esp/esp-idf/components/esp_hal_security ~/esp/esp-idf/components/esp_hal_timg ~/esp/esp-idf/components/esp_hal_touch_sens ~/esp/esp-idf/components/esp_hal_twai ~/esp/esp-idf/components/esp_hal_uart ~/esp/esp-idf/components/esp_hal_usb ~/esp/esp-idf/components/esp_hal_wdt ~/esp/esp-idf/components/esp_hid ~/esp/esp-idf/components/esp_http_client ~/esp/esp-idf/components/esp_http_server ~/esp/esp-idf/components/esp_https_ota ~/esp/esp-idf/components/esp_https_server ~/esp/esp-idf/components/esp_hw_support ~/esp/esp-idf/components/esp_lcd ~/esp/esp-idf/components/esp_libc ~/esp/esp-idf/components/esp_local_ctrl ~/esp/esp-idf/components/esp_mm ~/esp/esp-idf/components/esp_netif ~/esp/esp-idf/components/esp_netif_stack ~/esp/esp-idf/components/esp_partition ~/esp/esp-idf/components/esp_phy ~/esp/esp-idf/components/esp_pm ~/esp/esp-idf/components/esp_psram ~/esp/esp-idf/components/esp_ringbuf ~/esp/esp-idf/components/esp_rom ~/esp/esp-idf/components/esp_security ~/esp/esp-idf/components/esp_stdio ~/esp/esp-idf/components/esp_system ~/esp/esp-idf/components/esp_timer ~/esp/esp-idf/components/esp_trace ~/esp/esp-idf/components/esp_usb_cdc_rom_console ~/esp/esp-idf/components/esp_wifi ~/esp/esp-idf/components/espcoredump ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__cjson ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__cmake_utilities ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/espressif__esp_lcd_sh8601 ~/esp/esp-idf/components/esptool_py ~/esp/esp-idf/components/fatfs ~/esp/esp-idf/components/freertos ~/esp/esp-idf/components/hal ~/esp/esp-idf/components/heap ~/esp/esp-idf/components/http_parser ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/i2c_bsp ~/esp/esp-idf/components/idf_test ~/esp/esp-idf/components/ieee802154 ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/lcd_bl_pwm_bsp ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/lcd_touch_bsp ~/esp/esp-idf/components/log ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/managed_components/lvgl__lvgl ~/esp/esp-idf/components/lwip ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/main ~/esp/esp-idf/components/mbedtls ~/esp/esp-idf/components/nvs_flash ~/esp/esp-idf/components/nvs_sec_provider ~/esp/esp-idf/components/openthread ~/esp/esp-idf/components/partition_table ~/esp/esp-idf/components/perfmon ~/esp/esp-idf/components/protobuf-c ~/esp/esp-idf/components/protocomm ~/esp/esp-idf/components/pthread ~/esp/esp-idf/components/rt ~/esp/esp-idf/components/sdmmc ~/esp/esp-idf/components/soc ~/esp/esp-idf/components/spi_flash ~/esp/esp-idf/components/spiffs ~/esp/esp-idf/components/tcp_transport ~/esp/esp-idf/components/ulp ~/esp/esp-idf/components/unity ~/esp/esp-idf/components/vfs ~/esp/esp-idf/components/wear_levelling ~/esp/esp-idf/components/wpa_supplicant ~/esp/esp-idf/components/xtensa
-- Configuring done (9.6s)
-- Generating done (1.5s)
-- Build files have been written to: ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build
[  0%] Built target partition_table_bin
[  0%] Built target sections_ld_in_preprocess
[  0%] Built target blank_ota_data
[  0%] Built target memory_ld_in_preprocess
[  0%] Built target _project_elf_src
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
[  9%] Built target __idf_esp_netif
[ 71%] Built target __idf_esp_hal_gpspi
[ 74%] Built target __idf_esp_hal_dma
[ 75%] Built target __idf_micro-ecc
[ 76%] Built target __idf_esp_hal_pmu
[ 79%] Built target __idf_esp_hal_usb
[ 15%] Built target __idf_lwip
[ 84%] Built target __idf_esp_hal_gpio
[ 15%] Built target __idf_vfs
[ 88%] Built target __idf_hal
[ 15%] Built target __idf_esp_driver_usb_serial_jtag
[ 93%] Built target __idf_soc
[ 15%] Built target __idf_esp_phy
[ 95%] Built target __idf_xtensa
[ 97%] Built target __idf_main
[ 16%] Built target __idf_nvs_flash
[ 98%] Built target bootloader.elf
[ 17%] Built target __idf_nvs_sec_provider
[100%] Built target gen_bootloader_binary
[ 18%] Built target __idf_esp_event
[100%] Built target gen_project_binary
[ 19%] Built target __idf_esp_driver_uart
Bootloader binary size 0x5830 bytes. 0x27d0 bytes (31%) free.
[ 19%] Built target __idf_esp_psram
[100%] Built target bootloader_check_size
[100%] Built target app
[ 19%] Built target __idf_esp_ringbuf
[ 19%] No install step for 'bootloader'
[ 20%] Built target __idf_esp_timer
[ 20%] Completed 'bootloader'
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
[ 54%] Building C object esp-idf/dial_state/CMakeFiles/__idf_dial_state.dir/dial_state.c.obj
[ 54%] Built target __idf_esp_hal_twai
[ 54%] Built target __idf_esp_hal_ledc
[ 55%] Built target __idf_espressif__cjson
[ 55%] Built target __idf_esp_hal_lcd
[ 55%] Built target __idf_dial_time
[ 55%] Built target __idf_esp_driver_spi
[ 56%] Built target __idf_esp_driver_gptimer
[ 56%] Built target __idf_esp_driver_i2c
[ 57%] Built target __idf_unity
[ 58%] Built target __idf_esp_hal_cam
[ 58%] Built target __idf_esp_hal_pcnt
[ 58%] Built target __idf_esp_hal_mcpwm
[ 58%] Built target __idf_esp_hal_rmt
[ 58%] Built target __idf_esp_driver_sdm
[ 58%] Built target __idf_esp_driver_tsens
[ 58%] Built target __idf_esp_driver_twai
[ 59%] Built target __idf_sdmmc
[ 60%] Built target __idf_console
[ 60%] Built target __idf_esp_driver_touch_sens
[ 60%] Built target __idf_esp_hid
[ 60%] Built target __idf_esp_eth
[ 60%] Built target __idf_protobuf-c
[ 61%] Built target __idf_esp_https_server
[ 61%] Built target __idf_perfmon
[ 62%] Built target __idf_wear_levelling
[ 62%] Built target __idf_esp_driver_ledc
[ 62%] Built target __idf_rt
[ 63%] Built target __idf_driver
[ 64%] Built target __idf_spiffs
[ 64%] Built target __idf_cmock
[ 64%] Built target __idf_dial_somnus
[ 64%] Building C object esp-idf/dial_ota/CMakeFiles/__idf_dial_ota.dir/dial_ota.c.obj
[ 64%] Built target __idf_dial_net
[ 65%] Built target __idf_esp_lcd
[ 65%] Built target __idf_esp_driver_sd_intf
[ 66%] Built target __idf_esp_driver_cam
[ 66%] Built target __idf_esp_driver_pcnt
[ 67%] Built target __idf_esp_driver_mcpwm
[ 68%] Built target __idf_esp_driver_rmt
[ 69%] Built target __idf_esp_driver_sdspi
[ 70%] Built target __idf_protocomm
[ 70%] Built target __idf_espressif__esp_lcd_sh8601
[ 70%] Built target __idf_esp_driver_sdmmc
[ 71%] Built target __idf_esp_local_ctrl
[ 71%] Built target __idf_fatfs
[ 71%] Linking C static library libdial_state.a
[ 71%] Built target __idf_dial_state
[ 72%] Building C object esp-idf/dial_pad_discovery/CMakeFiles/__idf_dial_pad_discovery.dir/dial_pad_discovery.c.obj
[ 72%] Linking C static library libdial_ota.a
[ 72%] Built target __idf_dial_ota
[ 72%] Linking C static library libdial_pad_discovery.a
[ 72%] Built target __idf_dial_pad_discovery
[ 97%] Built target __idf_lvgl__lvgl
[ 97%] Built target __idf_dial_display
[ 97%] Built target __idf_lcd_bl_pwm_bsp
[ 97%] Built target __idf_lcd_touch_bsp
[ 97%] Built target __idf_i2c_bsp
[ 97%] Built target __idf_dial_haptics
[ 97%] Building C object esp-idf/dial_power/CMakeFiles/__idf_dial_power.dir/dial_power.c.obj
[ 97%] Linking C static library libdial_power.a
[ 97%] Built target __idf_dial_power
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_connecting.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_passkey.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_router.c.obj
[ 97%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_about.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_discovery.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_netpick.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/ui_screens.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_setup.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_dial.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_wifi.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_menu.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_updating.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_welcome.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_update_prompt.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_settings.c.obj
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_pad_address.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_adjust_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness_menu.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_brightness.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_mode.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_night_face.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby_face.c.obj
[100%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_timezone.c.obj
[100%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
[100%] Linking C static library libmain.a
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
NOTE: ~/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: 
BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 
'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 
'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is 
treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:235: 
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
somnus-dial.bin binary size 0x1896b0 bytes. Smallest app partition is 0x400000 bytes. 0x276950 bytes (62%) free.
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
(exit 0)
```

Warnings: the only warning is the CMake deprecation notice for
`cmake_minimum_required` at the top-level `firmware/dial-idf/CMakeLists.txt:5`,
a file this change did not touch. No compiler warning appears at all, so no
new warning mentions a file this change touched.

Size:

```
$ stat -f %z build/somnus-dial.bin
1611440
```

| Binary | Bytes |
|---|---|
| 1.0.2 Release asset | 1,612,320 |
| This build (`fe5b139` tree, still PROJECT_VER 1.0.2) | 1,611,440 |
| Difference | −880 |

Smaller, as expected.

## Step 6 — identity checks

```
$ strings build/somnus-dial.bin | grep -c "repos/matthewclaude/bedknob-for-somnus"
3
$ strings build/somnus-dial.bin | grep -c -E "somnus-dial-releases|orion-waveshare-rotary-dial"
0
$ strings build/somnus-dial.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$'
1.0.2
$ strings build/somnus-dial.bin | grep -c "Which side"
0
```

All four match: 3, 0, `1.0.2` only, 0.

## Step 7 — simulator

PNG count before the run: 49 (including `sidepick.png`). Full output,
home-directory prefix written as `~`:

```
$ cmake -B build -S simulator && cmake --build build && ./build/dial_sim
-- dial_sim: firmware version 1.0.2, OTA scenarios advertise 1.0.3
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
[ 42%] Built target lvgl
[ 42%] Building C object CMakeFiles/dial_sim.dir/main.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/sim_state.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir/stubs.c.o
[ 42%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_router.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/ui_screens.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_connecting.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_setup.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_dial.c.o
[ 43%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_netpick.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_passkey.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_menu.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_wifi.c.o
[ 44%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_about.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_updating.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_update_prompt.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby.c.o
[ 45%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_welcome.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_settings.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_timezone.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_discovery.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_pad_address.c.o
[ 46%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_adjust_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness_menu.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_brightness.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_mode.c.o
[ 47%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_night_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/scr_standby_face.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_palette.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_list.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_88.c.o
[ 48%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_16.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_icons_20.c.o
[ 49%] Building C object CMakeFiles/dial_sim.dir~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/components/dial_ui/dial_font_num_140.c.o
[ 49%] Linking C executable dial_sim
[ 49%] Built target dial_sim
[ 87%] Built target lvgl_examples
[100%] Built target lvgl_demos
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
wrote update           ~/Projects/somnus-waveshare-rotary-dial/docs/screens/update.png (5 distinct colors sampled)
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
wrote pad-degraded-real ~/Projects/somnus-waveshare-rotary-dial/docs/screens/pad-degraded-real.png (4 distinct colors sampled)
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
(exit 0)
```

```
$ ls docs/screens/*.png | wc -l
      48
$ git --no-optional-locks status --short docs/screens/
 M docs/screens/about-wifi-real.png
 M docs/screens/about-wifi-worst.png
 M docs/screens/about.png
D  docs/screens/sidepick.png
 M docs/screens/update-failed.png
 M docs/screens/update-prompt.png
 M docs/screens/update.png
```

48 PNGs, one fewer than before. `sidepick.png` shows as deleted (staged by
`git rm`). **Finding:** six other PNGs came out modified:
`about.png`, `about-wifi-real.png`, `about-wifi-worst.png`,
`update-failed.png`, `update-prompt.png`, `update.png`. Cause, checked by
eye on `about.png`: the committed copies still show firmware **v1.0.0**
(last regenerated in `effb9c6`), and the simulator now reads PROJECT_VER
1.0.2. They are stale since before 1.0.1 and have nothing to do with this
change. They were **not committed**; they were restored to HEAD with
`git checkout --` so the tree is clean. They will change again at the
1.0.3-beta.1 version bump, which is the natural place to regenerate them.

## Step 8 — commit

Each path was staged on its own with `git add -- <path>`, except the two
deletions (`scr_sidepick.c`, `sidepick.png`), which `git rm` staged when
they were deleted. `build/` is git-ignored and was not staged. After the
commit, `git status --short` printed nothing.

## Deviations from the spec or this task

- The pasted build and simulator output has the home-directory prefix
  written as `~` (the repo is public). Otherwise unfiltered.
- The main.c `git show` is shown with `--format=` to leave out the author
  e-mail header.
- The two file deletions were staged by `git rm` at the moment of deletion
  rather than one at a time at step 8.
- `docs/ARCHITECTURE.md`: after rewording the gate sentence, the paragraph's
  lines were rewrapped (same words) to keep the file's line width.

No deviation from the spec's content: every item in §4, §5 and §6 matched
the code on disk at the line numbers given.

## Other things noticed, not fixed

- Six stale-version simulator PNGs (see step 7).
- `dial_state.c` restore_prefs: `if (have_zone) { s_state.ui_zone     = ...; }`
  keeps the alignment spaces from when a second assignment sat under it.
  Cosmetic; left as is (spec said keep that line).
- Spec §7's Close row still says "on every bench dial", while the new order
  paragraph says there is exactly one dial. Harmless; not reworded because
  the task named only the T3 row.

## Not verified without hardware

Everything in spec §7 is unverified: T1 and T1-r (fresh One Bed, no picker,
Scale Relative across two RST reboots), T4 and T4-r (the latent 1.0.2 case:
non-fresh dial switched to Dual Sides, swipe, Scale stays Relative across
reboots, no side1 POST), T2, T2-a, T2-r and T2-z (fresh Dual Sides, face
opens on RIGHT SIDE, no picker, Scale stays Relative after a swipe and
reboots, no side1 POST), T3 (OTA upgrade from 1.0.2 keeps side and Scale),
and Close. The relmode seed's behaviour on NVS is proven only by reading
the code, not on a device.
