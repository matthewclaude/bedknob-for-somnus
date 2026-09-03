#pragma once
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "ui_router.h"
#include "dial_state.h"
#include "dial_palette.h"
#include "dial_time.h"

// Screen vtables defined one per file, gathered by ui_screens_register_all().
extern const ui_screen_t scr_connecting;
extern const ui_screen_t scr_pad_discovery;
extern const ui_screen_t scr_wifi_portal;
extern const ui_screen_t scr_netpick;
extern const ui_screen_t scr_passkey;
extern const ui_screen_t scr_dial;
extern const ui_screen_t scr_menu;
extern const ui_screen_t scr_standby;
extern const ui_screen_t scr_welcome;
extern const ui_screen_t scr_sidepick;
extern const ui_screen_t scr_settings;
extern const ui_screen_t scr_timezone;
extern const ui_screen_t scr_pad_address;
extern const ui_screen_t scr_adjust_mode;
extern const ui_screen_t scr_brightness_menu;
extern const ui_screen_t scr_brightness;
extern const ui_screen_t scr_night_mode;
extern const ui_screen_t scr_night_face;
extern const ui_screen_t scr_wifi;
extern const ui_screen_t scr_about;
extern const ui_screen_t scr_update;
extern const ui_screen_t scr_updating;
extern const ui_screen_t scr_update_prompt;

/*
 * Shared state->visual classification (design-spec.md §2's grammar table).
 * Both scr_dial (full glyph+word+accent) and scr_standby (accent only, for
 * the presence dots) need "which token does this zone's state map to", so it
 * lives here once instead of drifting between two copies.
 */
typedef enum { ZK_OFFLINE, ZK_STANDBY, ZK_HEATING, ZK_COOLING, ZK_HOLDING } zone_kind_t;

// Somnus's local API reports only a setpoint and a measured reading, not a
// thermal_state string (that was Orion's get_device_state) -- so heating/
// cooling/holding is derived here from the same two numbers
// dial_state_predict_thermal used to use, with the same 0.5C deadband.
static inline zone_kind_t dial_zone_kind(const zone_state_t *z, bool device_online)
{
    if (!device_online)               return ZK_OFFLINE;
    if (!z->on)                       return ZK_STANDBY;   // off IS standby, regardless of stale telemetry
    if (z->actual_c < 0)              return ZK_HOLDING;   // nothing measured yet to compare against
    float delta = (z->temp_dc / 10.0f) - z->actual_c;
    if (delta > 0.5f)  return ZK_HEATING;
    if (delta < -0.5f) return ZK_COOLING;
    return ZK_HOLDING;
}

/*
 * Every button on the dial is built here.
 *
 * LVGL's default theme hangs a grey drop shadow off every lv_btn, offset 4px
 * DOWNWARD (lv_theme_default.c styles->btn: shadow_width 3, opa 50%, ofs_y 4).
 * It's a material-style elevation cue that belongs to nothing else in this
 * design language — on the dial's near-black ground it reads as a smudge under
 * the power disc rather than as depth. Nothing here is meant to float above the
 * face; the chassis is flat and state is carried by ring color, not elevation.
 *
 * Zeroing it per-button is how it got missed twice, so the constructor owns it.
 */
static inline lv_obj_t *dial_btn_create(lv_obj_t *parent)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    return btn;
}

/*
 * Page-dot row, shared by the dial faces and the menu face so the two can't
 * drift out of agreement about the chain.
 *
 * Face order is Dial(B) - Dial(A) - Menu: zone_b is the LEFT side of the bed
 * and zone_a the RIGHT, so walking the chain left-to-right walks the bed
 * left-to-right. A single-zone topper has no partner face at all — its absent
 * side's dot is dropped and the remaining pair re-centered, rather than
 * leaving a dot for a face the swipe can never reach.
 *
 * Callers own the fill colors (which dot is "current" differs per screen).
 */
static inline void dial_dots_layout(const app_state_t *st, lv_obj_t *dot_b,
                                    lv_obj_t *dot_a, lv_obj_t *dot_menu)
{
    const lv_coord_t y = 340 - 180;   // same rim band on every face
    if (dial_state_is_dual(st)) {
        lv_obj_clear_flag(dot_b, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(dot_a, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(dot_b,    LV_ALIGN_CENTER, 164 - 180, y);
        lv_obj_align(dot_a,    LV_ALIGN_CENTER, 180 - 180, y);
        lv_obj_align(dot_menu, LV_ALIGN_CENTER, 196 - 180, y);
    } else {
        zone_idx_t p = dial_state_primary_zone(st);
        lv_obj_t *keep = (p == ZONE_A) ? dot_a : dot_b;
        lv_obj_t *drop = (p == ZONE_A) ? dot_b : dot_a;
        lv_obj_add_flag(drop, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(keep, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(keep,     LV_ALIGN_CENTER, 172 - 180, y);
        lv_obj_align(dot_menu, LV_ALIGN_CENTER, 188 - 180, y);
    }
}

static inline lv_color_t dial_zone_accent(zone_kind_t k, const dial_palette_t *pal)
{
    switch (k) {
    case ZK_OFFLINE: return pal->warning;
    case ZK_STANDBY: return pal->neutral_standby;
    case ZK_HEATING: return pal->accent_heat;
    case ZK_COOLING: return pal->accent_cool;
    default:         return pal->neutral_holding;
    }
}

/*
 * Night mode display (docs/SPEC-night-window.md §5/§7) — shared between
 * scr_settings.c's Settings-row value, scr_night_mode.c's picker subtitle,
 * and scr_update.c's derived "After wake" window, so the three can't drift
 * out of agreement about what a stored (night_on, night_start_min,
 * night_end_min) triple, or a derived window pair, reads as.
 */

// One clock time (minutes from local midnight) as 12-hour hour/minute/pm.
static inline void dial_fmt12(int min, int *h12, int *mm, bool *pm)
{
    int h24 = (min / 60) % 24;
    *mm = min % 60;
    *pm = h24 >= 12;
    int h = h24 % 12;
    *h12 = (h == 0) ? 12 : h;
}

// §5's fallback rendering for a stored pair that matches neither preset —
// "h:mm am - h:mm pm", both sides spelled out. ASCII hyphen, not an en
// dash: the compiled Montserrat fonts this UI renders with have no dash
// glyph at all (a box, not a hyphen), so every string here stays plain
// ASCII. Unreachable with today's two presets (dial_state's clamp-on-read
// always resolves a bad pair to exactly preset 0), kept for the
// custom-editor follow-up (§8) and as the belt-and-braces default a
// corrupt read can never actually reach.
static inline void dial_night_range_str(uint16_t start, uint16_t end, char *buf, size_t sz)
{
    int sh, sm, eh, em; bool spm, epm;
    dial_fmt12(start, &sh, &sm, &spm);
    dial_fmt12(end,   &eh, &em, &epm);
    snprintf(buf, sz, "%d:%02d %s - %d:%02d %s",
             sh, sm, spm ? "pm" : "am", eh, em, epm ? "pm" : "am");
}

// §6's Update-screen rendering for a derived (or fixed-fallback) window —
// one shared meridiem suffix when both ends fall on the same side of noon
// (today's presets always do: "9:00-11:00 am"), the full h:mm-am/pm-each
// form otherwise (a future custom editor could derive a window spanning
// noon or midnight). ASCII hyphen throughout -- see dial_night_range_str's
// comment on why.
static inline void dial_wake_window_str(int start, int end, char *buf, size_t sz)
{
    int sh, sm, eh, em; bool spm, epm;
    dial_fmt12(start, &sh, &sm, &spm);
    dial_fmt12(end,   &eh, &em, &epm);
    if (spm == epm)
        snprintf(buf, sz, "%d:%02d-%d:%02d %s", sh, sm, eh, em, spm ? "pm" : "am");
    else
        snprintf(buf, sz, "%d:%02d %s - %d:%02d %s",
                 sh, sm, spm ? "pm" : "am", eh, em, epm ? "pm" : "am");
}

// §7's "the row must not lie" annotation reason — "" while the clock the
// setting depends on is valid (dial_time_valid(), the SAME condition the
// consumer (dial_night_active, via dial_time_now()) depends on — not a
// proxy for it), else "set timezone" (no zone ever persisted) or "no clock"
// (a zone IS persisted, SNTP hasn't synced -- Wi-Fi up, internet down, the
// state a bedside device sits in during an outage; there is no user action
// to name there). Callers place a plain ASCII hyphen themselves (see
// dial_night_range_str's comment) — the picker (scr_night_mode.c) shows the
// reason bare, with no leading punctuation at all.
static inline const char *dial_night_clock_reason(void)
{
    if (dial_time_valid()) return "";
    char zone[48];
    return dial_time_get_iana_tz(zone, sizeof zone) ? "no clock" : "set timezone";
}

// Settings-row / picker-subtitle base value: "Off", the matching preset
// label, or (see dial_night_range_str) the raw pair — with §7's annotation
// appended when the clock isn't valid.
static inline void dial_night_row_value(const app_state_t *st, char *buf, size_t sz)
{
    if (!st->night_on) {
        strlcpy(buf, "Off", sz);
    } else {
        const char *label = NULL;
        for (int i = 0; i < DIAL_NIGHT_PRESETS_N; i++)
            if (st->night_start_min == DIAL_NIGHT_PRESET_START[i] &&
                st->night_end_min   == DIAL_NIGHT_PRESET_END[i]) { label = DIAL_NIGHT_PRESET_LABEL[i]; break; }
        if (label) strlcpy(buf, label, sz);
        else       dial_night_range_str(st->night_start_min, st->night_end_min, buf, sz);
    }
    const char *reason = dial_night_clock_reason();
    if (reason[0]) {
        strlcat(buf, " - ", sz);
        strlcat(buf, reason, sz);
    }
}
