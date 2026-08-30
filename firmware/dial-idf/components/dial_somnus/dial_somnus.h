#pragma once
#include <stdbool.h>
#include <stdint.h>

/*
 * dial_somnus — replacement for dial_mcp, talking directly to a Somnus Pad's
 * own local HTTP API (see the official local_api-2.yml spec, v0.2.0, and
 * somnus_bed.py / bed_web_app.py in the reference Python project this was
 * ported from).
 *
 * No cloud, no auth, no OAuth, no MCP envelope: the pad exposes a tiny plain
 * JSON REST API on the local network (default 192.168.1.169:8080):
 *
 *   GET  /api/state      -> current state for both sides
 *   POST /api/power      -> {"side0": {"is_on": true}, "side1": {...}}
 *   POST /api/target_t   -> {"side0": {"target_t": 20.0}, "side1": {...}}
 *
 * Deliberately talks to the PAD directly, not through bed_web_app.py's Flask
 * layer — the dial must keep working even if the Mac/NAS running that Flask
 * app is asleep or offline.
 *
 * ---- Single-zone vs. dual-zone (IMPORTANT) --------------------------------
 * The Somnus app has a "One Bed" / "Dual Sides" setting -- but the local API
 * has NO field anywhere that reports which mode the pad is currently in.
 * Per the official spec: in single-zone ("One Bed") mode, writes to side1
 * are explicitly undefined behavior ("firmware mirrors power states between
 * sides... values set on side0 are automatically mirrored to side1"). This
 * component can't detect the mode on its own.
 *
 * This is a runtime setting, not a compile-time one: it lives in the dial's
 * own Settings screen ("Bed Mode"/"Pad Address" rows, scr_settings.c /
 * scr_pad_address.c), NVS-backed via dial_state_get/set_zone_mode and
 * dial_state_get/set_pad_url (same mechanism as brightness, haptics, etc.),
 * so both can be changed on the device itself without a reflash. See
 * dial_somnus_set_zone_mode() and dial_somnus_connect() below. main.c's
 * worker task reads both from dial_state's persisted settings at boot before
 * the first real request, and re-applies them live on every
 * CMD_PAD_SETTINGS_CHANGED (posted whenever either Settings row commits a
 * change) rather than waiting for a reboot.
 *
 * Threading: same rules as dial_mcp — call only from the worker task, never
 * from the LVGL task. Not reentrant; the worker is already single-threaded
 * for network calls.
 */

// Compile-time fallback only, used if no base URL has ever been configured
// via Settings yet (e.g. very first boot before the user's entered one).
// Once dial_somnus_connect() has been called with a real address, prefer
// that over this default everywhere else in the codebase.
#define SOMNUS_DEFAULT_BASE_URL "http://192.168.1.169:8080"

// Same idea for zone mode: this is only the fallback used before the user
// has ever set the toggle in Settings. true = single-zone ("One Bed").
#define SOMNUS_DEFAULT_SINGLE_ZONE_MODE true

typedef enum { SOMNUS_SIDE_0 = 0, SOMNUS_SIDE_1 = 1, SOMNUS_SIDE_COUNT = 2 } somnus_side_t;

typedef struct {
    bool  is_on;
    bool  has_current;     // false until the pad has a real reading (first boot) --
                            // spec: current_t is nullable, "Null before first
                            // sensor reading"
    float current_c;       // only valid if has_current
    float target_c;        // spec: 12-42.3. Real-pad test (2026-08-30): POSTed a
                            // fractional target_t (20.5 against a pad sitting at
                            // 20.0) and a GET ten seconds later still read 20.5 --
                            // the pad accepts and holds fractional values, no
                            // snapping to whole degrees. (Caveat: the pad was
                            // powered off during that test, so this confirms
                            // storage granularity, not control-loop granularity.)
                            // The dial's own knob still steps in whole 1.0°C
                            // detents by deliberate design (matches the Somnus
                            // app's own scale), NOT because the pad requires it --
                            // see dial_state.h's canonical-unit comment.
    bool  water_low;       // spec field name: is_wl_low
} somnus_side_state_t;

typedef struct {
    somnus_side_state_t side[SOMNUS_SIDE_COUNT];
    bool system_error;     // spec field name: error, "True when a fatal error
                            // is active on the device"
} somnus_state_t;

// Point the client at the pad. base_url like "http://192.168.1.169:8080",
// no trailing slash. Call once, early — does an initial reachability probe
// via a real GET, there is no session to open. If the user changes the IP
// in Settings later, call this again with the new address before the next
// request — there's no persistent connection to tear down first.
bool dial_somnus_connect(const char *base_url);

// Set which write-safety mode is active. true = single-zone ("One Bed") --
// every side1 write is silently skipped, per the spec's undefined-behavior
// warning (see the header note above). false = dual-zone, writes to either
// side proceed normally. Call this whenever the Settings screen's toggle
// changes, and once on boot after loading the persisted value from NVS.
// Defaults to SOMNUS_DEFAULT_SINGLE_ZONE_MODE if never called.
void dial_somnus_set_zone_mode(bool single_zone);

// Current zone mode, for the Settings screen to reflect back and for
// anything else that needs to know without duplicating the state itself.
bool dial_somnus_get_zone_mode(void);

// GET /api/state, parsed into out. Returns false on transport/parse error
// (check dial_somnus_last_error()); out is left unmodified on failure so a
// failed poll can never clobber the last-known-good state. Always reads
// BOTH side0 and side1 regardless of SOMNUS_SINGLE_ZONE_MODE — in single-
// zone mode side1 will simply mirror side0's values (per spec), which is
// harmless to read, only harmful to write.
bool dial_somnus_get_state(somnus_state_t *out);

// POST /api/target_t for one side. temp_c is clamped to the pad's own
// 12.0-42.3 range by the pad itself — this call does not re-clamp, so an
// out-of-range value still round-trips and you can see what the pad
// actually applied on the next poll.
//
// double, not float: the caller (main.c) computes this straight from an
// integer tenths-of-°C value via a plain `/ 10.0` double division, so the
// exact double reaches cJSON_CreateNumber() with no float->double promotion
// in between. That promotion used to be a real bug — a float32 rounding
// error promoted to double made cJSON's own round-trip-or-fallback printer
// emit 15-17 digits of noise (e.g. "21.700000762939453") instead of a clean
// value. Passing a float here would silently reintroduce it, so don't.
//
// If SOMNUS_SINGLE_ZONE_MODE is true and side == SOMNUS_SIDE_1, this is a
// silent no-op that returns true without making any network call — per
// spec, side1 writes are undefined in this mode, so we simply never send
// one, rather than trusting the pad to ignore it safely.
bool dial_somnus_set_temp(somnus_side_t side, double temp_c);

// POST /api/power for one side. Same SOMNUS_SINGLE_ZONE_MODE guard as
// dial_somnus_set_temp — side1 writes are silently skipped, not sent.
bool dial_somnus_set_power(somnus_side_t side, bool on);

// Drop any kept-alive connection. Currently a no-op (see .c) — kept for
// interface symmetry with dial_mcp in case keep-alive is added later.
void dial_somnus_release_connection(void);

// Most recent transport/HTTP error, for display on the connecting/degraded
// screens. NULL if the last call succeeded. A skipped side1 write in
// single-zone mode is NOT an error and does not set this.
const char *dial_somnus_last_error(void);
