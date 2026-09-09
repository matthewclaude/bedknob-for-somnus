# REPORT — docs housekeeping: 0.1.5 report names, TEST-F3 on disk, REPORTS.md index

Date: 2026-09-09. Session: https://claude.ai/code/session_01T2F1kBEuT13CM3rWm5tnLD

## Verdict

**DONE** — five 0.1.5-era reports renamed with `git mv`, six prose cross-references updated, TEST-F3 tracked in docs/, REPORTS.md added, committed as `3fd1cc1` and pushed to `somnus` only; local HEAD and `somnus/main` both read 3fd1cc1, the `somnus-v1.0.0` tag is untouched, and nothing outside `docs/` changed.

## Gate (all four passed)

```
$ git rev-parse --short HEAD
195e77c

$ git --no-optional-locks status --short
?? docs/REPORT-sim-screens-push.md

$ ls "Claude outputs/TEST-F3-stuck-loop.md"
Claude outputs/TEST-F3-stuck-loop.md
$ ls docs/TEST-F3-stuck-loop.md
ls: docs/TEST-F3-stuck-loop.md: No such file or directory

$ ls docs/REPORT-release-beta3.md docs/REPORT-release-beta4.md docs/REPORT-beta5-commit.md docs/REPORT-beta5-tag-push.md docs/REPORT-beta5-ci-check.md
docs/REPORT-beta5-ci-check.md
docs/REPORT-beta5-commit.md
docs/REPORT-beta5-tag-push.md
docs/REPORT-release-beta3.md
docs/REPORT-release-beta4.md
```

## Step 1 — renames

```
git mv docs/REPORT-release-beta3.md   docs/REPORT-0.1.5-beta.3-release.md
git mv docs/REPORT-release-beta4.md   docs/REPORT-0.1.5-beta.4-release.md
git mv docs/REPORT-beta5-commit.md    docs/REPORT-0.1.5-beta.5-commit.md
git mv docs/REPORT-beta5-tag-push.md  docs/REPORT-0.1.5-beta.5-tag-push.md
git mv docs/REPORT-beta5-ci-check.md  docs/REPORT-0.1.5-beta.5-ci-check.md
```

All five succeeded; `git status` showed them as `R` with three at 100% similarity (0 lines changed) and two carrying the step 2 edits.

## Step 2 — references to the old names

`git grep -n -E 'REPORT-(release-beta[34]|beta5-(commit|tag-push|ci-check))' -- .` after the renames returned 27 hits (a plain `grep -rn` over the tree, excluding `.git`, `build/` and `managed_components/`, found no additional ones):

```
docs/REPORT-0.1.5-beta.5-commit.md:69:- `docs/REPORT-release-beta3.md`
docs/REPORT-0.1.5-beta.5-commit.md:70:- `docs/REPORT-release-beta4.md`
docs/REPORT-0.1.5-beta.5-commit.md:87:?? docs/REPORT-release-beta3.md
docs/REPORT-0.1.5-beta.5-commit.md:88:?? docs/REPORT-release-beta4.md
docs/REPORT-0.1.5-beta.5-commit.md:92:(This file, `docs/REPORT-beta5-commit.md`, is now also untracked, per report convention.)
docs/REPORT-0.1.5-beta.5-tag-push.md:92:`docs/REPORT-beta5-commit.md` and this file.
docs/REPORT-1.0.0-commit.md:92:docs/REPORT-ota-beta-not-found.md:115:| Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-beta5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
docs/REPORT-docs-sync.md:35:?? docs/REPORT-beta5-ci-check.md
docs/REPORT-docs-sync.md:36:?? docs/REPORT-beta5-commit.md
docs/REPORT-docs-sync.md:37:?? docs/REPORT-beta5-tag-push.md
docs/REPORT-docs-sync.md:42:?? docs/REPORT-release-beta3.md
docs/REPORT-docs-sync.md:43:?? docs/REPORT-release-beta4.md
docs/REPORT-docs-sync.md:202:A  docs/REPORT-beta5-ci-check.md
docs/REPORT-docs-sync.md:203:A  docs/REPORT-beta5-commit.md
docs/REPORT-docs-sync.md:204:A  docs/REPORT-beta5-tag-push.md
docs/REPORT-docs-sync.md:209:A  docs/REPORT-release-beta3.md
docs/REPORT-docs-sync.md:210:A  docs/REPORT-release-beta4.md
docs/REPORT-docs-sync.md:236: docs/REPORT-beta5-ci-check.md             |  124 ++++
docs/REPORT-docs-sync.md:237: docs/REPORT-beta5-commit.md               |   99 +++
docs/REPORT-docs-sync.md:238: docs/REPORT-beta5-tag-push.md             |   92 +++
docs/REPORT-docs-sync.md:243: docs/REPORT-release-beta3.md              |  183 ++++++
docs/REPORT-docs-sync.md:244: docs/REPORT-release-beta4.md              |  145 +++++
docs/REPORT-ota-beta-not-found.md:91:  `docs/REPORT-beta5-ci-check.md`). GitHub's `releases/latest` endpoint excludes
docs/REPORT-ota-beta-not-found.md:115:| Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-beta5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
```

### Changed (6 lines — prose or backtick references)

```
CHANGED docs/REPORT-0.1.5-beta.5-commit.md:69
  before: - `docs/REPORT-release-beta3.md`
  after:  - `docs/REPORT-0.1.5-beta.3-release.md` (then named `REPORT-release-beta3.md`)
CHANGED docs/REPORT-0.1.5-beta.5-commit.md:70
  before: - `docs/REPORT-release-beta4.md`
  after:  - `docs/REPORT-0.1.5-beta.4-release.md` (then named `REPORT-release-beta4.md`)
CHANGED docs/REPORT-0.1.5-beta.5-commit.md:92
  before: (This file, `docs/REPORT-beta5-commit.md`, is now also untracked, per report convention.)
  after:  (This file, `docs/REPORT-0.1.5-beta.5-commit.md` — then named `REPORT-beta5-commit.md` — is now also untracked, per report convention.)
CHANGED docs/REPORT-0.1.5-beta.5-tag-push.md:92
  before: `docs/REPORT-beta5-commit.md` and this file.
  after:  `docs/REPORT-0.1.5-beta.5-commit.md` (then named `REPORT-beta5-commit.md`) and this file.
CHANGED docs/REPORT-ota-beta-not-found.md:91
  before:   `docs/REPORT-beta5-ci-check.md`). GitHub's `releases/latest` endpoint excludes
  after:    `docs/REPORT-0.1.5-beta.5-ci-check.md`). GitHub's `releases/latest` endpoint excludes
CHANGED docs/REPORT-ota-beta-not-found.md:115
  before: | Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-beta5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
  after:  | Five stable releases fill the per_page=5 window | public releases API listing (this report, and `REPORT-0.1.5-beta.5-ci-check.md`) | stable: 0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4 = 5; all betas listed after them |
```

Where the old name is the historically accurate one (the beta.5 commit report's list of what was untracked at the time, and both "this file" sentences), the new name is given with "(then named `…`)" so the record of the tree at that time is still readable.

### Deliberately left (21 lines — quoted command output)

| File:line | Why it stays |
|---|---|
| `docs/REPORT-0.1.5-beta.5-commit.md:87-88` | Two `?? docs/REPORT-release-beta[34].md` lines inside the fenced `git status --short` block quoted after that commit. |
| `docs/REPORT-1.0.0-commit.md:92` | Quoted raw output of that task's check 4 grep, which itself quoted `REPORT-ota-beta-not-found.md:115` as it read on 2026-09-09. |
| `docs/REPORT-docs-sync.md:35-37, 42-43` | `??` lines in that report's quoted pre-commit `git status`. |
| `docs/REPORT-docs-sync.md:202-204, 209-210` | `A ` lines in its quoted post-add `git status`. |
| `docs/REPORT-docs-sync.md:236-238, 243-244` | Rows of its quoted `git diff --stat`. |

## Step 3 — TEST-F3 onto disk

`cp "Claude outputs/TEST-F3-stuck-loop.md" docs/TEST-F3-stuck-loop.md`; the source under `Claude outputs/` (gitignored via `.gitignore:50`) was only read, not modified. In the docs copy, intro lines 3-6:

Before:

```
Written 2026-09-03. Owner-run, at the board. This is the last hardware check
before `0.1.4` (V1-scope item 11, "still owed"). Lives only in the Claude
Project; results go to `HARDWARE-bringup-log.md` as a new §14.x entry and
V1-scope item 11 gets closed.
```

After:

```
Written 2026-09-03. Owner-run, at the board. This is the last hardware check
before `0.1.4` (V1-scope item 11, "still owed"). Lives in `docs/` in the
firmware repo (this file), mirrored in the Claude Project; results go to
`HARDWARE-bringup-log.md` as a new §14.x entry and V1-scope item 11 gets
closed.
```

## Step 4 — docs/REPORTS.md (full text as committed)

```markdown
# Reports index

A `REPORT-*.md` file is Claude Code's fixed-path output for one task: a verdict line, the gate commands and their raw output, raw build or CI output, `git diff --stat` and the resulting SHA, every deviation from the task as written, and whatever could not be verified without hardware or CI. One is written even when a gate blocks and the task stops, and it is committed in the next docs commit rather than in the commit it describes; `REVIEW-`, `PLAN-` and `TEST-` files are the same shape of artefact for reviews, plans and hardware test plans.

- `REVIEW-2026-09-02.md` — 2026-09-02 — Code review — firmware/dial-idf, branch firmware/somnus-port, 2026-09-02 — (no verdict line)
- `PLAN-screen-layout-fixes.md` — 2026-09-04 — PLAN — Implementing the screen layout audit (for review) — (no verdict line)
- `REPORT-0.1.5-beta.3-release.md` — 2026-09-04 — Release report — 0.1.5-beta.3 — **Verdict: RELEASED.** Release commit `1aafca5` ("release: 0.1.5-beta.3") made, changing exactly `firmware/dial-idf/CMakeLists.txt` and `CHANGELOG.md`.
- `REPORT-0.1.5-beta.4-release.md` — 2026-09-04 — Release report — 0.1.5-beta.4 — **Verdict: RELEASED.** Gate passed clean on the first run.
- `REPORT-0.1.5-beta.5-ci-check.md` — 2026-09-04 — 0.1.5-beta.5 CI / release check — **Verdict: RELEASE CONFIRMED BUILT AND PUBLISHED.**
- `REPORT-0.1.5-beta.5-commit.md` — 2026-09-04 — 0.1.5-beta.5 commit (step 1 of 2 — commit only) — **Verdict: COMMITTED.**
- `REPORT-0.1.5-beta.5-tag-push.md` — 2026-09-04 — 0.1.5-beta.5 tag + push (step 2 of 2) — **Verdict: TAGGED AND PUSHED to the private fork only.**
- `REPORT-0.1.5-ci-check.md` — 2026-09-04 — 0.1.5 stable release — CI / publish check — **Verdict: RELEASE BUILT AND PUBLISHED AS STABLE.**
- `REPORT-0.1.5-graduation-commit.md` — 2026-09-04 — 0.1.5 graduation commit — **Verdict: COMMITTED.**
- `REPORT-0.1.5-tag-push.md` — 2026-09-04 — 0.1.5 stable tag + push — **Verdict: TAGGED AND PUSHED.**
- `REPORT-0.1.5-upgrade-path-verify.md` — 2026-09-04 — 0.1.4 → 0.1.5 OTA upgrade path, verified on hardware — **Verdict: PASS.**
- `REPORT-about-layout-battery-glyph.md` — 2026-09-04 — About layout pass + battery badge/percentage review (Sep 4 2026, evening) — (no verdict line)
- `REPORT-battery-pct-about-redesign.md` — 2026-09-04 — Battery percentage + About redesign (SPEC-power-sensing.md §11) — (no verdict line)
- `REPORT-docs-sync.md` — 2026-09-04 — docs-sync commit (README → 0.1.5, Sep 3–5 reports, NAMING audit, ignore Claude outputs) — **Verdict: DONE.** Gate passed, docs-only commit `5228de3` created on `firmware/somnus-port` and pushed to the `somnus` remote.
- `REPORT-handoff.md` — 2026-09-04 — Handoff — Sep 7 2026, end of session — (no verdict line)
- `REPORT-layout-phase1.md` — 2026-09-04 — Screen layout pass, phase 1 (Section S + Tier A) — **DONE.** Two commits on `main`: `a009840` (Section S: 10 new scenarios + the S3 stub hook + the settings-pad re-aim, 40 PNGs regenerated as the BEFORE set) and `5bf68a5` (Tier A: A1–A5 in four `dial_ui` files, the AFTER PNGs, the two doc edits).
- `REPORT-night-face.md` — 2026-09-04 — Run 3 — commit 4 (spec revision 3, three review fixes) — **DONE.** Commit 4 (all three revision-3 fixes, all in `scr_dial.c`) built clean with `idf.py build`, zero warnings; version/tag/CHANGELOG/push left untouched, `Claude outputs/` not added, as instructed.
- `REPORT-ota-beta-not-found.md` — 2026-09-04 — manual OTA check — serial capture — (no verdict line)
- `REPORT-power-indicator.md` — 2026-09-04 — plugged-in / on-battery indicator (0.1.5-beta.4) — **Verdict: DONE.** Both commits built clean and landed exactly as specified (detector/state/About row in commit 1, the two glyphs in commit 2); nothing out of section 10.6's scope was added; PROJECT_VER/CHANGELOG/tag/push were left untouched as instructed.
- `REPORT-releases-repo-readme.md` — 2026-09-04 — releases-repo README and third-party license fixes — **DONE.** README replaced with the Bedknob text (with the "open" phrase corrected to source-available), ESP-IDF row added, full Apache-2.0 / MIT / OFL-1.1 texts appended to the extensionless `THIRD_PARTY_LICENSES`, committed as `4097b6e` and pushed to `main`.
- `REPORT-screen-layout-audit.md` — 2026-09-04 — Layout audit of every dial screen (Sep 4 2026) — (no verdict line)
- `REPORT-layout-a1b.md` — 2026-09-05 — A1b: °C setpoint without a zero decimal; no unit label in relative mode — **DONE — committed as `40b784a`, simulator renders verified by measurement, `idf.py` clean build with zero compiler warnings, wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.5, no assert/panic/abort/Guru/`E (` lines, no reset).** No tag, no version bump, no push.
- `REPORT-layout-phase1-flash.md` — 2026-09-05 — Phase 1 layout build + wire flash (bench dial) — **DONE — flashed and booting clean.** The tree at `5bf68a5` + the report commit was built from clean (zero warnings, 31 `dial_ui` objects compiled), wire-flashed to `/dev/cu.usbmodem83401` (all four segments hash-verified), and the 40 s boot capture shows App version 0.1.5, compile time one minute before the flash, boot from the app at 0x20000, pad connected, no panic / assert / abort / Guru / `E (` lines, no reboot.
- `REPORT-rails-fix.md` — 2026-09-05 — fix(dial): absolute rails 12-42 whole degrees; knob and drag snap to the grid — **DONE — committed as `db7f9d8`, both routes off the grid closed (rails 120–420 in main.c, knob snaps-then-steps, drag release snaps), all six requested simulator cases driven through the real `on_knob` and pointer-indev paths and rendered as specified, `idf.py fullclean && idf.py build` with zero compiler warnings, wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.5, compile time 17:39:59, no assert/panic/abort/Guru/`E (` lines, no reset).** No tag, no version bump, no push.
- `REPORT-release-0.1.6-beta.1.md` — 2026-09-05 — release somnus-v0.1.6-beta.1 — **SHIPPED — `somnus-v0.1.6-beta.1` is published as a prerelease from release commit `46f65b2`: main and the tag pushed to `somnus` only, release run 33996956909 concluded `success` (build-and-release 5m23s, deploy-pages 12s), `gh release view` shows isPrerelease true / isDraft false / two assets (somnus-dial.bin 1,610,128 B, somnus-dial-merged.bin 1,741,200 B), and `/releases/latest` still points at `somnus-v0.1.5`.** No source changes; the release commit touches exactly `firmware/dial-idf/CMakeLists.txt` and `CHANGELOG.md`.
- `REPORT-release-0.1.6-beta.2.md` — 2026-09-05 — release somnus-v0.1.6-beta.2 — **SHIPPED — `somnus-v0.1.6-beta.2` is published as a prerelease from release commit `ee42066`: main and the tag pushed to `somnus` only, release run 34000488242 concluded `success` (build-and-release 5m5s, deploy-pages 7s), `gh release view` shows isPrerelease true / isDraft false / two assets (somnus-dial.bin 1,611,632 B, somnus-dial-merged.bin 1,742,704 B), and `/releases/latest` still points at `somnus-v0.1.5`.** The release commit touches exactly `firmware/dial-idf/CMakeLists.txt` and `CHANGELOG.md`.
- `REPORT-standby-face-c1.md` — 2026-09-05 — Standby face, commit 1: `standby_screen()` consumer, hard-wired to Temperature — **DONE — `03e6259`: `standby_screen()` added to `main.c`, both `nav_policy` STANDBY sites route through it, hard-wired to `SCR_DIAL`; `idf.py fullclean && idf.py build` with zero compiler warnings (33 `main` + `dial_ui` objects), wire-flashed to `/dev/cu.usbmodem83401`, 40 s boot capture clean (App version 0.1.6-beta.1, compile time 18:18:37, no assert/panic/abort/Guru/`E (` lines, no reset).** `main.c` is the only file in the commit.
- `REPORT-standby-face-c2.md` — 2026-09-05 — Standby face, commit 2: the setting (Temperature / Clock), Night standby rename — **DONE — `9dad0b2`: pref `ui/sb_face` (u8, default 1 = Temperature on fresh device and on upgrade, clamp-on-read to {0,1} → 1), `dial_state_get/set_standby_face()` in night_face's shape, `SCR_STANDBY_FACE` + `scr_standby_face.c` (Back / Temperature / Clock, checkmark, tap sets and returns), the "Standby face" row directly under Screen timeout, `standby_screen()` reading the pref, `SCR_STANDBY_FACE` in both sticky lists, the §4b "Night (clock)" → "Night (standby)" rename (label + caption only, pref untouched) with the 0 % comment, and the two comment fixes.**
- `REPORT-sticky-brightness.md` — 2026-09-05 — Brightness percent picker sticky against poll commits — **DONE — `eef9257`: `SCR_BRIGHTNESS` added to `nav_policy()`'s have-state sticky set next to `SCR_BRIGHTNESS_MENU` with a one-line comment; incremental `idf.py build` recompiled `main.c` only with zero warning lines; wire-flashed; 40 s boot capture clean (App version 0.1.6-beta.1, no assert/panic/abort/Guru/`E (` lines, no reset).** Step 0 committed the sticky-night report as `9d0d785`.
- `REPORT-sticky-night-pickers.md` — 2026-09-05 — Night mode and Night face pickers sticky against poll commits — **DONE — `d80f66e`: `SCR_NIGHT_MODE` and `SCR_NIGHT_FACE` added to both of `nav_policy()`'s sticky sets next to `SCR_STANDBY_FACE`; incremental `idf.py build` recompiled `main.c` only with zero warning lines; wire-flashed; 40 s boot capture clean (App version 0.1.6-beta.1, no assert/panic/abort/Guru/`E (` lines, no reset).** Step 0 committed the spec status and the commit-2 report as `cdd7a90`.
- `REPORT-layout-tier-bc.md` — 2026-09-06 — Layout audit Tier B + Tier C, stale comments, discovery log noise (0.1.6-beta.3 fix pass) — **Verdict: DONE WITH DEVIATIONS** — all items done; two deviations from the task block's wording (discovery lines muted per-tag rather than re-levelled to DEBUG, because they are IDF library ESP_LOGE calls; comment (a) was at dial_state.h:183-241 and :557-568, not line 501), one from the plan (netpick keeps `lv_obj_center` after the width/long-mode lines, the plan said "replaces").
- `REPORT-dial-display-audit.md` — 2026-09-07 — Dial °F Setpoint Display Audit — **H1 (benign) is what the source implements.** The absolute-face °F numeral is a display-only `round(°C × 9/5 + 32)` of a canonical whole-°C setpoint (1.0 °C per detent, 31 levels, 12.0–42.0 °C); the write path posts that same whole-°C value straight to `POST /api/target_t`.
- `REPORT-release-0.1.6-beta.3.md` — 2026-09-07 — Release somnus-v0.1.6-beta.3 (2026-09-06) — **RELEASED.** Release commit `53fc0c9dad016dac148a9887d91773b2a835c51c`, annotated tag `somnus-v0.1.6-beta.3` on it, both pushed to the remote named `somnus` only.
- `REPORT-0.1.6-graduation.md` — 2026-09-09 — 0.1.6 stable graduation (2026-09-07) — **PASS.** 
- `REPORT-1.0.0-commit.md` — 2026-09-09 — somnus-v1.0.0 release commit (commit only; not tagged, not pushed) — **DONE** — commit `c56cc4d` created on `main` on top of `53a42c5`, touching exactly the five allowed files; not tagged, not pushed.
- `REPORT-1.0.0-tag-push.md` — 2026-09-09 — somnus-v1.0.0 tag, push, CI and published-release verification — **DONE** — `somnus-v1.0.0` tagged at `7c106fb`, pushed to the `somnus` remote only, release run 34412911352 and ci run 34412910378 both succeeded, and the public release is live as non-prerelease with both assets, `/releases/latest` = `somnus-v1.0.0`, Pages serving the merged image with HTTP 200.
- `REPORT-docs-housekeeping.md` — 2026-09-09 — docs housekeeping — five 0.1.5 reports renamed, TEST-F3 on disk, REPORTS.md index — **DONE** — five 0.1.5-era reports renamed with `git mv`, six prose cross-references updated, TEST-F3 tracked in docs/, REPORTS.md added, committed and pushed to `somnus` only.
- `REPORT-sim-screens-push.md` — 2026-09-09 — sim-screens report committed; main pushed to somnus — **DONE** — docs/REPORT-sim-version-screens.md committed as `195e77c` and `main` pushed to the `somnus` remote (7c106fb..195e77c); local HEAD and `somnus/main` both read 195e77c; the `somnus-v1.0.0` tag still points at 7c106fb locally and on the remote; ci run 34414740070 started on the push and was still in progress when this report was written.
- `REPORT-sim-version-screens.md` — 2026-09-09 — simulator reads PROJECT_VER at build time; screens regenerated at 1.0.0 — **DONE** — commit `effb9c6` on `main` (on top of `7c106fb` = `somnus-v1.0.0`): the simulator now derives the installed version (1.0.0) and the advertised OTA version (1.0.1) from `firmware/dial-idf/CMakeLists.txt` at configure time, the two hardcoded literals are gone behind `#error` guards, and 6 of 49 screens regenerated with the new strings while the other 43 are byte-identical.
- `TEST-F3-stuck-loop.md` — 2026-09-09 — Test plan: F3 — escape hatches work while stuck in the connect loop — (no verdict line)

Regenerate this index when adding reports; it is hand-maintained, not built.
```

**Entry count: 40.** Files under `docs/` matching `REPORT-*.md`, `REVIEW-*.md`, `PLAN-*.md`, `TEST-*.md` at commit time: 39, plus this task's own report (`docs/REPORT-docs-housekeeping.md`, untracked, written after the commit) = 40. The two numbers match once this file exists. (`REPORTS.md` itself does not match `REPORT-*.md` and is not an entry.)

How the fields were derived:

- **Date**: `git log --diff-filter=A --format=%ad --date=short -- <file> | tail -1`, run against the pre-rename path for the five renamed files (their history is now reachable via `--follow`, but the staged rename had no commit yet when the index was generated). Files with no commit (`TEST-F3-stuck-loop.md`, `REPORT-sim-screens-push.md`, `REPORTS.md`'s own sibling `REPORT-docs-housekeeping.md`) use today. Ties within a date are in filename order.
- **Title**: the file's first `# ` line with a leading `REPORT — ` / `REPORT: ` / `Report — ` stripped. `REPORT-night-face.md`'s H1 is literally "Run 3 — commit 4 (spec revision 3, three review fixes)" (it is a multi-run report whose H1 names the last run) and is reproduced as-is per the rule.
- **Verdict**: a line starting `**Verdict`, or the first bold line after a `## Verdict` / `## 1. Verdict` heading (matched case-insensitively, which is what brings in `REPORT-dial-display-audit.md`'s `## VERDICT`). Cut to one sentence: the bold opener plus the sentence that follows it, or, where the whole bold block is several sentences, its first sentence with the bold closed. Nine files have no such line and read "(no verdict line)": `REVIEW-2026-09-02.md`, `PLAN-screen-layout-fixes.md`, `TEST-F3-stuck-loop.md` (not report-shaped), `REPORT-screen-layout-audit.md`, `REPORT-about-layout-battery-glyph.md`, `REPORT-battery-pct-about-redesign.md`, `REPORT-ota-beta-not-found.md` (no verdict text at all), and `REPORT-handoff.md`, whose only verdict is a mid-document list item ("- Verdict: **H1 confirmed, H2 not supported by the code.**", line 40) about one sub-topic rather than the file, so it does not qualify under the rule.

## Step 5 — commit and push

```
$ git add -A docs/
$ git --no-optional-locks status --short
R  docs/REPORT-release-beta3.md -> docs/REPORT-0.1.5-beta.3-release.md
R  docs/REPORT-release-beta4.md -> docs/REPORT-0.1.5-beta.4-release.md
R  docs/REPORT-beta5-ci-check.md -> docs/REPORT-0.1.5-beta.5-ci-check.md
R  docs/REPORT-beta5-commit.md -> docs/REPORT-0.1.5-beta.5-commit.md
R  docs/REPORT-beta5-tag-push.md -> docs/REPORT-0.1.5-beta.5-tag-push.md
M  docs/REPORT-ota-beta-not-found.md
A  docs/REPORT-sim-screens-push.md
A  docs/REPORTS.md
A  docs/TEST-F3-stuck-loop.md

$ git --no-optional-locks status --short -- firmware simulator web-flasher .github
(empty)

$ git rev-parse --short HEAD
3fd1cc1

$ git diff --stat -M HEAD~1 HEAD
 ...ase-beta3.md => REPORT-0.1.5-beta.3-release.md} |   0
 ...ase-beta4.md => REPORT-0.1.5-beta.4-release.md} |   0
 ...ci-check.md => REPORT-0.1.5-beta.5-ci-check.md} |   0
 ...ta5-commit.md => REPORT-0.1.5-beta.5-commit.md} |   6 +-
 ...tag-push.md => REPORT-0.1.5-beta.5-tag-push.md} |   2 +-
 docs/REPORT-ota-beta-not-found.md                  |   4 +-
 docs/REPORT-sim-screens-push.md                    |  96 +++++++++++
 docs/REPORTS.md                                    |  46 ++++++
 docs/TEST-F3-stuck-loop.md                         | 175 +++++++++++++++++++++
 9 files changed, 323 insertions(+), 6 deletions(-)

$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   195e77c..3fd1cc1  main -> main
(exit 0)

$ git rev-parse --short HEAD
3fd1cc1
$ git rev-parse --short somnus/main
3fd1cc1

$ git rev-parse --short 'somnus-v1.0.0^{commit}'
7c106fb
$ git ls-remote --tags somnus somnus-v1.0.0
e4a35ebd2eef3f39da46bb77ce3687c5d9f2f92d	refs/tags/somnus-v1.0.0
```

No push to `origin`; no tag created or moved. This report is left untracked.

## Deviations from this spec

1. **`git diff --stat` was run with `-M`** so the five renames show as renames (`{old => new}`, 0 / 6 / 2 lines) rather than as five deletions plus five additions; content is identical either way.
2. **Two extra lines updated in the beta.5 commit report** (its lines 69-70, the bulleted list of files "not staged or committed"). The spec named only line 92 there as a known prose hit, but the rule as written ("a markdown link/backtick reference → update") covers a backtick list item, so they were updated with the old name kept in parentheses. Reversible if you'd rather treat that list as quoted record.
3. **Verdict cut rule applied to all-bold multi-sentence verdicts** by taking the first sentence inside the bold and closing it, rather than the (much longer) first sentence after the whole bold block; the spec's "one sentence" intent was preferred over its literal "after the bold opener".
4. **Case-insensitive `## Verdict` match**, which gives `REPORT-dial-display-audit.md` a verdict entry instead of "(no verdict line)".
5. **Commit trailers.** The commit carries the session's required `Co-Authored-By` / `Claude-Session` lines in addition to the spec's two `-m` paragraphs.
6. **Entry count includes this report**, which did not exist when the index was generated; the count therefore reads 40 against 39 files on disk at commit time and 40 now.

## Hardware

Nothing to verify on hardware; this task touched only files under `docs/`.
