# REPORT: somnus-v1.0.1 tag, push, dual Release and dual Pages verification — 2026-09-11

**DONE** — `somnus-v1.0.1` tagged at `f0aec2998affc513026a68f16109a38b8832409a` and pushed to `somnus` only (main `800f0cd..f0aec29` first, then the tag). Release run 34633631645 (build-and-release 3m15s, deploy-pages 12s) and ci run 34633630466 both succeeded. The stable Release exists in both `somnus-dial-releases` and `bedknob-for-somnus`, `isDraft` false and `isPrerelease` **false** in both, two assets each, byte-identical (same names, sizes and SHA-256 digests: **yes**). `/releases/latest` answers `somnus-v1.0.1` on both repos; both Pages sites serve the merged image with HTTP 200 and a content-length equal to the Release asset's size. Published bodies on both repos equal the CHANGELOG section plus the workflow's footer and nothing else. Tag-to-Release window closed 18:34:09Z. Not flashed; the dial was not touched; the §7.1 bench pass is the next step and was not attempted.

## Gate (all four passed)

```
$ git log -1 --format=%H
f0aec2998affc513026a68f16109a38b8832409a
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1")
$ git tag -l 'somnus-v1.0.1'
$ git --no-optional-locks status --short --untracked-files=no
$
```

Checks 3 and 4 printed nothing. HEAD is the "docs: 1.0.1 commit report and its REPORTS.md line" commit. Also checked before pushing: `git remote -v` shows `origin` push URL `no_push` and `somnus` = `git@github.com:matthewclaude/bedknob-for-somnus.git`; `git ls-remote --heads somnus main` was `800f0cd` (two commits behind local); `gh auth status` active account `matthewclaude`.

## Step 1 — annotation style

```
$ git for-each-ref refs/tags/somnus-v1.0.0 --format='%(contents)'
Bedknob for Somnus 1.0.0 — the 0.1.6 build renumbered; v1 scope complete

```

(For contrast, `somnus-v1.0.1-beta.1`'s annotation is just `somnus-v1.0.1-beta.1`; the stable tag follows 1.0.0's shape.)

This tag's annotation:

```
Bedknob for Somnus 1.0.1 — the 1.0.1-beta.1 build renumbered; OTA client polls bedknob-for-somnus
```

```
$ git tag -a somnus-v1.0.1 -m '…' f0aec2998affc513026a68f16109a38b8832409a
$ git for-each-ref refs/tags/somnus-v1.0.1 --format='%(objecttype) %(*objectname) %(contents)'
tag f0aec2998affc513026a68f16109a38b8832409a Bedknob for Somnus 1.0.1 — the 1.0.1-beta.1 build renumbered; OTA client polls bedknob-for-somnus
```

## Steps 2–3 — pushes (to `somnus` only; `origin` never touched)

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   800f0cd..f0aec29  main -> main
$ git push somnus somnus-v1.0.1
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.1 -> somnus-v1.0.1
```

Tag pushed at 2026-09-11T18:30:16Z.

## Step 4 — workflow runs (`--repo matthewclaude/bedknob-for-somnus`)

| run id | name | trigger | conclusion | created → updated | duration |
|---|---|---|---|---|---|
| 34633631645 | release | push of tag `somnus-v1.0.1` | **success** | 18:30:18Z → 18:34:27Z | 4m09s total; build-and-release 18:30:56Z→18:34:11Z (3m15s), deploy-pages 18:34:14Z→18:34:26Z (12s) |
| 34633630466 | ci | push to `main` | **success** | 18:30:17Z → 18:33:25Z | 3m08s; build job 18:30:20Z→18:33:25Z (3m05s) |

Release run steps, all ✓: Verify tag matches PROJECT_VER · Extract release notes from CHANGELOG.md · Classify release channel · Build firmware and merge flashable image · Upload merged image for the Pages job · Publish GitHub Release · Publish GitHub Release (this repo) · deploy-pages: Download merged image · Channel directory · Assemble Pages site · Deploy to gh-pages · Deploy to gh-pages (this repo). One annotation, the standard "Node.js 20 is deprecated" notice from `actions/checkout@v4`, `upload-artifact@v4` and `softprops/action-gh-release@v2`, as on every previous run.

## Step 5a — Release in both repos

```
$ gh release view somnus-v1.0.1 --repo matthewclaude/somnus-dial-releases --json tagName,isDraft,isPrerelease,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/557784998","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:04Z","digest":"sha256:90b27f3b00d0ffa1d9f382b6f92eb587ae4fb8e53d8ebda44c35a5c824b9e00d","downloadCount":0,"id":"RA_kwDOULeAcc4hPx-m","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-11T18:34:05Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/557785000","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:04Z","digest":"sha256:c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782","downloadCount":0,"id":"RA_kwDOULeAcc4hPx-o","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-11T18:34:04Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-11T18:34:05Z","tagName":"somnus-v1.0.1"}
$ gh release view somnus-v1.0.1 --repo matthewclaude/bedknob-for-somnus --json tagName,isDraft,isPrerelease,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/557785099","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:09Z","digest":"sha256:90b27f3b00d0ffa1d9f382b6f92eb587ae4fb8e53d8ebda44c35a5c824b9e00d","downloadCount":0,"id":"RA_kwDOUCLPIM4hPyAL","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-11T18:34:09Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/557785100","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:09Z","digest":"sha256:c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782","downloadCount":0,"id":"RA_kwDOUCLPIM4hPyAM","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-11T18:34:09Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-11T18:34:09Z","tagName":"somnus-v1.0.1"}
```

Both: `isDraft` false, `isPrerelease` **false** — a stable release, not a prerelease. Old repo published 18:34:05Z, new repo 18:34:09Z (old-repo step ran first, by design).

Asset comparison, `name size digest` sorted, old repo then new repo, then `diff`:

```
somnus-dial-merged.bin 1743152 sha256:90b27f3b00d0ffa1d9f382b6f92eb587ae4fb8e53d8ebda44c35a5c824b9e00d
somnus-dial.bin 1612080 sha256:c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782
---
somnus-dial-merged.bin 1743152 sha256:90b27f3b00d0ffa1d9f382b6f92eb587ae4fb8e53d8ebda44c35a5c824b9e00d
somnus-dial.bin 1612080 sha256:c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782
--- diff (empty = identical) ---
IDENTICAL
```

**Do the two asset lists match exactly in names, sizes and SHA-256 digests? Yes.**

## Step 5b — `/releases/latest`

```
$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
somnus-v1.0.1
$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq .tag_name
somnus-v1.0.1
```

Both repos answer `somnus-v1.0.1`. The new repo answering with a release instead of 404 is expected and is the point of this release. **From this moment the Beta-builds-off 404 check of §7 item 2 is no longer available**, exactly as `docs/SPEC-repo-consolidation.md` §7.1 states: both repos now serve identical content and no runtime check on the dial can tell them apart. Repo identity for stable was verified on the built binary in `docs/REPORT-1.0.1-commit.md` (3 / 0 / 0).

## Step 5c — Pages

HEAD requests, first attempt, no retry needed:

```
=== HEAD https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin ===
HTTP/2 200 
last-modified: Fri, 11 Sep 2026 18:34:43 GMT
etag: "6aa449c3-1a9930"
content-length: 1743152
=== HEAD https://matthewclaude.github.io/bedknob-for-somnus/firmware/latest/somnus-dial-merged.bin ===
HTTP/2 200 
last-modified: Fri, 11 Sep 2026 18:34:44 GMT
etag: "6aa449c4-1a9930"
content-length: 1743152
```

Both 200; both content-length 1743152, which equals the `somnus-dial-merged.bin` Release asset size (1743152) on both repos. Yes, the byte counts match.

## Step 5d — published body vs CHANGELOG

Local extract (release.yml lines 63–67 awk plus the line-69 blank-line trim, `ver=1.0.1`):

```
The dial now checks for firmware updates at the project's own repository,
`matthewclaude/bedknob-for-somnus`, where the source code also lives.
Nothing else changes — same screens, same behaviour, same pad control.
This is `1.0.1-beta.1` graduated to a stable release.

A dial on 1.0.0 is offered this update as usual; once it is installed,
updates come from the new location automatically. Flashing a dial from
the browser flasher continues to work exactly as before.

Releases are being published to both the old and the new locations during
this changeover, and a dial that never installs 1.0.1 will keep looking at
the old location — so update it while both are still being published.

Internal: the `1.0.1-beta.1` build renumbered; no code change beyond the
version string.
```

```
$ diff local-body.txt <(gh release view somnus-v1.0.1 --repo matthewclaude/somnus-dial-releases --json body --jq .body)
15a16,20
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
$ diff local-body.txt <(gh release view somnus-v1.0.1 --repo matthewclaude/bedknob-for-somnus --json body --jq .body)
15a16,20
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
$ diff old-body.txt new-body.txt
IDENTICAL
```

The only difference on each repo is the five appended lines: a blank line, the "Dials already running…" line, the browser-flasher line, a blank line, and the Required Notice line. Those are exactly what `release.yml` lines 85–88 append as the standard footer. **No difference beyond the footer; no finding.** The two published bodies are identical to each other. (The footer still links the old flasher page, per §4(c) of the spec: the flasher repoint is Phase 6.)

## Step 6

Nothing flashed. The dial was not touched. The §7.1 bench pass was not started.

## Deviations

No deviations.

For the record, two things done beyond the listed steps, both read-only: `git remote -v`, `git ls-remote --heads somnus main` and `gh auth status` were checked before pushing, and a 15-second wait preceded `gh run list` so both runs had been created. Steps 5a–5d and the ci watch were run concurrently once the release run had succeeded; step 5 was not started before that.

## Not verifiable without hardware — next step

The §7.1 bench pass: bench dial on `1.0.1-beta.1`, Beta builds **off**, Menu → Update → Check for updates → offered `1.0.1` → installs → reboots showing `1.0.1` under Menu → About, with a cat-based serial capture (not `idf.py monitor`) into `bench-logs/`. The tag-to-Release window closed at 18:34:09Z and both Pages deploys landed by 18:34:44Z, so §7.1's timing caution is satisfied from now on. Rollback if it misbehaves: wire-flash from the old flasher page, which stays live until Phase 6.
