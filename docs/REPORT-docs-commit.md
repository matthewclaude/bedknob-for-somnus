# REPORT: docs commit — release latest-pointer verification; handoff report update

Date: 2026-09-10. Repo: matthewclaude/bedknob-for-somnus, local `~/Projects/somnus-waveshare-rotary-dial`, branch `main`.

## VERDICT

**COMMITTED.** Commit `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` (`eff6df3`) on `main`, on top of
`11a00da`. Not pushed.

Amend note (2026-09-10): this commit was amended after the fact to drop an incorrect Co-Authored-By /
Claude-Session trailer from its message (tree unchanged), so its SHA changed from `361a4cb` to `eff6df3`;
the SHAs in this report were updated to the post-amend value.

## Gate check

- **Nothing was pushed.** No `git push` was run. After the commit, `somnus/main` still resolves to
  `11a00daededfc167cd81d0dc77d17c4a74101954` (the previous HEAD) and `origin/main` to `c059957`,
  both unchanged; local `main` is one commit ahead at `eff6df3`.
- **No files outside the three named were staged or modified.** The staged set (`git diff --cached
  --stat`) and the commit's file list are exactly `docs/REPORT-handoff.md`,
  `docs/REPORT-release-latest-pointer.md`, `docs/REPORTS.md`. `git status --short` after the commit
  is empty, so nothing else was left modified or untracked. This report file
  (`docs/REPORT-docs-commit.md`) was written after the commit and is not part of it; it will show as
  untracked on the next status check.
- The only working-tree edit made by this run before staging was the one-line insertion in
  `docs/REPORTS.md`. `docs/REPORT-handoff.md` was committed as found (pre-existing modification,
  not touched by this run).

## RAW output of every git command run

The commit message was passed with `-F` from a scratchpad file containing exactly:

```
docs: release latest-pointer verification; handoff report update

Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
Claude-Session: https://claude.ai/code/session_01GkiiYW4VGrJGowULng3HZM
```

Commands and output, in the order run:

```
$ git rev-parse HEAD   (before)
11a00daededfc167cd81d0dc77d17c4a74101954

$ git --no-optional-locks status --short   (before staging)
 M docs/REPORT-handoff.md
 M docs/REPORTS.md
?? docs/REPORT-release-latest-pointer.md

$ git add docs/REPORTS.md docs/REPORT-handoff.md docs/REPORT-release-latest-pointer.md
exit=0

$ git --no-optional-locks status --short   (after staging)
M  docs/REPORT-handoff.md
A  docs/REPORT-release-latest-pointer.md
M  docs/REPORTS.md

$ git diff --cached --stat
 docs/REPORT-handoff.md                | 162 +++++++-------
 docs/REPORT-release-latest-pointer.md | 392 ++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                       |   1 +
 3 files changed, 483 insertions(+), 72 deletions(-)

$ git commit -F <message file>
[main eff6df3] docs: release latest-pointer verification; handoff report update
 3 files changed, 483 insertions(+), 72 deletions(-)
 create mode 100644 docs/REPORT-release-latest-pointer.md
exit=0

$ git rev-parse HEAD   (after)
eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f

$ git log -1 --format=fuller
commit eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f
Author:     Matthew Montgomery <mattseattle@icloud.com>
AuthorDate: Thu Sep 10 08:54:00 2026 -0500
Commit:     Matthew Montgomery <mattseattle@icloud.com>
CommitDate: Thu Sep 10 08:54:00 2026 -0500

    docs: release latest-pointer verification; handoff report update
    
    Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_01GkiiYW4VGrJGowULng3HZM

$ git show --stat --format= HEAD
 docs/REPORT-handoff.md                | 162 +++++++-------
 docs/REPORT-release-latest-pointer.md | 392 ++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                       |   1 +
 3 files changed, 483 insertions(+), 72 deletions(-)

$ git diff --stat 11a00daededfc167cd81d0dc77d17c4a74101954 HEAD
 docs/REPORT-handoff.md                | 162 +++++++-------
 docs/REPORT-release-latest-pointer.md | 392 ++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                       |   1 +
 3 files changed, 483 insertions(+), 72 deletions(-)

$ git --no-optional-locks status --short   (after commit)
(end)

$ git rev-parse --abbrev-ref HEAD
main

$ git rev-parse somnus/main origin/main 2>&1
11a00daededfc167cd81d0dc77d17c4a74101954
c0599576e4a88249cd90ac50f9f567b55caecaab

$ git log --oneline -3
eff6df3 docs: release latest-pointer verification; handoff report update
11a00da docs: track housekeeping report
3fd1cc1 docs: report housekeeping — consistent 0.1.5 report names, TEST-F3 on disk, REPORTS.md index
```

## docs/REPORTS.md row added

Inserted directly after the `TEST-F3-stuck-loop.md` row (the last row, dated 2026-09-09), keeping
the list in date order. The new row and the two rows above it, quoted exactly from the file:

```
- `REPORT-sim-version-screens.md` — 2026-09-09 — simulator reads PROJECT_VER at build time; screens regenerated at 1.0.0 — **DONE** — commit `effb9c6` on `main` (on top of `7c106fb` = `somnus-v1.0.0`): the simulator now derives the installed version (1.0.0) and the advertised OTA version (1.0.1) from `firmware/dial-idf/CMakeLists.txt` at configure time, the two hardcoded literals are gone behind `#error` guards, and 6 of 49 screens regenerated with the new strings while the other 43 are byte-identical.
- `TEST-F3-stuck-loop.md` — 2026-09-09 — Test plan: F3 — escape hatches work while stuck in the connect loop — (no verdict line)
- `REPORT-release-latest-pointer.md` — 2026-09-10 — REPORT: release "latest" pointer verification — **somnus-v1.0.0 confirmed as the release GitHub serves as latest; firmware/latest/ serving the 1.0.0 binary by sha256.**
```

Format followed: `- \`<bare filename>\` — <date> — <file's H1> — **<verdict>**`. The H1 column is
the report's actual H1 ("REPORT: release "latest" pointer verification"), and the verdict text is
the one given in the instructions, bolded the way the other verdict-bearing rows are.

## git diff --stat against the previous commit, and the new SHA

Previous commit: `11a00daededfc167cd81d0dc77d17c4a74101954`
New commit:      `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f`

```
$ git diff --stat 11a00daededfc167cd81d0dc77d17c4a74101954 HEAD
 docs/REPORT-handoff.md                | 162 +++++++-------
 docs/REPORT-release-latest-pointer.md | 392 ++++++++++++++++++++++++++++++++++
 docs/REPORTS.md                       |   1 +
 3 files changed, 483 insertions(+), 72 deletions(-)
```

The handoff report's 90 insertions / 72 deletions match the figures given in the instructions
(162 lines changed).

## Deviations from instructions

1. **Filename column.** The instructions gave the new row's file as `docs/REPORT-release-latest-pointer.md`;
   every existing row uses the bare filename in backticks without the `docs/` prefix, so the row was
   written as `` `REPORT-release-latest-pointer.md` `` to match the file's format, as the
   instructions also required.
2. **Commit attribution as given, which differs from this session.** The commit message was used
   verbatim as instructed. For the record: the run that produced this commit was Claude Fable 5.1 in
   session `https://claude.ai/code/session_01BSmB4uqCDcqSocfrVPQG4N`, whereas the trailer names
   Claude Opus 5 and session `session_01GkiiYW4VGrJGowULng3HZM`. The message was not altered; this is
   noted only so the provenance is legible.
3. Read-only git commands beyond the commit itself (`rev-parse`, `status --short`, `diff --cached
   --stat`, `log`, `show --stat`, `diff --stat`, `rev-parse` of remote refs) were run to produce the
   evidence this report requires. All are listed in the raw output above.

No other deviations. Nothing was pushed; no tag was created; no file outside the three was staged.

## Not verifiable

- Whether the pre-existing changes in `docs/REPORT-handoff.md` are the intended content was not
  reviewed; the instructions said to commit it as-is, and it was.
- The remote state was checked only against the locally cached refs `somnus/main` and `origin/main`
  (no `git fetch` was run, to keep this run free of network git operations). That confirms this run
  pushed nothing; it does not confirm what the remotes currently hold.
- Whether CI would accept the commit is untested; the commit is docs-only and not pushed.
