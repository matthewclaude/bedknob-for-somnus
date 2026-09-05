# REPORT — docs-sync commit (README → 0.1.5, Sep 3–5 reports, NAMING audit, ignore Claude outputs)

**Verdict: DONE.** Gate passed, docs-only commit `5228de3` created on `firmware/somnus-port` and pushed to the `somnus` remote. No code changed, no tag created. One deviation (a typo in the commit's `Claude-Session:` trailer), recorded in section 11.

Date: 2026-09-04. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch `firmware/somnus-port`. This report is not part of the commit.

## 1. Gate check (raw output)

```
$ grep PROJECT_VER firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "0.1.5")

$ git tag --points-at HEAD
somnus-v0.1.5

$ git branch --show-current
firmware/somnus-port
```

Both gate conditions hold: PROJECT_VER is 0.1.5 and HEAD carries `somnus-v0.1.5`. Proceeded.

## 2. Status before staging

```
$ git --no-optional-locks status --short
 M docs/SPEC-ota-readiness.md
?? "Claude outputs/"
?? docs/PLAN-screen-layout-fixes.md
?? docs/REPORT-0.1.5-ci-check.md
?? docs/REPORT-0.1.5-graduation-commit.md
?? docs/REPORT-0.1.5-tag-push.md
?? docs/REPORT-0.1.5-upgrade-path-verify.md
?? docs/REPORT-about-layout-battery-glyph.md
?? docs/REPORT-battery-pct-about-redesign.md
?? docs/REPORT-beta5-ci-check.md
?? docs/REPORT-beta5-commit.md
?? docs/REPORT-beta5-tag-push.md
?? docs/REPORT-handoff.md
?? docs/REPORT-night-face.md
?? docs/REPORT-ota-beta-not-found.md
?? docs/REPORT-power-indicator.md
?? docs/REPORT-release-beta3.md
?? docs/REPORT-release-beta4.md
?? docs/REPORT-screen-layout-audit.md
```

## 3. Pre-edit inspection (raw output)

```
$ grep -n "0\.1\.4" README.md
5:> `somnus-v0.1.4`.

$ grep -n "0\.1\.5" README.md
105:  mode is new in 0.1.5-beta.1; on the stable channel the window is fixed at
106:  9 pm – 7 am until 0.1.5 ships.)*

$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git (fetch)
somnus	git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git (push)

$ ls docs/TEST-*.md
(eval):1: no matches found: docs/TEST-*.md
```

Verification of the three NAMING.md audit claims before writing them:

```
$ git -C ~/Projects/SomnusWidget rev-parse --is-inside-work-tree
fatal: not a git repository (or any of the parent directories): .git
$ ls -d ~/Projects/SomnusWidget
/Users/matthew/Projects/SomnusWidget

$ git -C ~/Projects/SomnusDialPreview log --oneline -1 d762ba6
d762ba6 Rename to Bedknob for Mac, adopt the firmware palette, and stop guessing the pad's address
$ git -C ~/Projects/SomnusDialPreview branch -r --contains d762ba6
(no output — no remote branch contains it)

$ git ls-remote --symref somnus HEAD
ref: refs/heads/main	HEAD
4a32427b56c89e2f90b9c9b52f9779187566c9b6	HEAD
$ git ls-remote somnus refs/heads/main refs/heads/firmware/somnus-port
1f527bb5c4aedd0b3ad050bed52dd8897fc6df56	refs/heads/firmware/somnus-port
4a32427b56c89e2f90b9c9b52f9779187566c9b6	refs/heads/main
```

All three claims hold: GitHub default branch is `main` at `4a32427` (not the ported branch), `d762ba6` in the Mac repo (`~/Projects/SomnusDialPreview` on disk) is on no remote branch, and `~/Projects/SomnusWidget` exists but is not a git repo.

## 4. README before/after

Line 5, before:
```
> `somnus-v0.1.4`.
```
Line 5, after:
```
> `somnus-v0.1.5`.
```

Lines 104–106, before:
```
  brightness for day, night and the clock, and a screen timeout. *(Night
  mode is new in 0.1.5-beta.1; on the stable channel the window is fixed at
  9 pm – 7 am until 0.1.5 ships.)*
```
Lines 104–105, after:
```
  brightness for day, night and the clock, and a screen timeout. *(Night
  mode arrived in 0.1.5-beta.1 and shipped on the stable channel in 0.1.5.)*
```

Post-edit check:
```
$ grep -n "0\.1\.4" README.md
(none)
```

The "Relationship to Orion Dial" section and all attribution text were not touched (the README diff below shows only the two hunks above).

## 5. Working-tree diffs of the three edited files (raw)

```
diff --git a/README.md b/README.md
index b1e9939..b738c69 100644
--- a/README.md
+++ b/README.md
@@ -2,7 +2,7 @@
 
 > **Not affiliated with, endorsed by, or supported by Somnus Lab or Waveshare.**
 > An independent, community-built project. **Beta** — current release
-> `somnus-v0.1.4`.
+> `somnus-v0.1.5`.
 
 Turn a knob to change the temperature of your bed. This project turns a
 **Waveshare ESP32-S3 round touch-LCD knob** into a standalone bedside dial
@@ -102,8 +102,7 @@ the normal way to update.
   its warm palette, dimmer backlight and softer haptics: 9 pm – 7 am, 10 pm
   – 6 am, or Off. Plus a **standby clock face** when idle, separate
   brightness for day, night and the clock, and a screen timeout. *(Night
-  mode is new in 0.1.5-beta.1; on the stable channel the window is fixed at
-  9 pm – 7 am until 0.1.5 ships.)*
+  mode arrived in 0.1.5-beta.1 and shipped on the stable channel in 0.1.5.)*
 - **Over-the-air updates** from this project's GitHub Releases, with
   bootloader rollback if an image fails to boot, an optional beta channel,
   and optional automatic installs in a two-hour window after night ends
diff --git a/docs/NAMING.md b/docs/NAMING.md
index cf96514..a784b0e 100644
--- a/docs/NAMING.md
+++ b/docs/NAMING.md
@@ -28,7 +28,7 @@ construction is not worth losing.
 |---|---|---|
 | Somnus Dial / the dial | Bedknob for Somnus | **Copy surfaces done Sep 3** — ... |
 | SomnusDialPreview | Bedknob for Mac | **Done Sep 2** — ... |
-| SomnusWidget | Bedknob Mini | **Not started** |
+| SomnusWidget | Bedknob Mini | **Not started — not under git** |
 
 ## What is still outstanding
 
@@ -48,6 +48,12 @@ construction is not worth losing.
    all updated Sep 2 2026. Only the summary's *filename*
    (`somnus-dial-project-summary.md`) still carries the old name; fold that into the
    rename pass.
+5. **Sep 5 2026 audit of what is actually published.** The firmware repo's GitHub
+   default branch is still `main`, which points at the un-ported Orion code — that is
+   why the GitHub repo page shows the Orion README rather than this one; the ported
+   firmware lives on `firmware/somnus-port`. The Mac repo's rename commit `d762ba6`
+   ("Rename to Bedknob for Mac …") was never pushed, so GitHub still shows
+   SomnusDialPreview. `~/Projects/SomnusWidget` is not a git repository at all.
 
 ## Naming assets
 
diff --git a/.gitignore b/.gitignore
index 221ebc4..85dd6d7 100644
--- a/.gitignore
+++ b/.gitignore
@@ -47,3 +47,4 @@ reference/sleep_temps.csv
 # Local research/reference material — personal pad data, the vendor's API
 # spec, and scratch viewers. Not part of the firmware; never publish.
 reference/
+Claude outputs/
```

(The two long unchanged table rows in the NAMING.md hunk are elided with `...` here only; the on-disk diff is complete.)

## 6. Staging

```
$ git add README.md .gitignore docs/NAMING.md docs/SPEC-ota-readiness.md docs/REPORT-*.md docs/PLAN-*.md

$ git --no-optional-locks status --short
M  .gitignore
M  README.md
M  docs/NAMING.md
A  docs/PLAN-screen-layout-fixes.md
A  docs/REPORT-0.1.5-ci-check.md
A  docs/REPORT-0.1.5-graduation-commit.md
A  docs/REPORT-0.1.5-tag-push.md
A  docs/REPORT-0.1.5-upgrade-path-verify.md
A  docs/REPORT-about-layout-battery-glyph.md
A  docs/REPORT-battery-pct-about-redesign.md
A  docs/REPORT-beta5-ci-check.md
A  docs/REPORT-beta5-commit.md
A  docs/REPORT-beta5-tag-push.md
A  docs/REPORT-handoff.md
A  docs/REPORT-night-face.md
A  docs/REPORT-ota-beta-not-found.md
A  docs/REPORT-power-indicator.md
A  docs/REPORT-release-beta3.md
A  docs/REPORT-release-beta4.md
A  docs/REPORT-screen-layout-audit.md
M  docs/SPEC-ota-readiness.md
```

`Claude outputs/` no longer appears as untracked because the new `.gitignore` line covers it.

Docs-only rule check:
```
$ git diff --cached --name-only | grep -E '^(firmware/dial-idf/main|firmware/dial-idf/components|web-flasher|\.github)/'
(no matches) → clean: no forbidden paths staged
```

## 7. `git diff --stat` (cached, identical to the commit's stat)

```
 .gitignore                                |    1 +
 README.md                                 |    5 +-
 docs/NAMING.md                            |    8 +-
 docs/PLAN-screen-layout-fixes.md          |  112 ++++
 docs/REPORT-0.1.5-ci-check.md             |  138 ++++
 docs/REPORT-0.1.5-graduation-commit.md    |   96 +++
 docs/REPORT-0.1.5-tag-push.md             |   76 +++
 docs/REPORT-0.1.5-upgrade-path-verify.md  |  181 ++++++
 docs/REPORT-about-layout-battery-glyph.md |  115 ++++
 docs/REPORT-battery-pct-about-redesign.md | 1007 +++++++++++++++++++++++++++++
 docs/REPORT-beta5-ci-check.md             |  124 ++++
 docs/REPORT-beta5-commit.md               |   99 +++
 docs/REPORT-beta5-tag-push.md             |   92 +++
 docs/REPORT-handoff.md                    |   72 +++
 docs/REPORT-night-face.md                 |  719 ++++++++++++++++++++
 docs/REPORT-ota-beta-not-found.md         |  124 ++++
 docs/REPORT-power-indicator.md            |  261 ++++++++
 docs/REPORT-release-beta3.md              |  183 ++++++
 docs/REPORT-release-beta4.md              |  145 +++++
 docs/REPORT-screen-layout-audit.md        |  434 +++++++++++++
 docs/SPEC-ota-readiness.md                |   18 +
 21 files changed, 4006 insertions(+), 4 deletions(-)
```

## 8. Commit

Message passed via `git commit -q -F -` (subject exactly as specified, plus the two attribution trailers this tooling appends):

```
docs: sync README to 0.1.5, commit Sep 3-5 reports, NAMING audit, ignore Claude outputs

Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01V4hYxvndehLHYJkuAuyhT2W
```

```
$ git log --oneline -1
5228de3 docs: sync README to 0.1.5, commit Sep 3-5 reports, NAMING audit, ignore Claude outputs
$ git rev-parse HEAD
5228de326859603befc73abfe001c0cdcb69b9d4
```

**Commit SHA: `5228de326859603befc73abfe001c0cdcb69b9d4`** (`5228de3`), parent `1f527bb` (the 0.1.5 release commit).

## 9. Push

```
$ git push somnus firmware/somnus-port
To github.com:matthewclaude/somnus-waveshare-rotary-dial.git
   1f527bb..5228de3  firmware/somnus-port -> firmware/somnus-port

$ git ls-remote somnus refs/heads/firmware/somnus-port
5228de326859603befc73abfe001c0cdcb69b9d4	refs/heads/firmware/somnus-port
```

Pushed to `somnus` only. `origin` was not touched (its push URL is `no_push`).

## 10. Status after commit and push

```
$ git --no-optional-locks status --short
(empty — clean tree; this report file did not yet exist)

$ git tag --points-at HEAD
(empty — no tag created; somnus-v0.1.5 remains on the parent 1f527bb)
```

After this report is written, `git status --short` shows only `?? docs/REPORT-docs-sync.md`, which is intentionally left uncommitted.

## 11. Deviations from the spec

1. **Commit trailer typo.** The `Claude-Session:` trailer in the commit body reads `session_01V4hYxvndehLHYJkuAuyhT2W`; the correct session id is `session_01V4hYxvndehLHYJkuAyhT2W` (one stray `u`). The subject line, which the spec dictates, is exact. The trailer is an attribution line the tooling convention adds, not part of the spec's required message. It was noticed only after the push. Correcting it would need `commit --amend` plus a force-push to `somnus`, which the spec did not authorize, so the commit was left as-is. To fix it yourself: `git commit --amend` with the corrected trailer, then `git push --force-with-lease somnus firmware/somnus-port`.
2. **No `docs/TEST-*.md` files existed.** The spec said to stage every untracked `docs/TEST-*.md`; the glob matched nothing, so none were staged. Noted for completeness, not an error.

Everything else: none.

## 12. What could not be verified

- Nothing in the gate, staging, commit or push. All outputs above are raw and were checked.
- The three NAMING.md audit facts were re-verified from the live remote and local filesystems (section 3) rather than taken on faith. The statement that `main` on GitHub is "the un-ported Orion code" rests on `main` at `4a32427` being the branch the spec identified as such; I did not diff `4a32427` against the Orion upstream to prove it independently.
- The spec dates the audit "Sep 5" while today's system date is Sep 4 2026. The NAMING.md item uses the spec's date ("Sep 5 2026") as instructed. Not a deviation, but not reconciled either.
