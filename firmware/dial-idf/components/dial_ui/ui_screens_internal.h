#pragma once
#include <stdio.h>
#include <string.h>
#include "lvgl.h"
#include "ui_router.h"
#include "dial_state.h"
#include "dial_palette.h"
#include "dial_time.h"
#include "dial_power.h"   // DIAL_BATTERY_PCT_LOW (§11.3's badge breathe threshold)

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
extern const ui_screen_t scr_standby_face;
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
 * Battery / plug-in badge (docs/SPEC-power-sensing.md §10.4, upgraded to a
 * custom-drawn fill by §11.3), shared by scr_dial.c and scr_standby.c so the
 * two can't drift out of agreement about which visual shows on which
 * transition. One slot, never more than one thing showing:
 *   power_src == BATTERY  -> the drawn assembly (`wrap`: outline `body` +
 *     `nub` + a `fill` bar whose WIDTH tracks power_pct continuously, not
 *     LVGL's five-bucket LV_SYMBOL_BATTERY_EMPTY rounding this replaces --
 *     same "percent of the slot's own width" idea scr_dial.c's `s_level` arc
 *     already applies to a partial fill, just measured in px instead of
 *     degrees since a battery reads as a bar, not a ring). Persistent.
 *   BATTERY -> PLUGGED (a real transition, not a screen re-create) -> a 3s
 *     LV_SYMBOL_CHARGE flash on `label` (untouched by §11.3), then hidden.
 *   otherwise (PLUGGED at steady state, UNKNOWN) -> everything hidden.
 * No confirmation on UNPLUG beyond the fill assembly itself appearing.
 *
 * At or below DIAL_BATTERY_PCT_LOW the whole assembly breathes pal->warning
 * (the existing "faults only, never thermal" token -- this qualifies, and
 * it's already night-safe/RGB565-quantized, so no new color is invented).
 * `wrap` is a plain lv_obj containing `body`/`nub`/`fill` as children with
 * NO drawing of its own (bg_opa TRANSP, border_width 0) purely so its own
 * opa can dim/breathe the whole trio in one animation -- LVGL8 composites a
 * child-bearing object to an offscreen "simple layer" whenever its own opa
 * is below COVER (lv_obj_style.c's layer-type decision), which is exactly
 * the every-part-fades-together behavior scr_dial.c's chevron pulse gets
 * from a single label; this is the same trick applied to three objects
 * instead of one.
 *
 * Callers own create (position/palette) and destroy (delete the timer/anim);
 * this owns the symbol/fill/breathe/visibility decision, driven off the
 * caller's own "last power_src this screen instance has rendered" — same
 * edge-triggered idiom scr_dial.c's s_stale_shown already uses, folded into
 * power_glyph_apply() so both screens share the transition logic exactly
 * rather than each re-deriving "was that a real transition" from scratch.
 */
typedef struct {
    lv_obj_t   *label;          // CHARGE flash text only (§11.3 moves the
                                 // persistent BATTERY visual to `wrap` below)
    lv_obj_t   *wrap;           // fill-assembly container; its own opa is the
                                 // one thing that dims/breathes body+nub+fill
    lv_obj_t   *body;           // battery outline
    lv_obj_t   *nub;            // battery terminal nub
    lv_obj_t   *fill;           // charge level; width tracks power_pct
    lv_timer_t *charge_timer;   // NULL except during the 3s CHARGE flash
    bool        breathing;      // low-battery breathe anim currently running
    bool        breathe_night;  // which period/range it's running at, so a
                                 // day/night flip mid-breathe restarts it
} power_glyph_t;

#define POWER_GLYPH_CHARGE_MS   3000
// Body is 20px wide, not the 16px of the LV_SYMBOL_BATTERY_EMPTY glyph it
// replaced: with a 2px inset each side the fill has 16px to cover 0..100%,
// i.e. one pixel per ~6%. At 16px the usable span was 12px and, with the
// old 2px floor and floor-rounding, everything from 0% to 24% drew as the
// same 2px bar -- the 15% low threshold sat inside a band the bar could
// not move in, leaving the red breathe as the only signal. Still ~1.8mm
// wide at this panel's 281ppi, well inside the caption above the arc.
#define POWER_GLYPH_BODY_W        20   // outline, px
#define POWER_GLYPH_BODY_H         9
#define POWER_GLYPH_NUB_W          2
#define POWER_GLYPH_NUB_H          4
#define POWER_GLYPH_FILL_INSET     2   // border + gap, each side of `fill`
#define POWER_GLYPH_FILL_MIN_W     1   // visible floor even at 0% (§11.3)

static inline void power_glyph_charge_timer_cb(lv_timer_t *t)
{
    power_glyph_t *pg = (power_glyph_t *)t->user_data;
    lv_obj_add_flag(pg->label, LV_OBJ_FLAG_HIDDEN);
    pg->charge_timer = NULL;
}

static inline void power_glyph_set_wrap_opa(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

// Low-battery breathe (§11.3): same primitive as scr_dial.c's chevron_start
// (lv_anim_path_ease_in_out, ping-pong, LV_ANIM_REPEAT_INFINITE) but
// CONTINUOUS -- this answers a standing state, not power_hint_pulse()'s
// finite two-breathe acknowledgment of one input. Period matches the chevron
// exactly (1.2s day / 2.4s night); the OPACITY RANGE deliberately does not --
// this badge already sits at a lower night ceiling than its day steady-state
// (LV_OPA_40 vs LV_OPA_COVER, scr_dial.c's own call site), because a
// full-brightness red breathe next to someone's face at 2am is wrong for a
// bedside device even as a battery warning.
#define POWER_GLYPH_BREATHE_DAY_LO   LV_OPA_60
#define POWER_GLYPH_BREATHE_DAY_HI   LV_OPA_100
#define POWER_GLYPH_BREATHE_NIGHT_LO LV_OPA_20
#define POWER_GLYPH_BREATHE_NIGHT_HI LV_OPA_50

static inline void power_glyph_breathe_start(power_glyph_t *pg, bool night)
{
    lv_anim_del(pg->wrap, power_glyph_set_wrap_opa);
    uint32_t half = night ? 2400 : 1200;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, pg->wrap);
    lv_anim_set_exec_cb(&a, power_glyph_set_wrap_opa);
    lv_anim_set_values(&a, night ? POWER_GLYPH_BREATHE_NIGHT_LO : POWER_GLYPH_BREATHE_DAY_LO,
                            night ? POWER_GLYPH_BREATHE_NIGHT_HI : POWER_GLYPH_BREATHE_DAY_HI);
    lv_anim_set_time(&a, half);
    lv_anim_set_playback_time(&a, half);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
    pg->breathing = true;
    pg->breathe_night = night;
}

static inline void power_glyph_breathe_stop(power_glyph_t *pg, lv_opa_t restore_opa)
{
    if (pg->breathing) lv_anim_del(pg->wrap, power_glyph_set_wrap_opa);
    pg->breathing = false;
    lv_obj_set_style_opa(pg->wrap, restore_opa, 0);
}

// `fill`'s width, px: the usable interior scaled by pct and rounded to the
// NEAREST pixel (floor-rounding drew 78% as 75%), with a 1px floor even at
// 0% (§11.3 -- an empty outline reads as broken, not "nearly empty"). With
// a 16px usable span: 5% -> 1px, 15% -> 2px, 20% -> 3px, 50% -> 8px,
// 100% -> 16px.
static inline void power_glyph_set_fill_pct(power_glyph_t *pg, int8_t pct)
{
    if (pct < 0)   pct = 0;
    if (pct > 100) pct = 100;
    int usable = POWER_GLYPH_BODY_W - 2 * POWER_GLYPH_FILL_INSET;
    int w = (usable * pct + 50) / 100;
    if (w < POWER_GLYPH_FILL_MIN_W) w = POWER_GLYPH_FILL_MIN_W;
    lv_obj_set_width(pg->fill, w);
}

// Create the shared assembly at LV_ALIGN_CENTER (0, y_off) -- callers pass
// their own (46 - CY) the same way every other element on these faces
// computes its offset. `label` keeps the old font/slot for the CHARGE flash
// text only; `wrap`+children are the new drawn fill, same slot, initially
// hidden -- power_glyph_apply's first call (power_src still UNKNOWN at
// boot) leaves everything hidden until there is something to show. Colors
// seeded from `pal` here; callers re-tint (and re-dim/breathe) every render
// via power_glyph_apply's own ink/opa/night arguments below.
static inline void power_glyph_create(power_glyph_t *pg, lv_obj_t *scr, lv_coord_t y_off, const dial_palette_t *pal)
{
    pg->label = lv_label_create(scr);
    lv_obj_set_style_text_font(pg->label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(pg->label, pal->ink_secondary, 0);
    lv_label_set_text(pg->label, LV_SYMBOL_CHARGE);   // the only text this label shows now
    lv_obj_clear_flag(pg->label, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(pg->label, LV_ALIGN_CENTER, 0, y_off);
    lv_obj_add_flag(pg->label, LV_OBJ_FLAG_HIDDEN);

    pg->wrap = lv_obj_create(scr);
    lv_obj_set_size(pg->wrap, POWER_GLYPH_BODY_W + POWER_GLYPH_NUB_W, POWER_GLYPH_BODY_H);
    lv_obj_set_style_bg_opa(pg->wrap, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pg->wrap, 0, 0);
    lv_obj_set_style_pad_all(pg->wrap, 0, 0);
    lv_obj_clear_flag(pg->wrap, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(pg->wrap, LV_ALIGN_CENTER, 0, y_off);
    lv_obj_add_flag(pg->wrap, LV_OBJ_FLAG_HIDDEN);

    pg->body = lv_obj_create(pg->wrap);
    lv_obj_set_size(pg->body, POWER_GLYPH_BODY_W, POWER_GLYPH_BODY_H);
    lv_obj_set_style_radius(pg->body, 1, 0);
    lv_obj_set_style_bg_opa(pg->body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(pg->body, 1, 0);
    lv_obj_set_style_border_color(pg->body, pal->ink_secondary, 0);
    lv_obj_set_style_pad_all(pg->body, 0, 0);
    lv_obj_clear_flag(pg->body, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(pg->body, LV_ALIGN_LEFT_MID, 0, 0);

    pg->nub = lv_obj_create(pg->wrap);
    lv_obj_set_size(pg->nub, POWER_GLYPH_NUB_W, POWER_GLYPH_NUB_H);
    lv_obj_set_style_radius(pg->nub, 0, 0);
    lv_obj_set_style_border_width(pg->nub, 0, 0);
    lv_obj_set_style_bg_color(pg->nub, pal->ink_secondary, 0);
    lv_obj_clear_flag(pg->nub, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(pg->nub, LV_ALIGN_RIGHT_MID, 0, 0);

    pg->fill = lv_obj_create(pg->body);
    lv_obj_set_size(pg->fill, POWER_GLYPH_FILL_MIN_W, POWER_GLYPH_BODY_H - 2 * POWER_GLYPH_FILL_INSET);
    lv_obj_set_style_radius(pg->fill, 0, 0);
    lv_obj_set_style_border_width(pg->fill, 0, 0);
    lv_obj_set_style_bg_color(pg->fill, pal->ink_secondary, 0);
    lv_obj_clear_flag(pg->fill, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(pg->fill, LV_ALIGN_LEFT_MID, POWER_GLYPH_FILL_INSET, 0);

    pg->charge_timer  = NULL;
    pg->breathing     = false;
    pg->breathe_night = false;
}

static inline void power_glyph_destroy(power_glyph_t *pg)
{
    if (pg->charge_timer) { lv_timer_del(pg->charge_timer); pg->charge_timer = NULL; }
    if (pg->wrap) lv_anim_del(pg->wrap, power_glyph_set_wrap_opa);
    pg->label = NULL;
    pg->wrap = pg->body = pg->nub = pg->fill = NULL;   // deleted with their screen (children of `scr`/`wrap`)
    pg->breathing = false;
}

// Call once per on_state with the screen's own persisted "last power_src
// rendered" (reset to PWR_UNKNOWN in create(), same as s_stale_shown is
// reset there), the current snapshot's power_src/power_pct, the ink color
// and steady-state opa the CALLER wants at non-low battery (each screen's
// own day/night tinting -- scr_dial.c is always ink_secondary at LV_OPA_40
// night dim, scr_standby.c swaps to neutral_holding at night instead and
// never dims its opa; see each call site), whether night mode is active
// (for the breathe's period/range), and the active palette (source of
// pal->warning for the breathe color). Idempotent: safe to call on every
// on_state regardless of whether power_src actually moved -- *last only
// differs from cur on a genuine transition, which is exactly when the
// CHARGE flash and the fill assembly are allowed to (re)trigger.
static inline void power_glyph_apply(power_glyph_t *pg, dial_power_src_t *last, dial_power_src_t cur,
                                      int8_t pct, lv_color_t ink, lv_opa_t opa, bool night,
                                      const dial_palette_t *pal)
{
    dial_power_src_t prev = *last;
    *last = cur;

    lv_obj_set_style_text_color(pg->label, ink, 0);
    lv_obj_set_style_text_opa(pg->label, opa, 0);

    if (cur == PWR_BATTERY) {
        // Persistent fill assembly. An unplug mid-flash must not leave a
        // stale CHARGE glyph on screen -- cancel any in-flight timer.
        if (pg->charge_timer) { lv_timer_del(pg->charge_timer); pg->charge_timer = NULL; }
        lv_obj_add_flag(pg->label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(pg->wrap, LV_OBJ_FLAG_HIDDEN);
        power_glyph_set_fill_pct(pg, pct);

        // No hysteresis needed here: dial_power.c holds power_pct non-
        // increasing while on battery, so once low it stays low until the
        // next plug-in resets it -- the boundary can't flap on noise.
        bool low = pct >= 0 && pct <= DIAL_BATTERY_PCT_LOW;
        if (low) {
            lv_obj_set_style_border_color(pg->body, pal->warning, 0);
            lv_obj_set_style_bg_color(pg->nub, pal->warning, 0);
            lv_obj_set_style_bg_color(pg->fill, pal->warning, 0);
            if (!pg->breathing || pg->breathe_night != night) power_glyph_breathe_start(pg, night);
        } else {
            power_glyph_breathe_stop(pg, opa);
            lv_obj_set_style_border_color(pg->body, ink, 0);
            lv_obj_set_style_bg_color(pg->nub, ink, 0);
            lv_obj_set_style_bg_color(pg->fill, ink, 0);
        }
        return;
    }
    if (cur == PWR_PLUGGED) {
        power_glyph_breathe_stop(pg, opa);
        lv_obj_add_flag(pg->wrap, LV_OBJ_FLAG_HIDDEN);
        if (prev == PWR_BATTERY && !pg->charge_timer) {
            // The one real transition that gets a confirmation (§10.4).
            lv_label_set_text(pg->label, LV_SYMBOL_CHARGE);
            lv_obj_clear_flag(pg->label, LV_OBJ_FLAG_HIDDEN);
            pg->charge_timer = lv_timer_create(power_glyph_charge_timer_cb, POWER_GLYPH_CHARGE_MS, pg);
            lv_timer_set_repeat_count(pg->charge_timer, 1);
        }
        // Already PLUGGED, or the flash is already running: nothing to do --
        // no glyph at PLUGGED steady state.
        return;
    }
    // UNKNOWN: nothing shown, and no leftover flash/breathe from a state
    // that can't legitimately follow it (defensive -- power_src never
    // actually reverts to UNKNOWN post-boot, see dial_power.c).
    power_glyph_breathe_stop(pg, opa);
    if (pg->charge_timer) { lv_timer_del(pg->charge_timer); pg->charge_timer = NULL; }
    lv_obj_add_flag(pg->label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(pg->wrap, LV_OBJ_FLAG_HIDDEN);
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
