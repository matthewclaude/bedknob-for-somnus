# REPORT: SPEC-repo-consolidation Phase 6a — cut the release workflow and the browser flasher over to this repo

Date: 2026-09-12. Repo `~/Projects/somnus-waveshare-rotary-dial`, branch `main`, starting HEAD `3da2dcd`.

**DONE.** Both files edited and committed together as `8ee93c41b7514834a6720d93f65afcc4a0002ca3`. No secret deleted, no PAT revoked, nothing archived, no version bump, no tag, no push. Nothing here is live until `somnus-v1.0.2-beta.1` runs the workflow (§8 below).

## 1. Gate (all three passed)

```
$ grep -c 'somnus-dial-releases' web-flasher/index.html
4
$ grep -c 'somnus-dial-releases' .github/workflows/release.yml
5
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

Premises checked before editing, all true on disk: the `Publish GitHub Release` step at lines 128–137 carried `repository: matthewclaude/somnus-dial-releases` and `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}`; the `Deploy to gh-pages` step at lines 186–201 carried `external_repository: matthewclaude/somnus-dial-releases` and `personal_token`; both `(this repo)` steps existed; `THIRD_PARTY_LICENSES.md` exists at the repo root (§5(d)); `docs/REPORT-phase6a.md` did not exist; the spec's 6a bullet (`docs/SPEC-repo-consolidation.md` line 141) lists exactly this work.

## 2. Edits to `.github/workflows/release.yml`

Line numbers: "was" = lines in `3da2dcd`, "now" = lines in `8ee93c4`.

### A6. Top-of-file comment block (was 10–21, now 10–17)

Before:
```
# The tag is pushed to (and this workflow runs in) THIS repo, which is
# public -- and since 1.0.1-beta.1 the Release object and the gh-pages site
# are published here AND into the matthewclaude/somnus-dial-releases repo
# (see docs/SPEC-repo-consolidation.md; docs/SPEC-ota-readiness.md §5 for
# why that repo existed). The old repo keeps getting releases because every
# shipped 1.0.0 dial polls it, and dual-publishing is how those dials find
# the build that repoints them. Both cross-repo writes (the Release API and
# the gh-pages branch push) use the SOMNUS_RELEASES_TOKEN secret -- a
# fine-grained PAT scoped to
# `contents: write` on that repo only, which covers both: a fine-grained
# PAT's "Contents" permission is what gates the Releases API AND git pushes
# to branches, so one token suffices for both jobs below.
```
After:
```
# The tag is pushed to (and this workflow runs in) THIS repo, which is
# public. The Release object and the gh-pages site are published here and
# nowhere else (docs/SPEC-repo-consolidation.md, Phase 6a).
# matthewclaude/somnus-dial-releases is the frozen pre-1.0.1 location that
# dials at or below 1.0.0 polled (docs/SPEC-ota-readiness.md §5 for why it
# existed). It is kept read-only and is never deleted or renamed, because
# its 1.0.1 Release is the permanent over-the-air migration path for any
# such dial. Nothing in this workflow writes to it.
```

### A7. Compare-link comment (was 74–80, now 70–73)

Before:
```
          # No "full changelog" compare link: the same notes get published
          # into THIS repo and into matthewclaude/somnus-dial-releases (see
          # top-of-file comment), and the latter has no commit/tag history
          # of its own -- a compare link built from ${GITHUB_REPOSITORY}
          # would resolve here but is dead text on the old repo's copy of
          # the notes, and one built against the old repo wouldn't resolve
          # at all (no tags to diff there). Revisit after the migration.
```
After:
```
          # No "full changelog" compare link yet. The notes are published
          # into THIS repo only, so a compare link built from
          # ${GITHUB_REPOSITORY} would now resolve; adding one is a separate
          # change, not part of the Phase 6a cut-over.
```
No link was added.

### A8. Release-notes footer flasher URL (was 86, now 79)

Before:
```
            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); \`somnus-dial-merged.bin\` below is the same image for flashing manually from offset 0x0."
```
After:
```
            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); \`somnus-dial-merged.bin\` below is the same image for flashing manually from offset 0x0."
```

### A1. Old-repo release step deleted (was 128–138, including the blank line after it)

Deleted in full, as it was:
```
      - name: Publish GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          repository: matthewclaude/somnus-dial-releases
          token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
          prerelease: ${{ steps.channel.outputs.prerelease }}
          body: ${{ steps.notes.outputs.body }}
          files: |
            firmware/dial-idf/build/somnus-dial.bin
            firmware/dial-idf/build/somnus-dial-merged.bin

```

### A4. Comment above the surviving release step (was 139–144, now 121–124)

Before:
```
      # Same Release, same notes, same two files, into THIS repo -- the
      # location dial_ota.c polls from 1.0.1-beta.1 on. No repository: or
      # token: keys: the action defaults to the workflow's own repo and to
      # GITHUB_TOKEN, which the workflow-level `contents: write` covers.
      # Runs after the old-repo step on purpose: if this one fails, the repo
      # that shipped 1.0.0 dials actually poll already has the Release.
```
After:
```
      # The only Release published: into THIS repo, which is where
      # dial_ota.c has polled since 1.0.1-beta.1. No repository: or
      # token: keys: the action defaults to the workflow's own repo and to
      # GITHUB_TOKEN, which the workflow-level `contents: write` covers.
```

### A3. Surviving step `Publish GitHub Release (this repo)` (was 145–152, now 125–132) — unchanged, name unchanged.

### A2. Old-repo Pages step deleted (was 186–202, including the blank line after it)

Deleted in full, as it was, including its comment block about `external_repository`:
```
      - name: Deploy to gh-pages
        uses: peaceiris/actions-gh-pages@v4
        with:
          # Same cross-repo PAT as the release-publish job above (see
          # top-of-file comment) -- external_repository + personal_token is
          # peaceiris/actions-gh-pages's documented way to push a gh-pages
          # branch into a different repo than the one the workflow runs in;
          # the default GITHUB_TOKEN can only ever push within this repo.
          personal_token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
          external_repository: matthewclaude/somnus-dial-releases
          publish_dir: ./_site
          publish_branch: gh-pages
          # NOT force_orphan: that wipes the branch on every deploy, which
          # would delete the OTHER channel's firmware image. keep_files
          # preserves firmware/latest/ when a beta publishes and vice versa.
          keep_files: true

```

### A5. Comment above the surviving Pages step (was 203–210, now 166–173)

Before:
```
      # The same _site into THIS repo's gh-pages branch, so the flasher
      # exists at this repo's Pages URL too (docs/SPEC-repo-consolidation.md
      # §4(b)). GITHUB_TOKEN is enough for a push within the workflow's own
      # repo. This repo has no gh-pages branch yet: the first run creates it
      # from _site alone. keep_files matters from the second run on, for the
      # same stable-vs-beta directory reason as above. Serving it is a
      # repo setting (Pages: gh-pages branch, root) that the workflow cannot
      # enable.
```
After:
```
      # _site into THIS repo's gh-pages branch, the only place the flasher
      # is deployed (docs/SPEC-repo-consolidation.md §4(b), Phase 6a).
      # GITHUB_TOKEN is enough for a push within the workflow's own repo.
      # The gh-pages branch exists: the 1.0.1-beta.1 run created it, and
      # Pages is enabled on it (gh-pages branch, root) and serving.
      # keep_files preserves the OTHER channel's directory across deploys:
      # a beta deploy must not delete firmware/latest/ and vice versa.
      # (NOT force_orphan, which wipes the branch on every deploy.)
```

### A3. Surviving step `Deploy to gh-pages (this repo)` (was 211–217, now 174–180) — unchanged, name unchanged.

The file is 180 lines now (was 217).

## 3. Edits to `web-flasher/index.html`

### B13 + B10. Footer `Releases` link, plus the new `Source` link (was 280, now 280–281)

Before:
```
      <a href="https://github.com/matthewclaude/somnus-dial-releases">Releases</a>
```
After:
```
      <a href="https://github.com/matthewclaude/bedknob-for-somnus">Source</a>
      <a href="https://github.com/matthewclaude/bedknob-for-somnus/releases">Releases</a>
```
`Releases` now carries the `/releases` path. `Source` uses the same `<a>` markup as its neighbours and sits first in the link row. The `Forked from Orion Dial` link (was 281, now 282) is byte-for-byte unchanged.

### B11. Footer `License` link (was 282, now 283)

Before:
```
      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/LICENSE">License</a>
```
After:
```
      <a href="https://github.com/matthewclaude/bedknob-for-somnus/blob/main/LICENSE">License</a>
```

### B12. Third-party notices link (was 288, now 289)

Before:
```
      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES">third-party
```
After:
```
      <a href="https://github.com/matthewclaude/bedknob-for-somnus/blob/main/THIRD_PARTY_LICENSES.md">third-party
```
The `.md` extension was verified with `ls` before writing (§5(d)).

### B9. `/releases/latest` fetch (was 304, now 305)

Before:
```
    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
```
After:
```
    fetch("https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/latest")
```

`web-flasher/manifest.json`, `web-flasher/manifest-beta.json`, `firmware/` and everything under `docs/` other than this report and its `REPORTS.md` line are untouched.

## 4. Verification, raw

### (a) `somnus-dial-releases` counts after the edit

```
$ grep -c 'somnus-dial-releases' web-flasher/index.html
0
$ grep -c 'somnus-dial-releases' .github/workflows/release.yml
1
$ grep -n 'somnus-dial-releases' .github/workflows/release.yml
13:# matthewclaude/somnus-dial-releases is the frozen pre-1.0.1 location that
```
The one surviving line is the deliberate top-of-file description of the frozen old repo (§2, A6). No `with:` key, URL or token reference to the old repo remains.

### (b) `SOMNUS_RELEASES_TOKEN` across the repo

```
$ grep -rn 'SOMNUS_RELEASES_TOKEN' . --exclude-dir=.git --exclude-dir=build --exclude-dir=managed_components
docs/REPORT-spec-phase6-order.md:27:Line 28 is blank and line 29 begins `permissions:`. There is no `workflow_dispatch`, no `push: branches`, no `pull_request`. A commit to `main` does not run this workflow; only a tag push matching `somnus-v*` does. The other premises checked before editing: the Phase 6 preamble is line 127 and reads exactly as the gate string; the five numbered steps are lines 129–133; `web-flasher/index.html` line 304 still fetches `matthewclaude/somnus-dial-releases/releases/latest` and lines 280, 282 and 288 still link into that repo; `release.yml` still has the old-repo `action-gh-release` step (lines 129–132, `repository: matthewclaude/somnus-dial-releases`, `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}`) and the old-repo `peaceiris` deploy (lines 187–195, `external_repository: matthewclaude/somnus-dial-releases`). Precondition confirmed on disk: `docs/REPORT-1.0.1-tag-push.md` records the dual Release and dual Pages, and `docs/REPORT-1.0.1-bench-gate.md` opens with **PASS**, both dated 2026-09-11.
docs/REPORT-spec-phase6-order.md:37:4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
docs/REPORT-spec-phase6-order.md:51:4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
docs/REPORT-spec-phase6-order.md:59:- **6b — after that release has deployed** and the new flasher page has been opened and confirmed to fetch `bedknob-for-somnus`: the secret half of item 4 (delete `SOMNUS_RELEASES_TOKEN`, revoke the PAT), then item 3 (the frozen-redirect README and the meta-refresh Pages index on the old repo), then item 5 (archive).
docs/REPORT-spec-phase6-order.md:80:1. **The "stays private" comments do not exist.** The instructions describe 6a as including a rewrite of "the workflow's stale 'stays private' comments". `grep -n -i private .github/workflows/release.yml` matches nothing. What the workflow does have is a top-of-file comment block (lines 10–21) saying the tag is pushed to "THIS repo, which is public" and that the Release and gh-pages site "are published here AND into the matthewclaude/somnus-dial-releases repo" using `SOMNUS_RELEASES_TOKEN`, and a changelog-extractor comment (lines 74–78) explaining there is no compare link because the notes are also published into the old repo. Both become stale at 6a. Rather than stop, or write a phrase that names comments the file does not contain, the 6a bullet describes them as "the top-of-file block and the changelog-extractor note that still describe dual-publishing into `somnus-dial-releases` and the token that does it". The intent — rewrite the comments that describe the old-repo arrangement — is preserved; the quoted phrase is not. If the owner meant a different comment, the 6a bullet is the line to correct.
docs/REPORT-1.0.1-beta.1-commit.md:124:          token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
docs/REPORT-1.0.1-beta.1-commit.md:139:          personal_token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
docs/SPEC-repo-consolidation.md:101:- line 129 `repository: matthewclaude/somnus-dial-releases` — the `softprops/action-gh-release@v2` step, authenticated with `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}` (line 130), a fine-grained PAT scoped to the old repo;
docs/SPEC-repo-consolidation.md:134:4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
docs/SPEC-repo-consolidation.md:142:- **6b — after that release has deployed** and the new flasher page has been opened and confirmed to fetch `bedknob-for-somnus`: the secret half of item 4 (delete `SOMNUS_RELEASES_TOKEN`, revoke the PAT), then item 3 (the frozen-redirect README and the meta-refresh Pages index on the old repo), then item 5 (archive).
docs/SPEC-ota-readiness.md:7:> `SOMNUS_RELEASES_TOKEN`, `somnus-v0.1.3` is current, and a real OTA
docs/REPORT-0.1.5-beta.5-tag-push.md:81:via `SOMNUS_RELEASES_TOKEN`. Confirm manually at:
docs/REPORT-spec-repo-consolidation.md:98:$ grep -n 'repository: matthewclaude/somnus-dial-releases\|external_repository\|matthewclaude.github.io/somnus-dial-releases\|contents: write\|permissions\|private\|keep_files\|SOMNUS_RELEASES_TOKEN' .github/workflows/release.yml
docs/REPORT-spec-repo-consolidation.md:101:16:# push) use the SOMNUS_RELEASES_TOKEN secret -- a fine-grained PAT scoped to
docs/REPORT-spec-repo-consolidation.md:108:130:          token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
docs/REPORT-spec-repo-consolidation.md:109:177:          personal_token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
```
Zero hits in `.github/workflows/release.yml` (before the edit it had three: lines 17, 132, 194). Every remaining hit is in `docs/` — two SPECs and four historical reports — which is expected and fine. `build/` and `managed_components/` were excluded from the search as generated trees; `.git` as history.

### (c) YAML parse

```
$ python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/release.yml')); print('yaml ok')"
ModuleNotFoundError: No module named 'yaml'
```
PyYAML is not installed in the system `python3`. Per the task, skipped; nothing was installed. The edits were pure block deletions and comment rewrites at unchanged indentation, applied as exact-string replacements that each matched exactly once, so the surviving structure is the pre-existing structure minus two list items. GitHub's own parse of the file is the real check and happens at the next tag push (§8).

### (d) Third-party notices filename at the repo root

```
$ ls -1 THIRD_PARTY_LICENSES.md
THIRD_PARTY_LICENSES.md
```

## 5. Diff stat and commit

```
$ git --no-optional-locks diff --stat
 .github/workflows/release.yml | 81 ++++++++++++-------------------------------
 web-flasher/index.html        |  9 ++---
 2 files changed, 27 insertions(+), 63 deletions(-)
```

Commit: `8ee93c41b7514834a6720d93f65afcc4a0002ca3` — `ci: Phase 6a — publish Release and flasher into this repo only (SPEC-repo-consolidation §5)`. Both files in the one commit. Not pushed: `main` is one commit ahead of `somnus/main` (`3da2dcd`) plus this report's commit.

```
$ git --no-optional-locks status --short --untracked-files=no
(no output, after the commit)
```

## 6. Deviations

1. **The `Source` link's position.** The task says to add it and match the markup; it does not say where in the row. It is first (`Source`, `Releases`, `Forked from Orion Dial`, `License`), because a stranger reading left to right meets the source before the releases built from it. Move it if a different order is wanted; no other line in the row changed.
2. **One extra clause in the surviving Pages comment.** Item 5 asked to keep the `keep_files` reasoning. The deleted old-repo step also carried the "NOT force_orphan" half of that reasoning inline, which would otherwise have left the file with no explanation of why `force_orphan` is absent. It was folded into the surviving comment as one parenthetical line. Nothing else was carried over from the deleted step.
3. **Search exclusions in (b).** `--exclude-dir=build --exclude-dir=managed_components --exclude-dir=.git` were added so the hit list is source and docs only. Nothing in those trees references the secret in any case; the exclusion only kept the scan fast.

Otherwise none: no version bump, no tag, no push, no secret or PAT touched, no manifest touched, nothing under `firmware/` touched, no step renamed.

## 7. What is not yet true, and why

`release.yml` triggers only on `push:` of tags matching `somnus-v*`. This commit changes nothing that is live:

- The Release for `1.0.2-beta.1` and later being published into `bedknob-for-somnus` only, with no old-repo Release, is verified by watching that tag's run and seeing exactly two publish steps in the step list, `Publish GitHub Release (this repo)` and `Deploy to gh-pages (this repo)`, with no `Publish GitHub Release` or `Deploy to gh-pages` bare-named step beside them.
- The workflow parsing at all under GitHub's YAML reader (§4(c) could not run locally) is verified the same way: the run starts.
- The flasher page at `https://matthewclaude.github.io/bedknob-for-somnus/` showing `Latest firmware: somnus-v…` from the new repo's `/releases/latest`, the `Source`/`Releases`/`License`/third-party links resolving (the notices link to `THIRD_PARTY_LICENSES.md` in particular, which is a 404 if the filename is wrong), is verified by opening that page after the deploy job has pushed `gh-pages`. Until then the live page is whatever the `1.0.1` run deployed, still fetching and linking the old repo.
- The release-notes footer pointing at the new flasher URL is verified in the `1.0.2-beta.1` Release body.
- `keep_files: true` preserving `firmware/latest/` when the beta deploy writes `firmware/beta/` is verified by fetching `https://matthewclaude.github.io/bedknob-for-somnus/firmware/latest/somnus-dial-merged.bin` after that deploy.
- The old repo, `matthewclaude/somnus-dial-releases`, stops receiving Releases from that tag on. Its `1.0.1` Release stays as the migration path. Its Pages site keeps serving the old flasher, still pointed at the old repo, until Phase 6b replaces it with a redirect.

Phase 6b (delete `SOMNUS_RELEASES_TOKEN`, revoke the PAT, freeze-redirect README and meta-refresh index on the old repo, archive) waits for that tag's run and a live check of the new flasher page, per the spec's ordering note.
