# REPORT — 0.1.6 stable graduation (2026-09-07)

**PASS.** Gate passed on all four conditions. Two local commits made on `main`: docs commit
`7fadc38`, release commit `53a42c5`. **No tag created. Nothing pushed.** `somnus/main` is still
at `c47fdc2`; the two new commits are exactly what `git log somnus/main..HEAD` shows. Newest
`somnus-v*` tag is still `somnus-v0.1.6-beta.3`. No firmware build was run (§3).

## 1. Verdict

**PASS.**

| # | Condition | Actual | Result |
|---|-----------|--------|--------|
| 1 | branch is `main` | `main` | PASS |
| 2 | `PROJECT_VER` reads `0.1.6-beta.3` | `firmware/dial-idf/CMakeLists.txt:22` = `set(PROJECT_VER "0.1.6-beta.3")` | PASS |
| 3 | newest `somnus-v*` tag is `somnus-v0.1.6-beta.3` | `somnus-v0.1.6-beta.3` | PASS |
| 4 | the ONLY modified tracked file is `docs/REPORT-handoff.md` | ` M docs/REPORT-handoff.md` and nothing else modified | PASS |

Untracked at gate time: `docs/REPORT-0.1.6-graduation.md` (the previous blocked run's report,
overwritten by this file), `docs/REPORT-release-0.1.6-beta.3.md`, `docs/SPEC-standby-poll.md`.

## 2. Gate check — raw output

```
$ git --no-optional-locks status --short
 M docs/REPORT-handoff.md
?? docs/REPORT-0.1.6-graduation.md
?? docs/REPORT-release-0.1.6-beta.3.md
?? docs/SPEC-standby-poll.md

$ git rev-parse --abbrev-ref HEAD
main

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.6-beta.3")

$ git tag --list 'somnus-v*' --sort=-v:refname | head -5
somnus-v0.1.6-beta.3
somnus-v0.1.6-beta.2
somnus-v0.1.6-beta.1
somnus-v0.1.5-beta.5
somnus-v0.1.5-beta.4

$ git log --oneline -3
c47fdc2 docs: audit absolute-face F display against pad API and Somnus app
53fc0c9 Release somnus-v0.1.6-beta.3
fe7c556 fix(discovery): mute per-host HTTP-stack errors during a scan; stale comments

$ git log somnus/main..HEAD --oneline
(no output — exit 0; HEAD == somnus/main == c47fdc2 at gate time)
```

## 3. Build output

**No build was run.** The release commit changes a CMake `set()` string, a Markdown changelog
and a Markdown README; nothing under `components/`, `main/` or the simulator changed. The
instructions did not call for a build, and the CI release workflow builds from the tag when
the owner pushes it. There is no build output to report. What a build would have confirmed
(the `App version: 0.1.6` string on boot) is listed under §8.

## 4. `git diff --stat` and commit SHAs

### Commit 1 — docs

```
$ git add docs/REPORT-handoff.md docs/REPORT-release-0.1.6-beta.3.md docs/SPEC-standby-poll.md
$ git diff --cached --stat
 docs/REPORT-handoff.md              | 127 +++++++++++++------------
 docs/REPORT-release-0.1.6-beta.3.md | 179 ++++++++++++++++++++++++++++++++++++
 docs/SPEC-standby-poll.md           | 113 +++++++++++++++++++++++
 3 files changed, 362 insertions(+), 57 deletions(-)

$ git show --stat --format="%H%n%s" HEAD
7fadc38140ae68fd517eb128ee293c626365b151
docs: Sep 7 handoff, beta.3 release report, standby poll spec

 docs/REPORT-handoff.md              | 127 +++++++++++++------------
 docs/REPORT-release-0.1.6-beta.3.md | 179 ++++++++++++++++++++++++++++++++++++
 docs/SPEC-standby-poll.md           | 113 +++++++++++++++++++++++
 3 files changed, 362 insertions(+), 57 deletions(-)
```

All three files existed; `docs/REPORT-handoff.md` was tracked-and-modified, the other two were
untracked. `docs/REPORT-handoff.md` was committed as-is, contents untouched.
`docs/REPORT-layout-tier-bc.md` was not added (already tracked since `53fc0c9`, as stated).
After this commit the only thing left in `git status` was this report file, untracked.

### Commit 2 — release

```
$ git diff --stat
 CHANGELOG.md                     | 69 ++++++++++++++++++++++++++++++++++++++++
 README.md                        |  2 +-
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 71 insertions(+), 2 deletions(-)

$ git show --stat --format="%H%n%s" HEAD
53a42c5c4f12736539ae021e1ff79c71ab4b3448
release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable

 CHANGELOG.md                     | 69 ++++++++++++++++++++++++++++++++++++++++
 README.md                        |  2 +-
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 71 insertions(+), 2 deletions(-)
```

The non-CHANGELOG hunks of that commit:

```
diff --git a/README.md b/README.md
index b738c69..063f70e 100644
--- a/README.md
+++ b/README.md
@@ -2,7 +2,7 @@
 
 > **Not affiliated with, endorsed by, or supported by Somnus Lab or Waveshare.**
 > An independent, community-built project. **Beta** — current release
-> `somnus-v0.1.5`.
+> `somnus-v0.1.6`.
 
 Turn a knob to change the temperature of your bed. This project turns a
 **Waveshare ESP32-S3 round touch-LCD knob** into a standalone bedside dial
diff --git a/firmware/dial-idf/CMakeLists.txt b/firmware/dial-idf/CMakeLists.txt
index 941710a..40cb606 100644
--- a/firmware/dial-idf/CMakeLists.txt
+++ b/firmware/dial-idf/CMakeLists.txt
@@ -19,5 +19,5 @@ add_compile_options("-Wno-format")
 # downgrade (is_newer() has no concept of a renumbering, only "older").
 # 1.0.0 is reserved for when docs/V1-scope.md is actually complete -- see
 # CHANGELOG.md's Somnus section header.
-set(PROJECT_VER "0.1.6-beta.3")
+set(PROJECT_VER "0.1.6")
 project(somnus-dial)
```

### State after both commits

```
$ git log --oneline -3
53a42c5 release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable
7fadc38 docs: Sep 7 handoff, beta.3 release report, standby poll spec
c47fdc2 docs: audit absolute-face F display against pad API and Somnus app

$ git log somnus/main..HEAD --oneline
53a42c5 release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable
7fadc38 docs: Sep 7 handoff, beta.3 release report, standby poll spec

$ git --no-optional-locks status --short
?? docs/REPORT-0.1.6-graduation.md

$ git tag --list 'somnus-v*' --sort=-v:refname | head -1
somnus-v0.1.6-beta.3
```

Both commit messages carry the standard `Co-Authored-By` / `Claude-Session` trailers.

## 5. CHANGELOG entry, exactly as written

Inserted at line 24 of `CHANGELOG.md`, directly above `## 0.1.6-beta.3 — 2026-09-06 (beta)`.
Heading shape copied from `## 0.1.5 — 2026-09-05` (no "(beta)" suffix; em-dash; ISO date;
intro paragraph; `### Added` / `### Fixed` / `### Internal` sections). Every bullet is a
verbatim copy from the beta.1, beta.2 and beta.3 entries in this file; the two `### Added`
bullets are beta.2's, the `### Fixed` list is beta.1's seven, then beta.2's one, then beta.3's
four, in release order; `### Internal` is beta.1's one. Nothing invented, nothing dropped, no
wording changed. The three beta entries remain in the file below it, unchanged.

Verified against the release workflow's own extractor (`.github/workflows/release.yml`, step
"Extract release notes from CHANGELOG.md") by running its `awk` locally with `ver=0.1.6`: it
selects this section and stops at the `## 0.1.6-beta.3` heading — 67 body lines, does not
bleed into the beta entry, and `0.1.6` does not match `0.1.6-beta.N` (the `index(... "## " ver
" ")` test requires the space).

The entry (lines 24–91 of `CHANGELOG.md`):

````markdown
## 0.1.6 — 2026-09-07

The first stable release since `0.1.5`. Everything below shipped incrementally
on the beta channel (`0.1.6-beta.1` through `-beta.3`); this collects it for
anyone upgrading straight from stable.

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
- The Night mode, Night face and brightness pickers no longer get kicked
  back to the dial face by a routine background refresh while you're on
  them.
- **Screen layout audit, Tier B.** The Update row's "tap to install" text no
  longer overflows the row with a long version number; network picker rows
  are sized to the row width and ellipsize long network names instead of
  running off both ends; the wrong-password message is shortened so it
  fits on one line whole; the Adjust mode, Night mode and Wi-Fi confirm
  screens have their vertical spacing corrected (the Back pill no longer
  touches the bezel, the Night mode note clears the row under it, the
  Wi-Fi confirm text clears the Continue button); and the standby clock
  block is recentred on the screen.
- **Screen layout audit, Tier C.** The Update row is rebuilt as a flex block
  (the same approach the About screen rows use) so its lines centre in the
  row in every state. The Connecting/error screen's offsets are fixed so
  multi-line degraded text no longer overlaps the headline, and its colours
  now come from the palette — the background matches the chassis colour
  instead of pure black, so booting no longer flashes from black into the
  dial face, and the text follows the night palette.
- Pad discovery no longer floods the serial log with per-host connection
  errors during a subnet scan.
- Stale comments corrected (screen timeout choices, auto-update window,
  simulator update scenario). No behaviour change.

### Internal

- Simulator scenarios for every screen state the layout audit measured,
  including knob- and drag-driven ones; 47 reference screenshots
  regenerated.
````

## 6. README lines, before and after

**Line 1 of 2 — current-release line (edited).** The instructions said this line named
`somnus-v0.1.4`; on disk it already named `somnus-v0.1.5` (a previous docs-sync had moved it
one release forward, but it was still one release behind). It was updated to `somnus-v0.1.6`
as the instruction's intent clearly requires. Recorded under Deviations 1.

Before (README.md lines 4–5):
```
> An independent, community-built project. **Beta** — current release
> `somnus-v0.1.5`.
```
After:
```
> An independent, community-built project. **Beta** — current release
> `somnus-v0.1.6`.
```

**Line 2 of 2 — night-window "until 0.1.5 ships" line: already correct, NOT edited.** There is
no line in `README.md` that reads the night window as pending. The only night-mode release
note is already past tense and names 0.1.5 as shipped (README.md lines 101–105, unchanged):

```
- **Day and night** — Settings → Night mode picks when the dial switches to
  its warm palette, dimmer backlight and softer haptics: 9 pm – 7 am, 10 pm
  – 6 am, or Off. Plus a **standby clock face** when idle, separate
  brightness for day, night and the clock, and a screen timeout. *(Night
  mode arrived in 0.1.5-beta.1 and shipped on the stable channel in 0.1.5.)*
```

Searched with `grep -n 'somnus-v0\.1\.\|0\.1\.5\|0\.1\.4\|[Nn]ight' README.md`; every hit was
inspected. No "until … ships" / pending wording exists. No edit invented.

## 7. Deviations

1. **README release line read `somnus-v0.1.5`, not `somnus-v0.1.4` as the instructions
   stated.** Edited to `somnus-v0.1.6` anyway, since "the line naming the current release" was
   unambiguous and the target value was given. Flagged so the owner knows the premise was off
   by one.
2. **README night-window line not edited** because it already reads as shipped (§6). This is
   the instruction's own "if already correct, say so" branch, not a departure — listed here so
   the two-line edit count in the task (2) versus the actual (1) is explained.
3. **A few read-only commands beyond the six prescribed** were run to do the work: `sed -n` /
   `grep -n` on `CHANGELOG.md`, `README.md` and `release.yml` to copy the heading shape and find
   the README lines; a local run of `release.yml`'s `awk` extractor against the new entry;
   `git diff --cached --stat`, `git show --stat`, `git rev-parse`, `git log`. All
   non-mutating.
4. **The previous run's `docs/REPORT-0.1.6-graduation.md` (BLOCKED report) was overwritten**
   by this file, as instructed. It is not committed (reports are never committed, per the
   project's convention; the two named report/spec files in Step 1 were an explicit exception).

Nothing else: the two commits contain exactly the files specified (3 + 3), no tag was
created, nothing was pushed, `REPORT-handoff.md` went in byte-for-byte as found, no build.

## 8. What could not be verified without hardware

- **`App version: 0.1.6` on boot.** Not built, not flashed. The version string is a one-token
  CMake change identical in form to every previous bump, but it has not been seen on a device
  or in a build log this run.
- **The beta.N → stable graduation OTA path.** A dial on `0.1.6-beta.3` with Beta builds on
  should, once `somnus-v0.1.6` is tagged and CI publishes it, see `0.1.6` as newer than
  `0.1.6-beta.3` and offer it. Per the Sep 5 handoff this path has never been observed on
  hardware; it can only be exercised after the owner tags and pushes.
- **Stable-channel discovery.** A dial on `0.1.5` with Beta builds off should find `0.1.6` via
  `/releases/latest`. Same dependency on the tag.
- **CI itself** — the "Verify tag matches PROJECT_VER" step (`0.1.6` == `somnus-v0.1.6` minus
  prefix) and the release-notes extraction. Both were checked locally by inspection and by
  running the extractor's `awk`, but the real run only happens on push.
- **The beta.3 overnight soak verdict (2026-09-06)** is still outstanding in the handoff. It is
  an input to whether tagging now is right; nothing in this run settles it.


## Tag and publish

**PUBLISHED — `somnus-v0.1.6` is live as the stable release.** Tag `somnus-v0.1.6` on
`53a42c5` pushed to `somnus`, `main` pushed to `somnus`, release CI run `34160552471` succeeded
(build-and-release 5m4s, deploy-pages 7s), release `somnus-v0.1.6` on
`matthewclaude/somnus-dial-releases` is `isPrerelease: false`, `isDraft: false`, both assets
attached, `/releases/latest` returns `somnus-v0.1.6`, and the published body is the CHANGELOG
`0.1.6` section byte-for-byte plus the workflow's fixed footer. One thing to know up front: the
four verification commands as written in the task point at `bedknob-for-somnus`, and there they
return "release not found" / HTTP 404 by design — the workflow publishes the Release object into
the public `somnus-dial-releases` repo (see Deviations 1).

### 1. Verdict

**PASS.** Tagged, pushed, built, published, verified.

### 2. Gate check — raw output

```
$ git --no-optional-locks status --short
?? docs/REPORT-0.1.6-graduation.md

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.6")

$ git log --oneline -3
53a42c5 release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable
7fadc38 docs: Sep 7 handoff, beta.3 release report, standby poll spec
c47fdc2 docs: audit absolute-face F display against pad API and Somnus app

$ git tag --list 'somnus-v*' --sort=-v:refname | head -3
somnus-v0.1.6-beta.3
somnus-v0.1.6-beta.2
somnus-v0.1.6-beta.1
```

Supplementary (read-only) checks run alongside the gate:

```
$ git rev-parse HEAD
53a42c5c4f12736539ae021e1ff79c71ab4b3448
$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
$ git tag --list somnus-v0.1.6
(no output — tag did not exist)
```

| # | Condition | Actual | Result |
|---|-----------|--------|--------|
| 1 | `PROJECT_VER` reads `0.1.6` | `22:set(PROJECT_VER "0.1.6")` | PASS |
| 2 | HEAD is the release commit `53a42c5` | `53a42c5c4f12736539ae021e1ff79c71ab4b3448` | PASS |
| 3 | no modified tracked files | only `?? docs/REPORT-0.1.6-graduation.md` (untracked, expected) | PASS |
| 4 | `somnus-v0.1.6` does not already exist as a tag | not in local tag list; newest was `somnus-v0.1.6-beta.3` | PASS |

### 3. Raw CI output

Tag and push:

```
$ git tag -a somnus-v0.1.6 -m "somnus-v0.1.6"
[exit 0]
$ git show -s --format="%H %d" somnus-v0.1.6^{commit}
53a42c5c4f12736539ae021e1ff79c71ab4b3448  (HEAD -> main, tag: somnus-v0.1.6)

$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   c47fdc2..53a42c5  main -> main
[exit 0]

$ git push somnus somnus-v0.1.6
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v0.1.6 -> somnus-v0.1.6
[exit 0]

$ git ls-remote somnus refs/heads/main refs/tags/somnus-v0.1.6*
53a42c5c4f12736539ae021e1ff79c71ab4b3448	refs/heads/main
ff0b7f996961d0aceefd605b99f0525e33f8b7ae	refs/tags/somnus-v0.1.6
53a42c5c4f12736539ae021e1ff79c71ab4b3448	refs/tags/somnus-v0.1.6^{}
1ec4f42c02b3c374a7ab6ab23e36436690ec2872	refs/tags/somnus-v0.1.6-beta.1
46f65b29a376da120221dc3497296fa6a18e0644	refs/tags/somnus-v0.1.6-beta.1^{}
6050e8bf7d7b83bf60fc8e7b7c3fcd34ebb7e1e8	refs/tags/somnus-v0.1.6-beta.2
ee420666ae374dcfbc08da914fbd43a3aa703bc8	refs/tags/somnus-v0.1.6-beta.2^{}
48cefaec25a6a13f09e6e781928de63ee85e5b81	refs/tags/somnus-v0.1.6-beta.3
53fc0c9dad016dac148a9887d91773b2a835c51c	refs/tags/somnus-v0.1.6-beta.3^{}
```

Run list, ~10 s after the push (the tag DID trigger a run — no delete/re-push needed):

```
$ gh run list --repo matthewclaude/bedknob-for-somnus --limit 3
in_progress		release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable	release	somnus-v0.1.6	push	34160552471	10s	2026-09-07T20:44:35Z
in_progress		release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable	ci	main	push	34160551330	11s	2026-09-07T20:44:34Z
completed	success	docs: audit absolute-face F display against pad API and Somnus app	ci	main	push	34149206605	4m57s	2026-09-07T17:50:03Z
```

Watch to completion (final screen of `gh run watch 34160552471 --repo matthewclaude/bedknob-for-somnus --exit-status --interval 20`):

```
✓ somnus-v0.1.6 release · 34160552471
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m4s (ID 101861348511)
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
✓ deploy-pages in 7s (ID 101862276676)
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

! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/download-artifact@v4. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
deploy-pages: .github#2
```

(The Node 20 deprecation annotations are the known, pre-existing CI noise carried in the
handoff's open list; identical to the beta.3 run.)

```
$ gh run view 34160552471 --repo matthewclaude/bedknob-for-somnus

✓ somnus-v0.1.6 release · 34160552471
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m4s (ID 101861348511)
✓ deploy-pages in 7s (ID 101862276676)

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/download-artifact@v4. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
deploy-pages: .github#2


ARTIFACTS
somnus-dial-merged

For more information about a job, try: gh run view --job=<job-id>
View this run on GitHub: https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34160552471
```

The `ci` workflow on `main` for the same push also passed:

```
$ gh run view 34160551330 --repo matthewclaude/bedknob-for-somnus

✓ main ci · 34160551330
Triggered via push about 5 minutes ago

JOBS
✓ build in 5m3s (ID 101861345115)

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build: .github#2


For more information about the job, try: gh run view --job=101861345115
View this run on GitHub: https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34160551330
```

Publish step log tail (`gh run view 34160552471 --repo matthewclaude/bedknob-for-somnus --job 101861348511 --log`, "Publish GitHub Release" lines):

```
  files: firmware/dial-idf/build/somnus-dial.bin
firmware/dial-idf/build/somnus-dial-merged.bin
  overwrite_files: true
👩‍🏭 Creating new GitHub release for tag somnus-v0.1.6...
Release 384311376 is not yet discoverable by tag somnus-v0.1.6, retrying... (2 retries remaining)
⬆️ Uploading somnus-dial.bin...
⬆️ Uploading somnus-dial-merged.bin...
✅ Uploaded somnus-dial.bin
✅ Uploaded somnus-dial-merged.bin
Finalizing release...
Getting assets list...
🎉 Release ready at https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.6
```

### 4. Commit SHA and tag as pushed

| | Value |
|---|---|
| Release commit | `53a42c5c4f12736539ae021e1ff79c71ab4b3448` (`release: somnus-v0.1.6 — graduate the 0.1.6 beta line to stable`) |
| Tag | `somnus-v0.1.6`, annotated, message `somnus-v0.1.6`, tag object `ff0b7f996961d0aceefd605b99f0525e33f8b7ae`, points at `53a42c5` |
| Pushed to | remote `somnus` = `git@github.com:matthewclaude/bedknob-for-somnus.git`, both `main` (`c47fdc2..53a42c5`) and the tag |
| Not pushed to | `origin` (chris023 upstream, push URL `no_push`) — not attempted |
| Release CI run | `34160552471` — https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34160552471 |
| Release URL | https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.6 |

### 5. Verification values as actually returned

**As written in the task (against `bedknob-for-somnus`) — both fail by design, see Deviations 1:**

```
$ gh release view somnus-v0.1.6 --repo matthewclaude/bedknob-for-somnus --json isPrerelease,isDraft,assets,tagName
release not found

$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq .tag_name
{"message":"Not Found","documentation_url":"https://docs.github.com/rest/releases/releases#get-the-latest-release","status":"404"}gh: Not Found (HTTP 404)
```

**Against the repo the workflow actually publishes to (`matthewclaude/somnus-dial-releases`, per
`release.yml` lines 10–14 and the `repository:` input on its Publish step, and the same repo the
beta.3 release report verified against):**

```
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362458","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:a353d2cbfc111f4b6a7820ad7c0c4f613b9a9249021c312c30c53452fa06a3d9","downloadCount":0,"id":"RA_kwDOULeAcc4gvpsa","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362457","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:89449a1e66999d43a73ecd95efec158d2853fec85cd2aeb7bf36f0908a523666","downloadCount":0,"id":"RA_kwDOULeAcc4gvpsZ","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"tagName":"somnus-v0.1.6"}

somnus-v0.1.6
```

| Expected | Actual | Result |
|---|---|---|
| CI success | run `34160552471` `✓`, both jobs success | PASS |
| `isPrerelease` false | `false` | PASS |
| `isDraft` false | `false` | PASS |
| both assets attached | `somnus-dial.bin` (1,612,080 B, sha256 `89449a1e…523666`) and `somnus-dial-merged.bin` (1,743,152 B, sha256 `a353d2cb…06a3d9`), both `state: uploaded` | PASS |
| `/releases/latest` → `somnus-v0.1.6` | `somnus-v0.1.6` | PASS |

Bonus check, not asked for — the Pages path the browser flasher uses:

```
$ curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin | head -8
HTTP/2 200 
server: GitHub.com
content-type: application/octet-stream
last-modified: Mon, 07 Sep 2026 20:50:08 GMT
access-control-allow-origin: *
strict-transport-security: max-age=31556952
etag: "6a9f2380-1a9930"
expires: Mon, 07 Sep 2026 21:02:29 GMT
```

**Published release body vs. the CHANGELOG `0.1.6` section.** The CHANGELOG section was
extracted locally with the workflow's own `awk` + trim pipeline (66 lines) and diffed against
`gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json body --jq .body`
(71 lines). The only difference is the five trailing lines the workflow appends to every release
by design (`release.yml` lines 79–87): a blank line, the "Dials already running…" sentence, the
browser-flasher sentence, a blank line, and the Required Notice. The 66 CHANGELOG lines are
identical, byte-for-byte, no re-wrapping, no dropped bullets, no bleed into the
`0.1.6-beta.3` section:

```
$ diff <(changelog 0.1.6 section) <(published body)
66a67,71
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
```

The published body in full, exactly as returned:

````markdown
The first stable release since `0.1.5`. Everything below shipped incrementally
on the beta channel (`0.1.6-beta.1` through `-beta.3`); this collects it for
anyone upgrading straight from stable.

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
- The Night mode, Night face and brightness pickers no longer get kicked
  back to the dial face by a routine background refresh while you're on
  them.
- **Screen layout audit, Tier B.** The Update row's "tap to install" text no
  longer overflows the row with a long version number; network picker rows
  are sized to the row width and ellipsize long network names instead of
  running off both ends; the wrong-password message is shortened so it
  fits on one line whole; the Adjust mode, Night mode and Wi-Fi confirm
  screens have their vertical spacing corrected (the Back pill no longer
  touches the bezel, the Night mode note clears the row under it, the
  Wi-Fi confirm text clears the Continue button); and the standby clock
  block is recentred on the screen.
- **Screen layout audit, Tier C.** The Update row is rebuilt as a flex block
  (the same approach the About screen rows use) so its lines centre in the
  row in every state. The Connecting/error screen's offsets are fixed so
  multi-line degraded text no longer overlaps the headline, and its colours
  now come from the palette — the background matches the chassis colour
  instead of pure black, so booting no longer flashes from black into the
  dial face, and the text follows the night palette.
- Pad discovery no longer floods the serial log with per-host connection
  errors during a subnet scan.
- Stale comments corrected (screen timeout choices, auto-update window,
  simulator update scenario). No behaviour change.

### Internal

- Simulator scenarios for every screen state the layout audit measured,
  including knob- and drag-driven ones; 47 reference screenshots
  regenerated.

Dials already running the firmware pick this up on their own — see Menu → Update.
For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.

Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
````

### 6. Deviations

1. **Verification ran against `matthewclaude/somnus-dial-releases`, not `bedknob-for-somnus`, for
   `gh release view` and `releases/latest`.** The task's commands name `bedknob-for-somnus`; run
   as written they return "release not found" and HTTP 404 (reproduced in §5). That is not a
   failure of the release: `release.yml` publishes the Release object and the gh-pages site into
   the public `somnus-dial-releases` repo (header comment lines 10–19, `repository:` on the
   Publish step), because the dial's OTA client and the web flasher are unauthenticated and
   cannot see a private repo. The beta.3 release report verified against the same public repo.
   The `gh run list` / `gh run view` commands were run against `bedknob-for-somnus` as written
   (the workflow runs there). The task's `--repo` for the two release queries should be
   updated for next time.
2. **Extra read-only commands** beyond those listed: `git rev-parse`, `git remote -v`,
   `git tag --list somnus-v0.1.6`, `git show -s`, `git ls-remote` (to confirm the pushed refs),
   `gh run list --json`, `gh run watch` (to wait on the run rather than poll `gh run view`),
   `gh run view --job --log` (Publish step), `gh release view --json body`, the local `awk`
   extraction and `diff`, `sed`/`grep` on `release.yml` and the beta.3 report, and one `curl -sI`
   on the Pages URL. None mutate anything.
3. Nothing else. Exactly one tag created, exactly two pushes, both to `somnus`. `origin` was not
   touched. No file in the repo changed except this report (still untracked, not committed).

### 7. What could not be verified without hardware

- **A dial actually installing 0.1.6.** Everything above is the server side. Not observed:
  a dial on `0.1.6-beta.3` (Beta builds on) being offered `0.1.6` and treating it as newer than
  its own prerelease build — the beta.N → stable graduation path, which the handoff notes has
  never been exercised on hardware; a dial on `0.1.5` (Beta builds off) picking `0.1.6` from
  `/releases/latest`; and the rollback-cancel after boot.
- **`App version: 0.1.6` on the About screen / serial boot line** of the CI image. The build
  passed the `Verify tag matches PROJECT_VER` step, so the string is in the binary, but nobody
  has booted it.
- **Whether the two asset sizes match the beta.3 build to the byte** (1,612,080 / 1,743,152 —
  identical to beta.3's) is consistent with the release commit changing only the version string,
  but the images were not downloaded or compared locally, and the sha256 digests differ as they
  must.
- **The beta.3 overnight soak verdict (2026-09-06)** remains unreported in the handoff. Stable is
  now out regardless; if that soak surfaces a problem, the fix is a `0.1.7` line, not a re-tag.
