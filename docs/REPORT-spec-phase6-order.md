# REPORT: SPEC-repo-consolidation §5 — Phase 6 ordering constraint and 6a/6b split — 2026-09-11

**DONE** — §5's Phase 6 block in `docs/SPEC-repo-consolidation.md` now records that `release.yml` runs only on a `somnus-v*` tag push, that the flasher repoint and the workflow edits are therefore inert until the next release deploys them, and that the old repo must not be frozen or archived until the repointed flasher has been deployed and checked live. The phase is split into 6a (on disk, lands with `somnus-v1.0.2-beta.1`) and 6b (after the new flasher is confirmed live: secret, PAT, redirect README and Pages index, archive), and the one-sentence rationale for in-repo-only publishing from `1.0.2-beta.1` onward is on disk with the standing never-remove rule restated. The five numbered steps are unchanged. Committed alone as `14045e91b795d813615b20b691b2d2c5bab51139` (parent `5bac7b9`). **Not pushed.** Nothing under `firmware/`, `.github/` or `web-flasher/` touched. One deviation to read (Deviations, item 1): `release.yml` contains no comment reading "stays private", so the 6a description names the stale comments by what they actually say.

## Gate (both passed)

```
$ grep -c 'after `1.0.1` stable has been dual-published and the §7 gate has passed' docs/SPEC-repo-consolidation.md
1
$ git --no-optional-locks status --short --untracked-files=no
$
```

Check 2 printed nothing.

## Premise: the `release.yml` trigger

`.github/workflows/release.yml` lines 24–27, verbatim:

```
24:on:
25:  push:
26:    tags:
27:      - 'somnus-v*'
```

Line 28 is blank and line 29 begins `permissions:`. There is no `workflow_dispatch`, no `push: branches`, no `pull_request`. A commit to `main` does not run this workflow; only a tag push matching `somnus-v*` does. The other premises checked before editing: the Phase 6 preamble is line 127 and reads exactly as the gate string; the five numbered steps are lines 129–133; `web-flasher/index.html` line 304 still fetches `matthewclaude/somnus-dial-releases/releases/latest` and lines 280, 282 and 288 still link into that repo; `release.yml` still has the old-repo `action-gh-release` step (lines 129–132, `repository: matthewclaude/somnus-dial-releases`, `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}`) and the old-repo `peaceiris` deploy (lines 187–195, `external_repository: matthewclaude/somnus-dial-releases`). Precondition confirmed on disk: `docs/REPORT-1.0.1-tag-push.md` records the dual Release and dual Pages, and `docs/REPORT-1.0.1-bench-gate.md` opens with **PASS**, both dated 2026-09-11.

## §5 Phase 6 block — BEFORE (lines 127–135 at `5bac7b9`)

```
**Phase 6, out of scope here** — the cut-over, after `1.0.1` stable has been dual-published and the §7 gate has passed against the stable build. Listed so the sequence is on disk without being tasked:

1. Repoint `web-flasher/index.html`: the `/releases/latest` fetch and the three footer links to `bedknob-for-somnus`.
2. Repoint the release-notes footer (§4(c)) to `https://matthewclaude.github.io/bedknob-for-somnus/`.
3. Replace the old repo's README with a frozen-redirect README: this repo is archived; source, Releases and the flasher are at `bedknob-for-somnus`; dials on 1.0.0 that never updated need one wire flash from the new flasher (§6). Make the old Pages site's `index.html` a meta-refresh redirect to the new one (§8).
4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
5. Archive `somnus-dial-releases`. Archive, not delete, not rename (§1).

Each of those is its own gated step with its own report. None of them happens in this beta.
```

## §5 Phase 6 block — AFTER (lines 127–144 at `14045e9`)

```
**Phase 6, out of scope here** — the cut-over, after `1.0.1` stable has been dual-published and the §7 gate has passed against the stable build. Listed so the sequence is on disk without being tasked:

1. Repoint `web-flasher/index.html`: the `/releases/latest` fetch and the three footer links to `bedknob-for-somnus`.
2. Repoint the release-notes footer (§4(c)) to `https://matthewclaude.github.io/bedknob-for-somnus/`.
3. Replace the old repo's README with a frozen-redirect README: this repo is archived; source, Releases and the flasher are at `bedknob-for-somnus`; dials on 1.0.0 that never updated need one wire flash from the new flasher (§6). Make the old Pages site's `index.html` a meta-refresh redirect to the new one (§8).
4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
5. Archive `somnus-dial-releases`. Archive, not delete, not rename (§1).

**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. That is the same silent-staleness shape as the beta-not-found finding of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`).

The phase therefore splits in two:

- **6a — on disk, landing with the next release.** Items 1, 2 and the workflow half of item 4: repoint `index.html`'s `/releases/latest` fetch and its three footer links, add a link to the source repo itself, repoint the release-notes footer, remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy, and rewrite the workflow's stale comments — the top-of-file block and the changelog-extractor note that still describe dual-publishing into `somnus-dial-releases` and the token that does it. These go live with `somnus-v1.0.2-beta.1`, the standby-poll release (`SPEC-standby-poll.md`) already queued behind this, rather than needing a release of their own.
- **6b — after that release has deployed** and the new flasher page has been opened and confirmed to fetch `bedknob-for-somnus`: the secret half of item 4 (delete `SOMNUS_RELEASES_TOKEN`, revoke the PAT), then item 3 (the frozen-redirect README and the meta-refresh Pages index on the old repo), then item 5 (archive).

In-repo-only publishing is safe from `1.0.2-beta.1` onward because `1.0.1` was the last dual-published release: a dial still on `1.0.0` migrates by installing the `1.0.1` Release that remains on the archived old repo, and the standing rule holds — that Release is never removed.

Each of those is its own gated step with its own report. None of them happens in this beta.
```

## Diff and commit

```
$ git --no-optional-locks diff --stat
 docs/SPEC-repo-consolidation.md | 9 +++++++++
 1 file changed, 9 insertions(+)
$ git rev-parse HEAD
14045e91b795d813615b20b691b2d2c5bab51139
```

Nine lines inserted between item 5 and the closing "Each of those is its own gated step" sentence; no line removed or altered. After the edit the gate-1 grep still returns 1. §1, §4(b) and every other section untouched.

## Deviations

1. **The "stays private" comments do not exist.** The instructions describe 6a as including a rewrite of "the workflow's stale 'stays private' comments". `grep -n -i private .github/workflows/release.yml` matches nothing. What the workflow does have is a top-of-file comment block (lines 10–21) saying the tag is pushed to "THIS repo, which is public" and that the Release and gh-pages site "are published here AND into the matthewclaude/somnus-dial-releases repo" using `SOMNUS_RELEASES_TOKEN`, and a changelog-extractor comment (lines 74–78) explaining there is no compare link because the notes are also published into the old repo. Both become stale at 6a. Rather than stop, or write a phrase that names comments the file does not contain, the 6a bullet describes them as "the top-of-file block and the changelog-extractor note that still describe dual-publishing into `somnus-dial-releases` and the token that does it". The intent — rewrite the comments that describe the old-repo arrangement — is preserved; the quoted phrase is not. If the owner meant a different comment, the 6a bullet is the line to correct.
2. **`SPEC-standby-poll.md` does not name `1.0.2-beta.1`.** Its status line still reads "Scheduled as `0.1.7-beta.1`, after `0.1.6` graduates to stable" (dated 2026-09-06), and its release item says only "its own beta". The new §5 text says `somnus-v1.0.2-beta.1` as instructed, on the owner's statement that it is the queued standby-poll release; `SPEC-repo-consolidation.md`'s own status line already says the standby-poll change "still queues behind this". The standby-poll spec's stale version number was not touched — it is outside this task's one-file scope.

Nothing else deviates: the five steps are verbatim, no other file changed except this report and its `docs/REPORTS.md` line, no push, no tag, no firmware, workflow or flasher edit.

## Not verifiable without hardware or a release run

- That `somnus-v1.0.2-beta.1`'s release run will in fact deploy a repointed flasher to both Pages sites, and that the new page then fetches `bedknob-for-somnus/releases/latest`. That is the 6b entry check and is by design a live check after the release, not something a spec edit can show.
- That an archived `somnus-dial-releases` keeps answering `/releases/latest` and serving the `1.0.1` asset. The spec's §5 one-way-door paragraph already rests on that GitHub behaviour; this edit adds nothing to it and tests nothing.
- No dial was on the bench and none was needed; this task is documentation only.
