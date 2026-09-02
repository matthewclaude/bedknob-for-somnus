#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/*
 * Wall-clock time for the dial: SNTP sync (esp_netif_sntp) + IANA -> POSIX TZ
 * resolution via an embedded copy of the posix_tz_db zones table, persisted
 * in NVS (ns "time") so the dial keeps showing local time across reboots
 * even before the account's timezone is (re)confirmed.
 */

// Start SNTP (esp_netif_sntp, pool.ntp.org) non-blocking, and restore the
// persisted POSIX TZ from NVS if present. Call once after Wi-Fi is up.
void dial_time_start(void);

// True once SNTP has synced at least once this boot AND a TZ is set.
bool dial_time_valid(void);

// Map an IANA zone name (e.g. "America/Denver") to a POSIX TZ string via the
// embedded table, apply it (setenv TZ + tzset), persist it in NVS (ns "time").
// Returns false if the name is unknown (leaves current TZ).
//
// Threading: this mutates global libc TZ state that dial_time_now() (below)
// reads via localtime_r() -- safe to call before dial_time_start() has run
// (nothing is reading TZ state yet, e.g. the Wi-Fi setup portal handler),
// but NOT safe to call from a task that might run concurrently with a
// dial_time_now() reader once the app is at steady state (e.g. the LVGL
// task, live at the same time worker_task is polling the pad). A caller in
// that situation must route through the UI->worker command queue and let
// worker_task make this call, the same task every dial_time_now() caller
// already runs on. See docs/SPEC-timezone-source.md's Threading section.
bool dial_time_set_iana_tz(const char *iana);

// Copy out the IANA zone name last successfully passed to
// dial_time_set_iana_tz() -- persisted in NVS (ns "time") alongside the POSIX
// string it resolved to, restored at dial_time_start(), so this survives a
// reboot. Returns false (leaving *out untouched) if no zone has ever been
// set on this device -- a real, honest "Not set" state, not an empty string
// standing in for it. Safe from any task: a plain copy of a small buffer,
// same risk profile as dial_net's existing cross-task string getters
// (dial_net_sta_ssid() etc.) -- no mutex, none of dial_net's getters have
// one either.
bool dial_time_get_iana_tz(char *out, size_t sz);

// Current local time, or false if not yet valid.
bool dial_time_now(struct tm *out);
