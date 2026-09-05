# REPORT — release somnus-v0.1.6-beta.1

Date: 2026-09-05 17:48–17:55 CDT (22:48–22:55 UTC). Branch `main`. Remote `somnus` = git@github.com:matthewclaude/bedknob-for-somnus.git.

## Verdict

**SHIPPED — `somnus-v0.1.6-beta.1` is published as a prerelease from release commit `46f65b2`: main and the tag pushed to `somnus` only, release run 33996956909 concluded `success` (build-and-release 5m23s, deploy-pages 12s), `gh release view` shows isPrerelease true / isDraft false / two assets (somnus-dial.bin 1,610,128 B, somnus-dial-merged.bin 1,741,200 B), and `/releases/latest` still points at `somnus-v0.1.5`.** No source changes; the release commit touches exactly `firmware/dial-idf/CMakeLists.txt` and `CHANGELOG.md`. Nothing was pushed to `origin`. Hardware precondition: the owner verified `db7f9d8` on the bench dial on 2026-09-05 (42 rail holds, 12 → 42 → 42 walk clean, °C unit clear of the ring), per the task.

## Step 0

```
$ git add docs/REPORT-rails-fix.md && git commit -m "docs: rails fix report"
[main baa7f72] docs: rails fix report
```

## Gate

```
$ git --no-optional-locks status --short
(empty)
$ git log --oneline -8
baa7f72 docs: rails fix report
db7f9d8 fix(dial): absolute rails 12-42 whole degrees; knob and drag snap to the grid
42eb3f4 docs: A1b report
40b784a ui(A1b): °C setpoint without a zero decimal; no unit label in relative mode
e2f2996 docs: phase 1 flash report
4b114c5 docs: phase 1 layout report
5bf68a5 ui: layout fixes A1-A5 from the screen audit
a009840 sim: scenarios for the layout audit (S1-S9, timezone, night-face)
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")
$ git tag -l somnus-v0.1.6-beta.1
(nothing)
$ git remote get-url somnus
git@github.com:matthewclaude/bedknob-for-somnus.git
$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
```

All four required commits present (db7f9d8, 40b784a, 5bf68a5, a009840). Gate passed; proceeded.

## Release commit

```
$ git add firmware/dial-idf/CMakeLists.txt && git add CHANGELOG.md
$ git diff --cached --stat
 CHANGELOG.md                     | 28 ++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 29 insertions(+), 1 deletion(-)
$ git commit -m "release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and absolute-rails fix"
46f65b2 release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and absolute-rails fix
$ git diff --stat HEAD~1 HEAD
 CHANGELOG.md                     | 28 ++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 29 insertions(+), 1 deletion(-)
```

Release commit: **`46f65b29a376da120221dc3497296fa6a18e0644`**. `PROJECT_VER` is now `"0.1.6-beta.1"` (the CI "Verify tag matches PROJECT_VER" step passed on it).

### CHANGELOG section as committed (verbatim, `git show 46f65b2:CHANGELOG.md`, from the new heading to the line before `## 0.1.5 — 2026-09-05`)

```markdown
## 0.1.6-beta.1 — 2026-09-05 (beta)

### Fixed

- **In °C, turning past 42 no longer lands on 42.3** and drags every later
  value off by a tenth; the dial stops at 42, the same top as the Somnus
  app. A setpoint set to a half degree from the app snaps to a whole degree
  on the first knob click or drag.
- **Dragging the temperature handle** now lands on whole degrees too, so
  the pad is never asked for a tenth.
- **The °C setpoint reads "34", not "34.0"** (every Somnus setpoint is a
  whole degree). The unit no longer sits on the ring or under the handle in
  °C, and relative mode shows just the level ("+15") with no suffix.
- **The unit follows the number** instead of a fixed spot, so nothing
  overlaps at any value.
- **Settings:** the Night mode and Timezone rows stack their value on a
  second line instead of running into the label.
- **Lists:** the rows farthest from the selection shrink a bit more so
  their text stays fully inside the round display.
- **The "Update available" line** on the dial face sits clear of the page
  dots.

### Internal

- Simulator scenarios for every screen state the layout audit measured,
  including knob- and drag-driven ones; 47 reference screenshots
  regenerated.
```

Inserted directly above `## 0.1.5 — 2026-09-05`. The workflow's extractor (`awk`: heading `== "## " ver` or starting with `"## " ver " "`, to the next `## `) picked it up — the release body on GitHub is this text minus the heading.

## Tag

```
$ git tag -a somnus-v0.1.6-beta.1 -m "somnus-v0.1.6-beta.1"
$ git tag -l somnus-v0.1.6-beta.1
somnus-v0.1.6-beta.1
$ git rev-parse somnus-v0.1.6-beta.1^{commit}
46f65b29a376da120221dc3497296fa6a18e0644
$ git cat-file -t somnus-v0.1.6-beta.1
tag
```

Annotated tag object `1ec4f42c02b3c374a7ab6ab23e36436690ec2872` → commit `46f65b2`.

## Push — raw output

`git push somnus main` (exit 0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
   09d1ba2..46f65b2  main -> main
```

`git push somnus somnus-v0.1.6-beta.1` (exit 0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v0.1.6-beta.1 -> somnus-v0.1.6-beta.1
```

Both at 17:48 CDT. Remote after the push (`git ls-remote somnus main somnus-v0.1.6-beta.1`):

```
46f65b29a376da120221dc3497296fa6a18e0644	refs/heads/main
1ec4f42c02b3c374a7ab6ab23e36436690ec2872	refs/tags/somnus-v0.1.6-beta.1
```

`origin` untouched (push URL `no_push`).

## CI — raw output

`gh run list --repo matthewclaude/bedknob-for-somnus --limit 3` (taken ~10 s after the push):

```
in_progress		release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and a…	release	somnus-v0.1.6-beta.1	push	33996956909	9s	2026-09-05T22:48:43Z
in_progress		release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and a…	ci	main	push	33996956512	10s	2026-09-05T22:48:42Z
completed	success	docs: RELEASES-README matches what was published to somnus-dial-relea…	ci	main	push	33939906277	5m34s	2026-09-05T02:44:01Z
```

Run for the tag: **33996956909** (workflow `release`, branch `somnus-v0.1.6-beta.1`, event push). The `ci` workflow also ran on main (33996956512).

`gh run watch 33996956909 --repo matthewclaude/bedknob-for-somnus --exit-status` (exit 0, returned 17:54:27 CDT):

```
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push less than a minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 1 minute ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 2 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 3 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 4 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  * Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
* build-and-release (ID 101389057595)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  * Publish GitHub Release
  * Post Run actions/checkout@v4
Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m23s (ID 101389057595)
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
* deploy-pages (ID 101389691984)

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m23s (ID 101389057595)
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
* deploy-pages (ID 101389691984)
  ✓ Set up job
  * Run actions/checkout@v4
  * Download merged image
  * Channel directory
  * Assemble Pages site
  * Deploy to gh-pages
  * Post Run actions/checkout@v4

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

Refreshing run status every 3 seconds. Press Ctrl+C to quit.

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m23s (ID 101389057595)
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
* deploy-pages (ID 101389691984)
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

* somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m23s (ID 101389057595)
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
* deploy-pages (ID 101389691984)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  ✓ Deploy to gh-pages
  * Post Run actions/checkout@v4

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

✓ somnus-v0.1.6-beta.1 release · 33996956909
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m23s (ID 101389057595)
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
✓ deploy-pages in 12s (ID 101389691984)
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

`gh run view 33996956909 --json conclusion,status,headSha,displayTitle,url`:

```
{"conclusion":"success","displayTitle":"release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and a…","headSha":"46f65b29a376da120221dc3497296fa6a18e0644","status":"completed","url":"https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/33996956909"}
```

**Run 33996956909 — conclusion: success** (build-and-release 5m23s, deploy-pages 12s; total 5m42s). The `ci` run on main (33996956512) also concluded `success` (5m11s). `gh run list` afterwards:

```
completed	success	release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and a…	release	somnus-v0.1.6-beta.1	push	33996956909	5m42s	2026-09-05T22:48:43Z
completed	success	release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and a…	ci	main	push	33996956512	5m11s	2026-09-05T22:48:42Z
completed	success	docs: RELEASES-README matches what was published to somnus-dial-relea…	ci	main	push	33939906277	5m34s	2026-09-05T02:44:01Z
```

The one annotation is GitHub's Node.js 20 deprecation notice against `actions/checkout@v4`, `actions/upload-artifact@v4`, `softprops/action-gh-release@v2` (forced onto Node 24; informational, present on prior runs too).

## Release — `gh release view somnus-v0.1.6-beta.1 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets`

```json
{
  "assets": [
    {
      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546402673",
      "contentType": "application/octet-stream",
      "createdAt": "2026-09-05T22:54:06Z",
      "digest": "sha256:571280ee640c5ce7097e78e340276db8a3616eb2d2737eda1d6389c648f10444",
      "downloadCount": 0,
      "id": "RA_kwDOULeAcc4gkXFx",
      "label": "",
      "name": "somnus-dial-merged.bin",
      "size": 1741200,
      "state": "uploaded",
      "updatedAt": "2026-09-05T22:54:06Z",
      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.1/somnus-dial-merged.bin"
    },
    {
      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546402674",
      "contentType": "application/octet-stream",
      "createdAt": "2026-09-05T22:54:06Z",
      "digest": "sha256:660d4c31b753871dce4cbba1f014fc372a8a88388735654747692f30548d4fcc",
      "downloadCount": 0,
      "id": "RA_kwDOULeAcc4gkXFy",
      "label": "",
      "name": "somnus-dial.bin",
      "size": 1610128,
      "state": "uploaded",
      "updatedAt": "2026-09-05T22:54:06Z",
      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.1/somnus-dial.bin"
    }
  ],
  "isDraft": false,
  "isPrerelease": true,
  "tagName": "somnus-v0.1.6-beta.1"
}
```

| Check | Result |
|---|---|
| tagName | `somnus-v0.1.6-beta.1` |
| isPrerelease | **true** |
| isDraft | **false** |
| assets | **2** — `somnus-dial-merged.bin` (1,741,200 B, sha256 571280ee…), `somnus-dial.bin` (1,610,128 B, sha256 660d4c31…), both `uploaded` at 22:54:06Z |

## `/releases/latest`

```
$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
somnus-v0.1.5
```

Still **`somnus-v0.1.5`** — the prerelease did not become "latest".

## Git state after this task

```
$ git log --oneline -3
46f65b2 release: 0.1.6-beta.1 — screen layout pass (audit Tier A + A1b) and absolute-rails fix
baa7f72 docs: rails fix report
db7f9d8 fix(dial): absolute rails 12-42 whole degrees; knob and drag snap to the grid
$ git --no-optional-locks status --short
(empty before this file was written; now only ?? docs/REPORT-release-0.1.6-beta.1.md)
```

This report is untracked and not part of the release commit.

## Deviations from the spec

1. **Commit message trailers.** Both commits (`baa7f72`, `46f65b2`) carry the session's two attribution trailer lines after the spec's title; the titles are verbatim.
2. **Tag type.** The spec asked for an annotated tag (`git tag -a … -m`) and that is what was made. Noting only that every earlier `somnus-v*` tag in this repo is lightweight (`commit` type); this one is a `tag` object. The workflow keys on the ref name, and the run and release confirm it made no difference.

Everything else: none.

## Not verified

- **The OTA pull onto the bench dial** with "Beta builds" on — the owner's next step. Expected log line on the next check: `ota: latest 0.1.6-beta.1, running 0.1.5 -- update available`, then the install and a reboot into 0.1.6-beta.1 with the app marked valid.
- **The flasher page's beta option** serving the new merged image: the `deploy-pages` job (12 s, `Channel directory` → beta, `Deploy to gh-pages`) succeeded, but the Pages site itself was not fetched; confirm by loading the flasher, choosing Beta, and checking it offers 0.1.6-beta.1 (`firmware/beta/somnus-dial-merged.bin`, 1,741,200 B, sha256 571280ee…).
- **Beta-to-stable graduation** (0.1.6 plain tag) remains future work; `latest` stays 0.1.5 until then.
