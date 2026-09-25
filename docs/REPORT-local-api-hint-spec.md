# REPORT: local-api-hint spec — SPEC-local-api-hint written and committed (1.0.4-beta.1 plan, spec only)

**DONE** — the gate passed, and `docs/SPEC-local-api-hint.md` was committed alone as `8607cef`. Both settled hint lines fit
on one line each at 320 px (250 px and 249 px), and the worst case is exactly 4 lines.
No firmware, simulator, README or CHANGELOG changes were made. Nothing was pushed.

Date: 2026-09-25. Base: `335b8af`.

## Gate

1. `git --no-optional-locks status --short --untracked-files=no`

   ```
   ```
   (empty — PASS)

2. `sed -n 21p firmware/dial-idf/CMakeLists.txt`

   ```
   set(PROJECT_VER "1.0.2")
   ```
   PASS. The value, verbatim: `1.0.2`.

3. `git grep -n 'set_error("http error: %s", esp_err_to_name(err));' firmware/dial-idf/components/dial_somnus/dial_somnus.c`

   ```
   firmware/dial-idf/components/dial_somnus/dial_somnus.c:117:        set_error("http error: %s", esp_err_to_name(err));
   ```
   Exactly one hit — PASS.

## Commit

SHA `8607ceffd6692557e583324afb1370f5243ec8dd`. Message: `docs: SPEC-local-api-hint - plain-language hint when the pad does not answer (1.0.4-beta.1 plan, spec only)`.

`git show --stat --format= HEAD`:

```
 docs/SPEC-local-api-hint.md | 320 ++++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 320 insertions(+)
```

The tracked tree was clean after the commit. The only public-repo hygiene hit is the pad API spec's example
address (`docs/local_api.yml:22`), which appears in the quote of the simulator string.
There are no home paths, emails, MACs or names in the spec.

## Consumers (at `335b8af`, firmware paths under `firmware/dial-idf/`)

`dial_somnus_last_error()`:

- `main/main.c:960` — `CMD_PAD_SETTINGS_CHANGED` re-probe failed → PH_DEGRADED
- `main/main.c:1065` — pre-READY connect loop → PH_DEGRADED (the fresh-device path)
- `main/main.c:1418` — steady state, 3 consecutive poll failures → PH_DEGRADED
- `components/dial_somnus/dial_somnus.c:42` (definition), `dial_somnus.h:144` (declaration), `dial_somnus.h:107`, `dial_somnus.c:85`, `components/dial_ui/scr_connecting.c:100` (comments)
- docs-only hits: `docs/REPORT-standby-poll-code-checks.md:65`, `docs/SPEC-connect-phases.md:115,245,260,494`, `docs/SPEC-pad-discovery.md:664,693` (quoted code, not code)

`phase_err`:

- `components/dial_state/dial_state.h:378` (the field, 128 bytes), `dial_state.c:927` (strlcpy in `dial_state_set_phase`; NULL keeps the old text), `dial_state.h:339-348` (cert comments)
- `components/dial_ui/scr_connecting.c:103-104` (PH_DEGRADED echo), `:94`, `:100` (comments)
- `components/dial_ui/scr_pad_discovery.c:97-116` (progress parser, splits on `\n`), `:15`, `:78-79` (comments)
- `components/dial_pad_discovery/dial_pad_discovery.c:330` (progress writer)
- `simulator/main.c:240-244`, `:832`, `:853`; `simulator/sim_state.c:223-224`, `:285-286`
- docs-only: `docs/PLAN-screen-layout-fixes.md:75,77`, `docs/REPORT-layout-phase1.md:44,46,50,338`, `docs/SPEC-connect-phases.md:43,252,276,455`, `docs/SPEC-pad-discovery.md:452,493,503,702`

`http error` prefix: code hits are only `dial_somnus.c:117` (the edited line) and
`tools/dial_display_audit.py:101` (the audit tool's own text). The rest are
quoted logs in `docs/REPORT-1.0.1-bench-gate.md` and
`docs/REPORT-1.0.2-bench-items-4-5.md`. Nothing compares against the prefix.

## Line count and wrap check

Byte count (`printf %s ... | wc -c`): line 1 = 31, line 2 = 29, so the hint is 61 bytes with
the `\n`. The worst-case subtitle is 61 + 16 + 20 = 97 bytes, which is under `phase_err[128]` and
`sub_txt[160]`.

Pixel check: a throwaway program in the session scratchpad (not in the repo),
built against the firmware's pinned LVGL 8.4.0 checkout with
`simulator/lv_conf.h`, calling `lv_txt_get_size(&p, text,
&lv_font_montserrat_16, 0, 0, maxw, LV_TEXT_FLAG_NONE)`. Lines = height / 18.
Raw output:

```
line_height=18
w=250 h= 18 lines=1  maxw=8191  "Is the pad's Local API enabled?"
w=249 h= 18 lines=1  maxw=8191  "Somnus support can turn it on"
w=123 h= 18 lines=1  maxw=8191  "Retrying in 60s"
w=111 h= 18 lines=1  maxw=8191  "Retrying in 5s"
w= 83 h= 18 lines=1  maxw=8191  "Retrying..."
w=160 h= 18 lines=1  maxw=8191  "Swipe left for menu"
w=301 h= 18 lines=1  maxw=8191  "http error: ESP_ERR_HTTP_CONNECT"
w=472 h= 18 lines=1  maxw=8191  "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)"
-- at 320:
w=250 h= 18 lines=1  maxw=320  "Is the pad's Local API enabled?"
w=249 h= 18 lines=1  maxw=320  "Somnus support can turn it on"
w=123 h= 18 lines=1  maxw=320  "Retrying in 60s"
w=111 h= 18 lines=1  maxw=320  "Retrying in 5s"
w= 83 h= 18 lines=1  maxw=320  "Retrying..."
w=160 h= 18 lines=1  maxw=320  "Swipe left for menu"
w=301 h= 18 lines=1  maxw=320  "http error: ESP_ERR_HTTP_CONNECT"
w=305 h= 36 lines=2  maxw=320  "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)"
w=250 h= 72 lines=4  maxw=320  "Is the pad's Local API enabled?\nSomnus support can turn it on\nRetrying in 60s\nSwipe left for menu"
w=250 h= 72 lines=4  maxw=320  "Is the pad's Local API enabled?\nSomnus support can turn it on\nRetrying in 27s\nSwipe left for menu"
w=250 h= 72 lines=4  maxw=320  "Is the pad's Local API enabled?\nSomnus support can turn it on\nRetrying...\nSwipe left for menu"
w=301 h= 54 lines=3  maxw=320  "http error: ESP_ERR_HTTP_CONNECT\nRetrying in 27s\nSwipe left for menu"
w=305 h= 72 lines=4  maxw=320  "Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)\nRetrying in 27s\nSwipe left for menu"
```

Neither hint line wraps. The worst case is exactly 4 lines (72 px), which is the
maximum documented at `scr_connecting.c:31-36`. No rewording is proposed.

## Deviations from the task

- **The troubleshooting entry is in `firmware/dial-idf/README.md:211-218`,
  not the top-level `README.md`.** The top-level README has only the
  requirements bullet (`README.md:39-43`). The spec plans a reword of both
  and names each file.
- **Additions to the spec that the task did not list.** None of them changes the settled text.
  - A note that the apostrophe must stay ASCII.
  - A note that `set_error()` also `ESP_LOGW`s the hint, so the second hint line prints as an
    untagged serial line. The spec accepts this.
  - The raw log line keeps the `http error: %s` form, so old bench-log greps still match.
- **One finding recorded as out of scope:**
  - `main.c:1055` sets PH_PAD_DISCOVERY with a NULL error, so on the second and later scans
    `scr_pad_discovery.c` shows the stale PH_DEGRADED text as its headline for one tick.
  - This happens today with the raw error. After the change it would show hint line 1.
  - It is not a regression. A one-line fix is noted for a later beta.
- `docs/REPORTS.md` was not edited. Per the task, this report's line goes in
  the next docs commit.

Otherwise none.

## Cannot be verified without hardware

- The on-device rendering: both hint lines unwrapped on the real panel, bezel
  clearance, and no overlap between the headline and the subtitle (the host LVGL measurement
  and the planned simulator PNG are proxies).
- That the serial log still carries `dial_somnus: http error: ESP_ERR_HTTP_...`
  after the change.
- The actual trigger: a pad with its Local API off. Only Somnus support can
  switch it, so the spec's bench uses an unplugged pad instead. That stimulus
  needs the owner, because the pad is a real bed in use.
- That a pad with its API off refuses :8080 at the transport level and does not answer
  with an HTTP status (taken from the user's screen). If it did answer, it would
  show `pad returned HTTP %d`, not the hint.
- Sequencing note: the `somnus-v1.0.3-beta.1` tag does not exist locally or
  on `somnus` (`git tag -l`, `git ls-remote --tags somnus`). The code for this
  change must wait for it.
