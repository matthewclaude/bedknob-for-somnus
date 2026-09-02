/*
 * SCR_TIMEZONE — Settings -> Timezone, the on-device backstop for setting
 * the clock's zone without a browser (docs/SPEC-timezone-source.md's
 * "Settings row" section: an already-provisioned device can't reach the
 * Wi-Fi setup portal without a factory reset, and a device provisioned
 * through SCR_NETPICK's on-device picker never had a browser to detect a
 * zone from in the first place).
 *
 * A curated list of 11 (dial_state.h's DIAL_TZ_IANA/DIAL_TZ_LABEL), not the
 * ~400-zone table dial_time.c embeds — dial_list.h walks one row per
 * detent, and ~400 rows is not a knob-scrollable list. See dial_state.h's
 * comment above those tables for why this is a convenience layer, not a
 * restriction: the full table stays embedded and the portal still accepts
 * any zone a browser reports.
 *
 * A rotor list (dial_list.h), same Back-row-0 shape as scr_netpick.c/
 * scr_brightness_menu.c, but closer to scr_netpick.c's in spirit: every row
 * here already IS a complete, final choice (like scr_adjust_mode.c's two
 * pills), not a row that opens yet another picker — a tap both commits and
 * returns to Settings in one step. The currently-persisted zone (if it's
 * one of these 11) gets a checkmark; nothing else here needs to represent
 * "not set" specially — that's Settings' own row's job (scr_settings.c),
 * and simply showing no checkmark on any row is already the honest picture.
 *
 * Threading (docs/SPEC-timezone-source.md's Threading section — read that
 * before touching this): unlike scr_settings.c's row_bed_mode_cb, which
 * writes dial_state's own mutex-protected store directly before posting a
 * bare CMD_PAD_SETTINGS_CHANGED signal, this screen must NOT call
 * dial_time_set_iana_tz() itself. It mutates global libc TZ state
 * (setenv+tzset) that worker_task's dial_time_now() callers read
 * concurrently at steady state — safe for the Wi-Fi portal's direct call
 * (nothing was reading TZ state that early in boot) but not safe from the
 * LVGL task here. So the tapped row's index rides in CMD_TZ_CHANGED's `a`
 * field, and only main.c's handle_immediate_cmd (running on worker_task)
 * ever actually calls dial_time_set_iana_tz().
 *
 * Two entry points, one packed `arg` (scr_adjust_mode.c's s_origin idiom,
 * itself citing scr_brightness.c's): 0 = scr_settings.c's Timezone row,
 * a deliberate visit that should return to Settings; 1 + zone
 * (docs/SPEC-timezone-source.md's Fix 1 setup gate) = main.c's nav_policy
 * force-routing here because no zone has ever been set, which should NOT
 * return to Settings — the user never opened it and has no reason to be
 * there. s_origin stores it as-is, same reason scr_adjust_mode.c's does
 * (one value for go_back() to switch on). All three exits (Back, either
 * zone_row_cb branch, swipe-right) funnel through go_back(), same
 * "can't disagree about where back means" reasoning as that file's.
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"
#include "dial_list.h"
#include "dial_time.h"

#define CY 180
#define ROW_H 76

static lv_obj_t *s_list;
static lv_obj_t *s_title_lbl;
static lv_obj_t *s_val_row[DIAL_TZ_COUNT];   // checkmark cell, one per zone row

// Packed entry origin (see header comment): 0 = Settings, 1+zone = nav_policy's
// setup gate. Captured once in create(), read only by go_back().
static uintptr_t s_origin;

/* ---- row factory (scr_settings.c's/scr_brightness_menu.c's idiom) ------- */

static lv_obj_t *make_row(lv_obj_t *parent, const char *label_txt, lv_event_cb_t cb, void *user_data, lv_obj_t **value_out)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), ROW_H);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
    // 36px side insets, not 20 — same round-panel chord clipping every other
    // list screen in this UI already works around.
    lv_obj_set_style_pad_hor(row, 36, 0);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    if (cb) lv_obj_add_event_cb(row, cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t *lbl = lv_label_create(row);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
    lv_label_set_text(lbl, label_txt);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *val = lv_label_create(row);
    lv_obj_set_style_text_font(val, &lv_font_montserrat_16, 0);
    lv_label_set_text(val, "");
    lv_obj_align(val, LV_ALIGN_RIGHT_MID, 0, 0);
    if (value_out) *value_out = val;

    return row;
}

/* ---- row actions ----------------------------------------------------------*/

// The one place that decodes s_origin (see header comment) — every exit
// funnels through this instead of each hardcoding a destination, so they
// can never disagree about where "back" means. arg 0 -> Settings, arrived
// at by a lateral menu swipe, so leaves the same way (MOVE_RIGHT); arg
// 1+zone -> the dial face for that zone, arrived at by nav_policy's own
// forced navigation rather than a swipe, so leaves the same modal-ish way
// (LV_SCR_LOAD_ANIM_NONE), matching scr_update_prompt.c's own
// nav_policy-raised-screen dismissal.
static void go_back(void)
{
    if (s_origin == 0) {
        ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    } else {
        zone_idx_t zone = (zone_idx_t)(s_origin - 1);
        ui_router_go(SCR_DIAL, (void *)(uintptr_t)zone, LV_SCR_LOAD_ANIM_NONE);
    }
}

static void back_row_cb(lv_event_t *e)
{
    (void)e;
    dial_haptics_play(HAPTIC_TICK);
    // Leaving without picking a zone still dismisses the setup gate (docs/
    // SPEC-timezone-source.md's "Fix 1") -- without this, nav_policy would
    // just route straight back here on the next state commit, since the
    // zone is still unset. Session-only: asking again next boot is correct.
    dial_state_set_tz_prompted();
    go_back();
}

// One callback for every zone row; which one rides in user_data as an index
// into DIAL_TZ_IANA/DIAL_TZ_LABEL (dial_state.h) — that table is
// compile-time-static, so the index is stable for the life of the app (same
// reasoning scr_netpick.c's network_row_cb uses for preferring an index over
// a pointer, even though the underlying table here never gets rebuilt).
static void zone_row_cb(lv_event_t *e)
{
    int i = (int)(uintptr_t)lv_event_get_user_data(e);

    char cur[48];
    bool have = dial_time_get_iana_tz(cur, sizeof cur);
    // Either branch below leaves the screen, so either branch dismisses the
    // setup gate too (docs/SPEC-timezone-source.md's "Fix 1") -- see
    // back_row_cb's comment for why this call has to be here at all.
    dial_state_set_tz_prompted();

    if (have && strcmp(cur, DIAL_TZ_IANA[i]) == 0) {
        // Already the current zone — leave without a redundant command or
        // NVS write (same "no-op a reselect of the already-current choice"
        // precedent as scr_adjust_mode.c's select_mode()), but the tap still
        // has to DO something: this row is also the exit, so still go back.
        dial_haptics_play(HAPTIC_TICK);
        go_back();
        return;
    }

    dial_haptics_play(HAPTIC_CONFIRM);
    // Cannot call dial_time_set_iana_tz() here — see this file's header
    // comment and docs/SPEC-timezone-source.md's Threading section.
    // worker_task applies it in main.c's handle_immediate_cmd.
    app_cmd_t cmd = { .kind = CMD_TZ_CHANGED, .a = i };
    dial_cmd_post(&cmd);
    go_back();
}

/* ---- palette ---------------------------------------------------------------*/

static void apply_palette(void)
{
    const dial_palette_t *pal = PAL();
    lv_obj_t *scr = lv_obj_get_parent(s_list);
    lv_obj_set_style_bg_color(scr, pal->bg, 0);
    lv_obj_set_style_text_color(s_title_lbl, pal->ink_secondary, 0);

    uint32_t n = lv_obj_get_child_cnt(s_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *row = lv_obj_get_child(s_list, i);
        lv_obj_set_style_border_color(row, pal->track, 0);
        uint32_t rc = lv_obj_get_child_cnt(row);
        for (uint32_t j = 0; j < rc; j++) {
            lv_obj_t *lbl = lv_obj_get_child(row, j);
            lv_obj_set_style_text_color(lbl, j == 0 ? pal->ink_primary : pal->ink_secondary, 0);
        }
    }
}

/* ---- vtable ----------------------------------------------------------------*/

static void create(lv_obj_t *scr, void *arg)
{
    s_origin = (uintptr_t)arg;   // 0 = Settings, 1+zone = nav_policy's setup gate (see header comment)
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    s_list = dial_list_create(scr, ROW_H);

    make_row(s_list, LV_SYMBOL_LEFT "  Back", back_row_cb, NULL, NULL);
    for (int i = 0; i < DIAL_TZ_COUNT; i++)
        make_row(s_list, DIAL_TZ_LABEL[i], zone_row_cb, (void *)(uintptr_t)i, &s_val_row[i]);

    // Created AFTER the list so it draws over rows scrolling beneath it —
    // same fixed title slot every other menu sub-screen uses.
    s_title_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_title_lbl, "TIMEZONE");
    lv_obj_align(s_title_lbl, LV_ALIGN_CENTER, 0, 64 - CY);

    apply_palette();
    dial_list_settle(s_list, 1);   // open on the first zone, not on Back
}

static void destroy(void)
{
    s_list = NULL;
    s_title_lbl = NULL;
    for (int i = 0; i < DIAL_TZ_COUNT; i++) s_val_row[i] = NULL;
}

// Checkmark placement is the only per-state render this screen has — every
// other visual (palette, row text) is fixed at create(). Re-derived from
// dial_time_get_iana_tz() on every state tick rather than cached: cheap (11
// strcmps), and it's the only way this screen ever finds out a
// CMD_TZ_CHANGED it just posted actually landed.
static void on_state(const app_state_t *st)
{
    (void)st;
    if (!s_list) return;
    apply_palette();

    char cur[48];
    bool have = dial_time_get_iana_tz(cur, sizeof cur);
    for (int i = 0; i < DIAL_TZ_COUNT; i++)
        lv_label_set_text(s_val_row[i], (have && strcmp(cur, DIAL_TZ_IANA[i]) == 0) ? LV_SYMBOL_OK : "");
}

// The knob walks the focused row (one per detent, dial_list's rotor snap) —
// nothing on this screen is itself an adjustable control.
static bool on_knob(int detents)
{
    if (!s_list || detents == 0) return false;
    int r = dial_list_knob(s_list, detents);
    if (r < 0) dial_haptics_play_soft(HAPTIC_STOP);   // rotor hit its first/last row
    return true;
}

static bool on_gesture(lv_dir_t dir)
{
    if (dir != LV_DIR_RIGHT) return false;
    // Swipe-back is also a way to leave without picking a zone -- see
    // back_row_cb's comment.
    dial_state_set_tz_prompted();
    go_back();
    return true;
}

const ui_screen_t scr_timezone = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_knob = on_knob, .on_gesture = on_gesture,
};
