# REPORT: 0.1.5-beta.5 tag + push (step 2 of 2)

**Verdict: TAGGED AND PUSHED to the private fork only. Tag `somnus-v0.1.5-beta.5` -> `afd23fffbee543d2de4ae9eb6becfda6ed55ad4c`, on remote `somnus` (matthewclaude fork). Origin untouched. CI status NOT checked — gh CLI is not installed on this machine.**

Date: 2026-09-04
Branch: `firmware/somnus-port`

## Pre-flight checks (all passed)

| Check | Command | Result |
|---|---|---|
| HEAD is the beta.5 commit | `git --no-optional-locks log --oneline -1` | `afd23ff feat(power): battery percentage + About screen redesign (0.1.5-beta.5)` |
| PROJECT_VER matches tag | `grep PROJECT_VER firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "0.1.5-beta.5")` |
| Prior tags are lightweight | `git cat-file -t <tag>` for every `somnus-v*` | all 9 (`somnus-v0.1.0` … `somnus-v0.1.5-beta.4`) report `commit`, i.e. lightweight |
| No pre-existing beta.5 tag | `git tag -l somnus-v0.1.5-beta.5` | empty |

## Remotes (`git remote -v`)

```
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git (fetch)
somnus	git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git (push)
```

The private fork remote is still named `somnus` and points to
`git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git`. Origin's push URL is
set to `no_push`, so origin could not have been pushed to even by accident. Nothing was
pushed to origin.

## Tag

```
git tag somnus-v0.1.5-beta.5
```

- Tag: `somnus-v0.1.5-beta.5`
- Type: lightweight (`git cat-file -t` -> `commit`)
- Points to: `afd23fffbee543d2de4ae9eb6becfda6ed55ad4c`

## Push (branch and tag, by name, to `somnus` only)

```
$ git push somnus firmware/somnus-port
To github.com:matthewclaude/somnus-waveshare-rotary-dial.git
   179a351..afd23ff  firmware/somnus-port -> firmware/somnus-port
exit=0

$ git push somnus somnus-v0.1.5-beta.5
To github.com:matthewclaude/somnus-waveshare-rotary-dial.git
 * [new tag]         somnus-v0.1.5-beta.5 -> somnus-v0.1.5-beta.5
exit=0
```

Note: the branch push range is `179a351..afd23ff`, which means two commits went up:
`490a3ec` (docs: power sensing section 10 built, verified, shipped as beta.4 — had not
been pushed previously) and `afd23ff` (the beta.5 feature commit). Both are on
`firmware/somnus-port` and both were already in local history; nothing new was created
beyond the tag.

No `--all`, `--tags`, or `--mirror` was used.

## Remote verification (`git ls-remote somnus …`)

```
afd23fffbee543d2de4ae9eb6becfda6ed55ad4c	refs/heads/firmware/somnus-port
afd23fffbee543d2de4ae9eb6becfda6ed55ad4c	refs/tags/somnus-v0.1.5-beta.5
```

Both refs on the fork resolve to the beta.5 commit.

## CI check

**Not checked — gh unavailable.** `gh` is not installed on this machine
(`command not found: gh`), so `gh run list` could not be run. No claim is made about
whether `.github/workflows/release.yml` picked up the tag or whether the release built.

For reference, `release.yml` in this repo is documented as triggering on a
`somnus-vX.Y.Z` tag push, building `firmware/dial-idf`, and publishing the Release
object and gh-pages artifacts into the public `matthewclaude/somnus-dial-releases` repo
via `SOMNUS_RELEASES_TOKEN`. Confirm manually at:

- https://github.com/matthewclaude/somnus-waveshare-rotary-dial/actions
- https://github.com/matthewclaude/somnus-dial-releases/releases

(or install gh and run `gh run list --repo matthewclaude/somnus-waveshare-rotary-dial --limit 3`).

## Working tree

Unchanged by this step. Still untracked (deliberately, per report convention):
`Claude outputs/`, `docs/PLAN-screen-layout-fixes.md`, all `docs/REPORT-*.md` including
`docs/REPORT-0.1.5-beta.5-commit.md` (then named `REPORT-beta5-commit.md`) and this file.
