# REPORT: SPEC-repo-consolidation — fleet size recorded in §3.3, Phase 6 ordering made explicit as tidiness — 2026-09-11

**DONE** — `docs/SPEC-repo-consolidation.md` §3.3 now carries a fleet-size paragraph (one dial, the bench unit, on `1.0.1` and already repointed; both flashers serving a repointed image; the stranding argument therefore precautionary, the `1.0.1` Release on the old repo kept as cheap insurance) together with the condition that ends it (asset `downloadCount` or Insights traffic showing pulls or clones that are not the owner's). §5's Phase 6 ordering constraint gains the sentence that, at this fleet size, the tag-only trigger makes the ordering tidiness rather than a correctness risk. Nothing in §3.3 or §6 was deleted. Committed alone as `d7b3ec836e474b173086f5808d5cba7cec18a5cd` (parent `605a7a2`). **Not pushed.** Nothing under `firmware/`, `.github/` or `web-flasher/` touched. **Read Deviations first**: this task's item 2 was already on disk from the immediately preceding task (`14045e9`, `docs/REPORT-spec-phase6-order.md`), so item 2 here is one added sentence, not a new paragraph.

## Gate (both passed)

```
$ grep -c 'after `1.0.1` stable has been dual-published and the §7 gate has passed' docs/SPEC-repo-consolidation.md
1
$ git --no-optional-locks status --short --untracked-files=no
$
```

Check 2 printed nothing. (Run at `605a7a2`, after the preceding task's two commits.)

## Premise: the `release.yml` trigger

`.github/workflows/release.yml` lines 24–27, verbatim:

```
24:on:
25:  push:
26:    tags:
27:      - 'somnus-v*'
```

Line 28 is blank; line 29 begins `permissions:`. No `workflow_dispatch`, no branch push, no `pull_request`. A commit to `main` never runs it; only a `somnus-v*` tag push does, so both Pages sites stay as the last release run left them until the next tag.

## Where the fleet-size paragraph went, and why

**§3.3**, as its closing paragraph (line 95, just before `## 4.`). §3.3 is where a reader first meets the stranding argument: it defines the dual-publish window and says why `1.0.1` had to be dual-published for a `1.0.0` dial with Beta builds off. §6's one-way-door bullet (now line 143) is the second, longer statement of the same argument and cites §3.3's window. Putting the fleet fact at the first occurrence means the reader reaches §6 already knowing the argument is precautionary; the paragraph names §6 explicitly so the reverse direction is covered too. §6 itself is unchanged.

## Passages changed — BEFORE and AFTER

### 1. §3.3 closing paragraph (fleet size)

BEFORE — line 94 at `605a7a2`, followed directly by the §4 heading:

```

## 4. What changes in `release.yml` — described, not implemented here
```

AFTER — the new line 95, inserted between those two:

```
**Fleet size, recorded 2026-09-11.** Everything above was written before the repoint shipped, and it is now overstated; this section should say why. There is exactly one dial in existence: the bench unit, running `1.0.1`, which polls the new repo. Both browser flashers serve the `1.0.1` merged image, and that image is itself repointed, so a board flashed from either page comes up polling the new repo; the only way left to make a pre-repoint unit is to flash an older release asset on purpose. The two further Waveshare boards inbound will be flashed from a flasher and arrive repointed too. So the window has no population left to protect, and the stranding argument here and in §6 is precautionary rather than load-bearing. The standing rule stays — the `1.0.1` Release on `somnus-dial-releases` is never deleted — but as cheap insurance against a board that turns up later, not as a live dependency. This stops being true the day GitHub shows activity from outside the project: the repo is public, and the fleet is assumed to be one unit only until then. The signals are the release assets' cumulative `downloadCount`, which every tag-push report already quotes (as of 2026-09-11: 2 and 1 on the old repo's `1.0.1-beta.1` assets, both the bench dial's own pull; 0 on the new repo), and the repository's Insights traffic — clones, unique cloners, unique visitors — a 14-day rolling window GitHub does not retain, so it has to be looked at or captured while it is there. Once either shows pulls or clones that are not the owner's own, this fleet-size assumption is stale and the stranding reasoning in this section and §6 is operative again.
```

### 2. §5 Phase 6 ordering-constraint paragraph (tidiness sentence)

BEFORE — line 135 at `605a7a2`:

```
**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. That is the same silent-staleness shape as the beta-not-found finding of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`).
```

AFTER — line 137 at `d7b3ec8` (one sentence appended; the rest of the paragraph is unchanged):

```
**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. That is the same silent-staleness shape as the beta-not-found finding of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`). Given the fleet size in §3.3, though, this ordering is tidiness, not a correctness risk: the only harm of freezing early is the old flasher page staying pinned at `1.0.1` for whoever finds it, and as of 2026-09-11 nobody but the owner has.
```

## Diff and commit

```
$ git --no-optional-locks diff --stat
 docs/SPEC-repo-consolidation.md | 4 +++-
 1 file changed, 3 insertions(+), 1 deletion(-)
$ git rev-parse HEAD
d7b3ec836e474b173086f5808d5cba7cec18a5cd
```

The one "deletion" is the Phase 6 paragraph's line being rewritten with the sentence appended; no existing sentence was removed anywhere. Gate-1 grep still returns 1 after the edit. §1, §4(b), §6 and every other section untouched.

## Deviations

1. **Item 2 was already on disk.** This task arrived while the preceding task (`docs/REPORT-spec-phase6-order.md`, spec commit `14045e9`) was finishing. That task had already added to §5 a paragraph stating the tag-only trigger, that the flasher repoint and the workflow removal are inert until the next tag, that they ride `somnus-v1.0.2-beta.1`, and that the old repo must not be frozen or archived before the repointed flasher is deployed and checked live — plus a 6a/6b split of the phase. That covers everything in this task's item 2 except the sentence "this is tidiness, not a correctness risk, given the fleet size". Writing item 2 as a second paragraph would have said the same thing twice in the same block, so the missing sentence was appended to the existing paragraph instead, with the "old flasher page stays pinned at `1.0.1`" consequence folded into it. If the owner wanted the earlier 6a/6b text replaced by this task's shorter form, that is a separate edit; nothing from `14045e9` was removed.
2. **The `downloadCount` figures are quoted as the owner's 2026-09-11 observation, not re-derived.** The tag-push reports quote the field at publish time, when it is 0 (`grep -c '"downloadCount":0'`:
```
docs/REPORT-1.0.1-beta.1-tag-push.md:2
docs/REPORT-1.0.1-tag-push.md:2
```
). Non-zero values do appear in the docs tree, so the "2 and 1" figure is consistent with something already recorded:
```
docs/REPORT-publish-audit.md:165:"downloadCount":1
docs/REPORT-publish-audit.md:168:"downloadCount":1
docs/REPORT-publish-audit.md:168:"downloadCount":2
docs/REPORT-release-latest-pointer.md:118:"downloadCount":1
docs/REPORT-release-latest-pointer.md:118:"downloadCount":2
docs/REPORT-release-latest-pointer.md:80:"downloadCount":1
```
No GitHub API call was made in this task to re-read the current counts; the new paragraph attributes the figures to "as of 2026-09-11", which is the owner's statement.

## Not verifiable without hardware or GitHub

- That there is exactly one dial in existence and that it is on `1.0.1`: the bench-gate report (`docs/REPORT-1.0.1-bench-gate.md`) shows the bench unit on `1.0.1`, and the owner's statement supplies the "only one". Nothing on disk can prove a negative about other boards.
- That both browser flashers currently serve the `1.0.1` merged image: `docs/REPORT-1.0.1-tag-push.md` records both Pages sites returning 200 at the `1.0.1` asset size on 2026-09-11; not re-fetched here.
- Current `downloadCount` values and the Insights traffic window: neither was queried. The paragraph tells a later reader to look; it does not claim to have looked.
- No dial was on the bench and none was needed; documentation only.
