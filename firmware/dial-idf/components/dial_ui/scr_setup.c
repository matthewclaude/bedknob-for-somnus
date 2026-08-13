/*
 * SCR_WIFI_PORTAL — how to give the dial your Wi-Fi. (This file used to also
 * hold SCR_OAUTH_QR, the Orion account-linking QR step; removed along with
 * the rest of the OAuth/MCP pipeline — see components/dial_somnus. Somnus's
 * local pad API needs no interactive consent step at all.)
 */
#include "ui_screens_internal.h"
#include "dial_haptics.h"

/* ---- Wi-Fi setup ------------------------------------------------------- */
/*
 * This screen used to show a WIFI: QR code, so the camera could join the dial's
 * SoftAP in one tap. It is gone, deliberately.
 *
 * iOS fails that join outright the first time on a network it hasn't recently
 * scanned — reported for years, never explained by Apple, and no AP-side
 * setting fixes it (a manual tap on the SAME AP seconds later works). Espressif
 * say as much themselves: with SoftAP provisioning on iOS, "discovery as well as
 * connection APIs are not available", so the trip to Settings isn't a fallback,
 * it IS the supported path. The QR was trying to skip a step the platform
 * doesn't let you skip, and turned a reliable ten-second task into a minute of
 * failure. No shipping consumer product does it this way.
 *
 * So: name the network, say where to tap, and offer the way out for anyone who
 * would rather not involve a phone at all.
 */

static lv_obj_t *s_wifi_ssid_lbl, *s_wifi_step_lbl, *s_wifi_hint_lbl, *s_wifi_btn, *s_wifi_btn_lbl;

static void wifi_on_dial_cb(lv_event_t *e)
{
    (void)e;
    dial_haptics_play(HAPTIC_TICK);
    ui_router_go(SCR_NETPICK, NULL, LV_SCR_LOAD_ANIM_MOVE_LEFT);
}

static void wifi_create(lv_obj_t *scr, void *arg)
{
    (void)arg;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);

    s_wifi_step_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_wifi_step_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_width(s_wifi_step_lbl, 280);
    lv_label_set_long_mode(s_wifi_step_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(s_wifi_step_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(s_wifi_step_lbl, "On your phone, open\nSettings " LV_SYMBOL_RIGHT " Wi-Fi\nand tap");
    lv_obj_align(s_wifi_step_lbl, LV_ALIGN_CENTER, 0, 96 - 180);

    // The one thing they have to find. Big enough to read from across the room,
    // and it is the whole instruction.
    s_wifi_ssid_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_wifi_ssid_lbl, &lv_font_montserrat_24, 0);
    lv_obj_set_width(s_wifi_ssid_lbl, 300);
    lv_label_set_long_mode(s_wifi_ssid_lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_align(s_wifi_ssid_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(s_wifi_ssid_lbl, "...");
    lv_obj_align(s_wifi_ssid_lbl, LV_ALIGN_CENTER, 0, 166 - 180);

    s_wifi_hint_lbl = lv_label_create(scr);
    lv_obj_set_style_text_font(s_wifi_hint_lbl, &lv_font_montserrat_12, 0);
    lv_label_set_text(s_wifi_hint_lbl, "A setup page opens by itself");
    lv_obj_align(s_wifi_hint_lbl, LV_ALIGN_CENTER, 0, 200 - 180);

    // The no-phone path.
    s_wifi_btn = dial_btn_create(scr);
    lv_obj_set_size(s_wifi_btn, 240, 72);
    lv_obj_set_style_radius(s_wifi_btn, 36, 0);
    lv_obj_set_style_border_width(s_wifi_btn, 1, 0);
    lv_obj_align(s_wifi_btn, LV_ALIGN_CENTER, 0, 272 - 180);
    lv_obj_add_event_cb(s_wifi_btn, wifi_on_dial_cb, LV_EVENT_CLICKED, NULL);

    s_wifi_btn_lbl = lv_label_create(s_wifi_btn);
    lv_obj_set_style_text_font(s_wifi_btn_lbl, &lv_font_montserrat_16, 0);
    lv_label_set_text(s_wifi_btn_lbl, "Set up on the dial");
    lv_obj_center(s_wifi_btn_lbl);
}

static void wifi_destroy(void)
{
    s_wifi_ssid_lbl = s_wifi_step_lbl = s_wifi_hint_lbl = NULL;
    s_wifi_btn = s_wifi_btn_lbl = NULL;
}

static void wifi_on_state(const app_state_t *st)
{
    if (!s_wifi_ssid_lbl) return;
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(lv_obj_get_parent(s_wifi_ssid_lbl), pal->bg, 0);
    lv_obj_set_style_text_color(s_wifi_step_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_text_color(s_wifi_ssid_lbl, pal->ink_primary, 0);
    lv_obj_set_style_text_color(s_wifi_hint_lbl, pal->ink_secondary, 0);
    lv_obj_set_style_bg_color(s_wifi_btn, pal->surface, 0);
    lv_obj_set_style_border_color(s_wifi_btn, pal->track, 0);
    lv_obj_set_style_text_color(s_wifi_btn_lbl, pal->ink_primary, 0);

    if (st->ap_ssid[0]) lv_label_set_text(s_wifi_ssid_lbl, st->ap_ssid);
}

const ui_screen_t scr_wifi_portal = {
    .create = wifi_create, .destroy = wifi_destroy, .on_state = wifi_on_state,
};
