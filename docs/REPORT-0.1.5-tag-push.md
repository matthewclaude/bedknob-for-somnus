# REPORT: 0.1.5 stable tag + push

**Verdict: TAGGED AND PUSHED. Lightweight tag `somnus-v0.1.5` -> `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56`, branch and tag pushed by name to remote `somnus` (matthewclaude private fork) only. Origin untouched, push URL still `no_push`.**

Date: 2026-09-05 (UTC) / 2026-09-04 evening local
Branch: `firmware/somnus-port`

## Gate checks (all passed)

| # | Check | Required | Observed | Result |
|---|---|---|---|---|
| 1 | `git rev-parse HEAD` | `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56` | `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56` | PASS |
| 2 | `PROJECT_VER` in `firmware/dial-idf/CMakeLists.txt` | `0.1.5` | `0.1.5` | PASS |
| 3 | `git tag -l "somnus-v*"` contains `somnus-v0.1.5` | must NOT | absent | PASS |
| 4 | `git remote get-url --push origin` | `no_push` | `no_push` | PASS |
| 5 | `git cat-file -t somnus-v0.1.4` / `somnus-v0.1.5-beta.5` | `commit` (lightweight) | `commit` / `commit` | PASS |

Target remote confirmed before pushing:
`git remote get-url --push somnus` -> `git@github.com:matthewclaude/somnus-waveshare-rotary-dial.git`

## Tag

```
git tag somnus-v0.1.5
```

- `git rev-parse somnus-v0.1.5` -> `1f527bb5c4aedd0b3ad050bed52dd8897fc6df56` (matches HEAD / gate 1)
- `git cat-file -t somnus-v0.1.5` -> `commit` (lightweight, matching convention)

## Push (by name, `somnus` remote only)

```
$ git push somnus firmware/somnus-port
To github.com:matthewclaude/somnus-waveshare-rotary-dial.git
   afd23ff..1f527bb  firmware/somnus-port -> firmware/somnus-port
exit=0

$ git push somnus somnus-v0.1.5
To github.com:matthewclaude/somnus-waveshare-rotary-dial.git
 * [new tag]         somnus-v0.1.5 -> somnus-v0.1.5
exit=0
```

Push range `afd23ff..1f527bb`: exactly one new commit (the graduation commit) on top of
the beta.5 commit that was already on the remote. No `--all`, `--tags`, or `--mirror`.

Remote verification (`git ls-remote somnus …`):

```
1f527bb5c4aedd0b3ad050bed52dd8897fc6df56	refs/heads/firmware/somnus-port
1f527bb5c4aedd0b3ad050bed52dd8897fc6df56	refs/tags/somnus-v0.1.5
```

## Post-push origin re-check

`git remote get-url --push origin` -> `no_push`. Unchanged; nothing touched origin.

## What happens next (not performed here)

The `somnus-v*` tag push triggers `.github/workflows/release.yml` on the private fork.
Because the tag has no `-beta.` suffix it is classified STABLE: `prerelease=false`,
Pages dir `firmware/latest/`. On success the Release `somnus-v0.1.5` appears on
`matthewclaude/somnus-dial-releases` and the browser flasher's default manifest starts
serving the 0.1.5 merged image. Verify with:

```
gh run list --repo matthewclaude/somnus-waveshare-rotary-dial --limit 3
gh release list --repo matthewclaude/somnus-dial-releases --limit 3
```

Field consequence once published: dials on `0.1.4` with Beta builds off, and dials on
any `0.1.5-beta.N`, pick up `0.1.5` on their next check. A `0.1.4` dial with Beta builds
ON still runs the old five-entry list scan (SPEC-ota-readiness §9.9); it most likely sees
`0.1.5` at the top of that list, but if it reports up to date, turning Beta builds off
makes the check deterministic via `/releases/latest`. §9.9's open item is resolved by this
release and can be closed out in the SPEC in a follow-up.
