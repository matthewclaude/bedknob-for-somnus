# REPORT: 1.0.4-beta.1 bench close-out — Local API hint on the bench dial

**Verdict: PASS** for the change, with one spec error. After a reboot with the pad unplugged, the hint screen shows correctly on the pre-READY path. Swiping left reaches the menu, the dial reconnected by itself when the pad came back, and none of the three capture logs contains a POST. The spec error is §10 step 3: a dial that already holds pad state stays on its face with the staleness dot and never shows the hint, which is the existing silent-staleness design. `docs/SPEC-local-api-hint.md` is corrected in this commit.

Date: 2026-09-30. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch main. Build under test: `5e9bc69`, wire-flashed with `idf.py flash`, NVS kept (`docs/REPORT-apihint-bench-flash.md`). Logs: `bench-logs/2026-09-30-apihint-attach{1,2,3}.log` (gitignored). In quoted log lines the pad's address is replaced with `<pad>`. Lines that name the Wi-Fi network or show the dial's MAC are not quoted.

## Gate

| # | Check | Raw output | Result |
|---|---|---|---|
| 1 | `sed -n 21p firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.3-beta.1")` | PASS |
| 2 | `git rev-parse HEAD` | `5e9bc69d6eba9ac271b24772ded918e80a97aab3` | PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | *(empty)* | PASS |
| 4 | both reports exist, untracked | `?? docs/REPORT-apihint-bench-flash.md`, `?? docs/REPORT-local-api-hint-code.md` | PASS |
| 5 | `ls bench-logs/2026-09-30-apihint-attach*.log` | attach1 (417 B), attach2 (8691 B), attach3 (8080 B) | PASS |

git was run with `DEVELOPER_DIR=/Library/Developer/CommandLineTools` (the Xcode-licence workaround); no licence complaint appeared.

## Step 1: capture stopped

`bench-logs/capture.pid` held **27264**. Processes found with `ps -axo pid,ppid,command`:

| PID | PPID | Role | Action |
|---|---|---|---|
| 27264 | 1 | `sh -c 'DEV=/dev/cu.usbmodem83401; …'` (the loop, PID in capture.pid) | killed first, so no new reader could start |
| 27438 | 27264 | `cat /dev/cu.usbmodem83401` (the current reader; attach3) | killed |
| 27267 | 27264 | `caffeinate -i -s sh -c DEV=/dev/cu.usbmodem83401; …` | killed |
| 27551 | 20179 | `caffeinate -i -t 300` | left alone: its command line does not contain cu.usbmodem, so it is not ours |

Afterwards `ps -axo pid,command | grep -E 'cu\.usbmodem' | grep -v grep` printed nothing (exit 1), and `ps -p 27264,27267,27438` printed nothing (exit 1). None of the three is left. The reader at stop time was 27438, not the 27271 named in the flash report, because each USB re-enumeration (two RST presses) started a new `cat`.

## Build identity

attach2 starts with the full boot banner from the owner's RST press:

```
I (786) app_init: App version:      1.0.3-beta.1
I (790) app_init: Compile time:     Sep 30 2026 11:45:45
I (795) app_init: ELF file SHA256:  5a0c4fcbc...
```

```
$ shasum -a 256 firmware/dial-idf/build/somnus-dial.elf
5a0c4fcbc61c8375a6cc84a1f7f682a9c48f7c0f987069ac559f38b8c3cc0456  firmware/dial-idf/build/somnus-dial.elf
```

The prefix matches. The banner's compile time 11:45:45 is stale because ESP-IDF refreshes the app description only when that file recompiles, and the incremental build for 5e9bc69 did not recompile it. The ELF hash is the evidence. attach3's banner shows the same three lines.

## Results

| Step (spec §10) | Result |
|---|---|
| 1. READY on the live pad | **PASS** |
| 2/3. Pad unplugged, steady state | Firmware as designed; **spec error** (no hint screen, stale dot) |
| 4. RST with the pad still unplugged (pre-READY path) | **PASS** |
| 6. Swipe left from the degraded screen | **PASS** |
| 5. Pad plugged back in, reconnect | **PASS** |
| No writes to the pad | **PASS** (0 POST in each log) |

### Step 1: READY on the live pad (attach2, before the unplug)

```
I (3994) app: pad connected at http://<pad>
I (4104) app: side A: on=0 set=17.0C water=24.1C
…
I (161493) app: side A: on=0 set=17.0C water=24.1C
```

`161493` is the last good poll before the unplug. **PASS.**

### Steps 2/3: pad unplugged, steady state (attach2)

The first three failures:

```
E (176696) esp-tls: [sock=54] select() timeout
E (176696) transport_base: Failed to open a new connection: 32774
E (176696) HTTP_CLIENT: Connection failed, sock < 0
W (176699) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (176704) dial_somnus: Is the pad's Local API enabled?
Somnus support can turn it on
E (191914) esp-tls: [sock=54] select() timeout
E (191914) transport_base: Failed to open a new connection: 32774
E (191914) HTTP_CLIENT: Connection failed, sock < 0
W (191917) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (191922) dial_somnus: Is the pad's Local API enabled?
Somnus support can turn it on
E (207132) esp-tls: [sock=54] select() timeout
E (207132) transport_base: Failed to open a new connection: 32774
E (207132) HTTP_CLIENT: Connection failed, sock < 0
W (207135) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (207140) dial_somnus: Is the pad's Local API enabled?
Somnus support can turn it on
```

attach2 has nine failures in all (176696 to 315844), every one logging the raw `http error:` line and then the hint, with the hint's second line untagged, as spec §3 expects. The dial dropped to standby cadence between failures six and seven (`I (253104) app: poll: standby cadence 300s`, then `I (277704) app: poll: active cadence 10s`).

**Owner's observation:** the normal dial face with a yellow staleness dot, **not** the hint screen.

**Cause.** The dial already held device state (`have_state` true since the first poll, `main.c:498`). nav_policy routes PH_DEGRADED in the same group as PH_READY, and with device state it returns the dial face:

```
firmware/dial-idf/main/main.c
319:    case PH_READY:
320:    case PH_DEGRADED:
321:    case PH_WIFI_LOST:
340:        if (st->have_state) {
425:            return dial_power_level() == DPWR_STANDBY ? standby_screen(st) : SCR_DIAL;
454:        return st->phase == PH_READY ? SCR_CONNECTING : SCR_ERROR;
```

The dial face lights the staleness dot for any phase other than READY:

```
firmware/dial-idf/components/dial_ui/scr_dial.c
731:    bool stale = (st->phase != PH_READY) || !st->device_online;
```

`main.c:1418` sets `phase_err` to the hint, but only `SCR_ERROR` (line 454, reached only without device state) displays it. This is a **spec error, not a firmware fault**. The spec expected the hint screen at step 3. The existing silent-staleness design is unchanged by 5e9bc69.

### Step 4: RST with the pad still unplugged, pre-READY path (attach3)

First failure, then the discovery scan:

```
W (20144) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (20149) dial_somnus: Is the pad's Local API enabled?
Somnus support can turn it on
I (20157) pad_discovery: scan: 253 candidates, pass 1 (300ms)
I (36807) pad_discovery: scan: pass 1 found nothing, pass 2 (600ms)
I (68966) pad_discovery: scan: no pad found
```

Retries after the scan (the `http error` line of each; each is followed by the two hint lines, as above):

```
W (78973) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (93991) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (119008) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (164033) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
W (229056) dial_somnus: http error: ESP_ERR_HTTP_CONNECT
```

The gaps between retries grow (15 s, 25 s, 45 s, 65 s), as the connect loop's backoff should.

**Owner's observation:** "Pad unreachable" in red, both hint lines, each on one line, not clipped, the apostrophe correct, and a retry countdown. **PASS.**

### Swipe left from the degraded screen (spec §10 step 6, taken here in the spec's order)

**Owner's observation:** the menu opened with the Settings row as the heading. **PASS.** No log line is expected for a screen change.

### Pad plugged back in (attach3)

```
I (291510) dial_somnus: zone mode set to single (One Bed)
I (291510) app: pad connected at http://<pad>
I (291567) app: side A: on=0 set=27.0C water=24.1C
```

At uptime 291.5 s, about 62 s after the last failure (229056), the dial reconnected with no user action. The next line, `I (591943) app: side A: on=0 set=27.0C water=24.1C`, is a standby-cadence poll 300 s later. **PASS.**

### No writes to the pad

```
$ grep -c "POST" bench-logs/2026-09-30-apihint-attach1.log
0
$ grep -c "POST" bench-logs/2026-09-30-apihint-attach2.log
0
$ grep -c "POST" bench-logs/2026-09-30-apihint-attach3.log
0
```

### Pad state before and after

| | Side A on/off | Side A setpoint | Source |
|---|---|---|---|
| Before | off (both sides off) | 17.0 °C (both sides) | `REPORT-apihint-bench-flash.md` step 5 (read-only GET) |
| After | off | 27.0 °C | attach3, `291567` and `591943` |

The setpoint changed because the pad lost power: on power-up it returns to its default, level 0 = 27.0 °C. The dial did not cause it; it sent nothing (0 POST above). The owner's Somnus schedule re-asserts the setpoint at its next stage. Nobody restored it during this block, which was read-only by rule.

### Evidence note: the untested assumption

An unplugged pad produces `ESP_ERR_HTTP_CONNECT` at the transport level. That is the same error an outside user's dial showed on 2026-09-25, while that pad's Local API was known to be off (the user had not asked Somnus support to enable it). This supports the spec's premise that a pad with the API off refuses the connection instead of answering with an HTTP status. The claim closes fully when that user's API is enabled and the same dial connects.

One nuance, recorded but not a finding: on this bench the lower layer was a connect timeout (`esp-tls: … select() timeout`, `Failed to open a new connection: 32774`), because an unplugged pad does not answer at all. A pad that is up with its API off may refuse the connection instead, which gives a different ESP-TLS code underneath. Both surface as `ESP_ERR_HTTP_CONNECT` and take the same `err != ESP_OK` branch, so the dial shows the same hint either way.

### Also recorded

`REPORT-local-api-hint-code.md` says the hint is 63 bytes including the `\n`. It is **61** (31 + 1 + 29), as spec §6 says:

```
$ printf '%s' "Is the pad's Local API enabled?" | wc -c
      31
$ printf '%s' "Somnus support can turn it on" | wc -c
      29
```

This is harmless: both buffers are far larger. That report is committed unedited, as the block requires.

## Docs changes in this commit

- `docs/SPEC-local-api-hint.md`: the status line (code `5e9bc69`, bench run 2026-09-30, PASS with the steady-state correction). §4: the `main.c:1418` and `main.c:960` rows and the paragraph about a pad that drops off now say the hint is set but not shown while the dial holds device state. `main.c:960` is a steady-state case, since `CMD_PAD_SETTINGS_CHANGED` reaches its handler only from the steady-state loop, and before READY `backoff_wait()` cuts the wait short instead (`main.c:801-802`). Only `main.c:1065` shows the hint. §10 step 3 now expects the stale dot, and the photograph belongs to step 4. The closing paragraph of §10 now cites the 2026-09-25 user's dial and this bench.
- `simulator/main.c`: comment above `scenario_pad_degraded_real` only. It now says the subtitle is four lines (the two hint lines, the retry line and the swipe line), still the tallest case. `git diff -U0 simulator/main.c` shows only `//` lines changed. The simulator was not rebuilt.
- `docs/REPORTS.md`: three lines after the `REPORT-1.0.3-beta.1-tag-push.md` line.
- `docs/REPORT-local-api-hint-code.md`, `docs/REPORT-apihint-bench-flash.md`: committed as they were, unedited.

## Staged diff

```
$ git diff --cached --name-only
docs/REPORT-apihint-bench-flash.md
docs/REPORT-apihint-bench.md
docs/REPORT-local-api-hint-code.md
docs/REPORTS.md
docs/SPEC-local-api-hint.md
simulator/main.c

$ git diff --cached --stat
 docs/REPORT-apihint-bench-flash.md | 154 ++++++++++++++++++++
 docs/REPORT-apihint-bench.md       | 241 +++++++++++++++++++++++++++++++
 docs/REPORT-local-api-hint-code.md | 287 +++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                    |   3 +
 docs/SPEC-local-api-hint.md        |  71 ++++++---
 simulator/main.c                   |  12 +-
 6 files changed, 745 insertions(+), 23 deletions(-)
```

## Deviations

- This report cannot carry its own commit SHA. The commit is the one whose message begins "docs: 1.0.4-beta.1 bench PASS (hint screen, reconnect)"; `git log --oneline -1 -- docs/REPORT-apihint-bench.md` finds it.
- Spec §10 step 3 expected the hint screen on the steady-state path. The logs and the owner's observation show the dial face with the staleness dot instead. This is the spec error above, corrected in the spec, not a firmware fault.
- `REPORT-local-api-hint-code.md` gives the hint as 63 bytes; it is 61. Left unedited in that report, by rule.
- attach3 has no closing `=== reader lost device … ===` line. The loop was killed before its reader, so the loop never wrote the line. The log is otherwise complete.
- The reader PID at stop time (27438) differs from the flash report's 27271, because each RST re-enumerated USB and the loop started a new `cat`. Both are our capture, not a stray process.

## Not verified

- A pad with its Local API actually switched off. Only Somnus support can do that.
- A Dual Sides dial on this path. The bench dial is One Bed (`zone mode set to single`).
- Night-mode colours on the hint screen. The bench ran in daytime colours.
