# REPORT: amend the docs commit message; track the docs-commit report

Date: 2026-09-10. Repo: matthewclaude/bedknob-for-somnus, local `~/Projects/somnus-waveshare-rotary-dial`, branch `main`.

## VERDICT

**BOTH STEPS DONE.**
Step 1: `361a4cb` amended (message only, tree unchanged) to `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` (`eff6df3`).
Step 2: `docs/REPORT-docs-commit.md` + `docs/REPORTS.md` committed as `0b6c62f8cf7b084d4af1910bcfeea9d8ec5e5131` (`0b6c62f`).
Neither commit is pushed.

This report file (`docs/REPORT-docs-amend.md`) is deliberately left **untracked**. It is owed to the
next docs commit, the same way this repo has handled report files before (a report is committed in the
commit after the one it describes).

## Gate check

- **Nothing was pushed.** No `git push` was run. After both steps, `somnus/main` still resolves to
  `11a00daededfc167cd81d0dc77d17c4a74101954` and `origin/main` to `c0599576e4a88249cd90ac50f9f567b55caecaab`,
  both unchanged from before this run. Local `main` is two commits ahead of `somnus/main`.
- **No tag was created.** No `git tag` write was run; `git tag --points-at` on both new commits prints
  nothing.
- **No file outside the two named in step 2 was staged.** The staged set for the second commit was
  exactly `docs/REPORT-docs-commit.md` (added) and `docs/REPORTS.md` (modified); `git status --short`
  after the commit is empty apart from this report, which was written afterwards and not staged.
  The amend in step 1 used `--only` with no paths and touched no tree content (verified below).
- **Git commands run**, in order: `git rev-parse HEAD`, `git --no-optional-locks status --short`,
  `git commit --amend --only -m ...`, `git rev-parse HEAD`, `git log -1 --format=fuller`,
  `git diff <new> 361a4cb`, `git rev-parse HEAD^{tree} 361a4cb^{tree}`,
  `git --no-optional-locks status --short`, `git add docs/REPORT-docs-commit.md docs/REPORTS.md`,
  `git --no-optional-locks status --short`, `git diff --cached --stat`, `git commit -m ...`,
  `git rev-parse HEAD`, `git log -1 --format=fuller`, `git show --stat --format= HEAD`,
  `git --no-optional-locks status --short`, `git tag --points-at ...` (read-only listing),
  `git rev-parse somnus/main origin/main`, `git log --oneline -4`.
  **Plain `git status` was not run.** Every status call used `--no-optional-locks --short`.

## RAW output of every git command run

### Step 1 — amend

```
$ git rev-parse HEAD   (before amend)
361a4cbad38b1afbb518e374dcf1e24dfaefa7b9

$ git --no-optional-locks status --short   (before amend)
?? docs/REPORT-docs-commit.md
(end)

$ git commit --amend --only -m "docs: release latest-pointer verification; handoff report update"
[main eff6df3] docs: release latest-pointer verification; handoff report update
 Date: Thu Sep 10 08:54:00 2026 -0500
 3 files changed, 483 insertions(+), 72 deletions(-)
 create mode 100644 docs/REPORT-release-latest-pointer.md
exit=0

$ git rev-parse HEAD   (after amend)
eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f

$ git log -1 --format=fuller
commit eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f
Author:     Matthew Montgomery <mattseattle@icloud.com>
AuthorDate: Thu Sep 10 08:54:00 2026 -0500
Commit:     Matthew Montgomery <mattseattle@icloud.com>
CommitDate: Thu Sep 10 08:59:42 2026 -0500

    docs: release latest-pointer verification; handoff report update

$ git diff $(git rev-parse HEAD) 361a4cb   (must be empty)
exit=0 (lines of output: 0)

$ git rev-parse HEAD^{tree} 361a4cb^{tree}
703b10ccba2bae4ca1791a87b45789f252659d8e
703b10ccba2bae4ca1791a87b45789f252659d8e

```

### Step 2 — commit the loose report

```
$ git --no-optional-locks status --short   (after edits, before staging)
 M docs/REPORTS.md
?? docs/REPORT-docs-commit.md

$ git add docs/REPORT-docs-commit.md docs/REPORTS.md
exit=0

$ git --no-optional-locks status --short   (after staging)
A  docs/REPORT-docs-commit.md
M  docs/REPORTS.md

$ git diff --cached --stat
 docs/REPORT-docs-commit.md | 169 +++++++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md            |   1 +
 2 files changed, 170 insertions(+)

$ git commit -m "docs: track docs-commit report"
[main 0b6c62f] docs: track docs-commit report
 2 files changed, 170 insertions(+)
 create mode 100644 docs/REPORT-docs-commit.md
exit=0

$ git rev-parse HEAD
0b6c62f8cf7b084d4af1910bcfeea9d8ec5e5131

$ git log -1 --format=fuller
commit 0b6c62f8cf7b084d4af1910bcfeea9d8ec5e5131
Author:     Matthew Montgomery <mattseattle@icloud.com>
AuthorDate: Thu Sep 10 09:00:15 2026 -0500
Commit:     Matthew Montgomery <mattseattle@icloud.com>
CommitDate: Thu Sep 10 09:00:15 2026 -0500

    docs: track docs-commit report

$ git show --stat --format= HEAD
 docs/REPORT-docs-commit.md | 169 +++++++++++++++++++++++++++++++++++++++++++++
 docs/REPORTS.md            |   1 +
 2 files changed, 170 insertions(+)

$ git --no-optional-locks status --short   (after commit)
(end)

$ git tag --points-at HEAD~1 ; git tag --points-at HEAD
(end)

$ git rev-parse somnus/main origin/main
11a00daededfc167cd81d0dc77d17c4a74101954
c0599576e4a88249cd90ac50f9f567b55caecaab

$ git log --oneline -4
0b6c62f docs: track docs-commit report
eff6df3 docs: release latest-pointer verification; handoff report update
11a00da docs: track housekeeping report
3fd1cc1 docs: report housekeeping — consistent 0.1.5 report names, TEST-F3 on disk, REPORTS.md index
```

## Step 1 empty-tree verification (quoted)

```
$ git diff $(git rev-parse HEAD) 361a4cb   (must be empty)
exit=0 (lines of output: 0)

$ git rev-parse HEAD^{tree} 361a4cb^{tree}
703b10ccba2bae4ca1791a87b45789f252659d8e
703b10ccba2bae4ca1791a87b45789f252659d8e
```

`git diff eff6df3 361a4cb` produced no output, and both commits point at the same tree object
`703b10c`. The amend changed the message only. Step 2 therefore proceeded.

## SHA occurrences replaced in docs/REPORT-docs-commit.md

Eight occurrences on seven lines (line 7 carried both a full and a short form). Full forms stayed
full, short forms stayed short. Line numbers on the "after" side are shifted by +4 from line 14
onward because the amend note (step 2b) was inserted above them.

| before (line: text) | after (line: text) |
|---|---|
| 7: `` Commit `361a4cbad38b1afbb518e374dcf1e24dfaefa7b9` (`361a4cb`) on `main` `` | 7: `` Commit `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` (`eff6df3`) on `main` `` |
| 14: `` local `main` is one commit ahead at `361a4cb`. `` | 18: `` local `main` is one commit ahead at `eff6df3`. `` |
| 62: `[main 361a4cb] docs: release latest-pointer verification; handoff report update` | 66: `[main eff6df3] docs: release latest-pointer verification; handoff report update` |
| 68: `361a4cbad38b1afbb518e374dcf1e24dfaefa7b9` | 72: `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` |
| 71: `commit 361a4cbad38b1afbb518e374dcf1e24dfaefa7b9` | 75: `commit eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` |
| 105: `361a4cb docs: release latest-pointer verification; handoff report update` | 109: `eff6df3 docs: release latest-pointer verification; handoff report update` |
| 128: `` New commit:      `361a4cbad38b1afbb518e374dcf1e24dfaefa7b9` `` | 132: `` New commit:      `eff6df3ca9bea50f5c67efaf8e3f442c8b4a781f` `` |

After replacement the only remaining `361a4cb` in the file is the one inside the new amend note,
which names it intentionally as the pre-amend SHA. The note added under the VERDICT (step 2b):

```
Amend note (2026-09-10): this commit was amended after the fact to drop an incorrect Co-Authored-By /
Claude-Session trailer from its message (tree unchanged), so its SHA changed from `361a4cb` to `eff6df3`;
the SHAs in this report were updated to the post-amend value.
```

## docs/REPORTS.md row added

Inserted directly after the `REPORT-release-latest-pointer.md` row, which is the other 2026-09-10 row,
keeping date order. The new row and the two rows above it, quoted exactly:

```
- `TEST-F3-stuck-loop.md` — 2026-09-09 — Test plan: F3 — escape hatches work while stuck in the connect loop — (no verdict line)
- `REPORT-release-latest-pointer.md` — 2026-09-10 — REPORT: release "latest" pointer verification — **somnus-v1.0.0 confirmed as the release GitHub serves as latest; firmware/latest/ serving the 1.0.0 binary by sha256.**
- `REPORT-docs-commit.md` — 2026-09-10 — REPORT: docs commit — release latest-pointer verification; handoff report update — **COMMITTED.** Commit `eff6df3` on `main` (three docs files), not pushed; the message was later amended to drop an incorrect attribution trailer, tree unchanged.
```

## git log --oneline -4 (at the end)

```
0b6c62f docs: track docs-commit report
eff6df3 docs: release latest-pointer verification; handoff report update
11a00da docs: track housekeeping report
3fd1cc1 docs: report housekeeping — consistent 0.1.5 report names, TEST-F3 on disk, REPORTS.md index
```

## Deviations from instructions

1. **Quoted raw git output inside REPORT-docs-commit.md was rewritten too.** The instruction was to
   replace every occurrence of the old SHA, so the five occurrences that sit inside that report's
   "RAW output" block (the `[main 361a4cb]` commit line, the `rev-parse` result, the `log` header, the
   `log --oneline` line, and the diff-stat heading) now read `eff6df3`. Those lines are therefore no
   longer a literal transcript of what git printed at the time; the amend note in that report explains
   why. This is exactly what was asked, but it is flagged because it makes a quoted transcript
   non-literal.
2. The REPORTS.md row's verdict text goes slightly beyond the bare "**COMMITTED.**" H1 verdict, adding a
   short clause about the later amend, so a reader of the index is not surprised that the SHA in the
   row differs from the one the file originally recorded. Other rows in the file carry similar
   post-verdict clauses.
3. `git diff --cached --stat`, `git show --stat`, `git tag --points-at`, `git rev-parse` of the remote
   refs and `git log -1 --format=fuller` were run as read-only evidence commands; they were not named
   in the instructions but the report sections require what they show.

No other deviations. Not pushed, not tagged, nothing beyond the two named files staged, this report
left untracked.

## Not verifiable

- Remote state was checked only against the locally cached refs `somnus/main` and `origin/main`; no
  `git fetch` was run. That proves this run pushed nothing; it does not prove what the remotes hold now.
- The old commit object `361a4cb` still exists in the local object store (reachable from the reflog)
  until git garbage-collects it; it is not on any branch and was never pushed, so nothing external
  references it. Not verified beyond "no remote ref points at it".
- Whether the content of `docs/REPORT-handoff.md` (carried unchanged inside the amended commit) is what
  the owner intended was not reviewed; the amend did not touch it.
