/*
 * SCR_PAD_DISCOVERY — live progress face while dial_pad_discovery_scan()
 * sweeps the subnet for the pad (docs/SPEC-pad-discovery.md). Shown while
 * conn_phase_t is PH_PAD_DISCOVERY, driven entirely by that phase — nothing
 * on this screen is itself interactive besides the escape gesture below.
 *
 * Nothing opens this screen on purpose (unlike SCR_SETTINGS/SCR_TIMEZONE/
 * etc.) — it is what nav_policy shows when a scan is running and the user
 * hasn't deliberately gone anywhere else, so it is never itself in
 * nav_policy's sticky "deliberately opened" screen list
 * (docs/SPEC-connect-phases.md). A swipe still reaches the menu, same
 * escape scr_connecting.c already offers from PH_SOMNUS_CONNECTING/
 * PH_DEGRADED, so a scan running in the background never traps anyone here.
 *
 * Progress text rides app_state_t.phase_err — the same channel
 * dial_state_set_phase() already carries PH_DEGRADED's error/retry text on,
 * no new IPC. dial_pad_discovery.c's worker tasks write
 * "<pass headline>\n<checked>/<total>" after every probe; this screen
 * splits on the newline. The headline itself changes between pass 1
 * ("Looking for your Somnus pad...") and pass 2 ("Still looking (checking
 * more slowly)...") specifically so the counter resetting to 0/total at the
 * pass boundary reads as a deliberate escalation, not a restart or a glitch
 * (docs/SPEC-pad-discovery.md's "New phase and screen" section).
 */
#include "ui_screens_internal.h"

#define CX 180
#define CY 180
#define ARC_R 110

static lv_obj_t *s_arc;
static lv_obj_t *s_headline_lbl;
static lv_obj_t *s_fraction_lbl;

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    // Display-only progress ring — same "no knob dot, CLICKABLE cleared"
    // treatment every other non-draggable arc in this UI uses
    // (scr_brightness.c's own chassis ring, scr_adjust_mode.c's).
    s_arc = lv_arc_create(scr);
    lv_obj_set_size(s_arc, 2 * ARC_R, 2 * ARC_R);
    lv_obj_align(s_arc, LV_ALIGN_CENTER, 0, 30);
    lv_arc_set_rotation(s_arc, 135);
    lv_arc_set_bg_angles(s_arc, 0, 270);
    lv_arc_set_range(s_arc, 0, 100);
    lv_arc_set_value(s_arc, 0);
    lv_obj_set_style_arc_width(s_arc, 14, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc, 14, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(s_arc, true, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(s_arc, LV_OBJ_FLAG_CLICKABLE);

    s_headline_lbl = lv_label_create(scr);
    lv_obj_set_width(s_headline_lbl, 280);
    lv_label_set_long_mode(s_headline_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(s_headline_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_headline_lbl, &lv_font_montserrat_16, 0);
    lv_obj_align(s_headline_lbl, LV_ALIGN_CENTER, 0, 60 - CY);
    lv_label_set_text(s_headline_lbl, "");

    s_fraction_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_fraction_lbl, &lv_font_montserrat_24, 0);
    lv_obj_align(s_fraction_lbl, LV_ALIGN_CENTER, 0, 30);
    lv_label_set_text(s_fraction_lbl, "");
}

static void destroy(void)
{
    s_arc = NULL;
    s_headline_lbl = NULL;
    s_fraction_lbl = NULL;
}

// Splits phase_err's "<headline>\n<checked>/<total>" (dial_pad_discovery.c)
// into the two labels plus the arc's percent. Defensive about a phase_err
// that hasn't been set yet (empty, or missing the '/') — renders a plain
// "Looking for your Somnus pad..." with an empty ring rather than garbage,
// which covers the one tick between this screen appearing and the first
// worker task actually reporting progress.
static void on_state(const app_state_t *st)
{
    if (!s_arc) return;
    const dial_palette_t *pal = PAL();
    lv_obj_t *scr = lv_obj_get_parent(s_arc);
    lv_obj_set_style_bg_color(scr, pal->bg, 0);
    lv_obj_set_style_arc_color(s_arc, pal->track, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(s_arc, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_arc_color(s_arc, pal->ink_primary, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(s_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_text_color(s_headline_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_fraction_lbl, pal->ink_primary, 0);

    const char *err = st->phase_err;
    const char *nl = strchr(err, '\n');
    int checked = 0, total = 0;
    bool have_fraction = false;
    if (nl && sscanf(nl + 1, "%d/%d", &checked, &total) == 2 && total > 0) {
        have_fraction = true;
    }

    if (nl) {
        char headline[80];
        size_t len = (size_t)(nl - err);
        if (len >= sizeof(headline)) len = sizeof(headline) - 1;
        memcpy(headline, err, len);
        headline[len] = '\0';
        lv_label_set_text(s_headline_lbl, headline);
    } else {
        // Not set yet, or an unexpected shape — the one covered "no
        // progress reported this tick" case (see the function comment).
        lv_label_set_text(s_headline_lbl, err[0] ? err : "Looking for your Somnus pad...");
    }

    if (have_fraction) {
        char frac[24];
        snprintf(frac, sizeof(frac), "%d/%d", checked, total);
        lv_label_set_text(s_fraction_lbl, frac);
        lv_arc_set_value(s_arc, (int32_t)((checked * 100L) / total));
    } else {
        lv_label_set_text(s_fraction_lbl, "");
        lv_arc_set_value(s_arc, 0);
    }
}

// Same escape scr_connecting.c already offers from PH_SOMNUS_CONNECTING/
// PH_DEGRADED (see this file's header comment) — a scan running in the
// background must never be the one screen nobody can swipe away from.
static bool on_gesture(lv_dir_t dir)
{
    if (dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return false;
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    return true;
}

const ui_screen_t scr_pad_discovery = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_gesture = on_gesture,
};
