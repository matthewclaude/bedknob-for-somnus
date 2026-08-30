/*
 * Somnus dial — app wiring.
 *
 * Structure:
 *   dial_display  LCD/touch/LVGL bring-up + the LVGL task and lock
 *   dial_state    snapshot store + UI->worker command queue + input stamp
 *   dial_ui       screen router (LVGL task) + screens
 *   worker_task   (here) the single network task: Wi-Fi -> Somnus pad
 *                 connect -> command/poll loop, as a supervisor state
 *                 machine. Every failure is a phase + backoff, never a dead
 *                 end. Replaces the earlier Orion OAuth->MCP pipeline —
 *                 dial_somnus (components/dial_somnus) talks a plain local
 *                 JSON REST API straight to the pad, no cloud/auth involved.
 *
 * Threading: the worker never touches LVGL; it commits to dial_state and the
 * router's dispatcher renders. Knob callbacks (esp_timer task) only feed the
 * router's atomic accumulator. LVGL event callbacks already hold the LVGL
 * mutex and must not re-lock it.
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "dial_display.h"
#include "dial_state.h"
#include "ui_router.h"
#include "ui_screens.h"
#include "dial_wifi.h"
#include "dial_somnus.h"
#include "dial_time.h"
#include "dial_haptics.h"
#include "dial_power.h"
#include "dial_palette.h"
#include "dial_ota.h"
#include "bidi_switch_knob.h"
// secrets.h is an optional dev convenience (git-ignored) that pre-seeds Wi-Fi
// creds so a developer build skips on-device provisioning; a fresh clone has
// none, and WIFI_SSID falls back to dial_net_seed's own placeholder string so
// the seed is a provable no-op and the dial provisions on-device as normal.
#if __has_include("secrets.h")
#include "secrets.h"
#else
#define WIFI_SSID     "your-wifi-ssid"
#define WIFI_PASSWORD ""
#endif

static const char *TAG = "app";

// Device resync is gated on a quiet period: every user input stamps the store,
// and the poll only reads the bed back once there's been no input for a while
// (so an update can never land mid-interaction).
#define KNOB_SETTLE_US    2500000    // 2.5s of no input before the bed is read back
#define POLL_INTERVAL_US 10000000    // and at most every ~10s when idle
/*
 * ...but the interval is not one number. Right after a write the device is the
 * thing that's changing (the bed takes a few seconds to actually start heating
 * or cooling), so a 10s cadence means the dial insists nothing is happening
 * long after the user can hear the pump. Poll hard for a few rounds, then fall
 * back to idle cadence. The quiet gate above still holds the whole time, so a
 * knob spin is never interrupted by a read.
 */
#define POLL_CONFIRM_US   2000000    // 2s between the confirm polls after a write
#define POLL_CONFIRM_N          3    // how many of them before returning to idle cadence

// Auto OTA check (M6): once per uptime-day, and only checks (never applies)
// — see the gating comment at its call site for the full safe-window rule.
// 6h, not 24h (owner asked what it costs to check more often, 2026-08-04).
// One check is a single GitHub request against a 60/hour per-IP limit, so
// frequency is free on that side; the only real cost is briefly handing the
// TLS session over from the Somnus client, which is noise a few times a day.
// The old 24h interval combined with a narrow daytime band meant a release
// took up to a full day to be noticed — the band, not the interval, was the
// binding constraint (see the window gate below, now widened to "any time
// outside the sleep window").
#define OTA_AUTOCHECK_INTERVAL_US (6LL * 60 * 60 * 1000000)

// OTA_FAILED is not allowed to be terminal (field bug: it used to stay
// wedged until a manual power cycle) — see dial_ota_clear_stale_failure()'s
// comment. This is the time-based half of that fix; CMD_OTA_CLEAR_FAILED
// (scr_settings.c's screen teardown) is the immediate half.
#define OTA_FAILED_AUTOCLEAR_US (25LL * 1000000)

#define BACKOFF_MIN_S  5
#define BACKOFF_MAX_S 60

/* ---- rotary knob ------------------------------------------------------ */
// GPIO8 (A) / GPIO7 (B); only electrically live under the full board init.
// Callbacks run in the decoder's esp_timer task: feed the router's atomic
// accumulator and nothing else (blocking here stalls lv_tick_inc).
#define KNOB_A 8
#define KNOB_B 7
static knob_handle_t s_knob;

// No haptic per detent: the encoder is mechanically detented, so a motor
// pulse on top of each click is redundant and reads as noise (owner,
// 2026-07-29). This is where that pulse used to fire -- in the decoder's own
// task, ahead of the router -- which is why muting the router's dispatch
// alone did not silence it. Range-end feedback survives, but it is raised by
// the screens themselves via dial_haptics_play_soft().
//
// A detent arriving in standby wakes the screen and is consumed — a 3am
// reach must not change the temp.
static void knob_step(int dir)
{
    dial_state_stamp_input();
    if (dial_power_wake_consumes()) return;
    ui_router_knob_input(dir);
}
static void knob_left_cb(void *arg, void *data)  { (void)arg; (void)data; knob_step(-1); }
static void knob_right_cb(void *arg, void *data) { (void)arg; (void)data; knob_step(+1); }

// Touch filter (LVGL task, every 20ms sample): any contact stamps activity;
// the press that wakes a standby screen is swallowed end-to-end (LVGL never
// sees it, so it can't click a button or drag the arc).
static bool touch_filter(bool pressed)
{
    static bool s_consuming;
    if (!pressed) {
        s_consuming = false;
        return false;
    }
    dial_state_stamp_input();
    if (s_consuming) return true;
    if (dial_power_wake_consumes()) {
        s_consuming = true;   // swallow until release
        return true;
    }
    return false;
}

static void knob_init(void)
{
    knob_config_t cfg = { .gpio_encoder_a = KNOB_A, .gpio_encoder_b = KNOB_B };
    s_knob = iot_knob_create(&cfg);
    if (!s_knob) { ESP_LOGE(TAG, "knob create failed"); return; }
    iot_knob_register_cb(s_knob, KNOB_LEFT, knob_left_cb, NULL);
    iot_knob_register_cb(s_knob, KNOB_RIGHT, knob_right_cb, NULL);
    ESP_LOGI(TAG, "knob ready on GPIO%d/%d", KNOB_A, KNOB_B);
}

/* ---- navigation policy (runs in the LVGL task) ------------------------ */

static screen_id_t nav_policy(const app_state_t *st, void **arg)
{
    // OTA install takeover (M6 UX hardening): once the confirmed install on
    // SCR_UPDATE actually starts pulling bytes, lock the user onto a
    // dedicated full-screen progress ring until the device reboots (success
    // — dial_ota_download_and_apply()'s esp_restart() never returns, so this
    // never even gets a chance to route away) or the install fails. Checked
    // ahead of everything else, the same forced-navigation trick the welcome
    // splash below uses, so a routine poll or phase blip can never surface a
    // normal screen mid-download. READY_REBOOT (image written + verified,
    // about to restart) stays on the same screen — it's the tail of this
    // same flow, not a new one.
    if (st->ota.status == OTA_DOWNLOADING || st->ota.status == OTA_READY_REBOOT) {
        // An UNATTENDED install on a sleeping dial stays dark: nobody asked
        // for it, and lighting a bedside screen for two minutes to narrate an
        // update nobody is watching is exactly the interruption this
        // firmware's night handling exists to avoid. If the display is awake
        // — the user is present, or wakes it mid-install — the takeover still
        // wins, because a dial that reboots under someone's hands with no
        // explanation is worse than the interruption.
        // ...and once it IS showing, it stays for the rest of the install.
        // Without the ui_router_current() clause an unattended install that a
        // user woke, looked at, and walked away from would drop back to the
        // clock face mid-download and then reboot with no explanation.
        if (!st->ota.unattended || dial_power_level() != DPWR_STANDBY ||
            ui_router_current() == SCR_UPDATING)
            return SCR_UPDATING;
    }
    // Status moved off the takeover's two states while we were still showing
    // it — only OTA_FAILED does this in practice (the stale-tap guard on
    // CMD_OTA_APPLY keeps a race from reaching here any other way). Back to
    // Update (M7: moved off scr_about.c), where the stacked error line under
    // the row says why.
    if (ui_router_current() == SCR_UPDATING)
        return SCR_UPDATE;

    // Onboarding (M4): a genuinely fresh device (no Wi-Fi creds at boot; see
    // app_main) parks on the welcome splash through the earliest connection
    // phases until the user acknowledges it (tap/knob -> dial_state_set_
    // welcomed). Checked ahead of the phase switch below so it also
    // pre-empts PH_WIFI_PORTAL's own screen (join-the-AP QR) — the welcome
    // screen is meant to be seen first, then dismissed into that QR.
    if (st->fresh_device && !st->welcomed &&
        (st->phase == PH_BOOT || st->phase == PH_WIFI_CONNECTING || st->phase == PH_WIFI_PORTAL))
        return SCR_WELCOME;

    switch (st->phase) {
    case PH_WIFI_PORTAL: {
        // A password the router rejected sends the user straight back to the
        // password screen FOR THE SAME NETWORK, which is the only place they can
        // do anything about it. Landing them at the start of setup — as this
        // did — makes them re-pick the network to fix a typo.
        if (st->wifi_join_failed && st->wifi_join_idx >= 0) {
            *arg = (void *)(uintptr_t)st->wifi_join_idx;
            return SCR_PASSKEY;
        }
        // Setting up on the dial (network list, password entry) is a deliberate
        // journey inside this phase — a routine state commit must not yank the
        // user back to the instructions screen halfway through typing.
        //
        // SCR_CONNECTING is deliberately NOT in this set: it is also the boot
        // screen, so it is "current" every time the portal first comes up, and
        // making it sticky here pinned the UI on "Connecting to Wi-Fi..." while
        // the portal ran underneath, forever. The connect attempt shows the
        // connecting screen through the PHASE instead (PH_WIFI_CONNECTING),
        // which the passkey screen sets before it hands the credentials over.
        screen_id_t cur = ui_router_current();
        if (cur == SCR_NETPICK || cur == SCR_PASSKEY) return cur;
        return SCR_WIFI_PORTAL;
    }
    case PH_READY:
    case PH_DEGRADED:
    case PH_WIFI_LOST:
        // Once we have device state, stay on the dial (with its staleness dot)
        // through transient outages rather than yanking the user to a status
        // screen mid-interaction.
        if (st->have_state) {
            // Boost, the Schedule/Hold picker, and settings are transient
            // overlays reached by a deliberate action; a routine state
            // commit (poll landing, night-mode flip, ...) must not yank the
            // user back to the dial mid-flow. Returning the current screen
            // unchanged is a no-op in ui_router_go (same id + same arg), so
            // this is safe every tick.
            screen_id_t cur = ui_router_current();

            // Update prompt (docs/SPEC-update-prompt.md, reworked): the
            // worker raises ota_prompt_due on a WAKE EDGE (STANDBY/DIMMED ->
            // ACTIVE) when every entry gate holds at that instant, not a
            // continuously re-evaluated condition -- the earlier design
            // re-checked its gates every idle tick, so a touch (which resets
            // idle time) withdrew the sheet mid-reach. The worker itself
            // still withdraws the offer, but only on one of three explicit
            // exit checks (display back to STANDBY, night begins, update no
            // longer available -- see that gate's own "exit" comment), so it
            // survives being touched. When it does withdraw, this falls
            // through to the normal fallback below and lands back on
            // SCR_DIAL/SCR_STANDBY same as any other abandoned sub-screen.
            // ENTRY is scoped to SCR_DIAL specifically -- not Settings/Menu/
            // Wi-Fi/etc. -- so a routine background flag can never yank the
            // user out of a screen they navigated to on purpose (the same
            // restraint every other branch below already applies to a
            // routine poll landing); cur == SCR_UPDATE_PROMPT keeps it
            // sticky once shown, same shape as QUICK/BOOST/SETTINGS below,
            // so a poll landing mid-decision can't yank it away either.
            if (st->ota_prompt_due && (cur == SCR_DIAL || cur == SCR_UPDATE_PROMPT)) {
                *arg = (void *)(uintptr_t)st->ui_zone;
                return SCR_UPDATE_PROMPT;
            }

            // The menu face and its passive sub-screens (WIFI/ABOUT/UPDATE)
            // are reached by swipe/tap and join the sticky set below, but
            // unlike QUICK/BOOST/SETTINGS/BRIGHTNESS_MENU/ADJUST_MODE (which
            // only leave via a deliberate user action) they're also
            // dismissed by the standby idle timeout — someone can swipe
            // there and fall asleep on it — so that check must win over
            // stickiness, checked BEFORE folding them into the sticky-set
            // return. UPDATE joined this set at M7 (moved off ABOUT, which
            // was already here) — it's the one sub-screen where getting
            // yanked away mid-check/mid-confirm by a routine poll commit
            // would be user-visibly broken, not just an inconvenience.
            bool passive = cur == SCR_MENU ||
                           cur == SCR_WIFI || cur == SCR_ABOUT || cur == SCR_UPDATE;
            if (passive && dial_power_level() == DPWR_STANDBY) {
                *arg = (void *)(uintptr_t)st->ui_zone;
                return SCR_STANDBY;
            }
            // ADJUST_MODE joins BRIGHTNESS_MENU here (not the idle-dismissed
            // passive set above): both are Settings sub-screens reached by a
            // deliberate tap, and a routine poll landing mid-choice must not
            // yank the user off either one.
            if (passive || cur == SCR_SETTINGS ||
                cur == SCR_BRIGHTNESS_MENU || cur == SCR_ADJUST_MODE ||
                cur == SCR_PAD_ADDRESS) return cur;
            // First link on a fresh device: pick a default side before showing
            // the dial (SCR_SIDEPICK). Nothing to pick on a single-zone topper,
            // so that device goes straight to its one face. The `cur` half of
            // the OR keeps a poll from yanking the user off the picker
            // mid-decision.
            if (dial_state_is_dual(st) &&
                ((st->fresh_device && !st->side_picked) || cur == SCR_SIDEPICK))
                return SCR_SIDEPICK;
            *arg = (void *)(uintptr_t)st->ui_zone;
            return dial_power_level() == DPWR_STANDBY ? SCR_STANDBY : SCR_DIAL;
        }
        // Never trap the user (field incident 2026-07-28): with no device
        // state the connect/error screen used to own the display outright,
        // making Settings' Re-link, Wi-Fi change, and About's update —
        // the only tools that FIX a stuck dial — unreachable. A deliberately
        // opened menu face or sub-screen stays put; scr_connecting offers
        // the swipe that gets there.
        {
            screen_id_t cur = ui_router_current();
            if (cur == SCR_MENU || cur == SCR_SETTINGS || cur == SCR_ABOUT ||
                cur == SCR_WIFI || cur == SCR_BRIGHTNESS ||
                cur == SCR_BRIGHTNESS_MENU || cur == SCR_UPDATE ||
                cur == SCR_PAD_ADDRESS)
                return cur;
        }
        return st->phase == PH_READY ? SCR_CONNECTING : SCR_ERROR;
    default:                    return SCR_CONNECTING;
    }
}

/* ---- store mutators (file-scope: no GCC nested-fn trampolines) --------- */

typedef struct {
    zone_state_t zones[ZONE_COUNT];
    bool present[ZONE_COUNT];  // sides actually in play this poll (see dial_somnus_get_zone_mode)
    bool online;
    bool system_error;
    int64_t poll_started_us;   // when the /api/state round-trip began
} device_snapshot_t;

static void mut_device_state(app_state_t *st, void *arg)
{
    device_snapshot_t *d = arg;
    // A response that LEFT the pad before the user's last input cannot know
    // about what they just did. Taking its control fields (on/setpoint)
    // would visibly undo the optimistic update for a beat and then redo it
    // on the following poll — the "jumpy" flicker after a tap or a knob
    // turn. Its telemetry (measured water, water-low) is still good: the
    // user's input didn't change those, so only the control state is held.
    bool predates_input = dial_state_last_input_us() > d->poll_started_us;

    for (int z = 0; z < ZONE_COUNT; z++) {
        zone_state_t keep = st->zones[z];
        st->zones[z] = d->zones[z];
        if (predates_input) {                       // see the note above
            st->zones[z].on      = keep.on;
            st->zones[z].temp_dc = keep.temp_dc;
        }
    }
    for (int z = 0; z < ZONE_COUNT; z++) st->zone_present[z] = d->present[z];
    // A single-zone bed (Somnus "One Bed" mode) has no partner face to show.
    // If the persisted side names a zone this device doesn't have (a dial
    // moved between beds, a zone-mode flip, or an NVS value from before this
    // field existed), fall back to the one it does — otherwise nav_policy
    // would keep routing to a face built from an empty zone.
    if (!st->zone_present[st->ui_zone])
        st->ui_zone = dial_state_primary_zone(st);
    st->device_online = d->online;
    st->system_error = d->system_error;
    st->have_state = true;
    // Clear optimistic intent ONLY if no input arrived after this poll's
    // round-trip began (the failed-write case still converges: the next
    // quiet-period poll runs with no newer input and clears the stale intent).
    if (!predates_input)
        for (int z = 0; z < ZONE_COUNT; z++)
            st->ui_temp_dc[z] = -1;
}

// temp_min_dc/temp_max_dc: the pad's fixed range (local_api spec, dial_somnus.h
// -- no discovery call reports it, unlike Orion's list_devices), seeded once
// after a successful dial_somnus_connect(). See worker_task's call site.
typedef struct { int temp_min_dc, temp_max_dc; } temp_range_t;
static void mut_temp_range(app_state_t *st, void *arg)
{
    temp_range_t *r = arg;
    st->temp_min_dc = r->temp_min_dc;
    st->temp_max_dc = r->temp_max_dc;
}

/*
 * Write acks. `issued_us` is when the worker STARTED the write, so these can
 * tell the difference between "the user has not touched anything since" and
 * "the user kept going while this was in flight". A write that lands after
 * newer input must not roll the display back to what it happened to send.
 */
typedef struct { int zone; bool on; int64_t issued_us; } zone_on_t;
static void mut_zone_on(app_state_t *st, void *arg)
{
    zone_on_t *u = arg;
    if (dial_state_last_input_us() > u->issued_us) return;   // user moved on; leave their state alone
    st->zones[u->zone].on = u->on;
}

typedef struct { int zone; int temp_dc; int64_t issued_us; } zone_temp_t;
static void mut_zone_temp(app_state_t *st, void *arg)
{
    zone_temp_t *u = arg;
    st->zones[u->zone].temp_dc = u->temp_dc;        // the pad now holds this target

    // Only retire the optimistic display value if nothing newer has been dialled
    // in since this write left. Clearing it unconditionally is what made the
    // numeral run up with the knob and then SNAP BACK to whatever value the
    // in-flight write happened to carry — a number the user had already turned
    // past. The newer value has its own CMD_SET_TEMP queued behind this one.
    if (dial_state_last_input_us() <= u->issued_us)
        st->ui_temp_dc[u->zone] = -1;
}

static void mut_retry_in(app_state_t *st, void *arg)  { st->retry_in_s = *(int *)arg; }
static void mut_ap_ssid(app_state_t *st, void *arg)   { strlcpy(st->ap_ssid, arg, sizeof(st->ap_ssid)); }
static void mut_sta_ssid(app_state_t *st, void *arg)  { strlcpy(st->sta_ssid, arg, sizeof(st->sta_ssid)); }
static void mut_clock_valid(app_state_t *st, void *arg) { st->clock_valid = *(bool *)arg; }
static void mut_fresh_device(app_state_t *st, void *arg) { st->fresh_device = *(bool *)arg; }

// Mirrors a dial_ota_info_t snapshot into app_state_t.ota (see dial_state.h's
// comment on that field for why it's a plain-int mirror, not a #include).
static void mut_ota(app_state_t *st, void *arg)
{
    const dial_ota_info_t *info = arg;
    st->ota.status = (int)info->status;
    strlcpy(st->ota.latest, info->latest, sizeof(st->ota.latest));
    st->ota.progress_pct = info->progress_pct;
    strlcpy(st->ota.err, info->err, sizeof(st->ota.err));
    st->ota.pending_verify = info->pending_verify;
}

// Fetches the fresh dial_ota_get() snapshot and commits it.
static void commit_ota_snapshot(void)
{
    dial_ota_info_t info;
    dial_ota_get(&info);
    dial_state_commit(mut_ota, &info);
}

// Update prompt (docs/SPEC-update-prompt.md). The worker RAISES
// ota_prompt_due on a wake edge (via this mutator, see the "Update prompt:
// entry" gate below) and LOWERS it itself on one of three exit conditions
// (see "Update prompt: exit", same use site); scr_update_prompt.c also
// LOWERS it directly (dial_state_clear_ota_prompt_due) the instant the user
// acts — the same asymmetric set/clear split this file already uses for
// fresh_device (set here) / welcomed (cleared by scr_welcome.c).
static void mut_ota_unattended(app_state_t *st, void *arg) { st->ota.unattended = *(bool *)arg; }

static void mut_ota_prompt_due(app_state_t *st, void *arg) { st->ota_prompt_due = *(bool *)arg; }

// Auto-update two-strikes tracking (spec): worker-only, deliberately NOT
// persisted to NVS or mirrored into app_state_t — a device that fails an
// overnight install doesn't reboot (only a SUCCESSFUL apply does, via
// esp_restart() below), so this naturally survives every retry across many
// nights within one boot session; a rare manual power cycle just re-arms
// it, which is fine either way ("retry the next day" already covers it).
// The failure itself still surfaces on SCR_UPDATE — see the idle loop's
// stale-failure auto-clear gate below — by simply leaving dial_ota's own
// OTA_FAILED/.err alone (no new UI surface needed).
static char s_ota_auto_fail_ver[16];
static int  s_ota_auto_fail_count;
// At most one auto-install attempt per overnight-window OCCURRENCE (success
// or fail): latched the instant an attempt starts, re-armed when the clock
// walks back outside the window so tomorrow's occurrence gets its own try —
// without this, a failed attempt would retry every ~300ms for the rest of
// the ~2h window instead of "the next day" (spec).
static bool s_ota_auto_attempted;
// Live "is the prompt sheet currently raised" flag, mirrored into
// app_state_t.ota_prompt_due only on a false<->true transition — same
// edge-triggered shape as s_ui_night/s_ui_clock_valid below, so a tick where
// nothing changed doesn't bump the generation (and re-render everything) for
// no reason. Unlike the shipped v1.0-1.2 design, this is no longer a live
// re-evaluation of a gate table: it's set true exactly once per wake (see
// s_ota_prev_pwr_level below) and only ever cleared by the three explicit
// exit checks (or by scr_update_prompt.c itself, on a deliberate user
// action) — see the "Update prompt: entry" / "Update prompt: exit" comments
// at this flag's use site for the full story.
static bool s_ota_prompt_live;

// dial_power_level() as of the PREVIOUS idle tick, sampled every tick
// regardless of clock validity — the wake-edge gate below needs a
// STANDBY/DIMMED -> ACTIVE transition, which a level re-read at a single
// instant can't tell from "has been ACTIVE the whole time". Seeded ACTIVE
// (the real boot-time level, dial_power.c) so the very first tick can never
// look like a wake — the spec explicitly wants the prompt to never raise on
// the initial boot transition, only on an OBSERVED prior standby.
static dial_power_level_t s_ota_prev_pwr_level = DPWR_ACTIVE;

// OTA rollback health check (M6): dial_ota_mark_valid_if_pending() is
// idempotent, but there's nothing left to confirm after the first success —
// this guards against re-reading the OTA partition state on every ~10s poll
// for the rest of the device's uptime.
//
// Field incident: this used to be the ONLY confirm path, reached solely via
// "first successful pad poll" below (and its steady-state twin) — 30-60+s
// after boot, and hostage to Wi-Fi + the pad + a poll ALL
// succeeding. A user power-cycled inside that window and the bootloader
// silently reverted a good install because it never got the chance to
// confirm; worse, on a network outage post-update it could never confirm at
// all. Rollback exists to catch a genuinely BROKEN image, not to hold a good
// one hostage to a cloud outage — a broken image crash-loops well inside 30
// seconds. See the 30s ota_confirm_timer_cb fallback armed in app_main:
// whichever of that timer or a real poll success gets here first wins (this
// function is idempotent via s_ota_confirmed), and the loser's call becomes
// a no-op.
static bool s_ota_confirmed;
static void ota_confirm_once(void)
{
    if (s_ota_confirmed) return;
    dial_ota_mark_valid_if_pending();
    s_ota_confirmed = true;
    // Push the fresh snapshot (pending_verify very likely just flipped
    // false) so scr_connecting/scr_dial's "Finalizing update" notice drops
    // immediately, instead of waiting for some unrelated OTA-mirror commit.
    commit_ota_snapshot();
}

// The 30s stable-boot fallback itself (armed once from app_main). Runs in
// the esp_timer task, not an ISR -- esp_timer's default ESP_TIMER_TASK
// dispatch method, so calling into esp_ota_ops (via ota_confirm_once ->
// dial_ota_mark_valid_if_pending) here is safe, same as calling it from the
// worker task. The only cross-task hazard is s_ota_confirmed itself, which
// is a plain bool read-then-write from two possible callers (this timer's
// task and the worker task) with no lock; the benign race is a redundant
// dial_ota_mark_valid_if_pending() call if both sides read it false before
// either sets it true -- that function is itself idempotent (re-marking an
// already-valid partition, or re-reading a state that's already settled), so
// the worst case is one harmless extra flash-state read, never a double
// confirm or a lost one.
static void ota_confirm_timer_cb(void *arg)
{
    (void)arg;
    ota_confirm_once();
}

// No-op mutator: dial_state_commit() bumps the generation unconditionally, so
// this is just a way to force the dispatcher to re-render after a palette
// swap (screens re-read PAL() from on_state; they never cache day/night).
static void mut_bump(app_state_t *st, void *arg) { (void)st; (void)arg; }

// Tracks the night flag actually applied to the UI palette, separate from
// dial_power's own internal one (dial_power.c must not depend on dial_ui, so
// it can't call dial_palette_set_night itself — this is the seam instead).
static bool s_ui_night;

// Tracks the clock-valid flag actually committed to the store, mirroring
// s_ui_night's pattern, so the commit only fires on an actual transition.
static bool s_ui_clock_valid;

/* ---- Somnus pad calls (worker task only) -------------------------------- */

// zone_idx_t (ZONE_A=0/ZONE_B=1, dial_state.h) and somnus_side_t
// (SOMNUS_SIDE_0=0/SOMNUS_SIDE_1=1, dial_somnus.h) are deliberately the same
// small int enum shape, so a zone index casts straight to a pad side and
// back — no id-string mapping to maintain, unlike Orion's "zone_a"/"zone_b".

// Poll: GET /api/state (dial_somnus_get_state), copied into a device_snapshot_t
// exactly like orion_refresh_state used to build one from get_device_state.
static bool somnus_refresh_state(void)
{
    int64_t started_us = esp_timer_get_time();
    somnus_state_t s;
    if (!dial_somnus_get_state(&s)) return false;

    device_snapshot_t d = { .poll_started_us = started_us, .online = true };
    // Which side(s) are actually in play mirrors the pad's own zone-mode
    // setting (see dial_somnus.h's single-zone note): in "One Bed" mode
    // side1 mirrors side0's readings but can never be written to, so it must
    // not be offered as an independent partner face (same reasoning as
    // Orion's single-zone toppers before it — see app_state_t.zone_present).
    bool single_zone = dial_somnus_get_zone_mode();
    d.present[ZONE_A] = true;
    d.present[ZONE_B] = !single_zone;
    for (int z = 0; z < ZONE_COUNT; z++) {
        const somnus_side_state_t *side = &s.side[z];
        d.zones[z].on        = side->is_on;
        // The ONE deliberate, one-directional quantization boundary: the
        // wire's own float (arbitrary precision, whatever the pad reports)
        // rounds to the nearest 0.1°C to enter the canonical tenths-of-°C
        // representation. Purely Celsius-to-Celsius -- no °F is ever
        // involved on this path, so this is not the round-trip the Q1 fix
        // removed.
        d.zones[z].temp_dc   = (int)lroundf(side->target_c * 10.0f);
        d.zones[z].actual_c  = side->has_current ? side->current_c : -1.0f;
        d.zones[z].water_low = side->water_low;
    }
    d.system_error = s.system_error;

    for (int z = 0; z < ZONE_COUNT; z++)
        if (d.present[z])
            ESP_LOGI(TAG, "side %c: on=%d set=%.1fC water=%.1fC%s",
                     'A' + z, d.zones[z].on, d.zones[z].temp_dc / 10.0f, d.zones[z].actual_c,
                     d.zones[z].water_low ? " LOW-WATER" : "");

    dial_state_commit(mut_device_state, &d);
    return true;
}

/* ---- worker supervisor ------------------------------------------------- */

// Sleep `seconds` while publishing a countdown for the error screen.
static void backoff_wait(int seconds)
{
    for (int s = seconds; s > 0; s--) {
        dial_state_commit(mut_retry_in, &s);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    int zero = 0;
    dial_state_commit(mut_retry_in, &zero);
}

// dial_ota_download_and_apply's progress callback: fires on every
// esp_https_ota_perform() iteration (every ~4KB read), far too often to
// commit unthrottled — a store commit bumps the generation and triggers a
// full settings-screen re-render. Only commit on a >=5-point change (or the
// final 100%), same idea as the staleness-dot fade elsewhere in the app.
static int s_ota_last_committed_pct = -100;
static void ota_progress_cb(int pct)
{
    if (pct < 100 && pct - s_ota_last_committed_pct < 5) return;
    s_ota_last_committed_pct = pct;
    commit_ota_snapshot();
}

// The remaining non-SET_TEMP/TOGGLE_ON commands are rare (a deliberate tap on
// a Settings row, not a knob spin) — no coalescing, just run them in arrival
// order. No auth to retry here (dial_somnus is a plain unauthenticated local
// client), unlike the Orion with_auth_retry() this replaces.
static void handle_immediate_cmd(const app_cmd_t *cmd)
{
    switch (cmd->kind) {
    // Settings (M4) destructive actions: each erases some NVS state and
    // reboots — there's no follow-up state commit because esp_restart()
    // never returns.
    case CMD_WIFI_RESET:
        ESP_LOGW(TAG, "settings: change-network requested — rebooting into the setup portal");
        dial_net_request_setup();
        esp_restart();
        break;
    case CMD_FACTORY_RESET:
        ESP_LOGW(TAG, "settings: factory reset requested — erasing NVS");
        nvs_flash_erase();
        esp_restart();
        break;
    // Software update (M6/M7), from SCR_UPDATE's "Check for updates" row.
    // Gated on clock_valid the same as the M6 auto-check above
    // (dial_ota_check() is all HTTPS, and mbedTLS cert validation needs a
    // real wall clock) — but a manual tap is a user waiting on feedback,
    // not a background sweep, so a blocked check must say why rather than
    // silently doing nothing. st.beta selects the channel (M7) — see
    // dial_ota_check()'s own doc comment.
    case CMD_OTA_CHECK: {
        app_state_t st;
        dial_state_get(&st);
        // A MANUAL check re-arms the prompt: tapping "Check for updates" is a
        // user saying they care about updates right now, which outranks an
        // earlier "Later" (23h defer) or the once-a-day shown ceiling. Without
        // this there is no way back to the prompt short of waiting out the
        // timers — the owner lost it for a day to an accidental dismissal and
        // had no way to summon it again. The background check deliberately
        // does NOT do this; only a deliberate tap.
        dial_state_set_ota_defer(0);
        dial_state_set_ota_shown(0);
        // One connection at a time (see dial_somnus_release_connection): the
        // OTA client is about to open its own connection to GitHub, and a
        // second concurrent TLS session fails its handshake on this build.
        dial_somnus_release_connection();
        // Hold the screen for the duration: the user tapped this and is
        // waiting on the answer. Released in the same handler below, which
        // restarts the idle clock so the result is readable even on a 5s
        // timeout.
        dial_power_inhibit(DPWR_INHIBIT_TASK, true);
        if (st.clock_valid) {
            // Show "Checking..." for the duration of the call instead of
            // flicking straight to the answer -- a tap with no visible
            // response reads as a dead button (owner feedback).
            dial_ota_mark_checking();
            commit_ota_snapshot();
            dial_ota_check(st.beta);
        }
        else dial_ota_set_blocked("waiting for time sync - try again shortly");
        commit_ota_snapshot();
        dial_power_inhibit(DPWR_INHIBIT_TASK, false);
        break;
    }
    case CMD_OTA_APPLY: {
        // Held from the tap, not from the moment DOWNLOADING is published:
        // esp_https_ota_begin can take seconds (and retries once), and until
        // status flips, nav_policy hasn't yet forced the SCR_UPDATING screen
        // whose own inhibit would cover this.
        dial_power_inhibit(DPWR_INHIBIT_TASK, true);
        // Stale-tap guard: the row's confirm is only armed while AVAILABLE,
        // but the store could have moved on (an unrelated auto-check landed,
        // say) between the tap and the worker draining this command.
        app_state_t st;
        dial_state_get(&st);
        if (st.ota.status != OTA_AVAILABLE) {
            dial_power_inhibit(DPWR_INHIBIT_TASK, false);   // early out must not leak the hold
            break;
        }
        // This command only ever arrives from a deliberate, confirmed user
        // tap (scr_update.c's tap-twice, or scr_update_prompt.c's "Update
        // now") — never from the unattended overnight path below, which
        // calls dial_ota_download_and_apply() directly. A human actively
        // watching this install is exactly the case the auto-updater's
        // two-failed-attempts brake (docs/SPEC-update-prompt.md) doesn't
        // need to protect against, so a manual attempt always gets to try,
        // and resets the strike count for next time the auto-updater looks.
        s_ota_auto_fail_count = 0;
        s_ota_auto_fail_ver[0] = 0;
        s_ota_last_committed_pct = -100;   // guarantee the first progress commit fires
        // Hands the socket back: the downloader opens its own TLS session
        // with bigger buffers, and it should not have to compete with the
        // pad client for memory (see dial_somnus_release_connection).
        dial_somnus_release_connection();
        bool attended = false;
        dial_state_commit(mut_ota_unattended, &attended);
        bool ok = dial_ota_download_and_apply(ota_progress_cb);
        commit_ota_snapshot();
        if (ok) {
            ESP_LOGI(TAG, "OTA image ready; rebooting into it");
            esp_restart();
        }
        dial_power_inhibit(DPWR_INHIBIT_TASK, false);
        break;
    }
    // scr_settings.c posts this from destroy() (screen teardown) so a FAILED
    // row never survives to the next visit — see dial_ota_clear_stale_failure()'s
    // comment. max_age_us=0: clear immediately regardless of how recently it
    // failed. A no-op (no commit) if status already moved off OTA_FAILED.
    case CMD_OTA_CLEAR_FAILED:
        if (dial_ota_clear_stale_failure(0)) commit_ota_snapshot();
        break;
    // Settings' "Pad Address"/"Bed Mode" rows just persisted a new value
    // (dial_state_set_pad_url/set_zone_mode already wrote it to the store
    // and NVS) — apply it to dial_somnus live rather than waiting for a
    // reboot. dial_somnus_set_zone_mode() is cheap and always safe to
    // re-run; dial_somnus_connect() re-probes the (possibly new) address
    // with a real GET. A failed probe just sets PH_DEGRADED here and falls
    // through to the existing poll/backoff handling below — no separate
    // retry path needed: the drain loop's post-command
    // last_poll_us=0/poll_confirms=POLL_CONFIRM_N (below) forces a fast
    // recheck, and steady-state poll failures already march the phase
    // through PH_DEGRADED on their own if it's still unreachable after that.
    case CMD_PAD_SETTINGS_CHANGED: {
        app_state_t st;
        dial_state_get(&st);
        dial_somnus_set_zone_mode(st.pad_single_zone);
        if (!dial_somnus_connect(st.pad_base_url))
            dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
        break;
    }
    default:
        break;   // CMD_SET_TEMP/CMD_TOGGLE_ON never reach here (see the drain loop)
    }
}


static void worker_task(void *arg)
{
    (void)arg;
    int backoff_s = BACKOFF_MIN_S;

    /*
     * Input comes up FIRST, before the network.
     *
     * These used to be initialised after Wi-Fi, the pad connect and the
     * first poll had all succeeded — which meant that during Wi-Fi setup the
     * encoder callbacks did not exist yet and the knob was simply dead. That
     * was survivable when setup was "scan a QR with your phone", and fatal the
     * moment the dial started asking you to TYPE A PASSWORD with the knob.
     *
     * Nothing here needs the network: the display, touch and I2C bus are all up
     * (app_main did them), and iot_knob only needs its two GPIOs. Haptics is
     * safe to call before this runs — dial_haptics_play() no-ops until the
     * driver is present — so the ordering was never load-bearing, just late.
     */
    knob_init();
    dial_haptics_init();
    {
        // Apply the persisted haptics preference (restored into the store at
        // boot) — the driver defaults to Auto and settings only writes on
        // taps, so without this an Off/Low/High preference reverts every reboot.
        app_state_t st;
        dial_state_get(&st);
        dial_haptics_set_level((haptic_level_t)st.haptics_level);
    }

    // ---- Wi-Fi (blocking bringup; portal phase published via events) ----
    dial_state_set_phase(PH_WIFI_CONNECTING, NULL);
    dial_net_bringup();
    dial_state_commit(mut_sta_ssid, (void *)dial_net_sta_ssid());
    dial_time_start();

    // ---- Somnus pad connect, with retry/backoff -----------------------
    // No discovery, no auth, no interactive consent (see dial_somnus.h): a
    // single reachability probe against the PERSISTED base URL (Settings'
    // "Pad Address" row; dial_state_restore_prefs() already ran in app_main,
    // so this reads whatever the user last saved, or the compiled
    // DIAL_PAD_DEFAULT_* fallback on a fresh device). Read once here, not
    // re-read on every retry: a settings change mid-retry-loop reaches
    // dial_somnus through CMD_PAD_SETTINGS_CHANGED instead (handle_immediate_
    // cmd below), not by this loop noticing a moving target.
    char pad_url[DIAL_PAD_URL_MAX_LEN + 1];
    dial_state_get_pad_url(pad_url, sizeof(pad_url));
    bool pad_single_zone = dial_state_get_zone_mode();
    for (;;) {
        dial_state_set_phase(PH_SOMNUS_CONNECTING, NULL);
        if (dial_somnus_connect(pad_url)) break;
        dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
        backoff_wait(backoff_s);
        backoff_s = (backoff_s * 2 > BACKOFF_MAX_S) ? BACKOFF_MAX_S : backoff_s * 2;
    }
    backoff_s = BACKOFF_MIN_S;
    dial_somnus_set_zone_mode(pad_single_zone);
    ESP_LOGI(TAG, "pad connected at %s", pad_url);

    // Fixed pad range (local_api spec, dial_somnus.h) — no discovery call to
    // report it, unlike Orion's list_devices, so seed it once here instead
    // of per-poll. Hardcoded directly in the canonical unit (tenths of °C):
    // 12.0-42.3°C == 120-423dc, no conversion needed or wanted — there is no
    // °F anywhere upstream of this to convert from.
    {
        temp_range_t range = { 120, 423 };
        dial_state_commit(mut_temp_range, &range);
    }

    bool first_poll_ok = somnus_refresh_state();
    dial_state_set_phase(PH_READY, NULL);
    // OTA rollback health check (M6): reaching here with a successful poll
    // proves Wi-Fi + the pad + real device state all work on this image --
    // cancel the bootloader's pending-verify rollback timer if this boot
    // came from an OTA install. (Also re-tried on the first successful poll
    // in the steady-state loop below, in case this exact poll hit a
    // transient failure -- ota_confirm_once() only ever does real work once.)
    if (first_poll_ok) ota_confirm_once();

    // ---- steady state: drain commands (coalescing per zone), gated poll ----
    int64_t last_poll_us      = esp_timer_get_time();
    int     poll_confirms     = 0;   // fast reads still owed after a write
    int64_t last_ota_check_us = esp_timer_get_time();   // first auto-check ~24h after boot
    int poll_failures = 0;
    for (;;) {
        app_cmd_t cmd;
        if (dial_cmd_receive(&cmd, 300)) {
            // Rare, non-coalesced commands: handle the head of the queue
            // immediately and go back around, rather than folding them into
            // the temp/toggle coalescing loop below (which only knows those
            // two kinds).
            if (cmd.kind != CMD_SET_TEMP && cmd.kind != CMD_TOGGLE_ON) {
                handle_immediate_cmd(&cmd);
                last_poll_us = 0;                  // read it back now, not in 10s
                poll_confirms = POLL_CONFIRM_N;    // ...and again while the bed acts on it
                continue;
            }

            // Coalesce a burst: per zone, at most one net toggle + final temp.
            // A burst mixing in a rare command (above) is vanishingly
            // unlikely — each one is either a tap on its own settings row or
            // a trip through its own screen (SCR_SETTINGS) — but if one lands
            // mid-drain, stop coalescing and handle it right after rather
            // than silently mis-treating it as a toggle.
            int last_temp_dc[ZONE_COUNT] = { -1, -1 };  // tenths of °C, canonical unit
            int want_on[ZONE_COUNT]      = { -1, -1 };  // -1 = untouched this burst
            bool have_pending = false;
            app_cmd_t pending;
            // The command carries the DESIRED on state, so the last one posted
            // wins. (It used to count toggle parity and then re-derive
            // !current from the store — which now flips optimistically on tap,
            // so re-deriving would have undone the user's own press.)
            do {
                if (cmd.kind == CMD_SET_TEMP)       last_temp_dc[cmd.zone] = cmd.temp_dc;
                else if (cmd.kind == CMD_TOGGLE_ON) want_on[cmd.zone] = cmd.a ? 1 : 0;
                else { have_pending = true; pending = cmd; break; }
            } while (dial_cmd_receive(&cmd, 0));

            // Stamped BEFORE the writes: anything the user does during the
            // round trip is newer than this batch, and must not be undone by
            // the commits below (that was the knob "jumping back" to a value
            // the user had already turned past).
            int64_t issued_us = esp_timer_get_time();

            for (int z = 0; z < ZONE_COUNT; z++) {
                if (want_on[z] >= 0) {
                    zone_on_t up = { z, want_on[z] != 0, issued_us };
                    if (dial_somnus_set_power((somnus_side_t)z, up.on))
                        dial_state_commit(mut_zone_on, &up);
                }
                if (last_temp_dc[z] >= 0) {
                    // int tenths -> double directly, no float intermediate:
                    // this is the Q1 fix's other half (dial_somnus_set_temp's
                    // doc comment) — it's what keeps the JSON body a clean
                    // "21.7"/"22.0" instead of a float32-promoted
                    // "21.700000762939453".
                    double c = last_temp_dc[z] / 10.0;
                    zone_temp_t up = { z, last_temp_dc[z], issued_us };
                    if (dial_somnus_set_temp((somnus_side_t)z, c))
                        dial_state_commit(mut_zone_temp, &up);
                }
            }
            if (have_pending) handle_immediate_cmd(&pending);

            // We just changed the bed, so read it back as soon as the user
            // stops touching it, then keep reading for a few rounds while it
            // acts on the command. This used to set last_poll_us = now, which
            // pushed the confirming read a FULL interval away — so switching a
            // zone on left the dial showing the old state for ~10s even though
            // the quiet gate below was already all the protection a knob spin
            // needed.
            last_poll_us = 0;
            poll_confirms = POLL_CONFIRM_N;
            continue;
        }

        // Publish clock validity for anything that renders a real clock time.
        bool clock_valid = dial_time_valid();
        if (clock_valid != s_ui_clock_valid) {
            s_ui_clock_valid = clock_valid;
            dial_state_commit(mut_clock_valid, &clock_valid);
        }

        // Update-prompt wake edge (docs/SPEC-update-prompt.md rework):
        // sampled every idle tick regardless of clock validity, so a
        // STANDBY/DIMMED -> ACTIVE transition is never missed while waiting
        // on the entry gates below (which do need a valid clock -- see the
        // "Update prompt: entry" block inside the dial_time_now() branch).
        // A single dial_power_level() read can't tell "just woke" from
        // "has been ACTIVE for an hour"; comparing against the previous
        // tick's level is what makes it an edge instead of a level.
        dial_power_level_t ota_pwr_level = dial_power_level();
        bool ota_prompt_woke = (s_ota_prev_pwr_level == DPWR_STANDBY ||
                                 s_ota_prev_pwr_level == DPWR_DIMMED) &&
                                ota_pwr_level == DPWR_ACTIVE;
        s_ota_prev_pwr_level = ota_pwr_level;

        // Night mode: warm-dim + quiet haptics while the household sleeps.
        // Somnus's local API has no sleep-schedule endpoint to derive a real
        // window from (unlike Orion's get_sleep_schedules) -- fixed
        // 21:00-07:00 window, unconditionally.
        struct tm lt;
        if (dial_time_now(&lt)) {
            int now_min = lt.tm_hour * 60 + lt.tm_min;
            app_state_t st;
            dial_state_get(&st);

            bool night = (lt.tm_hour >= 21 || lt.tm_hour < 7);
            dial_power_set_night(night);
            // Swap the UI palette too, and force a re-render — screens read
            // PAL() from on_state, so a bare palette swap without a commit
            // would sit unapplied until the next unrelated state change.
            if (night != s_ui_night) {
                s_ui_night = night;
                dial_palette_set_night(night);
                dial_state_commit(mut_bump, NULL);
            }

            // ---- Update prompt: entry (docs/SPEC-update-prompt.md rework) --
            // Raised ONLY on the wake edge sampled above (STANDBY/DIMMED ->
            // ACTIVE), never on the initial boot transition (ota_prompt_woke
            // can't be true before an observed prior standby -- see
            // s_ota_prev_pwr_level's seed). The shipped v1.0-1.2 design
            // re-evaluated an idle-window gate (DPWR_ACTIVE + idle_us in
            // [10s,30s), the only slice between "settled" and "display about
            // to dim") continuously, every ~300ms tick: it could only ever
            // raise the prompt into an empty room (the user had already
            // walked away by the time idle_us cleared 10s), it consumed the
            // once-per-24h ota_shown stamp on that same empty-room raise, and
            // re-checking idle_us on every tick meant a touch (which resets
            // it) withdrew the sheet mid-reach. A wake edge is the one moment
            // a human is provably in front of the dial, so it needs no idle
            // window at all -- just the rest of the spec's gates, checked
            // once, right then.
            if (ota_prompt_woke) {
                time_t now_epoch = time(NULL);
                bool want_prompt =
                    st.ota.status == OTA_AVAILABLE &&
                    strcmp(st.ota_skip, st.ota.latest) != 0 &&
                    (uint32_t)now_epoch >= st.ota_defer &&
                    (st.ota_shown == 0 || (uint32_t)now_epoch - st.ota_shown >= 24 * 3600) &&
                    !night &&
                    st.phase == PH_READY && st.have_state &&
                    clock_valid &&
                    st.ota_auto == 0;   // Auto-update Off — if it's on, the dial handles it, don't ask
                if (want_prompt && !s_ota_prompt_live) {
                    s_ota_prompt_live = true;
                    dial_state_commit(mut_ota_prompt_due, &s_ota_prompt_live);
                    // Once-per-24h ceiling's own clock, independent of
                    // "Later"'s separate ota_defer — stamped the instant the
                    // sheet is actually raised, which is now honest: it only
                    // fires when a human just woke the device and is looking
                    // at it, not into an empty room.
                    dial_state_set_ota_shown((uint32_t)now_epoch);
                }
            }

            // ---- Update prompt: exit (deliberately separate from the entry
            // gates above) — once raised, the sheet is sticky (nav_policy
            // keeps routing to it, scoped to SCR_DIAL/SCR_UPDATE_PROMPT)
            // until the user acts (scr_update_prompt.c clears the flag
            // itself on every exit path: Update now / Later / Update
            // options / swipe-dismiss) or one of these three fires:
            //   - the display made it all the way back to STANDBY (the user
            //     walked away without acting)
            //   - night began
            //   - the update stopped being available (installed some other
            //     way, or superseded)
            // NOT a touch, and NOT a re-run of the entry gates: re-running
            // them was exactly what let a touch (idle_us resetting below the
            // old gate's floor) withdraw the sheet mid-reach before this
            // rework. These three are checked on their own so the sheet
            // survives being touched — that's the one thing "wake and reach
            // for the dial" is guaranteed to involve.
            if (s_ota_prompt_live &&
                (ota_pwr_level == DPWR_STANDBY || night || st.ota.status != OTA_AVAILABLE)) {
                s_ota_prompt_live = false;
                dial_state_commit(mut_ota_prompt_due, &s_ota_prompt_live);
            }

            // ---- Auto-update overnight install (docs/SPEC-update-prompt.md) -
            // Reuses dial_ota_download_and_apply() directly — the exact same
            // install path SCR_UPDATE's confirmed manual tap uses (CMD_OTA_APPLY
            // in handle_immediate_cmd above), including the takeover screen if
            // someone walks up mid-install (nav_policy's OTA check already
            // forces SCR_UPDATING off ota.status alone, regardless of who
            // started the download) and the v1.0.10 confirm-on-stable-boot
            // behavior (ota_confirm_once, unchanged by this).
            if (st.ota_auto == 1 && st.ota.status == OTA_AVAILABLE) {
                // Fixed 09:00-11:00 fallback (spec) -- Somnus has no sleep
                // schedule to derive a real post-wake window from, unlike
                // Orion's have_sched branch before it.
                int auto_start = 9 * 60, auto_end = 11 * 60;
                bool in_window = (now_min >= auto_start && now_min < auto_end);

                // NOT gated on the zones being off. The dial is a remote
                // control, not the bed's controller — heating/cooling is
                // driven by the pad's own hardware, and the dial's ~30s
                // reboot to apply an install has zero effect on the bed's
                // operation. A zones-off requirement protects against
                // nothing and permanently starves auto-update for anyone
                // who runs their bed through the day — silently, forever.
                // Do not reintroduce it on the assumption it was protecting
                // something.
                int64_t auto_idle_us = esp_timer_get_time() - dial_state_last_input_us();
                bool eligible = in_window && !night && st.phase == PH_READY &&
                                auto_idle_us >= 30LL * 60 * 1000000;

                if (!in_window) {
                    s_ota_auto_attempted = false;   // window closed; re-arm for tomorrow's occurrence
                } else if (eligible && !s_ota_auto_attempted) {
                    s_ota_auto_attempted = true;    // at most one attempt per window, success or fail
                    bool version_blocked = s_ota_auto_fail_ver[0] &&
                        s_ota_auto_fail_count >= 2 &&
                        strcmp(s_ota_auto_fail_ver, st.ota.latest) == 0;
                    if (version_blocked) {
                        ESP_LOGI(TAG, "auto-update: v%s blocked after 2 failed attempts -- skipping",
                                 st.ota.latest);
                    } else {
                        ESP_LOGI(TAG, "auto-update: attempting v%s in the overnight window",
                                 st.ota.latest);
                        s_ota_last_committed_pct = -100;   // guarantee the first progress commit fires
                        // Hands the socket back, same as the manual
                        // CMD_OTA_APPLY path above.
                        dial_somnus_release_connection();
                        bool unattended = true;
                        dial_state_commit(mut_ota_unattended, &unattended);
                        bool ok = dial_ota_download_and_apply(ota_progress_cb);
                        commit_ota_snapshot();
                        if (ok) {
                            ESP_LOGI(TAG, "auto-update: image ready; rebooting into it");
                            esp_restart();
                        } else {
                            dial_ota_info_t info;
                            dial_ota_get(&info);
                            if (strcmp(s_ota_auto_fail_ver, info.latest) != 0) {
                                strlcpy(s_ota_auto_fail_ver, info.latest, sizeof(s_ota_auto_fail_ver));
                                s_ota_auto_fail_count = 0;
                            }
                            s_ota_auto_fail_count++;
                            ESP_LOGW(TAG, "auto-update: attempt %d failed for v%s",
                                     s_ota_auto_fail_count, s_ota_auto_fail_ver);
                        }
                    }
                }
            }
        }

        // No command this tick. Resync only when quiet AND due — "due" being
        // sooner while we're still confirming a write the user just made.
        int64_t now = esp_timer_get_time();
        if (now - dial_state_last_input_us() < KNOB_SETTLE_US) continue;
        int64_t due = poll_confirms > 0 ? POLL_CONFIRM_US : POLL_INTERVAL_US;
        if (now - last_poll_us < due) continue;
        if (poll_confirms > 0) poll_confirms--;

        if (!dial_wifi_is_connected()) {
            // dial_net auto-reconnects; reflect the outage and wait it out.
            dial_state_set_phase(PH_WIFI_LOST, NULL);
            vTaskDelay(pdMS_TO_TICKS(1000));
            last_poll_us = esp_timer_get_time();
            continue;
        }

        if (somnus_refresh_state()) {
            poll_failures = 0;
            dial_state_set_phase(PH_READY, NULL);
            ota_confirm_once();
        } else if (++poll_failures >= 3) {
            dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error());
        }
        last_poll_us = esp_timer_get_time();

        // OTA_FAILED must not be terminal (field bug: it used to stay wedged
        // showing "Update failed" until a manual power cycle — see
        // dial_ota_clear_stale_failure()'s comment). Re-checked at this same
        // idle cadence; a no-op the vast majority of ticks (status isn't
        // FAILED, or it's not stale yet) so it's cheap to leave ungated.
        // This is the belt to CMD_OTA_CLEAR_FAILED's suspenders: it's what
        // un-wedges the row even if the user never leaves Settings at all.
        //
        // EXCEPT the two-failed-auto-installs case (docs/SPEC-update-prompt.md):
        // an unattended nightly retry that just hit its second strike must
        // leave a trace someone can actually find hours later on SCR_UPDATE,
        // not have this 25s timer erase it before anyone's looked. That
        // screen's own CMD_OTA_CLEAR_FAILED (posted on teardown) still clears
        // it the moment the user actually visits — this only skips the
        // silent, nobody-watching auto-clear.
        {
            dial_ota_info_t info;
            dial_ota_get(&info);
            bool blocked_and_failed = info.status == OTA_FAILED &&
                s_ota_auto_fail_ver[0] && s_ota_auto_fail_count >= 2 &&
                strcmp(s_ota_auto_fail_ver, info.latest) == 0;
            if (!blocked_and_failed && dial_ota_clear_stale_failure(OTA_FAILED_AUTOCLEAR_US))
                commit_ota_snapshot();
        }

        // Auto-check for firmware updates (M6): at most once per uptime-day,
        // and only CHECKS (never applies) -- the settings row just gets a
        // badge; the user still has to tap-confirm to install. Gated to a
        // window that can never coincide with sleep: clock known, and
        // steady state. Re-evaluated (cheaply) every idle tick once due, but
        // only actually calls dial_ota_check() -- and re-arms the 24h timer
        // -- once all of those hold.
        if (esp_timer_get_time() - last_ota_check_us >= OTA_AUTOCHECK_INTERVAL_US) {
            app_state_t ota_st;
            dial_state_get(&ota_st);
            struct tm ota_lt;
            // Per-device offset into the 10:00-16:00 window. Without it the
            // whole fleet checks at 10:00 sharp: the 24h timer expires
            // overnight for everybody, so the first tick past the window's
            // start releases every device at once. Installing a release makes
            // that worse rather than better -- an OTA reboots every device
            // that took it, aligning their timers from then on.
            //
            // Derived from the Wi-Fi MAC, so it is STABLE per device (a
            // random draw would re-roll each boot and could keep landing on
            // the same minute) and needs no storage. 0-299 minutes leaves at
            // least an hour of window after the latest offset.
            static int s_ota_check_offset_min = -1;
            if (s_ota_check_offset_min < 0) {
                uint8_t mac[6] = { 0 };
                esp_read_mac(mac, ESP_MAC_WIFI_STA);
                s_ota_check_offset_min = ((mac[4] << 8) | mac[5]) % 300;
                ESP_LOGI(TAG, "OTA auto-check window offset: +%dmin past 10:00",
                         s_ota_check_offset_min);
            }
            // Fill ota_lt BEFORE reading it: the old single-expression form
            // called dial_time_now() mid-condition, so anything computed from
            // ota_lt above it would have read stale/uninitialised fields.
            bool have_local = ota_st.clock_valid && dial_time_now(&ota_lt);
            int now_min_local = have_local ? ota_lt.tm_hour * 60 + ota_lt.tm_min : -1;
            // "Not asleep" is the only real requirement: the daytime band
            // existed to keep checks away from the sleep window. No night
            // gate at all (owner, 2026-08-04): the CHECK is silent -- one
            // HTTPS request, no screen, no sound -- so there is nothing to
            // protect a sleeping household from. What must never appear at
            // night is the PROMPT, and that has its own !night entry gate,
            // as does the ambient "Update available" line. Checking around
            // the clock just means a release published in the evening is
            // known by morning instead of waiting for the next daylight
            // window. Keeping the per-device jitter as a
            // minutes-into-the-hour stagger.
            bool in_window = have_local &&
                              (now_min_local % 60) >= (s_ota_check_offset_min % 60);
            if (in_window && ota_st.phase == PH_READY) {
                dial_somnus_release_connection();   // one connection at a time
                dial_ota_check(ota_st.beta);
                commit_ota_snapshot();
                last_ota_check_us = esp_timer_get_time();
            }
        }
    }
}

/* ---- app entry --------------------------------------------------------- */

static void net_event_cb(dial_net_event_t ev)
{
    // Runs on the Wi-Fi event task: only phase bookkeeping, nothing blocking.
    app_state_t st;
    switch (ev) {
    case DIAL_NET_EV_PORTAL:
        dial_state_set_phase(PH_WIFI_PORTAL, NULL);
        break;
    case DIAL_NET_EV_CONNECTING:
        // Credentials accepted (from the web form OR the dial's own keypad) and
        // a join is being attempted — move to the connecting phase so the
        // connecting screen shows through nav_policy rather than through screen
        // stickiness. Ignored once we're actually online, so a mid-session
        // reconnect blip doesn't flash "Connecting to Wi-Fi" over the dial.
        dial_state_get(&st);
        if (st.phase == PH_WIFI_PORTAL || st.phase == PH_BOOT)
            dial_state_set_phase(PH_WIFI_CONNECTING, NULL);
        break;
    case DIAL_NET_EV_SETUP_FAILED:
        // The credentials just submitted were rejected. nav_policy uses this to
        // put the user back on the password screen for the same network,
        // instead of at the start of setup with no idea what happened.
        dial_state_set_wifi_join_failed();
        break;
    case DIAL_NET_EV_LOST:
        dial_state_get(&st);
        if (st.phase == PH_READY || st.phase == PH_DEGRADED)
            dial_state_set_phase(PH_WIFI_LOST, NULL);
        break;
    case DIAL_NET_EV_GOT_IP:
        dial_state_get(&st);
        if (st.phase == PH_WIFI_LOST)
            dial_state_set_phase(st.have_state ? PH_READY : PH_WIFI_CONNECTING, NULL);
        break;
    default:
        break;
    }
}

void app_main(void)
{
    dial_display_start();
    dial_state_init();
    dial_display_set_touch_filter(touch_filter);
    dial_power_start();

    // Capture the boot's pending-verify state (rollback armed?) and mirror
    // it into app_state_t BEFORE the first screen renders below -- the
    // user needs the "don't unplug" warning from frame one, not 30s+ from
    // now when ota_confirm_once() would otherwise first touch this state.
    dial_ota_init();
    commit_ota_snapshot();

    // Router + screens live in the LVGL task from here on. ui_router_start
    // needs the LVGL lock because the LVGL task is already running.
    ui_screens_register_all();
    ui_router_set_nav_policy(nav_policy);
    if (dial_display_lock(-1)) {
        ui_router_start(SCR_CONNECTING, NULL);
        dial_display_unlock();
    }

    dial_net_on_event(net_event_cb);
    dial_net_init();
    // Onboarding (M4): "fresh" means no Wi-Fi creds were ever stored — read
    // BEFORE dial_net_seed()'s dev convenience below can inject any, so a
    // fresh-flashed dev build still exercises the real onboarding flow. A
    // user-requested network change also leaves NVS credential-less, but that
    // device is NOT fresh (it keeps its tokens, side, and prefs) — it should
    // land straight on the portal QR, not replay the welcome splash.
    bool fresh = !dial_net_have_creds() && !dial_net_setup_requested();
    dial_state_commit(mut_fresh_device, &fresh);
    dial_state_restore_prefs();   // last shown side + rotation (needs NVS, hence after net init)
    // dial_power_start() ran before prefs existed, so its first fade used the
    // 100% RAM default; without this nudge a saved dimmer preference wouldn't
    // apply until the next level change (~30s of full brightness after boot).
    dial_power_brightness_changed();

    // Apply the saved rotation to the panel. The store only remembers it; the
    // display is what has to act on it.
    {
        app_state_t st;
        dial_state_get(&st);
        if (st.rotation && dial_display_lock(-1)) {
            dial_display_set_rotation(st.rotation);
            dial_display_unlock();
        }
    }
    dial_net_seed(WIFI_SSID, WIFI_PASSWORD);
    dial_state_commit(mut_ap_ssid, (void *)dial_net_ap_ssid());

    // 30s stable-boot fallback confirm (see ota_confirm_once()'s comment for
    // the field incident this fixes): a crash-looping bad image never
    // survives anywhere near 30s, so this can't paper over a genuinely
    // broken update -- it only stops a cloud/network outage from holding a
    // GOOD image hostage to rollback. One-shot, never re-armed; left
    // allocated for the rest of the device's uptime rather than deleted
    // after firing, same as this codebase's other long-lived esp_timers
    // (e.g. dial_wifi.c's retry timer).
    const esp_timer_create_args_t ota_confirm_timer_args = {
        .callback = ota_confirm_timer_cb,
        .name = "ota_confirm",
    };
    esp_timer_handle_t ota_confirm_timer;
    ESP_ERROR_CHECK(esp_timer_create(&ota_confirm_timer_args, &ota_confirm_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(ota_confirm_timer, 30ULL * 1000000ULL));

    // The Somnus/OTA HTTP clients need a big stack; everything network runs on this task.
    // Priority 3 and pinned to core 0 — BELOW the LVGL task (5, core 1), which
    // the user is actually looking at. It used to be 4, outranking the UI, so a
    // TLS handshake froze the screen and swallowed taps. Core 0 is where Wi-Fi
    // and lwIP already live, so the network work stays on the network core and
    // leaves the UI a core of its own.
    xTaskCreatePinnedToCore(worker_task, "worker", 16384, NULL, 3, NULL, 0);
}
