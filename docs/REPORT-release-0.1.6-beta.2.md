# REPORT — release somnus-v0.1.6-beta.2

Date: 2026-09-05 19:07–19:14 CDT (2026-09-06 00:07–00:14 UTC; the CHANGELOG date is the UTC day, per the task). Branch `main`. Remote `somnus` = git@github.com:matthewclaude/bedknob-for-somnus.git.

## Verdict

**SHIPPED — `somnus-v0.1.6-beta.2` is published as a prerelease from release commit `ee42066`: main and the tag pushed to `somnus` only, release run 34000488242 concluded `success` (build-and-release 5m5s, deploy-pages 7s), `gh release view` shows isPrerelease true / isDraft false / two assets (somnus-dial.bin 1,611,632 B, somnus-dial-merged.bin 1,742,704 B), and `/releases/latest` still points at `somnus-v0.1.5`.** The release commit touches exactly `firmware/dial-idf/CMakeLists.txt` and `CHANGELOG.md`. Nothing pushed to `origin`. Hardware precondition per the task: the owner verified the Standby face setting on the bench dial on 2026-09-05 (°C and °F, night and day, Clock and Temperature, choice survives reboot) and sat on the night and brightness pickers through polls.

## Step 0

```
$ git add docs/REPORT-sticky-brightness.md && git commit -m "docs: sticky brightness report"
a961936 docs: sticky brightness report
```

(It was not already committed, so the step ran.)

## Gate

```
$ git --no-optional-locks status --short
(empty)
$ git log --oneline -12
a961936 docs: sticky brightness report
eef9257 fix(nav): brightness percent picker is sticky against poll commits
9d0d785 docs: sticky night pickers report
d80f66e fix(nav): Night mode and Night face pickers are sticky against poll commits
cdd7a90 docs: standby face verified on hardware; commit 2 report
9dad0b2 feat(standby): Standby face setting (Temperature/Clock, default Temperature); Night clock row renamed Night standby
93c0f9c docs: SPEC-standby-face — commit 1 verified on hardware; §4b Night clock row rename
2ad672a docs: SPEC-standby-face default Temperature (owner ruling); commit 1 report
03e6259 feat(standby): standby_screen() helper; dial face at STANDBY (hard-wired, spec §6 commit 1)
1f04499 docs: report
c559b8e docs: SPEC-standby-face — Clock/Temperature standby face, replaces the screen-timeout Off idea
46f65b2 release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and absolute-rails fix
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.6-beta.1")
$ git tag -l somnus-v0.1.6-beta.2
(nothing)
$ git remote get-url somnus
git@github.com:matthewclaude/bedknob-for-somnus.git
$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
```

All required commits present (03e6259, 9dad0b2, d80f66e, and eef9257 "fix(nav): brightness percent picker is sticky against poll commits"). Gate passed; proceeded.

## Release commit

```
$ git add firmware/dial-idf/CMakeLists.txt && git add CHANGELOG.md
$ git diff --cached --stat
 CHANGELOG.md                     | 22 ++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 23 insertions(+), 1 deletion(-)
$ git commit -m "release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky settings pickers"
ee42066 release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky settings pickers
```

Release commit: **`ee420666ae374dcfbc08da914fbd43a3aa703bc8`**. `PROJECT_VER` is now `"0.1.6-beta.2"` (CI's "Verify tag matches PROJECT_VER" step passed on it).

### CHANGELOG section as committed (verbatim, `git show ee42066:CHANGELOG.md`, heading to the line before `## 0.1.6-beta.1 …`)

```markdown
## 0.1.6-beta.2 — 2026-09-06 (beta)

### Added

- **Settings → Standby face.** Choose what the dial shows after the screen
  timeout: **Temperature** (the default) keeps the dial face on screen,
  dimmed — at night with Night face set to Number only that's the big
  setpoint alternating with the water temperature; by day it's the dial
  itself. **Clock** is the previous behaviour. Everything else about standby
  is unchanged: the timeout, night dimming, automatic overnight updates and
  the update prompt all work the same whichever face you pick.
- The Brightness row for the night standby level is now called
  **Night (standby)** instead of "Night (clock)", since it applies to
  whichever standby face you choose. Same setting, same value; 0 % still
  means the standby face is off at night.

### Fixed

- The Night mode, Night face and brightness pickers no longer get kicked
  back to the dial face by a routine background refresh while you're on
  them.
```

The workflow's extractor picked it up (the "Extract release notes from CHANGELOG.md" step passed); the release body on GitHub is this text minus the heading.

## Tag

```
$ git tag -a somnus-v0.1.6-beta.2 -m "somnus-v0.1.6-beta.2"
$ git tag -l somnus-v0.1.6-beta.2
somnus-v0.1.6-beta.2
$ git rev-parse somnus-v0.1.6-beta.2^{commit}
ee420666ae374dcfbc08da914fbd43a3aa703bc8
$ git cat-file -t somnus-v0.1.6-beta.2
tag
```

Annotated tag object `6050e8bf7d7b83bf60fc8e7b7c3fcd34ebb7e1e8` → commit `ee42066`.

## Push — raw output

`git push somnus main` (exit 0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
   93c0f9c..ee42066  main -> main
```

`git push somnus somnus-v0.1.6-beta.2` (exit 0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v0.1.6-beta.2 -> somnus-v0.1.6-beta.2
```

Both at 19:08 CDT. The main push range `93c0f9c..ee42066` carries the eight commits above the last push (cdd7a90 … ee42066). Remote after (`git ls-remote somnus main somnus-v0.1.6-beta.2`):

```
ee420666ae374dcfbc08da914fbd43a3aa703bc8	refs/heads/main
6050e8bf7d7b83bf60fc8e7b7c3fcd34ebb7e1e8	refs/tags/somnus-v0.1.6-beta.2
```

`origin` untouched (push URL `no_push`).

## CI — raw output

`gh run list --repo matthewclaude/bedknob-for-somnus --limit 3` (~10 s after the push):

```
in_progress		release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky sett…	release	somnus-v0.1.6-beta.2	push	34000488242	7s	2026-09-06T00:08:07Z
in_progress		release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky sett…	ci	main	push	34000487421	8s	2026-09-06T00:08:06Z
completed	success	docs: SPEC-standby-face — commit 1 verified on hardware; §4b Night cl…	ci	main	push	33998693371	5m11s	2026-09-05T23:26:49Z
```

Run for the tag: **34000488242** (workflow `release`, branch `somnus-v0.1.6-beta.2`). The `ci` workflow also ran on main (34000487421).

`gh run watch 34000488242 --repo matthewclaude/bedknob-for-somnus --exit-status` (exit 0, returned 19:13:29 CDT):

```
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  * Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m5s (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
* deploy-pages (ID 101398956579)

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m5s (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
* deploy-pages (ID 101398956579)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  * Deploy to gh-pages
  * Post Run actions/checkout@v4

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m5s (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
* deploy-pages (ID 101398956579)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  ✓ Deploy to gh-pages
  ✓ Post Run actions/checkout@v4
  ✓ Complete job

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

✓ somnus-v0.1.6-beta.2 release · 34000488242
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m5s (ID 101398348671)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
✓ deploy-pages in 7s (ID 101398956579)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  ✓ Deploy to gh-pages
  ✓ Post Run actions/checkout@v4
  ✓ Complete job

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2
```

`gh run view 34000488242 --json conclusion,status,headSha,displayTitle,url`:

```
{"conclusion":"success","displayTitle":"release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky sett…","headSha":"ee420666ae374dcfbc08da914fbd43a3aa703bc8","status":"completed","url":"https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34000488242"}
```

**Run 34000488242 — conclusion: success** (build-and-release 5m5s, deploy-pages 7s; 5m18s total). The `ci` run on main (34000487421) also `success` (5m2s). `gh run list` afterwards:

```
completed	success	release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky sett…	release	somnus-v0.1.6-beta.2	push	34000488242	5m18s	2026-09-06T00:08:07Z
completed	success	release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky sett…	ci	main	push	34000487421	5m2s	2026-09-06T00:08:06Z
completed	success	docs: SPEC-standby-face — commit 1 verified on hardware; §4b Night cl…	ci	main	push	33998693371	5m11s	2026-09-05T23:26:49Z
```

The one annotation is GitHub's Node.js 20 deprecation notice on `actions/checkout@v4`, `actions/upload-artifact@v4`, `softprops/action-gh-release@v2` (informational; present on every prior run).

## Release — `gh release view somnus-v0.1.6-beta.2 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets`

```json
{
  "assets": [
    {
      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546487234",
      "contentType": "application/octet-stream",
      "createdAt": "2026-09-06T00:13:12Z",
      "digest": "sha256:8c9a24b5afa5991d312b38934b1870a6dd90c6d6ae50a990495bd216c86094ea",
      "downloadCount": 0,
      "id": "RA_kwDOULeAcc4gkrvC",
      "label": "",
      "name": "somnus-dial-merged.bin",
      "size": 1742704,
      "state": "uploaded",
      "updatedAt": "2026-09-06T00:13:12Z",
      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.2/somnus-dial-merged.bin"
    },
    {
      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546487233",
      "contentType": "application/octet-stream",
      "createdAt": "2026-09-06T00:13:12Z",
      "digest": "sha256:7c06f15195347f69b7bd0dd65928013fc160f528b6d4b0458290126ea9ccbf3a",
      "downloadCount": 0,
      "id": "RA_kwDOULeAcc4gkrvB",
      "label": "",
      "name": "somnus-dial.bin",
      "size": 1611632,
      "state": "uploaded",
      "updatedAt": "2026-09-06T00:13:12Z",
      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.2/somnus-dial.bin"
    }
  ],
  "isDraft": false,
  "isPrerelease": true,
  "tagName": "somnus-v0.1.6-beta.2"
}
```

| Check | Result |
|---|---|
| tagName | `somnus-v0.1.6-beta.2` |
| isPrerelease | **true** |
| isDraft | **false** |
| assets | **2** — `somnus-dial-merged.bin` (1,742,704 B, sha256 8c9a24b5…), `somnus-dial.bin` (1,611,632 B, sha256 7c06f151…), both `uploaded` at 00:13:12Z |

## `/releases/latest`

```
$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
somnus-v0.1.5
```

Still **`somnus-v0.1.5`** — the prerelease did not become "latest".

## Git state after this task

```
$ git log --oneline -3
ee42066 release: 0.1.6-beta.2 — Standby face (Temperature/Clock); sticky settings pickers
a961936 docs: sticky brightness report
eef9257 fix(nav): brightness percent picker is sticky against poll commits
$ git --no-optional-locks status --short
(empty before this file; now only ?? docs/REPORT-release-0.1.6-beta.2.md)
```

This report is untracked and not part of the release commit.

## Deviations from the spec

1. **Commit message trailers.** Both commits (`a961936`, `ee42066`) carry the session's two attribution trailer lines after the spec's title; the titles are verbatim.
2. **CHANGELOG wording.** The two "Added" bullets and the "Fixed" bullet are the task's text with the file's own bolding conventions applied (the row/value names in bold, matching the 0.1.5 sections); no words changed.

Everything else: none.

## Not verified

- **The OTA pull onto the bench dial** with "Beta builds" on — the owner's next step. Expected on the next check: `ota: latest 0.1.6-beta.2, running 0.1.6-beta.1 -- update available`, then the install and a reboot into 0.1.6-beta.2 with the app marked valid. (The bench dial currently runs a wire-flashed 0.1.6-beta.1 build at `eef9257` — the same code as this release minus the version string — with Standby face set to Clock in NVS; that choice should survive the OTA the same way it survived the wire flashes.)
- **The flasher page's beta option** serving the new merged image after the Pages job (`deploy-pages` succeeded; the site itself was not fetched).
- **Stable graduation** (a plain 0.1.6 tag) remains future work; `latest` stays 0.1.5 until then.
