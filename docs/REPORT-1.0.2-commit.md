# REPORT — somnus-v1.0.2 stable release, block 1 of 2 (commit only)

**DONE** — one commit on `main`, three files, nothing tagged, nothing pushed.

Date: 2026-09-17. Commit `5512537f7b34c4d8919471905709b86cf808d18c`.

## Gate (run before any edit)

All four passed.

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2-beta.1")

$ git --no-optional-locks tag -l 'somnus-v1.0.2'
(empty)

$ git --no-optional-locks status --short --untracked-files=no
(empty)

$ git --no-optional-locks log --oneline somnus/main..HEAD | wc -l
       0
```

Context at gate time: `HEAD` = `ff42f6d`; `git tag -l 'somnus-v1.0.2*'` returned
only `somnus-v1.0.2-beta.1`.

## Commit

```
$ git rev-parse HEAD
5512537f7b34c4d8919471905709b86cf808d18c

$ git diff --stat HEAD~1..HEAD
 CHANGELOG.md                     | 12 ++++++++++++
 README.md                        |  2 +-
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 14 insertions(+), 2 deletions(-)

$ git --no-optional-locks status --short --untracked-files=no
(empty)

$ git --no-optional-locks log -1 --format='%H%n%s%n%b'
5512537f7b34c4d8919471905709b86cf808d18c
release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY poll cadence 300 s goes stable
Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>

$ git --no-optional-locks log --oneline somnus/main..HEAD
5512537 release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY poll cadence 300 s goes stable

$ git --no-optional-locks tag -l 'somnus-v1.0.2*'
somnus-v1.0.2-beta.1
```

The CMakeLists.txt hunk is the single line 21 (`git diff -U0` on that file
shows exactly one `-`/`+` pair). The README hunk is the single line 5.

## What is on disk now

`firmware/dial-idf/CMakeLists.txt` line 21:

```
set(PROJECT_VER "1.0.2")
```

`README.md` line 5:

```
> `somnus-v1.0.2`.
```

New `CHANGELOG.md` section, lines 25–35, inserted directly above
`## 1.0.2-beta.1 — 2026-09-13 (beta)`. This is what the release workflow
will publish as the Release body:

```
## 1.0.2 — 2026-09-17

While the screen is off and the dial has gone to standby, it now asks the
pad for its state once every five minutes instead of every ten seconds.
Touching the knob still reads the pad immediately, so the face is current
the moment you look at it. Nothing about the screens or pad control
changes. This is `1.0.2-beta.1` graduated to a stable release after a
full night in use and an over-the-air install of its own.

Internal: the `1.0.2-beta.1` build renumbered; no code change beyond the
version string.
```

No existing CHANGELOG section was edited; the diff for CHANGELOG.md is
twelve added lines and zero removed.

## Deviations

1. **Commit message trailer.** The commit subject is the line given in the
   task, verbatim. The body carries one additional trailer line,
   `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`, which the
   session's attribution rule requires on every commit. It contains no path,
   address, MAC or credential. If the trailer is unwanted, amend before
   tagging (block 2 tags this SHA, so the amend must come first).
2. **Git invocation.** The first gate run failed before any git output with
   `You have not agreed to the Xcode license agreements` from `/usr/bin/git`
   (the Xcode shim; `xcode-select -p` points at Xcode.app). Every git command
   in this block was therefore run with `DEVELOPER_DIR=/Library/Developer/CommandLineTools`
   set in the shell environment, which routes the shim to the Command Line
   Tools git (Apple Git-157, 2.54.0). This is a per-process environment
   variable; no system setting was changed and no `sudo` was used. The four
   gate checks were re-run in full under that environment and are the ones
   pasted above. Block 2 will need the same variable, or the Xcode license
   accepted, before it can tag and push.

No other deviations.

## Not verified in this block

No build, no flash, no tag, no push and no CI run happened here. The dial
and the pad were not touched. That the release workflow accepts the new
CHANGELOG heading and that `PROJECT_VER` equals the tag are checked by CI
only once block 2 pushes the tag.

This file is deliberately left uncommitted; it and its `docs/REPORTS.md`
line go in the next docs commit.
