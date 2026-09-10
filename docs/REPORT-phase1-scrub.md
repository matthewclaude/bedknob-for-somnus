# REPORT: Phase 1a scrub — untrack reference/, relocate the API spec, scrub the home IP from the SPEC docs

**DONE** — `reference/` untracked (files still on disk, `.gitignore` already had the line), `reference/local_api.yml` moved to `docs/local_api.yml` with both referrers updated and zero stale hits, the home pad address replaced by `192.168.1.100` in the six `docs/SPEC-*.md` hits and deliberately left in the 21 non-SPEC hits, all as commit `c7d23e0` on `main`; STEP 0 landed as `eb12e99`. No firmware file changed; git history not rewritten. Push and CI result: see the post-push addendum at the end of this report (it could not be written into the commit that carries it).

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `3cc247e1cb38671360f2772e11acfe0019b5d7e1`; `somnus/main` before: `0b6c62f8cf7b084d4af1910bcfeea9d8ec5e5131` (local one ahead, as the task expected)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

Untracked at gate time (expected): `docs/REPORT-readme-license-fix.md`.

## STEP 0 — clear the owed report

Added to `docs/REPORTS.md`, directly after the `REPORT-docs-amend.md` line:

```
- `REPORT-readme-license-fix.md` — 2026-09-10 — REPORT: README license and contributing text fix — **BLOCKED** — the text on disk did not match the task's premise: `README.md`'s License and Contributing sections were already reworded, the disclaimer already read "Somnus Lab", and two of the four predicted strings appear nowhere in the tree; no README edited, STEP 0 done as `3cc247e`.
```

Committed as **`eb12e99a05983c385d101b441f12316c9ffcf8a5`** — "docs: track the readme-license-fix report".

```
 docs/REPORT-readme-license-fix.md | 187 ++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                   |   1 +
 2 files changed, 188 insertions(+)
```

## STEP 1 — relocate the API spec

```
$ git mv reference/local_api.yml docs/local_api.yml
```

Rename recorded by git as `{reference => docs}/local_api.yml | 0` (content identical).

`README.md` line 236:

```
before: - [`reference/local_api.yml`](reference/local_api.yml) — Somnus's published local API spec
after:  - [`docs/local_api.yml`](docs/local_api.yml) — Somnus's published local API spec
```

`docs/ARCHITECTURE.md` line 15:

```
before: ([`reference/local_api.yml`](../reference/local_api.yml), v0.2.0) and the
after:  ([`docs/local_api.yml`](local_api.yml), v0.2.0) and the
```

Zero-hit check:

```
$ git grep -n "reference/local_api"
(no output, exit 1)
```

## STEP 2 — untrack reference/

```
$ git rm -r --cached reference/
rm 'reference/LOGGING_README.md'
rm 'reference/bed_web_app.py'
rm 'reference/sleep_temp_funky.html'
rm 'reference/sleep_temp_viewer.html'
rm 'reference/somnus_bed.py'
```

(`local_api.yml` had already left the directory in STEP 1, so five files, not six.)

Verification 1 — index is empty for the directory:

```
$ git ls-files reference/
(no output)
```

Verification 2 — files remain on disk:

```
$ ls reference/
bed_web_app.py
LOGGING_README.md
sleep_temp_funky.html
sleep_temp_viewer.html
sleep_temps.csv
somnus_bed.py
upstream-pr4-battery-diag
waveshare-schematic
```

(`sleep_temps.csv`, `upstream-pr4-battery-diag/` and `waveshare-schematic/` were never tracked; they are unaffected.)

Verification 3 — `.gitignore` already carries the bare line at line 49, so nothing was added:

```
$ grep -n "^reference/$" .gitignore
49:reference/
```

## STEP 3 — scrub the home IP from the SPEC docs only

Full grep before any edit:

```
$ git grep -n "192\.168\.1\.169"
docs/REPORT-0.1.5-upgrade-path-verify.md:109:179:I (3429) app: pad connected at http://192.168.1.169:8080
docs/REPORT-dial-display-audit.md:281:$ PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py --once
docs/REPORT-dial-display-audit.md:367:`PAD_HOST=192.168.1.169`, 1 Hz `GET /api/state` only. No writes were issued
docs/REPORT-layout-a1b.md:713:I (2768) app: pad connected at http://192.168.1.169:8080
docs/REPORT-layout-phase1-flash.md:542:I (3085) app: pad connected at http://192.168.1.169:8080
docs/REPORT-rails-fix.md:882:I (5715) app: pad connected at http://192.168.1.169:8080
docs/REPORT-standby-face-c1.md:489:I (4178) app: pad connected at http://192.168.1.169:8080
docs/REPORT-standby-face-c2.md:554:I (5286) app: pad connected at http://192.168.1.169:8080
docs/REPORT-sticky-brightness.md:449:I (2645) app: pad connected at http://192.168.1.169:8080
docs/REPORT-sticky-night-pickers.md:478:I (3027) app: pad connected at http://192.168.1.169:8080
docs/SPEC-connect-phases.md:512:- Leaves the temporary `192.168.1.169` default (since reverted to `192.168.1.100`), the `main.c` connect-loop
docs/SPEC-ota-readiness.md:954:`http://192.168.1.169:8080` — a real home IP — while pad discovery was
docs/SPEC-ota-readiness.md:989:Zero occurrences of `192.168.1.169` anywhere in the binary. `0.1.0` (§7)
docs/SPEC-ota-readiness.md:1011:if so whether they still describe `192.168.1.169` as a live,
docs/SPEC-pad-discovery.md:62:`192.168.1.169` compiled default
docs/SPEC-pad-discovery.md:724:- Not touching the temporary `192.168.1.169` default (since reverted to
reference/bed_web_app.py:18:BED_IP = "192.168.1.169"
reference/somnus_bed.py:8:    def __init__(self, ip="192.168.1.169", port="8080"):
reference/somnus_bed.py:156:    bed = SomnusPad(ip="192.168.1.169", port="8080")
tools/dial_display_audit.py:12:hardcoded default, even though the bench pad's current address (192.168.1.169)
tools/dial_display_audit.py:15:    PAD_HOST=192.168.1.169 python3 tools/dial_display_audit.py
tools/dial_display_audit.py:16:    python3 tools/dial_display_audit.py --host 192.168.1.169
tools/dial_display_audit.py:88:        print("(bench-logs/*.log shows this dial's pad at 192.168.1.169 as of "
(exit 0)
```

27 hits in 16 files (the `reference/` hits were still in the index at grep time; the grep ran before STEP 2).

### Changed (6 hits, 3 files — all `docs/SPEC-*.md`)

| File:line | After |
|---|---|
| `docs/SPEC-connect-phases.md:512` | `- Leaves the temporary \`192.168.1.100\` default (since reverted to \`192.168.1.100\`), the \`main.c\` connect-loop` |
| `docs/SPEC-ota-readiness.md:954` | `\`http://192.168.1.100:8080\` — a real home IP — while pad discovery was` |
| `docs/SPEC-ota-readiness.md:989` | `Zero occurrences of \`192.168.1.100\` anywhere in the binary. \`0.1.0\` (§7)` |
| `docs/SPEC-ota-readiness.md:1011` | `if so whether they still describe \`192.168.1.100\` as a live,` |
| `docs/SPEC-pad-discovery.md:62` | `\`192.168.1.100\` compiled default` |
| `docs/SPEC-pad-discovery.md:724` | `- Not touching the temporary \`192.168.1.100\` default (since reverted to` |

Post-edit check: `git grep -n "192\.168\.1\.169" -- 'docs/SPEC-*.md'` returns no hits. The other 12 `docs/SPEC-*.md` files had no occurrence.

**Owner should read these four before publishing** (flagged, not changed — the task's rule was a blanket replace in SPEC files): the substitution makes four sentences self-referential or false as history, because they were *about* the old address:

- `SPEC-connect-phases.md:512` and `SPEC-pad-discovery.md:724` now read "the temporary `192.168.1.100` default (since reverted to `192.168.1.100`)" — the same value on both sides.
- `SPEC-ota-readiness.md:954` now calls `192.168.1.100` "a real home IP", which it is not (it is the documented example/compiled default).
- `SPEC-ota-readiness.md:989` now says the binary scan found zero occurrences of `192.168.1.100`; the scan was for the old address. The compiled default is `192.168.1.100`, so whether the new sentence is literally true is unknown without re-running the scan.

Rewording these is a judgement call outside this task; a follow-up could replace the four with wording like "the temporary home-address default".

### Deliberately left (21 hits, 13 files)

| File:line | Why left |
|---|---|
| `docs/REPORT-0.1.5-upgrade-path-verify.md:109` | quoted serial boot log in a report — historical record |
| `docs/REPORT-dial-display-audit.md:281` | quoted terminal command in a report |
| `docs/REPORT-dial-display-audit.md:367` | report prose describing the audit run |
| `docs/REPORT-layout-a1b.md:713` | quoted serial boot log |
| `docs/REPORT-layout-phase1-flash.md:542` | quoted serial boot log |
| `docs/REPORT-rails-fix.md:882` | quoted serial boot log |
| `docs/REPORT-standby-face-c1.md:489` | quoted serial boot log |
| `docs/REPORT-standby-face-c2.md:554` | quoted serial boot log |
| `docs/REPORT-sticky-brightness.md:449` | quoted serial boot log |
| `docs/REPORT-sticky-night-pickers.md:478` | quoted serial boot log |
| `reference/bed_web_app.py:18` | in `reference/`, untracked by STEP 2; on disk only, never edited |
| `reference/somnus_bed.py:8` | same |
| `reference/somnus_bed.py:156` | same |
| `tools/dial_display_audit.py:12` | not a `docs/SPEC-*.md` file; out of this task's scope (still tracked and still carries the home address — see Cannot verify / follow-ups) |
| `tools/dial_display_audit.py:15` | same |
| `tools/dial_display_audit.py:16` | same |
| `tools/dial_display_audit.py:88` | same |

Note: `CHANGELOG.md` had no hit for this address, although the task anticipated CHANGELOG entries. Nothing to leave there.

## Commits — `git diff --stat` per commit, every SHA

**Commit A (STEP 0)** — `eb12e99a05983c385d101b441f12316c9ffcf8a5` — "docs: track the readme-license-fix report"

```
 docs/REPORT-readme-license-fix.md | 187 ++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                   |   1 +
 2 files changed, 188 insertions(+)
```

**Commit B (STEPS 1–3)** — `c7d23e0112ac266438550f739d18ffb61cd5c7c5` — "docs: untrack reference/, move the API spec to docs/, scrub the example pad address"

```
 README.md                         |   2 +-
 docs/ARCHITECTURE.md              |   2 +-
 docs/SPEC-connect-phases.md       |   2 +-
 docs/SPEC-ota-readiness.md        |   6 +-
 docs/SPEC-pad-discovery.md        |   4 +-
 {reference => docs}/local_api.yml |   0
 reference/LOGGING_README.md       |  65 -----
 reference/bed_web_app.py          | 185 ------------
 reference/sleep_temp_funky.html   | 575 --------------------------------------
 reference/sleep_temp_viewer.html  | 401 --------------------------
 reference/somnus_bed.py           | 182 ------------
 11 files changed, 8 insertions(+), 1416 deletions(-)
```

No file under `firmware/` is touched by either commit. Working tree clean after commit B.

**Commit C (this report + its index line)** — SHA cannot appear inside the commit that creates it; recorded in the addendum below and in the chat reply.

## Push and CI

See the post-push addendum at the end of this report.

## STEP 5 — two dangling paths in `docs/SPEC-power-sensing.md` (reported, NOT fixed)

Both directories are untracked (they were never in the index; STEP 2 does not change that), so both sentences point at nothing in the public repo.

Line 96:

> - **The schematic** (five sheets, re-read Sep 3; the five PNGs are now in `reference/waveshare-schematic/`, copied from Sandjab/Waveshare-Knob on GitHub because Waveshare's own download was unreachable): the same five sheets §5 describes. Sheet 1 has USB-C, the TLV62569 buck from the `5V` net to 3V3, the backlight FET and the two encoder switches. Sheet 4 has the `BATT_ADC` divider (R62/R63, 10K/10K, off `5V`), the PDM mic (MSM261D4030H1CPM), the TF socket. **No battery socket, no charger, no boost, no power button, no charge-status net on any sheet.** Sheet 2's ESP32-S3 net list confirms it: the S3's only power-related pin is `GPIO1 = BATT_ADC`. There is no `CHRG`, `PGOOD`, `PWR_KEY` or PMIC I2C. The battery section of the board is simply not in the published schematic, exactly as §5 concluded.

Line 213:

> **Supersedes §9.5's "no percentage, no low warning, no setting" and §10.6's matching exclusions.** Those were the right call with only this project's own measurements in hand. Overtaken by finding [chris023/orion-waveshare-rotary-dial#4](https://github.com/chris023/orion-waveshare-rotary-dial/pull/4) (author: borski, unmerged as of this writing) — an independent battery implementation on the same board, same pin, same divider, whose measured thresholds land within noise of this project's own (§9.5, §10.2). Full PR content fetched read-only and kept for reference at `reference/upstream-pr4-battery-diag/` (not tracked, not built against directly — see that folder's own README). Owner's decision: adopt the percentage curve and the low-battery visual treatment; do not adopt their detector, their separate diagnostics screen, or their new dial_battery component wholesale.

Rewording is the owner's call. Not changed.

## Deviations

- **Three commits, not two, after STEP 0.** The task asked for the scrub commit, then a commit for this report, then the push, and for the report to contain the push result and CI outcome. A report cannot contain the outcome of a push that happens after it is committed, so the push result and CI conclusion are appended as a short addendum in a third, docs-only commit ("docs: phase1-scrub report — push and CI addendum") rather than left as an uncommitted edit to a tracked file, which would fail the next gate. The addendum records its own predecessor's SHA and the CI result; its own SHA is in the chat reply only.
- **Four SPEC sentences became self-referential or historically false** after the blanket replace (listed under STEP 3). The replacement was applied exactly as instructed; the sentences are flagged for the owner rather than reworded.
- The STEP 3 grep was run before STEP 2's `git rm --cached`, so its output still lists the three `reference/*.py` hits. Running it after STEP 2 would drop those three lines; nothing else differs.
- Otherwise none: every command in STEPS 0–5 ran as written, `.gitignore` needed no edit, no firmware file was touched, no history rewritten, nothing pushed to `origin`.

## Cannot verify from disk

- **Whether GitHub's rendering of the public repo shows `reference/` as gone.** Locally the index is empty for it; the remote tree can only be confirmed after the push, and only for the `somnus` remote (see addendum).
- **`tools/dial_display_audit.py` still carries the home address** in a docstring, two usage lines and one runtime print (lines 12, 15, 16, 88). It is tracked and outside this task's `docs/SPEC-*.md` scope. Whether it should be scrubbed before the repo goes public is the owner's call; a follow-up would also need to decide about the ten report files.
- **Whether `192.168.1.100` occurs in the release binary** (the now-rewritten claim at `SPEC-ota-readiness.md:989`). Not checked; would need a `strings` pass over `somnus-dial.bin` at `somnus-v1.0.0`.
- **CI at the time of writing** — see addendum.
