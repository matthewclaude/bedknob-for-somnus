/*
 * SCR_NIGHT_FACE — Settings -> Night face, the two-choice picker for
 * docs/SPEC-night-face.md §4's user-settable night-face layout. Same shape
 * as scr_night_mode.c: a dial_list (Back at row 0) where every other row is
 * a complete, final choice — Number only, then Full.
 *
 * Row tap: dial_state_set_night_face_min(true) for Number only,
 * dial_state_set_night_face_min(false) for Full. Checkmark: whichever
 * matches st->night_face_min.
 *
 * Threading — these prefs live in dial_state's own mutex-protected store,
 * so the setters are called directly from this screen (the LVGL task),
 * exactly like scr_night_mode.c's own row callbacks (and scr_brightness.c's
 * dial_state_set_bri_night_pct()) — no CMD_*, no queue plumbing.
 *
 * Cursor seeding: seeds onto the CURRENT value (scr_night_mode.c's own
 * fix — see that file's header comment's "Cursor seeding" note), not a
 * hardcoded row.
 *
 * Entered from Settings only (no packed arg needed); always returns there.
 * The row that opens this screen is itself present only while night_on is
 * true (scr_settings.c's sync_night_face_row, same add/remove-not-HIDDEN
 * mechanism as the Brightness night rows) — see that file for why.
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"
#include "dial_list.h"

#define CY 180
#define ROW_H 76

// Row 0 = Back, row 1 = Number only, row 2 = Full.
#define ROW_MIN  1
#define ROW_FULL 2

static lv_obj_t *s_list;
static lv_obj_t *s_title_lbl;
static lv_obj_t *s_val_min;
static lv_obj_t *s_val_full;

/* ---- row factory (scr_night_mode.c's, ported verbatim) --------------------*/

static lv_obj_t *make_row(lv_obj_t *parent, const char *label_txt, lv_event_cb_t cb, lv_obj_t **value_out)
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
    if (cb) lv_obj_add_event_cb(row, cb, LV_EVENT_CLICKED, NULL);

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

static void min_row_cb(lv_event_t *e)
{
    (void)e;
    if (dial_state_get_night_face_min()) {
        // Already Number only — same no-op-reselect precedent as
        // scr_night_mode.c's off_row_cb: the tap still has to do
        // something, so still exit.
        dial_haptics_play(HAPTIC_TICK);
        go_back();
        return;
    }
    dial_haptics_play(HAPTIC_CONFIRM);
    dial_state_set_night_face_min(true);
    go_back();
}

static void full_row_cb(lv_event_t *e)
{
    (void)e;
    if (!dial_state_get_night_face_min()) {
        dial_haptics_play(HAPTIC_TICK);
        go_back();
        return;
    }
    dial_haptics_play(HAPTIC_CONFIRM);
    dial_state_set_night_face_min(false);
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

// See the header comment's "Cursor seeding" note — this screen seeds onto
// the current value instead of a hardcoded row.
static int focus_idx_for(const app_state_t *st)
{
    return st->night_face_min ? ROW_MIN : ROW_FULL;
}

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    app_state_t st;
    dial_state_get(&st);

    s_list = dial_list_create(scr, ROW_H);

    make_row(s_list, LV_SYMBOL_LEFT "  Back", back_row_cb, NULL);
    make_row(s_list, "Number only", min_row_cb,  &s_val_min);
    make_row(s_list, "Full",        full_row_cb, &s_val_full);

    // Title, created AFTER the list so it draws over rows scrolling beneath
    // it (same fixed-slot idiom every other menu sub-screen uses).
    s_title_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_title_lbl, "NIGHT FACE");
    lv_obj_align(s_title_lbl, LV_ALIGN_CENTER, 0, 64 - CY);

    apply_palette();
    dial_list_settle(s_list, focus_idx_for(&st));
}

static void destroy(void)
{
    s_list = NULL;
    s_title_lbl = NULL;
    s_val_min = NULL;
    s_val_full = NULL;
}

// Checkmark placement is the only per-state render this screen has — the
// palette and row text are fixed at create(). Re-derived from app_state_t
// on every state tick rather than cached, same reasoning as
// scr_night_mode.c's on_state.
static void on_state(const app_state_t *st)
{
    if (!s_list) return;
    apply_palette();

    lv_label_set_text(s_val_min,  st->night_face_min  ? LV_SYMBOL_OK : "");
    lv_label_set_text(s_val_full, !st->night_face_min ? LV_SYMBOL_OK : "");
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

const ui_screen_t scr_night_face = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_knob = on_knob, .on_gesture = on_gesture,
};
