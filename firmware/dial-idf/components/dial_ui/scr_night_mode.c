/*
 * SCR_NIGHT_MODE — Settings -> Night mode, the three-choice picker for
 * docs/SPEC-night-window.md's user-settable night window. Same shape as
 * scr_timezone.c: a dial_list (Back at row 0) where every other row is a
 * complete, final choice — Off, then one row per dial_state.h preset
 * (DIAL_NIGHT_PRESET_START/END/LABEL). No time editing, no schedule engine
 * (§2's "why presets, not a time editor" explains the scope cut).
 *
 * Row tap: Off -> dial_state_set_night_on(false); a preset ->
 * dial_state_set_night_start_min/end_min to the pair THEN
 * dial_state_set_night_on(true) — flag last, so the worker (main.c's
 * dial_night_active, read every steady-state tick) never sees on=true
 * paired with a half-written window (§5). Checkmark: Off if !night_on, the
 * matching preset if night_on and the stored pair matches one, nothing
 * otherwise (a pair a future custom editor produced — see
 * dial_night_row_value's comment in ui_screens_internal.h).
 *
 * Threading — NOT the timezone case (§5's own warning: the resemblance is a
 * trap). These prefs live in dial_state's own mutex-protected store, so the
 * setters are called directly from this screen (the LVGL task), exactly
 * like scr_brightness.c already calls dial_state_set_bri_night_pct() — no
 * CMD_*, no queue plumbing.
 *
 * Cursor seeding (§5's "check which"): dial_list opens on row 0 (Back)
 * unless told otherwise (dial_list_settle's own header comment) — and every
 * existing screen in this UI (scr_timezone.c, scr_settings.c, scr_update.c,
 * scr_brightness_menu.c) hardcodes a fixed focus_idx rather than tracking
 * the checkmark. This screen is the one the spec asked to fix instead:
 * focus_idx_for() below seeds onto the CURRENT value.
 *
 * Entered from Settings only (no packed arg needed); always returns there.
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"
#include "dial_list.h"

#define CY 180
#define ROW_H 76

// Row 0 = Back, row 1 = Off, rows 2..2+DIAL_NIGHT_PRESETS_N-1 = presets.
#define ROW_OFF     1
#define ROW_PRESET0 2

static lv_obj_t *s_list;
static lv_obj_t *s_title_lbl;
static lv_obj_t *s_note_lbl;   // §7 annotation line under the title, "" when the clock is valid
static lv_obj_t *s_val_off;
static lv_obj_t *s_val_preset[DIAL_NIGHT_PRESETS_N];

/* ---- row factory (scr_timezone.c's, ported verbatim) --------------------*/

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

static void go_back(void)
{
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

static void back_row_cb(lv_event_t *e)
{
    (void)e;
    dial_haptics_play(HAPTIC_TICK);
    go_back();
}

static void off_row_cb(lv_event_t *e)
{
    (void)e;
    app_state_t st;
    dial_state_get(&st);
    if (!st.night_on) {
        // Already off — same no-op-reselect precedent as scr_timezone.c's
        // zone_row_cb: the tap still has to do something, so still exit.
        dial_haptics_play(HAPTIC_TICK);
        go_back();
        return;
    }
    dial_haptics_play(HAPTIC_CONFIRM);
    dial_state_set_night_on(false);
    go_back();
}

// One callback for every preset row; which one rides in user_data as an
// index into DIAL_NIGHT_PRESET_START/END/LABEL (dial_state.h) — that table
// is compile-time-static, same reasoning scr_timezone.c's zone_row_cb uses
// for preferring an index over a pointer.
static void preset_row_cb(lv_event_t *e)
{
    int i = (int)(uintptr_t)lv_event_get_user_data(e);
    app_state_t st;
    dial_state_get(&st);
    bool already = st.night_on && st.night_start_min == DIAL_NIGHT_PRESET_START[i] &&
                                   st.night_end_min   == DIAL_NIGHT_PRESET_END[i];
    if (already) {
        dial_haptics_play(HAPTIC_TICK);
        go_back();
        return;
    }
    dial_haptics_play(HAPTIC_CONFIRM);
    // Start then end then the flag last (§5) — the worker must never see
    // on=true paired with a half-written window.
    dial_state_set_night_start_min(DIAL_NIGHT_PRESET_START[i]);
    dial_state_set_night_end_min(DIAL_NIGHT_PRESET_END[i]);
    dial_state_set_night_on(true);
    go_back();
}

/* ---- palette ---------------------------------------------------------------*/

static void apply_palette(void)
{
    const dial_palette_t *pal = PAL();
    lv_obj_t *scr = lv_obj_get_parent(s_list);
    lv_obj_set_style_bg_color(scr, pal->bg, 0);
    lv_obj_set_style_text_color(s_title_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_note_lbl, pal->warning, 0);

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

// See the header comment's "Cursor seeding" note — this screen seeds onto
// the current value instead of a hardcoded row.
static int focus_idx_for(const app_state_t *st)
{
    if (!st->night_on) return ROW_OFF;
    for (int i = 0; i < DIAL_NIGHT_PRESETS_N; i++)
        if (st->night_start_min == DIAL_NIGHT_PRESET_START[i] &&
            st->night_end_min   == DIAL_NIGHT_PRESET_END[i]) return ROW_PRESET0 + i;
    return ROW_OFF;   // stored pair matches no preset (unreachable today,
                       // see dial_night_row_value's comment) -- park on Off
                       // rather than Back.
}

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    app_state_t st;
    dial_state_get(&st);

    s_list = dial_list_create(scr, ROW_H);

    make_row(s_list, LV_SYMBOL_LEFT "  Back", back_row_cb, NULL, NULL);
    make_row(s_list, "Off", off_row_cb, NULL, &s_val_off);
    for (int i = 0; i < DIAL_NIGHT_PRESETS_N; i++)
        make_row(s_list, DIAL_NIGHT_PRESET_LABEL[i], preset_row_cb, (void *)(uintptr_t)i, &s_val_preset[i]);

    // Title + §7 annotation, created AFTER the list so both draw over rows
    // scrolling beneath them (same fixed-slot idiom every other menu
    // sub-screen uses). Title at 56, not the shared 64 slot every other list
    // uses: this is the one list screen with a second fixed line (the note)
    // under its title, and at 64/84 the note's box (76.5-91.5) touched the
    // row-above-focus label's box (91.7-116.3) with 0.2px to spare
    // (docs/REPORT-screen-layout-audit.md §12). 56/74 puts the note at
    // 66.5-81.5, 10px clear of that label; the chord at y 47 is 242px, so
    // "NIGHT MODE" (108px) has plenty of room.
    s_title_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_title_lbl, "NIGHT MODE");
    lv_obj_align(s_title_lbl, LV_ALIGN_CENTER, 0, 56 - CY);

    s_note_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_note_lbl, &lv_font_montserrat_12, 0);
    lv_label_set_text(s_note_lbl, "");
    lv_obj_align(s_note_lbl, LV_ALIGN_CENTER, 0, 74 - CY);

    apply_palette();
    dial_list_settle(s_list, focus_idx_for(&st));
}

static void destroy(void)
{
    s_list = NULL;
    s_title_lbl = NULL;
    s_note_lbl = NULL;
    s_val_off = NULL;
    for (int i = 0; i < DIAL_NIGHT_PRESETS_N; i++) s_val_preset[i] = NULL;
}

// Checkmark placement + the §7 annotation are the only per-state renders
// this screen has — every other visual (palette, row text) is fixed at
// create(). Re-derived from app_state_t/dial_time_valid() on every state
// tick rather than cached, same reasoning as scr_timezone.c's on_state.
static void on_state(const app_state_t *st)
{
    if (!s_list) return;
    apply_palette();

    lv_label_set_text(s_val_off, !st->night_on ? LV_SYMBOL_OK : "");
    for (int i = 0; i < DIAL_NIGHT_PRESETS_N; i++) {
        bool match = st->night_on && st->night_start_min == DIAL_NIGHT_PRESET_START[i] &&
                                      st->night_end_min   == DIAL_NIGHT_PRESET_END[i];
        lv_label_set_text(s_val_preset[i], match ? LV_SYMBOL_OK : "");
    }

    // No leading dash here (unlike the Settings row's " - reason" suffix,
    // ui_screens_internal.h) -- this line has no preceding value to
    // separate from, and the compiled fonts have no dash glyph to spend on
    // pure punctuation.
    const char *reason = dial_night_clock_reason();
    if (reason[0]) {
        lv_label_set_text(s_note_lbl, reason);
    } else {
        lv_label_set_text(s_note_lbl, "");
    }
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
    go_back();
    return true;
}

const ui_screen_t scr_night_mode = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_knob = on_knob, .on_gesture = on_gesture,
};
