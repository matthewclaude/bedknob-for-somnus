# REPORT — somnus-v1.0.1-beta.1 tag, push, dual Release and dual Pages verification

Date: 2026-09-10. Session: https://claude.ai/code/session_01BSmB4uqCDcqSocfrVPQG4N

Block 2 of 2 for `1.0.1-beta.1` per `docs/SPEC-repo-consolidation.md`. Block 1 is `REPORT-1.0.1-beta.1-commit.md` (commit `2716be9`). No code changes; nothing flashed.

## Verdict

**DONE** — `somnus-v1.0.1-beta.1` tagged at `2716be9`, pushed to the `somnus` remote only (`matthewclaude/bedknob-for-somnus`), release run 34522218461 and ci run 34522219038 both succeeded, the prerelease exists in BOTH repos with byte-identical assets (same SHA-256 digests), `/releases/latest` is still `somnus-v1.0.0` on the old repo and 404 on the new one, the old-repo Pages URL serves the merged image with HTTP 200, and `gh-pages` now exists on `bedknob-for-somnus` carrying the same merged image.

**Tag-to-Release window end (§7 item 4): 2026-09-10T19:48:23Z** — the moment `gh run watch` on the release run returned with `--exit-status` 0. Both Releases were confirmed present after that instant (step 4 below). The bench dial may be touched from that time on.

## Gate (all seven passed)

```
--1 rev-parse--
$ git rev-parse --short HEAD
2716be9
$ git rev-parse --abbrev-ref HEAD
main
--2 sed 21p--
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.1-beta.1")
--3 tag -l--
$ git tag -l 'somnus-v1.0.1*'
(empty)
--4 status--
$ git --no-optional-locks status --short --untracked-files=no
(empty)
--5 push url--
$ git remote get-url --push origin
no_push
--6 log--
$ git log --oneline somnus/main..HEAD
2716be9 release: 1.0.1-beta.1 — OTA client polls bedknob-for-somnus; dual-publish releases
--7 gh auth--
$ gh auth status
github.com
  ✓ Logged in to github.com account matthewclaude (keyring)
  - Active account: true
  - Git operations protocol: https
  - Token: gho_************************************
  - Token scopes: 'gist', 'read:org', 'repo', 'workflow'
exit=0
```

## Step 1 — tag

```
$ git tag -a somnus-v1.0.1-beta.1 -m "somnus-v1.0.1-beta.1" 2716be9
tag ok
```

## Step 2 — push main, push tag (somnus remote only)

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   ba25b17..2716be9  main -> main
exit=0

$ git push somnus somnus-v1.0.1-beta.1
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.1-beta.1 -> somnus-v1.0.1-beta.1
exit=0

$ date -u
2026-09-10T19:44:49Z
```

## Step 3 — runs

### Run list (taken ~15 s after the push)

```
$ gh run list --repo matthewclaude/bedknob-for-somnus --limit 4
in_progress		release: 1.0.1-beta.1 — OTA client polls bedknob-for-somnus; dual-pub…	ci	main	push	34522219038	19s	2026-09-10T19:44:51Z
in_progress		release: 1.0.1-beta.1 — OTA client polls bedknob-for-somnus; dual-pub…	release	somnus-v1.0.1-beta.1	push	34522218461	19s	2026-09-10T19:44:51Z
completed	success	docs: spec-repo-consolidation report — push and CI addendum	ci	main	push	34521105500	3m11s	2026-09-10T19:33:27Z
completed	success	docs: SPEC-repo-consolidation — gh-pages branch is upstream's, not th…	ci	main	push	34520740644	3m13s	2026-09-10T19:29:55Z
```

| Workflow | Trigger | Run id | Conclusion |
|---|---|---|---|
| `ci.yml` | push of `main` (`2716be9`) | 34522219038 | success |
| `release.yml` | push of tag `somnus-v1.0.1-beta.1` | 34522218461 | success |

### Release run watch (final frame; the 3-second refresh frames that preceded it are identical in shape and were not copied)

```
$ gh run watch 34522218461 --repo matthewclaude/bedknob-for-somnus --exit-status
✓ somnus-v1.0.1-beta.1 release · 34522218461
Triggered via push about 3 minutes ago

JOBS
✓ build-and-release in 3m16s (ID 103022268392)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release
  ✓ Publish GitHub Release (this repo)
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
✓ deploy-pages in 8s (ID 103023330122)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  ✓ Deploy to gh-pages
  ✓ Deploy to gh-pages (this repo)
  ✓ Post Run actions/checkout@v4
  ✓ Complete job

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

release watch exit=0
2026-09-10T19:48:23Z
```

### CI run watch

```
$ gh run watch 34522219038 --repo matthewclaude/bedknob-for-somnus --exit-status
Run ci (34522219038) has already completed with 'success'
ci watch exit=0
2026-09-10T19:48:32Z
```

### Per-job timings

```
$ gh run view 34522218461 --repo matthewclaude/bedknob-for-somnus --json databaseId,conclusion,status,createdAt,updatedAt,jobs --jq '{id:.databaseId,status,conclusion,createdAt,updatedAt,jobs:[.jobs[]|{name,conclusion,startedAt,completedAt}]}'
{"conclusion":"success","createdAt":"2026-09-10T19:44:51Z","id":34522218461,"jobs":[{"completedAt":"2026-09-10T19:48:10Z","conclusion":"success","name":"build-and-release","startedAt":"2026-09-10T19:44:54Z"},{"completedAt":"2026-09-10T19:48:21Z","conclusion":"success","name":"deploy-pages","startedAt":"2026-09-10T19:48:13Z"}],"status":"completed","updatedAt":"2026-09-10T19:48:22Z"}

$ gh run view 34522219038 --repo matthewclaude/bedknob-for-somnus --json databaseId,conclusion,status,createdAt,updatedAt,jobs --jq '{id:.databaseId,status,conclusion,createdAt,updatedAt,jobs:[.jobs[]|{name,conclusion,startedAt,completedAt}]}'
{"conclusion":"success","createdAt":"2026-09-10T19:44:51Z","id":34522219038,"jobs":[{"completedAt":"2026-09-10T19:47:54Z","conclusion":"success","name":"build","startedAt":"2026-09-10T19:44:54Z"}],"status":"completed","updatedAt":"2026-09-10T19:47:55Z"}
```

| Run | Job | Started (UTC) | Completed (UTC) | Duration |
|---|---|---|---|---|
| release 34522218461 | build-and-release | 19:44:54 | 19:48:10 | 3m16s |
| release 34522218461 | deploy-pages | 19:48:13 | 19:48:21 | 8s |
| ci 34522219038 | build | 19:44:54 | 19:47:54 | 3m00s |

Tag push to release run complete: 19:44:49 → 19:48:22, 3m33s.

## Step 4 — Release exists in BOTH repos

```
$ gh release view somnus-v1.0.1-beta.1 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/555704814","contentType":"application/octet-stream","createdAt":"2026-09-10T19:48:04Z","digest":"sha256:2ef03a22553d96e3ba7bcee5051a0874bcf2448ec5ac63be3d003077e785fabe","downloadCount":0,"id":"RA_kwDOULeAcc4hH2Hu","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-10T19:48:05Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1-beta.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/555704822","contentType":"application/octet-stream","createdAt":"2026-09-10T19:48:04Z","digest":"sha256:0ac8205fe49eaeeeab51817685257b8502fe821b86d9c93b45187a5bd4a4524f","downloadCount":0,"id":"RA_kwDOULeAcc4hH2H2","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-10T19:48:05Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1-beta.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":true,"publishedAt":"2026-09-10T19:48:03Z","tagName":"somnus-v1.0.1-beta.1"}

$ gh release view somnus-v1.0.1-beta.1 --repo matthewclaude/bedknob-for-somnus --json tagName,isPrerelease,isDraft,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/555704889","contentType":"application/octet-stream","createdAt":"2026-09-10T19:48:07Z","digest":"sha256:2ef03a22553d96e3ba7bcee5051a0874bcf2448ec5ac63be3d003077e785fabe","downloadCount":0,"id":"RA_kwDOUCLPIM4hH2I5","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-10T19:48:07Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.1-beta.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/555704887","contentType":"application/octet-stream","createdAt":"2026-09-10T19:48:07Z","digest":"sha256:0ac8205fe49eaeeeab51817685257b8502fe821b86d9c93b45187a5bd4a4524f","downloadCount":0,"id":"RA_kwDOUCLPIM4hH2I3","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-10T19:48:07Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.1-beta.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":true,"publishedAt":"2026-09-10T19:48:06Z","tagName":"somnus-v1.0.1-beta.1"}
```

Both: `isPrerelease` true, `isDraft` false, two assets each.

| Asset | somnus-dial-releases | bedknob-for-somnus | SHA-256 |
|---|---|---|---|
| `somnus-dial.bin` | 1612080 bytes | 1612080 bytes | `0ac8205f…4524f` (identical) |
| `somnus-dial-merged.bin` | 1743152 bytes | 1743152 bytes | `2ef03a22…5fabe` (identical) |

Sizes identical across the two repos; the API also reports identical digests, so the two copies are the same bytes, not merely the same length. Old repo published at 19:48:03Z, new repo at 19:48:06Z — the workflow's two publish steps, in order.

## Step 5 — /releases/latest on BOTH repos

```
$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
somnus-v1.0.0
exit=0

$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest
{"message":"Not Found","documentation_url":"https://docs.github.com/rest/releases/releases#get-the-latest-release","status":"404"}gh: Not Found (HTTP 404)
exit=1
```

Old repo: `somnus-v1.0.0` remains latest — the prerelease did not displace it. New repo: HTTP 404, expected — no stable Release there yet. This 404 is exactly what a repointed dial with Beta builds OFF sees until `1.0.1` stable is published there; §3 of the spec describes that as "up to date" behaviour on the dial.

## Step 6 — published body vs CHANGELOG section (old repo)

```
$ gh release view somnus-v1.0.1-beta.1 --repo matthewclaude/somnus-dial-releases --json body --jq .body > /tmp/body.txt
$ awk -v ver=1.0.1-beta.1 '
    $0 == "## " ver || index($0, "## " ver " ") == 1 { inside = 1; next }
    inside && /^## / { exit }
    inside { print }
  ' CHANGELOG.md | sed -e '/./,$!d' | sed -e :a -e '/^\n*$/{$d;N;};/\n$/ba' > changelog-section.txt
$ diff changelog-section.txt /tmp/body.txt
12a13,17
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
diff exit=1
```

Only the footer lines differ (appended after line 12 of the CHANGELOG section): the two-line install note and the required-notice line. The CHANGELOG text itself is byte-identical in the published body.

## Step 7 — Pages deploys

```
$ gh api repos/matthewclaude/bedknob-for-somnus/branches --jq '.[].name'
gh-pages
main
exit=0

$ git ls-remote somnus refs/heads/gh-pages
64e090e305438589f2c3d2a578d7d821eb7c8854	refs/heads/gh-pages
exit=0

$ gh api "repos/matthewclaude/bedknob-for-somnus/contents/firmware/beta/somnus-dial-merged.bin?ref=gh-pages" --jq .size
1743152
exit=0

$ gh api "repos/matthewclaude/somnus-dial-releases/contents/firmware/beta/somnus-dial-merged.bin?ref=gh-pages" --jq .size
1743152
exit=0

$ curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/beta/somnus-dial-merged.bin | head -3
HTTP/2 200 
server: GitHub.com
content-type: application/octet-stream
```

`gh-pages` now exists on `bedknob-for-somnus` (created by this run's "Deploy to gh-pages (this repo)" step, as the spec's correction addendum predicted). Both `gh-pages` copies of the merged image are 1743152 bytes, equal to the step-4 merged asset. The old-repo Pages URL returned 200 on the first try, no retry needed. The new repo's github.io URL was not curled and Pages was not enabled there — owner step (a) below.

## Step 8 — tag object

```
$ git show -s --format='%H %d' somnus-v1.0.1-beta.1^{commit}
2716be909dff3ebad031dc40ab3564556089b63c  (HEAD -> main, tag: somnus-v1.0.1-beta.1, somnus/main, somnus/HEAD)

$ git rev-parse somnus-v1.0.1-beta.1
f78b7b4fa0ffb181c11b21475340dab930849cfb

$ git rev-parse 2716be9
2716be909dff3ebad031dc40ab3564556089b63c
```

## SHAs

- Commit `2716be9` = `2716be909dff3ebad031dc40ab3564556089b63c`
- Annotated tag object `somnus-v1.0.1-beta.1` = `f78b7b4fa0ffb181c11b21475340dab930849cfb` → peels to the commit above
- `gh-pages` tip on `bedknob-for-somnus` = `64e090e305438589f2c3d2a578d7d821eb7c8854`

## Deviations

- The release-run `gh run watch` output is reproduced as its final frame only. The tool reprints the whole job tree every 3 seconds; the ~45 intermediate frames were captured to a scratch file but not copied here, since each is a strict prefix-of-progress of the final one. Nothing else was trimmed.
- Step 6's extractor was run outside `/tmp` (scratch dir) for the CHANGELOG side; the published body went to `/tmp/body.txt` as instructed.

## False premises

None. Every expected value in the task matched: gate values, both run conclusions, prerelease/draft flags, two assets per repo, identical sizes, `somnus-v1.0.0` latest on the old repo, 404 latest on the new one, footer-only body diff, `gh-pages` present on both repos with matching sizes, Pages 200, tag peels to `2716be9`.

## Hardware gate — owner

Copied verbatim from `docs/SPEC-repo-consolidation.md` §7. The tag-to-Release window (item 4) closed at **2026-09-10T19:48:23Z**; both Releases were confirmed present after that (step 4 above).

- [ ] 1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo (the serial log shows the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50` and the Release fetch for `somnus-v1.0.1-beta.1` returning 200) → installs → reboots showing `1.0.1-beta.1` under Menu → About.
- [ ] 2. Same dial → Check for updates → the serial log must show the request going to `api.github.com/repos/matthewclaude/bedknob-for-somnus/…` and the result `latest 1.0.1-beta.1, running 1.0.1-beta.1 -- up to date`.
- [ ] 3. **A serial capture of item 2 is required** and goes into the bring-up record. A silent "up to date" against the wrong host is indistinguishable from success on the screen — that was the failure shape of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`: eight `ota:` lines, every one "up to date", while the newer beta sat unseen on the server). Capture with the cat-based serial method from the bring-up notes, not `idf.py monitor`.
- [ ] 4. Not inside the ~6-minute tag-to-Release window of §3.2(b). Confirm with `gh run watch` that the release run for the tag has completed **and** that both Releases exist (`gh release view somnus-v1.0.1-beta.1 --repo <each>`) before touching the dial.
- [ ] 5. Rollback, if the repointed build cannot see the new repo (item 2 fails, or ends `OTA_FAILED`): wire-flash the `1.0.0` merged image from the **old** flasher page, which stays live throughout the beta and the cut-over. Five minutes, one unit. Then the beta is withdrawn (delete the two Releases; the tag stays, which is harmless per §3.2(a) once a newer tag with a Release exists) and this spec gets a §9 saying what was wrong.

Owner-only steps, in order:

- [ ] (a) Enable GitHub Pages on `bedknob-for-somnus`: Settings → Pages → source "Deploy from a branch" → branch `gh-pages` / folder `/ (root)`. Only possible now that the branch exists (created by run 34522218461 at 19:48:21Z). Not done in this block by design.
- [ ] (b) Run §7 items 1–5 on the bench dial. The tag-to-Release window ended at 2026-09-10T19:48:23Z (release run 34522218461 complete, both Releases verified in step 4), so any time after that is outside it. Capture item 2's serial log with the cat-based method and add it to the bring-up record.
