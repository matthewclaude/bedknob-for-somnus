# REPORT: 0.1.5-beta.5 CI / release check

**Verdict: RELEASE CONFIRMED BUILT AND PUBLISHED. `somnus-v0.1.5-beta.5` is live as a prerelease on `matthewclaude/somnus-dial-releases` with both firmware assets, and the gh-pages beta channel serves the new merged image. Workflow run 33932029987 on the private fork confirmed via authenticated gh: `completed` / `success`.**

Date: 2026-09-05 (UTC) / 2026-09-04 evening local
Tag: `somnus-v0.1.5-beta.5` -> `afd23fffbee543d2de4ae9eb6becfda6ed55ad4c`

## gh install

- Homebrew present at `/opt/homebrew/bin/brew` (arm64 Mac), so the brew path was used:
  `brew install gh`
- Installed: `gh version 2.100.0 (2026-09-03)` at `/opt/homebrew/bin/gh`, which is on
  PATH. `gh` works as a plain command.
- Homebrew was NOT installed as a side effect (it was already there).

## gh auth

`gh auth status` -> `You are not logged into any GitHub hosts.`

`gh auth login` is an interactive browser/device-code flow and could not be completed
from an unattended session. No re-auth was attempted beyond the status check. Because of
this, the two gh commands requested (`gh run list` on the private fork and
`gh release list` on the public repo) could not run. The public repo was checked via the
unauthenticated GitHub REST API instead, which is sufficient to prove the release exists.

## Workflow run (private fork `matthewclaude/somnus-waveshare-rotary-dial`)

Initially not directly observable (gh unauthenticated at the time); see "Outstanding item
resolved" below for the confirmed run. Indirect evidence gathered first:

- `release.yml` triggers on `push: tags: ['somnus-v*']` and publishes to the public repo
  only on a successful build.
- The tag was pushed at ~00:08 UTC (commit timestamp 2026-09-05T00:06:47Z).
- The release object appeared on the public repo at `2026-09-05T00:14:40Z` and the
  gh-pages beta image at `00:15:12Z` — roughly 6–7 minutes after the tag push, matching
  beta.3 and beta.4 build durations.

Both cross-repo writes (Release API + gh-pages push) only happen from that workflow, so
the run completed successfully. Run URL to confirm once logged in:
https://github.com/matthewclaude/somnus-waveshare-rotary-dial/actions
(`gh run list --repo matthewclaude/somnus-waveshare-rotary-dial --limit 5`).

## Public releases repo (`matthewclaude/somnus-dial-releases`)

Polled `GET /repos/matthewclaude/somnus-dial-releases/releases/tags/somnus-v0.1.5-beta.5`:

```
00:14:47 poll 1: 404
00:15:17 poll 2: 200
```

Release detail:

```
tag: somnus-v0.1.5-beta.5 | prerelease: True | draft: False | published: 2026-09-05T00:14:40Z
url: https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.5-beta.5
  asset: somnus-dial-merged.bin 1740944 bytes
  asset: somnus-dial.bin 1609872 bytes
```

Release body is the CHANGELOG 0.1.5-beta.5 section (battery percentage, About redesign,
steadier battery reading), as expected from the workflow's changelog extraction.

Full release listing (REST API, `per_page=30`; API sorts stable before prerelease):

```
somnus-v0.1.4        | stable | 2026-09-03T16:17:47Z
somnus-v0.1.3        | stable | 2026-09-02T20:36:17Z
somnus-v0.1.2        | stable | 2026-09-02T03:27:57Z
somnus-v0.1.1        | stable | 2026-09-02T02:57:55Z
somnus-v0.1.0        | stable | 2026-09-02T02:26:51Z
somnus-v0.1.5-beta.5 | pre    | 2026-09-05T00:14:40Z   <- new
somnus-v0.1.5-beta.4 | pre    | 2026-09-04T02:09:06Z
somnus-v0.1.5-beta.3 | pre    | 2026-09-04T00:55:47Z
somnus-v0.1.5-beta.2 | pre    | 2026-09-03T20:26:54Z
somnus-v0.1.5-beta.1 | pre    | 2026-09-03T19:12:28Z
```

## gh-pages beta channel

```
HEAD https://matthewclaude.github.io/somnus-dial-releases/firmware/beta/somnus-dial-merged.bin
HTTP/2 200
last-modified: Sat, 05 Sep 2026 00:15:12 GMT
content-length: 1740944
```

Content-length matches the `somnus-dial-merged.bin` release asset (1740944 bytes), so the
browser flasher's beta manifest now serves the beta.5 image. The stable channel
(`firmware/latest/`) was untouched: still 1678432 bytes, last-modified 2026-09-04 02:09 GMT
(the v0.1.4 image), as the workflow's channel separation intends.

## Outstanding item resolved

After `gh auth login` (account `matthewclaude`, keyring), ran:

```
gh run list --repo matthewclaude/somnus-waveshare-rotary-dial --limit 5
```

```
completed  success  feat(power): battery percentage + About screen redesign (0.1.5-beta.5)  release  somnus-v0.1.5-beta.5  push  33932029987  5m28s  2026-09-05T00:09:26Z
completed  success  release: 0.1.5-beta.4                                                   release  somnus-v0.1.5-beta.4  push  33828089795  5m44s  2026-09-04T02:03:36Z
completed  success  release: 0.1.5-beta.3                                                   release  somnus-v0.1.5-beta.3  push  33823415650  5m27s  2026-09-04T00:50:37Z
completed  success  release: 0.1.5-beta.2                                                   release  somnus-v0.1.5-beta.2  push  33801718212  6m16s  2026-09-03T20:20:52Z
completed  success  night window: ASCII hyphens; the compiled fonts have no dash glyphs     release  somnus-v0.1.5-beta.1  push  33794603843  5m3s   2026-09-03T19:07:48Z
```

Run triggered by the `somnus-v0.1.5-beta.5` tag push:

| Field | Value |
|---|---|
| Workflow | `release` |
| Run ID | 33932029987 |
| Event / ref | `push` / `somnus-v0.1.5-beta.5` |
| Status | `completed` |
| Conclusion | `success` |
| Created | 2026-09-05T00:09:26Z |
| Finished | 2026-09-05T00:14:54Z (5m28s) |
| URL | https://github.com/matthewclaude/somnus-waveshare-rotary-dial/actions/runs/33932029987 |

The run's finish time (00:14:54Z) lines up with the release publish (00:14:40Z) and the
gh-pages beta deploy (00:15:12Z) recorded above. Nothing remains outstanding for this
release check.
