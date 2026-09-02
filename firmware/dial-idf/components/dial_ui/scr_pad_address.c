/*
 * SCR_PAD_ADDRESS — edit the Somnus pad's base URL ON THE DIAL, reached from
 * SCR_SETTINGS' "Pad Address" row.
 *
 * Reuses scr_passkey.c's character-wheel input pattern verbatim (spin the
 * knob to a candidate glyph, tap the checkmark disc to commit it) rather
 * than inventing a second on-screen text-entry idiom — see that file's own
 * header comment for why a wheel and not a QWERTY grid. The alphabet here is
 * just narrower: pad addresses are "http://" + an IPv4-or-hostname + an
 * optional ":port", so digits and a handful of URL punctuation come first,
 * with lowercase letters (for a hostname like "somnus.local") after.
 *
 * Unlike the Wi-Fi password screen, this field is not typed from blank every
 * time: it opens pre-filled with the CURRENTLY persisted address (dial_state_
 * get_pad_url), so the common case — nothing to change, or only one octet to
 * fix — is a few backspaces and re-typed characters, not retyping the whole
 * URL. Swiping away without tapping Save discards the edit; nothing is
 * persisted until Save.
 *
 * Validation is deliberately shallow (see dial_somnus.h's own header note):
 * this is a local, trusted-network field, not user-facing internet input.
 * Save only refuses a blank field or one that doesn't start with "http://" —
 * enough to catch "forgot the scheme" and "cleared it by accident" without
 * pretending to be a real URL parser.
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"

#define CX 180
#define CY 180

// Digits and URL punctuation first (an IP:port address needs nothing else),
// then lowercase letters for a hostname like "somnus.local". No uppercase,
// no symbols beyond what a host:port actually uses — this alphabet only
// needs to be short enough that spinning to any one of them is quick, unlike
// scr_passkey's 88-character wheel (which has to cover arbitrary Wi-Fi
// passwords).
static const char WHEEL[] = "0123456789.:/-abcdefghijklmnopqrstuvwxyz";

static lv_obj_t *s_title_lbl;
static lv_obj_t *s_url_lbl;
static lv_obj_t *s_hint_lbl;
static lv_obj_t *s_prev_lbl, *s_cand_lbl, *s_next_lbl;
static lv_obj_t *s_cand_btn;
static lv_obj_t *s_slot_hair;
static lv_obj_t *s_del_btn, *s_del_glyph;
static lv_obj_t *s_add_btn, *s_add_glyph;
static lv_obj_t *s_done_btn, *s_done_glyph;
static lv_obj_t *s_del_cap, *s_add_cap, *s_done_cap;

static int  s_pos;                             // index into WHEEL
static char s_buf[DIAL_PAD_URL_MAX_LEN + 1];
static int  s_len;

/* ---- motion helpers (same vocabulary as scr_passkey.c) ------------------*/

static void set_zoom_cb(void *obj, int32_t v) { lv_obj_set_style_transform_zoom((lv_obj_t *)obj, (int16_t)v, 0); }
static void set_x_cb(void *obj, int32_t v)    { lv_obj_set_x((lv_obj_t *)obj, (lv_coord_t)v); }

static void anim_zoom_bump(lv_obj_t *obj)
{
    lv_anim_del(obj, set_zoom_cb);
    lv_obj_set_style_transform_zoom(obj, 256, 0);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, set_zoom_cb);
    lv_anim_set_values(&a, 256, 266);
    lv_anim_set_time(&a, 45);
    lv_anim_set_playback_time(&a, 45);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_start(&a);
}

static void anim_nudge(lv_obj_t *obj, int dir)
{
    lv_anim_del(obj, set_x_cb);
    lv_obj_set_x(obj, 4 * dir);
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, set_x_cb);
    lv_anim_set_values(&a, 4 * dir, 0);
    lv_anim_set_time(&a, 140);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_start(&a);
}

/* ---- rendering -----------------------------------------------------------*/

static void render_wheel(void)
{
    int n = (int)strlen(WHEEL);
    char buf[2] = { 0, 0 };

    if (s_pos > 0) { buf[0] = WHEEL[s_pos - 1]; lv_label_set_text(s_prev_lbl, buf); }
    else             lv_label_set_text(s_prev_lbl, "");

    buf[0] = WHEEL[s_pos];
    lv_label_set_text(s_cand_lbl, buf);

    if (s_pos < n - 1) { buf[0] = WHEEL[s_pos + 1]; lv_label_set_text(s_next_lbl, buf); }
    else                 lv_label_set_text(s_next_lbl, "");
}

// Same shallow check Save enforces (see this file's header comment) —
// shared so the hint below and the Save button agree on what "invalid" means.
static bool url_is_valid(void)
{
    return s_len > 0 && strncmp(s_buf, "http://", 7) == 0;
}

// Cleartext readout with a trailing "|" caret, same idiom as scr_passkey's
// password field — there's no reason to hide a LAN address any more than a
// Wi-Fi password typed in the room it secures. Unlike that screen, this one
// always shows the buffer (even empty, as a bare caret): the field opens
// pre-filled, so "nothing typed yet" isn't a state worth a placeholder
// message the way a blank password field is.
static void render_readout(void)
{
    char out[sizeof(s_buf) + 1];
    snprintf(out, sizeof(out), "%s|", s_buf);
    lv_label_set_text(s_url_lbl, out);

    if (url_is_valid()) {
        lv_label_set_text(s_hint_lbl, "");
    } else {
        lv_label_set_text(s_hint_lbl, s_len == 0 ? "Enter the pad's address"
                                                  : "Must start with http://");
    }
}

/* ---- events ----------------------------------------------------------------*/

static void cand_event_cb(lv_event_t *e)
{
    (void)e;
    if (s_len >= (int)sizeof(s_buf) - 1) return;
    s_buf[s_len++] = WHEEL[s_pos];
    s_buf[s_len] = '\0';
    dial_haptics_play(HAPTIC_CONFIRM);
    render_readout();
}

static void del_event_cb(lv_event_t *e)
{
    (void)e;
    if (s_len == 0) {
        dial_haptics_play_soft(HAPTIC_STOP);
        return;
    }
    s_buf[--s_len] = '\0';
    dial_haptics_play(HAPTIC_TICK);
    render_readout();
}

static void done_event_cb(lv_event_t *e)
{
    (void)e;
    if (!url_is_valid()) {
        // Refuse rather than guess: an empty or scheme-less address would
        // silently point the dial at nothing (see dial_somnus_connect's own
        // contract — it does a real GET against whatever it's handed).
        dial_haptics_play(HAPTIC_ERROR);
        render_readout();   // makes sure the hint is showing, not just relying on the last edit having done it
        return;
    }
    dial_haptics_play(HAPTIC_CONFIRM);
    dial_state_set_pad_url(s_buf);
    // Apply it live rather than waiting for a reboot. Once the device has
    // reached the steady-state loop, main.c's handle_immediate_cmd sees this
    // CMD_PAD_SETTINGS_CHANGED, re-reads the address we just persisted, and
    // re-probes it (dial_somnus_connect() must run on the worker task, never
    // here — see dial_somnus.h's threading contract). If the device is still
    // in the initial connect retry loop, handle_immediate_cmd isn't running
    // yet — that loop re-reads the persisted address on every attempt
    // instead, so the change takes effect on the next retry either way.
    app_cmd_t cmd = { .kind = CMD_PAD_SETTINGS_CHANGED };
    dial_cmd_post(&cmd);
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

/* ---- palette -----------------------------------------------------------*/

static void apply_palette(void)
{
    const dial_palette_t *pal = PAL();
    lv_obj_t *scr = lv_obj_get_parent(s_title_lbl);
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    lv_obj_set_style_text_color(s_title_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_url_lbl, pal->ink_primary, 0);
    lv_obj_set_style_text_color(s_hint_lbl, pal->warning, 0);

    lv_obj_set_style_text_color(s_prev_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_opa(s_prev_lbl, LV_OPA_40, 0);
    lv_obj_set_style_text_color(s_next_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_opa(s_next_lbl, LV_OPA_40, 0);
    lv_obj_set_style_text_color(s_cand_lbl, pal->ink_primary, 0);

    lv_obj_set_style_bg_color(s_slot_hair, pal->track, 0);

    lv_obj_set_style_bg_color(s_del_btn, pal->surface, 0);
    lv_obj_set_style_border_color(s_del_btn, pal->track, 0);
    lv_obj_set_style_text_color(s_del_glyph, pal->ink_primary, 0);

    lv_obj_set_style_bg_color(s_add_btn, pal->surface, 0);
    lv_obj_set_style_border_color(s_add_btn, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_add_glyph, pal->ink_primary, 0);

    lv_obj_set_style_bg_color(s_done_btn, pal->surface, 0);
    lv_obj_set_style_border_color(s_done_btn, pal->track, 0);
    lv_obj_set_style_text_color(s_done_glyph, pal->ink_primary, 0);

    lv_obj_set_style_text_color(s_del_cap, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_add_cap, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_done_cap, pal->ink_secondary, 0);
}

/* ---- vtable ------------------------------------------------------------*/

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    s_pos = 0;
    dial_state_get_pad_url(s_buf, sizeof(s_buf));
    s_len = (int)strlen(s_buf);

    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    s_title_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_title_lbl, "PAD ADDRESS");
    lv_obj_align(s_title_lbl, LV_ALIGN_CENTER, 0, 56 - CY);

    s_url_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_url_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_width(s_url_lbl, 300);
    lv_label_set_long_mode(s_url_lbl, LV_LABEL_LONG_DOT);   // keeps the tail (the port) visible if it overruns
    lv_obj_set_style_text_align(s_url_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_url_lbl, LV_ALIGN_CENTER, 0, 92 - CY);

    s_hint_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_hint_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_width(s_hint_lbl, 280);
    lv_obj_set_style_text_align(s_hint_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_hint_lbl, LV_ALIGN_CENTER, 0, 116 - CY);

    s_prev_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_prev_lbl, &lv_font_montserrat_24, 0);
    lv_obj_align(s_prev_lbl, LV_ALIGN_CENTER, 100 - CX, 170 - CY);

    s_next_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_next_lbl, &lv_font_montserrat_24, 0);
    lv_obj_align(s_next_lbl, LV_ALIGN_CENTER, 260 - CX, 170 - CY);

    s_slot_hair = lv_obj_create(scr);
    lv_obj_set_size(s_slot_hair, 56, 2);
    lv_obj_set_style_border_width(s_slot_hair, 0, 0);
    lv_obj_clear_flag(s_slot_hair, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(s_slot_hair, LV_ALIGN_CENTER, 0, 200 - CY);

    s_cand_btn = dial_btn_create(scr);
    lv_obj_set_size(s_cand_btn, 100, 100);
    lv_obj_set_style_radius(s_cand_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(s_cand_btn, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(s_cand_btn, 0, 0);
    lv_obj_align(s_cand_btn, LV_ALIGN_CENTER, 0, 170 - CY);
    lv_obj_add_event_cb(s_cand_btn, cand_event_cb, LV_EVENT_CLICKED, NULL);

    s_cand_lbl = lv_label_create(s_cand_btn);
    lv_obj_set_style_text_font(s_cand_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_transform_pivot_x(s_cand_lbl, LV_PCT(50), 0);
    lv_obj_set_style_transform_pivot_y(s_cand_lbl, LV_PCT(50), 0);
    lv_obj_center(s_cand_lbl);

    s_del_btn = dial_btn_create(scr);
    lv_obj_set_size(s_del_btn, 88, 88);
    lv_obj_set_style_radius(s_del_btn, 44, 0);
    lv_obj_set_style_border_width(s_del_btn, 1, 0);
    lv_obj_align(s_del_btn, LV_ALIGN_CENTER, 84 - CX, 240 - CY);
    lv_obj_add_event_cb(s_del_btn, del_event_cb, LV_EVENT_CLICKED, NULL);
    s_del_glyph = lv_label_create(s_del_btn);
    lv_obj_set_style_text_font(s_del_glyph, &lv_font_montserrat_28, 0);
    lv_label_set_text(s_del_glyph, LV_SYMBOL_BACKSPACE);
    lv_obj_center(s_del_glyph);

    s_add_btn = dial_btn_create(scr);
    lv_obj_set_size(s_add_btn, 88, 88);
    lv_obj_set_style_radius(s_add_btn, 44, 0);
    lv_obj_set_style_border_width(s_add_btn, 2, 0);
    lv_obj_align(s_add_btn, LV_ALIGN_CENTER, 0, 240 - CY);
    lv_obj_add_event_cb(s_add_btn, cand_event_cb, LV_EVENT_CLICKED, NULL);
    s_add_glyph = lv_label_create(s_add_btn);
    lv_obj_set_style_text_font(s_add_glyph, &lv_font_montserrat_28, 0);
    lv_label_set_text(s_add_glyph, LV_SYMBOL_OK);
    lv_obj_center(s_add_glyph);

    s_done_btn = dial_btn_create(scr);
    lv_obj_set_size(s_done_btn, 88, 88);
    lv_obj_set_style_radius(s_done_btn, 44, 0);
    lv_obj_set_style_border_width(s_done_btn, 1, 0);
    lv_obj_align(s_done_btn, LV_ALIGN_CENTER, 276 - CX, 240 - CY);
    lv_obj_add_event_cb(s_done_btn, done_event_cb, LV_EVENT_CLICKED, NULL);
    s_done_glyph = lv_label_create(s_done_btn);
    lv_obj_set_style_text_font(s_done_glyph, &lv_font_montserrat_28, 0);
    lv_label_set_text(s_done_glyph, LV_SYMBOL_SAVE);
    lv_obj_center(s_done_glyph);

    struct { lv_obj_t **lbl; const char *txt; lv_coord_t x; } caps[] = {
        { &s_del_cap,  "Delete", 84  },
        { &s_add_cap,  "Add",    180 },
        { &s_done_cap, "Save",   276 },
    };
    for (size_t i = 0; i < sizeof(caps) / sizeof(caps[0]); i++) {
        lv_obj_t *c = lv_label_create(scr);
        lv_obj_set_style_text_font(c, &lv_font_montserrat_12, 0);
        lv_label_set_text(c, caps[i].txt);
        lv_obj_align(c, LV_ALIGN_CENTER, caps[i].x - CX, 292 - CY);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_CLICKABLE);
        *caps[i].lbl = c;
    }

    render_wheel();
    render_readout();
    apply_palette();
}

static void destroy(void)
{
    if (s_cand_lbl) lv_anim_del(s_cand_lbl, NULL);
    s_len = 0;
    s_buf[0] = '\0';

    s_title_lbl = NULL;
    s_url_lbl = NULL;
    s_hint_lbl = NULL;
    s_prev_lbl = s_cand_lbl = s_next_lbl = NULL;
    s_cand_btn = NULL;
    s_slot_hair = NULL;
    s_del_btn = s_del_glyph = NULL;
    s_add_btn = s_add_glyph = NULL;
    s_done_btn = s_done_glyph = NULL;
    s_del_cap = s_add_cap = s_done_cap = NULL;
}

static void on_state(const app_state_t *st)
{
    (void)st;
    if (!s_title_lbl) return;
    apply_palette();
}

static bool on_knob(int detents)
{
    if (!s_cand_lbl || detents == 0) return false;

    int n = (int)strlen(WHEEL);
    int np = s_pos + detents;
    if (np < 0)     np = 0;
    if (np > n - 1) np = n - 1;

    if (np == s_pos) {
        dial_haptics_play_soft(HAPTIC_STOP);
        anim_nudge(s_cand_btn, detents > 0 ? 1 : -1);
        return true;
    }

    s_pos = np;
    render_wheel();
    dial_haptics_play(HAPTIC_TICK);
    anim_zoom_bump(s_cand_lbl);
    return true;
}

static bool on_gesture(lv_dir_t dir)
{
    if (dir != LV_DIR_RIGHT) return false;
    ui_router_go(SCR_SETTINGS, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    return true;
}

const ui_screen_t scr_pad_address = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_knob = on_knob, .on_gesture = on_gesture,
};
