#pragma once
#include "dial_state.h"

/*
 * sim_state.c implements the dial_state.h API the real components/dial_state
 * (FreeRTOS mutex + NVS + esp_timer) provides, but as a single-threaded
 * global over one app_state_t — there is no worker task here, so "the store
 * mutex" is just "the only thread there is."
 *
 * This header adds ONE thing the real dial_state.h doesn't: direct access to
 * the live struct, so main.c's scenarios can set fields no setter in the
 * dial_ui-facing API covers (zones[], have_state, serial, ap_ssid,
 * ota, ...) before calling ui_router_go. Screens never see this header —
 * only main.c does.
 */
app_state_t *sim_state_ptr(void);

// Reset to a blank-but-valid baseline (zero, generation 0). main.c calls this
// between scenarios so nothing bleeds across screenshots.
void sim_state_reset(void);

// Test-only override for esp_wifi_sta_get_ap_info()'s fake result (stubs.c
// normally always returns FAKE_SCAN[0], "Home"/-48) -- lets a scenario render
// an SSID/RSSI combination beyond that fixed default, e.g. a worst-case
// 32-char SSID for a layout check. Pass ssid=NULL to go back to FAKE_SCAN[0].
void sim_set_fake_ap(const char *ssid, int8_t rssi);

// Test-only override for dial_time_get_iana_tz() (stubs.c normally returns
// false -- "no zone ever set", the honest fresh-device state). Lets one
// scenario render Settings' Timezone row with a raw IANA name the curated
// picker doesn't offer (the 2026-09-04 layout audit's §2b case). Pass NULL
// to go back to the default. dial_time_valid() stays false either way: the
// hook fakes a persisted zone, not a synced clock.
void sim_set_fake_iana_tz(const char *iana);
