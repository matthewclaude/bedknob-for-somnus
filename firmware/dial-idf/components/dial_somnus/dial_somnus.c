#include "dial_somnus.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "dial_somnus";

#define SOMNUS_HTTP_TIMEOUT_MS 5000
#define SOMNUS_RESP_BUF_MAX    4096   // the pad's /api/state body is small; generous headroom

static char s_base_url[128] = {0};
static char s_last_error[160] = {0};
static bool s_single_zone_mode = SOMNUS_DEFAULT_SINGLE_ZONE_MODE;

void dial_somnus_set_zone_mode(bool single_zone)
{
    s_single_zone_mode = single_zone;
    ESP_LOGI(TAG, "zone mode set to %s", single_zone ? "single (One Bed)" : "dual (Dual Sides)");
}

bool dial_somnus_get_zone_mode(void)
{
    return s_single_zone_mode;
}

static void set_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(s_last_error, sizeof(s_last_error), fmt, ap);
    va_end(ap);
    ESP_LOGW(TAG, "%s", s_last_error);
}

static void clear_error(void) { s_last_error[0] = '\0'; }

const char *dial_somnus_last_error(void)
{
    return s_last_error[0] ? s_last_error : NULL;
}

/* ---- response buffering ------------------------------------------------
 * Accumulate the body into a growing heap buffer via the HTTP_EVENT_ON_DATA
 * callback, since esp_http_client's internal buffer isn't guaranteed to
 * hand you the whole body in one event for anything but tiny fixed-size
 * responses.
 */
typedef struct {
    char   *buf;
    size_t  len;
    size_t  cap;
} resp_accum_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    resp_accum_t *acc = (resp_accum_t *)evt->user_data;
    if (evt->event_id != HTTP_EVENT_ON_DATA || !acc) return ESP_OK;

    if (acc->len + evt->data_len + 1 > acc->cap) {
        size_t new_cap = acc->cap ? acc->cap * 2 : 512;
        while (new_cap < acc->len + evt->data_len + 1) new_cap *= 2;
        if (new_cap > SOMNUS_RESP_BUF_MAX) {
            ESP_LOGE(TAG, "response exceeds %d bytes, truncating", SOMNUS_RESP_BUF_MAX);
            return ESP_OK;
        }
        char *grown = realloc(acc->buf, new_cap);
        if (!grown) { ESP_LOGE(TAG, "OOM growing response buffer"); return ESP_FAIL; }
        acc->buf = grown;
        acc->cap = new_cap;
    }
    memcpy(acc->buf + acc->len, evt->data, evt->data_len);
    acc->len += evt->data_len;
    acc->buf[acc->len] = '\0';
    return ESP_OK;
}

/* ---- one-shot request helper --------------------------------------------
 * method: HTTP_METHOD_GET or HTTP_METHOD_POST. body may be NULL for GET.
 * On success, *resp_out is a heap string the caller must free(); on failure
 * returns false and sets the error string (dial_somnus_last_error()).
 */
static bool do_request(const char *path, esp_http_client_method_t method,
                        const char *body, char **resp_out)
{
    char url[192];
    snprintf(url, sizeof(url), "%s%s", s_base_url, path);

    resp_accum_t acc = {0};
    esp_http_client_config_t cfg = {
        .url = url,
        .method = method,
        .timeout_ms = SOMNUS_HTTP_TIMEOUT_MS,
        .event_handler = http_event_handler,
        .user_data = &acc,
        // Plain local HTTP, no TLS: the pad's API is http:// only,
        // same-subnet. No cert handling needed at all.
    };

    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) { set_error("http client init failed"); return false; }

    if (body) {
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, body, strlen(body));
    }

    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK) {
        set_error("http error: %s", esp_err_to_name(err));
        free(acc.buf);
        return false;
    }
    if (status < 200 || status >= 300) {
        // Spec: 400 for bad request, 500 for internal server error, both
        // carry an ErrorResponse body ({"error": "..."}) we could surface
        // here if we wanted the exact pad-side message; for now we just
        // report the status code, which is enough to distinguish "pad is
        // unreachable" from "pad rejected this specific request".
        set_error("pad returned HTTP %d", status);
        free(acc.buf);
        return false;
    }

    clear_error();
    *resp_out = acc.buf ? acc.buf : strdup("");
    return true;
}

/* ---- public API ---------------------------------------------------------- */

bool dial_somnus_connect(const char *base_url)
{
    strncpy(s_base_url, base_url, sizeof(s_base_url) - 1);
    s_base_url[sizeof(s_base_url) - 1] = '\0';

    somnus_state_t probe;
    return dial_somnus_get_state(&probe);
}

static bool parse_side(const cJSON *side_obj, somnus_side_state_t *out)
{
    if (!cJSON_IsObject(side_obj)) return false;

    // Field names per the official spec (local_api-2.yml SideState schema):
    // is_on, is_wl_low, target_t, current_t (nullable).
    const cJSON *is_on   = cJSON_GetObjectItemCaseSensitive(side_obj, "is_on");
    const cJSON *target  = cJSON_GetObjectItemCaseSensitive(side_obj, "target_t");
    const cJSON *current = cJSON_GetObjectItemCaseSensitive(side_obj, "current_t");
    const cJSON *wl      = cJSON_GetObjectItemCaseSensitive(side_obj, "is_wl_low");

    out->is_on     = cJSON_IsTrue(is_on);
    out->target_c  = cJSON_IsNumber(target) ? (float)target->valuedouble : out->target_c;
    out->water_low = cJSON_IsTrue(wl);

    // current_t: spec says "Null before first sensor reading" -- treat
    // anything that isn't a JSON number (null, absent, or the empty-string
    // shape bed_web_app.py's own Flask layer sometimes produces) as "no
    // reading yet", not as zero.
    if (cJSON_IsNumber(current)) {
        out->has_current = true;
        out->current_c   = (float)current->valuedouble;
    } else {
        out->has_current = false;
    }
    return true;
}

bool dial_somnus_get_state(somnus_state_t *out)
{
    char *resp = NULL;
    if (!do_request("/api/state", HTTP_METHOD_GET, NULL, &resp)) return false;

    cJSON *root = cJSON_Parse(resp);
    free(resp);
    if (!root) { set_error("bad JSON from /api/state"); return false; }

    somnus_state_t parsed = *out;  // start from last-known-good so a partial
                                    // body can't zero fields it didn't touch
    bool ok = true;
    ok &= parse_side(cJSON_GetObjectItemCaseSensitive(root, "side0"), &parsed.side[SOMNUS_SIDE_0]);
    ok &= parse_side(cJSON_GetObjectItemCaseSensitive(root, "side1"), &parsed.side[SOMNUS_SIDE_1]);

    // Spec field name is "error", a plain bool at the top level of
    // StateResponse -- not nested under either side.
    const cJSON *error = cJSON_GetObjectItemCaseSensitive(root, "error");
    parsed.system_error = cJSON_IsTrue(error);

    cJSON_Delete(root);

    if (!ok) { set_error("malformed /api/state body"); return false; }

    *out = parsed;
    clear_error();
    return true;
}

static bool post_side_field(const char *path, somnus_side_t side,
                              const char *field, cJSON *value)
{
    // Single-zone safety guard: per the official spec, "In single-zone
    // mode, you should only operate side0... Providing side1 can produce
    // undefined/undesired behavior." We can't detect the mode from the API
    // (no field reports it -- see dial_somnus.h), so we trust whatever the
    // Settings screen last told us via dial_somnus_set_zone_mode() and
    // simply never send a side1 write while it's set, rather than sending
    // one and hoping the pad handles it gracefully.
    if (s_single_zone_mode && side == SOMNUS_SIDE_1) {
        cJSON_Delete(value);  // value was heap-allocated by the caller
                               // (cJSON_CreateNumber/CreateBool) expecting
                               // us to consume it either way -- free it
                               // here since we're not building a body
        ESP_LOGD(TAG, "single-zone mode: skipping side1 write to %s", path);
        clear_error();
        return true;   // not an error -- a deliberate, silent no-op
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *side_obj = cJSON_CreateObject();
    cJSON_AddItemToObject(side_obj, field, value);
    cJSON_AddItemToObject(root, side == SOMNUS_SIDE_0 ? "side0" : "side1", side_obj);

    char *body = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!body) { set_error("OOM building request body"); return false; }

    // Verification for the units fix (2026-08-30): log the exact bytes going
    // over the wire, so a clean "21.7"/"22.0" can be confirmed by eye against
    // the float->double promotion bug this used to carry (see
    // dial_somnus_set_temp's doc comment) — never something like
    // "21.700000762939453".
    ESP_LOGI(TAG, "POST %s %s", path, body);

    char *resp = NULL;
    bool ok = do_request(path, HTTP_METHOD_POST, body, &resp);
    free(body);
    free(resp);
    return ok;
}

bool dial_somnus_set_temp(somnus_side_t side, double temp_c)
{
    return post_side_field("/api/target_t", side, "target_t", cJSON_CreateNumber(temp_c));
}

bool dial_somnus_set_power(somnus_side_t side, bool on)
{
    return post_side_field("/api/power", side, "is_on", cJSON_CreateBool(on));
}

void dial_somnus_release_connection(void)
{
    // No persistent connection kept between calls (each do_request() opens
    // and closes its own client) — nothing to drop yet. Kept as a no-op
    // stub for interface symmetry with dial_mcp.
}
