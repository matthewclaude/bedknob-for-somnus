#include "dial_power.h"

#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "dial_state.h"
#include "dial_haptics.h"

static const char *TAG = "power";

/*
 * Plugged-in / on-battery detector (docs/SPEC-power-sensing.md §10).
 * GPIO1 / ADC1_CH0, net BATT_ADC, a 10K/10K divider off the board's 5V rail
 * (§1) -- adc_read_mv() below reads it x2 to undo the divider. Same
 * adc_oneshot + adc_cali setup as Waveshare's 01_ADC_Test demo (§10.1);
 * deliberately not a hand-rolled raw*3.3/4096.
 *
 * Detection is §10.2, verbatim:
 *   - Hysteresis: enter PLUGGED at >= PWR_ENTER_PLUGGED_MV, leave PLUGGED
 *     (i.e. enter BATTERY) at <= PWR_LEAVE_PLUGGED_MV. The 100mV band between
 *     them is centered in the ~150mV worst-case gap §10.2's table measured.
 *   - Debounce: a state change needs PWR_DEBOUNCE_N consecutive 1-second
 *     samples whose classification agrees -- a marginal cable (§4) sagging
 *     across the band flaps at most once per five seconds, and with the
 *     slope term below, not at all.
 *   - Slope tiebreak inside the band: over the last PWR_RING_N samples, if
 *     the mean of the newest 5 exceeds the mean of the oldest 5 by more than
 *     PWR_SLOPE_TIEBREAK_MV, that's charging regardless of level; if lower
 *     by more than that, discharging. This is what makes a depleted cell
 *     climbing through the band on plug-in (§9.5) classify correctly instead
 *     of reading "on battery" for however long the climb takes.
 *   - Boot: UNKNOWN until PWR_BOOT_MIN_SAMPLES samples exist (~5s).
 * Sampled once per second -- every PWR_SAMPLE_EVERY_TICKS'th tick of
 * power_task's existing 100ms loop, not every tick: the ADC is noisy and the
 * number moves in minutes, not milliseconds.
 */
#define PWR_ADC_CHANNEL         ADC_CHANNEL_0
#define PWR_ADC_ATTEN           ADC_ATTEN_DB_12
#define PWR_ADC_BITWIDTH        ADC_BITWIDTH_12
#define PWR_SAMPLE_EVERY_TICKS  10      // power_task ticks at 100ms -> 1 sample/s
#define PWR_RING_N               10     // slope tiebreak window (§10.2)
#define PWR_BOOT_MIN_SAMPLES      5     // UNKNOWN until this many samples exist
#define PWR_DEBOUNCE_N             5    // consecutive agreeing samples to change state
#define PWR_ENTER_PLUGGED_MV    4280    // hysteresis: enter PLUGGED at >= 4.28V
#define PWR_LEAVE_PLUGGED_MV    4180    // hysteresis: leave PLUGGED at <= 4.18V
#define PWR_SLOPE_TIEBREAK_MV      20   // mean(newest5) - mean(oldest5), mV

static adc_oneshot_unit_handle_t s_pwr_adc;
static adc_cali_handle_t         s_pwr_cali;

// Chronological ring, oldest at [0] .. newest at [s_pwr_ring_n-1], saturating
// at PWR_RING_N (see pwr_ring_push). Plain shift-on-push rather than a
// circular index: one push per second, so the memmove cost is nothing.
static uint16_t         s_pwr_ring[PWR_RING_N];
static uint8_t          s_pwr_ring_n;
static dial_power_src_t s_power_src = PWR_UNKNOWN;   // mirrors the store; power_task's own view
static dial_power_src_t s_pwr_pending_vote = PWR_UNKNOWN;
static uint8_t           s_pwr_pending_count;

static void pwr_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &s_pwr_adc));
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = PWR_ADC_ATTEN,
        .bitwidth = PWR_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_pwr_adc, PWR_ADC_CHANNEL, &chan_cfg));

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = PWR_ADC_ATTEN,
        .bitwidth = PWR_ADC_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_pwr_cali));
}

// One calibrated reading, x2 for the divider (§1, §10.1). Skips the sample
// (returns false) on a transient read/cali error rather than crashing the
// task -- the ring's own PWR_BOOT_MIN_SAMPLES/PWR_RING_N gates already treat
// a short gap as "not enough history yet".
static bool pwr_read_mv(uint16_t *out_mv)
{
    int raw = 0;
    if (adc_oneshot_read(s_pwr_adc, PWR_ADC_CHANNEL, &raw) != ESP_OK) return false;
    int mv = 0;
    if (adc_cali_raw_to_voltage(s_pwr_cali, raw, &mv) != ESP_OK) return false;
    *out_mv = (uint16_t)(mv * 2);
    return true;
}

static void pwr_ring_push(uint16_t mv)
{
    if (s_pwr_ring_n < PWR_RING_N) {
        s_pwr_ring[s_pwr_ring_n++] = mv;
    } else {
        memmove(&s_pwr_ring[0], &s_pwr_ring[1], (PWR_RING_N - 1) * sizeof(uint16_t));
        s_pwr_ring[PWR_RING_N - 1] = mv;
    }
}

// §10.2's slope tiebreak: mean(newest 5) - mean(oldest 5), only once the ring
// is full (PWR_RING_N samples deep, ~10s after boot or after a gap) -- before
// that there is no well-defined "oldest 5" distinct from "newest 5", so an
// in-band reading is left inconclusive (see pwr_classify) rather than voting
// off a half-populated window.
static bool pwr_ring_slope_mv(int *out_slope)
{
    if (s_pwr_ring_n < PWR_RING_N) return false;
    uint32_t oldest_sum = 0, newest_sum = 0;
    for (int i = 0; i < 5; i++) oldest_sum += s_pwr_ring[i];
    for (int i = 5; i < PWR_RING_N; i++) newest_sum += s_pwr_ring[i];
    *out_slope = (int)(newest_sum / 5) - (int)(oldest_sum / 5);
    return true;
}

// This sample's vote: PLUGGED/BATTERY by threshold outside the hysteresis
// band, the slope tiebreak inside it, or PWR_UNKNOWN if inconclusive (inside
// the band with no full ring yet, or a slope within +-PWR_SLOPE_TIEBREAK_MV).
// An inconclusive vote breaks the debounce run without changing power_src --
// see pwr_sample_and_classify.
static dial_power_src_t pwr_classify(uint16_t mv)
{
    if (mv >= PWR_ENTER_PLUGGED_MV) return PWR_PLUGGED;
    if (mv <= PWR_LEAVE_PLUGGED_MV) return PWR_BATTERY;
    int slope;
    if (!pwr_ring_slope_mv(&slope)) return PWR_UNKNOWN;
    if (slope > PWR_SLOPE_TIEBREAK_MV)  return PWR_PLUGGED;
    if (slope < -PWR_SLOPE_TIEBREAK_MV) return PWR_BATTERY;
    return PWR_UNKNOWN;
}

static void mut_power_src(app_state_t *st, void *arg)
{
    st->power_src = *(dial_power_src_t *)arg;
}

// Runs once per second (every PWR_SAMPLE_EVERY_TICKS'th power_task tick).
// power_mv is written to the store on every call, without a commit (§10.3) --
// only a debounced power_src change commits (and logs a transition line).
static void pwr_sample_and_classify(void)
{
    uint16_t mv;
    if (!pwr_read_mv(&mv)) return;

    pwr_ring_push(mv);
    dial_state_set_power_mv(mv);
    // Bench-only instrumentation (§10.2's "open measurement, before
    // building": capture the plug-in curve with the log level raised).
    // Silent by default at ESP_LOGD -- removed from any future commit past
    // this series once the curve has been captured, per the task's own
    // instruction to keep it only "for the bench only and only in this
    // commit series".
    ESP_LOGD(TAG, "sample %u mV", mv);

    if (s_pwr_ring_n < PWR_BOOT_MIN_SAMPLES) return;   // still UNKNOWN (§10.2)

    dial_power_src_t vote = pwr_classify(mv);
    if (vote == PWR_UNKNOWN) {
        s_pwr_pending_count = 0;   // inconclusive sample breaks the run
        return;
    }
    if (vote == s_pwr_pending_vote) {
        s_pwr_pending_count++;
    } else {
        s_pwr_pending_vote = vote;
        s_pwr_pending_count = 1;
    }
    if (s_pwr_pending_count < PWR_DEBOUNCE_N || vote == s_power_src) return;

    s_power_src = vote;
    dial_state_commit(mut_power_src, &vote);
    switch (vote) {
    case PWR_PLUGGED: ESP_LOGI(TAG, "plugged (%u mV)", mv); break;
    case PWR_BATTERY: ESP_LOGI(TAG, "battery (%u mV)", mv); break;
    default:          ESP_LOGI(TAG, "unknown"); break;
    }
}

// Backlight is LEDC timer 3 / channel 1 (lcd_bl_pwm_bsp); we install the fade
// service and drive the same channel with hardware fades.
#define BL_MODE    LEDC_LOW_SPEED_MODE
#define BL_CHANNEL LEDC_CHANNEL_1

#define FADE_MS           400

/*
 * STANDBY fires after dial_state's user-configurable "Screen timeout" pref
 * (dial_state_get_screen_timeout_s(), NVS "ui"/"scr_to" — Settings' "Screen
 * timeout" row offers 30s/1m/2m/5m/10m; see dial_state.h's
 * DIAL_SCR_TIMEOUT_CHOICES). DIM fires at roughly a THIRD of that timeout —
 * short enough that even the 30s minimum still gets a distinct "about to
 * lock" warning before STANDBY, clamped to [10s, 30s] so it can never run
 * past the old fixed 30s (the shortest offered timeout would otherwise dim
 * immediately, i.e. zero warning) and never drops below ~10s (still a
 * perceptible heads-up at the floor). Both are read LIVE, once per
 * power_task tick, exactly like the day/night brightness percents below —
 * `want`'s idle comparison runs unconditionally every 100ms regardless of
 * s_force_reapply, so a pref change reaches the standby/dim DECISION within
 * one tick with no separate "changed" hook needed. (Brightness needs
 * dial_power_brightness_changed() because its percent only feeds the duty
 * computed INSIDE the "level changed" branch below; the timeout pref feeds
 * `want` itself, which is recomputed every tick no matter what.)
 */
static inline int64_t standby_after_us(void)
{
    return (int64_t)dial_state_get_screen_timeout_s() * 1000000;
}
static inline int64_t dim_after_us(void)
{
    uint16_t t = dial_state_get_screen_timeout_s();
    // The dim tier is a heads-up before the clock face takes over, so it only
    // makes sense when there is room for one. At 5s and 15s there isn't --
    // a third of 5s is under a second, and the old 10s floor would have
    // exceeded the timeout itself and dimmed AFTER standby. Below 30s the
    // tier is skipped entirely (dim == standby, so ACTIVE goes straight to
    // STANDBY); above it, a third of the timeout capped at the historical 30s.
    if (t < 30) return (int64_t)t * 1000000;
    int d = t / 3;
    if (d < 10) d = 10;
    if (d > 30) d = 30;
    return (int64_t)d * 1000000;
}

// Duty targets (8-bit). Night values keep the room dark. These are the BASE
// tables at the user's brightness pref == 100%; the apply path below scales
// them by dial_state's bri_day/bri_night percent (10..100) at every tick, so
// these constants must stay exactly what a 100% preference has always meant.
// Exception: DUTY_NIGHT.standby (the screensaver clock at night) is scaled by
// its OWN pref, bri_night_clock_pct, not bri_night_pct — see power_task's
// apply path below for the split and dial_state.h for the migration that
// keeps existing devices' clock glow unchanged when this pref was introduced.
typedef struct { uint8_t active, dimmed, standby; } duty_set_t;
static const duty_set_t DUTY_DAY   = { 255, 90, 25 };
static const duty_set_t DUTY_NIGHT = { 140, 40, 6  };

// Floors applied AFTER scaling, so a 10% preference dims a lot without ever
// blacking the panel out or making the standby glow imperceptible.
#define FLOOR_ACTIVE   13
#define FLOOR_DIMMED   10
#define FLOOR_STANDBY   4

/*
 * The night clock gets its own percent -> duty curve instead of scaling
 * DUTY_NIGHT.standby, because scaling that base is useless: it is 6, and the
 * standby floor is 4, so the whole 0-100% range collapsed into duties 4-6 --
 * indistinguishable to the eye (owner-reported, and correct).
 *
 * Range is therefore explicit: 0 turns the clock OFF (a dark bedroom is a
 * legitimate choice; touch or a detent still wakes the dial to the night
 * ACTIVE duty), and 100 reaches NIGHT_CLOCK_MAX_DUTY, comfortably readable
 * across a room. The curve is quadratic because perceived brightness is not
 * linear in duty: the interesting decisions all live in the first few duty
 * steps, and a linear ramp would spend most of the knob's travel on
 * differences nobody can see. Squaring puts ~a third of the range below duty
 * 7, which is where a bedside clock at 3am actually lives.
 *
 * 30% lands on duty 6 -- exactly the fixed value this firmware used before
 * the setting existed, which is what dial_state's migration seeds to.
 */
#define NIGHT_CLOCK_MAX_DUTY 64

uint8_t dial_power_night_clock_duty(uint8_t pct)
{
    if (pct == 0) return 0;                 // off, deliberately
    if (pct > 100) pct = 100;
    uint32_t d = ((uint32_t)NIGHT_CLOCK_MAX_DUTY * pct * pct) / 10000;
    return d < 1 ? 1 : (uint8_t)d;          // any non-zero choice stays visible
}

/*
 * Day/Night percent -> duty. The slider reads 0-100, but 0 is NOT black: it
 * lands on the dimmest duty this firmware has ever produced for that tier,
 * which is what the old 10%-floored slider bottomed out at. That keeps the
 * end values identical (owner: "make it go to 0% instead of 10% but keep the
 * end values the same") while giving the whole knob travel to the range
 * people actually use -- and it keeps the in-use screen legible at 0, which
 * a literal 0 duty would not.
 *
 * min = the old floor for this tier: max(hard floor, base/10), i.e. exactly
 * what scale_duty(base, 10, floor) returned before. Interpolating from there
 * to base is EXACTLY appearance-preserving for the old 10-100 values once
 * they are rescaled onto 0-100 (dial_state's migration does that): both
 * curves are linear in percent through the same two endpoints.
 */
static inline uint8_t tier_duty(uint8_t base, uint8_t pct, uint8_t floor)
{
    uint32_t min = (uint32_t)base / 10;
    if (min < floor) min = floor;
    if (pct > 100) pct = 100;
    uint32_t v = min + ((uint32_t)(base - min) * pct) / 100;
    if (v > 255) v = 255;
    return (uint8_t)v;
}

/*
 * Concurrency model (deliberate, after review):
 *  - s_level (the DECIDED level) is guarded by a spinlock so it can be read
 *    and written from the knob decoder's esp_timer callback, the LVGL task's
 *    touch filter, and the worker — none of which may block on a mutex.
 *  - LEDC fades are issued ONLY by power_task. Everyone else just changes the
 *    decision; power_task notices (s_applied != decided, or a forced reapply)
 *    within one 100ms tick. A wake therefore starts its fade <=100ms after
 *    the input — imperceptible against the 400ms fade itself.
 */
static portMUX_TYPE s_lvl_spin = portMUX_INITIALIZER_UNLOCKED;
static dial_power_level_t s_level = DPWR_ACTIVE;

static volatile bool s_night;
static volatile bool s_force_reapply;
#define INHIBIT_MAX_US (5LL * 60 * 1000000)   // see power_task
static volatile uint32_t s_inhibit_mask;   // see dial_power_inhibit   // set_night flips tables mid-level

// Settings-screen live preview override: -1 = none, else the exact duty to
// fade to instead of the normal level duty. Set only from dial_power_preview
// (LVGL task); read and applied only by power_task, same rule as s_level.
static volatile int s_preview_duty = -1;

static void fade_to(uint8_t duty)
{
    ledc_set_fade_with_time(BL_MODE, BL_CHANNEL, duty, FADE_MS);
    ledc_fade_start(BL_MODE, BL_CHANNEL, LEDC_FADE_NO_WAIT);
}

static const duty_set_t *duties(void) { return s_night ? &DUTY_NIGHT : &DUTY_DAY; }

static dial_power_level_t level_get(void)
{
    taskENTER_CRITICAL(&s_lvl_spin);
    dial_power_level_t l = s_level;
    taskEXIT_CRITICAL(&s_lvl_spin);
    return l;
}

// Runs only in power_task: decide from idle time (unless someone already
// forced ACTIVE via wake), then apply the duty when it changed.
static void power_task(void *arg)
{
    (void)arg;
    dial_power_level_t applied = (dial_power_level_t)-1;
    uint32_t pwr_tick = 0;
    for (;;) {
        int64_t idle = esp_timer_get_time() - dial_state_last_input_us();

        // An inhibit EXTENDS the timeout, it does not disable it. Treating it
        // as "never sleep" deadlocks: SCR_UPDATE_PROMPT inhibits sleep, and
        // its own exit condition is "the display returned to standby" — so an
        // unanswered prompt would hold the screen lit forever. The same trap
        // applies to any abandoned task screen: a dial left on the pairing QR
        // (which re-arms a fresh code every 5 minutes) or mid-passkey would
        // glow all night. Five minutes is far longer than any real pause in
        // these flows and still guarantees the screen eventually sleeps.
        int64_t sb_us  = standby_after_us();
        int64_t dim_us = dim_after_us();
        if (s_inhibit_mask) {
            if (sb_us < INHIBIT_MAX_US) sb_us = INHIBIT_MAX_US;
            dim_us = sb_us;   // no dim tier mid-task: dimming under someone's
                              // hands reads as the screen dying, not resting
        }
        dial_power_level_t want =
            (idle >= sb_us) ? DPWR_STANDBY :
            (idle >= dim_us) ? DPWR_DIMMED  : DPWR_ACTIVE;

        taskENTER_CRITICAL(&s_lvl_spin);
        s_level = want;
        taskEXIT_CRITICAL(&s_lvl_spin);

        if (want != applied || s_force_reapply) {
            s_force_reapply = false;
            int preview = s_preview_duty;
            if (preview >= 0) {
                // Settings screen is live-previewing a candidate value —
                // that duty wins outright, independent of the idle level.
                fade_to((uint8_t)preview);
            } else {
                const duty_set_t *d = duties();
                // ACTIVE/DIMMED always follow bri_day_pct/bri_night_pct. STANDBY
                // (the screensaver clock) is the one tier that forks by table:
                // day's standby duty still follows bri_day_pct (untouched by this
                // feature), but night's standby duty follows the separate
                // bri_night_clock_pct pref instead — that's the whole point of
                // the setting (a bedroom clock glow independent of in-use night
                // brightness). See dial_power_preview's header comment for the
                // matching preview-side split.
                uint8_t pct_live = s_night ? dial_state_get_bri_night_pct()
                                            : dial_state_get_bri_day_pct();
                uint8_t pct_standby = s_night ? dial_state_get_bri_night_clock_pct()
                                               : dial_state_get_bri_day_pct();
                uint8_t duty;
                switch (want) {
                case DPWR_ACTIVE:  duty = tier_duty(d->active,  pct_live,    FLOOR_ACTIVE);  break;
                case DPWR_DIMMED:  duty = tier_duty(d->dimmed,  pct_live,    FLOOR_DIMMED);  break;
                case DPWR_STANDBY:
                    duty = s_night ? dial_power_night_clock_duty(pct_standby)
                                   : tier_duty(d->standby, pct_standby, FLOOR_STANDBY);
                    break;
                default:           duty = d->active; break;
                }
                fade_to(duty);
            }
            applied = want;
        }

        // Plugged-in / on-battery detector (§10.2): every 10th tick of this
        // same 100ms loop, i.e. once per second -- not every tick, the ADC is
        // noisy and the number moves in minutes.
        if (++pwr_tick % PWR_SAMPLE_EVERY_TICKS == 0) pwr_sample_and_classify();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void dial_power_start(void)
{
    ledc_fade_func_install(0);
    fade_to(duties()->active);
    pwr_adc_init();
    xTaskCreate(power_task, "power", 2560, NULL, 2, NULL);
}

dial_power_level_t dial_power_level(void) { return level_get(); }

// Called from the knob esp_timer callback and the LVGL touch filter: must be
// lock-free-ish (spinlock only) and must NOT touch LEDC. Flipping the level
// here makes the wake decision immediately visible to both callers and the
// router; power_task issues the actual fade on its next tick (input was just
// stamped, so it computes ACTIVE too).
bool dial_power_wake_consumes(void)
{
    bool consumed = false;
    taskENTER_CRITICAL(&s_lvl_spin);
    if (s_level == DPWR_STANDBY) {
        s_level = DPWR_ACTIVE;
        consumed = true;
    }
    taskEXIT_CRITICAL(&s_lvl_spin);
    return consumed;
}

void dial_power_inhibit(dial_power_inhibit_src_t src, bool on)
{
    // Read-modify-write from TWO tasks (the LVGL task sets SCREEN on every
    // navigation, the worker sets TASK around multi-second calls), so it
    // takes the same spinlock the rest of this file's cross-task state uses.
    // Unguarded, an interleave drops one source's bit — and losing SCREEN is
    // precisely the passkey-screen-sleeps-mid-entry bug this feature exists
    // to prevent.
    taskENTER_CRITICAL(&s_lvl_spin);
    uint32_t before = s_inhibit_mask;
    if (on) s_inhibit_mask |= (uint32_t)src;
    else    s_inhibit_mask &= ~(uint32_t)src;
    uint32_t after = s_inhibit_mask;
    taskEXIT_CRITICAL(&s_lvl_spin);
    if (after == before) return;

    // Releasing the LAST hold restarts the idle clock. Without this the clock
    // resumes from the user's last real touch, which by then is older than the
    // whole timeout — so a 5s screen slept the instant a request resolved,
    // before the result could be read (owner, 2026-08-04). A task ending is
    // exactly when someone starts looking, so they get a full timeout from
    // that moment.
    if (before && !after) dial_state_stamp_input();
    s_force_reapply = true;   // power_task re-decides on its next 100ms tick
}

void dial_power_set_night(bool night)
{
    if (s_night == night) return;
    s_night = night;
    dial_haptics_set_night(night);
    s_force_reapply = true;   // power_task re-fades with the new duty table
    ESP_LOGI(TAG, "night mode %s", night ? "on" : "off");
}

// Called (from the LVGL task) after dial_state_set_bri_day_pct/
// set_bri_night_pct already persisted the new preference. Just a decision
// flip, like every other setter in this file — power_task issues the actual
// fade on its next 100ms tick.
void dial_power_brightness_changed(void)
{
    s_force_reapply = true;
}

void dial_power_preview(bool night, dial_power_level_t level, uint8_t pct)
{
    const duty_set_t *base = night ? &DUTY_NIGHT : &DUTY_DAY;
    uint8_t duty;
    switch (level) {
    case DPWR_DIMMED:  duty = tier_duty(base->dimmed,  pct, FLOOR_DIMMED);  break;
    case DPWR_STANDBY:
        // Same curve power_task applies, or the preview would be showing a
        // brightness the setting cannot actually produce.
        duty = night ? dial_power_night_clock_duty(pct)
                     : tier_duty(base->standby, pct, FLOOR_STANDBY);
        break;
    case DPWR_ACTIVE:
    default:           duty = tier_duty(base->active,  pct, FLOOR_ACTIVE);  break;
    }
    s_preview_duty = duty;
    s_force_reapply = true;
}

void dial_power_preview_end(void)
{
    s_preview_duty = -1;
    s_force_reapply = true;
}
