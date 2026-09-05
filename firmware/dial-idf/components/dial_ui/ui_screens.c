#include "ui_screens.h"
#include "ui_screens_internal.h"

void ui_screens_register_all(void)
{
    ui_router_register(SCR_CONNECTING, &scr_connecting);
    ui_router_register(SCR_ERROR, &scr_connecting);   // same rendering, distinct id
    ui_router_register(SCR_PAD_DISCOVERY, &scr_pad_discovery);
    ui_router_register(SCR_WIFI_PORTAL, &scr_wifi_portal);
    ui_router_register(SCR_NETPICK, &scr_netpick);
    ui_router_register(SCR_PASSKEY, &scr_passkey);
    ui_router_register(SCR_DIAL, &scr_dial);
    ui_router_register(SCR_MENU, &scr_menu);
    ui_router_register(SCR_STANDBY, &scr_standby);
    ui_router_register(SCR_WELCOME, &scr_welcome);
    ui_router_register(SCR_SIDEPICK, &scr_sidepick);
    ui_router_register(SCR_SETTINGS, &scr_settings);
    ui_router_register(SCR_TIMEZONE, &scr_timezone);
    ui_router_register(SCR_PAD_ADDRESS, &scr_pad_address);
    ui_router_register(SCR_ADJUST_MODE, &scr_adjust_mode);
    ui_router_register(SCR_BRIGHTNESS_MENU, &scr_brightness_menu);
    ui_router_register(SCR_BRIGHTNESS, &scr_brightness);
    ui_router_register(SCR_NIGHT_MODE, &scr_night_mode);
    ui_router_register(SCR_NIGHT_FACE, &scr_night_face);
    ui_router_register(SCR_STANDBY_FACE, &scr_standby_face);
    ui_router_register(SCR_WIFI, &scr_wifi);
    ui_router_register(SCR_ABOUT, &scr_about);
    ui_router_register(SCR_UPDATE, &scr_update);
    ui_router_register(SCR_UPDATING, &scr_updating);
    ui_router_register(SCR_UPDATE_PROMPT, &scr_update_prompt);
}
