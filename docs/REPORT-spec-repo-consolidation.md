# REPORT: SPEC-repo-consolidation — one public repo for source, releases and flasher

**DONE** — `docs/SPEC-repo-consolidation.md` written (spec only; no code, no workflow edits, no tag, no push), this report, and one line in `docs/REPORTS.md`. Nothing under `firmware/`, `.github/` or `web-flasher/` changed.

Date: 2026-09-10. Parent commit (HEAD before this commit): `2619c5738a7016594a1be69c848ed1597552549d`.

## Gate (all five passed)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.0")
$ git tag -l somnus-v1.0.0
somnus-v1.0.0
$ git tag -l 'somnus-v1.0.1*'
(no output)
$ git --no-optional-locks status --short --untracked-files=no
(no output)
$ git rev-parse --abbrev-ref HEAD
main
$ git remote get-url --push origin
no_push
$ test ! -e docs/SPEC-repo-consolidation.md && echo absent-ok
absent-ok
```

## Premise verification (raw output)

### §2 — dial_ota.c

```
$ grep -n 'somnus-dial-releases' firmware/dial-idf/components/dial_ota/dial_ota.c
32:// publishing to somnus-dial-releases because every shipped dial resolves
37:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
45:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
49:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
303:    // matthewclaude/somnus-dial-releases repo, not a check failure. Report

$ grep -n 'GITHUB_API_URL\|ASSET_NAME\|TAG_PREFIX\|OTA_BETA_CANDIDATE_CAP\|browser_download_url\|freshly created\|per_page' dial_ota.c   (selected)
36:#define GITHUB_API_URL \
44:#define GITHUB_API_URL_TAGS \
48:#define GITHUB_API_URL_RELEASE_BY_TAG_FMT \
50:#define ASSET_NAME     "somnus-dial.bin"
59:#define TAG_PREFIX     "somnus-v"
66:#define OTA_BETA_CANDIDATE_CAP 3
269:        if (!cJSON_IsString(name) || strcmp(name->valuestring, ASSET_NAME) != 0) continue;
270:                cJSON *url = cJSON_GetObjectItem(a, "browser_download_url");
302:    // releases -- expected right now for the freshly created
303:    // matthewclaude/somnus-dial-releases repo, not a check failure. Report
502:    if (!called_finish) set_status(OTA_FAILED, NULL, "no usable release for newest tags");
```

Header comment above `GITHUB_API_URL` is lines 30–35 and reads "The source repo is public too (since 2026-09-10), but releases keep publishing to somnus-dial-releases because every shipped dial resolves updates by exactly this URL; consolidating releases into the source repo is a separate, planned step (the OTA repoint)". `check_stable()` 404 branch (lines 306–311) logs "no releases published yet (HTTP 404)" and sets `OTA_IDLE`. `check_beta()` header comment lines 347–357 describes steps 1–3 as the spec states them; the cap-exhausted status is line 502.

### §2 — dial_state.h

```
$ grep -n 'somnus-waveshare-rotary-dial\|DIAL_CERT_ERR_MSG\|github.com' firmware/dial-idf/components/dial_state/dial_state.h
353: * The repo line omits the "github.com/" host: at the error screen's sub
358:#define DIAL_CERT_ERR_MSG \
361:    "matthewclaude/somnus-waveshare-rotary-dial"
```

### §2 — certs.yml

```
$ grep -n 'api.github.com\|objects.githubusercontent.com\|cron' .github/workflows/certs.yml
3:# Only dial_ota makes TLS connections (to api.github.com and
4:# objects.githubusercontent.com, for the release check and binary download);
22:    - cron: '17 6 1 * *'   # monthly, 1st at 06:17 UTC
52:          for h in api.github.com objects.githubusercontent.com; do
```

### §2 — shipped identifiers

```
$ grep -n 'project(' firmware/dial-idf/CMakeLists.txt
22:project(somnus-dial)
$ grep -n 'somnus-dial-merged' .github/workflows/release.yml
6, 7, 84, 117, 122, 123, 135, 146, 147, 166   (merge-bin at 117, release file at 135, Pages copy at 166)
```

### §3.2 — tag count

```
$ git tag -l 'somnus-v*' | wc -l
16
```

(0.1.0, 0.1.1, 0.1.2, 0.1.3, 0.1.4, 0.1.5, 0.1.5-beta.1–5, 0.1.6, 0.1.6-beta.1–3, 1.0.0.)

### §3.2(b) — tag-to-Release window

From `docs/REPORT-1.0.0-*.md`: release run 34412911352 for `somnus-v1.0.0`, job 5m45s, run 6m2s. "~6 minutes" holds.

### §4 — release.yml

```
$ grep -n 'repository: matthewclaude/somnus-dial-releases\|external_repository\|matthewclaude.github.io/somnus-dial-releases\|contents: write\|permissions\|private\|keep_files\|SOMNUS_RELEASES_TOKEN' .github/workflows/release.yml
11:# private -- but the Release object and the gh-pages site are published into
14:# unauthenticated clients and GitHub 404s everything under a private repo to
16:# push) use the SOMNUS_RELEASES_TOKEN secret -- a fine-grained PAT scoped to
27:permissions:
28:  contents: write
73:          # THIS (private) repo, but the release/notes get published into
76:          # built from ${GITHUB_REPOSITORY} would point at a private repo
84:            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); ..."
129:          repository: matthewclaude/somnus-dial-releases
130:          token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
177:          personal_token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
178:          external_repository: matthewclaude/somnus-dial-releases
184:          keep_files: true
```

"Four comments describing this repo stays private": `grep -n private` gives exactly four lines (11, 14, 73, 76), in two comment blocks. Written as such.

### §4(b) — the target gh-pages branch (found, not a premise)

```
$ git log --format='%h %ad %s' --date=short origin/gh-pages | head -3
e2f3b97 2026-08-05 deploy: 4a32427b56c89e2f90b9c9b52f9779187566c9b6
fcc2e78 2026-08-05 deploy: 583f8b546c7135c6ef379b616649e4828a5d39ff
ee1d23c 2026-08-05 deploy: 94ad2fac426285283a3f5e463b5af06756b00e84
$ git log origin/gh-pages | grep -c '^commit'
12
$ git log -1 --format='%h %ad %s' --date=short 4a32427
4a32427 2026-08-05 firmware(dial-idf): v1.4.2 — Rotation fix promoted to stable
$ git ls-tree -r --name-only origin/gh-pages
.nojekyll
firmware/beta/orion-dial-merged.bin
firmware/latest/orion-dial-merged.bin
index.html
manifest-beta.json
manifest.json
```

### §5 — web-flasher/index.html

```
$ grep -n 'releases/latest\|somnus-dial-releases' web-flasher/index.html
280:      <a href="https://github.com/matthewclaude/somnus-dial-releases">Releases</a>
282:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/LICENSE">License</a>
288:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES">third-party
304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
```

### "item 13"

```
$ grep -rln 'item 13\|Item 13\|§13\|## 13' docs/
docs/SPEC-voice.md
docs/REPORT-screen-layout-audit.md
```

Neither is the OTA finding. NAMING.md's numbered list ends at item 6. "Item 13" is not resolvable on disk; the on-disk record of that failure shape is `docs/REPORT-ota-beta-not-found.md` (2026-09-05 capture: eight `ota:` lines all "up to date" while beta.5 sat unseen; fix `819f102` on 2026-09-03). The spec cites that report by path instead of "item 13".

## git diff --stat

```
$ git diff --stat HEAD   (with the three files staged)
  docs/REPORT-spec-repo-consolidation.md | 186 +++++++++++++++++++++++++++++++++
  docs/REPORTS.md                        |   1 +
  docs/SPEC-repo-consolidation.md        | 160 ++++++++++++++++++++++++++++
  3 files changed, 347 insertions(+)
```

## Deviations

1. §3.2(c) and §7(3): "item 13" replaced by a citation of `docs/REPORT-ota-beta-not-found.md`, because the item number does not resolve to anything in this repo (see above).
2. §6, the one-way door: the task's premise was that a `1.0.0` dial that never installs during the window "is stranded on the old repo after cut-over and needs one wire flash". Written with the qualification that an archived repo still answers reads, so the dual-published `1.0.1` Release on the archived old repo remains a permanent over-the-air path; the dial is stranded only if the old repo is deleted/renamed, that Release is removed, or GitHub changes what archived repos serve. The spec keeps the wire-flash instruction for those cases, keeps the "window matters" argument, and adds a standing rule to Phase 6 that the old repo's `1.0.1` Release is never deleted. Flagged for the owner rather than written as stated, because writing the stronger claim would have been writing something disk and GitHub's documented archive behaviour do not support.
3. §1 and §4(b): the premise "holds no Releases and no Pages site" is written as "no Releases, serves no Pages site, but carries an inherited `gh-pages` branch" — see False premises. Consequences (keep_files leaves two stale upstream images; a hand cleanup listed as Phase 6 item 6; §8 question 4) added.
4. §4(d): added a note that the release-notes step's "no compare link" reasoning also stops being true once Releases live in the source repo. Comment-only observation, no task.
5. §8: two owner questions added beyond the three specified (inherited gh-pages cleanup timing; Pages enabled before vs after the tag), both arising from the branch finding.
6. Added a §9 "Not in this spec", matching the shape of `SPEC-standby-poll.md`.

## False premises

1. **"the source repo … holds no Releases and no Pages site."** Half true. No Releases: not verifiable from disk (a GitHub-side fact), stated as given. No Pages site: whether Pages is *enabled* is also GitHub-side and stated as given, but `origin/gh-pages` **exists**, with 12 `deploy:` commits from the upstream fork's own release workflow, the last on 2026-08-05 for upstream v1.4.2, containing the upstream flasher page and two `orion-dial-merged.bin` files. The spec says so in §1 and §4(b).
2. **"the three #define continuation lines near 36/44/48".** The `#define` lines are 36/44/48; the continuation lines carrying the repo name are 37/45/49. "Near" holds; the spec cites the exact lines.
3. **"item 13"** — not on disk under that name (see Deviations 1).
4. **"expected right now for the freshly created matthewclaude/somnus-dial-releases repo"** — present, but split across lines 302–303. Quoted correctly in the spec.

Everything else matched: all three URL constants, `ASSET_NAME`, `TAG_PREFIX`, cap = 3, `browser_download_url` at line 270, certs.yml's two hosts, `permissions: contents: write` at lines 27–28, the two cross-repo steps at 129 and 178, the footer at line 84, the flasher's fetch at 304 and footer links at 280/282/288, 16 tags, `DIAL_CERT_ERR_MSG` at 358–361 still naming the pre-rename source repo with the width comment at 353–355.

## Not verifiable without hardware

Nothing — no code changed. Two GitHub-side facts (no Releases in `bedknob-for-somnus`; Pages not enabled there) were taken from the task, not verified, since this block is offline and read-only against the remote.
