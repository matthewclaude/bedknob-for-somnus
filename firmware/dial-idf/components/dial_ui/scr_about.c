/*
 * SCR_ABOUT — device/about sub-screen, reached from SCR_MENU. A single
 * scrollable, read-only list: Firmware / IDF / Wi-Fi / Pad / Battery. There
 * is no other entry point (no schedule, no zone) so on_state has nothing to
 * gate on besides its own root pointer.
 *
 * The Software update control this screen used to own (M6) moved to its own
 * SCR_UPDATE sub-screen (M7, reached from SCR_MENU's "Update" row) — see
 * scr_update.c for the status-driven row, the tap-twice-to-confirm install,
 * and the FAILED-clearing teardown that all lived here before the split.
 *
 * Wi-Fi/Pad/Battery rows (docs/SPEC-power-sensing.md §11.3): this is the
 * "diagnostics content becomes rows on the existing About screen, not a new
 * screen" decision, adopted from upstream PR #4's percentage/low-battery
 * work without adopting their separate diagnostics screen or its swipe-down
 * entry point (neither works on this port — same touch_filter-swallows-the-
 * gesture problem their own commit names — and this port doesn't want a new
 * screen or gesture regardless).
 *
 * Block redesign (docs/REPORT-battery-pct-about-redesign.md's "block
 * redesign" addendum): the info rows (everything except Back) used to be
 * label-left/value-right, like scr_settings.c's rows. That shape caused
 * three straight rounds of hardware bugs on this exact screen once the
 * Wi-Fi row needed a longer value than a label-left layout leaves room for
 * -- two things kept fighting for the same horizontal space, tuned pixel
 * constant by tuned pixel constant. Fixed architecturally, not with a
 * fourth constant: every info row is now a flex-column block centred on
 * both axes of its row (small title on top, prominent value in the middle,
 * optional smaller detail line at the bottom) -- see make_info_row(). Rows
 * never compete for width with anything any more; LVGL's own flex layout
 * does the centering instead of hand-computed lv_obj_align offsets. The
 * "‹ Back" row is untouched -- it's a nav control, not an information item,
 * still built by the original make_row() and still left-aligned.
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"
#include "dial_list.h"
#include "esp_app_desc.h"
#include "esp_wifi.h"
#include "dial_wifi.h"

#define CY 180
#define ROW_H 76

// Info-row internal vertical rhythm (docs/REPORT-battery-pct-about-
// redesign.md's "block redesign" addendum) -- measured, not guessed:
// lv_font_montserrat_16 (title/detail) lays out 18px tall and
// lv_font_montserrat_24 (value) 27px tall (geometry dump, simulator and
// hardware agree). The tightest rows (Wi-Fi, Battery: title + value +
// detail, 3 lines) stack 18+27+18 = 63px of label content plus two PAD_ROW
// gaps inside ROW_H's 76px. PAD_ROW=4 leaves 76 - 63 - 8 = 5px of slack,
// which the row's flex layout splits above and below the block (see
// make_info_row()), so the detail line no longer sits flush on the row's
// bottom border the way the earlier top-anchored PAD_TOP=3/PAD_ROW=5
// zero-slack fit did. The 2-line rows (Firmware/IDF/Pad) use the same
// constant and centre their 18+4+27 = 49px of content in the same 76px
// box -- ROW_H is shared with the 3-line rows, not shrunk to fit these.
#define INFO_ROW_PAD_ROW 4

static lv_obj_t *s_title_lbl;
static lv_obj_t *s_list;
static lv_obj_t *s_val_wifi, *s_det_wifi;     // "Wi-Fi" row's value/detail (§11.3; block redesign)
static lv_obj_t *s_val_pad;                    // "Pad" row's value (§11.3; was "Serial", app_state_t.serial is a dead field)
static lv_obj_t *s_val_power, *s_det_power;   // "Battery" row's value/detail (docs/SPEC-power-sensing.md §10.4, extended §11.3)

/* ---- row factories --------------------------------------------------- */

// The "‹ Back" row's own factory (scr_settings.c's, ported verbatim) --
// UNCHANGED by the block redesign below: Back is a nav control, not an
// information item, and stays label-left/value-right (the "value" here is
// always empty -- make_row predates having any real use for it on this
// screen, kept only so Back shares this exact row shape/border/sizing with
// every other make_row() caller in the codebase).
static lv_obj_t *make_row(lv_obj_t *parent, const char *label_txt, lv_event_cb_t cb, lv_obj_t **value_out)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), ROW_H);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
    // 36px side insets, not 20: neighbor rows in the rotor rest where the
    // round panel's chord is narrower, and 20 left their ends cropped.
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

// Info-row factory (block redesign): always creates THREE labels, in this
// order -- title, value, detail -- so apply_palette()'s child-index color
// mapping and this row shape never have to special-case which rows use all
// three vs. some (every info row gets all three; unused detail labels just
// stay hidden with empty text, one code path instead of two row variants).
//
// Fonts deliberately SWAP this screen's old label/value sizes rather than
// adding a third size: title (small, secondary) is lv_font_montserrat_16 --
// the font the OLD value label used -- and value (prominent) is
// lv_font_montserrat_24 -- the font the OLD row label used. detail reuses
// title's 16pt. Two sizes total, both already compiled into this build; no
// change to lv_conf.h.
static lv_obj_t *make_info_row(lv_obj_t *parent, const char *title_txt,
                                lv_obj_t **title_out, lv_obj_t **value_out, lv_obj_t **detail_out)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), ROW_H);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(row, 0, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_pad_hor(row, 36, 0);   // same side insets as make_row()'s Back row
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    // Vertically stacked block, centred on BOTH axes. Centring on the main
    // (vertical) axis -- rather than the earlier top-anchored START -- puts
    // the focused row's block on the rotor's own centre line instead of
    // ~12px above it, lands every row's bottom border in the middle of the
    // gap before the next row's title rather than 3px above that title,
    // and gives the 3-line rows the same breathing room top and bottom
    // instead of hugging the bottom border. (The text-to-text gap between
    // two consecutive 2-line rows is the same 26px either way; top-
    // anchoring only moved where the border fell inside that gap.)
    // LV_FLEX_ALIGN_CENTER on the cross (horizontal) axis centres every
    // child -- this, not manual x/y math, is what makes title/value/detail
    // read as one centred block. Vertical padding is pinned to 0 so the
    // slack budget in INFO_ROW_PAD_ROW's comment is the whole story.
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(row, 0, 0);
    lv_obj_set_style_pad_row(row, INFO_ROW_PAD_ROW, 0);

    lv_obj_t *title = lv_label_create(row);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_label_set_text(title, title_txt);

    lv_obj_t *value = lv_label_create(row);
    lv_obj_set_style_text_font(value, &lv_font_montserrat_24, 0);
    lv_label_set_text(value, "");

    lv_obj_t *detail = lv_label_create(row);
    lv_obj_set_style_text_font(detail, &lv_font_montserrat_16, 0);
    lv_label_set_text(detail, "");
    lv_obj_add_flag(detail, LV_OBJ_FLAG_HIDDEN);   // most info rows never use this (Firmware/IDF/Pad)

    // Width/truncation safety, simpler than the old design now that nothing
    // competes for width any more: every label gets the row's own measured
    // content width, centered text, and (value/detail only) LV_LABEL_LONG_DOT
    // as defense-in-depth for a genuinely long SSID or pad address.
    // lv_obj_update_layout() forces the row's LV_PCT(100) width to resolve
    // immediately -- the same idiom this file has used since the one-line
    // Wi-Fi fix, not a new technique.
    lv_obj_update_layout(row);
    lv_coord_t content_w = lv_obj_get_width(row)
                          - lv_obj_get_style_pad_left(row, LV_PART_MAIN)
                          - lv_obj_get_style_pad_right(row, LV_PART_MAIN);
    lv_obj_t *labels[3] = { title, value, detail };
    for (int i = 0; i < 3; i++) {
        lv_obj_set_width(labels[i], content_w);
        lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_label_set_long_mode(value, LV_LABEL_LONG_DOT);
    lv_label_set_long_mode(detail, LV_LABEL_LONG_DOT);

    if (title_out)  *title_out  = title;
    if (value_out)  *value_out  = value;
    if (detail_out) *detail_out = detail;
    return row;
}

/* ---- Wi-Fi row (§11.3: scr_wifi.c's own pattern, verbatim) ---------------*/

// Duplicated from scr_wifi.c's own signal_word(), not shared through a
// header: four lines is lower-risk here than a refactor that couples two
// screens' rows to one signature (see this pass's own SPEC section).
static const char *signal_word(int8_t rssi)
{
    if (rssi >= -60) return "Strong";
    if (rssi >= -70) return "Good";
    return "Weak";
}

// esp_wifi_sta_get_ap_info() is a documented thread-safe getter (scr_wifi.c's
// own header comment) -- safe to call straight from the LVGL task's on_state,
// no new plumbing through dial_state needed.
//
// Block redesign: value = bare SSID, detail = the full signal reading
// (word AND raw dBm, not just the word -- a deliberate decision, not a
// truncation), each its own label now instead of two manually-stacked
// lines sharing one column. No manual realignment needed any more -- flex
// handles vertical position, so this function only ever sets text and
// shows/hides the detail label.
static void render_wifi_row(void)
{
    if (!dial_wifi_is_connected()) {
        lv_label_set_text(s_val_wifi, "Not connected");
        lv_obj_add_flag(s_det_wifi, LV_OBJ_FLAG_HIDDEN);   // clear any stale signal reading
        return;
    }
    wifi_ap_record_t ap;
    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK) {
        lv_label_set_text(s_val_wifi, "Not connected");
        lv_obj_add_flag(s_det_wifi, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    lv_label_set_text(s_val_wifi, (const char *)ap.ssid);
    char detail[32];   // worst case: "Strong (-100 dBm)" (18 chars)
    snprintf(detail, sizeof(detail), "%s (%d dBm)", signal_word(ap.rssi), (int)ap.rssi);
    lv_label_set_text(s_det_wifi, detail);
    lv_obj_clear_flag(s_det_wifi, LV_OBJ_FLAG_HIDDEN);
}

/* ---- Pad row (§11.3: was Serial -- app_state_t.serial is unwritten here) -*/

// scr_settings.c's own "Pad Address" subtitle strips the scheme the same
// way (see that file's render, right before its Bed Mode row) -- matched
// here rather than inventing new formatting. This port's pad addresses are
// always http:// (scr_pad_address.c's Save refuses anything else), but
// https:// is stripped too since nothing stops a future address using it.
static void render_pad_row(void)
{
    char url[DIAL_PAD_URL_MAX_LEN + 1];
    dial_state_get_pad_url(url, sizeof(url));
    const char *p = url;
    if      (strncmp(p, "http://",  7) == 0) p += 7;
    else if (strncmp(p, "https://", 8) == 0) p += 8;
    lv_label_set_text(s_val_pad, p);
}

/* ---- row actions ----------------------------------------------------------*/

// Row 0 on every menu sub-screen (see scr_settings.c): the right-swipe still
// works, but it isn't discoverable on its own.
static void row_back_cb(lv_event_t *e)
{
    (void)e;
    dial_haptics_play(HAPTIC_TICK);
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

/* ---- palette ---------------------------------------------------------------*/

static void apply_palette(lv_obj_t *scr)
{
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);
    lv_obj_set_style_text_color(s_title_lbl, pal->ink_secondary, 0);

    uint32_t n = lv_obj_get_child_cnt(s_list);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *row = lv_obj_get_child(s_list, i);
        lv_obj_set_style_border_color(row, pal->track, 0);
        uint32_t rc = lv_obj_get_child_cnt(row);
        for (uint32_t j = 0; j < rc; j++) {
            lv_obj_t *lbl = lv_obj_get_child(row, j);
            // Info rows (make_info_row(), 3 children: title/value/detail,
            // in that creation order -- confirmed via this pass's own
            // geometry dump, not assumed) color index 1 (value) as the
            // prominent line and 0/2 (title/detail) as supporting text.
            // The Back row (make_row(), 2 children, untouched by this
            // redesign) keeps its own original index-0-is-primary rule --
            // branched on the actual child count rather than a shared
            // index rule, since the two row shapes disagree about which
            // slot is the prominent one.
            bool primary = (rc == 3) ? (j == 1) : (j == 0);
            lv_obj_set_style_text_color(lbl, primary ? pal->ink_primary : pal->ink_secondary, 0);
        }
    }
}

/* ---- vtable ----------------------------------------------------------------*/

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    s_list = dial_list_create(scr, ROW_H);

    make_row(s_list, LV_SYMBOL_LEFT "  Back", row_back_cb, NULL);

    lv_obj_t *fw_val, *idf_val;
    make_info_row(s_list, "Firmware", NULL, &fw_val, NULL);
    make_info_row(s_list, "IDF",      NULL, &idf_val, NULL);
    make_info_row(s_list, "Wi-Fi",    NULL, &s_val_wifi, &s_det_wifi);
    make_info_row(s_list, "Pad",      NULL, &s_val_pad, NULL);
    make_info_row(s_list, "Battery",  NULL, &s_val_power, &s_det_power);

    const esp_app_desc_t *desc = esp_app_get_description();
    char fw[36];
    snprintf(fw, sizeof(fw), "v%s", desc->version);
    lv_label_set_text(fw_val, fw);
    lv_label_set_text(idf_val, desc->idf_ver);
    lv_label_set_text(s_val_wifi, "--");   // filled from on_state (below)
    lv_label_set_text(s_val_pad, "--");    // ditto
    lv_label_set_text(s_val_power, "--");  // ditto — power_src starts UNKNOWN anyway

    // Created AFTER the list so it draws over rows scrolling beneath it.
    // Title slot is every menu sub-screen's shared "64 - CY" convention
    // (scr_settings.c, scr_wifi.c) and has to stay there: the rotor parks
    // row boundaries at y = 66 and y = 142 (dial_list's pad_top is
    // 180 - ROW_H/2 with the focused row centred), so a title at 64 sits in
    // the seam between the two rows above the focused one, where neither
    // row's zoomed-down content reaches. An earlier pass pulled it down to
    // 84 - CY to tighten the gap to Back, which parked it INSIDE the upper
    // row's box -- harmless while Back was that row, but as soon as the
    // list scrolled the info rows' own titles ("IDF", "Pad") rendered
    // straight through "ABOUT" (docs/screens/about-battery-pct.png before
    // this change). Back to the seam.
    s_title_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_title_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_title_lbl, "ABOUT");
    lv_obj_align(s_title_lbl, LV_ALIGN_CENTER, 0, 64 - CY);

    apply_palette(scr);
    dial_list_settle(s_list, 1);   // open on "Firmware", not on Back
}

static void destroy(void)
{
    s_title_lbl = NULL;
    s_list = NULL;
    s_val_wifi = NULL;
    s_det_wifi = NULL;
    s_val_pad = NULL;
    s_val_power = NULL;
    s_det_power = NULL;
}

// docs/SPEC-power-sensing.md §10.4's "the only place the number lives" row,
// extended by §11.2/§11.3 with the percentage, now split into value+detail
// (block redesign) the same shape as the Wi-Fi row above: USB shows a short
// "On USB" value with the voltage as detail; on battery the percentage is
// the prominent value and the voltage is the detail; UNKNOWN shows "--"
// with detail hidden. power_mv/power_pct ride along in the store without
// their own commit (dial_power.c writes both every 1s sample under the
// mutex, no generation bump — see app_state_t.power_mv/power_pct's
// comments), so this reads whatever value happened to be current the last
// time ANY commit landed and re-ran on_state here — exactly §10.3's "About
// reads it on on_state, so the row refreshes whenever anything else
// changes". No separate timer: this screen already has an on_state (the
// Wi-Fi/Pad rows above), so that mechanism does the refreshing.
static void render_power_row(const app_state_t *st)
{
    char val[16], det[24];
    bool show_det;
    switch (st->power_src) {
    case PWR_PLUGGED:
        strlcpy(val, "On USB", sizeof(val));
        snprintf(det, sizeof(det), "%.2f V", st->power_mv / 1000.0f);
        show_det = true;
        break;
    case PWR_BATTERY:
        snprintf(val, sizeof(val), "%d%%", (int)st->power_pct);
        snprintf(det, sizeof(det), "%.2f V", st->power_mv / 1000.0f);
        show_det = true;
        break;
    default:
        strlcpy(val, "--", sizeof(val));
        det[0] = '\0';
        show_det = false;
        break;
    }
    lv_label_set_text(s_val_power, val);
    lv_label_set_text(s_det_power, det);
    if (show_det) lv_obj_clear_flag(s_det_power, LV_OBJ_FLAG_HIDDEN);
    else          lv_obj_add_flag(s_det_power, LV_OBJ_FLAG_HIDDEN);
}

static void on_state(const app_state_t *st)
{
    if (!s_list) return;
    apply_palette(lv_obj_get_parent(s_list));
    render_wifi_row();
    render_pad_row();
    render_power_row(st);
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
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    return true;
}

const ui_screen_t scr_about = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_knob = on_knob, .on_gesture = on_gesture,
};
