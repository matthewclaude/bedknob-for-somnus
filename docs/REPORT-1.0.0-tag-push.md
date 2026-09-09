# REPORT — somnus-v1.0.0 tag, push, CI and published-release verification

Date: 2026-09-09. Session: https://claude.ai/code/session_01T2F1kBEuT13CM3rWm5tnLD

## Verdict

**DONE** — `somnus-v1.0.0` tagged at `7c106fb`, pushed to the `somnus` remote only, release run 34412911352 and ci run 34412910378 both succeeded, and the public release is live as non-prerelease with both assets, `/releases/latest` = `somnus-v1.0.0`, Pages serving the merged image with HTTP 200.

## Gate (all five passed)

```
$ git rev-parse --short HEAD
c56cc4d

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "1.0.0")

$ git tag -l somnus-v1.0.0
(empty)

$ git --no-optional-locks status --short
?? docs/REPORT-1.0.0-commit.md

$ git remote get-url --push somnus
git@github.com:matthewclaude/bedknob-for-somnus.git

(informational) $ git remote get-url --push origin
no_push
```

## Step 1 — housekeeping commit

Before (firmware/dial-idf/CMakeLists.txt lines 20-22):

```
# 1.0.0 is reserved for when docs/V1-scope.md is actually complete -- see
# CHANGELOG.md's Somnus section header.
set(PROJECT_VER "1.0.0")
```

After (lines 20-21):

```
# Somnus versioning: tag somnus-vX.Y.Z must equal PROJECT_VER exactly (release.yml verifies). 1.0.0 shipped 2026-09-09; see CHANGELOG.md.
set(PROJECT_VER "1.0.0")
```

Only the two comment lines were replaced by the one new comment line; nothing else in the file changed. Because two lines became one, `set(PROJECT_VER "1.0.0")` is now **line 21**, not line 22 (see Deviations).

```
$ git commit -m "chore: drop stale 1.0.0-reserved comment; track 1.0.0 commit report"
$ git rev-parse --short HEAD
7c106fb

$ git diff --stat HEAD~1 HEAD
 docs/REPORT-1.0.0-commit.md      | 234 +++++++++++++++++++++++++++++++++++++++
 firmware/dial-idf/CMakeLists.txt |   3 +-
 2 files changed, 235 insertions(+), 2 deletions(-)
```

## Step 2 — tag

```
$ git tag -a somnus-v1.0.0 -m "Bedknob for Somnus 1.0.0 — the 0.1.6 build renumbered; v1 scope complete"
$ git tag -l somnus-v1.0.0
somnus-v1.0.0
$ git rev-parse somnus-v1.0.0            (tag object)
e4a35ebd2eef3f39da46bb77ce3687c5d9f2f92d
$ git rev-parse somnus-v1.0.0^{commit}   (commit it points at)
7c106fbb9ba70b6d733dca0e0048ae176635afe8
$ git rev-parse HEAD
7c106fbb9ba70b6d733dca0e0048ae176635afe8
```

Tag commit == HEAD.

## Step 3 — push (somnus only)

```
$ git push somnus main
To github.com:matthewclaude/bedknob-for-somnus.git
   53a42c5..7c106fb  main -> main
(exit 0)

$ git push somnus somnus-v1.0.0
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.0 -> somnus-v1.0.0
(exit 0)
```

No push to `origin` was made.

## Step 4 — CI (source repo matthewclaude/bedknob-for-somnus)

`gh run list --repo matthewclaude/bedknob-for-somnus --limit 5` (15 s after push) showed two new runs on head `7c106fb`: **release** 34412911352 (branch `somnus-v1.0.0`, in_progress) and **ci** 34412910378 (branch `main`, queued); the three older entries were the 0.1.6 release/ci runs from 2026-09-07.

`gh run watch 34412911352 --repo matthewclaude/bedknob-for-somnus --exit-status` exited 0. Run details:

```
$ gh run view 34412911352 --repo matthewclaude/bedknob-for-somnus --json databaseId,conclusion,status,event,headBranch,headSha,createdAt,updatedAt,jobs
{"conclusion":"success","createdAt":"2026-09-09T22:34:44Z","databaseId":34412911352,"event":"push","headBranch":"somnus-v1.0.0","headSha":"7c106fbb9ba70b6d733dca0e0048ae176635afe8","jobs":[{"completedAt":"2026-09-09T22:40:31Z","conclusion":"success","name":"build-and-release","startedAt":"2026-09-09T22:34:46Z"},{"completedAt":"2026-09-09T22:40:45Z","conclusion":"success","name":"deploy-pages","startedAt":"2026-09-09T22:40:34Z"}],"status":"completed","updatedAt":"2026-09-09T22:40:46Z"}

$ gh run view 34412910378 (ci) --json databaseId,conclusion,status,headBranch,headSha,jobs
{"conclusion":"success","createdAt":"2026-09-09T22:34:43Z","databaseId":34412910378,"headBranch":"main","headSha":"7c106fbb9ba70b6d733dca0e0048ae176635afe8","jobs":[{"completedAt":"2026-09-09T22:40:22Z","conclusion":"success","name":"build","startedAt":"2026-09-09T22:35:22Z"}],"status":"completed","updatedAt":"2026-09-09T22:40:22Z"}
```

Summary:

| Run | Workflow | Job | Started → completed (UTC) | Duration | Conclusion |
|---|---|---|---|---|---|
| 34412911352 | release (tag `somnus-v1.0.0`) | build-and-release | 22:34:46 → 22:40:31 | 5m45s | success |
| 34412911352 | release | deploy-pages | 22:40:34 → 22:40:45 | 11s | success |
| 34412910378 | ci (push to `main`) | build | 22:35:22 → 22:40:22 | 5m00s | success |

Release run steps, all ✓ (from the watch output): Set up job; checkout; **Verify tag matches PROJECT_VER**; **Extract release notes from CHANGELOG.md**; Classify release channel; Build firmware and merge flashable image; Upload merged image for the Pages job; Publish GitHub Release; then deploy-pages: Download merged image; Channel directory; Assemble Pages site; Deploy to gh-pages.

One annotation, not a failure: GitHub's Node.js 20 deprecation notice for actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2 (they are being forced onto Node 24). Pre-existing, unrelated to this release.

No failed steps, so `gh run view --log-failed` was not needed.

## Step 5 — published release (releases repo matthewclaude/somnus-dial-releases)

```
$ gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,publishedAt,assets
{"assets":[{"name":"somnus-dial-merged.bin","size":1743152},{"name":"somnus-dial.bin","size":1612080}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-09T22:40:28Z","tagName":"somnus-v1.0.0"}

$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
somnus-v1.0.0

$ curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin | head -1
HTTP/2 200 

$ curl -sI ... | grep -i -E "content-length|last-modified"
last-modified: Wed, 09 Sep 2026 22:41:12 GMT
content-length: 1743152
```

Result: `isPrerelease` false, `isDraft` false, published 2026-09-09T22:40:28Z, assets `somnus-dial-merged.bin` 1,743,152 bytes and `somnus-dial.bin` 1,612,080 bytes. `/releases/latest` is `somnus-v1.0.0`. The Pages path `firmware/latest/somnus-dial-merged.bin` (as given in the spec, no fallback to the manifest needed) returns HTTP/2 200 with content-length 1,743,152 — identical to the release asset — and last-modified 22:41:12Z, i.e. after the deploy-pages job. (The flasher page at the root loads `./manifest.json`, or `./manifest-beta.json` with the beta box checked; not needed since the direct path resolved.)

### Release body vs CHANGELOG 1.0.0 section

`gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json body --jq .body` saved, and diffed against the CHANGELOG section extracted with release.yml's awk (`ver=1.0.0`):

```
$ diff <changelog-1.0.0-section> <release-body>
26a27,31
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
```

The only difference is the five appended lines (a blank, the OTA/flasher pointer paragraph, a blank, and the PolyForm Required Notice) — the footer release.yml appends to every release. Confirmed against the workflow source:

```
$ grep -n -E 'Required Notice|pick this up on their own|browser flasher' .github/workflows/release.yml
83:            echo "Dials already running the firmware pick this up on their own — see Menu → Update."
84:            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); \`somnus-dial-merged.bin\` below is the same image for flashing manually from offset 0x0."
86:            echo "Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)"
```

The 26 lines of the CHANGELOG section are byte-identical in the published body.

## Deviations from this spec

1. **`PROJECT_VER` is now on line 21, not 22.** The spec asked to replace the two-line comment (lines 20-21) with a single line and also for line 22 to still read `set(PROJECT_VER "1.0.0")`; those two requirements are mutually exclusive, and the replacement was done as written. The line content is unchanged; only its number moved. CI's "Verify tag matches PROJECT_VER" step passed, so the check does not depend on the line number.
2. **Commit trailers.** The housekeeping commit carries the session's required `Co-Authored-By` / `Claude-Session` trailers in addition to the spec's subject line.
3. **Saved the release body to the session scratchpad** rather than `/tmp/rel-body.txt`, per this session's scratchpad rule. Same content, same diff.

Everything else — gate, edit scope, commit, annotated tag text, pushes to `somnus` only, run watching, release/asset/latest/Pages checks — matches the spec.

## Could not verify without hardware

- A real dial on `0.1.6` receiving and installing `1.0.0` over the air (manual "Check for updates" or the automatic morning window), and the post-install rollback validation on the device. `/releases/latest` and the asset are in place for it, but no dial was observed in this session.
- Nothing was flashed or booted here; CI's build is the only build of `7c106fb`.
