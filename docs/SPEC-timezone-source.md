# Spec: Timezone source (captive portal)

Status: **APPROVED AND IN BUILD (2026-09-01).**

## Problem

`components/dial_time/` (SNTP + an embedded posix_tz_db table + NVS
persistence) is complete and working, but `dial_time_set_iana_tz()` is never
called from anywhere outside the component. The header's own comment says the
zone is confirmed by "the account" — that was the cloud account reached
through the OAuth/MCP pipeline this port deleted. The timezone's only source
died with it and nothing replaced it, so the clock has always shown UTC.

Knock-on effect: `dial_time_valid()` returns `s_synced && s_tz_set`, so with
`s_tz_set` permanently false it is always false, and anything gated on a
valid clock never runs — including the OTA auto-check's local-time logic
around `main.c:1159`.

## Mechanism

The Wi-Fi setup captive portal (`components/dial_net/dial_wifi.c`) is already
served through a real browser during provisioning, and that browser already
knows the answer: `Intl.DateTimeFormat().resolvedOptions().timeZone` (e.g.
`"America/Chicago"`). Add it as a hidden field on the portal form
(`root_get()`), filled by a small inline `<script>` — the page already
tolerates inline JS (the network-`<select>`'s `onchange` handler) and already
degrades gracefully with `<noscript>` for the one place that needs it, so a
browser with JS off (or without `Intl`) just submits an empty field, handled
below. Zero new UI, no IANA picker on a rotary knob.

`save_post()` parses the submitted `tz` field the same manual way it already
parses `ssid`/`other`/`pass`, bounds and validates it (see Input bounding),
and — if it survives that — calls `dial_time_set_iana_tz()` **directly**,
synchronously, in the httpd handler.

**Threading — direct call, no command queue.** `dial_time.h` states no
worker-task-only contract (unlike `dial_somnus.h`/`dial_mcp`'s explicit
"call only from the worker task... not reentrant" for their shared,
non-reentrant HTTP client). `dial_time_set_iana_tz()`'s only shared state is
two `volatile bool`s, an NVS handle opened/closed within the call (already
this file's own pattern via `save_creds()`), and libc `setenv`/`tzset`. No
concurrent reader of TZ state exists during the portal window:
`dial_time_start()` — the only thing that starts SNTP and thus the only
reason `dial_time_valid()` could ever gate a reader onto fresh TZ state —
has not run yet at this point in `main.c`'s boot sequence (`dial_net_bringup()`
runs first). Posting through the existing `dial_cmd_post`/`handle_immediate_cmd`
queue instead would also be actively wrong here: that queue isn't drained
until the steady-state loop, reachable only after Wi-Fi *and* the Somnus pad
connect both succeed — long after setup should have taken effect.

**Ordering — verified, not assumed.** `dial_time_start()` calls
`restore_posix_tz()` (reads NVS key `"posix_tz"`) *before* starting SNTP.
Since `save_post()` runs synchronously on the httpd task and completes
(including the `dial_time_set_iana_tz()` call) strictly before
`sta_connect()` succeeds and `dial_net_bringup()` returns to `worker_task`,
by the time `main.c` calls `dial_time_start()` the NVS value is already
whatever the portal just wrote. `restore_posix_tz()` then just re-reads and
re-applies that same value — redundant, not a race, not an overwrite.

## Failure behavior

`dial_time_set_iana_tz()` returns false on empty, unknown, or malformed
input, and every false path returns before `apply_posix_tz()`/
`persist_posix_tz()` run — failure is a strict no-op, never worse than no
zone at all. The portal handler must never gate credential saving or Wi-Fi
connection on this call's return value; provisioning succeeds or fails on
the Wi-Fi credentials alone.

## Input bounding

`tz` is the first browser-supplied field to reach `dial_time`. Inside
`dial_time_set_iana_tz()` itself, the `snprintf` search-needle bound already
makes an oversized value a safe no-op. But `save_post()`'s own body parsing
is manual `strstr` into fixed local/static buffers with no library form
parser behind it, so the bound belongs there too, on `save_post()`'s own
terms: the decoded value is capped to a fixed local buffer the same way
`ssid`/`other`/`pass` already are, and every character is checked against
what an IANA zone name can actually contain (`A-Z a-z 0-9 / _ + -`) before
`dial_time_set_iana_tz()` is called at all — anything else is treated as no
timezone submitted. This runs in its own isolated block, the same shape as
the existing `ssid=`/`other=`/`pass=` blocks, so it cannot disturb their
parsing regardless of field order or length.

## Settings → Timezone row (added 2026-09-01)

The on-device backstop named above as a follow-up, now in scope. Two reasons
it's needed, not one:

1. `dial_time_set_iana_tz()` had exactly one caller — the portal handler
   above. A dial that is **already provisioned** cannot reach the portal
   without a factory reset, so an existing device had no way to set its
   timezone at all.
2. It closes the on-device-picker gap this spec already named:
   `dial_net_submit_creds()` (`SCR_NETPICK`, the dial's own "Set up on the
   dial" fallback) is a second entry point into the same provisioning state
   machine, used when no phone is involved. It never touches a browser or
   JS, so a device provisioned entirely on-device got no timezone from the
   portal mechanism above, no matter what. The Settings row is the fix for
   that device, not a workaround.

### A curated list, not the full table

Settings shows 11 fixed choices (exact IANA key, friendly label) — not the
~400-zone table `dial_time` embeds. `dial_list` (this UI's rotor-list
widget) walks one row per detent; ~400 rows is not a knob-scrollable list,
and a short, US/UK/EU/AU-weighted set covers essentially every real user.

    America/New_York       Eastern
    America/Chicago        Central
    America/Denver         Mountain
    America/Phoenix        Arizona
    America/Los_Angeles    Pacific
    America/Anchorage      Alaska
    Pacific/Honolulu       Hawaii
    Europe/London          UK
    Europe/Paris           Central Europe
    Australia/Sydney       Sydney
    Etc/UTC                UTC

Every IANA string was checked against `dial_time`'s embedded `zones.csv`
directly (not assumed) before this list was finalized. One did not resolve
as originally proposed: **`"UTC"` alone is not a key in the table** — only
`"Etc/UTC","UTC0"` is. The row uses `Etc/UTC` for that reason; the label
stays "UTC".

**Revised 2026-09-01, before first flash:** the list originally also
carried `Europe/Berlin` labelled "Central Europe" — identical to
`Europe/Paris`'s label, and not by coincidence: the two share both offset
and DST rules (see the collision noted below), so the entry added a
duplicate row a knob-scrolled list can't tell apart, for no coverage gain.
Dropped; `DIAL_TZ_COUNT` is 11, not 12. Also shortened "Arizona (no DST)" to
plain **"Arizona"** — it was the value column's longest string (16 chars) in
a screen using a fixed single-line row rather than the stacked-label
treatment Pad Address/Adjustment Mode use for their longer values;
`America/Phoenix` is the only US zone without DST, which the name itself
already implies to anyone selecting it deliberately.

**This is a convenience layer, not a restriction.** The full posix_tz_db
table stays embedded in `dial_time` exactly as it was, and the Wi-Fi portal
(above) still accepts whatever IANA zone the browser reports, curated list
or not — the portal's coverage is not narrowed by this row existing. The
curated list only bounds what a knob can practically scroll through; it
does not bound what the device can be told.

### Threading — NOT the same case as the portal

This is the non-obvious part, and it's worth being explicit about why the
two call sites differ, because they call the exact same function.

The portal handler (above) was safe to call `dial_time_set_iana_tz()`
**directly**, synchronously, from the httpd task, because at that point in
boot `dial_time_start()` had not run yet — nothing anywhere was reading TZ
state, so the `setenv`/`tzset()` mutation inside it had no concurrent reader
to race.

A Settings row runs at steady state, on the LVGL task, *after*
`dial_time_start()` has long since run and `worker_task` is repeatedly
calling `dial_time_now()` (`main.c:907`, `:932`, `:1161`) — which calls
`localtime_r()`, which reads the same global libc TZ state `setenv`/
`tzset()` mutates. Calling `dial_time_set_iana_tz()` directly from the LVGL
task here would be a genuine, live data race between two tasks, unlike the
portal case. **The reasoning does not transfer just because the call looks
identical on the page** — what makes a call site safe is what else is
running at that moment, not which function it calls.

The fix: the row cannot persist/apply anything itself. It posts a new
`CMD_TZ_CHANGED` command (index into the curated table riding in `app_cmd_t`'s
existing generic `a` field) through the same UI→worker queue
`CMD_PAD_SETTINGS_CHANGED` uses, and `main.c`'s `handle_immediate_cmd` —
running on `worker_task`, the same task every `dial_time_now()` caller
already runs on — is the only place that actually calls
`dial_time_set_iana_tz()`. This is a different shape than
`CMD_PAD_SETTINGS_CHANGED`, which carries no payload: that command's UI-side
setter (`dial_state_set_pad_url()`) writes into dial_state's own
mutex-protected store directly and safely from the LVGL task, then the
command is just a "go re-read and re-apply" signal. Timezone has no
equivalent protected store standing in front of it — `dial_time` keeps its
state entirely outside dial_state — so the command has to carry the
choice itself, not just announce that something changed.

**Known latency gap, accepted, not fixed here:** the command queue is only
drained in the steady-state loop, unreachable until the pad connect retry
loop succeeds. A timezone change made from Settings while the pad is
unreachable sits queued until it connects — which, unlike the portal's
brief provisioning window, can be indefinite if the pad never comes up. This
is the exact same latency characteristic `CMD_PAD_SETTINGS_CHANGED` already
has for the Pad Address row, for the identical structural reason — not a
new problem this row introduces. The connect loop is not being restructured
to close it.

### Displaying the current value

`dial_time` persists the resolved POSIX string under NVS key `"posix_tz"`,
not the IANA name — there was no getter for "which zone is this" before this
row, and the row needs one.

Reverse-lookup from the POSIX string was considered and rejected — checked,
not assumed to be ambiguous: **`Europe/Paris` and `Europe/Berlin`, both in
the curated list as originally drafted, resolve to the byte-identical POSIX
string** (`CET-1CEST,M3.5.0,M10.5.0/3`). A reverse lookup cannot tell them
apart. This collision existed inside the curated list itself, not just
somewhere in the wider ~400-zone table — the same collision that later got
`Europe/Berlin` dropped outright (see the revision note above): two entries
sharing a POSIX string were always going to share a label too, in either
direction (as a stored value or as a display name), so removing the
duplicate fixed both problems at once.

An index into the curated list, persisted separately (e.g. in
`app_state_t`), was also rejected: it ties a persisted value to that list's
order and contents. Reordering or extending the list later (already
happened once in drafting this spec — see the UTC fix above) would silently
repoint an old persisted index at the wrong zone, and it would be a second,
independently-persisted copy of "which zone" alongside `dial_time`'s own
`posix_tz`, with nothing keeping the two in sync.

**Chosen approach:** `dial_time` persists the submitted IANA string itself,
in a second NVS key (`"iana_tz"`, same `"time"` namespace) written in the
same `dial_time_set_iana_tz()` call that already writes `"posix_tz"`,
restored alongside it at boot, and exposed through a new
`dial_time_get_iana_tz()` getter. Single source of truth, colocated with
where the TZ state already lives — the same shape `dial_net`'s existing
getters use (`dial_net_sta_ssid()`/`dial_net_ap_ssid()`/`dial_net_hostname()`
are already bare getters read directly from UI screens, outside the
`app_state_t` store, written from a different task with no mutex); shaped
specifically like `dial_net_ip()` (copy-out + bool return) rather than a
bare-pointer getter, since "never set" is a real, meaningful state here that
needs to be reported honestly, not papered over with an empty string.

The row shows, in priority order: the curated label if the persisted IANA
string matches one of the 11 entries; the raw IANA string itself if a zone
was set but doesn't match any curated entry (e.g. a portal-detected browser
zone outside this list, such as `Asia/Tokyo`) — never silently mapped to the
wrong row, never hidden; **"Not set"** only when `dial_time_get_iana_tz()`
reports nothing has ever been persisted, which is this device's current,
actual state.

## Out of scope

- Any compile-time or hardcoded default timezone — this project just
  finished neutralizing one baked-in personal constant (the pad IP); it does
  not need another.
- IP geolocation or any new network dependency.
