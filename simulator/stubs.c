/*
 * stubs.c — every non-dial_state external dependency the compiled dial_ui
 * screens call, per the scout report's per-file symbol list. All of it is
 * either a hardware driver (haptics, power, display rotation) with nothing
 * to actually do on a host, or Wi-Fi/OTA/app-descriptor state the simulator
 * fakes with fixed, plausible values instead of running the real drivers.
 *
 * Canned network scan list and IP per the build spec:
 *   "Home" (-48), "Home-Guest" (-55), "Bluebird Cottage" (-67), "Attic AP" (-79)
 *   dial_net_ip -> "192.168.1.23"
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "dial_haptics.h"
#include "dial_power.h"
#include "dial_display.h"
#include "dial_wifi.h"
#include "dial_time.h"
#include "esp_wifi.h"
#include "esp_app_desc.h"
#include "sim_state.h"

/* ---- dial_time ---------------------------------------------------------- */
// Settings' Timezone row and the picker read these; the simulator has no
// SNTP and no persisted zone, so it renders the honest "Not set" state --
// except when a scenario has installed a fake zone via sim_set_fake_iana_tz()
// (sim_state.h), the one hook that lets settings-timezone-raw.png show the
// raw-IANA fallback. Default (no hook) is unchanged: false.
static char s_fake_iana[48];
static bool s_fake_iana_set;

void sim_set_fake_iana_tz(const char *iana)
{
    if (!iana) { s_fake_iana_set = false; return; }
    strncpy(s_fake_iana, iana, sizeof(s_fake_iana) - 1);
    s_fake_iana[sizeof(s_fake_iana) - 1] = '\0';
    s_fake_iana_set = true;
}

bool dial_time_get_iana_tz(char *out, size_t sz)
{
    if (!s_fake_iana_set || !out || sz == 0) return false;
    strncpy(out, s_fake_iana, sz - 1);
    out[sz - 1] = '\0';
    return true;
}
bool dial_time_get_posix_tz(char *out, size_t sz) { (void)out; (void)sz; return false; }
bool dial_time_set_iana_tz(const char *iana) { (void)iana; return true; }
// No zone ever persisted here (see the two getters above), so this is
// honestly false — the same "set timezone" reason a real fresh device with
// no zone shows (dial_night_clock_reason(), ui_screens_internal.h).
bool dial_time_valid(void) { return false; }

/* ---- dial_haptics ------------------------------------------------------ */

void dial_haptics_play(haptic_effect_t fx) { (void)fx; }
void dial_haptics_set_level(haptic_level_t level) { (void)level; }
void dial_haptics_set_night(bool night) { (void)night; }
void dial_haptics_mute(bool muted) { (void)muted; }
void dial_haptics_play_soft(haptic_effect_t fx) { (void)fx; }
void dial_haptics_init(void) {}

/* ---- dial_power ---------------------------------------------------------
 * Always ACTIVE: the simulator only ever renders one screen at a time, on
 * demand — there is no idle clock counting down toward DIMMED/STANDBY. */

dial_power_level_t dial_power_level(void) { return DPWR_ACTIVE; }
bool dial_power_wake_consumes(void) { return false; }
void dial_power_start(void) {}
void dial_power_set_night(bool night) { (void)night; }
void dial_power_inhibit(dial_power_inhibit_src_t src, bool on) { (void)src; (void)on; }
void dial_power_brightness_changed(void) {}
void dial_power_preview(bool night, dial_power_level_t level, uint8_t pct) { (void)night; (void)level; (void)pct; }
void dial_power_preview_end(void) {}

/* ---- dial_display --------------------------------------------------------
 * Rotation always "succeeds" — the real function only fails when the 90/270
 * DMA scratch buffer can't be allocated, a panel-driver detail the host build
 * has no equivalent of. */

bool dial_display_set_rotation(uint8_t quarters) { (void)quarters; return true; }
uint8_t dial_display_rotation(void) { return 0; }
void dial_display_start(void) {}
bool dial_display_lock(int timeout_ms) { (void)timeout_ms; return true; }
void dial_display_unlock(void) {}
void dial_display_set_touch_filter(dial_display_touch_filter_t filter) { (void)filter; }

/* ---- dial_net / Wi-Fi ----------------------------------------------------
 * A fixed 4-network scan result and a fixed "connected to Home" status —
 * enough for scr_netpick/scr_passkey/scr_wifi to render real content without
 * a Wi-Fi driver underneath. */

typedef struct { const char *ssid; int8_t rssi; } fake_ap_t;
static const fake_ap_t FAKE_SCAN[] = {
    { "Home",             -48 },
    { "Home-Guest",       -55 },
    { "Bluebird Cottage", -67 },
    { "Attic AP",         -79 },
};
#define FAKE_SCAN_COUNT (int)(sizeof(FAKE_SCAN) / sizeof(FAKE_SCAN[0]))

int dial_net_scan_count(void) { return FAKE_SCAN_COUNT; }

const char *dial_net_scan_ssid(int i)
{
    if (i < 0 || i >= FAKE_SCAN_COUNT) return "";
    return FAKE_SCAN[i].ssid;
}

void dial_net_scan_request(void) { /* result is static; nothing to kick off */ }
void dial_net_submit_creds(const char *ssid, const char *pass) { (void)ssid; (void)pass; }

bool dial_wifi_is_connected(void) { return true; }

bool dial_net_ip(char *out, size_t sz)
{
    if (!out || sz == 0) return false;
    snprintf(out, sz, "192.168.1.23");
    return true;
}

// Test-only override (sim_state.h) for a scenario that needs an SSID/RSSI
// combination beyond FAKE_SCAN[0] -- e.g. About's Wi-Fi row layout check
// against a worst-case 32-char SSID. NULL ssid = back to the fixed default.
static char    s_fake_ap_ssid[33];
static int8_t  s_fake_ap_rssi;
static bool    s_fake_ap_set;

void sim_set_fake_ap(const char *ssid, int8_t rssi)
{
    if (!ssid) { s_fake_ap_set = false; return; }
    strncpy(s_fake_ap_ssid, ssid, sizeof(s_fake_ap_ssid) - 1);
    s_fake_ap_ssid[sizeof(s_fake_ap_ssid) - 1] = '\0';
    s_fake_ap_rssi = rssi;
    s_fake_ap_set = true;
}

esp_err_t esp_wifi_sta_get_ap_info(wifi_ap_record_t *ap_info)
{
    if (!ap_info) return ESP_FAIL;
    memset(ap_info, 0, sizeof(*ap_info));
    if (s_fake_ap_set) {
        strncpy((char *)ap_info->ssid, s_fake_ap_ssid, sizeof(ap_info->ssid) - 1);
        ap_info->rssi = s_fake_ap_rssi;
    } else {
        strncpy((char *)ap_info->ssid, FAKE_SCAN[0].ssid, sizeof(ap_info->ssid) - 1);
        ap_info->rssi = FAKE_SCAN[0].rssi;
    }
    return ESP_OK;
}

/* Unused by dial_ui (no screen calls dial_net_init/bringup/etc.), but part of
 * dial_wifi.h's surface; provided so any incidental reference still links. */
void dial_net_init(void) {}
void dial_net_seed(const char *ssid, const char *pass) { (void)ssid; (void)pass; }
bool dial_net_have_creds(void) { return true; }
void dial_net_forget(void) {}
void dial_net_request_setup(void) {}
bool dial_net_setup_requested(void) { return false; }
void dial_net_bringup(void) {}
const char *dial_net_ap_ssid(void) { return "Bedknob-A1B2"; }
void dial_net_on_event(dial_net_event_cb_t cb) { (void)cb; }

/* ---- deterministic clock -------------------------------------------------
 * scr_standby.c (the real, unmodified firmware source — CMakeLists.txt's
 * "HARD RULE: nothing under firmware/ is modified" rules out fixing this at
 * the call site) reads the clock straight from libc: time(NULL) then
 * localtime_r(), no dial_time abstraction involved. Left alone, that means
 * standby.png/standby-update.png render whatever moment the simulator
 * happens to run at, so they show as "changed" after every single run with
 * no actual layout change behind it (2026-08-30: confirmed benign, but not
 * useful as a diff signal).
 *
 * Fixed here instead, by definition rather than a linker trick: a strong
 * time()/localtime_r() symbol in one of the simulator's own object files
 * wins the link over libc's dynamic one for the whole dial_sim executable
 * (verified — libc's never gets called), so scr_standby.c gets a fixed
 * moment without knowing anything changed. localtime_r() ignores its
 * `timep` argument entirely and always returns the same struct: fixing only
 * time()'s return value would still leave the rendered hour/weekday at the
 * mercy of whatever TZ the host machine happens to have set, which defeats
 * the point.
 *
 * The moment itself is picked to be unmistakably a placeholder, not
 * anything that could be read as a real recorded timestamp: 2000-01-01
 * 12:34:00 UTC — Y2K, ascending clock digits, and (not chosen for this,
 * just how the calendar landed) a Saturday. Renders as clock "12:34", date
 * "SAT - JAN 1". The epoch value below is that exact moment (946730040),
 * kept consistent with the struct tm rather than an unrelated number, even
 * though localtime_r() below never actually converts it.
 */
#define SIM_FIXED_EPOCH 946730040   /* 2000-01-01T12:34:00Z */

time_t time(time_t *tloc)
{
    if (tloc) *tloc = SIM_FIXED_EPOCH;
    return SIM_FIXED_EPOCH;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    (void)timep;
    memset(result, 0, sizeof(*result));
    result->tm_year = 100;   // 2000 (years since 1900)
    result->tm_mon  = 0;     // January (0-indexed)
    result->tm_mday = 1;
    result->tm_hour = 12;
    result->tm_min  = 34;
    result->tm_wday = 6;     // Saturday (0=Sunday, matches scr_standby.c's WD[])
    return result;
}

/* ---- esp_app_desc --------------------------------------------------------
 * Fixed "v1.0.1 / v6.0" identity for scr_about.c's Firmware/IDF rows —
 * kept in step with firmware/dial-idf/CMakeLists.txt's PROJECT_VER so the
 * simulator's about.png never shows a version the real firmware doesn't. */

const esp_app_desc_t *esp_app_get_description(void)
{
    static const esp_app_desc_t desc = { .version = "0.1.4", .idf_ver = "v6.0" };
    return &desc;
}
