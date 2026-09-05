# REPORT: 0.1.5 graduation commit

**Verdict: COMMITTED. `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56` on `firmware/somnus-port` bumps PROJECT_VER to `0.1.5`, adds the `## 0.1.5 — 2026-09-05` CHANGELOG section, and carries the pre-existing §9.9 SPEC addition. Exactly three files, no untracked files swept in. Not tagged, not pushed.**

Date: 2026-09-05 (UTC) / 2026-09-04 20:03 local
Supersedes the earlier gate-3 STOP report of the same name (that attempt edited nothing).

## Gate checks (pre-edit)

| # | Check | Required | Observed | Result |
|---|---|---|---|---|
| 1 | `PROJECT_VER` in `firmware/dial-idf/CMakeLists.txt` (perl extraction; macOS grep has no `-P`) | `0.1.5-beta.5` | `0.1.5-beta.5` | PASS |
| 2 | First `## ` heading in `CHANGELOG.md` | `## 0.1.5-beta.5 — 2026-09-04 (beta)` | `## 0.1.5-beta.5 — 2026-09-04 (beta)` | PASS |
| 3 | `git status --porcelain`, tracked lines only | only ` M docs/SPEC-ota-readiness.md` | only ` M docs/SPEC-ota-readiness.md`; 15 `??` untracked lines (reports/plan/"Claude outputs/"), ignored per convention | PASS |

Pre-edit check of the SPEC's existing modification: `git diff --stat` = 1 file, 59
insertions, 0 deletions; a single hunk at `@@ -1323,3 +1323,62 @@` whose first added
heading is `### 9.9 Stable channel never received the beta-discovery fix` and whose last
added line is `decision is made on purpose.` — i.e. only the §9.9 addition.

## Edits made

1. `firmware/dial-idf/CMakeLists.txt` line 22: `set(PROJECT_VER "0.1.5-beta.5")` ->
   `set(PROJECT_VER "0.1.5")`.
2. `CHANGELOG.md`: the supplied 0.1.5 section inserted verbatim above what was line 24
   (`## 0.1.5-beta.5 — 2026-09-04 (beta)`), directly after the intro prose. The new heading
   is now line 24; the beta.5 heading moved to line 63, separated by one blank line.
   Inserted block compared byte-for-byte against the supplied text: identical (39 lines
   including trailing blank).
3. `docs/SPEC-ota-readiness.md`: not touched by this task; included as-is.

## Post-edit verification

| # | Check | Observed | Result |
|---|---|---|---|
| 1 | `PROJECT_VER` now | `0.1.5` | PASS |
| 2 | Inserted CHANGELOG heading | line 24: `## 0.1.5 — 2026-09-05` (matches PROJECT_VER `0.1.5`; the workflow's extractor matches `## 0.1.5 ` and will not collide with `## 0.1.5-beta.5`) | PASS |
| 3 | `git diff --stat docs/SPEC-ota-readiness.md` after edits | still `59 insertions(+)`, 0 deletions — unchanged from pre-edit | PASS |

Tracked porcelain before staging:

```
 M CHANGELOG.md
 M docs/SPEC-ota-readiness.md
 M firmware/dial-idf/CMakeLists.txt
```

## Staging

```
git add firmware/dial-idf/CMakeLists.txt
git add CHANGELOG.md
git add docs/SPEC-ota-readiness.md
```

`git --no-optional-locks diff --cached --stat` before commit:

```
 CHANGELOG.md                     | 39 ++++++++++++++++++++++++++
 docs/SPEC-ota-readiness.md       | 59 ++++++++++++++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 99 insertions(+), 1 deletion(-)
```

`diff --cached --name-only` grep for `REPORT-|PLAN-|Claude outputs`: no matches. No
untracked file was staged.

## Commit

SHA: `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56`

`git --no-optional-locks show --stat HEAD`:

```
commit 1f527bb5c4aedd0b3ad050bed52dd8897fc6df56
Author: Matthew Montgomery <mattseattle@icloud.com>
Date:   Fri Sep 4 20:03:03 2026 -0500

    release: 0.1.5 — first stable release since 0.1.4, graduates beta.1-beta.5 (night mode/face, battery indicator+percentage, About redesign) and the OTA beta-discovery fix to the stable channel

 CHANGELOG.md                     | 39 ++++++++++++++++++++++++++
 docs/SPEC-ota-readiness.md       | 59 ++++++++++++++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 3 files changed, 99 insertions(+), 1 deletion(-)
```

Post-commit: no tracked changes remain. Untracked report/plan/scratch files remain
untracked, as intended.

## Not done (by instruction)

- No `git tag somnus-v0.1.5`.
- No `git push`.

Tag + push is a separate step. When it happens: lightweight tag on `1f527bb`, pushed by
name to remote `somnus` only, same as beta.5.
