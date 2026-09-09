# REPORT — somnus-v1.0.0 release commit (commit only; not tagged, not pushed)

Date: 2026-09-09. Session: https://claude.ai/code/session_01T2F1kBEuT13CM3rWm5tnLD

## Verdict

**DONE** — commit `c56cc4d` created on `main` on top of `53a42c5`, touching exactly the five allowed files; not tagged, not pushed. One deviation recorded below (check 4 is non-empty as literally written, with every hit in historical report/spec files outside the allowed diff set).

## Gate (all four passed)

```
$ git rev-parse --short HEAD
53a42c5

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.6")

$ git tag -l 'somnus-v*' --sort=-v:refname | head -1
somnus-v0.1.6-beta.3

$ git tag -l somnus-v1.0.0
(empty)

$ git --no-optional-locks status --short
?? docs/REPORT-0.1.6-graduation.md
```

## Edit F pre-check: OTA URLs in firmware/dial-idf/components/dial_ota/dial_ota.c lines 35-47

```
#define GITHUB_API_URL \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
#define GITHUB_API_URL_TAGS \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
#define GITHUB_API_URL_RELEASE_BY_TAG_FMT \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
```

All three match the clause written into docs/ARCHITECTURE.md (`/releases/latest`, `/tags?per_page=50`, `/releases/tags/<tag>`).

## Checks 1-5, raw output (run against the working tree after edits, before commit)

```
### Check 1: grep -n -E '0\.1\.6' README.md CHANGELOG.md | head
README.md:218:**1.0.0 shipped 2026-09-09** — the `0.1.6` build renumbered, once a
CHANGELOG.md:27:Bedknob for Somnus 1.0. This is the `0.1.6` build renumbered — no code
CHANGELOG.md:34:If your dial is on `0.1.6`, it will offer `1.0.0` as an update (or install
CHANGELOG.md:54:## 0.1.6 — 2026-09-07
CHANGELOG.md:57:on the beta channel (`0.1.6-beta.1` through `-beta.3`); this collects it for
CHANGELOG.md:123:## 0.1.6-beta.3 — 2026-09-06 (beta)
CHANGELOG.md:148:## 0.1.6-beta.2 — 2026-09-06 (beta)
CHANGELOG.md:170:## 0.1.6-beta.1 — 2026-09-05 (beta)

### Check 2: extractor ver=1.0.0
Bedknob for Somnus 1.0. This is the `0.1.6` build renumbered — no code
changes — after it soaked in nightly use. `1.0.0` marks the point where a
new user can buy a Waveshare ESP32-S3-Knob-Touch-LCD-1.8, flash it from
the browser at https://matthewclaude.github.io/somnus-dial-releases/, join
it to Wi-Fi from a phone, and have it find the Somnus Pad on its own —
without typing an IP address — and then receive fixes over the air.

If your dial is on `0.1.6`, it will offer `1.0.0` as an update (or install
it automatically in the morning window if automatic updates are on).
Nothing about how the dial behaves changes with this update.

### What 1.0 includes

- Pad discovery on the local network, with the address re-persisted when
  the pad moves.
- Setup from a phone: Wi-Fi captive portal, timezone captured from the
  browser or picked on the dial.
- The Somnus relative scale (−15 to +15, matching the Somnus app) or
  absolute °C / °F.
- Night mode, Night face, Standby face, and separate day / night / standby
  brightness.
- Over-the-air updates with bootloader rollback, an optional beta channel,
  and optional automatic installs in a window after night ends.
- Change network, Check for updates and Factory reset that work even when
  the dial cannot reach the pad.
- A power indicator that knows whether the dial is on USB or battery.
--- line count:       26

### Check 3: extractor ver=0.1.6 (first and last lines)
first: The first stable release since `0.1.5`. Everything below shipped incrementally
last:    regenerated.
lines:       66

### Check 4: grep -rn 'per_page=5' docs/ README.md
docs/REPORT-0.1.5-ci-check.md:105:**Release list order as the 0.1.4 beta-on code path sees it** (`/releases?per_page=5`):
docs/REPORT-ota-beta-not-found.md:110:| Fix is commit `819f102` | `git show --stat 819f102` | 2026-09-03 15:10:39 -0500, "ota: beta channel picks the newest tag, then fetches that release; the release list order is not newest-first"; touches only `dial_ota.c`/`dial_ota.h`; diff removes `releases?per_page=5` + `RELEASES_LIST_SCAN_CAP 5`, adds `tags?per_page=50` + `releases/tags/%s` |
docs/REPORT-ota-beta-not-found.md:112:| Old path still in 0.1.4 code | `git show somnus-v0.1.4:.../dial_ota.c \| grep per_page` | line 44 `releases?per_page=5`, line 61 `RELEASES_LIST_SCAN_CAP 5` |
docs/REPORT-ota-beta-not-found.md:115:| Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-beta5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
docs/SPEC-ota-readiness.md:12:> §9 (beta channel — **§9.7 supersedes §9.3's `per_page=5` paragraph**: the
docs/SPEC-ota-readiness.md:197:GET https://api.github.com/repos/matthewclaude/somnus-waveshare-rotary-dial/releases?per_page=5
docs/SPEC-ota-readiness.md:544:`/releases/latest` and `/releases?per_page=5` the same way I did in §1 and
docs/SPEC-ota-readiness.md:1024:invisible to the dial; §9.3's `per_page=5` paragraph below is kept as
docs/SPEC-ota-readiness.md:1035:| Beta endpoint `/releases?per_page=5` (includes them) | `dial_ota.c` `GITHUB_API_URL_LIST` |
docs/SPEC-ota-readiness.md:1062:As written 2026-09-02: `per_page=5` and `RELEASES_LIST_SCAN_CAP` were deliberate
docs/SPEC-ota-readiness.md:1240:**Why.** `GET /releases?per_page=5` returned, in order: `0.1.4`, `0.1.3`,
docs/SPEC-ota-readiness.md:1345:switching from `GET /releases?per_page=5` (scan cap 5) to `GET
docs/SPEC-ota-readiness.md:1352:`dial_ota.c` at `somnus-v0.1.4` still carries the `releases?per_page=5`
docs/SPEC-night-window.md:400:newest-first, so `per_page=5` never contained the beta. `0.1.5-beta.2`
(exit 0)

### Check 5: git diff --stat HEAD (working tree + index vs HEAD)
 CHANGELOG.md                     |  38 ++-
 README.md                        |  11 +-
 docs/ARCHITECTURE.md             |  22 +-
 docs/REPORT-0.1.6-graduation.md  | 703 +++++++++++++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |   2 +-
 5 files changed, 756 insertions(+), 20 deletions(-)

### git status --short
 M CHANGELOG.md
 M README.md
 M docs/ARCHITECTURE.md
A  docs/REPORT-0.1.6-graduation.md
 M firmware/dial-idf/CMakeLists.txt
```

### Check 1 — why each remaining `0.1.6` hit stays

| Hit | Why it stays |
|---|---|
| README.md:218 | The new 1.0.0 paragraph (edit E, spec-verbatim): "the `0.1.6` build renumbered". |
| CHANGELOG.md:27 | The new 1.0.0 section (edit C, spec-verbatim): "This is the `0.1.6` build renumbered". |
| CHANGELOG.md:34 | The new 1.0.0 section (edit C, spec-verbatim): upgrade note "If your dial is on `0.1.6`". |
| CHANGELOG.md:54 | The `## 0.1.6 — 2026-09-07` section heading itself — historical, and the extractor for `ver=0.1.6` depends on it. |
| CHANGELOG.md:57 | Body of the 0.1.6 section, naming its own beta line — historical. |
| CHANGELOG.md:123, 148, 170 | The `## 0.1.6-beta.1/2/3` section headings — historical. |

No hit is a "current release" claim; the README release line (lines 4-5) now reads `somnus-v1.0.0`.

### Check 2 — result

Non-empty (26 lines). Starts at "Bedknob for Somnus 1.0." and stops before `## 0.1.6` (the section that follows in the file). Full text reproduced in the "Release notes" section below.

### Check 3 — result

`ver=0.1.6` still extracts only the 0.1.6 section: 66 lines, first line "The first stable release since `0.1.5`. Everything below shipped incrementally", last line "  regenerated." It does not bleed into the 1.0.0 section above it or the `0.1.6-beta.3` section below it (the exact-match awk rejects `## 0.1.6-beta.3` because the character after `0.1.6` is `-`, not a space or end of line).

### Check 4 — result: NOT empty as literally written (see Deviations)

Hits by file, all historical:

- `docs/REPORT-0.1.5-ci-check.md:105` — dated CI report describing what the 0.1.4 code path requested.
- `docs/REPORT-ota-beta-not-found.md:110, 112, 115` — the forensic report on the beta-not-found bug; the hits are the quoted old endpoint and the commit that removed it.
- `docs/SPEC-ota-readiness.md:12, 197, 544, 1024, 1035, 1062, 1240, 1345, 1352` — the OTA spec; line 12 states outright that §9.7 supersedes the §9.3 `per_page=5` paragraph, and lines 1024/1345/1352 are the supersession narrative itself.
- `docs/SPEC-night-window.md:400` — history of why `0.1.5-beta.2` was invisible.

Zero hits in `README.md` and zero in `docs/ARCHITECTURE.md`, the two current-state docs this task edits. The remaining hits live in files the RULE forbids touching (only version string, CHANGELOG, README, one ARCHITECTURE paragraph, and the graduation report may differ), and scrubbing them would erase the record of the bug they document.

### Check 5 — result

`git diff --stat HEAD` touched exactly the five permitted paths and nothing else. `git status --short` at that point showed the four modifications plus `A  docs/REPORT-0.1.6-graduation.md`.

## Commit

```
$ git diff --stat HEAD~1 HEAD
 CHANGELOG.md                     |  38 ++-
 README.md                        |  11 +-
 docs/ARCHITECTURE.md             |  22 +-
 docs/REPORT-0.1.6-graduation.md  | 703 +++++++++++++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |   2 +-
 5 files changed, 756 insertions(+), 20 deletions(-)

$ git rev-parse --short HEAD
c56cc4d

$ git tag -l somnus-v1.0.0
(empty — not tagged)

$ git --no-optional-locks status --short   (immediately after commit, before this report existed)
(empty)
```

Only change under `firmware/` in the commit:

```
-set(PROJECT_VER "0.1.6")
+set(PROJECT_VER "1.0.0")
```

Commit message (first line): `release: somnus-v1.0.0 — the 0.1.6 build renumbered; v1 scope complete`. Body as specified, plus the session's mandated `Co-Authored-By` / `Claude-Session` trailers.

## Release notes — the extracted 1.0.0 text (check 2), published verbatim by CI

```
Bedknob for Somnus 1.0. This is the `0.1.6` build renumbered — no code
changes — after it soaked in nightly use. `1.0.0` marks the point where a
new user can buy a Waveshare ESP32-S3-Knob-Touch-LCD-1.8, flash it from
the browser at https://matthewclaude.github.io/somnus-dial-releases/, join
it to Wi-Fi from a phone, and have it find the Somnus Pad on its own —
without typing an IP address — and then receive fixes over the air.

If your dial is on `0.1.6`, it will offer `1.0.0` as an update (or install
it automatically in the morning window if automatic updates are on).
Nothing about how the dial behaves changes with this update.

### What 1.0 includes

- Pad discovery on the local network, with the address re-persisted when
  the pad moves.
- Setup from a phone: Wi-Fi captive portal, timezone captured from the
  browser or picked on the dial.
- The Somnus relative scale (−15 to +15, matching the Somnus app) or
  absolute °C / °F.
- Night mode, Night face, Standby face, and separate day / night / standby
  brightness.
- Over-the-air updates with bootloader rollback, an optional beta channel,
  and optional automatic installs in a window after night ends.
- Change network, Check for updates and Factory reset that work even when
  the dial cannot reach the pad.
- A power indicator that knows whether the dial is on USB or battery.
```

## Deviations from this spec

1. **Check 4 is non-empty.** `grep -rn 'per_page=5\b' docs/ README.md` returns 14 hits (listed above), all in dated reports and the OTA spec's superseded-history sections, none in README.md or docs/ARCHITECTURE.md. Fixing them would require editing files outside the RULE's allowed set, so I committed anyway and am flagging it here. If you want the literal check to pass, the change belongs in a separate docs commit; if you disagree with committing, `git reset --soft HEAD~1` restores the pre-commit state (nothing is tagged or pushed).
2. **Commit trailers.** The commit body carries `Co-Authored-By: Claude Fable 5.1` and `Claude-Session:` lines that the spec's `-m` text did not include; the session's attribution rule requires them.
3. **Edit F wrapping.** The ARCHITECTURE paragraph was rewrapped at 79 columns (the file's existing width); the sentence content is otherwise as specified.

Everything else (edits A–E, G, the commit command's file list, no tag, no push, this report left untracked) matches the spec.

## Observations, not acted on (outside the allowed diff)

- `firmware/dial-idf/CMakeLists.txt` lines 20-21 still carry the comment "1.0.0 is reserved for when docs/V1-scope.md is actually complete -- see CHANGELOG.md's Somnus section header." It is now stale. The RULE limits firmware/ changes to the PROJECT_VER line, so it was left alone; worth a one-line follow-up commit.

## Could not verify here (no hardware / CI)

- The firmware was **not built** in this session; no `idf.py build` was run and no binary was flashed. Since the only firmware change is the version string, the 0.1.6 build's behaviour is what ships, but that equivalence is asserted, not compiled.
- CI (`.github/workflows/release.yml`) will verify, on tag push, that the tag `somnus-v1.0.0` equals `PROJECT_VER` (`1.0.0`) and that the CHANGELOG section extracts non-empty; the extractor was run locally with the same awk and passed, but the tag-vs-version check only runs in CI.
- OTA acceptance (a dial on 0.1.6 offering/installing 1.0.0) can only be observed after tag + publish, on a device.
