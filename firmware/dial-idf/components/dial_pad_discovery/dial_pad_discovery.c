#include "dial_pad_discovery.h"

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "cJSON.h"
#include "dial_state.h"
#include "dial_wifi.h"

static const char *TAG = "pad_discovery";

/* ---- tunables (docs/SPEC-pad-discovery.md) ------------------------------
 * All four of these are the spec's own, measured/derived numbers -- see
 * that doc for the reasoning, not repeated here as bare constants.
 */
#define PROBE_PORT           8080
#define CONCURRENCY          4      // sockets: worst-case budget is 8 free
                                     // of CONFIG_LWIP_MAX_SOCKETS=10; this
                                     // leaves real margin, not the whole
                                     // remainder
#define TIMEOUT_PASS1_MS     300    // measured against the real pad
                                     // (cold ttfb 61-189ms) x ~1.6
#define TIMEOUT_PASS2_MS     600
#define MAX_HOSTS            256    // cap: /24 or tighter only
#define SCAN_COOLDOWN_US     (5LL * 60 * 1000000)   // 5 minutes
#define PROBE_RESP_BUF       1024   // the pad's /api/state body is small

static bool s_ever_attempted;
static int64_t s_last_attempt_us;

/* ---- scan-time log quieting ---------------------------------------------
 * A per-host probe failing is the normal outcome of a subnet sweep (~250 of
 * 254 hosts have nothing on :8080), and every failed esp_http_client_perform
 * makes the HTTP stack itself log three ERROR lines under its own tags:
 * esp-tls ("[sock=N] select() timeout" / "connect() error"), transport_base
 * ("Failed to open a new connection") and HTTP_CLIENT ("Connection failed,
 * sock < 0") -- ~3,000 lines per scan, drowning this component's own five
 * INFO lines. They are the library's own ESP_LOGE calls, so they can't be
 * re-levelled to DEBUG from here; instead those three tags are muted for the
 * duration of the scan and restored on every exit path. Scoped, not global:
 * the scan runs on (and blocks) the worker task, which is the only
 * esp_http_client user in this firmware (dial_somnus, dial_ota), so no other
 * request's failure can land in the muted window. This component's own
 * lines are untouched.
 */
static const char *const QUIET_TAGS[] = { "esp-tls", "transport_base", "HTTP_CLIENT" };
#define QUIET_TAGS_N (sizeof(QUIET_TAGS) / sizeof(QUIET_TAGS[0]))

static void quiet_http_logs(esp_log_level_t saved[QUIET_TAGS_N])
{
    for (size_t i = 0; i < QUIET_TAGS_N; i++) {
        saved[i] = esp_log_level_get(QUIET_TAGS[i]);
        esp_log_level_set(QUIET_TAGS[i], ESP_LOG_NONE);
    }
}

static void restore_http_logs(const esp_log_level_t saved[QUIET_TAGS_N])
{
    for (size_t i = 0; i < QUIET_TAGS_N; i++)
        esp_log_level_set(QUIET_TAGS[i], saved[i]);
}

bool dial_pad_discovery_should_attempt(void)
{
    if (!s_ever_attempted) return true;
    return (esp_timer_get_time() - s_last_attempt_us) >= SCAN_COOLDOWN_US;
}

void dial_pad_discovery_mark_attempted(void)
{
    s_ever_attempted = true;
    s_last_attempt_us = esp_timer_get_time();
}

/* ---- candidate ordering (Refinement 1) ---------------------------------
 * Builds one flat, de-duplicated, priority-ordered list of host-order IPv4
 * addresses to probe. Every tier below shares one "seen" table so a later
 * tier never re-adds an address an earlier one already placed -- this is
 * what makes the ordering purely a reprioritization: the same candidate set
 * as a plain linear sweep, just walked in a different order.
 */
typedef struct {
    uint32_t *out;
    int       cap;
    int       count;
    bool      seen[MAX_HOSTS];
} builder_t;

static uint32_t clampu(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void add_ip(builder_t *b, uint32_t ip, uint32_t network, uint32_t broadcast, uint32_t self_ip)
{
    if (ip <= network || ip >= broadcast) return;          // network/broadcast/out-of-range
    uint32_t offset = ip - network;
    if (offset >= (uint32_t)MAX_HOSTS) return;
    if (b->seen[offset]) return;
    b->seen[offset] = true;
    if (ip == self_ip) return;                              // never probe ourselves
    if (b->count < b->cap) b->out[b->count++] = ip;
}

static void add_range(builder_t *b, uint32_t lo, uint32_t hi, uint32_t network, uint32_t broadcast, uint32_t self_ip)
{
    for (uint32_t ip = lo; ip <= hi; ip++) add_ip(b, ip, network, broadcast, self_ip);
}

// Returns candidate count, or -1 if the subnet is looser than /24 (skip the
// scan entirely rather than attempt an intolerably long sweep -- spec's
// "Subnet size cap").
static int build_candidate_list(uint32_t *out, int cap, uint32_t self_ip, uint32_t netmask,
                                 bool have_persisted, uint32_t persisted_ip)
{
    uint32_t network   = self_ip & netmask;
    uint32_t host_span = ~netmask;              // e.g. 255 for a /24
    uint32_t broadcast = network | host_span;

    if (host_span == 0 || host_span > 255) return -1;

    builder_t b = { .out = out, .cap = cap, .count = 0 };
    memset(b.seen, 0, sizeof(b.seen));

    // Tier 0 (re-scan only): the address that just failed, + its immediate
    // neighbors. Skipped entirely on a fresh device (no real prior address
    // to hypothesize from) -- callers only pass have_persisted=true when
    // the failed URL parsed as a real, non-default IPv4 address.
    if (have_persisted && persisted_ip > network && persisted_ip < broadcast) {
        add_ip(&b, persisted_ip, network, broadcast, self_ip);
        add_range(&b, clampu(persisted_ip - 5, network + 1, broadcast - 1),
                      clampu(persisted_ip + 5, network + 1, broadcast - 1),
                  network, broadcast, self_ip);
    }

    // Tier 1: the dial's own DHCP neighborhood.
    add_range(&b, clampu(self_ip - 10, network + 1, broadcast - 1),
                  clampu(self_ip + 10, network + 1, broadcast - 1),
              network, broadcast, self_ip);

    // Tier 2: conventional low/static range.
    add_range(&b, network + 1, clampu(network + 20, network + 1, broadcast - 1), network, broadcast, self_ip);

    // Tier 3: conventional DHCP pool start.
    if (network + 100 < broadcast)
        add_range(&b, network + 100, clampu(network + 150, network + 100, broadcast - 1), network, broadcast, self_ip);

    // Tier 4: everything else, ascending.
    add_range(&b, network + 1, broadcast - 1, network, broadcast, self_ip);

    return b.count;
}

// Lenient best-effort "http://a.b.c.d[:port]" IPv4 extraction -- only used
// to seed tier 0's hypothesis, not to validate storage shape (that's
// scr_pad_address.c's job, a separate piece of work). A hostname, or
// anything else that doesn't parse, just means tier 0 is skipped.
static bool parse_ipv4_from_url(const char *url, uint32_t *out_ip)
{
    if (!url || strncmp(url, "http://", 7) != 0) return false;
    unsigned a, b, c, d;
    if (sscanf(url + 7, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) return false;
    if (a > 255 || b > 255 || c > 255 || d > 255) return false;
    *out_ip = (a << 24) | (b << 16) | (c << 8) | d;
    return true;
}

static void format_url(uint32_t ip, char *out, size_t sz)
{
    snprintf(out, sz, "http://%u.%u.%u.%u:%d",
             (unsigned)(ip >> 24) & 0xFF, (unsigned)(ip >> 16) & 0xFF,
             (unsigned)(ip >> 8) & 0xFF, (unsigned)ip & 0xFF, PROBE_PORT);
}

/* ---- validation: decode the JSON shape, never trust a 200 --------------
 * Same bar dial_somnus.c's own parse_side()/dial_somnus_get_state() already
 * hold GET /api/state to: side0 and side1 present, each with is_on (bool)
 * and target_t (number). Not calling that function directly -- it reads/
 * writes dial_somnus.c's own single s_base_url and is bound by its
 * worker-task-only contract -- just matching its validation shape.
 */
static bool validate_side(const cJSON *obj)
{
    if (!cJSON_IsObject(obj)) return false;
    const cJSON *is_on  = cJSON_GetObjectItemCaseSensitive(obj, "is_on");
    const cJSON *target = cJSON_GetObjectItemCaseSensitive(obj, "target_t");
    return is_on && (cJSON_IsTrue(is_on) || cJSON_IsFalse(is_on)) && cJSON_IsNumber(target);
}

static bool response_is_pad_state(const char *body)
{
    cJSON *root = cJSON_Parse(body);
    if (!root) return false;
    bool ok = validate_side(cJSON_GetObjectItemCaseSensitive(root, "side0")) &&
              validate_side(cJSON_GetObjectItemCaseSensitive(root, "side1"));
    cJSON_Delete(root);
    return ok;
}

/* ---- one probe ----------------------------------------------------------
 * Own esp_http_client instance per call, init/perform/cleanup, same shape
 * dial_somnus.c's do_request() uses -- just against an arbitrary candidate
 * IP instead of dial_somnus's own persistent s_base_url, and run
 * concurrently across worker tasks below (never sharing a client instance).
 */
typedef struct {
    char   buf[PROBE_RESP_BUF];
    size_t len;
} probe_accum_t;

static esp_err_t probe_event_handler(esp_http_client_event_t *evt)
{
    probe_accum_t *acc = (probe_accum_t *)evt->user_data;
    if (evt->event_id != HTTP_EVENT_ON_DATA || !acc) return ESP_OK;
    size_t room = sizeof(acc->buf) - 1 - acc->len;
    size_t take = (size_t)evt->data_len < room ? (size_t)evt->data_len : room;
    if (take > 0) {
        memcpy(acc->buf + acc->len, evt->data, take);
        acc->len += take;
        acc->buf[acc->len] = '\0';
    }
    return ESP_OK;
}

static bool probe_one(uint32_t ip, int timeout_ms)
{
    char url[48];
    snprintf(url, sizeof(url), "http://%u.%u.%u.%u:%d/api/state",
             (unsigned)(ip >> 24) & 0xFF, (unsigned)(ip >> 16) & 0xFF,
             (unsigned)(ip >> 8) & 0xFF, (unsigned)ip & 0xFF, PROBE_PORT);

    probe_accum_t acc = { .len = 0 };
    acc.buf[0] = '\0';
    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = timeout_ms,
        .event_handler = probe_event_handler,
        .user_data = &acc,
        // Numeric IP, never a hostname -- no DNS lookup, no extra socket.
        // Plain HTTP, same as dial_somnus.h's own "no TLS" design.
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return false;

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK || status < 200 || status >= 300) return false;
    return response_is_pad_state(acc.buf);
}

/* ---- one pass: N worker tasks racing a shared cursor --------------------
 * esp_http_client_perform() is blocking; there is no async/multiplexed API
 * for many concurrent targets, so bounded concurrency comes from N short-
 * lived tasks pulling "the next candidate" from one shared, mutex-guarded
 * cursor -- not from one task juggling sockets itself. First validated hit
 * wins: the flag is checked before claiming each new candidate, so a probe
 * already in flight when another task finds the pad is simply allowed to
 * finish (bounded to one probe timeout of tail latency) rather than
 * force-closed from outside.
 */
typedef struct {
    const uint32_t *candidates;
    int             count;
    int             next_idx;      // guarded by mux
    bool            found;         // guarded by mux
    uint32_t        found_ip;      // guarded by mux
    int             checked;       // guarded by mux
    int             timeout_ms;
    const char     *pass_headline;
    SemaphoreHandle_t mux;
    SemaphoreHandle_t done_sem;    // counting; one give per worker on exit
} scan_ctx_t;

static void worker_task(void *arg)
{
    scan_ctx_t *ctx = (scan_ctx_t *)arg;
    for (;;) {
        int idx;
        bool stop;
        xSemaphoreTake(ctx->mux, portMAX_DELAY);
        stop = ctx->found || ctx->next_idx >= ctx->count;
        idx = ctx->next_idx;
        if (!stop) ctx->next_idx++;
        xSemaphoreGive(ctx->mux);
        if (stop) break;

        bool hit = probe_one(ctx->candidates[idx], ctx->timeout_ms);

        int checked_now;
        int total;
        xSemaphoreTake(ctx->mux, portMAX_DELAY);
        ctx->checked++;
        checked_now = ctx->checked;
        total = ctx->count;
        if (hit) {
            if (!ctx->found) {
                ctx->found = true;
                ctx->found_ip = ctx->candidates[idx];
            } else {
                ESP_LOGI(TAG, "duplicate hit at %u.%u.%u.%u ignored, first wins",
                         (unsigned)(ctx->candidates[idx] >> 24) & 0xFF,
                         (unsigned)(ctx->candidates[idx] >> 16) & 0xFF,
                         (unsigned)(ctx->candidates[idx] >> 8) & 0xFF,
                         (unsigned)ctx->candidates[idx] & 0xFF);
            }
        }
        xSemaphoreGive(ctx->mux);

        if (hit) {
            uint32_t found_ip = ctx->candidates[idx];
            ESP_LOGI(TAG, "pad found at %u.%u.%u.%u",
                     (unsigned)(found_ip >> 24) & 0xFF, (unsigned)(found_ip >> 16) & 0xFF,
                     (unsigned)(found_ip >> 8) & 0xFF, (unsigned)found_ip & 0xFF);
        }

        // Progress (docs/SPEC-pad-discovery.md's "New phase and screen"):
        // reuses dial_state_set_phase's existing generation-bump channel,
        // no new IPC. Safe to call from every worker concurrently --
        // dial_state's own store mutex serializes it; updates may arrive
        // slightly out of order across tasks, harmless for a monotonic
        // counter display. "<headline>\n<checked>/<total>" -- the UI splits
        // on the newline (scr_pad_discovery.c).
        char progress[96];
        snprintf(progress, sizeof(progress), "%s\n%d/%d", ctx->pass_headline, checked_now, total);
        dial_state_set_phase(PH_PAD_DISCOVERY, progress);
    }
    xSemaphoreGive(ctx->done_sem);
    vTaskDelete(NULL);
}

static bool run_pass(const uint32_t *candidates, int count, int timeout_ms,
                      const char *pass_headline, uint32_t *found_ip_out)
{
    if (count <= 0) return false;

    scan_ctx_t ctx = {
        .candidates = candidates,
        .count = count,
        .timeout_ms = timeout_ms,
        .pass_headline = pass_headline,
    };
    ctx.mux = xSemaphoreCreateMutex();
    ctx.done_sem = xSemaphoreCreateCounting(CONCURRENCY, 0);
    if (!ctx.mux || !ctx.done_sem) {
        ESP_LOGE(TAG, "scan: out of semaphores, aborting pass");
        if (ctx.mux) vSemaphoreDelete(ctx.mux);
        if (ctx.done_sem) vSemaphoreDelete(ctx.done_sem);
        return false;
    }

    for (int i = 0; i < CONCURRENCY; i++)
        xTaskCreate(worker_task, "pad_probe", 4096, &ctx, 5, NULL);

    for (int i = 0; i < CONCURRENCY; i++)
        xSemaphoreTake(ctx.done_sem, portMAX_DELAY);

    vSemaphoreDelete(ctx.mux);
    vSemaphoreDelete(ctx.done_sem);

    if (ctx.found && found_ip_out) *found_ip_out = ctx.found_ip;
    return ctx.found;
}

/* ---- entry point ---------------------------------------------------------*/

bool dial_pad_discovery_scan(const char *failed_url, char *out_url, size_t out_sz)
{
    uint32_t self_ip, netmask;
    if (!dial_net_subnet(&self_ip, &netmask)) {
        ESP_LOGW(TAG, "scan: no STA IP, skipping");
        return false;
    }

    uint32_t persisted_ip = 0;
    bool have_persisted = parse_ipv4_from_url(failed_url, &persisted_ip) &&
                           strcmp(failed_url, DIAL_PAD_DEFAULT_BASE_URL) != 0;

    static uint32_t s_candidates[MAX_HOSTS];   // one scan at a time; not reentrant
    int n = build_candidate_list(s_candidates, MAX_HOSTS, self_ip, netmask, have_persisted, persisted_ip);
    if (n <= 0) {
        ESP_LOGW(TAG, "scan: subnet looser than /24 (or invalid), skipping");
        return false;
    }

    esp_log_level_t saved_levels[QUIET_TAGS_N];
    quiet_http_logs(saved_levels);

    ESP_LOGI(TAG, "scan: %d candidates, pass 1 (%dms)", n, TIMEOUT_PASS1_MS);
    uint32_t found_ip;
    bool found = run_pass(s_candidates, n, TIMEOUT_PASS1_MS, "Looking for your Somnus pad...", &found_ip);
    if (!found) {
        ESP_LOGI(TAG, "scan: pass 1 found nothing, pass 2 (%dms)", TIMEOUT_PASS2_MS);
        found = run_pass(s_candidates, n, TIMEOUT_PASS2_MS, "Still looking (checking more slowly)...", &found_ip);
    }

    restore_http_logs(saved_levels);

    if (found) {
        format_url(found_ip, out_url, out_sz);
        return true;
    }
    // INFO, not WARN: an absent pad is one of the two ordinary outcomes of a
    // scan (the caller already surfaces it on screen as PH_DEGRADED), not a
    // fault in the scan itself.
    ESP_LOGI(TAG, "scan: no pad found");
    return false;
}
