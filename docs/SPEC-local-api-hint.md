# SPEC — Plain-language hint when the pad does not answer (1.0.4-beta.1)

Status: written 2026-09-25 against `335b8af` (`PROJECT_VER` 1.0.2). Code
committed as `5e9bc69`. The §10 bench ran on 2026-09-30
(`docs/REPORT-apihint-bench.md`): **PASS**, with one correction to this
spec. On a dial that already holds pad state, the steady-state path keeps
the dial face with its staleness dot and never shows the hint; §4 and §10
step 3 are corrected below.

Scope rule in force: no new features, one change per beta, spec on disk
before code, hardware test before tag. The one change in 1.0.4-beta.1 is
**replacing the raw transport error on the "Pad unreachable" screen with a
two-line hint about the pad's Local API**.

All `file:line` references are at `335b8af`. Firmware paths are relative to
`firmware/dial-idf/` unless they start with `simulator/`, `docs/` or are the
top-level `README.md`.

---

## 1. Background

A user flashed the dial and joined Wi-Fi without trouble, then could not
connect to the pad because the pad's Local API was off. The dial showed:

```
Pad unreachable
http error: ESP_ERR_HTTP_CONNECT
Retrying in Ns
Swipe left for menu
```

`ESP_ERR_HTTP_CONNECT` is an ESP-IDF error name. It is right, but it does
not tell a user what to do next.

**Settled fact (2026-09-25):** today the Local API is turned on only by a
request to Somnus support. There is no switch in the Somnus app. Somnus has
said an app switch is planned.

**Rewording trigger:** when the Somnus app ships a Local API switch, the
second hint line ("Somnus support can turn it on") is out of date. It gets
reworded in its own beta, following the same rules as this one (spec first,
one change). Nothing else in this spec depends on that line.

## 2. The path a fresh device takes

A fresh device has no saved Pad Address, so it uses the compiled default.

1. `main.c:1015` — `PH_WIFI_CONNECTING`; `main.c:1016` `dial_net_bringup()`
   joins Wi-Fi (the part that worked for the user).
2. `main.c:1038` — `dial_state_get_pad_url()` returns the fresh-device
   default `DIAL_PAD_DEFAULT_BASE_URL` (`components/dial_state/dial_state.h:372`,
   the pad API spec's example address, `docs/local_api.yml:22`).
3. `main.c:1039-1040` — `PH_SOMNUS_CONNECTING`, then `dial_somnus_connect()`
   (`components/dial_somnus/dial_somnus.c:139`) → `dial_somnus_get_state()` →
   `do_request("/api/state", GET)` (`dial_somnus.c:179`) →
   `esp_http_client_perform()` (`dial_somnus.c:112`). Nothing is listening
   on :8080, so `err != ESP_OK`, and `dial_somnus.c:117` sets
   `s_last_error` to `"http error: ESP_ERR_HTTP_CONNECT"`.
4. `main.c:1054-1063` — first failure, so `dial_pad_discovery_should_attempt()`
   is true: `PH_PAD_DISCOVERY` (`main.c:1055`) and a subnet scan
   (`main.c:1057`). Each probe is its own `esp_http_client` in
   `components/dial_pad_discovery/dial_pad_discovery.c:245-253` and returns
   `false` on any failure **without touching `dial_somnus`'s `s_last_error`**.
   With the API off, nothing answers on :8080 anywhere, so `found` is false.
5. `main.c:1065` — `dial_state_set_phase(PH_DEGRADED, dial_somnus_last_error())`,
   so `phase_err` still carries step 3's string. `backoff_wait()`
   (`main.c:783-812`) counts `retry_in_s` down from `backoff_s` (5 s, doubling
   to a 60 s cap, `main.c:99-100`, `:1073`).
6. `components/dial_ui/scr_connecting.c:90-110` renders PH_DEGRADED:
   headline "Pad unreachable" (`:96`), subtitle = `phase_err` + "Retrying in
   Ns" (`:103-107`) + "Swipe left for menu" (`:132-136`).

The loop repeats steps 2-6, rescanning every 5 minutes. The user sees
exactly the screen in §1 until they give up.

## 3. The change

In `components/dial_somnus/dial_somnus.c`, the transport-failure branch
(`err != ESP_OK` after `esp_http_client_perform()`, `:116-120`):

- Sets a user-facing string instead of the raw esp_err name. **Settled
  wording**, two lines joined by `\n`:

  ```
  Is the pad's Local API enabled?
  Somnus support can turn it on
  ```

  The apostrophe is ASCII `'` (0x27). A typographic `’` is not in the
  Montserrat subset and would render as a missing glyph.
- Still logs the raw name. Emit `ESP_LOGW(TAG, "http error: %s",
  esp_err_to_name(err))` in the same form as today's log line, so earlier
  bench-log greps for `dial_somnus: http error: ESP_ERR_HTTP_CONNECT`
  (e.g. `docs/REPORT-1.0.2-bench-items-4-5.md:143`) still match and
  debugging loses nothing.
- `set_error()` (`dial_somnus.c:31-38`) also logs its string with `ESP_LOGW`,
  so the hint prints too. The embedded `\n` makes the second hint line a
  separate serial line with no tag prefix. That is harmless and needs no
  new API: accept it.

Unchanged:

- The headline "Pad unreachable" (`scr_connecting.c:96`).
- `"pad returned HTTP %d"` (`dial_somnus.c:127`): the pad answered, so the
  API is on.
- `"http client init failed"` (`dial_somnus.c:105`): a local resource
  failure, not the pad.
- The parse/OOM strings (`dial_somnus.c:183`, `:198`, `:232`): the pad
  answered.
- `dial_pad_discovery.c`: it keeps its own client and its own silence.

## 4. Who reads `dial_somnus_last_error()` and `phase_err`

`git grep -n 'dial_somnus_last_error'` hits in firmware (docs hits are
older SPEC/REPORT quotes and are not code):

| Where | Path | Gets the hint? |
|---|---|---|
| `main/main.c:960` | `CMD_PAD_SETTINGS_CHANGED`: user saved a Pad Address / Bed Mode, re-probe failed | Set, but not shown: a steady-state case (see below) |
| `main/main.c:1065` | pre-READY connect loop (§2 step 5) | Yes — the case this spec is for, and the only one that shows it |
| `main/main.c:1418` | steady state: 3 consecutive poll failures | Set, but not shown: the dial face keeps its staleness dot (see below) |
| `components/dial_somnus/dial_somnus.c:42`, `.h:144`, `.h:107`, `.c:85` | definition, declaration, comments | — |
| `components/dial_ui/scr_connecting.c:100` | comment | — |

`git grep -n 'phase_err'` hits in firmware and simulator:

| Where | Role |
|---|---|
| `components/dial_state/dial_state.h:378` | `char phase_err[128]` |
| `components/dial_state/dial_state.c:927` | `dial_state_set_phase()` copies with `strlcpy` (NULL keeps the old text) |
| `components/dial_state/dial_state.h:339-348` | cert-error comments (dead path, see `scr_connecting.c:91-95`) |
| `components/dial_ui/scr_connecting.c:103-104` | PH_DEGRADED echo (see §5) |
| `components/dial_ui/scr_pad_discovery.c:97-116` | PH_PAD_DISCOVERY progress parser (see below) |
| `components/dial_pad_discovery/dial_pad_discovery.c:330` | writes `"<headline>\n<n>/<total>"` progress |
| `simulator/main.c:240-244`, `:832`, `:853` | baseline reset; discovery and degraded-real scenarios |
| `simulator/sim_state.c:223`, `:285` | sim's own `"pad unreachable (simulated)"` and setter |

**Where the hint is actually shown (corrected after the 2026-09-30 bench;
line numbers at `5e9bc69`, unchanged from `335b8af` in these files).**
Setting `phase_err` is not the same as showing it. `nav_policy`
(`main.c:171`) routes PH_DEGRADED together with PH_READY and PH_WIFI_LOST
(`main.c:319-321`), and once the dial holds device state
(`if (st->have_state)`, `main.c:340`) it returns the dial face
(`main.c:425`). `scr_dial.c:731` then lights the staleness dot for any
phase other than PH_READY. Only with no device state does the route fall
through to `SCR_ERROR` (`main.c:454`), which is where
`scr_connecting.c` renders the hint.

- `main.c:1418` runs in the steady-state loop, after the first successful
  poll has set `have_state` (`main.c:498`). The dial stays on its face with
  the staleness dot; the hint is never displayed. This is the existing
  silent-staleness design and this change leaves it alone.
- `main.c:960` is the same case. `CMD_PAD_SETTINGS_CHANGED` reaches
  `handle_immediate_cmd` only from the steady-state loop (`main.c:1126`,
  `:1176`); before READY, `backoff_wait()` intercepts it and cuts the wait
  short instead (`main.c:801-802`), and the retry goes through `:1065`. So a
  Pad Address saved on a running dial that does not answer gives the
  staleness dot, not the hint.
- `main.c:1065` is the only path that shows the hint: the dial has had no
  pad state since boot, either a fresh device or a dial rebooted while the
  pad is down. (One narrow exception: if the very first poll after connect
  fails, `main.c:1099`, the dial reaches READY without device state, and a
  later `:1418` would show the hint. Not seen on the bench.)

**A pad that was working and then drops off** (power cut, Wi-Fi drop on the
pad side, a DHCP move) therefore shows the staleness dot while the dial
keeps running, and gets the hint only through `main.c:1065` after a reboot.
**That is acceptable.** The hint is a question, not a claim,
and "is the API enabled?" is still the right first thing to check. The
headline still says what happened. The weak case is a pad whose API is on
but which is powered off or off the network: the question sends the user to
check something that is fine. A user who has used the dial before already
knows the API is on, so they will answer "yes" and look elsewhere. The
firmware README troubleshooting entry (§8) keeps the power and
same-network checks next to it.

**A pre-existing leak that the change makes visible, but does not cause.** `main.c:1055` sets
`PH_PAD_DISCOVERY` with a NULL error, so `phase_err` keeps the last
PH_DEGRADED text until the first discovery worker reports
(`dial_pad_discovery.c:330`). On the first scan of a fresh device it is
still `""`, because `main.c:1039` also passes NULL and nothing earlier set it.
On every later scan (5-minute cooldown) it is the previous failure text,
and `scr_pad_discovery.c:97-116` renders it for that tick:

- Today: no `\n`, so the whole `"http error: ESP_ERR_HTTP_CONNECT"` is the
  headline, with an empty fraction and a 0 % arc.
- After this change: the `\n` splits it. The headline is "Is the pad's Local API
  enabled?", `sscanf` on "Somnus support..." fails, so the fraction is empty
  and the arc is 0 %.

Both are one-tick flashes, and after the change the flash reads better. **Out of
scope** (one change per beta). It is recorded here so the bench does not
report it as a regression. The fix would be one line (`main.c:1055` passing
`""`), for a later beta if the owner wants it.

## 5. Nothing matches on the old prefix, and the echo handles `\n`

`git grep -n 'http error'` at `335b8af`:

- `components/dial_somnus/dial_somnus.c:117` — the line this change edits.
- `tools/dial_display_audit.py:101` — the audit tool's **own** log text for
  its own failed GET. It does not read the dial's string.
- `docs/REPORT-1.0.1-bench-gate.md:105`, `docs/REPORT-1.0.2-bench-items-4-5.md:143-168`
  — quoted serial logs (history; they stay as they are).

No code compares against, `strncmp`s or `strstr`s the `"http error"` prefix.
`git grep -n 'ESP_ERR_HTTP_CONNECT'` hits only those same two historical
reports.

`scr_connecting.c:103` echoes `phase_err` when it is non-empty and not equal
to "Pad unreachable". The hint passes that test. `:107`/`:109` insert it with
`"%s%s..."` into `sub_txt`, and `snprintf` copies the embedded `\n`
unchanged. The subtitle is an `LV_LABEL_LONG_WRAP` label (`:51`), and LVGL
breaks lines on `\n`. That is the same mechanism the existing `\n` joins
between the reason, the retry line and "Swipe left for menu" already use.
Nothing else is needed.

## 6. Length and wrap

**Bytes.** The hint is 31 + 1 + 29 = **61 bytes** + NUL. It fits `phase_err[128]`
(`dial_state.h:378`), `s_last_error[160]` (`dial_somnus.c:17`), and the
subtitle buffer `sub_txt[160]` (`scr_connecting.c:67`). The worst case is
61 + "\nRetrying in 60s" (16) + "\nSwipe left for menu" (20) = 97 bytes.

**Pixels.** The subtitle is 320 px wide at `lv_font_montserrat_16`
(`scr_connecting.c:50`, `:53`). The block offsets are documented to fit a
4-line subtitle (`scr_connecting.c:31-36`: sub 168-240 px, 72 px). Measured with
LVGL 8.4.0's own `lv_txt_get_size()` (the firmware's pinned checkout,
the simulator's `lv_conf.h`, letter and line space 0, line height 18 px):

| Text | Width (px) | Lines at 320 px |
|---|---:|---:|
| `Is the pad's Local API enabled?` | 250 | 1 |
| `Somnus support can turn it on` | 249 | 1 |
| `Retrying in 60s` (widest countdown) | 123 | 1 |
| `Retrying...` (steady-state path, `retry_in_s` = 0) | 83 | 1 |
| `Swipe left for menu` | 160 | 1 |
| full subtitle, hint + "Retrying in 60s" + swipe | 250 | **4** (72 px) |
| full subtitle, hint + "Retrying..." + swipe | 250 | **4** (72 px) |
| today: `http error: ESP_ERR_HTTP_CONNECT` + retry + swipe | 301 | 3 |
| today's sim string `Somnus pad at ... (HTTP -1)` + retry + swipe | 305 | 4 |

**Neither hint line wraps.** Each has about 70 px of spare width. The worst case is exactly
4 lines, which is the layout's documented maximum, so no rewording is needed.
Both lines are narrower than today's sim string (305 px), so they sit inside
the bezel wherever that string already did.

## 7. Simulator

`simulator/main.c:848-860` (`scenario_pad_degraded_real`) sets
`phase_err` to `"Somnus pad at 192.168.1.100:8080 not responding (HTTP -1)"`
(`:853-854`). The firmware has never produced that string. Plan:

- Set it to the new hint (same two lines, same `\n`), `retry_in_s` stays
  27. The comment at `:841-847` still holds: the subtitle stays at four lines.
- Rebuild `dial_sim` and regenerate. **Keep only
  `docs/screens/pad-degraded-real.png`**. Back up `docs/screens/` before the run and
  restore every other file, so unrelated renderer churn does not land.
- Before restoring, diff every regenerated PNG against the backup. **Any
  PNG other than `pad-degraded-real.png` that differs is a finding** and gets
  reported, not committed. `pad-unreachable.png` should not change: it uses
  `sim_state.c:223`'s own `"pad unreachable (simulated)"` string, not
  `dial_somnus`.
- `sim_state.c` is not changed.

## 8. README

**Landed early.** Both rewordings below were pulled forward and committed
in the docs commit "docs: README - ask Somnus support to enable the Local
API before flashing (pulled forward from SPEC-local-api-hint)", ahead of the
code: `README.md` (the requirements bullet) and `firmware/dial-idf/README.md`
(the troubleshooting entry). The 1.0.4-beta.1 code change therefore no
longer includes any README change. The items below are kept as the record
of what was asked for; line numbers are at `335b8af`.

When the Somnus app ships a Local API switch (§1), both READMEs go stale
together with the hint's second line, and all three are reworded in that
later beta.

Two places, both to **reword** (no replacement strings dictated here):

- `README.md:39-43` — the requirements bullet says that if the pad does not
  answer, "the local API is off at the pad's firmware level — that is a
  question for Somnus". Reword it to say plainly: ask Somnus support to
  enable the Local API. Use the same words the dial now shows.
- `firmware/dial-idf/README.md:211-218` — the troubleshooting entry
  **The dial is stuck on "Connecting…" / "Pad unreachable".** (It is in
  the firmware README, not the top-level one.) It currently says those
  screens "show the actual error". Reword it to describe the on-screen
  question and to name Somnus support as the way to turn the API on. Keep
  the `curl` quick test.

The earlier plan to put both rewordings in the code commit is superseded:
they landed early in the docs commit named above, so the code commit does
not touch either README. When the app switch ships (§1), both are reworded
again along with the hint.

## 9. Draft CHANGELOG section (for the release commit; not in `CHANGELOG.md` yet)

```markdown
## 1.0.4-beta.1 — YYYY-MM-DD (beta)

When the dial cannot reach your pad, the "Pad unreachable" screen now asks
"Is the pad's Local API enabled?" and says Somnus support can turn it on,
instead of showing a technical error code. The Local API has to be on for
the dial to work, and today it is turned on by asking Somnus support.
Nothing else about connecting, retrying or controlling the pad changes.

Internal: the raw network error still goes to the serial log.
```

## 10. Bench plan (the one dial)

A wrong Pad Address alone is **not** a valid reproduction. Pad discovery
(§2 step 4) finds the real pad on the first failure and self-heals the
address, so the hint would at most flash by. The pad must truly not answer
on :8080 for the test window.

**Proposed stimulus: unplug the pad's power for the test window** (about
5 minutes), then plug it back in. **This needs the owner.** It is a real bed
in use: pick a time when nobody is on it, and afterwards check that the pad
comes back in the same on/off and setpoint state it was in before.

Steps (serial captured with the cat-based logger into `bench-logs/`):

1. Flash the 1.0.4-beta.1 candidate. Confirm READY on the live pad.
2. Owner unplugs the pad. Wake the dial and keep it awake (the 60 s screen
   timeout would otherwise send it to STANDBY).
3. After 3 failed polls (`main.c:1418`, 10 s cadence): the serial shows
   `dial_somnus: http error: ESP_ERR_HTTP_...` followed by the hint. The
   screen stays on the dial face with the yellow staleness dot, because the
   dial already holds pad state (§4). *Corrected after the 2026-09-30 bench:
   this step first expected the hint screen here.* The hint screen, and its
   photograph (both hint lines unwrapped, no clipping at the bezel,
   headline and block not overlapping), belong to step 4.
4. Reboot the dial with the pad still unplugged: the pre-READY path
   (`main.c:1065`) shows the hint with a "Retrying in Ns" countdown
   (4 lines). Photograph it. A discovery scan runs. Note the one-tick
   discovery-screen flash from §4 on the second scan only if the window is
   long enough (it is expected, not a failure).
5. Owner plugs the pad back in: the dial returns to READY without a user
   action. Check the pad state against step 1.
6. Swipe left from the degraded screen once: the menu opens (unchanged
   escape hatch).

The real trigger, a pad with its Local API turned off, cannot be
reproduced here: only Somnus support can switch it. The claim that such a pad
refuses :8080 at the transport level, and does not answer with an HTTP
status, is now supported twice. The reporting user's dial (2026-09-25)
showed `ESP_ERR_HTTP_CONNECT` while that pad's Local API was known to be off
(no one had yet asked Somnus support to enable it), and the 2026-09-30 bench
showed the same transport-level error from an unplugged pad, not an HTTP
status. It closes fully when that user's API is enabled and the same dial
connects. If a pad with the API off ever turns out to answer with a status
code, it would show `"pad returned HTTP %d"` and not the hint. That is
outside this change.

## 11. Sequencing

- `main` carries no branches. **The code for this change must not land on
  `main` until the `somnus-v1.0.3-beta.1` tag exists.** Anything committed
  before that tag ships inside 1.0.3-beta.1, and that would make it two
  changes in one beta. At `335b8af` the tag does not exist, locally or on `somnus`.
- One change per beta: this is the only change in 1.0.4-beta.1.
- Spec before code: this file is committed first, on its own.
- Hardware before tag: §10 runs on the dial before `somnus-v1.0.4-beta.1`
  is tagged.
- Later: the app's Local API switch ships → reword hint line 2 and the two
  README passages in their own beta.
