/*
 * SCR_CONNECTING — boot/progress text, driven entirely by the connection
 * phase. Also serves PH_DEGRADED with the last error + retry countdown.
 * SCR_ERROR shares this implementation (registered separately for clarity
 * of navigation intent).
 */
#include "ui_screens_internal.h"

static lv_obj_t *s_label;
static lv_obj_t *s_sub;

// Palette tokens, not the pure-black / fixed-grey literals this screen used
// to carry: every other screen paints pal->bg, so boot (this screen -> dial)
// and every PH_DEGRADED transition flashed from #000000 to the chassis
// colour, and at night the fixed light grey ignored the ember palette's
// blue-channel rule (docs/REPORT-screen-layout-audit.md §13). Re-applied
// from on_state so a night swap takes effect on the next render, same as
// everywhere else.
static void apply_palette(lv_obj_t *scr)
{
    const dial_palette_t *pal = PAL();
    lv_obj_set_style_bg_color(scr, pal->bg, 0);
    lv_obj_set_style_text_color(s_label, pal->ink_primary, 0);
    lv_obj_set_style_text_color(s_sub, pal->ink_secondary, 0);
}

static void create(lv_obj_t *scr, void *arg)
{
    (void)arg;

    // Block offsets -24 / +24 (were -12 / +28): PH_DEGRADED's subtitle wraps
    // to 4 lines (72px) with a realistic pad error + "Retrying in Ns" +
    // "Swipe left for menu", and at -12/+28 the main label's box (169-191)
    // overlapped the subtitle's top (172) by 7px while the whole block sat
    // 20px below the panel centre. At -24/+24 the 4-line case is main
    // 145-167, sub 168-240: no box overlap, block centred at 192.
    s_label = lv_label_create(scr);
    lv_obj_set_width(s_label, 300);
    lv_label_set_long_mode(s_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(s_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_label, &lv_font_montserrat_20, 0);
    lv_obj_align(s_label, LV_ALIGN_CENTER, 0, -24);
    lv_label_set_text(s_label, "");

    s_sub = lv_label_create(scr);
    // 320, not 300: the DIAL_CERT_ERR_MSG repo line measures ~300px at this
    // font, and the round bezel has headroom to spare at this label's y
    // offset (a 320-wide chord still clears it), so the extra 20px is cheap
    // margin against an unwanted mid-word wrap rather than a tight fit.
    lv_obj_set_width(s_sub, 320);
    lv_label_set_long_mode(s_sub, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(s_sub, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(s_sub, &lv_font_montserrat_16, 0);
    lv_obj_align(s_sub, LV_ALIGN_CENTER, 0, 24);
    lv_label_set_text(s_sub, "");

    apply_palette(scr);
}

static void destroy(void) { s_label = s_sub = NULL; }

static void on_state(const app_state_t *st)
{
    if (!s_label) return;
    apply_palette(lv_obj_get_parent(s_label));
    const char *main_txt = "";
    char sub_txt[160] = "";
    lv_color_t main_color = PAL()->ink_primary;   // this screen's usual tone; PH_DEGRADED overrides

    switch (st->phase) {
    case PH_BOOT:              main_txt = "Starting up..."; break;
    // PH_WIFI_PORTAL lands here only in the beat between the dial handing over
    // credentials and the provisioning task picking them up — the instructions
    // screen owns that phase otherwise. It used to fall through to the default
    // "...", which is what the user saw after typing a password: nothing.
    case PH_WIFI_PORTAL:
    case PH_WIFI_CONNECTING:
        main_txt = "Connecting to Wi-Fi...";
        // Name the network when we know it. Sitting on a bare "..." while the
        // dial silently tries a password you just typed is the worst possible
        // moment to say nothing.
        if (st->wifi_join_ssid[0])
            snprintf(sub_txt, sizeof(sub_txt), "%s", st->wifi_join_ssid);
        break;
    case PH_WIFI_LOST:
        main_txt = "Wi-Fi lost";
        snprintf(sub_txt, sizeof(sub_txt), "Reconnecting...");
        break;
    case PH_SOMNUS_CONNECTING: main_txt = "Connecting to your bed..."; break;
    case PH_DEGRADED: {
        // dial_somnus talks plain local HTTP (no TLS -- see dial_somnus.h),
        // so there is no cert-classified failure mode to special-case here
        // the way the old Orion/OAuth/MCP pipeline needed (DIAL_CERT_ERR_MSG
        // is still defined in dial_state.h for whatever sets phase_err
        // verbatim, but nothing in main.c does anymore).
        main_txt = "Pad unreachable";
        // Night-quiet errors (design-spec.md's "silent staleness at night"):
        // dim to ink_secondary instead of a bright warning tone at 3am.
        main_color = dial_palette_is_night() ? PAL()->ink_secondary : PAL()->warning;
        // phase_err carries the specific reason (dial_somnus_last_error());
        // only echo it in the subtitle when it adds something the headline
        // doesn't already say.
        bool echo = st->phase_err[0] && strcmp(st->phase_err, main_txt) != 0;
        const char *why = echo ? st->phase_err : "";
        const char *nl  = echo ? "\n" : "";
        if (st->retry_in_s > 0)
            snprintf(sub_txt, sizeof(sub_txt), "%s%sRetrying in %ds", why, nl, st->retry_in_s);
        else
            snprintf(sub_txt, sizeof(sub_txt), "%s%sRetrying...", why, nl);
        break;
    }
    default:                   main_txt = "..."; break;
    }
    // Pending-verify notice: this boot is a fresh OTA install the bootloader
    // will roll back on the next power-cycle unless it survives long enough
    // to confirm (main.c's ota_confirm_once(), either the first successful
    // poll or a 30s fallback timer). This is exactly the screen a user stares
    // at right after that reboot, so it's the one place this warning matters
    // most. Appended (not on PH_DEGRADED — that branch above already owns its
    // own honest headline/subtitle and this must not disturb it) rather than
    // replacing whatever the phase already says, since "Connecting to
    // Wi-Fi..." / the SSID / "Reconnecting..." are still true and useful.
    if (st->phase != PH_DEGRADED && st->ota.pending_verify) {
        size_t len = strlen(sub_txt);
        snprintf(sub_txt + len, sizeof(sub_txt) - len, "%sFinalizing update - keep powered",
                 len ? "\n" : "");
    }
    // Degraded is where a user might be stuck for a while: point at the
    // menu escape hatch (nav_policy keeps a deliberately opened menu/
    // settings/about sticky even without device state) so Re-link, Wi-Fi
    // change, and software update are always reachable from a sick dial.
    if (st->phase == PH_DEGRADED) {
        size_t len = strlen(sub_txt);
        snprintf(sub_txt + len, sizeof(sub_txt) - len, "%sSwipe left for menu",
                 len ? "\n" : "");
    }
    lv_obj_set_style_text_color(s_label, main_color, 0);
    lv_label_set_text(s_label, main_txt);
    lv_label_set_text(s_sub, sub_txt);
}

// The escape hatch itself: a horizontal swipe (same gesture that walks faces
// on a healthy dial) opens the menu even while connecting/degraded.
static bool on_gesture(lv_dir_t dir)
{
    if (dir != LV_DIR_LEFT && dir != LV_DIR_RIGHT) return false;
    ui_router_go(SCR_MENU, NULL, LV_SCR_LOAD_ANIM_MOVE_LEFT);
    return true;
}

const ui_screen_t scr_connecting = {
    .create = create, .destroy = destroy, .on_state = on_state,
    .on_gesture = on_gesture,
};
