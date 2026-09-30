# somnus-v1.0.4-beta.1 tag, push and verify

**Verdict: PASS.** `somnus-v1.0.4-beta.1` (tag object `ac499d8`, commit `1d01f3b`) is on `somnus` and `main` there is `1d01f3b`. Both the ci and release runs succeeded. The Release is a non-draft prerelease with the two expected assets, and its body is the CHANGELOG section plus the standard workflow footer. `releases/latest` is still `somnus-v1.0.2`. The flasher's beta channel serves the new image and the latest channel serves 1.0.2. Download counts match the owner's.

## Gate

| # | Check | Raw output | Result |
|---|---|---|---|
| 1 | `sed -n 21p firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.4-beta.1")` | PASS |
| 2 | `git rev-parse HEAD` | `1d01f3b664e6a217bbf10bb34152f683d1a4d91e` | PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | *(empty)* | PASS |
| 4 | `git tag -l somnus-v1.0.4-beta.1` ; `git ls-remote --tags somnus 'somnus-v1.0.4*'` | *(empty)* ; *(empty, rc=0)* | PASS |
| 5 | `git ls-remote --heads somnus main` ; `git merge-base --is-ancestor ea76e17… HEAD` | `ea76e170c0e6f10a00400c78cdcd9f981a0cd2c7	refs/heads/main` ; rc=0 | PASS |
| 6 | `grep -c '^## 1.0.4-beta.1 — 2026-09-30 (beta)$' CHANGELOG.md` | `1` | PASS |

No command reported a problem with the Xcode licence. `DEVELOPER_DIR` was not set. Remotes: `somnus` pushes to `git@github.com:matthewclaude/bedknob-for-somnus.git`, and `origin`'s push URL is `no_push`.

## Step 1: tag

`git tag -a somnus-v1.0.4-beta.1 1d01f3b -m "somnus-v1.0.4-beta.1"`

| | Expected | Actual |
|---|---|---|
| `git rev-parse somnus-v1.0.4-beta.1` (tag object) | an annotated tag object | `ac499d80091e055a56bcf5c555670517fec5a720` |
| `git rev-parse somnus-v1.0.4-beta.1^{commit}` | `1d01f3b…` | `1d01f3b664e6a217bbf10bb34152f683d1a4d91e` |

## Step 2: push

`git push somnus main` (rc=0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
   ea76e17..1d01f3b  main -> main
```

This matches the expected `ea76e17..1d01f3b`, which carries 5e9bc69, 452df8e and 1d01f3b.

`git push somnus somnus-v1.0.4-beta.1` (rc=0):

```
To github.com:matthewclaude/bedknob-for-somnus.git
 * [new tag]         somnus-v1.0.4-beta.1 -> somnus-v1.0.4-beta.1
```

The tag push completed at **2026-09-30T20:16:38Z** (UTC).

`git ls-remote somnus refs/heads/main 'refs/tags/somnus-v1.0.4-beta.1*'`:

```
1d01f3b664e6a217bbf10bb34152f683d1a4d91e	refs/heads/main
ac499d80091e055a56bcf5c555670517fec5a720	refs/tags/somnus-v1.0.4-beta.1
1d01f3b664e6a217bbf10bb34152f683d1a4d91e	refs/tags/somnus-v1.0.4-beta.1^{}
```

main = 1d01f3b and the peeled tag = 1d01f3b, as expected.

## Step 3: workflows

`gh run list --limit 8` right after the tag push (first two rows):

```
{"conclusion":"","createdAt":"2026-09-30T20:16:39Z","databaseId":36771477790,"event":"push","headBranch":"somnus-v1.0.4-beta.1","headSha":"1d01f3b664e6a217bbf10bb34152f683d1a4d91e","name":"release","status":"queued"}
{"conclusion":"","createdAt":"2026-09-30T20:16:35Z","databaseId":36771468681,"event":"push","headBranch":"main","headSha":"1d01f3b664e6a217bbf10bb34152f683d1a4d91e","name":"ci","status":"in_progress"}
```

Both runs were watched with `gh run watch <id> --exit-status`, and both exited with rc=0.

| Run | ID | Trigger | Conclusion | Start → end (UTC) | Duration |
|---|---|---|---|---|---|
| ci | 36771468681 | push main @ 1d01f3b | success | 20:16:35 → 20:19:46 | 3 min 11 s |
| release | 36771477790 | push tag somnus-v1.0.4-beta.1 @ 1d01f3b | success | 20:16:39 → 20:20:05 | 3 min 26 s |
| pages build and deployment | 36771874487 | gh-pages @ d0141e4 | success | 20:20:03 → 20:20:33 | 30 s |

The only annotation in either watch output is GitHub's notice that ubuntu-latest will migrate to Ubuntu 26 beginning October 19, 2026. It is informational.

## Step 4: Release

`gh release view somnus-v1.0.4-beta.1 --json tagName,isDraft,isPrerelease,publishedAt,body,assets`:

| Field | Expected | Actual |
|---|---|---|
| tagName | somnus-v1.0.4-beta.1 | somnus-v1.0.4-beta.1 |
| isDraft | false | false |
| isPrerelease | true | true |
| publishedAt | (after the tag push) | 2026-09-30T20:19:46Z |
| asset count | 2 | 2 |

| Asset | Size (B) | Digest | downloadCount |
|---|---|---|---|
| somnus-dial.bin | 1,611,568 | sha256:43683c66b038033b0de750e87f5ebbf683de66025c67a1c2d1a19e9f2771fc51 | 0 |
| somnus-dial-merged.bin | 1,742,640 | sha256:4ab54439d84a6fa06fc7fbb956ee713234a3d72253f38e44f5eb8c847045c829 | 0 |

`somnus-dial.bin` is 1,611,568 B, the same size as the local build recorded in REPORT-1.0.4-beta.1-commit.md.

**Tag push to publishedAt:** 20:16:38Z → 20:19:46Z = **3 min 08 s**.

**Body diff.** The CHANGELOG section was extracted with release.yml's awk range and trim, then compared with `diff <section> <body>`:

```diff
7a8,12
> 
> Dials already running the firmware pick this up on their own — see Menu → Update.
> For a first install, use the [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
> 
> Required Notice: Copyright (c) 2026 Chris Meyer (https://github.com/chris023/orion-waveshare-rotary-dial)
```

The only difference is lines added after the section. They are the footer that `.github/workflows/release.yml` writes (lines 78 and 81 contain the "pick this up on their own" and "Required Notice" lines). The CHANGELOG text itself is unchanged, as expected.

`gh api repos/matthewclaude/bedknob-for-somnus/releases/latest --jq .tag_name` returned `somnus-v1.0.2`, as expected: the beta did not become latest.

## Step 5: flasher

The pages deployment run 36771874487 finished with success at 20:20:33Z. Both files were then downloaded at 20:20:41Z into `bench-logs/verify-1.0.4b1/`, which `git check-ignore` confirms is ignored. The first attempt succeeded, so no retries were needed.

| Channel | HTTP | Size (B) | shasum -a 256 | Expected sha256 | Version strings | Expected |
|---|---|---|---|---|---|---|
| beta | 200 | 1,742,640 | 4ab54439d84a6fa06fc7fbb956ee713234a3d72253f38e44f5eb8c847045c829 | new merged asset digest `4ab54439…c829` | `1.0.4-beta.1` | 1.0.4-beta.1 |
| latest | 200 | 1,743,392 | e9f88689b9313cb33c1e933db2394bcc24b481e16bb2f69b071187cebfe13462 | e9f88689…3462 | `1.0.2` | 1.0.2 |

Both channels match. The manifests were not used as evidence.

## Step 6: download counts

| Release | Asset | Now | Owner's own | Above owner's |
|---|---|---|---|---|
| somnus-v1.0.2 | somnus-dial.bin | 2 | 2 | 0 |
| somnus-v1.0.2 | somnus-dial-merged.bin | 0 | 0 | 0 |
| somnus-v1.0.3-beta.1 | somnus-dial.bin | 1 | 1 | 0 |
| somnus-v1.0.3-beta.1 | somnus-dial-merged.bin | 0 | 0 | 0 |
| somnus-v1.0.4-beta.1 | somnus-dial.bin | 0 | 0 | 0 |
| somnus-v1.0.4-beta.1 | somnus-dial-merged.bin | 0 | 0 | 0 |

No count is above the owner's. Bedknob #1, which already runs 1.0.4-beta.1 by wire flash, has not added a download. The Step 5 downloads came from GitHub Pages, not from the Release assets, so they do not count here.

## Deviations

- `gh run watch` was given `--interval 20` (and `--interval 15` for the pages run) to poll less often. The ci and release watches ran in parallel. Neither change affects the results.
- Otherwise none.

## Not verifiable here

- An over-the-air install of 1.0.4-beta.1 from 1.0.3-beta.1 on a real dial. Bedknob #1 already runs this build by wire flash.
- A browser-flasher install of the beta image.

## Next step for the owner

Make the docs commit that regenerates the six version-bearing screens at 1.0.4-beta.1. It also commits this report and `docs/REPORT-1.0.4-beta.1-commit.md`, with a REPORTS.md line for each.
