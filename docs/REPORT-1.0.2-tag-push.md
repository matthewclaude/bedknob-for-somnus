# REPORT — somnus-v1.0.2 stable release, block 2 of 2 (tag, push, verify)

**DONE** — annotated tag `somnus-v1.0.2` on `5512537`, pushed to `somnus`
only; release and ci runs green; Release published as stable (not a
prerelease) with both assets; `releases/latest` now points at
`somnus-v1.0.2`; the flasher page serves the 1.0.2 image and the beta
channel is untouched.

Date: 2026-09-17.

## Tag-push-to-publishedAt window (UTC)

| Event | Time (UTC) |
|---|---|
| `git push somnus somnus-v1.0.2` returned (`date -u`) | 2026-09-17 23:12:21 |
| release run created | 2026-09-17 23:12:22 |
| Release assets uploaded | 2026-09-17 23:15:26 |
| Release `publishedAt` | 2026-09-17 23:15:27 |
| release run `updatedAt` (deploy-pages done) | 2026-09-17 23:15:44 |
| Pages image `last-modified` | 2026-09-17 23:15:59 |

Tag push to `publishedAt`: **3 min 06 s**.

## Gate (run before tagging)

All four passed.

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2")

$ git --no-optional-locks tag -l 'somnus-v1.0.2'
(empty)

$ git --no-optional-locks status --short --untracked-files=no
(empty)

$ git --no-optional-locks log --oneline somnus/main..HEAD
5512537 release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY poll cadence 300 s goes stable

$ git --no-optional-locks remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)

$ git rev-parse HEAD
5512537f7b34c4d8919471905709b86cf808d18c

$ git --no-optional-locks status --short
?? docs/REPORT-1.0.2-commit.md
?? docs/REPORT-docs-index-commit.md
```

The two untracked report files are expected and go in the next docs commit.

## Step 1 — annotated tag

```
$ git tag -a somnus-v1.0.2 -m 'somnus-v1.0.2 — STANDBY poll cadence 300 s, stable'
(no output)

$ git --no-optional-locks show --no-patch --format='%H %s' 'somnus-v1.0.2^{}'
5512537f7b34c4d8919471905709b86cf808d18c release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY poll cadence 300 s goes stable

$ git --no-optional-locks tag -l -n1 somnus-v1.0.2
somnus-v1.0.2   somnus-v1.0.2 — STANDBY poll cadence 300 s, stable
```

## Step 2 — push main

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   ff42f6d..5512537  main -> main
```

## Step 3 — push tag

```
$ git push somnus somnus-v1.0.2
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.2 -> somnus-v1.0.2

$ date -u
Thu Sep 17 23:12:21 UTC 2026
```

## Step 4 — tag on the right remote and on no other

```
$ git ls-remote --tags https://github.com/matthewclaude/bedknob-for-somnus.git 'somnus-v1.0.2*'
0c1dbb15d7288b410ed4b98e94ea8d4b2040d2b2	refs/tags/somnus-v1.0.2
5512537f7b34c4d8919471905709b86cf808d18c	refs/tags/somnus-v1.0.2^{}
45ce226a19bd538ae4b4a44e5c50dc3148be9f9f	refs/tags/somnus-v1.0.2-beta.1
00bd94d263e36c643cfa27756f3d0463905bad86	refs/tags/somnus-v1.0.2-beta.1^{}

$ git ls-remote --tags https://github.com/matthewclaude/somnus-dial-releases.git 'somnus-v1.0.2*'
(empty, exit 0)
```

## Step 5 — CI

```
$ gh run list --repo matthewclaude/bedknob-for-somnus --limit 5
in_progress		release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY p…	release	somnus-v1.0.2	push	35285754918	7s	2026-09-17T23:12:22Z
in_progress		release: somnus-v1.0.2 — the 1.0.2-beta.1 build renumbered; STANDBY p…	ci	main	push	35285753616	8s	2026-09-17T23:12:21Z
completed	success	docs: MCP integration and spec-location reports, and their REPORTS.md…	ci	main	push	34900274028	3m9s	2026-09-14T21:42:19Z
completed	success	docs: Phase 7 report, its REPORTS.md line, and a stale "ground truth"…	ci	main	push	34779338813	3m12s	2026-09-13T19:59:18Z
completed	success	docs: Phase 7 sweep - point docs at the single repo, note the archive…	ci	main	push	34778960227	3m10s	2026-09-13T19:51:25Z

$ gh run watch 35285754918 --repo matthewclaude/bedknob-for-somnus --exit-status

JOBS
✓ build-and-release in 3m6s (ID 105417579705)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Verify tag matches PROJECT_VER
  ✓ Extract release notes from CHANGELOG.md
  ✓ Classify release channel
  ✓ Build firmware and merge flashable image (firmware/dial-idf)
  ✓ Upload merged image for the Pages job
  ✓ Publish GitHub Release (this repo)
  ✓ Post Run actions/checkout@v4
  ✓ Complete job
✓ deploy-pages in 11s (ID 105418346499)
  ✓ Set up job
  ✓ Run actions/checkout@v4
  ✓ Download merged image
  ✓ Channel directory
  ✓ Assemble Pages site
  ✓ Deploy to gh-pages (this repo)
  ✓ Post Run actions/checkout@v4
  ✓ Complete job

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

- "The ubuntu-latest label will migrate to Ubuntu 26 beginning October 19, 2026. For more information, see https://github.com/actions/runner-images/issues/14748"
build-and-release: .github#1

$ gh run watch 35285753616 --repo matthewclaude/bedknob-for-somnus --exit-status
Run ci (35285753616) has already completed with 'success'

$ gh run view 35285754918 --repo matthewclaude/bedknob-for-somnus --json databaseId,name,conclusion,headBranch,headSha,createdAt,updatedAt
{"conclusion":"success","createdAt":"2026-09-17T23:12:22Z","databaseId":35285754918,"headBranch":"somnus-v1.0.2","headSha":"5512537f7b34c4d8919471905709b86cf808d18c","name":"release","updatedAt":"2026-09-17T23:15:44Z"}

$ gh run view 35285753616 --repo matthewclaude/bedknob-for-somnus --json databaseId,name,conclusion,headBranch,headSha,createdAt,updatedAt
{"conclusion":"success","createdAt":"2026-09-17T23:12:21Z","databaseId":35285753616,"headBranch":"main","headSha":"5512537f7b34c4d8919471905709b86cf808d18c","name":"ci","updatedAt":"2026-09-17T23:15:32Z"}
```

| Run | ID | Ref | Conclusion |
|---|---|---|---|
| release | 35285754918 | somnus-v1.0.2 | success |
| ci | 35285753616 | main | success |

The two annotations are GitHub runner deprecation notices, not failures;
the same two appeared on the 1.0.2-beta.1 run.

## Step 6 — the Release

```
$ gh release view somnus-v1.0.2 --repo matthewclaude/bedknob-for-somnus --json tagName,isDraft,isPrerelease,publishedAt,body,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/571348957","contentType":"application/octet-stream","createdAt":"2026-09-17T23:15:26Z","digest":"sha256:e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462","downloadCount":0,"id":"RA_kwDOUCLPIM4iDhfd","label":"","name":"somnus-dial-merged.bin","size":1743392,"state":"uploaded","updatedAt":"2026-09-17T23:15:27Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/571348956","contentType":"application/octet-stream","createdAt":"2026-09-17T23:15:26Z","digest":"sha256:58e10172386fb5b8942789b25d05256b73fc68af397adcd7258f4a550f15cff9","downloadCount":0,"id":"RA_kwDOUCLPIM4iDhfc","label":"","name":"somnus-dial.bin","size":1612320,"state":"uploaded","updatedAt":"2026-09-17T23:15:27Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2/somnus-dial.bin"}],"body":"While the screen is off and the dial has gone to standby, it now asks the\npad for its state once every five minutes instead of every ten seconds.\nTouching the knob still reads the pad immediately, so the face is current\nthe moment you look at it. Nothing about the screens or pad control\nchanges. This is `1.0.2-beta.1` graduated to a stable release after a\nfull night in use and an over-the-air install of its own.\n\nInternal: the `1.0.2-beta.1` build renumbered; no code change beyond the\nversion string.\n\nDials already running the firmware pick this up on their own — see Menu → Update.\nFor a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.\n\nRequired Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)","isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-17T23:15:27Z","tagName":"somnus-v1.0.2"}

$ gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq .tag_name
somnus-v1.0.2

$ gh release view somnus-v1.0.2 --repo matthewclaude/somnus-dial-releases
release not found
(exit 1)
```

Checks against expectation:

| Field | Expected | Actual |
|---|---|---|
| `tagName` | somnus-v1.0.2 | somnus-v1.0.2 |
| `isDraft` | false | false |
| `isPrerelease` | **false** | **false** |
| assets | 2 | 2 (`somnus-dial.bin`, `somnus-dial-merged.bin`) |
| `releases/latest` | somnus-v1.0.2 | somnus-v1.0.2 (was somnus-v1.0.1 before this block) |

Body: the first nine lines of the Release body are byte-identical to
`CHANGELOG.md` lines 27–35 (the `## 1.0.2 — 2026-09-17` section body;
verified with `diff`, no output). After those nine lines the workflow's
"Extract release notes" step appends its standard footer (OTA note,
flasher link, required copyright notice) from `release.yml` lines 78–81.
The `somnus-v1.0.1` Release carries the same footer, so this is the
established Release shape, not a change.

Assets:

| Asset | Size (bytes) | sha256 | downloadCount |
|---|---|---|---|
| `somnus-dial.bin` | 1612320 | 58e10172386fb5b8942789b25d05256b73fc68af397adcd7258f4a550f15cff9 | 0 |
| `somnus-dial-merged.bin` | 1743392 | e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462 | 0 |

Both asset sizes are identical to the 1.0.2-beta.1 assets (1612320 and
1743392), as expected for a build whose only difference is the version
string; the digests differ.

## Step 7 — flasher page and manifests

```
$ curl -sI https://matthewclaude.github.io/bedknob-for-somnus/ | head -1
HTTP/2 200 

$ curl -s https://matthewclaude.github.io/bedknob-for-somnus/manifest.json
{
  "name": "Bedknob for Somnus",
  "version": "latest",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "firmware/latest/somnus-dial-merged.bin", "offset": 0 }
      ]
    }
  ]
}

$ curl -sI https://matthewclaude.github.io/bedknob-for-somnus/firmware/latest/somnus-dial-merged.bin
HTTP/2 200 
last-modified: Thu, 17 Sep 2026 23:15:59 GMT
etag: "6aac74af-1a9a20"
content-length: 1743392

$ curl -sI https://matthewclaude.github.io/bedknob-for-somnus/manifest-beta.json | head -1
HTTP/2 200 

$ curl -s https://matthewclaude.github.io/bedknob-for-somnus/manifest-beta.json
{
  "name": "Bedknob for Somnus (beta)",
  "version": "beta",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "firmware/beta/somnus-dial-merged.bin", "offset": 0 }
      ]
    }
  ]
}

$ curl -sI https://matthewclaude.github.io/bedknob-for-somnus/firmware/beta/somnus-dial-merged.bin
HTTP/2 200 
last-modified: Thu, 17 Sep 2026 23:15:59 GMT
content-length: 1743392
```

The manifest's `version` field is the literal string `latest`, not
`1.0.2`. That is the committed source (`web-flasher/manifest.json` line 3
says `"version": "latest"`, and the beta manifest says `"beta"`); the
workflow copies the file as-is, so the manifest never carries a version
number. The task's "expect 1.0.2" therefore cannot be met by the manifest
text itself. The image the manifest points at was verified instead:

```
$ curl -s -o latest-merged.bin https://matthewclaude.github.io/bedknob-for-somnus/firmware/latest/somnus-dial-merged.bin
$ shasum -a 256 latest-merged.bin
e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462  latest-merged.bin
(1743392 bytes — equal to the somnus-v1.0.2 Release asset digest and size)

$ strings latest-merged.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$' | sort | uniq -c
   1 1.0.2
```

Beta channel not disturbed by the stable deploy:

```
$ curl -s -o beta-merged.bin https://matthewclaude.github.io/bedknob-for-somnus/firmware/beta/somnus-dial-merged.bin
$ shasum -a 256 beta-merged.bin
a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d  beta-merged.bin

$ gh release view somnus-v1.0.2-beta.1 --repo matthewclaude/bedknob-for-somnus --json isPrerelease,assets --jq '{isPrerelease, assets: [.assets[] | {name, size, digest}]}'
{"assets":[{"digest":"sha256:a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d","name":"somnus-dial-merged.bin","size":1743392},{"digest":"sha256:764f2228b8555a6b5591a722b959473ed97fd9b77898f24f0e67d3bcd6b973a0","name":"somnus-dial.bin","size":1612320}],"isPrerelease":true}

$ strings beta-merged.bin | grep -E '^1\.0\.[0-9](-beta\.[0-9])?$' | sort | uniq -c
   1 1.0.2-beta.1
```

The served `firmware/beta/` image still matches the 1.0.2-beta.1 Release
asset digest and embeds `1.0.2-beta.1`; the beta Release is still a
prerelease. (The `last-modified` on both images is the deploy time because
the Pages job rewrites the whole site each deploy; the beta bytes did not
change.)

## Step 8 — download-counter baseline

From the step 6 JSON at `publishedAt` 2026-09-17T23:15:27Z (queried about
two minutes later):

| Asset | downloadCount |
|---|---|
| `somnus-dial.bin` | 0 |
| `somnus-dial-merged.bin` | 0 |

Note the flasher image verification above fetched `firmware/latest/` from
GitHub Pages, not the Release asset URL, so it does not count against
these counters.

## Final local state

```
$ git fetch somnus
From github.com:matthewclaude/bedknob-for-somnus
   c9ab58c..a645265  gh-pages   -> somnus/gh-pages
$ git rev-parse somnus/main
5512537f7b34c4d8919471905709b86cf808d18c
$ git rev-parse HEAD
5512537f7b34c4d8919471905709b86cf808d18c
$ git --no-optional-locks log --oneline somnus/main..HEAD | wc -l
       0
$ git --no-optional-locks status --short
?? docs/REPORT-1.0.2-commit.md
?? docs/REPORT-docs-index-commit.md
```

No tracked file was edited. Nothing was committed. Nothing was pushed to
`origin`. Together with this file, three untracked reports now await the
docs commit.

## Deviations

1. **Manifest version check.** Step 7 expected `manifest.json` to name
   `1.0.2`. It names `latest`, by design of the committed source file (see
   step 7). The intent of the check — that the flasher serves the 1.0.2
   build — was verified instead by sha256 of the served image against the
   Release asset and by the embedded version string. No file was changed.
2. **Git environment.** Every `git` command ran with
   `DEVELOPER_DIR=/Library/Developer/CommandLineTools`, as instructed. One
   `strings` call (also an Xcode-shimmed binary) was first run without that
   variable and printed the Xcode license message instead of output; it was
   re-run with the variable set and the re-run is what is pasted above.
   No system setting was changed, no `sudo` was used.
3. **Extra checks beyond the block**, all read-only: the beta image's digest
   was compared to the beta.1 Release asset (to prove the stable deploy
   did not overwrite it, which the block asked to confirm only via the
   manifest head), the `somnus-v1.0.1` Release body was read to establish
   the footer precedent, and `git fetch somnus` was run to record the
   final `somnus/main` == `HEAD` state.

No other deviations.

## Not verified in this block

No dial was updated in this block; the dial and the pad were not touched
and no build, flash or `idf.py monitor` ran locally. The over-the-air
offer of `somnus-v1.0.2` to a dial on `1.0.1` (stable channel) or on
`1.0.2-beta.1` (beta channel, now behind stable) is checked by the owner
on hardware afterward.
