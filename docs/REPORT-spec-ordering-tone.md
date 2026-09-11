# REPORT: SPEC-repo-consolidation §5 — ordering-constraint paragraph reconciled to the tidiness framing

**DONE.** The sentence likening early freezing of the old repo to the beta-not-found finding of 2026-09-03 is removed from the §5 ordering-constraint paragraph, together with its `docs/REPORT-ota-beta-not-found.md` citation. The mechanism, the ordering it implies, and the closing tidiness sentence with its "pinned at `1.0.1`" consequence are unchanged. Spec committed alone as `3efbe2f`. Not pushed.

Date: 2026-09-11. Repo: `~/Projects/somnus-waveshare-rotary-dial`, branch `main`. Documentation only: no firmware, workflow, version bump or push.

## Gate

All three checks passed before anything was changed. Raw output:

```
$ grep -c 'same silent-staleness shape as the beta-not-found finding' docs/SPEC-repo-consolidation.md
1
$ grep -c 'this ordering is tidiness, not a correctness risk' docs/SPEC-repo-consolidation.md
1
$ git --no-optional-locks status --short --untracked-files=no

```

(The third check printed nothing; the tree was clean.)

## Paragraph, BEFORE

**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. That is the same silent-staleness shape as the beta-not-found finding of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`). Given the fleet size in §3.3, though, this ordering is tidiness, not a correctness risk: the only harm of freezing early is the old flasher page staying pinned at `1.0.1` for whoever finds it, and as of 2026-09-11 nobody but the owner has.

## Paragraph, AFTER

**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. Given the fleet size in §3.3, though, this ordering is tidiness, not a correctness risk: the only harm of freezing early is the old flasher page staying pinned at `1.0.1` for whoever finds it, and as of 2026-09-11 nobody but the owner has.

Only the one sentence was removed. No connective wording needed changing: the surviving "though" in the tidiness sentence now pivots directly against "first-install users would silently keep being offered the last build that repo ever saw", which is the contrast it was written for.

## `docs/REPORT-ota-beta-not-found.md` is still cited elsewhere

This was not its only mention. After the edit the file still cites it on lines 84,160:

```
84: - **(c) The 50-tag page cap.** At 16 tags today and roughly two to three per release cycle (one stable plus it…
160: 3. **A serial capture of item 2 is required** and goes into the bring-up record: it must contain the `ota: no …
```

Line 84 is §4(c), the 50-tag page cap, where the comparison is apt (a paginated list hiding the newest entry, at 50 instead of 5). Line 160 is the §7 bring-up step requiring a serial capture, where the comparison is likewise apt (a silent "up to date" on the display). Both are the same failure shape as the original finding; the §5 case was not.

```
$ grep -c 'same silent-staleness shape' docs/SPEC-repo-consolidation.md
0
```

## Diff and commit

```
$ git diff --stat HEAD~1 HEAD
 docs/SPEC-repo-consolidation.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

Spec commit: `3efbe2f085ea56a64566d06b3d83e3b62a1390d1` — `docs: SPEC-repo-consolidation §5 — drop the beta-not-found comparison from the ordering paragraph`. It contains only that file. This report and its `docs/REPORTS.md` line are a second, following commit. Nothing was pushed; `main` is now six commits ahead of the last push (the four already unpushed plus these two).

Untouched, as instructed: §3.3's fleet-size paragraph, the 6a/6b bullets, §1, §4(b), §6, and everything under `firmware/`, `.github/` and `web-flasher/`.

## Deviations

No deviations.

## Not verifiable without hardware

Nothing in this task touches the device. Nothing required hardware to verify.
