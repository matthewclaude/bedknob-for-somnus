# REPORT — somnus-v1.0.2-beta.1 tag, push, single-repo Release and Pages verification

Date: 2026-09-12 (UTC 2026-09-13). Session: https://claude.ai/code/session_01LX9RCTwWzWZfuYhk3ZRp4E

Block 2 of 2 for `1.0.2-beta.1`. Block 1 is `REPORT-1.0.2-beta.1-commit.md` (commit `c6261fa`). This is the first tag since Phase 6a (`REPORT-phase6a.md`, commit `8ee93c4`), so it is the first release published to `bedknob-for-somnus` ONLY — the first live run of the trimmed workflow. No code changes; nothing flashed; the dial and the pad were not touched.

## Verdict

**DONE** — `somnus-v1.0.2-beta.1` tagged at `00bd94d`, pushed to the `somnus` remote only (`matthewclaude/bedknob-for-somnus`), ci run 34727385574 and release run 34727391763 both succeeded, the prerelease exists in this repo with two assets, `somnus-dial-releases` does NOT have the tag (`release not found`), `/releases/latest` is still `somnus-v1.0.1`, and the flasher page on this repo's gh-pages serves HTTP 200 with zero references to `somnus-dial-releases`. No deviations.

**Tag-to-Release window: 2026-09-13T00:12:48Z → 2026-09-13T00:16:03Z** (3 m 15 s). The tag push completed at 00:12:48Z; the Release's `publishedAt` is 00:16:03Z; the release run's second job (Pages) completed at 00:16:26Z and the Pages build-and-deployment run at ~00:16:5xZ. Dials polling for updates could see the beta from 00:16:03Z on.

## Gate (all four passed)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2-beta.1")
$ git --no-optional-locks tag -l 'somnus-v1.0.2*'
(empty)
$ git --no-optional-locks log --oneline somnus/main..HEAD | wc -l
       4
$ git --no-optional-locks status --short --untracked-files=no
(empty)
```

Supporting context captured before tagging:

```
$ git --no-optional-locks remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
$ git --no-optional-locks rev-parse HEAD
00bd94d263e36c643cfa27756f3d0463905bad86
$ git --no-optional-locks log --oneline somnus/main..HEAD
00bd94d docs: 1.0.2-beta.1 release-commit report (block 1, DONE) and its REPORTS.md line
c6261fa release: 1.0.2-beta.1 — STANDBY poll cadence 300 s (PROJECT_VER bump + CHANGELOG section)
6cd1cc3 docs: 1.0.2 bench report for SPEC-standby-poll §6 items 4 and 5 (both PASS) and its REPORTS.md line
7fda070 docs: SPEC-standby-poll §6 item 4 — night face via the on-device Timezone picker, not the Tokyo trick
```

Previous beta tag shape, for the annotation: `somnus-v1.0.1-beta.1` is an annotated tag whose message is its own name. Matched.

## Step 1 — tag

```
$ git tag -a somnus-v1.0.2-beta.1 -m 'somnus-v1.0.2-beta.1'
$ git --no-optional-locks for-each-ref --format='%(refname:short) %(objectname) -> %(*objectname) | %(contents:subject) | %(taggerdate:iso8601)' refs/tags/somnus-v1.0.2-beta.1
somnus-v1.0.2-beta.1 45ce226a19bd538ae4b4a44e5c50dc3148be9f9f -> 00bd94d263e36c643cfa27756f3d0463905bad86 | somnus-v1.0.2-beta.1 | 2026-09-12 19:12:39 -0500
```

- Tag object: `45ce226a19bd538ae4b4a44e5c50dc3148be9f9f`
- Tagged commit: `00bd94d263e36c643cfa27756f3d0463905bad86` (HEAD, `main`)
- Annotation: `somnus-v1.0.2-beta.1`

## Steps 2–3 — push main, then the tag (somnus remote only)

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   3dacfd5..00bd94d  main -> main
```

```
--- before: 2026-09-13T00:12:47Z
$ git push somnus somnus-v1.0.2-beta.1
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.2-beta.1 -> somnus-v1.0.2-beta.1
--- after: 2026-09-13T00:12:48Z
$ git ls-remote --tags somnus 'somnus-v1.0.2*'
45ce226a19bd538ae4b4a44e5c50dc3148be9f9f	refs/tags/somnus-v1.0.2-beta.1
00bd94d263e36c643cfa27756f3d0463905bad86	refs/tags/somnus-v1.0.2-beta.1^{}
```

**UTC tag-push time: 2026-09-13T00:12:48Z.** Nothing was pushed to `origin` (push URL `no_push`).

## Step 4 — both workflow runs

Run list immediately after the tag push, and again after completion:

```
$ gh run list --repo matthewclaude/bedknob-for-somnus --limit 5 --json databaseId,name,event,headBranch,status,conclusion,createdAt
[{"conclusion":"","createdAt":"2026-09-13T00:12:50Z","databaseId":34727391763,"event":"push","headBranch":"somnus-v1.0.2-beta.1","name":"release","status":"queued"},
 {"conclusion":"","createdAt":"2026-09-13T00:12:43Z","databaseId":34727385574,"event":"push","headBranch":"main","name":"ci","status":"in_progress"},
 {"conclusion":"success","createdAt":"2026-09-12T19:21:17Z","databaseId":34713920604,"event":"push","headBranch":"main","name":"ci","status":"completed"}, ...]

(after completion)
[{"conclusion":"","createdAt":"2026-09-13T00:16:25Z","databaseId":34727543988,"event":"dynamic","headBranch":"gh-pages","name":"pages build and deployment","status":"in_progress"},
 {"conclusion":"success","createdAt":"2026-09-13T00:12:50Z","databaseId":34727391763,"event":"push","headBranch":"somnus-v1.0.2-beta.1","name":"release","status":"completed"},
 {"conclusion":"success","createdAt":"2026-09-13T00:12:43Z","databaseId":34727385574,"event":"push","headBranch":"main","name":"ci","status":"completed"}, ...]
```

`gh run watch 34727391763 --exit-status` returned 0 at 2026-09-13T00:16:28Z. The only annotation on the run is GitHub's Node.js 20 deprecation notice for `actions/checkout@v4`, `actions/upload-artifact@v4` and `softprops/action-gh-release@v2` (informational, not a failure).

### release run 34727391763 (event push, headBranch `somnus-v1.0.2-beta.1`) — success

| job | id | started | completed | duration | conclusion |
|---|---|---|---|---|---|
| build-and-release | 103643838938 | 00:12:55Z | 00:16:14Z | 3 m 19 s | success |
| deploy-pages | 103644237530 | 00:16:17Z | 00:16:26Z | 9 s | success |

Raw `gh run view 34727391763 --json jobs`:

```
{"jobs":[{"completedAt":"2026-09-13T00:16:14Z","conclusion":"success","databaseId":103643838938,"name":"build-and-release","startedAt":"2026-09-13T00:12:55Z","status":"completed","steps":[{"completedAt":"2026-09-13T00:12:57Z","conclusion":"success","name":"Set up job","number":1,"startedAt":"2026-09-13T00:12:56Z","status":"completed"},{"completedAt":"2026-09-13T00:12:58Z","conclusion":"success","name":"Run actions/checkout@v4","number":2,"startedAt":"2026-09-13T00:12:57Z","status":"completed"},{"completedAt":"2026-09-13T00:12:58Z","conclusion":"success","name":"Verify tag matches PROJECT_VER","number":3,"startedAt":"2026-09-13T00:12:58Z","status":"completed"},{"completedAt":"2026-09-13T00:12:58Z","conclusion":"success","name":"Extract release notes from CHANGELOG.md","number":4,"startedAt":"2026-09-13T00:12:58Z","status":"completed"},{"completedAt":"2026-09-13T00:12:58Z","conclusion":"success","name":"Classify release channel","number":5,"startedAt":"2026-09-13T00:12:58Z","status":"completed"},{"completedAt":"2026-09-13T00:16:01Z","conclusion":"success","name":"Build firmware and merge flashable image (firmware/dial-idf)","number":6,"startedAt":"2026-09-13T00:12:58Z","status":"completed"},{"completedAt":"2026-09-13T00:16:03Z","conclusion":"success","name":"Upload merged image for the Pages job","number":7,"startedAt":"2026-09-13T00:16:01Z","status":"completed"},{"completedAt":"2026-09-13T00:16:12Z","conclusion":"success","name":"Publish GitHub Release (this repo)","number":8,"startedAt":"2026-09-13T00:16:03Z","status":"completed"},{"completedAt":"2026-09-13T00:16:12Z","conclusion":"success","name":"Post Run actions/checkout@v4","number":16,"startedAt":"2026-09-13T00:16:12Z","status":"completed"},{"completedAt":"2026-09-13T00:16:12Z","conclusion":"success","name":"Complete job","number":17,"startedAt":"2026-09-13T00:16:12Z","status":"completed"}],"url":"https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34727391763/job/103643838938"},{"completedAt":"2026-09-13T00:16:26Z","conclusion":"success","databaseId":103644237530,"name":"deploy-pages","startedAt":"2026-09-13T00:16:17Z","status":"completed","steps":[{"completedAt":"2026-09-13T00:16:19Z","conclusion":"success","name":"Set up job","number":1,"startedAt":"2026-09-13T00:16:18Z","status":"completed"},{"completedAt":"2026-09-13T00:16:20Z","conclusion":"success","name":"Run actions/checkout@v4","number":2,"startedAt":"2026-09-13T00:16:19Z","status":"completed"},{"completedAt":"2026-09-13T00:16:21Z","conclusion":"success","name":"Download merged image","number":3,"startedAt":"2026-09-13T00:16:20Z","status":"completed"},{"completedAt":"2026-09-13T00:16:21Z","conclusion":"success","name":"Channel directory","number":4,"startedAt":"2026-09-13T00:16:21Z","status":"completed"},{"completedAt":"2026-09-13T00:16:21Z","conclusion":"success","name":"Assemble Pages site","number":5,"startedAt":"2026-09-13T00:16:21Z","status":"completed"},{"completedAt":"2026-09-13T00:16:24Z","conclusion":"success","name":"Deploy to gh-pages (this repo)","number":6,"startedAt":"2026-09-13T00:16:21Z","status":"completed"},{"completedAt":"2026-09-13T00:16:24Z","conclusion":"success","name":"Post Run actions/checkout@v4","number":12,"startedAt":"2026-09-13T00:16:24Z","status":"completed"},{"completedAt":"2026-09-13T00:16:24Z","conclusion":"success","name":"Complete job","number":13,"startedAt":"2026-09-13T00:16:24Z","status":"completed"}],"url":"https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34727391763/job/103644237530"}]}
```

Note the step list: exactly one `Publish GitHub Release (this repo)` and one `Deploy to gh-pages (this repo)` — the old-repo twins that Phase 6a removed are absent, as intended.

### ci run 34727385574 (event push, headBranch `main`) — success

| job | id | started | completed | duration | conclusion |
|---|---|---|---|---|---|
| build | 103643823661 | 00:12:46Z | 00:15:59Z | 3 m 13 s | success |

Raw `gh run view 34727385574 --json jobs`:

```
{"jobs":[{"completedAt":"2026-09-13T00:15:59Z","conclusion":"success","databaseId":103643823661,"name":"build","startedAt":"2026-09-13T00:12:46Z","status":"completed","steps":[{"completedAt":"2026-09-13T00:12:47Z","conclusion":"success","name":"Set up job","number":1,"startedAt":"2026-09-13T00:12:46Z","status":"completed"},{"completedAt":"2026-09-13T00:12:53Z","conclusion":"success","name":"Run actions/checkout@v4","number":2,"startedAt":"2026-09-13T00:12:47Z","status":"completed"},{"completedAt":"2026-09-13T00:15:58Z","conclusion":"success","name":"Build firmware (firmware/dial-idf)","number":3,"startedAt":"2026-09-13T00:12:53Z","status":"completed"},{"completedAt":"2026-09-13T00:15:58Z","conclusion":"success","name":"Post Run actions/checkout@v4","number":6,"startedAt":"2026-09-13T00:15:58Z","status":"completed"},{"completedAt":"2026-09-13T00:15:58Z","conclusion":"success","name":"Complete job","number":7,"startedAt":"2026-09-13T00:15:58Z","status":"completed"}],"url":"https://github.com/matthewclaude/bedknob-for-somnus/actions/runs/34727385574/job/103643823661"}]}
```

### GitHub's own Pages run 34727543988 (event dynamic, headBranch `gh-pages`) — success

Not one of "the two runs", but it is what actually publishes the flasher after `Deploy to gh-pages (this repo)` pushes the branch. Created 00:16:25Z, completed with `success` before the step-8 fetch at 00:16:58Z.

## Step 5 — the Release

```
$ gh release view somnus-v1.0.2-beta.1 --repo matthewclaude/bedknob-for-somnus --json tagName,isDraft,isPrerelease,publishedAt,assets,body
```

| field | value |
|---|---|
| tagName | `somnus-v1.0.2-beta.1` |
| isDraft | false |
| isPrerelease | **true** |
| publishedAt | 2026-09-13T00:16:03Z |

Assets (two):

| name | size (bytes) | sha256 | state |
|---|---|---|---|
| `somnus-dial-merged.bin` | 1743392 | `a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d` | uploaded |
| `somnus-dial.bin` | 1612320 | `764f2228b8555a6b5591a722b959473ed97fd9b77898f24f0e67d3bcd6b973a0` | uploaded |

Raw JSON:

```
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/560214835","contentType":"application/octet-stream","createdAt":"2026-09-13T00:16:05Z","digest":"sha256:a0dac94c7df18ee247480883790ed07a6b8744807b5a545535b35e224b19405d","downloadCount":0,"id":"RA_kwDOUCLPIM4hZDMz","label":"","name":"somnus-dial-merged.bin","size":1743392,"state":"uploaded","updatedAt":"2026-09-13T00:16:11Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2-beta.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/assets/560214834","contentType":"application/octet-stream","createdAt":"2026-09-13T00:16:05Z","digest":"sha256:764f2228b8555a6b5591a722b959473ed97fd9b77898f24f0e67d3bcd6b973a0","downloadCount":0,"id":"RA_kwDOUCLPIM4hZDMy","label":"","name":"somnus-dial.bin","size":1612320,"state":"uploaded","updatedAt":"2026-09-13T00:16:05Z","url":"https://github.com/matthewclaude/bedknob-for-somnus/releases/download/somnus-v1.0.2-beta.1/somnus-dial.bin"}],"body":"While the screen is off and the dial has gone to standby, it now asks the\npad for its state once every five minutes instead of every ten seconds.\nTouching the knob still reads the pad immediately, so the face is current\nthe moment you look at it. Nothing about the screens or pad control\nchanges.\n\nInternal: `POLL_STANDBY_US` at 300 s in the worker loop's due computation;\nthe cadence is logged when crossing into or out of standby; a pad outage\nin standby now shows as stale after about fifteen minutes instead of three.\n\nDials already running the firmware pick this up on their own — see Menu → Update.\nFor a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.\n\nRequired Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)","isDraft":false,"isPrerelease":true,"publishedAt":"2026-09-13T00:16:03Z","tagName":"somnus-v1.0.2-beta.1"}
```

### Body vs the CHANGELOG section

`diff` of the `## 1.0.2-beta.1 — 2026-09-13 (beta)` section body (heading dropped, trailing blank lines trimmed) against the published `body`:

```
1d0
<
10a10,14
>
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
>
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
```

The `1d0` line is the blank line that follows the heading in CHANGELOG.md (an artefact of my extraction, not a content difference). The only other difference is the four-line footer the workflow appends — the OTA note, the flasher link (now pointing at `bedknob-for-somnus`, as Phase 6a rewrote it) and the required copyright notice. The two prose paragraphs are byte-identical. **As expected.**

## Step 6 — the old repo did NOT receive it (Phase 6a's first live test)

```
$ gh release view somnus-v1.0.2-beta.1 --repo matthewclaude/somnus-dial-releases --json tagName 2>&1
release not found
exit=1
```

**PASS.** This is the object-returning command, not a summarizer; the API reports no such release on `somnus-dial-releases`. The workflow's step list (step 4) corroborates it: there is no old-repo publish step left to have run.

## Step 7 — `/releases/latest` on this repo

```
$ gh release view --repo matthewclaude/bedknob-for-somnus --json tagName
{"tagName":"somnus-v1.0.1"}
```

`somnus-v1.0.1`, as expected: `isPrerelease` is true on the new Release, and the latest endpoint excludes prereleases. By design, not a fault. Stable dials polling `latest` will not see the beta.

## Step 8 — the flasher page, served from this repo's gh-pages for the first time

Fetched at 00:16:58Z, about 30 s after the Pages build-and-deployment run finished. No retry was needed.

```
$ curl -sSI https://matthewclaude.github.io/bedknob-for-somnus/ | head -1
HTTP/2 200
$ curl -sS https://matthewclaude.github.io/bedknob-for-somnus/ | grep -c 'somnus-dial-releases'
0
```

Supporting: the `bedknob-for-somnus` paths the page does reference:

```
$ curl -sS https://matthewclaude.github.io/bedknob-for-somnus/ | grep -o 'bedknob-for-somnus[^"'"'"' ]*' | sort -u
bedknob-for-somnus
bedknob-for-somnus/blob/main/LICENSE
bedknob-for-somnus/blob/main/THIRD_PARTY_LICENSES.md
bedknob-for-somnus/releases
bedknob-for-somnus/releases/latest
```

HTTP 200, count 0. **PASS.**

## Deviations

None. Every gate check matched its expected value, main was pushed before the tag, only the `somnus` remote was pushed, no `gh run` was retried, nothing was re-tagged, the flasher fetch succeeded on the first attempt, and no flash, monitor, dial or pad interaction took place.

Two things worth noting that are not deviations:

- The release run's `Publish GitHub Release (this repo)` step ran 00:16:03–00:16:12Z but the Release's `publishedAt` is 00:16:03Z — GitHub stamps the Release at creation, before the asset uploads (assets `createdAt` 00:16:05Z) complete. The window end above uses `publishedAt`.
- GitHub's own `pages build and deployment` run (34727543988) was still `in_progress` when the release run finished; it completed on its own before the step-8 fetch.

## Not verified here (needs the bench dial)

- That a dial running `somnus-v1.0.1` on the **beta** channel actually offers and installs `1.0.2-beta.1` from this repo's Release (SPEC-standby-poll §6 item 7's second half, and the end-to-end OTA path after the repo switch). The Release object, its two assets and its prerelease flag are correct from the API side; the client-side check is a bench task.
- That a stable-channel dial stays on `1.0.1` (the `/releases/latest` result says it will, but this was not observed on a device).
- That the flasher page can actually flash a dial from the new URL (it serves 200 and links only this repo; a browser-driven flash was not attempted).

## State at close

- `main` on `somnus` == local `main` == `00bd94d` (this report's commit will move it one ahead until pushed).
- `somnus-v1.0.2-beta.1` → `00bd94d` on the remote; tag object `45ce226`.
- `origin` untouched (push URL `no_push`).
- Nothing flashed; the bench dial and the pad are in whatever state the owner left them.
