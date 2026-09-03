/*
 * OTA updates from GitHub Releases (M6). See dial_ota.h for the threading
 * contract and the overall flow.
 */
#include "dial_ota.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "cJSON.h"

// Embedded multi-root PEM (EMBED_TXTFILES, CMakeLists.txt) -- see dial_ota.h's
// header comment for what's in it and why. Used to be dial_oauth_root_ca(),
// shared with the now-removed Orion client; this component is its only
// remaining consumer, so the anchors are embedded directly here.
extern const char trust_roots_pem_start[] asm("_binary_trust_roots_pem_start");

static const char *TAG = "ota";

// Points at the public binaries-only release repo (docs/SPEC-ota-readiness.md
// §5), not the private source repo this firmware is built from -- GitHub
// 404s every endpoint under a private repo to an unauthenticated client
// (§1), and both the dial and ESP Web Tools are unauthenticated by design.
#define GITHUB_API_URL \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
// Beta channel only (docs/REPORT-beta-fix.md, 2026-09-03 finding): GitHub's
// /releases list is ordered by created_at, which here is the date of the
// one commit every release/tag points at, so every entry ties and a
// per_page cap cannot be trusted to contain the newest one -- the first
// beta release landed sixth, past a per_page=5 cap. The tags endpoint is
// scanned in full instead (below) and never assumed to be ordered either.
#define GITHUB_API_URL_TAGS \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
// One release, by its exact tag name -- the beta channel's second request,
// made only for the single tag check_beta() already picked as newest.
#define GITHUB_API_URL_RELEASE_BY_TAG_FMT \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
#define ASSET_NAME     "somnus-dial.bin"
// "somnus-v" since 2026-09-01 (docs/SPEC-ota-readiness.md §7) -- the repo's
// own release history through "dial-v1.4.2" is inherited lineage from the
// upstream fork, not a Somnus release; this prefix must match whatever
// .github/workflows/release.yml's tag trigger + PROJECT_VER-verification
// steps use, or release_version() below silently fails to strip it and
// is_newer() rejects every release forever (an unparseable tag_name makes
// sscanf match zero fields, which reads as "not newer" -- no error, no log
// distinguishing it from "already current").
#define TAG_PREFIX     "somnus-v"
#define CHECK_BUF_CAP  (64 * 1024)   // release JSON is normally ~10-30KB
// Beta channel only: how many candidate tags (highest first) check_beta()
// will fetch a release object for before giving up. Bounds the worst case
// where the newest tag or two turn out to be a draft or a deleted release
// -- this device has no business scanning its whole tag history for a
// usable release.
#define OTA_BETA_CANDIDATE_CAP 3

// Guards s_info and s_asset_url. A short spinlock (never held across a
// blocking call), matching dial_state.c's s_input_mux idiom for cross-task
// state that's only ever read/written as a quick copy.
static portMUX_TYPE       s_mux = portMUX_INITIALIZER_UNLOCKED;
static dial_ota_info_t    s_info;
static char               s_asset_url[300];   // captured by dial_ota_check()
// esp_timer_get_time() at the moment s_info.status last became OTA_FAILED --
// dial_ota_clear_stale_failure()'s clock. Only meaningful while status ==
// OTA_FAILED; never read otherwise.
static int64_t             s_failed_at_us;

void dial_ota_get(dial_ota_info_t *out)
{
    taskENTER_CRITICAL(&s_mux);
    *out = s_info;
    taskEXIT_CRITICAL(&s_mux);
}

static void set_status(dial_ota_status_t status, const char *latest, const char *err)
{
    int64_t now = esp_timer_get_time();
    taskENTER_CRITICAL(&s_mux);
    s_info.status = status;
    if (status == OTA_FAILED) s_failed_at_us = now;
    if (latest) strlcpy(s_info.latest, latest, sizeof(s_info.latest));
    if (err)    strlcpy(s_info.err, err, sizeof(s_info.err));
    else        s_info.err[0] = 0;
    taskEXIT_CRITICAL(&s_mux);
}

bool dial_ota_clear_stale_failure(int64_t max_age_us)
{
    int64_t now = esp_timer_get_time();
    bool cleared = false;
    taskENTER_CRITICAL(&s_mux);
    if (s_info.status == OTA_FAILED && (now - s_failed_at_us) >= max_age_us) {
        s_info.status = OTA_IDLE;
        s_info.err[0] = 0;
        cleared = true;
    }
    taskEXIT_CRITICAL(&s_mux);
    return cleared;
}

static void set_progress(int pct)
{
    taskENTER_CRITICAL(&s_mux);
    s_info.progress_pct = pct;
    taskEXIT_CRITICAL(&s_mux);
}

// Parses an optional "-beta.N" suffix trailing a major.minor.patch core (the
// only prerelease form this firmware ships). *has_suffix reports whether one
// was found at all; *n is its numeral (0 if absent, or if present but
// unparseable -- a malformed "-beta" with no digits still counts as SOME
// prerelease for ordering purposes rather than silently acting like a full
// release, which would let a busted tag skip the channel altogether).
static void parse_beta_suffix(const char *ver, bool *has_suffix, int *n)
{
    const char *p = strstr(ver, "-beta.");
    *has_suffix = (p != NULL);
    *n = 0;
    if (p) sscanf(p + 6, "%d", n);
}

// Semver-ish compare, prerelease-aware (semver §11): split the major.minor.
// patch core on dots, numeric compare. Missing trailing components default
// to 0 ("1.2" == "1.2.0"); %d naturally stops at the first non-digit, so a
// "-beta.N" suffix on either string doesn't perturb the core parse (this is
// what lets `current` -- PROJECT_VER, which for a beta build is literally
// "1.1.0-beta.1" -- flow through the same sscanf as `latest` with no special
// casing). Any core component that fails to parse as a number for EITHER
// string is treated as "not newer" -- an unexpected tag format must never
// trigger a spurious update.
//
// Equal cores fall through to the prerelease tiebreak: a build with no
// "-beta.N" suffix outranks one that has it (a stable release beats any
// prerelease of the same core version -- this is what lets a device parked
// on a beta graduate onto the matching stable release automatically, and,
// the other direction, is why a device already ON stable X.Y.Z never
// "upgrades" to a beta of that same X.Y.Z even with the beta toggle on).
// Between two betas of the same core, higher N wins.
static bool is_newer(const char *latest, const char *current)
{
    int am = 0, an = 0, ap = 0, bm = 0, bn = 0, bp = 0;
    if (sscanf(latest, "%d.%d.%d", &am, &an, &ap) < 1)  return false;
    if (sscanf(current, "%d.%d.%d", &bm, &bn, &bp) < 1) return false;
    if (am != bm) return am > bm;
    if (an != bn) return an > bn;
    if (ap != bp) return ap > bp;

    bool a_beta, b_beta;
    int  a_n, b_n;
    parse_beta_suffix(latest, &a_beta, &a_n);
    parse_beta_suffix(current, &b_beta, &b_n);
    if (a_beta != b_beta) return !a_beta;   // one's a prerelease, the other isn't
    if (!a_beta) return false;              // both plain, equal core -> not newer
    return a_n > b_n;
}

/* ---- version-check HTTP GET -------------------------------------------- */

typedef struct { char *buf; int len; int cap; bool overflow; } check_resp_t;

static esp_err_t on_check_http(esp_http_client_event_t *e)
{
    if (e->event_id != HTTP_EVENT_ON_DATA || e->data_len <= 0) return ESP_OK;
    check_resp_t *r = e->user_data;
    if (r->overflow) return ESP_OK;

    int newlen = r->len + e->data_len;
    if (newlen + 1 > CHECK_BUF_CAP) { r->overflow = true; return ESP_OK; }
    if (newlen + 1 > r->cap) {
        int newcap = newlen * 2 + 512;
        if (newcap > CHECK_BUF_CAP) newcap = CHECK_BUF_CAP;
        char *nb = realloc(r->buf, newcap);
        if (!nb) { r->overflow = true; return ESP_OK; }
        r->buf = nb;
        r->cap = newcap;
    }
    memcpy(r->buf + r->len, e->data, e->data_len);
    r->len = newlen;
    r->buf[r->len] = 0;
    return ESP_OK;
}

// Extracts + TAG_PREFIX-strips a release object's version tag into `out`
// (sized like dial_ota_info_t.latest). False (leaving *out untouched) if the
// object has no usable tag_name -- callers treat that as "malformed
// release" (this is only ever called on a single, already-identified
// release object -- /releases/latest, or the one release check_beta()
// fetched by tag).
static bool release_version(cJSON *rel, char *out, size_t out_sz)
{
    cJSON *tag = cJSON_GetObjectItem(rel, "tag_name");
    if (!cJSON_IsString(tag) || !tag->valuestring) return false;
    const char *ver = tag->valuestring;
    if (!strncmp(ver, TAG_PREFIX, strlen(TAG_PREFIX))) ver += strlen(TAG_PREFIX);
    strlcpy(out, ver, out_sz);
    return true;
}

// Extracts + TAG_PREFIX-validates a tags-list entry's "name" field into
// `out` (sized like dial_ota_info_t.latest). False (leaving *out untouched,
// caller skips the entry) for anything that isn't a Somnus release tag --
// unlike release_version() above, this REQUIRES the prefix: the tags
// endpoint lists every git tag in the repo, including the pre-fork
// "dial-v*" lineage (TAG_PREFIX's own comment), and those must never enter
// the beta-channel comparison at all, not just lose it on an unparseable
// core.
static bool tag_version(cJSON *tag, char *out, size_t out_sz)
{
    cJSON *name = cJSON_GetObjectItem(tag, "name");
    if (!cJSON_IsString(name) || !name->valuestring) return false;
    const char *ver = name->valuestring;
    size_t plen = strlen(TAG_PREFIX);
    if (strncmp(ver, TAG_PREFIX, plen) != 0) return false;
    strlcpy(out, ver + plen, out_sz);
    return true;
}

// One GET against the GitHub API, using this component's one HTTP client
// setup (cert_pem, user_agent, Accept header) -- shared by the stable
// channel's /releases/latest fetch and the beta channel's two-request path
// below, so there is exactly one place that builds the request. Fills `r`
// with the (possibly capped) response body; returns the HTTP status code,
// or -1 on a transport-level error (*err_out, if non-NULL, carries why).
static int ota_http_get(const char *url, const char *user_agent, check_resp_t *r, esp_err_t *err_out)
{
    esp_http_client_config_t cfg = {
        .url           = url,
        .event_handler = on_check_http,
        .user_data     = r,
        .cert_pem      = trust_roots_pem_start,
        .user_agent    = user_agent,   // required by the GitHub API
        .timeout_ms    = 15000,
    };
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    esp_http_client_set_header(c, "Accept", "application/vnd.github+json");
    esp_err_t err = esp_http_client_perform(c);
    int status = (err == ESP_OK) ? esp_http_client_get_status_code(c) : -1;
    esp_http_client_cleanup(c);
    if (err_out) *err_out = err;
    return status;
}

// Shared tail once a single release object has been chosen -- the
// /releases/latest object for the stable channel, or the fetched-by-tag
// object check_beta() picked below: pull the ASSET_NAME download URL from
// assets[], compare `latest` against the running version, and set the
// final status. Verbatim from the pre-fix single-request version of this
// function (docs/REPORT-beta-fix.md) -- factored out so the beta channel's
// extra HTTP round trip changes nothing about how a chosen release is
// actually consumed.
static bool finish_from_release(cJSON *chosen, const char *latest, const esp_app_desc_t *desc)
{
    char asset_url[sizeof(s_asset_url)] = { 0 };
    cJSON *assets = cJSON_GetObjectItem(chosen, "assets");
    cJSON *a;
    cJSON_ArrayForEach(a, assets) {
        cJSON *name = cJSON_GetObjectItem(a, "name");
        if (!cJSON_IsString(name) || strcmp(name->valuestring, ASSET_NAME) != 0) continue;
        cJSON *url = cJSON_GetObjectItem(a, "browser_download_url");
        if (cJSON_IsString(url)) strlcpy(asset_url, url->valuestring, sizeof(asset_url));
        break;
    }

    if (!asset_url[0]) {
        set_status(OTA_FAILED, latest, "no " ASSET_NAME " asset in latest release");
        return false;
    }
    if (is_newer(latest, desc->version)) {
        taskENTER_CRITICAL(&s_mux);
        strlcpy(s_asset_url, asset_url, sizeof(s_asset_url));
        taskEXIT_CRITICAL(&s_mux);
        ESP_LOGI(TAG, "latest %s, running %s -- update available", latest, desc->version);
        set_status(OTA_AVAILABLE, latest, NULL);
        return true;
    }
    ESP_LOGI(TAG, "latest %s, running %s -- up to date", latest, desc->version);
    set_status(OTA_IDLE, latest, NULL);
    return true;
}

// Stable channel: the one /releases/latest object. Unchanged from before
// the beta-channel fix (docs/REPORT-beta-fix.md) apart from the HTTP-client
// setup moving into ota_http_get, shared with the beta channel below.
static bool check_stable(const esp_app_desc_t *desc, const char *user_agent)
{
    check_resp_t r = { 0 };
    esp_err_t err;
    int status = ota_http_get(GITHUB_API_URL, user_agent, &r, &err);

    // GitHub 404s /releases/latest when the repo has zero published
    // releases -- expected right now for the freshly created
    // matthewclaude/somnus-dial-releases repo, not a check failure. Report
    // it exactly like "checked, nothing newer" rather than an error state,
    // and don't fall back to any other repo.
    if (err == ESP_OK && status == 404) {
        ESP_LOGI(TAG, "no releases published yet (HTTP 404)");
        set_status(OTA_IDLE, NULL, NULL);
        free(r.buf);
        return true;
    }

    if (err != ESP_OK || status != 200 || !r.buf) {
        ESP_LOGW(TAG, "release check failed: %s (HTTP %d)", esp_err_to_name(err), status);
        char msg[96];
        snprintf(msg, sizeof(msg), "check failed (HTTP %d)", status);
        set_status(OTA_FAILED, NULL, msg);
        free(r.buf);
        return false;
    }
    if (r.overflow) {
        ESP_LOGW(TAG, "release JSON exceeded %d bytes", CHECK_BUF_CAP);
        set_status(OTA_FAILED, NULL, "release JSON too large");
        free(r.buf);
        return false;
    }

    cJSON *root = cJSON_Parse(r.buf);
    free(r.buf);
    if (!root) {
        set_status(OTA_FAILED, NULL, "bad release JSON");
        return false;
    }

    char latest[16];
    if (!release_version(root, latest, sizeof(latest))) {
        set_status(OTA_FAILED, NULL, "no tag_name in release");
        cJSON_Delete(root);
        return false;
    }

    bool ok = finish_from_release(root, latest, desc);
    cJSON_Delete(root);
    return ok;
}

// Beta channel (docs/REPORT-beta-fix.md, 2026-09-03 finding -- see
// GITHUB_API_URL_TAGS's own comment for why the list endpoint can't be
// trusted). Two requests, bounded and order-independent:
//  1. GET the tags list (one page, per_page=50) and find the single
//     highest Somnus release tag anywhere in it -- order is never assumed.
//  2. If that tag isn't even newer than the running version, stop: one
//     request, no release object ever fetched.
//  3. Otherwise GET that one release by tag name. If it 404s or is a
//     draft, retry with the next-highest UNTRIED tag, up to
//     OTA_BETA_CANDIDATE_CAP attempts, then continue into the same
//     finish_from_release() tail the stable channel uses.
static bool check_beta(const esp_app_desc_t *desc, const char *user_agent)
{
    check_resp_t r = { 0 };
    esp_err_t err;
    int status = ota_http_get(GITHUB_API_URL_TAGS, user_agent, &r, &err);

    if (err != ESP_OK || status != 200 || !r.buf) {
        ESP_LOGW(TAG, "tag list check failed: %s (HTTP %d)", esp_err_to_name(err), status);
        char msg[96];
        snprintf(msg, sizeof(msg), "check failed (HTTP %d)", status);
        set_status(OTA_FAILED, NULL, msg);
        free(r.buf);
        return false;
    }
    if (r.overflow) {
        ESP_LOGW(TAG, "tag list JSON exceeded %d bytes", CHECK_BUF_CAP);
        set_status(OTA_FAILED, NULL, "tag list JSON too large");
        free(r.buf);
        return false;
    }

    cJSON *root = cJSON_Parse(r.buf);
    free(r.buf);
    if (!root) {
        set_status(OTA_FAILED, NULL, "bad tag list JSON");
        return false;
    }
    if (!cJSON_IsArray(root)) {
        set_status(OTA_FAILED, NULL, "tag list JSON not an array");
        cJSON_Delete(root);
        return false;
    }
    int n = cJSON_GetArraySize(root);

    // Step 1: the single highest Somnus release tag anywhere in the page --
    // order is irrelevant and never relied on (see the finding above).
    char highest[16] = { 0 };
    bool have_highest = false;
    for (int i = 0; i < n; i++) {
        cJSON *tag = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsObject(tag)) continue;
        char ver[16];
        if (!tag_version(tag, ver, sizeof(ver))) continue;   // not a somnus-v* tag
        if (!have_highest || is_newer(ver, highest)) {
            strlcpy(highest, ver, sizeof(highest));
            have_highest = true;
        }
    }
    if (!have_highest) {
        ESP_LOGI(TAG, "no somnus-v tags, running %s -- up to date", desc->version);
        set_status(OTA_IDLE, NULL, NULL);
        cJSON_Delete(root);
        return true;
    }

    // Step 2: one request total if the highest tag isn't even newer than
    // what's running -- no release object ever fetched.
    if (!is_newer(highest, desc->version)) {
        ESP_LOGI(TAG, "latest %s, running %s -- up to date", highest, desc->version);
        set_status(OTA_IDLE, highest, NULL);
        cJSON_Delete(root);
        return true;
    }

    // Step 3: fetch the chosen tag's release object; on a 404 or a draft,
    // fall back to the next-highest tag not already tried and try again,
    // up to OTA_BETA_CANDIDATE_CAP times total.
    char tried[OTA_BETA_CANDIDATE_CAP][16];
    int  n_tried = 0;
    char candidate[16];
    strlcpy(candidate, highest, sizeof(candidate));
    bool ok = false;

    for (int attempt = 0; attempt < OTA_BETA_CANDIDATE_CAP; attempt++) {
        strlcpy(tried[n_tried++], candidate, sizeof(tried[0]));

        char full_tag[24];
        snprintf(full_tag, sizeof(full_tag), "%s%s", TAG_PREFIX, candidate);
        char url[160];
        snprintf(url, sizeof(url), GITHUB_API_URL_RELEASE_BY_TAG_FMT, full_tag);

        check_resp_t rr = { 0 };
        esp_err_t rerr;
        int rstatus = ota_http_get(url, user_agent, &rr, &rerr);

        cJSON *rel = NULL;
        bool usable = false;
        if (rerr == ESP_OK && rstatus == 404) {
            ESP_LOGW(TAG, "release for %s not found (HTTP 404) -- trying next tag", full_tag);
        } else if (rerr != ESP_OK || rstatus != 200 || !rr.buf) {
            ESP_LOGW(TAG, "release fetch for %s failed: %s (HTTP %d) -- trying next tag",
                     full_tag, esp_err_to_name(rerr), rstatus);
        } else if (rr.overflow) {
            ESP_LOGW(TAG, "release JSON for %s exceeded %d bytes -- trying next tag",
                     full_tag, CHECK_BUF_CAP);
        } else {
            rel = cJSON_Parse(rr.buf);
            if (!rel) {
                ESP_LOGW(TAG, "bad release JSON for %s -- trying next tag", full_tag);
            } else if (cJSON_IsTrue(cJSON_GetObjectItem(rel, "draft"))) {
                ESP_LOGW(TAG, "%s is a draft -- trying next tag", full_tag);
            } else {
                usable = true;
            }
        }
        free(rr.buf);

        if (usable) {
            ok = finish_from_release(rel, candidate, desc);
            cJSON_Delete(rel);
            break;
        }
        if (rel) cJSON_Delete(rel);

        // Next-highest tag not already tried, if any -- re-scanned from
        // `root` rather than precomputed, so this is always the true
        // runner-up regardless of how many attempts have already failed.
        char next[16] = { 0 };
        bool have_next = false;
        for (int i = 0; i < n; i++) {
            cJSON *tag = cJSON_GetArrayItem(root, i);
            if (!cJSON_IsObject(tag)) continue;
            char ver[16];
            if (!tag_version(tag, ver, sizeof(ver))) continue;
            bool already_tried = false;
            for (int t = 0; t < n_tried; t++)
                if (strcmp(ver, tried[t]) == 0) { already_tried = true; break; }
            if (already_tried) continue;
            if (!have_next || is_newer(ver, next)) {
                strlcpy(next, ver, sizeof(next));
                have_next = true;
            }
        }
        if (!have_next) break;
        strlcpy(candidate, next, sizeof(candidate));
    }

    cJSON_Delete(root);
    if (!ok) set_status(OTA_FAILED, NULL, "no usable release for newest tags");
    return ok;
}

void dial_ota_mark_checking(void) { set_status(OTA_CHECKING, NULL, NULL); }

bool dial_ota_check(bool beta)
{
    set_status(OTA_CHECKING, NULL, NULL);

    const esp_app_desc_t *desc = esp_app_get_description();
    char user_agent[40];
    snprintf(user_agent, sizeof(user_agent), "somnus-dial/%s", desc->version);

    return beta ? check_beta(desc, user_agent) : check_stable(desc, user_agent);
}

void dial_ota_set_blocked(const char *reason)
{
    set_status(OTA_FAILED, NULL, reason);
}

/* ---- download + apply ---------------------------------------------------*/

bool dial_ota_download_and_apply(void (*progress_cb)(int pct))
{
    char asset_url[sizeof(s_asset_url)];
    taskENTER_CRITICAL(&s_mux);
    strlcpy(asset_url, s_asset_url, sizeof(asset_url));
    taskEXIT_CRITICAL(&s_mux);

    set_status(OTA_DOWNLOADING, NULL, NULL);
    set_progress(0);

    if (!asset_url[0]) {
        set_status(OTA_FAILED, NULL, "no update URL (check() never ran or found none)");
        return false;
    }

    esp_http_client_config_t http_cfg = {
        .url        = asset_url,
        .cert_pem   = trust_roots_pem_start,
        .timeout_ms = 30000,
        .buffer_size = 4096,
        /* GitHub 302s the asset to a signed URL ~900 bytes long; the default
         * 512-byte TX buffer can't even fit the redirected request line. */
        .buffer_size_tx = 4096,
    };
    esp_https_ota_config_t ota_cfg = { .http_config = &http_cfg };

    esp_https_ota_handle_t handle = NULL;
    // One automatic retry before surfacing a failure. esp_https_ota_begin
    // opens its own TLS session (with larger buffers than the rest of this
    // firmware uses), and a first attempt was observed failing where an
    // immediate retry succeeded -- most likely another session still winding
    // down. A user watching a confirmed install should not be told "download
    // start failed" for something that clears itself in a second; a genuinely
    // unreachable server still fails, just twice.
    esp_err_t err = ESP_FAIL;
    for (int attempt = 0; attempt < 2; attempt++) {
        err = esp_https_ota_begin(&ota_cfg, &handle);
        if (err == ESP_OK) break;
        ESP_LOGW(TAG, "esp_https_ota_begin: %s%s", esp_err_to_name(err),
                 attempt == 0 ? " -- retrying once" : "");
        if (attempt == 0) vTaskDelay(pdMS_TO_TICKS(1500));
    }
    if (err != ESP_OK) {
        char msg[96];
        snprintf(msg, sizeof(msg), "download start failed: %s", esp_err_to_name(err));
        set_status(OTA_FAILED, NULL, msg);
        return false;
    }

    int image_size = esp_https_ota_get_image_size(handle);
    int last_pct = -1;
    while ((err = esp_https_ota_perform(handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
        int read = esp_https_ota_get_image_len_read(handle);
        int pct = (image_size > 0) ? (int)(((int64_t)read * 100) / image_size) : 0;
        if (pct != last_pct) {
            last_pct = pct;
            set_progress(pct);
        }
        if (progress_cb) progress_cb(pct);
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_perform: %s", esp_err_to_name(err));
        esp_https_ota_abort(handle);
        char msg[96];
        snprintf(msg, sizeof(msg), "download failed: %s", esp_err_to_name(err));
        set_status(OTA_FAILED, NULL, msg);
        return false;
    }

    if (!esp_https_ota_is_complete_data_received(handle)) {
        ESP_LOGE(TAG, "incomplete image received");
        esp_https_ota_abort(handle);
        set_status(OTA_FAILED, NULL, "incomplete image received");
        return false;
    }

    // esp_https_ota_finish() cleans up the handle regardless of its return
    // value -- esp_https_ota_abort() must NOT be called after this point
    // (see esp_https_ota.h's note beside esp_https_ota_abort).
    err = esp_https_ota_finish(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_finish: %s", esp_err_to_name(err));
        char msg[96];
        snprintf(msg, sizeof(msg), "image validation failed: %s", esp_err_to_name(err));
        set_status(OTA_FAILED, NULL, msg);
        return false;
    }

    ESP_LOGI(TAG, "OTA image written and verified; ready to reboot");
    set_progress(100);
    set_status(OTA_READY_REBOOT, NULL, NULL);
    return true;
}

/* ---- rollback health check ----------------------------------------------*/

void dial_ota_init(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    bool pending = false;
    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK) {
        ESP_LOGW(TAG, "esp_ota_get_state_partition failed at boot; assuming not pending");
    } else {
        pending = (ota_state == ESP_OTA_IMG_PENDING_VERIFY);
    }
    taskENTER_CRITICAL(&s_mux);
    s_info.pending_verify = pending;
    taskEXIT_CRITICAL(&s_mux);
    ESP_LOGI(TAG, "boot pending-verify: %s", pending ? "true (rollback armed)" : "false");
}

void dial_ota_mark_valid_if_pending(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) != ESP_OK) {
        ESP_LOGW(TAG, "esp_ota_get_state_partition failed; leaving rollback state as-is");
        return;
    }
    if (ota_state != ESP_OTA_IMG_PENDING_VERIFY) {
        ESP_LOGI(TAG, "boot not pending verification (state %d) -- nothing to do", ota_state);
        return;
    }
    if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
        ESP_LOGI(TAG, "app marked valid; rollback cancelled");
        // Clears the mirror so scr_connecting/scr_dial's "Finalizing update"
        // notice drops as soon as the caller (main.c) re-commits the OTA
        // snapshot -- pending_verify must never stay true once the
        // bootloader has actually stopped tracking a rollback.
        taskENTER_CRITICAL(&s_mux);
        s_info.pending_verify = false;
        taskEXIT_CRITICAL(&s_mux);
    } else {
        ESP_LOGE(TAG, "failed to mark app valid / cancel rollback");
    }
}
