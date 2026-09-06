/*
 * main.c — the simulator harness. Brings up LVGL against a 360x360 host
 * framebuffer, registers the real firmware's screens (ui_screens_register_all,
 * the actual firmware/dial-idf/components/dial_ui sources), then for each
 * named scenario: builds a demo app_state_t, navigates the real router to
 * the real screen, pumps simulated time so creation/animation settle, and
 * writes a circularly-masked PNG to docs/screens/.
 *
 * No touchscreen or knob hardware exists here, but the INPUTS still go
 * through the real code paths: ui_router_knob_input() is the same entry
 * point the knob decoder's esp_timer task calls on real hardware, and the
 * one screen that needs a simulated tap (scr_passkey, to show a few
 * characters already typed) gets it through a real LV_INDEV_TYPE_POINTER
 * indev — the same press/release/CLICKED pipeline a finger on the panel
 * would drive. Nothing here reaches into a screen's file-static state.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "lvgl.h"
#include "ui_router.h"
#include "ui_screens.h"
#include "dial_state.h"
#include "dial_palette.h"
#include "dial_ota.h"
#include "sim_state.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define SCREEN_W 360
#define SCREEN_H 360

#ifndef DIAL_SIM_OUTPUT_DIR
#define DIAL_SIM_OUTPUT_DIR "docs/screens"
#endif

// The "available" version every OTA scenario advertises. One constant rather
// than a literal per scenario, because it has to stay AHEAD of the version the
// simulator reports as installed (stubs.c's esp_app_desc_t, which tracks
// PROJECT_VER) — otherwise the screenshots show a dial offering to update
// itself to something it already runs. Bump it with each release.
#define SIM_OTA_LATEST "1.4.3"

/* ---- host framebuffer + LVGL display driver ----------------------------- */

static uint16_t s_host_fb[SCREEN_W * SCREEN_H];
static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t s_lv_buf[SCREEN_W * SCREEN_H];   /* full-frame, single buffer */

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    for (lv_coord_t y = area->y1; y <= area->y2; y++) {
        for (lv_coord_t x = area->x1; x <= area->x2; x++) {
            if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
                s_host_fb[y * SCREEN_W + x] = color_p->full;
            color_p++;
        }
    }
    lv_disp_flush_ready(drv);
}

/* ---- simulated pointer indev (for scr_passkey's pre-fill tap) ----------- */

static bool      s_ptr_pressed;
static lv_coord_t s_ptr_x, s_ptr_y;

static void indev_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;
    data->point.x = s_ptr_x;
    data->point.y = s_ptr_y;
    data->state = s_ptr_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

/* ---- time pump / input helpers ------------------------------------------ */

static void pump_ms(int ms)
{
    for (int t = 0; t < ms; t += 5) {
        lv_tick_inc(5);
        lv_timer_handler();
    }
}

// Pumps simulated time until LVGL reports no animations left running (a
// sheet slide, scroll momentum, an elastic-overscroll bounce-back — anything
// driven by lv_anim), or max_ms elapses as a backstop. A fixed pump_ms(N)
// is a guess against however long those animations turn out to take; this
// instead gates on LVGL's own bookkeeping so a capture never lands mid-anim.
static void pump_until_idle(int max_ms)
{
    int waited = 0;
    while (lv_anim_count_running() > 0 && waited < max_ms) {
        lv_tick_inc(5);
        lv_timer_handler();
        waited += 5;
    }
}

// Turns the knob `detents` and lets one dispatcher tick (50ms) drain it into
// the active screen's on_knob — the same accumulate-then-drain path the real
// decoder's esp_timer task and ui_router.c's dispatch_tick implement.
static void sim_knob(int detents)
{
    ui_router_knob_input(detents);
    pump_ms(100);
}

// A real press-then-release at an absolute screen coordinate, through the
// pointer indev registered below — drives LV_EVENT_PRESSED/RELEASED/CLICKED
// on whatever widget actually sits there, exactly as a finger would.
static void sim_tap(lv_coord_t x, lv_coord_t y)
{
    s_ptr_x = x; s_ptr_y = y;
    s_ptr_pressed = true;
    pump_ms(60);
    s_ptr_pressed = false;
    pump_ms(60);
}

// A press at (x0,y0) that travels to (x1,y1) and is HELD there — the same
// pointer indev as sim_tap, so LV_EVENT_PRESSED lands on whatever sits at
// the start point and LV_EVENT_PRESSING follows the point. Two intermediate
// steps so the move looks like a finger, not a teleport. sim_release() ends
// it (LV_EVENT_RELEASED). Used to drive scr_dial's setpoint handle.
static void sim_drag_to(lv_coord_t x0, lv_coord_t y0, lv_coord_t x1, lv_coord_t y1)
{
    s_ptr_x = x0; s_ptr_y = y0;
    s_ptr_pressed = true;
    pump_ms(60);
    for (int i = 1; i <= 3; i++) {
        s_ptr_x = x0 + (x1 - x0) * i / 3;
        s_ptr_y = y0 + (y1 - y0) * i / 3;
        pump_ms(60);
    }
}

static void sim_release(void)
{
    s_ptr_pressed = false;
    pump_ms(60);
}

/* ---- PNG output ----------------------------------------------------------*/

static void ensure_dir(const char *path)
{
    mkdir(path, 0755);   // ignores EEXIST-equivalent; good enough for our own tree
}

// Converts the host RGB565 framebuffer to RGBA8888 and stamps a 180px-radius
// anti-aliased circular alpha mask over it (the round panel), then writes it
// as docs/screens/<name>.png.
static int s_snapshots_written;   // counted for the "done:" summary — a
                                  // hardcoded total drifted the first time a
                                  // scenario was added

static void snapshot(const char *name)
{
    static uint8_t rgba[SCREEN_W * SCREEN_H * 4];
    const float cx = SCREEN_W / 2.0f, cy = SCREEN_H / 2.0f, r = 180.0f;

    for (int y = 0; y < SCREEN_H; y++) {
        for (int x = 0; x < SCREEN_W; x++) {
            uint16_t px = s_host_fb[y * SCREEN_W + x];
            uint8_t r5 = (px >> 11) & 0x1F;
            uint8_t g6 = (px >> 5) & 0x3F;
            uint8_t b5 = px & 0x1F;
            uint8_t rr = (uint8_t)((r5 * 255 + 15) / 31);
            uint8_t gg = (uint8_t)((g6 * 255 + 31) / 63);
            uint8_t bb = (uint8_t)((b5 * 255 + 15) / 31);

            float dx = (x + 0.5f) - cx, dy = (y + 0.5f) - cy;
            float dist = sqrtf(dx * dx + dy * dy);
            float alpha = (r - dist) / 1.5f;      // ~1.5px anti-aliased edge
            if (alpha > 1.0f) alpha = 1.0f;
            if (alpha < 0.0f) alpha = 0.0f;

            int idx = (y * SCREEN_W + x) * 4;
            rgba[idx + 0] = rr;
            rgba[idx + 1] = gg;
            rgba[idx + 2] = bb;
            rgba[idx + 3] = (uint8_t)(alpha * 255.0f + 0.5f);
        }
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s.png", DIAL_SIM_OUTPUT_DIR, name);
    if (!stbi_write_png(path, SCREEN_W, SCREEN_H, 4, rgba, SCREEN_W * 4)) {
        fprintf(stderr, "FAILED to write %s\n", path);
        return;
    }

    // Cheap sanity check: sample a coarse grid over the whole circle and
    // count distinct raw pixel values seen, to flag a render that came out
    // suspiciously uniform (almost certainly a blank/broken screen).
    uint16_t seen[64];
    int n_seen = 0;
    for (int gy = 20; gy < SCREEN_H - 20; gy += 40) {
        for (int gx = 20; gx < SCREEN_W - 20; gx += 40) {
            uint16_t v = s_host_fb[gy * SCREEN_W + gx];
            bool known = false;
            for (int i = 0; i < n_seen; i++) if (seen[i] == v) { known = true; break; }
            if (!known && n_seen < (int)(sizeof(seen) / sizeof(seen[0]))) seen[n_seen++] = v;
        }
    }
    printf("wrote %-16s %s (%d distinct colors sampled)\n", name, path, n_seen);
    s_snapshots_written++;
}

/* ---- scenario baseline --------------------------------------------------- */

// Shared, realistic dual-zone state every scenario starts from; individual
// scenarios below only touch the fields their screen actually cares about.
static void apply_baseline(void)
{
    app_state_t *st = sim_state_ptr();

    st->phase = PH_READY;
    st->have_state = true;
    st->device_online = true;
    st->clock_valid = true;
    st->serial[0] = '\0';   // real hardware never sets one; About shows "--"

    // Undoes scenario_pad_unreachable()'s dial_state_set_pad_url() so a
    // scenario running after it (there's no reset between scenarios other
    // than what apply_baseline itself does) doesn't inherit a stuck
    // "unreachable.invalid:8080"/PH_DEGRADED into an unrelated screen's
    // About/Pad row -- sim_state_reset()'s own fresh-device default,
    // reapplied the same way it's seeded there.
    dial_state_set_pad_url(DIAL_PAD_DEFAULT_BASE_URL);   // also restores PH_READY
    // scenario_pad_discovery / scenario_pad_degraded_real leave a phase_err
    // and retry countdown behind; dial_state_set_pad_url() above only
    // resets the phase, so clear the text too or the next PH_DEGRADED
    // render would echo a stale reason.
    st->phase_err[0] = '\0';
    st->retry_in_s = 0;

    st->zone_present[ZONE_A] = true;
    st->zone_present[ZONE_B] = true;
    st->units_c = false;
    st->rel_mode = false;   // absolute by default; relative scenarios opt in
    st->rotation = 0;
    st->haptics_level = 1;   // HAPTIC_LEVEL_AUTO
    st->welcomed = true;
    st->side_picked = true;
    st->ui_zone = ZONE_A;
    st->away = false;

    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = 211;     // 21.1C -> 70F
    a->actual_c = 21.1f;  // at target: HOLDING

    zone_state_t *b = &st->zones[ZONE_B];
    b->on = true;
    b->temp_dc = 222;     // 22.2C -> 72F target
    b->actual_c = 20.0f;  // -> 68F current, still warming: HEATING

    st->ota.status = 0;   // OTA_IDLE
}

/* ---- scenarios ------------------------------------------------------------*/

static void scenario_welcome(void)
{
    apply_baseline();
    ui_router_go(SCR_WELCOME, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("welcome");
}

static void scenario_wifi_portal(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->phase = PH_WIFI_PORTAL;
    // Same format as the real construction site (dial_wifi.c's
    // "Bedknob-%02X%02X", MAC-derived) -- kept as a literal here since the
    // simulator never links the real dial_net component (no ESP-IDF Wi-Fi
    // driver on the host), same reasoning as stubs.c's dial_net_ap_ssid().
    snprintf(st->ap_ssid, sizeof(st->ap_ssid), "Bedknob-A1B2");
    ui_router_go(SCR_WIFI_PORTAL, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("wifi-portal");
}

static void scenario_netpick(void)
{
    apply_baseline();
    ui_router_go(SCR_NETPICK, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("netpick");
}

// Types "Sky4" onto the wheel (knob turns + a tap on the "Add" disc per
// character, the same commit path a finger on the real disc drives) so the
// screenshot shows a password mid-entry instead of the blank first-open state.
static void scenario_passkey(void)
{
    apply_baseline();
    ui_router_go(SCR_PASSKEY, (void *)(uintptr_t)0 /* "Home" */, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);

    static const int WHEEL_INDEX[] = { 44, 10, 24, 56 };  // 'S','k','y','4'
    int pos = 0;
    for (size_t i = 0; i < sizeof(WHEEL_INDEX) / sizeof(WHEEL_INDEX[0]); i++) {
        int target = WHEEL_INDEX[i];
        sim_knob(target - pos);
        pos = target;
        sim_tap(180, 240);   // the "Add" disc — commits the candidate glyph
    }
    pump_ms(200);
    snapshot("passkey");
}

static void scenario_sidepick(void)
{
    apply_baseline();
    ui_router_go(SCR_SIDEPICK, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("sidepick");
}

static void scenario_connecting(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->phase = PH_WIFI_CONNECTING;
    snprintf(st->wifi_join_ssid, sizeof(st->wifi_join_ssid), "Home");
    ui_router_go(SCR_CONNECTING, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("connecting");
}

static void scenario_dial(void)
{
    apply_baseline();   // zone B (left) is already ON/heating, 68 -> 72, by baseline
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_B, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("dial");
}

// The Home face with the ambient "Update available" indicator (owner
// reassessment, docs/SPEC-update-prompt.md) — unconditional on
// ota.status==OTA_AVAILABLE and not-night, no idle window/daily ceiling
// unlike the SCR_UPDATE_PROMPT sheet (see scenario_update_prompt). Also
// proves it doesn't collide with the M6 "Finalizing update..."
// pending_verify caption that shares this same slot: apply_baseline()
// leaves pending_verify false, so this is the OTA_AVAILABLE side of that
// shared label. Not exercising the tap-to-SCR_UPDATE affordance here — the
// label's own presence/position is what needs a permanent screenshot; the
// simulator's sim_tap harness is reserved for scr_passkey's pre-fill need.
static void scenario_dial_update(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_AVAILABLE;
    snprintf(st->ota.latest, sizeof(st->ota.latest), SIM_OTA_LATEST);
    st->generation++;
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_B, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("dial-update");
}

// The Home face in RELATIVE scale. Deliberately a POSITIVE, OFF-GRID setpoint:
// 300dc (30.0°C), which is level +3 (1.0°C per level, level 0 = 27.0°C,
// commit 018d8f6) — so the
// render proves the spliced '+' glyph draws AND that an off-grid device
// value shows as the nearest level. Water below the setpoint keeps the
// heating overlay + pill on screen, and the neutral notch is visible (the
// "LEVEL" suffix is gone since A1b — relative mode shows no unit label).
static void scenario_dial_relative(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->rel_mode = true;
    st->ui_zone = ZONE_A;
    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = 300;     // 30.0C -> level +3
    a->actual_c = 26.0f;  // -> 79F, below setpoint: still warming
    st->generation++;     // direct field-sets don't bump it; make on_state re-run
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("dial-relative");
}

// The Home face in °C (Settings -> Units), at 20.0°C: the widest absolute
// numeral the 88px font produces ("20.0", 191px -- only 1-heavy values are
// narrower) and the 2026-09-04 layout audit's §1a worst case for the unit
// label, which used to sit at a fixed screen slot the digits overprinted.
// Bounces through SCR_MENU first: the previous scenario ended on this same
// (SCR_DIAL, ZONE_A) pair and ui_router_go() no-ops on an identical one.
static void scenario_dial_celsius(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->units_c = true;
    st->ui_zone = ZONE_A;
    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = 200;     // 20.0C
    a->actual_c = 20.0f;  // at target: HOLDING
    st->generation++;
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("dial-celsius");
}

// The Home face in RELATIVE scale at the +15 rail (DIAL_REL_MAX_DC, 42.0°C):
// the widest relative numeral ("+15" -- three glyphs, the spliced '+' plus
// the wide '1'/'5'), the other §1a collision case for the old "LEVEL"
// suffix — since A1b relative mode shows no unit label at all, so this
// render proves the bare "+15" with nothing to its right.
// Same SCR_MENU bounce as scenario_dial_celsius, same reason.
static void scenario_dial_relative_max(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->rel_mode = true;
    st->ui_zone = ZONE_A;
    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = DIAL_REL_MAX_DC;   // 420 -> level +15
    a->actual_c = 26.0f;            // below setpoint: still warming
    st->generation++;
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
    snapshot("dial-relative-max");
}

// The night face (Number only) mid water-alternation (docs/SPEC-night-face.md
// §3a) in °C with a FRACTIONAL water reading: A1b drops a zero tenth from
// the shared °C renderer ("34", not "34.0") and this is the case that must
// keep its tenth — the water at 23.4 °C must read "23.4", in the accent, with
// the WATER word under it. Night comes from dial_palette_set_night() (the
// real firmware's night worker is what calls it); sim_state_reset() already
// ships night_face_min = true. Timing: create() leaves the alternation
// unlocked (s_last_interact_ms = 0 and the sim tick is well past
// ALT_KNOB_LOCK_MS by now), the timer is created on the on_state inside
// ui_router_go and first fires at +2000 ms into the water phase, which
// holds until +4000 — so a 3000 ms pump lands mid-phase. Same SCR_MENU
// bounce as scenario_dial_celsius, same reason; day palette restored after.
static void scenario_dial_night_water(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->units_c = true;
    st->ui_zone = ZONE_A;
    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = 340;     // 34.0C -> "34" on the setpoint phase
    a->actual_c = 23.4f;  // below setpoint: heating -> alternation runs; "23.4"
    st->generation++;
    dial_palette_set_night(true);
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(3000);
    snapshot("dial-night-water");
    dial_palette_set_night(false);
}

/* ---- absolute-mode rails + whole-degree grid (fix(dial), 2026-09-05) ----
 * The dial's absolute rails are 120..420 (main.c seeds them on connect —
 * the Somnus app's whole-degree scale, same as the relative rails). The
 * sim's baseline leaves temp_min_dc/temp_max_dc at -1 (fallback 100..450),
 * so these scenarios seed the connected values themselves and put them
 * back after, leaving every other dial render untouched. °C so the numeral
 * shows the tenth if one survives. ui_temp_dc is cleared too: sim_knob /
 * a drag on the dial post through dial_state_set_ui_temp, and on_state
 * prefers that over temp_dc, so a stale one from the previous scenario
 * would otherwise win. Same SCR_MENU bounce as scenario_dial_celsius. */
static void rails_setup(int temp_dc)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->units_c = true;
    st->temp_min_dc = 120;
    st->temp_max_dc = 420;
    st->ui_temp_dc[ZONE_A] = -1;
    st->ui_temp_dc[ZONE_B] = -1;
    st->ui_zone = ZONE_A;
    zone_state_t *a = &st->zones[ZONE_A];
    a->on = true;
    a->temp_dc = temp_dc;
    a->actual_c = 26.0f;   // below every setpoint used here: heating
    st->generation++;
    ui_router_go(SCR_DIAL, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(600);
}

static void rails_teardown(void)
{
    app_state_t *st = sim_state_ptr();
    st->temp_min_dc = -1;
    st->temp_max_dc = -1;
    st->ui_temp_dc[ZONE_A] = -1;
    st->ui_temp_dc[ZONE_B] = -1;
    st->generation++;
}

// Screen point on the ring at a setpoint, for the pointer indev. Mirrors
// scr_dial.c's position_handle / value_from_point mapping: 270° sweep from
// 135° (lower-left) clockwise, radius = ARC_R − arc width / 2 = 165 − 8 (the
// handle's ext_click_area of 14 px makes a few px of error irrelevant).
static void ring_point(int dc, int lo, int hi, lv_coord_t *x, lv_coord_t *y)
{
    float frac = (float)(dc - lo) / (float)(hi - lo);
    float ang  = (135.0f + frac * 270.0f) * 3.14159265f / 180.0f;
    *x = (lv_coord_t)(180 + lroundf(157.0f * cosf(ang)));
    *y = (lv_coord_t)(180 + lroundf(157.0f * sinf(ang)));
}

// From the 42.0 rail: one detent up is the range stop ("42" stays, the
// numeral nudges), then one detent down reads "41". Before the fix the
// rail was 423 and the first detent read "42.3".
static void scenario_rails_420(void)
{
    rails_setup(420);
    sim_knob(1);
    pump_until_idle(800);   // the range-stop nudge is an lv_anim
    snapshot("rails-420-up");
    sim_knob(-1);
    pump_until_idle(800);
    snapshot("rails-420-up-down");
    rails_teardown();
}

// A pad setpoint of 42.3 (set by an older build or the app) above the
// 42.0 rail: renders as "42.3" with the handle pinned at the rail end
// (position_handle clamps frac to 1, lv_arc clamps the value), and the
// first detent — up, against the rail — snaps it onto the grid: "42".
static void scenario_rails_423(void)
{
    rails_setup(423);
    snapshot("rails-423");
    sim_knob(1);
    pump_until_idle(800);
    snapshot("rails-423-up");
    rails_teardown();
}

// A handle drag that lands off-grid: press the handle at 30.0, drag it to
// the ring angle for 33.7 and hold — the live numeral follows the finger
// ("33.7"); release — the commit snaps to the nearest whole degree ("34").
static void scenario_rails_drag_337(void)
{
    rails_setup(300);
    lv_coord_t x0, y0, x1, y1;
    ring_point(300, 120, 420, &x0, &y0);
    ring_point(337, 120, 420, &x1, &y1);
    sim_drag_to(x0, y0, x1, y1);
    snapshot("rails-drag-337-live");
    sim_release();
    pump_until_idle(800);
    snapshot("rails-drag-337");
    rails_teardown();
}

// Also documents the M7 permanent "Update" row (replaces the M6 conditional
// "Install X.Y.Z" row — confirmation moved into SCR_UPDATE itself, this row
// is now pure navigation): sets the OTA status to available with a pending
// version before navigating, then knob-walks focus down onto the row itself
// (Back/Settings/Update/Wi-Fi/About — 1 detent past the "Settings" the list
// opens on) so menu.png actually shows the version badge it's meant to
// document, not just proves it doesn't crash off-frame.
static void scenario_menu(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_AVAILABLE;
    snprintf(st->ota.latest, sizeof(st->ota.latest), SIM_OTA_LATEST);
    st->generation++;
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(1);            // Settings -> Update (badge: SIM_OTA_LATEST)
    pump_ms(300);
    pump_until_idle(800);  // rotor snap is an lv_anim; land before the capture
    snapshot("menu");
}

// The Update submenu (M7), opened with an update pending — shows "Check for
// updates"' AVAILABLE state (tap-to-confirm prompt not yet armed) and the
// Beta builds toggle in its default Off state beneath it.
static void scenario_update(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_AVAILABLE;
    snprintf(st->ota.latest, sizeof(st->ota.latest), SIM_OTA_LATEST);
    st->generation++;
    ui_router_go(SCR_UPDATE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);

    // Knob-walked one row past the rotor's opening focus. scr_update.c's
    // create() builds Back(0) / Check for updates(1) / Installed(2) / Skip
    // <version>(3, present only while OTA_AVAILABLE) / Auto-update(4) / Beta
    // builds(5) and settles on row 1, so +1 detent lands focus on Installed
    // with Check for updates in the band above and Skip / Auto-update below
    // — this one shot still documents the OTA-available status AND proves
    // docs/SPEC-update-prompt.md's rows actually render (rather than adding
    // a second persisted screenshot just for them). Auto-update On (its
    // value renders the derived post-wake window) and the version left
    // unskipped so both show a real, non-blank value rather than their
    // empty defaults.
    st->ota_auto = 1;
    st->generation++;
    sim_knob(1);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("update");
}

// The update-prompt sheet (docs/SPEC-update-prompt.md): the real firmware
// only ever reaches this screen via nav_policy (the worker's idle-loop gate
// raising ota_prompt_due while showing SCR_DIAL) — the simulator doesn't run
// a nav policy at all, so this navigates there directly, same as every other
// scenario, with an update pending so the sheet's copy has a real version to
// show.
static void scenario_update_prompt(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_AVAILABLE;
    snprintf(st->ota.latest, sizeof(st->ota.latest), SIM_OTA_LATEST);
    st->generation++;
    ui_router_go(SCR_UPDATE_PROMPT, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    // Same idle-out-the-slide-anim gate scenario_quick uses for its sheet.
    pump_ms(400);
    pump_until_idle(1000);
    snapshot("update-prompt");
}

// The Update submenu with the last check FAILED: "Check for updates" grows
// its third line (the error, LONG_DOT, warning tint) under label + value --
// the tallest state that row has, and the one the 2026-09-04 layout audit
// (§5a) measured against the 76px row. No knob-walk: the rotor already
// opens on that row (scr_update.c's dial_list_settle(s_list, 1); row order
// Back(0)/Check for updates(1)/Installed(2)/Auto-update(3)/Beta builds(4)),
// so the three lines render in the focused, unzoomed slot.
static void scenario_update_failed(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_FAILED;
    snprintf(st->ota.err, sizeof(st->ota.err), "check failed (HTTP -1)");
    st->generation++;
    ui_router_go(SCR_UPDATE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("update-failed");
}

static void scenario_settings(void)
{
    apply_baseline();
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("settings");
}

// Knob-walked down to the "Pad Address"/"Bed Mode" pair. Row order as of
// 2026-09-02 (Adjustment mode hidden; Night mode / Night face / Screen
// timeout added -- scr_settings.c's create(); Night face is present because
// sim_state_reset() ships night_on = true): Back(0)/Brightness(1)/Night
// mode(2)/Night face(3)/Screen timeout(4)/Standby face(5)/Scale(6)/Units(7)/
// Haptics(8)/Rotation(9)/Timezone(10)/Pad Address(11)/Bed Mode(12)/Factory
// reset(13) — Standby face added 2026-09-05 (docs/SPEC-standby-face.md),
// which is why every detent count below grew by one that day.
// The rotor opens on Brightness (index 1, dial_list_settle in create()), so
// +10 detents lands focus on Pad Address with Timezone/Bed Mode as its
// zoomed/faded neighbors, putting both rows in frame at once. (The earlier
// +7 dated from the pre-2026-09-02 list and had drifted onto Rotation --
// docs/REPORT-screen-layout-audit.md's "Stale screenshots".)
static void scenario_settings_pad(void)
{
    apply_baseline();
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(10);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("settings-pad");
}

// Settings' Timezone row showing a RAW IANA name rather than a curated
// label -- the state a Wi-Fi-portal-applied zone outside scr_timezone.c's
// 11 rows leaves behind (docs/SPEC-timezone-source.md "Displaying the
// current value"; docs/REPORT-screen-layout-audit.md §2b). The only
// simulator hook that fakes a persisted zone is sim_set_fake_iana_tz()
// (stubs.c); dial_time_valid() stays false, so the Night mode row's §7
// annotation reads "no clock" here instead of "set timezone" -- both are
// real states and both have to fit. The zone has to be one NOT in
// dial_state.h's DIAL_TZ_IANA[] or the row shows the curated label instead
// (the audit's "America/Los_Angeles" example is in the list, and renders as
// "Pacific"); America/Mexico_City is a real portal-detectable zone of the
// same 19-character width, so the audit's §2b overlap numbers still apply.
// Same row indices as scenario_settings_pad: +8 from Brightness(1) is
// Timezone(9). Bounces via SCR_MENU because the previous scenario ended on
// (SCR_SETTINGS, NULL).
static void scenario_settings_timezone_raw(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    sim_set_fake_iana_tz("America/Mexico_City");
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(9);   // Timezone is row 10 since the Standby face row (see scenario_settings_pad)
    pump_ms(300);
    pump_until_idle(800);
    snapshot("settings-timezone-raw");
    sim_set_fake_iana_tz(NULL);   // back to "Not set" for every scenario after this one
}

// The curated timezone picker (scr_timezone.c), opened from Settings (arg 0
// -- see that file's s_origin packing). Plain navigate-and-snapshot: the
// list had no checked-in render before the 2026-09-04 layout audit.
static void scenario_timezone(void)
{
    apply_baseline();
    ui_router_go(SCR_TIMEZONE, (void *)(uintptr_t)0 /* from Settings */, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("timezone");
}

// The night-window picker (scr_night_mode.c) with night on (sim_state_reset's
// default) and the clock invalid (stubs.c's dial_time_valid() is false), so
// the §7 note under the title renders -- the audit's §12 seam case.
static void scenario_night_mode(void)
{
    apply_baseline();
    ui_router_go(SCR_NIGHT_MODE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("night-mode");
}

// The night-face picker (scr_night_face.c), Number only / Full. Plain
// navigate-and-snapshot, same reason as scenario_timezone.
static void scenario_night_face(void)
{
    apply_baseline();
    ui_router_go(SCR_NIGHT_FACE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("night-face");
}

// The standby-face picker (scr_standby_face.c), Temperature / Clock —
// docs/SPEC-standby-face.md §4. Checkmark on Temperature (the default).
// No `standby-temperature` scenario: the sim's dial_power stub always
// reports DPWR_ACTIVE and there is no nav_policy here (main.c is not
// linked), so the STANDBY-tier routing is hardware-only.
static void scenario_standby_face(void)
{
    apply_baseline();
    ui_router_go(SCR_STANDBY_FACE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("standby-face");
}

// Settings knob-walked onto the new "Standby face" row (index 5, directly
// under Screen timeout — see scenario_settings_pad's row list): +4 from
// Brightness. scenario_settings' own frame opens on Brightness with the
// list's top four rows in view, so the row is off-frame there by design;
// this is the render that shows it, value "Temperature".
static void scenario_settings_standby_face(void)
{
    apply_baseline();
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(4);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("settings-standby-face");
}

// The Pad Address text-entry screen (scr_pad_address.c), opened straight
// from Settings' row — pre-filled with sim_state_reset()'s
// DIAL_PAD_DEFAULT_BASE_URL, same as a real fresh device, so this documents
// what editing an EXISTING value looks like (the common case), not a blank
// field.
static void scenario_pad_address(void)
{
    apply_baseline();
    ui_router_go(SCR_PAD_ADDRESS, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("pad-address");
}

// The PH_DEGRADED fallback after a pad-address change that doesn't resolve —
// exercises the exact path a real IP typo or a moved pad would take
// (CMD_PAD_SETTINGS_CHANGED's dial_somnus_connect() failing, main.c setting
// PH_DEGRADED with the real error string), except there is no real
// dial_somnus/network in this simulator to actually fail against, so
// sim_state.c's dial_state_set_pad_url() fakes the same outcome for any URL
// containing "unreachable" (see that function's own comment). The simulator
// has no nav policy (every scenario navigates directly, same as every other
// one here), so this goes straight to SCR_CONNECTING/SCR_ERROR's shared
// rendering, which is what a real dial would land on too once nav_policy
// saw PH_DEGRADED with no menu/settings screen deliberately open.
static void scenario_pad_unreachable(void)
{
    apply_baseline();
    dial_state_set_pad_url("http://unreachable.invalid:8080");
    ui_router_go(SCR_CONNECTING, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("pad-unreachable");
}

// The subnet-scan progress screen (scr_pad_discovery.c), mid pass 2: the
// phase is PH_PAD_DISCOVERY and phase_err carries dial_pad_discovery.c's
// "<headline>\n<checked>/<total>" shape, with the longer pass-2 headline
// that wraps to two lines (docs/REPORT-screen-layout-audit.md §14). Nothing
// opens this screen on purpose in the firmware (nav_policy does, on the
// phase); the simulator has no nav policy, so it navigates directly.
static void scenario_pad_discovery(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->phase = PH_PAD_DISCOVERY;
    snprintf(st->phase_err, sizeof(st->phase_err),
             "Still looking (checking more slowly)...\n137/254");
    st->generation++;
    ui_router_go(SCR_PAD_DISCOVERY, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("pad-discovery");
}

// PH_DEGRADED with a REALISTIC reason, not scenario_pad_unreachable's short
// "(simulated)" one: the subtitle then wraps to four lines (the echoed
// dial_somnus error, "Retrying in 27s", "Swipe left for menu") -- the
// tallest case scr_connecting.c's block has to centre (audit §13). Same
// "unreachable" URL trick to get the phase, then the reason and countdown
// overwritten with what main.c's supervisor would actually publish. The
// address is the pad API spec's example, not any real network.
static void scenario_pad_degraded_real(void)
{
    apply_baseline();
    dial_state_set_pad_url("http://unreachable.invalid:8080");
    app_state_t *st = sim_state_ptr();
    snprintf(st->phase_err, sizeof(st->phase_err),
             "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)");
    st->retry_in_s = 27;
    st->generation++;
    ui_router_go(SCR_CONNECTING, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("pad-degraded-real");
}

// The Adjustment mode choice screen, reached from Settings' "Adjustment
// mode" row: apply_baseline() doesn't touch sched_follow, and
// sim_state_reset() left it at its fresh-device default (true = Schedule),
// so this renders with no extra state poking — Schedule selected, its
// description visible below the two options.
static void scenario_adjust_mode(void)
{
    apply_baseline();
    ui_router_go(SCR_ADJUST_MODE, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("adjust-mode");
}

// The Brightness submenu (M7, collapses Settings' old separate Day/Night
// rows): shows the last-committed percents read straight off app_state_t
// (and "Off" for a Night (clock) at 0), same contract the rows had before
// the collapse.
static void scenario_brightness_menu(void)
{
    apply_baseline();
    ui_router_go(SCR_BRIGHTNESS_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("brightness-menu");
}

// The full-screen SCR_BRIGHTNESS picker (replaces the old inline settings-row
// edit — see scr_brightness.c) opened on the Night (in use) row, then knob-adjusted
// so the render shows a deliberately chosen value (and the drag handle parked
// at it) rather than the untouched opening state — same idiom scenario_boost
// uses for its own picker. Turned UP, not down: the picker opens on the stored
// pref, and Night's shipped default is 0%, so any downward turn just clamps at
// the floor and documents nothing. sim_knob(+3) drains as one on_knob(+3)
// batch: |batch|>=3 accelerates to BRI_ACCEL_3_MULT (6%/detent), so
// 0% -> 0 + 3*6 = 18%.
static void scenario_settings_brightness(void)
{
    apply_baseline();
    ui_router_go(SCR_BRIGHTNESS, (void *)(uintptr_t)1 /* night */, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(3);   // 0% -> 18% (accelerated: 3 detents * 6%/detent)
    pump_ms(300);
    snapshot("brightness");
}

// The same picker opened on the Night (clock) row (packed arg 2) — mainly
// to verify the "NIGHT (CLOCK)" caption clears the arc's chord at the
// same y-offset tuned for the old "NIGHT BRIGHTNESS" (see scr_brightness.c's
// create() comment), and that the live preview visibly differs from the Night row
// above (it previews the NIGHT table's STANDBY duty, deliberately very dim).
// sim_knob(+3) drains as one on_knob(+3) batch: |batch|>=3 accelerates to
// BRI_ACCEL_3_MULT (6%/detent), so the 20% default -> 20 + 3*6 = 38%.
static void scenario_settings_brightness_clock(void)
{
    apply_baseline();
    ui_router_go(SCR_BRIGHTNESS, (void *)(uintptr_t)2 /* night clock */, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(3);   // 20% -> 38% (accelerated: 3 detents * 6%/detent)
    pump_ms(300);
    snapshot("brightness-clock");
}

// The clock picker driven to 0 — the one value on this row that is a state,
// not a level: the standby clock goes genuinely dark
// (dial_power_night_clock_duty(0) == 0), and the unit slot names it, "Off"
// replacing "%" — the 88px numeral font is digits-only, so the word rides
// the small label (scr_brightness.c's render_numeral).
static void scenario_settings_brightness_clock_off(void)
{
    apply_baseline();
    // The previous scenario ends with this same picker open on this same row,
    // and ui_router_go no-ops on an identical id+arg pair (ui_router.c) — so
    // step out to the menu first to force a real rebuild, then re-seed the
    // pref that stepping out just committed (destroy() persisted its 38%).
    ui_router_go(SCR_BRIGHTNESS_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    sim_state_ptr()->bri_night_clock_pct = 20;
    ui_router_go(SCR_BRIGHTNESS, (void *)(uintptr_t)2 /* night clock */, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(-6);   // one -6 batch: 6%/detent -> -36, clamping at the 0 rail -> "Off"
    pump_ms(300);
    snapshot("brightness-clock-off");
}

static void scenario_wifi_info(void)
{
    apply_baseline();
    ui_router_go(SCR_WIFI, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("wifi-info");
}

// SCR_WIFI's CONFIRM MODE (the "Change network" tap): body copy, Continue,
// Cancel. Knob-walked from the opening focus (Network, index 1) down three
// rows to "Change network" (index 4: Back/Network/IP/Signal/Change
// network), then tapped at the focused row's centre through the same
// pointer indev scr_passkey's pre-fill uses -- the real
// row_change_network_cb path, not a reach into the screen's statics. Had
// no checked-in render before the 2026-09-04 audit (§6). Bounces via
// SCR_MENU: the previous scenario ended on this same (SCR_WIFI, NULL).
static void scenario_wifi_confirm(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    ui_router_go(SCR_WIFI, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(3);
    pump_ms(300);
    pump_until_idle(800);
    sim_tap(180, 180);   // the focused "Change network" row
    pump_ms(300);
    pump_until_idle(800);
    snapshot("wifi-confirm");
}

static void scenario_about(void)
{
    apply_baseline();
    ui_router_go(SCR_ABOUT, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("about");
}

// Evidence for the About screen's Wi-Fi row overlap fix (found on hardware
// bench test, docs/REPORT-battery-pct-about-redesign.md's addendum): both
// scenarios knob-walk from the rotor's opening focus (Firmware, index 1 --
// scr_about.c's own dial_list_settle(s_list, 1)) down two rows to Wi-Fi
// (index 3: Back/Firmware/IDF/Wi-Fi/Pad/Battery), so the row under test is
// centered in frame instead of the opening screenshot's partially-cropped one.
//
// Worst case: a full 32-character SSID (the real maximum) at the longest
// signal_word()/rssi combination ("Weak (-99 dBm)") -- must clip via
// LV_LABEL_LONG_DOT, never collide with the "Wi-Fi" label.
//
// ui_router_go() no-ops on an identical (id, arg) pair (ui_router.c) --
// scenario_about() just navigated to this same (SCR_ABOUT, NULL), so without
// bouncing through another screen first, this scenario's own navigation
// below would be swallowed and never re-render with the new fake AP.
static void scenario_about_wifi_worst(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    sim_set_fake_ap("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef", -99);   // 32 chars, "Weak"
    ui_router_go(SCR_ABOUT, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(2);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("about-wifi-worst");
}

// Real case: the exact SSID/RSSI from the hardware bench report that
// triggered this fix ("TT5CiDPi2 - Weak (-73 dBm)", 26 chars) -- must show
// in full, un-truncated, since it already fits comfortably at the chosen
// width. Same SCR_MENU bounce as above, for the same reason.
static void scenario_about_wifi_real(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    sim_set_fake_ap("TT5CiDPi2", -73);
    ui_router_go(SCR_ABOUT, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(2);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("about-wifi-real");
    sim_set_fake_ap(NULL, 0);   // back to FAKE_SCAN[0] for every scenario after this one
}

// Evidence for the About screen's block redesign (docs/REPORT-battery-pct-
// about-redesign.md's "block redesign" addendum): the Battery row is the
// other 3-line (title/value/detail) row besides Wi-Fi, so it gets the same
// tightest-fit scrutiny. Two states, two scenarios -- on-battery (value =
// percentage, detail = voltage) and on-USB (value = "On USB", detail =
// voltage) -- knob-walked from the rotor's opening focus (Firmware, index 1)
// down four rows to Battery (index 5: Back/Firmware/IDF/Wi-Fi/Pad/Battery).
static void scenario_about_battery_pct(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->power_src = PWR_BATTERY;
    st->power_pct = 78;
    st->power_mv = 4050;
    st->generation++;
    ui_router_go(SCR_ABOUT, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(4);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("about-battery-pct");
}

static void scenario_about_battery_usb(void)
{
    apply_baseline();
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(100);
    app_state_t *st = sim_state_ptr();
    st->power_src = PWR_PLUGGED;
    st->power_mv = 4610;
    st->generation++;
    ui_router_go(SCR_ABOUT, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    sim_knob(4);
    pump_ms(300);
    pump_until_idle(800);
    snapshot("about-battery-usb");
}

// Full-screen OTA install takeover (M6 UX hardening) — nav_policy forces this
// in the real firmware the moment ota.status becomes OTA_DOWNLOADING, but the
// simulator doesn't run a nav policy at all (every scenario navigates
// directly), so this just sets the status/progress a real download-in-
// -progress commit would carry and goes straight there. ~62% so the ring
// reads as mid-download, not just-started or about-to-finish.
static void scenario_updating(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = 3;   // OTA_DOWNLOADING (dial_ota.h's dial_ota_status_t)
    st->ota.progress_pct = 62;
    ui_router_go(SCR_UPDATING, NULL, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("updating");
}

static void scenario_standby(void)
{
    apply_baseline();
    ui_router_go(SCR_STANDBY, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("standby");
}

// The clock face with the ambient "Update available" indicator (see
// scenario_dial_update's own comment — same owner reassessment, same
// unconditional status+night gate, no idle window/daily ceiling). Confirms
// the notice's slot on this face (otherwise-empty gap below the clock/date,
// no page dots here to collide with) renders correctly. Like scenario_standby
// itself this bakes in the live wall clock — not meant to be diffed against
// a checked-in reference; see this file's own header / the caller's note on
// why standby*.png never stay checked out after a run.
static void scenario_standby_update(void)
{
    apply_baseline();
    app_state_t *st = sim_state_ptr();
    st->ota.status = OTA_AVAILABLE;
    snprintf(st->ota.latest, sizeof(st->ota.latest), SIM_OTA_LATEST);
    st->generation++;
    ui_router_go(SCR_STANDBY, (void *)(uintptr_t)ZONE_A, LV_SCR_LOAD_ANIM_NONE);
    pump_ms(300);
    snapshot("standby-update");
}

/* ---- entry point -----------------------------------------------------------*/

int main(void)
{
    ensure_dir(DIAL_SIM_OUTPUT_DIR);

    lv_init();

    lv_disp_draw_buf_init(&s_draw_buf, s_lv_buf, NULL, SCREEN_W * SCREEN_H);
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &s_draw_buf;
    disp_drv.flush_cb = flush_cb;
    disp_drv.hor_res = SCREEN_W;
    disp_drv.ver_res = SCREEN_H;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read_cb;
    lv_indev_drv_register(&indev_drv);

    sim_state_reset();
    ui_screens_register_all();
    // Sets up the 50ms dispatcher (drains ui_router_knob_input); the initial
    // screen doesn't matter since every scenario below navigates explicitly.
    ui_router_start(SCR_CONNECTING, NULL);

    scenario_welcome();
    scenario_wifi_portal();
    scenario_netpick();
    scenario_passkey();
    scenario_sidepick();
    scenario_connecting();
    scenario_dial();
    scenario_dial_update();
    scenario_dial_relative();
    scenario_dial_celsius();
    scenario_dial_relative_max();
    scenario_dial_night_water();
    scenario_rails_420();
    scenario_rails_423();
    scenario_rails_drag_337();
    scenario_menu();
    scenario_update();
    scenario_update_prompt();
    scenario_update_failed();
    scenario_settings();
    scenario_settings_pad();
    scenario_settings_timezone_raw();
    scenario_timezone();
    scenario_night_mode();
    scenario_night_face();
    scenario_standby_face();
    scenario_settings_standby_face();
    scenario_pad_address();
    scenario_pad_unreachable();
    scenario_pad_discovery();
    scenario_pad_degraded_real();
    scenario_adjust_mode();
    scenario_brightness_menu();
    scenario_settings_brightness();
    scenario_settings_brightness_clock();
    scenario_settings_brightness_clock_off();
    scenario_wifi_info();
    scenario_wifi_confirm();
    scenario_about();
    scenario_about_wifi_worst();
    scenario_about_wifi_real();
    scenario_about_battery_pct();
    scenario_about_battery_usb();
    scenario_updating();
    scenario_standby();
    scenario_standby_update();

    printf("done: %d screens rendered to %s\n", s_snapshots_written, DIAL_SIM_OUTPUT_DIR);
    return 0;
}
