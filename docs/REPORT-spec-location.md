# REPORT: Where does SPEC-home-integration.md actually live? — read-only investigation

Date: 2026-09-14. Follows `docs/REPORT-mcp-integration.md`, which created `docs/SPEC-home-integration.md` in `abac7a1` after finding no prior file in the repo.

## 1. Verdict

**NO.** No pre-existing "real" `SPEC-home-integration.md` exists on disk anywhere under `~/Projects`. The only copy on disk is the one commit `abac7a1` added to this repo. No `GUIDE-home-assistant-setup.md` exists under `~/Projects` either.

## 2. Read-only confirmation

Nothing was changed, reverted, committed or pushed. The only file written by this task is this report, at the path the task names. `docs/REPORTS.md` was deliberately **not** given an index line for this report (see section 6). The working tree before and after this task is identical: `main` one commit ahead of `somnus/main` (`abac7a1`, unpushed), `docs/REPORTS.md` modified by one line and `docs/REPORT-mcp-integration.md` untracked, both left over from the previous task.

## 3. Raw, unfiltered output of every command

Run from the repo root in the session shell (zsh). Each command is followed by its exit code.

```
$ git rev-parse --show-toplevel
/Users/matthew/Projects/somnus-waveshare-rotary-dial
exit 0

$ git --no-optional-locks status --short --branch
## main...somnus/main [ahead 1]
 M docs/REPORTS.md
?? docs/REPORT-mcp-integration.md
exit 0

$ git log --oneline --follow -- docs/SPEC-home-integration.md
abac7a1 docs: SPEC-home-integration - Home Assistant MCP server (Assist) via Claude Desktop + mcp-remote, 2026-09-14
exit 0

$ git show --stat abac7a1
commit abac7a1b232d1665cac898b3e74e0be434d2d244
Author: Matthew Montgomery <mattseattle@icloud.com>
Date:   Mon Sep 14 16:33:39 2026 -0500

    docs: SPEC-home-integration - Home Assistant MCP server (Assist) via Claude Desktop + mcp-remote, 2026-09-14
    
    New file. Records the HA "Model Context Protocol Server" integration
    (Streamable HTTP, stateless, home-assistant 1.26.0, protocol 2025-06-18,
    long-lived token), the Claude Desktop config bridged by mcp-remote@0.14.2
    with the absolute npx path, the LAN-only stance, the no-number-domain
    constraint that keeps the pad setpoint out of reach, the unverified
    climate-entity path, the MCP client as a fourth pad writer, and four
    troubleshooting notes. The task asked to update an existing spec and
    cross-reference an Alexa RangeController finding and a section 7.5;
    neither the file nor those sections exist in this repo, so section 2
    lists them as not present instead of paraphrasing them.
    
    Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_01QTmZqggMYuy2SHqdpnVWCT

 docs/SPEC-home-integration.md | 93 +++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 93 insertions(+)
exit 0

$ ls ~/Projects/*/docs/SPEC-home-integration.md 2>/dev/null
/Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/SPEC-home-integration.md
exit 0

$ ls ~/Projects/*/docs/GUIDE-home-assistant-setup.md 2>/dev/null
(eval):7: no matches found: /Users/matthew/Projects/*/docs/GUIDE-home-assistant-setup.md
exit 1

$ find ~/Projects -maxdepth 3 -name "SPEC-home-integration*" 2>/dev/null
/Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/SPEC-home-integration.md
exit 0

$ find ~/Projects -maxdepth 3 -name "GUIDE-home-assistant-setup*" 2>/dev/null
exit 0

$ ls -la docs/
total 4248
drwxr-xr-x  106 matthew  staff   3392 Sep 14 16:34 .
drwxr-xr-x   21 matthew  staff    672 Sep 11 13:00 ..
-rw-r--r--@   1 matthew  staff   8196 Sep  4 12:11 .DS_Store
-rw-r--r--    1 matthew  staff  14572 Sep 13 14:50 ARCHITECTURE.md
-rw-r--r--    1 matthew  staff   3413 Sep  7 12:42 dial-audit-run.log
-rw-------    1 matthew  staff   6315 Aug 30 17:44 local_api.yml
-rw-r--r--    1 matthew  staff   5465 Sep 13 14:50 NAMING.md
-rw-r--r--    1 matthew  staff   9876 Sep  4 22:06 PLAN-screen-layout-fixes.md
-rw-r--r--    1 matthew  staff   6910 Sep  3 19:48 REPORT-0.1.5-beta.3-release.md
-rw-r--r--    1 matthew  staff   4758 Sep  3 21:02 REPORT-0.1.5-beta.4-release.md
-rw-r--r--    1 matthew  staff   5755 Sep  4 19:21 REPORT-0.1.5-beta.5-ci-check.md
-rw-r--r--    1 matthew  staff   4128 Sep  9 18:21 REPORT-0.1.5-beta.5-commit.md
-rw-r--r--    1 matthew  staff   3823 Sep  9 18:21 REPORT-0.1.5-beta.5-tag-push.md
-rw-r--r--    1 matthew  staff   6473 Sep  4 20:11 REPORT-0.1.5-ci-check.md
-rw-r--r--    1 matthew  staff   4136 Sep  4 20:03 REPORT-0.1.5-graduation-commit.md
-rw-r--r--    1 matthew  staff   3255 Sep  4 20:05 REPORT-0.1.5-tag-push.md
-rw-r--r--    1 matthew  staff   8973 Sep  4 20:54 REPORT-0.1.5-upgrade-path-verify.md
-rw-r--r--    1 matthew  staff  34828 Sep  7 15:52 REPORT-0.1.6-graduation.md
-rw-r--r--    1 matthew  staff  13502 Sep  9 17:13 REPORT-1.0.0-commit.md
-rw-r--r--    1 matthew  staff   9618 Sep  9 17:42 REPORT-1.0.0-tag-push.md
-rw-r--r--    1 matthew  staff   6596 Sep 11 13:48 REPORT-1.0.1-bench-gate.md
-rw-r--r--    1 matthew  staff  17180 Sep 10 14:42 REPORT-1.0.1-beta.1-commit.md
-rw-r--r--    1 matthew  staff  16724 Sep 10 14:50 REPORT-1.0.1-beta.1-tag-push.md
-rw-r--r--    1 matthew  staff  10222 Sep 11 13:03 REPORT-1.0.1-commit.md
-rw-r--r--    1 matthew  staff  12214 Sep 11 13:36 REPORT-1.0.1-tag-push.md
-rw-r--r--    1 matthew  staff  14723 Sep 11 15:25 REPORT-1.0.2-bench-cadence.md
-rw-r--r--    1 matthew  staff  16407 Sep 11 16:06 REPORT-1.0.2-bench-items-2-3.md
-rw-r--r--    1 matthew  staff  17691 Sep 12 18:57 REPORT-1.0.2-bench-items-4-5.md
-rw-r--r--    1 matthew  staff   7351 Sep 12 19:07 REPORT-1.0.2-beta.1-commit.md
-rw-r--r--    1 matthew  staff  16614 Sep 13 13:57 REPORT-1.0.2-beta.1-ota-bench.md
-rw-r--r--    1 matthew  staff  18448 Sep 12 19:18 REPORT-1.0.2-beta.1-tag-push.md
-rw-r--r--    1 matthew  staff  14371 Sep 12 07:22 REPORT-1.0.2-overnight-analysis.md
-rw-r--r--    1 matthew  staff   9278 Sep 11 20:43 REPORT-1.0.2-overnight-start.md
-rw-r--r--    1 matthew  staff  10307 Sep 12 07:22 REPORT-1.0.2-overnight.md
-rw-r--r--    1 matthew  staff   6973 Sep  4 15:42 REPORT-about-layout-battery-glyph.md
-rw-r--r--    1 matthew  staff  55329 Sep  4 15:09 REPORT-battery-pct-about-redesign.md
-rw-r--r--    1 matthew  staff  11214 Sep 11 16:07 REPORT-bench-notes-correction.md
-rw-r--r--    1 matthew  staff  21060 Sep  7 12:44 REPORT-dial-display-audit.md
-rw-r--r--    1 matthew  staff  10854 Sep 10 09:00 REPORT-docs-amend.md
-rw-r--r--    1 matthew  staff   8276 Sep 10 09:00 REPORT-docs-commit.md
-rw-r--r--    1 matthew  staff  25988 Sep  9 18:24 REPORT-docs-housekeeping.md
-rw-r--r--    1 matthew  staff  12335 Sep  4 21:14 REPORT-docs-sync.md
-rw-r--r--    1 matthew  staff   6407 Sep 11 12:46 REPORT-handoff.md
-rw-r--r--    1 matthew  staff  49028 Sep  5 17:30 REPORT-layout-a1b.md
-rw-r--r--    1 matthew  staff  31611 Sep  4 22:18 REPORT-layout-phase1-flash.md
-rw-r--r--    1 matthew  staff  30963 Sep  4 22:07 REPORT-layout-phase1.md
-rw-r--r--    1 matthew  staff  50211 Sep  6 08:37 REPORT-layout-tier-bc.md
-rw-r--r--    1 matthew  staff  10796 Sep 14 16:34 REPORT-mcp-integration.md
-rw-r--r--    1 matthew  staff  32755 Sep  3 18:10 REPORT-night-face.md
-rw-r--r--    1 matthew  staff   6718 Sep  9 18:21 REPORT-ota-beta-not-found.md
-rw-r--r--    1 matthew  staff  17281 Sep 10 11:50 REPORT-phase1-scrub.md
-rw-r--r--    1 matthew  staff  15797 Sep 10 12:01 REPORT-phase1b-wording.md
-rw-r--r--    1 matthew  staff  20359 Sep 12 13:05 REPORT-phase6a.md
-rw-r--r--    1 matthew  staff  20106 Sep 13 14:43 REPORT-phase6b.md
-rw-r--r--    1 matthew  staff  33345 Sep 13 14:56 REPORT-phase7-doc-sweep.md
-rw-r--r--    1 matthew  staff  11358 Sep 10 14:05 REPORT-post-public-sweep.md
-rw-r--r--    1 matthew  staff  13030 Sep  3 20:07 REPORT-power-indicator.md
-rw-r--r--    1 matthew  staff  10884 Sep 10 12:45 REPORT-public-readme-fix.md
-rw-r--r--    1 matthew  staff  45818 Sep 10 12:32 REPORT-publish-audit.md
-rw-r--r--    1 matthew  staff  58852 Sep  5 17:43 REPORT-rails-fix.md
-rw-r--r--    1 matthew  staff  12192 Sep 10 11:34 REPORT-readme-license-fix.md
-rw-r--r--    1 matthew  staff  57824 Sep  5 17:55 REPORT-release-0.1.6-beta.1.md
-rw-r--r--    1 matthew  staff  56380 Sep  5 19:14 REPORT-release-0.1.6-beta.2.md
-rw-r--r--    1 matthew  staff  10272 Sep  6 08:57 REPORT-release-0.1.6-beta.3.md
-rw-r--r--    1 matthew  staff  18298 Sep 10 08:47 REPORT-release-latest-pointer.md
-rw-r--r--    1 matthew  staff  33373 Sep  4 21:41 REPORT-releases-repo-readme.md
-rw-r--r--    1 matthew  staff  32547 Sep  5 17:40 REPORT-screen-layout-audit.md
-rw-r--r--    1 matthew  staff   3510 Sep  9 17:58 REPORT-sim-screens-push.md
-rw-r--r--    1 matthew  staff  20247 Sep  9 17:55 REPORT-sim-version-screens.md
-rw-r--r--    1 matthew  staff  10704 Sep 11 14:04 REPORT-spec-fleet-and-order.md
-rw-r--r--    1 matthew  staff   4844 Sep 11 12:36 REPORT-spec-gate-consistency.md
-rw-r--r--    1 matthew  staff   9555 Sep 11 12:32 REPORT-spec-gate-reword.md
-rw-r--r--    1 matthew  staff   6057 Sep 11 14:08 REPORT-spec-ordering-tone.md
-rw-r--r--    1 matthew  staff  10505 Sep 11 14:02 REPORT-spec-phase6-order.md
-rw-r--r--    1 matthew  staff  14843 Sep 10 14:33 REPORT-spec-repo-consolidation.md
-rw-r--r--    1 matthew  staff   9971 Sep 11 12:54 REPORT-spec-stable-gate.md
-rw-r--r--    1 matthew  staff  21658 Sep 11 14:28 REPORT-spec-standby-300s.md
-rw-r--r--    1 matthew  staff  31282 Sep  5 18:21 REPORT-standby-face-c1.md
-rw-r--r--    1 matthew  staff  40457 Sep  5 18:49 REPORT-standby-face-c2.md
-rw-r--r--    1 matthew  staff  21677 Sep 11 14:35 REPORT-standby-poll-code-checks.md
-rw-r--r--    1 matthew  staff  15432 Sep 11 14:43 REPORT-standby-poll-impl.md
-rw-r--r--    1 matthew  staff  24666 Sep  5 19:04 REPORT-sticky-brightness.md
-rw-r--r--    1 matthew  staff  27366 Sep  5 18:59 REPORT-sticky-night-pickers.md
-rw-r--r--    1 matthew  staff  13943 Sep 10 12:28 REPORT-third-party-licenses-restore.md
-rw-r--r--    1 matthew  staff  18911 Sep 10 12:17 REPORT-third-party-licenses.md
-rw-r--r--    1 matthew  staff  48708 Sep 14 16:34 REPORTS.md
-rw-r--r--    1 matthew  staff  30926 Sep  2 18:00 REVIEW-2026-09-02.md
drwxr-xr-x   51 matthew  staff   1632 Sep  6 08:33 screens
-rw-------    1 matthew  staff  11573 Sep 10 11:42 SPEC-brand-palette.md
-rw-r--r--    1 matthew  staff  27395 Sep 10 11:54 SPEC-connect-phases.md
-rw-r--r--    1 matthew  staff   7638 Sep 10 11:42 SPEC-dial-side-scheduling.md
-rw-------    1 matthew  staff  16244 Sep 10 11:42 SPEC-differential-firmware-port.md
-rw-r--r--    1 matthew  staff   9119 Sep 14 16:33 SPEC-home-integration.md
-rw-------    1 matthew  staff  12082 Sep 10 11:42 SPEC-night-face.md
-rw-r--r--    1 matthew  staff  20513 Sep 10 11:42 SPEC-night-window.md
-rw-r--r--    1 matthew  staff  83913 Sep 13 14:58 SPEC-ota-readiness.md
-rw-r--r--    1 matthew  staff  45816 Sep 10 11:54 SPEC-pad-discovery.md
-rw-------    1 matthew  staff  37339 Sep 10 13:25 SPEC-power-sensing.md
-rw-r--r--    1 matthew  staff  35724 Sep 13 14:48 SPEC-repo-consolidation.md
-rw-r--r--    1 matthew  staff   6766 Sep 10 11:42 SPEC-somnus-relative-scale.md
-rw-r--r--    1 matthew  staff   9205 Sep 10 11:42 SPEC-standby-face.md
-rw-------    1 matthew  staff  17231 Sep 12 18:10 SPEC-standby-poll.md
-rw-r--r--    1 matthew  staff  20289 Sep 10 11:42 SPEC-timezone-source.md
-rw-r--r--    1 matthew  staff  11272 Sep 10 11:42 SPEC-update-prompt.md
-rw-------    1 matthew  staff  20006 Sep 10 11:42 SPEC-voice.md
-rw-------@   1 matthew  staff   9076 Sep  9 18:20 TEST-F3-stuck-loop.md
exit 0

$ git diff --stat
 docs/REPORTS.md | 1 +
 1 file changed, 1 insertion(+)
exit 0

$ git rev-parse HEAD
abac7a1b232d1665cac898b3e74e0be434d2d244
exit 0
```

## 4. `git diff --stat` and current HEAD

```
$ git diff --stat
 docs/REPORTS.md | 1 +
 1 file changed, 1 insertion(+)

$ git rev-parse HEAD
abac7a1b232d1665cac898b3e74e0be434d2d244
```

HEAD is `abac7a1` itself, the commit under investigation. The one-line `REPORTS.md` diff is the previous task's uncommitted index line for `REPORT-mcp-integration.md`; it is not part of `abac7a1` and was not touched here.

## 5. Recommendation: `abac7a1` should STAND

Reasoning, from the evidence above:

1. **It is genuinely new content with no on-disk counterpart.** `git log --oneline --follow` for the path returns exactly one commit, `abac7a1`, so the file has no earlier history in this repo. The two `ls` globs and two `find` sweeps over `~/Projects` (depth 3, which reaches `~/Projects/<repo>/docs/<file>`) return only `/Users/matthew/Projects/somnus-waveshare-rotary-dial/docs/SPEC-home-integration.md`, the file the commit created. There is nothing it duplicates and nothing it conflicts with.
2. **It adds one file and touches nothing else.** `git show --stat abac7a1`: `1 file changed, 93 insertions(+)`. No existing file was modified, so a revert would gain nothing except deleting the only on-disk record of the MCP integration.
3. **The file is honest about its own provenance.** Its Status line and §2 state that it was created new, that the task expected an existing spec, and that the Alexa RangeController finding and "§7.5" are not in this repo. If the intended spec turns up somewhere this investigation could not see (section 7), the file's §1 is written to be lifted into it, and the file can then be replaced or renamed in an ordinary docs commit. Reverting now would not make that merge easier.
4. **The `GUIDE-home-assistant-setup.md` probe was negative too**, so there is no sibling document on disk under `~/Projects` that `abac7a1` should have extended instead.

What a revert would require the owner to have: a copy of the intended spec that this search could not find. Absent that, keep `abac7a1`, and treat `docs/SPEC-home-integration.md` as the spec of record for this integration.

## 6. Deviations from the instructions

1. **The second `ls` printed a zsh error instead of nothing.** `ls ~/Projects/*/docs/GUIDE-home-assistant-setup.md 2>/dev/null` produced `(eval):7: no matches found: ...` with exit 1. The glob failed to match, and in zsh a non-matching glob is a shell error raised before `ls` runs, so `2>/dev/null` on the `ls` does not suppress it. The result is the same as an empty listing: no such file. The command was run exactly as written and not re-run with different options.
2. **No `docs/REPORTS.md` line was added for this report**, although the repo's convention (its preamble) is one line per report. The task says "do not change any files", which was taken to override the convention. The index is therefore one entry short of the file count until the next docs commit adds it.
3. **Each command was wrapped to print its exit code** (`echo "exit $?"`) so the raw output in section 3 shows success or failure for the silent ones. The commands themselves were not altered.

No other deviations. Every command listed in the task was run once, verbatim, and its output is shown unedited in section 3.

## 7. Not determinable from the filesystem alone

- **Whether the intended spec exists in the Claude Project** (the owner's non-repo document store, where `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md` and others live per the notes in `SPEC-standby-poll.md` and `SPEC-repo-consolidation.md`). That store is not on disk and was not consulted.
- **Whether a copy exists on disk outside `~/Projects`** (Desktop, Documents, Downloads, iCloud, another volume). The task's four search commands are all scoped to `~/Projects`, and the search was not widened, per "do not hunt for it elsewhere".
- **Whether `GUIDE-home-assistant-setup.md` was ever expected to exist**, or was named only as a second probe. Nothing on disk or in this repo's history mentions it.
- **Whether the Alexa RangeController finding and "§7.5" cited by the previous task exist anywhere.** They are absent from this repo (checked in the previous task across all refs); their existence elsewhere cannot be established from disk.
