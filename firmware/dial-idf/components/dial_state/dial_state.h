#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

/*
 * The single app-state store. The network worker (and only it) commits device
 * truth; input handlers commit optimistic UI intent; the LVGL-side dispatcher
 * polls the generation counter and re-renders the active screen on change.
 *
 * Rules:
 *  - Readers take a full snapshot with dial_state_get() — never hold pointers
 *    into the store.
 *  - Writers mutate inside dial_state_commit()'s callback — the store mutex is
 *    held for the duration, so keep mutators tiny and never block in them.
 *  - The quiet-period input gate (dial_state_stamp_input / last_input_us) is
 *    the proven resync mechanism: the worker only reads the bed back after
 *    2.5s of no input, so a poll can never land mid-interaction.
 */

typedef enum {
    PH_BOOT = 0,
    PH_WIFI_CONNECTING,
    PH_WIFI_PORTAL,          // SoftAP captive portal is up, waiting for creds
    PH_WIFI_LOST,            // had Wi-Fi, lost it; supervisor is retrying
    // Somnus's pad API is a plain local JSON REST endpoint (dial_somnus.h) --
    // no discovery, no DCR, no interactive consent, so the three Orion
    // OAuth/MCP phases this replaces (PH_OAUTH_DISCOVER/PH_OAUTH_WAIT_CONSENT/
    // PH_MCP_CONNECTING) collapse into this one: worker_task is just probing
    // dial_somnus_connect(base_url) with a real GET.
    PH_SOMNUS_CONNECTING,
    PH_READY,                // steady state: command + poll loop
    PH_DEGRADED,             // net up but the pad's calls failing; retrying w/ backoff
    // Subnet scan running (docs/SPEC-pad-discovery.md), after the persisted
    // address has failed at least once. worker_task blocks in
    // dial_pad_discovery_scan() for the duration -- up to ~57.6s worst case
    // (two passes, 300ms + 600ms probe timeouts) -- so this is a real,
    // long-running phase, not a blip; it needs its own case in nav_policy
    // (main.c), not the bare default (see docs/SPEC-connect-phases.md's
    // trace of that exact bug for PH_SOMNUS_CONNECTING).
    PH_PAD_DISCOVERY,
} conn_phase_t;

typedef enum { ZONE_A = 0, ZONE_B = 1, ZONE_COUNT = 2 } zone_idx_t;

// Plugged-in / on-battery detector (docs/SPEC-power-sensing.md §10). UNKNOWN
// until dial_power.c's power_task has accumulated 5 one-second samples of
// GPIO1/BATT_ADC after boot -- see app_state_t.power_src below.
typedef enum { PWR_UNKNOWN = 0, PWR_BATTERY, PWR_PLUGGED } dial_power_src_t;

/*
 * Canonical temperature unit (2026-08-30 units fix, see the Q1 audit this
 * fixes): tenths of a degree Celsius, integer, "dc" for short. The pad's own
 * wire unit IS Celsius (target_t/current_t, local_api spec) and the
 * confirmed real adjustment step is exactly 1.0°C — so the setpoint is
 * stored, stepped, and written in dc EVERYWHERE (zone_state_t.temp_dc,
 * app_state_t.ui_temp_dc, app_cmd_t.temp_dc, the DIAL_REL_* level tables).
 * °F exists ONLY as a render-time label — dial_dc_to_f() below, called
 * exclusively from scr_dial.c's render_numeral() — and is never stored, never
 * stepped, and never sent anywhere. This is the reverse of the pre-fix
 * design (whole °F was canonical, Celsius was derived at POST time), which
 * silently produced fractional, non-1.0°C POST bodies on every knob turn: a
 * real-pad test (2026-08-30) confirmed the pad accepts and holds fractional
 * target_t with no snapping, so there was no floor catching that drift.
 *
 * dial_c_to_f(float c) below is kept, unchanged, for the ONE thing it was
 * always legitimately used for: display of the pad's raw measured-water
 * reading (zone_state_t.actual_c), which has no canonical-storage role and
 * is never round-tripped back into a write. It must never again be used on
 * the setpoint.
 */
static inline int dial_c_to_f(float c) { return (int)lroundf(c * 1.8f + 32.0f); }

// Setpoint display-only conversion: canonical dc (tenths of °C) -> nearest
// whole °F, for render_numeral()'s °F mode. Called ONLY at render time —
// never on a read or write path, never stored back — so there is no
// C->F->C round-trip anywhere in the setpoint's life. (There used to be a
// dial_f_to_c(int f) alongside dial_c_to_f: it was the write-path half of
// that round-trip and is deleted, not renamed — nothing should ever again
// convert a whole-°F value back into the canonical unit.)
static inline int dial_dc_to_f(int dc) { return (int)lroundf((float)dc * 0.18f + 32.0f); }

// ABSOLUTE display range, tenths of °C. Superseded at runtime by the pad's
// own fixed range (dial_somnus.h: 12.0-42.3°C, seeded once into
// app_state_t.temp_min_dc/temp_max_dc right after the first successful
// connect — see main.c's worker_task) — screens read it through
// dial_state_temp_min_dc()/dial_state_temp_max_dc() below, never these
// macros directly, except as the fallback those two functions use before
// that first connect has ever completed (fresh boot, offline). 100/450 is
// the exact tenths-of-°C equivalent of the pre-fix 50/113°F fallback pair
// (10.0°C/45.0°C) — unit-converted, not re-picked, so this fallback covers
// the same real range it always did.
#define DIAL_TEMP_MIN_DC 100
#define DIAL_TEMP_MAX_DC 450

/*
 * RELATIVE temperature scale ("LEVEL" mode). A signed −15…+15 level scale,
 * uniform, Celsius-native: 1.0°C per level, level 0 = 27.0°C. Purely a
 * display/input convention on our side — the wire is always °C (set_zone
 * takes Celsius) — but unlike this file's pre-2026-09-01 version, it now IS
 * the same 1.0°C-per-step as absolute mode; a relative-mode detent moves one
 * level == exactly 1.0°C, nothing fancier.
 *
 * This replaces a −10…+10 lookup table that was mechanically transcribed
 * from Orion's (the upstream product's) whole-Fahrenheit relative scale.
 * That table was wrong for this product: it was never re-derived against
 * the Somnus app, so it silently disagreed with what the Somnus app itself
 * displays for the same bed temperature, at every level, the entire time
 * this port has existed. Three measurements against the real pad on
 * 2026-09-01 pin the actual Somnus scale — app level −15 = 12.0°C, −6 =
 * 21.0°C, +15 = 42.0°C — which is exactly uniform at 1.0°C/level, so the
 * fix is arithmetic, not a corrected table. Full derivation and the
 * measurements: docs/SPEC-somnus-relative-scale.md.
 *
 * The old table's rails (10.0°C/45.0°C) also exceeded the Somnus API's
 * accepted range (12.0-42.3°C per local_api spec) — the pad silently
 * clamped, so the dial's extreme levels collapsed onto the same real
 * temperature. The new rails sit inside the API's accepted range instead.
 *
 * See test/test_dial_rel.c for the invariants this must keep holding.
 */
#define DIAL_REL_MIN     (-15)
#define DIAL_REL_MAX     ( 15)
#define DIAL_REL_MIN_DC  120   // level −15 rail (12.0°C, the API minimum)
#define DIAL_REL_MAX_DC  420   // level +15 rail (42.0°C)
#define DIAL_REL_ZERO_DC 270   // level 0 (27.0°C)

// The tenths-of-°C value carrying a level (level clamped to range first).
static inline int dial_rel_to_dc(int level)
{
    if (level < DIAL_REL_MIN) level = DIAL_REL_MIN;
    if (level > DIAL_REL_MAX) level = DIAL_REL_MAX;
    return DIAL_REL_ZERO_DC + 10 * level;
}

// Nearest relative level for a tenths-of-°C value (clamped to −15…+15).
// Rounds to nearest, never truncates -- truncation would bias every
// off-grid value toward zero. Ties (exactly halfway between two levels)
// resolve toward the WARMER level, i.e. round-half-up on (dc -
// DIAL_REL_ZERO_DC)/10 regardless of sign: integer division truncates
// toward zero in C, so plain "/10" would round -0.5 the wrong way (toward
// zero, i.e. warmer, only on one side) -- the explicit floor below makes
// the half-up rule symmetric across zero instead.
static inline int dial_rel_from_dc(int dc)
{
    int diff = dc - DIAL_REL_ZERO_DC + 5;   // +5: round-half-up, not truncate
    int lvl  = diff / 10;
    if (diff % 10 < 0) lvl--;               // true floor for negative diff
    if (lvl < DIAL_REL_MIN) lvl = DIAL_REL_MIN;
    if (lvl > DIAL_REL_MAX) lvl = DIAL_REL_MAX;
    return lvl;
}

// One detent = exactly one level in the turned direction, from whatever level
// is currently DISPLAYED (so an off-grid device value snaps onto the grid in
// the direction the user turned — the numeral and the bed always move together
// or not at all). Returns the new carrier's tenths-of-°C value; equals dc only
// when pinned at a rail (caller treats that as the range stop).
static inline int dial_rel_step(int dc, int detents)
{
    int cur = dial_rel_from_dc(dc);
    int nl  = cur + detents;
    if (nl < DIAL_REL_MIN) nl = DIAL_REL_MIN;
    if (nl > DIAL_REL_MAX) nl = DIAL_REL_MAX;
    return (nl == cur) ? dc : dial_rel_to_dc(nl);
}

// Parse an "HH:MM" (24h) time string into minutes-from-midnight. Returns
// false (leaving *out_min untouched) on any malformed input.
static inline bool dial_parse_hhmm(const char *s, int *out_min)
{
    int hh, mm;
    if (!s || sscanf(s, "%d:%d", &hh, &mm) != 2) return false;
    if (hh < 0 || hh > 23 || mm < 0 || mm > 59) return false;
    *out_min = hh * 60 + mm;
    return true;
}

/*
 * Screen (lock/standby) timeout: the fixed set of idle durations Settings'
 * "Screen timeout" row offers, seconds. Deliberately NOT "Never" — this is a
 * bedside device whose whole standby design (night dimming, the standby
 * clock face, dial_power_wake_consumes' wake-eats-first-input rule) exists
 * to stop it glowing at 3am; an always-on face would be a footgun the rest
 * of the firmware is built to avoid.
 *
 * app_state_t.screen_timeout_s (below) holds this preference; its own
 * default (90) is deliberately NOT one of these five — see that field's
 * comment for why. dial_scr_timeout_label treats any value that isn't an
 * exact member (today only that 90s default) as "nearest to one of these
 * five, ties toward the LONGER one", so the row always shows one of its own
 * five labels (a fresh device reads "2m", the nearest to the true 90s).
 * dial_scr_timeout_next always advances one step PAST that nearest choice —
 * never just snaps onto it — so even a device's very first tap visibly
 * changes the label instead of silently reproducing the same nearest choice
 * the row was already (mis)reporting; only every tap thereafter is a plain
 * one-step cycle, same idiom as Rotation's modulo.
 */
#define DIAL_SCR_TIMEOUT_CHOICES_N 4
// Seconds only, no "Off": an always-on option was tried and removed as not
// worth its complexity (it needed an infinite-threshold special case in
// dial_power, and it defeats the night dimming and clock face this device is
// built around). Anything above 1m was cut too — on a dial you glance at and
// walk away from, the useful range is seconds. A device that somehow stored
// the old 0 sentinel falls back to the 90s default via clamp_screen_timeout_s.
static const uint16_t DIAL_SCR_TIMEOUT_CHOICES[DIAL_SCR_TIMEOUT_CHOICES_N] = { 5, 15, 30, 60 };
static const char *const DIAL_SCR_TIMEOUT_LABELS[DIAL_SCR_TIMEOUT_CHOICES_N] = { "5s", "15s", "30s", "1m" };

static inline int dial_scr_timeout_nearest_idx(uint16_t s)
{
    int idx = 0, best_d = -1;
    for (int i = 0; i < DIAL_SCR_TIMEOUT_CHOICES_N; i++) {
        int d = (int)s - (int)DIAL_SCR_TIMEOUT_CHOICES[i];
        if (d < 0) d = -d;
        if (best_d < 0 || d < best_d ||
            (d == best_d && DIAL_SCR_TIMEOUT_CHOICES[i] > DIAL_SCR_TIMEOUT_CHOICES[idx])) {
            best_d = d;
            idx = i;
        }
    }
    return idx;
}

// Display label for the Screen timeout row — always one of the five choices'
// own strings (see the block comment above for how an off-menu value like
// the 90s default maps onto one).
static inline const char *dial_scr_timeout_label(uint16_t s)
{
    return DIAL_SCR_TIMEOUT_LABELS[dial_scr_timeout_nearest_idx(s)];
}

// Tap-to-cycle step for the Screen timeout row — same idiom as Rotation's
// plain modulo, just over an irregular set, and ALWAYS moves one step past
// the nearest choice (see the block comment above) so a tap can never be a
// silent no-op, even from the off-menu 90s default.
static inline uint16_t dial_scr_timeout_next(uint16_t cur)
{
    int idx = (dial_scr_timeout_nearest_idx(cur) + 1) % DIAL_SCR_TIMEOUT_CHOICES_N;
    return DIAL_SCR_TIMEOUT_CHOICES[idx];
}

/*
 * User-settable night window (docs/SPEC-night-window.md). True when
 * now_min (minutes from local midnight) falls inside [start, end). Windows
 * wrap midnight -- 21:00-07:00 is start > end, the normal case -- so both
 * orders are handled here rather than at each call site. A pure function of
 * its arguments: the same "what should it be at 3:14am" discipline
 * SPEC-dial-side-scheduling.md §5 argues for, so a reboot mid-night resolves
 * correctly with no sequence state to restart. Also reused by §6's
 * auto-update window (dial_auto_update_window, below app_state_t) -- a
 * second hand-written wrap comparison is how the two would drift apart.
 * dial_night_active/dial_auto_update_window live further down this file,
 * next to dial_state_is_dual — both take a `const app_state_t *`, which
 * isn't defined yet at this point in the file.
 */
static inline bool dial_in_window(uint16_t start, uint16_t end, int now_min)
{
    if (start == end) return false;   // unreachable after the clamp-on-read below; belt-and-braces
    return (start < end) ? (now_min >= start && now_min <  end)
                         : (now_min >= start || now_min <  end);
}

/*
 * Curated timezone list for Settings' "Timezone" row (docs/SPEC-timezone-
 * source.md's "A curated list, not the full table" section) — NOT the
 * ~400-zone posix_tz_db table dial_time.c embeds; dial_list.h walks one row
 * per detent, and ~400 rows is not a knob-scrollable list. A short,
 * US/UK/EU/AU-weighted set covers essentially every real user. This is the
 * single source both scr_timezone.c (the rows it renders) and main.c's
 * handle_immediate_cmd (CMD_TZ_CHANGED below resolves the tapped index back
 * through this same table) read — never duplicated between the two.
 *
 * Every IANA string here was checked against dial_time.c's embedded
 * zones.csv directly, not assumed to resolve — a caller passing a name the
 * table doesn't have is dial_time_set_iana_tz()'s own safe no-op (dial_time.h),
 * but a curated entry that silently never applied would be a much quieter,
 * much worse version of the exact bug this whole feature exists to fix.
 * "UTC" alone is NOT a key in that table (only "Etc/UTC" is) — caught by
 * that check, not shipped as originally proposed. "Europe/Berlin" was
 * dropped for a different reason after the fact (2026-09-01): it shares
 * both an offset and DST rules with "Europe/Paris", so the two originally
 * had the same "Central Europe" label -- indistinguishable rows in a
 * knob-scrolled list, and the Settings row couldn't say which one was
 * actually stored either. Safe to remove entries here: the selection is
 * persisted by IANA string (dial_time.c's "iana_tz" NVS key), never by
 * index into this table -- CMD_TZ_CHANGED's index (below) only lives from
 * the moment a row is tapped to the moment handle_immediate_cmd resolves
 * it to a string, in the same boot, and is never itself written to NVS.
 *
 * This is a convenience layer, not a restriction: the full table stays
 * embedded and the Wi-Fi portal (dial_wifi.c's root_get()/save_post()) still
 * accepts whatever IANA zone a browser reports, curated or not.
 */
#define DIAL_TZ_COUNT 11
static const char *const DIAL_TZ_IANA[DIAL_TZ_COUNT] = {
    "America/New_York", "America/Chicago", "America/Denver", "America/Phoenix",
    "America/Los_Angeles", "America/Anchorage", "Pacific/Honolulu",
    "Europe/London", "Europe/Paris", "Australia/Sydney", "Etc/UTC",
};
static const char *const DIAL_TZ_LABEL[DIAL_TZ_COUNT] = {
    "Eastern", "Central", "Mountain", "Arizona",
    "Pacific", "Alaska", "Hawaii",
    "UK", "Central Europe", "Sydney", "UTC",
};

/*
 * Trimmed for the Somnus pad's local API (components/dial_somnus/dial_somnus.h)
 * in place of the Orion zone_state_t above/before it — the pad's /api/state
 * exposes exactly on/off, one setpoint, one measured reading, and a low-water
 * flag per side (somnus_side_state_t), with no name, no thermal-relief
 * concept, and no server-side sleep schedule to mirror. Field names/shapes
 * below otherwise still match dial_somnus.h's own somnus_side_state_t —
 * EXCEPT the setpoint, which is the canonical tenths-of-°C int here
 * (temp_dc) against the wire's own float (target_c) there; main.c's worker
 * quantizes explicitly at the boundary instead of copying it straight
 * across, see temp_dc's own comment below.
 */
typedef struct {
    bool  on;
    // Setpoint, tenths of °C, the canonical unit (see the block comment above
    // zone_idx_t). NOT a straight 1:1 mirror of somnus_side_state_t.target_c
    // (which stays float, the wire's own type) — main.c's somnus_refresh_state()
    // rounds the pad's reported float to the nearest 0.1°C on the way in, the
    // one deliberate, one-directional C(float)->C(int tenths) quantization
    // this design makes; nothing downstream of this field ever converts
    // through °F. Spec range 12.0-42.3°C = 120-423 here.
    int   temp_dc;
    float actual_c;     // measured water temp (current_c); <0 = unknown (mirrors
                        // somnus_side_state_t.has_current — the pad reports no
                        // reading at all before its first sensor sample)
    bool  water_low;    // somnus_side_state_t.water_low (spec field is_wl_low)
} zone_state_t;

/*
 * Honest, actionable phase_err text for a TLS certificate-verification
 * failure (the device's embedded trust anchors are too old for whatever the
 * server presents now) — this hardware can outlive its maintainer, so a
 * stale anchor must not read as the same routine connectivity outage as a
 * Wi-Fi blip. Currently dormant: dial_oauth/dial_mcp (the only clients that
 * ever set this verbatim as the phase_err, via their last_err_cert()
 * getters) are gone along with the rest of the Orion pipeline, and
 * dial_somnus talks plain local HTTP with no TLS to fail. Kept, not pulled,
 * because scr_connecting.c's cert-vs-generic split (it recognizes
 * DIAL_CERT_ERR_TITLE as the first line of phase_err and promotes it to the
 * screen's headline) is generic connection-error handling that any future
 * TLS-verifying client (e.g. a cloud fallback) could reuse by setting this
 * same string, unchanged.
 *
 * The repo line omits the "github.com/" host: at the error screen's sub
 * label (300px @ lv_font_montserrat_16) the full URL measures ~400px and
 * wraps awkwardly, while the bare "owner/repo" form fits on one line.
 */
#define DIAL_CERT_ERR_TITLE "Secure connection failed"
#define DIAL_CERT_ERR_MSG \
    DIAL_CERT_ERR_TITLE "\n" \
    "This firmware may be too old\n" \
    "matthewclaude/somnus-waveshare-rotary-dial"

// Fallback pad address / zone-mode, used only until Settings has ever
// persisted a real value (fresh device, or one that predates this
// preference). Mirrors dial_somnus.h's SOMNUS_DEFAULT_BASE_URL/
// SOMNUS_DEFAULT_SINGLE_ZONE_MODE without depending on that header --
// dial_state is a leaf component (no REQUIRES beyond esp_timer/nvs_flash,
// same reasoning as app_state_t.ota below not #include-ing dial_ota.h), so
// this is a deliberate duplicate, not a typo. Keep both exactly in sync with
// dial_somnus.h's own copies if either ever changes.
#define DIAL_PAD_URL_MAX_LEN 127
#define DIAL_PAD_DEFAULT_BASE_URL "http://192.168.1.100:8080"
#define DIAL_PAD_DEFAULT_SINGLE_ZONE true

typedef struct {
    // Connection / lifecycle
    conn_phase_t phase;
    char    phase_err[128];   // last human-readable error (offline/error screens)
    int     retry_in_s;       // seconds until the supervisor's next retry (0 = n/a)
    // No oauth_url field here anymore: the Somnus pad's local API
    // (components/dial_somnus) needs no interactive consent/QR step, so
    // there is no authorize URL to ever hold.
    char    ap_ssid[33];      // SoftAP name while PH_WIFI_PORTAL
    // The dial's own HOME network SSID (dial_net_sta_ssid() mirror, committed
    // once worker_task's dial_net_bringup() returns). NOT ap_ssid above.
    // "" if unknown.
    char    sta_ssid[33];

    /*
     * On-device Wi-Fi setup. A rejected password must not throw the user back
     * to the start of setup with no explanation — it has to say what went wrong
     * and put them back on the password screen for the SAME network. So the
     * store remembers which network the dial is trying, and whether the last
     * attempt came back rejected.
     */
    char    wifi_join_ssid[33];
    int8_t  wifi_join_idx;    // index into the scan list; -1 = came from the captive portal
    bool    wifi_join_failed;

    // Device truth (valid once have_state)
    bool    have_state;
    char    serial[16];       // unused by dial_somnus (the pad has no serial
                              // number, only its base_url) -- left in place
                              // rather than pulled, since About-style screens
                              // may still want a "what am I talking to" line.
    bool    device_online;
    zone_state_t zones[ZONE_COUNT];
    // Which zones the device actually reports. Mirrors dial_somnus's
    // single-zone/dual-zone ("One Bed"/"Dual Sides") setting: everything
    // that assumes a partner side (the side-swap chain, the page dots, the
    // side picker) must gate on this rather than on ZONE_COUNT, same
    // reasoning as Orion's single-zone toppers before it. Valid once
    // have_state; see dial_state_is_dual().
    bool    zone_present[ZONE_COUNT];
    // somnus_state_t.system_error ("True when a fatal error is active on the
    // device") -- top-level, not per-side, replacing Orion's safety{error,desc}
    // struct and water_fill string (per-side low-water is now zone_state_t.
    // water_low instead; the pad's API carries no free-text description to
    // put in a desc field).
    bool    system_error;
    // Away mode. Kept as a dormant placeholder rather than pulled outright:
    // Orion's set_away had no Somnus equivalent, so nothing anywhere ever
    // sets this true (there's no CMD_AWAY, no setter, no worker mutator) --
    // it's permanently false until the pad grows an away concept of its own,
    // at which point this field and scr_dial.c's badge just start working
    // again with no further plumbing. Not session-optimistic like Orion's
    // version was; there's no write path to be optimistic about.
    bool    away;
    // Absolute temperature range, tenths of °C, mirrored here once
    // worker_task connects successfully. Somnus's pad has no discovery
    // call to report its own rails (unlike Orion's list_devices), but the
    // local_api spec fixes them at 12.0-42.3°C regardless of pad -- see
    // dial_somnus.h. This, not the DIAL_TEMP_MIN_DC/MAX_DC constants, is what
    // the arc range / knob clamp / drag clamp use in ABSOLUTE mode.
    // -1 = not yet known (fresh boot, before the first successful connect)
    // -- dial_state_temp_min_dc()/_max_dc() below fall back to the
    // DIAL_TEMP_MIN_DC/MAX_DC constants then.
    int     temp_min_dc;
    int     temp_max_dc;

    // Wall clock
    bool    clock_valid;

    // Plugged-in / on-battery detector (docs/SPEC-power-sensing.md §10).
    // Worker-owned fact published into the snapshot, same model as
    // clock_valid just above: dial_power.c's power_task is the only writer.
    // power_mv updates every 1s sample directly under the store mutex,
    // WITHOUT a commit (dial_state_set_power_mv, no generation bump) --
    // a commit per second would wake every screen for nothing (§10.3).
    // power_src only changes (and only THEN triggers a real
    // dial_state_commit, bumping generation) on a debounced transition --
    // see dial_power.c's detector. UNKNOWN renders as nothing (§10.2).
    dial_power_src_t power_src;
    // Last calibrated reading, already x2 for the 10K/10K divider --
    // diagnostic only (About's Power row); not itself state that decides
    // anything on the face.
    uint16_t         power_mv;
    // Battery percentage (docs/SPEC-power-sensing.md §11.2), from
    // dial_power.c's BATT_CURVE applied to a short median of recent
    // samples (not the single power_mv reading above) and held non-
    // increasing until the next plug-in, so it neither flickers nor ticks
    // upward while discharging. 0..100 while power_src == PWR_BATTERY;
    // -1 (unknown) while PLUGGED or UNKNOWN --
    // there is no cell reading to give while the rail is the charger (§9.5's
    // "two regimes, one pin"), and inventing one is worse than admitting
    // there isn't one. Same diagnostic-only, no-commit pattern as power_mv
    // just above (dial_state_set_power_pct, called right alongside
    // dial_state_set_power_mv from dial_power.c's pwr_sample_and_classify);
    // also drives the dial-face/standby badge's fill width and low-battery
    // breathe (ui_screens_internal.h's power_glyph_apply).
    int8_t           power_pct;

    // UI intent (optimistic layer, kept apart from device truth). Canonical
    // unit tenths of °C (see the block comment above zone_idx_t) -- NOT °F,
    // even though the field predates that fix and used to be.
    int     ui_temp_dc[ZONE_COUNT]; // shown setpoint, tenths of °C; -1 = follow device
    zone_idx_t ui_zone;             // which side the UI is showing (persisted)

    // --- Onboarding (M4) ---
    // True for the whole session when the device booted with no stored Wi-Fi
    // credentials (set once in app_main from !dial_net_have_creds(), before
    // dial_net_bringup runs the portal). Gates SCR_WELCOME/SCR_SIDEPICK so an
    // already-provisioned device (upgraded firmware, never picked a side) is
    // never routed through onboarding again.
    bool fresh_device;
    // SCR_WELCOME dismissed (tap or knob). Session-only, deliberately NOT
    // persisted — the point is just to stop nav_policy from pinning the
    // welcome screen once the user acknowledges it.
    bool welcomed;
    // True once a default side is known: either the user picked one on
    // SCR_SIDEPICK, or (upgrade path) NVS already had a "zone" key from
    // before this flag existed. Restored from that key's *existence* in
    // dial_state_restore_prefs, not its value.
    bool side_picked;
    // SCR_TIMEZONE's setup-gate prompt dismissed this session (docs/SPEC-
    // timezone-source.md's "Fix 1"). Session-only, deliberately NOT
    // persisted, same reasoning as `welcomed` above -- but here the reason
    // cuts the other way: if the user skips and reboots, asking again is
    // CORRECT, because the clock is still genuinely wrong. Persisting a
    // "don't ask again" here would silently strand a device on UTC forever
    // after one dismissal.
    bool tz_prompted;

    // --- Settings (M4) ---
    // Display units: false = °F (canonical/internal — the store's temp_c is
    // always °C regardless), true = °C for display only. In RELATIVE mode this
    // governs only the absolute readouts that persist there (the measured water
    // caption), never the setpoint hero.
    bool units_c;
    // Temperature SCALE for the setpoint hero: false = absolute (°F/°C per
    // units_c), true = relative levels (−10…+10). Default is relative on a
    // fresh device, absolute on one upgrading from ≤v1.0.6 (see
    // dial_state_restore_prefs — an existing user must not have the big number
    // silently change meaning under an OTA). The wire is °C regardless; the
    // internal setpoint stays whole °F in both scales.
    bool rel_mode;
    // Screen rotation in quarter turns clockwise (0..3), persisted. The dial
    // sits on a nightstand and its cable exits one edge, so which way is "up"
    // is a property of the room, not of the device.
    uint8_t rotation;
    // Haptics feedback level, mirrored here so screens can read the current
    // setting; dial_haptics_set_level() is the actual enforcement point.
    // Mirrors dial_haptics.h's haptic_level_t values field-for-value without
    // depending on that header (same int-mirror trick as ota.status below):
    // 0=Off (nothing plays), 1=Auto (FIRM by day, SOFT during the sleep
    // window — today's pre-M7 "enabled" behavior, unchanged), 2=Low (SOFT
    // always), 3=High (FIRM always, including at 3am — a deliberate, global
    // user choice, not time-of-day-gated). The NVS key "haptics" is
    // unchanged from the old On/Off bool pref — every device in the field
    // already stores 0 or 1, and 1 already meant "the adaptive behavior", so
    // it maps to exactly Off/Auto with no migration; 2 (Low) and 3 (High)
    // are new. See dial_state_set_haptics_level.
    uint8_t haptics_level;
    // Day/night backlight brightness, 10..100 (percent), 10% steps. dial_power
    // scales its whole duty table (active/dimmed/standby together) by this at
    // apply time; dial_power_start() is what actually enforces it. Defaults to
    // 100 (today's exact brightness) both here and in NVS-absent restores —
    // see dial_state_get_bri_day_pct/set_bri_day_pct.
    uint8_t bri_day_pct;
    uint8_t bri_night_pct;
    // Night-only override for JUST the standby (screensaver clock) duty —
    // the face that glows in a dark bedroom all night, independent of the
    // brightness used while the dial is actually being interacted with at
    // night. Day is untouched: day's standby tier still follows bri_day_pct
    // like the rest of the day table. Same 10..100/10%-step range and
    // clamp-on-read contract as the pair above. Defaults to 100 on a fresh
    // device (matches bri_day_pct/bri_night_pct's own fresh-device default),
    // but an UPGRADING device must NOT see this default — see
    // dial_state_restore_prefs's migration comment: on that path it is
    // seeded from the device's own bri_night_pct instead, so a device
    // already dimmed at night keeps exactly the clock glow it has tonight
    // until the user deliberately pulls the two apart. See
    // dial_state_get_bri_night_clock_pct/set_bri_night_clock_pct.
    uint8_t bri_night_clock_pct;
    // Screen (lock/standby) timeout: idle seconds before dial_power drops the
    // display to DPWR_STANDBY (its "STANDBY" threshold — see dial_power.c's
    // power_task, which reads this live every 100ms tick, same as the
    // brightness prefs above). Settings' "Screen timeout" row offers exactly
    // DIAL_SCR_TIMEOUT_CHOICES above (5s/15s/30s/1m — deliberately no
    // "Never", see that table's comment). Default 90 is deliberately NOT one
    // of those five: it's the exact value this firmware hardcoded as
    // STANDBY_AFTER_US before this preference existed, so introducing it
    // changes zero devices' behavior until the user taps the row — see
    // dial_scr_timeout_label/_next for how the row displays and steps off
    // that off-menu default. Persisted to NVS "ui"/"scr_to"; see
    // dial_state_get_screen_timeout_s/set_screen_timeout_s.
    uint16_t screen_timeout_s;
    // Night window (docs/SPEC-night-window.md, Settings' "Night mode" row).
    // night_on == false disables night mode entirely (day palette, day duty,
    // day haptics, around the clock); the two times are then dormant, not
    // cleared, so switching back on restores the previous window. Minutes
    // from local midnight. Defaults on / 21:00 / 07:00 reproduce the fixed
    // window this firmware hardcoded before the setting existed, so an OTA
    // changes zero devices' behavior until the user taps a row -- same
    // discipline as screen_timeout_s's off-menu 90s default and rel_mode's
    // absolute-stays-absolute migration. Persisted to NVS "ui"/"night_on"
    // (u8), "ui"/"night_s" and "ui"/"night_e" (u16, nvs_set_u16/
    // nvs_get_u16 -- a u8 anywhere on the time path turns 22:00 (1320) into
    // 40 = 00:40 and the feature "works" with a phantom window; grepped for
    // at review time, see dial_state.c). Clamp-on-read is the only
    // corruption guard (dial_state_restore_prefs): a stored time outside
    // 0..1439, or a pair with start == end, snaps the PAIR back to
    // 21:00/07:00 (never one value alone); a stored flag outside {0,1}
    // snaps to on. See dial_night_active/dial_auto_update_window above and
    // dial_state_get_night_on/set_night_on etc. below.
    bool     night_on;
    uint16_t night_start_min;
    uint16_t night_end_min;
    // Night face (docs/SPEC-night-face.md §4, Settings' "Night face" row):
    // whether the night window's number-only layout is active, vs. the full
    // day-shaped face just dimmed. true = Number only, false = Full.
    // Consumed by exactly one place, scr_dial.c's apply_palette_and_state
    // (minimal = night && night_face_min) — §3's whole layout swap reads
    // this one flag. Default TRUE (Number only) on a fresh device --
    // deliberately NOT the "an OTA changes nothing" convention the rest of
    // the night window follows (night_on/night_start_min/night_end_min
    // above): this is presentation, not behavior (no write path, no timing,
    // no brightness change) and is the whole point of the beta -- the Full
    // row is the one-tap way back (§4). Persisted to NVS "ui"/"night_face"
    // (u8); clamp-on-read snaps anything outside {0,1} to 1 (Number only),
    // same defensive shape as night_on's own clamp -- see
    // dial_state_get_night_face_min/set_night_face_min below.
    bool     night_face_min;
    // Beta OTA channel opt-in (SCR_UPDATE's "Beta builds" toggle), persisted
    // to NVS "ui"/"beta". dial_state has no business knowing about dial_ota,
    // so this is just the stored preference -- the worker (main.c) reads it
    // out of its app_state_t snapshot and passes it to dial_ota_check(),
    // same division of labor as haptics_level/dial_haptics_set_level.
    // Default false (stable channel only) both here and in an NVS-absent
    // restore -- see dial_state_get_beta/dial_state_set_beta.
    bool beta;
    // "Dial adjusts" preference: true = Follow schedule, false = Hold
    // tonight. Originally meant a knob turn during tonight's active
    // sleep-schedule phase either retargeted just that phase (Follow, via
    // Orion's override_sleep_schedule_tonight) or set a plain hold for the
    // rest of the night (Hold) — matching the Orion phone app's own toggle.
    // That write path (main.c's temp_write_phase()/sleep_phase_now()) did
    // not survive the Somnus port's worker_task rewrite and was never
    // replaced: main.c has zero references to sched_follow today, so this
    // is currently a UI-only preference a user can change with no
    // downstream effect. Kept as a dormant field rather than pulled
    // outright — same treatment as app_state_t.away above — because
    // SCR_ADJUST_MODE (Schedule/Hold, still reachable via scr_dial.c's
    // power-disc long-press even though scr_settings.c's row to it is
    // hidden — see that file) and this NVS value are exactly what a real
    // dial-side scheduling write path would want to read once one exists;
    // see docs/SPEC-dial-side-scheduling.md. Persisted to NVS
    // "ui"/"sched_follow". Default TRUE (owner decision) both here and in an
    // NVS-absent restore -- see dial_state_get_sched_follow/
    // dial_state_set_sched_follow.
    bool sched_follow;

    // Somnus pad address + zone mode (Settings' "Pad Address"/"Bed Mode"
    // rows). Both start at the DIAL_PAD_DEFAULT_* fallback above until the
    // user edits them; from then on this is the one persisted source of
    // truth -- main.c's worker reads it at boot instead of the compiled
    // SOMNUS_DEFAULT_* macros, and re-applies it live via
    // CMD_PAD_SETTINGS_CHANGED whenever either setter below runs (see
    // dial_somnus.h's own header note asking for exactly this design).
    // pad_single_zone true = "One Bed" (Somnus app terminology), false =
    // "Dual Sides" -- mirrors dial_somnus_get_zone_mode()'s single_zone
    // sense exactly. Persisted to NVS "ui"/"pad_url" and "ui"/"pad_1zone".
    char pad_base_url[DIAL_PAD_URL_MAX_LEN + 1];
    bool pad_single_zone;

    // --- OTA (M6) ---
    // Mirrors dial_ota_info_t (components/dial_ota/dial_ota.h) field-for-
    // field, without a direct dependency on it: dial_state is a leaf
    // component (no REQUIRES beyond esp_timer/nvs_flash), so it can't
    // #include dial_ota.h. The worker (main.c, which includes both) commits
    // this after every dial_ota_* call; `status` holds a dial_ota_status_t
    // value (int-cast, same enum ordering/values on both sides).
    struct {
        int  status;
        char latest[16];
        int  progress_pct;
        char err[96];
        // Mirrors dial_ota_info_t.pending_verify -- true while this boot is
        // still ESP_OTA_IMG_PENDING_VERIFY (a fresh OTA install the
        // bootloader will roll back on the next reset/power-cycle unless it
        // survives to confirm). bool needs no int-cast trick like `status`
        // above: it's a plain stdbool.h type on both sides, not an enum
        // defined in dial_ota.h. Drives scr_connecting's/scr_dial's
        // "Finalizing update -- keep powered" notice.
        bool pending_verify;
        // True while an install the USER never asked for is running (the
        // overnight auto-updater). nav_policy uses it to leave a SLEEPING
        // dial dark rather than lighting the takeover screen at a bedside
        // for two minutes to narrate an update nobody is watching. Manual
        // installs clear it and always take over — the user is standing
        // there, and a dial that reboots under their hands unexplained is
        // worse than the interruption.
        bool unattended;
    } ota;

    // --- Update prompt / auto-update (docs/SPEC-update-prompt.md) ---
    // Auto-update setting (SCR_UPDATE's "Auto-update" row): 0 = Off,
    // 1 = Overnight. Persisted to NVS "ui"/"ota_auto". Default 0 (Off) both
    // here and on an NVS-absent restore -- spec's explicit consent
    // requirement (a device that silently reboots itself must not surprise
    // someone who never asked for it). Not a bool/haptics_level-style enum
    // mirror of anything in dial_ota.h -- this preference is dial_state's own,
    // dial_ota has no concept of "overnight".
    uint8_t ota_auto;
    // "Later" defer timestamp: an epoch-seconds wall clock value (survives
    // reboots correctly, unlike an uptime timer) before which the update
    // prompt must not be shown again. Persisted to NVS "ui"/"ota_defer".
    // Default 0, which is always in the past -- a fresh device has nothing
    // to defer. Set to now+23h (deliberately not 24h -- see the spec) by
    // scr_update_prompt.c's "Later" action and its swipe-dismiss equivalent.
    uint32_t ota_defer;
    // Exact version string ("X.Y.Z"/"X.Y.Z-beta.N") the user chose to skip
    // via SCR_UPDATE's "Skip this version" row -- an EXACT match against
    // ota.latest suppresses the prompt for that version only; anything
    // newer (a different string) prompts again. Persisted to NVS
    // "ui"/"ota_skip". Default "" (nothing skipped).
    char ota_skip[16];
    // Epoch seconds the update prompt was last actually shown -- the
    // once-per-24h ceiling on prompt frequency, independent of "Later"'s own
    // defer window (a device that's never been shown the prompt has this at
    // 0, which is always >=24h in the past). Persisted to NVS "ui"/"ota_shown".
    // Committed by the worker (main.c) the instant it raises ota_prompt_due
    // below -- see dial_state_set_ota_shown.
    uint32_t ota_shown;
    // Session-only (deliberately NOT persisted): "is the update prompt sheet
    // raised right now" (docs/SPEC-update-prompt.md's 2026-07-29 rework).
    // Raised by the worker (main.c) on a WAKE EDGE
    // (DPWR_STANDBY/DPWR_DIMMED -> DPWR_ACTIVE) when every entry gate holds
    // at that instant -- NOT a continuously re-evaluated condition; once
    // raised it stays true until the worker's own three separate exit
    // checks fire (display back to STANDBY, night begins, update no longer
    // available) or scr_update_prompt.c clears it directly on a deliberate
    // user action. Mirrors s_ui_night's edge-triggered commit pattern (only
    // touched on an actual change), but the edge it tracks is now "just
    // woke", not "gate table still holds" -- the earlier continuous
    // re-evaluation was the mid-reach-withdrawal bug this rework fixed (a
    // touch resets idle time, which used to be one of the gates).
    // nav_policy routes to SCR_UPDATE_PROMPT exactly when this is true (and
    // the dial face is what's showing -- see nav_policy's own comment for
    // why it's scoped to SCR_DIAL). Cleared immediately by
    // scr_update_prompt.c on every exit path (Update now / Later / Update
    // options / swipe-dismiss) via dial_state_clear_ota_prompt_due(), so a
    // deliberate dismissal can never race the worker's own next tick.
    bool ota_prompt_due;

    // Bumped on every commit; the UI dispatcher re-renders when it changes.
    uint32_t generation;
} app_state_t;

/*
 * Zone-count helpers (single-zone toppers). Before have_state nothing is known
 * about the device, so these answer for the layout the UI is currently drawing:
 * dial_state_is_dual() is false until the device says otherwise, which keeps a
 * booting dial from flashing a partner face that may not exist.
 */
static inline bool dial_state_is_dual(const app_state_t *st)
{
    return st->zone_present[ZONE_A] && st->zone_present[ZONE_B];
}

// The zone a single-zone device actually has. ZONE_A unless the device
// reports only ZONE_B. (This used to also double as "the side whose
// schedule the worker's overnight write-path follows," via
// sleep_phase_now()/temp_write_phase() in main.c — those functions didn't
// survive the Somnus port's worker_task rewrite; see app_state_t.sched_
// follow's own comment above and docs/SPEC-dial-side-scheduling.md for
// that feature's current, dormant state.)
static inline zone_idx_t dial_state_primary_zone(const app_state_t *st)
{
    return (!st->zone_present[ZONE_A] && st->zone_present[ZONE_B]) ? ZONE_B : ZONE_A;
}

// Effective ABSOLUTE-mode temperature range, tenths of °C: the device's own
// reported rails once discovery has found them, else the
// DIAL_TEMP_MIN_DC/MAX_DC fallback (see app_state_t.temp_min_dc/temp_max_dc's
// comment). Every absolute-mode consumer (scr_dial.c's arc range, knob
// clamp, drag clamp) reads the range through these two, never the macros or
// the raw fields directly, so there is exactly one place that decides
// "known vs. fallback".
static inline int dial_state_temp_min_dc(const app_state_t *st)
{
    return st->temp_min_dc >= 0 ? st->temp_min_dc : DIAL_TEMP_MIN_DC;
}
static inline int dial_state_temp_max_dc(const app_state_t *st)
{
    return st->temp_max_dc >= 0 ? st->temp_max_dc : DIAL_TEMP_MAX_DC;
}

// Night mode presets (docs/SPEC-night-window.md §3/§5) -- Settings' "Night
// mode" row opens a three-choice dial_list (Back, Off, one row per preset
// here). Labels use whole hours with no ":00" because every preset is on
// the hour, keeping the Settings value column short; a future custom editor
// would render its own values as "h:mm am" and leave these short forms
// alone. Three values total (Off is night_on == false, not a third row in
// these tables) -- see app_state_t.night_on's comment for why Off is a flag
// rather than a sentinel in here.
#define DIAL_NIGHT_PRESETS_N 2
static const uint16_t DIAL_NIGHT_PRESET_START[DIAL_NIGHT_PRESETS_N] = { 21 * 60, 22 * 60 };
static const uint16_t DIAL_NIGHT_PRESET_END  [DIAL_NIGHT_PRESETS_N] = {  7 * 60,  6 * 60 };
static const char *const DIAL_NIGHT_PRESET_LABEL[DIAL_NIGHT_PRESETS_N] = { "9 pm - 7 am", "10 pm - 6 am" };

// True while night mode is actually engaged right now (docs/SPEC-night-
// window.md §4) -- takes the whole app_state_t, not just the two times,
// because it also has to consult the night_on flag.
static inline bool dial_night_active(const app_state_t *st, int now_min)
{
    return st->night_on && dial_in_window(st->night_start_min, st->night_end_min, now_min);
}

// Post-wake install window (docs/SPEC-night-window.md §6): two hours after
// night ends, two hours wide. This is what Orion derived from the account's
// sleep schedule; the Somnus port lost the schedule and froze the result at
// 09:00-11:00, which is right only for a 07:00 riser. Falls back to that
// same fixed pair when night is off, since there is no wake time to derive
// from. With the defaults this computes 09:00-11:00 -- byte-identical to
// the fixed window this firmware hardcoded before this setting existed.
static inline void dial_auto_update_window(const app_state_t *st, int *start, int *end)
{
    if (!st->night_on) { *start = 9 * 60; *end = 11 * 60; return; }
    *start = (st->night_end_min + 120) % 1440;
    *end   = (*start + 120)            % 1440;
}

// Initialize the store (mutex + defaults). Call once before any other call.
void dial_state_init(void);

// Restore persisted UI preferences (last shown side). Requires NVS to be
// initialized, so call after dial_net_init, before the first real screen.
void dial_state_restore_prefs(void);

// Copy the current state under the store mutex.
void dial_state_get(app_state_t *out);

// Run `mutate` on the live state under the mutex, then bump the generation.
void dial_state_commit(void (*mutate)(app_state_t *st, void *arg), void *arg);

// Convenience: set the connection phase (+ optional error text, NULL to keep).
void dial_state_set_phase(conn_phase_t phase, const char *err);

// Update app_state_t.power_mv under the store mutex with NO generation bump
// (docs/SPEC-power-sensing.md §10.3) -- dial_power.c's power_task calls this
// every 1s sample; About's Power row picks the new value up on its next
// on_state regardless (dial_state_get() always returns the live store, gen
// bump or not). power_src changes go through dial_state_commit() instead, so
// a real transition still wakes every screen.
void dial_state_set_power_mv(uint16_t mv);

// Same shape as dial_state_set_power_mv above, for app_state_t.power_pct
// (docs/SPEC-power-sensing.md §11.2) -- dial_power.c calls this right
// alongside the power_mv setter, on every sample, with -1 whenever
// power_src isn't PWR_BATTERY. No commit here either; About's Battery row
// and the dial-face/standby badge pick up the new value on whatever
// on_state a power_src commit (or anything else) next triggers.
void dial_state_set_power_pct(int8_t pct);

// Hot-path setter used by the dial screen during knob/drag interaction.
// temp_dc is the canonical unit, tenths of °C — see the block comment above
// zone_idx_t.
void dial_state_set_ui_temp(zone_idx_t zone, int temp_dc);

// Optimistic power flip — call from the UI on tap, so the face answers the
// press instead of waiting for the write to the pad to come back. The next
// poll reconciles (and reverts it, if the write failed).
void dial_state_set_zone_on(zone_idx_t zone, bool on);

// Record which side the UI is showing. The nav policy follows this, so any
// screen that switches sides MUST commit it here (or the next state commit
// navigates right back — the side choice lives in the store, not the router).
void dial_state_set_ui_zone(zone_idx_t zone);

// --- Onboarding / settings setters (M4) ---
// Dismiss SCR_WELCOME. Not persisted (see app_state_t.welcomed).
void dial_state_set_welcomed(void);
// Mark that a default side is known (see app_state_t.side_picked). Callers
// that pick a side also call dial_state_set_ui_zone() to persist it.
void dial_state_set_side_picked(void);
// Dismiss SCR_TIMEZONE's setup-gate prompt for this session. Not persisted
// (see app_state_t.tz_prompted) -- call whenever the user leaves that screen,
// whichever way, so nav_policy's gate stops re-forcing it but asks again
// next boot if the zone is still unset.
void dial_state_set_tz_prompted(void);
// Set the display-units preference; persists to NVS "ui"/"units".
void dial_state_set_units_c(bool units_c);
// Set the temperature-scale preference (relative vs absolute); persists to NVS
// "ui"/"relmode".
void dial_state_set_rel_mode(bool rel_mode);
// Set the haptics feedback level (0=Off/1=High/2=Low — see app_state_t.
// haptics_level above); persists to NVS "ui"/"haptics" immediately. Does NOT
// itself call dial_haptics_set_level() — dial_state has no business knowing
// about the haptics driver, so callers do both. `level` is not typed
// haptic_level_t: dial_state can't #include dial_haptics.h (leaf component),
// same reasoning as ota.status's int mirror.
void dial_state_set_haptics_level(uint8_t level);

// Day/night backlight brightness preference, 10..100 (percent, 10% steps).
// Getters always return a value in that range (clamped on read; 100 when no
// "bri_day"/"bri_night" NVS key has ever been written). Setters clamp too and
// persist immediately to NVS "ui"/"bri_day" and "ui"/"bri_night" — same
// immediate-commit pattern as dial_state_set_haptics_level above. dial_state
// has no business knowing about the backlight driver, so callers that want the
// new brightness to actually take effect must also call
// dial_power_brightness_changed() (dial_power.h), same division of labor as
// haptics_level/dial_haptics_set_level().
uint8_t dial_state_get_bri_day_pct(void);
uint8_t dial_state_get_bri_night_pct(void);
void    dial_state_set_bri_day_pct(uint8_t pct);
void    dial_state_set_bri_night_pct(uint8_t pct);

// Night-clock (screensaver-at-night) brightness override — see app_state_t.
// bri_night_clock_pct's comment for what it governs and how it migrates.
// Same getter/setter/clamp/NVS-immediate-commit shape as the pair above,
// persisted to NVS "ui"/"bri_nclk".
uint8_t dial_state_get_bri_night_clock_pct(void);
void    dial_state_set_bri_night_clock_pct(uint8_t pct);

// Screen (lock/standby) timeout preference, seconds (see app_state_t.
// screen_timeout_s above for the choice set/default/NVS key). Same
// clamp-on-read/immediate-commit shape as the brightness pair, except the
// clamp snaps an out-of-range/corrupt stored value to 90 (today's legacy
// behavior), not to the nearest UI choice — a corrupt byte should fail back
// to the ORIGINAL fixed threshold, not to whatever choice happens to be
// numerically closest. The setter does NOT itself call into dial_power —
// power_task reads this getter fresh every 100ms tick (same as the
// brightness pair), so a change reaches the standby/dim decision within one
// tick with no separate "changed" hook required; see dial_power.c.
uint16_t dial_state_get_screen_timeout_s(void);
void     dial_state_set_screen_timeout_s(uint16_t seconds);

// Night window preference (see app_state_t.night_on/night_start_min/
// night_end_min above for defaults/NVS keys/the pair's clamp-on-read
// contract). Same getter+setter/immediate-commit shape as the pair above,
// no "changed" hook — worker_task (dial_night_active/
// dial_auto_update_window, dial_state.h) re-reads the snapshot every
// steady-state tick. dial_state_set_night_start_min/_end_min do NOT flip
// night_on — SCR_NIGHT_MODE calls all three in sequence (start, end, flag
// last) so the worker never observes on=true paired with a half-written
// window; see that screen's header comment.
bool     dial_state_get_night_on(void);
void     dial_state_set_night_on(bool on);
uint16_t dial_state_get_night_start_min(void);
void     dial_state_set_night_start_min(uint16_t min);
uint16_t dial_state_get_night_end_min(void);
void     dial_state_set_night_end_min(uint16_t min);

// Night face preference (see app_state_t.night_face_min above for the
// default/NVS key/clamp contract). Getter/setter copied from night_on's
// exact shape -- direct mutex mutate + generation bump + immediate NVS
// commit, no dial_state_commit() wrapper and no "changed" hook: the face
// re-renders on the next on_state (dial_ui's dispatcher already re-runs
// every screen's on_state on a generation bump), same as night_on's own
// consumer (dial_night_active, re-read every steady-state tick).
bool     dial_state_get_night_face_min(void);
void     dial_state_set_night_face_min(bool minimal);

// Beta OTA channel preference (see app_state_t.beta above). Same
// getter+setter shape as the brightness pair; setter persists immediately to
// NVS "ui"/"beta".
bool dial_state_get_beta(void);
void dial_state_set_beta(bool enabled);

// "Dial adjusts" preference (see app_state_t.sched_follow above). Same
// getter+setter shape as beta; setter persists immediately to NVS
// "ui"/"sched_follow".
bool dial_state_get_sched_follow(void);
void dial_state_set_sched_follow(bool follow);

// Somnus pad address + zone-mode preference (see app_state_t.pad_base_url/
// pad_single_zone above). Same getter+setter shape as beta/sched_follow;
// dial_state_get_pad_url copies into a caller-owned buffer of at least
// DIAL_PAD_URL_MAX_LEN+1 bytes (a URL doesn't fit the plain "return by
// value" shape the bool/uint8_t prefs use). Setters persist immediately to
// NVS "ui"/"pad_url" and "ui"/"pad_1zone" respectively. Neither setter
// touches dial_somnus itself -- dial_state has no business knowing about
// the pad client, same division of labor as haptics_level/
// dial_haptics_set_level -- the worker (main.c) is what actually calls
// dial_somnus_connect()/dial_somnus_set_zone_mode(), via
// CMD_PAD_SETTINGS_CHANGED below.
void dial_state_get_pad_url(char *out, size_t out_sz);
void dial_state_set_pad_url(const char *url);
bool dial_state_get_zone_mode(void);
void dial_state_set_zone_mode(bool single_zone);

// --- Update prompt / auto-update (see app_state_t's comments above for what
// each field means and its NVS key). All four setters persist immediately,
// same shape as dial_state_set_beta -- callers are rare taps in the LVGL
// task (SCR_UPDATE's Auto-update/Skip rows, scr_update_prompt.c's Later
// action), except dial_state_set_ota_shown, which the WORKER calls the
// instant it raises ota_prompt_due (main.c's idle loop; a ~ms NVS write
// there is fine, it happens at most once a day).
void dial_state_set_ota_auto(uint8_t mode);
void dial_state_set_ota_defer(uint32_t epoch);
void dial_state_set_ota_skip(const char *version);
void dial_state_set_ota_shown(uint32_t epoch);

// Clears the session-only ota_prompt_due flag (see app_state_t's comment).
// Called directly from the LVGL task by scr_update_prompt.c's every exit
// path -- NOT persisted, no NVS write, just an immediate commit (same shape
// as dial_state_set_welcomed).
void dial_state_clear_ota_prompt_due(void);

// Screen rotation, quarter turns clockwise (0..3). Persisted; apply it to the
// panel with dial_display_set_rotation.
void dial_state_set_rotation(uint8_t quarters);

// On-device Wi-Fi setup: what the dial is currently trying to join (called just
// before the credentials are handed over), and whether that attempt was
// rejected. See app_state_t's wifi_join_* fields.
void dial_state_set_wifi_join(int idx, const char *ssid);
void dial_state_set_wifi_join_failed(void);
void dial_state_clear_wifi_join_failed(void);

// --- Input quiet-period gate (torn-read-safe on 32-bit) ---
void    dial_state_stamp_input(void);   // call on EVERY user input
int64_t dial_state_last_input_us(void);

/*
 * UI -> worker command queue. Screens post; the (single) network worker
 * drains, coalescing bursts (a knob spin collapses to one set_zone).
 */
// Trimmed for the Somnus pad's local API in place of the Orion command set
// above/before it: the pad's whole write surface is POST /api/power and
// POST /api/target_t (dial_somnus_set_power/dial_somnus_set_temp), so
// CMD_BOOST_START/CANCEL (no thermal-relief concept on the pad) and CMD_AWAY
// (no away-mode endpoint) are gone, and CMD_RELINK goes with them — there is
// no auth token to forget, dial_somnus is a plain unauthenticated local
// client (see dial_somnus.h). The Settings-destructive and OTA commands are
// device-agnostic (dial_net/dial_ota, not Orion/Somnus) and are unchanged.
typedef enum {
    CMD_SET_TEMP,      // zone + temp_dc
    CMD_TOGGLE_ON,     // zone + a = the DESIRED on state (1/0), not "flip it".
                       // The UI flips the store optimistically before posting,
                       // so a worker that re-derived !current would undo it.

    // Settings (M4) destructive actions — each erases some NVS state and
    // reboots. Handled in main.c's handle_immediate_cmd like the others
    // above; none of them return (esp_restart()).
    CMD_WIFI_RESET,      // clear Wi-Fi credentials
    CMD_FACTORY_RESET,   // erase all of NVS

    // Software update (M6/M7), from SCR_UPDATE's "Check for updates" row.
    CMD_OTA_CHECK,       // -> dial_ota_check(st.beta); zone/a/b unused
    CMD_OTA_APPLY,       // -> dial_ota_download_and_apply() + reboot on
                         // success; worker drops this unless ota.status is
                         // already OTA_AVAILABLE (stale-tap guard)
    // Posted by scr_update.c's destroy() (screen teardown, M7 -- scr_about.c
    // before it) so a FAILED "Check for updates" row never survives to the
    // next visit -- OTA_FAILED isn't allowed to be terminal (field bug: it
    // used to stay wedged until a manual power cycle). ->
    // dial_ota_clear_stale_failure(0); zone/a/b unused. A no-op if status
    // has already moved off OTA_FAILED. See also main.c's periodic
    // idle-loop call for the time-based ~25s auto-clear that covers the
    // case where the user never leaves the screen at all.
    CMD_OTA_CLEAR_FAILED,

    // Settings' "Pad Address"/"Bed Mode" rows just persisted a new value via
    // dial_state_set_pad_url()/dial_state_set_zone_mode() -- zone/a/b unused.
    // The worker re-reads both from the store and re-applies them to
    // dial_somnus live (dial_somnus_set_zone_mode() always;
    // dial_somnus_connect() again to re-probe the (possibly new) address),
    // rather than requiring a reboot. See main.c's handle_immediate_cmd.
    CMD_PAD_SETTINGS_CHANGED,

    // Settings' "Timezone" row (docs/SPEC-timezone-source.md's Threading
    // section) -- `a` = index into DIAL_TZ_IANA/DIAL_TZ_LABEL above. Unlike
    // CMD_PAD_SETTINGS_CHANGED, the row could NOT persist this itself before
    // posting: dial_time_set_iana_tz() mutates global libc TZ state that
    // worker_task's dial_time_now() callers read concurrently at steady
    // state, so the call has to happen ON worker_task, not the LVGL task --
    // there is no dial_state-owned, mutex-protected store standing in front
    // of it the way pad_base_url has. -> dial_time_set_iana_tz(DIAL_TZ_IANA[a]).
    CMD_TZ_CHANGED,
} cmd_kind_t;

typedef struct {
    cmd_kind_t kind;
    zone_idx_t zone;
    int        temp_dc; // CMD_SET_TEMP, tenths of °C (canonical unit)
    int        a, b;    // generic args: CMD_TOGGLE_ON's `a` = desired on state,
                         // CMD_TZ_CHANGED's `a` = index into DIAL_TZ_IANA/
                         // DIAL_TZ_LABEL above; `b` is still unused by anything,
                         // kept for shape/alignment with dial_cmd_post's callers
                         // and any future command that needs a second.
} app_cmd_t;

void dial_cmd_post(const app_cmd_t *cmd);
bool dial_cmd_receive(app_cmd_t *out, int timeout_ms);
