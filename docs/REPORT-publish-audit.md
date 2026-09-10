# REPORT: pre-publication audit (read-only)

**CLEAR TO PUBLISH** — the secret scan is clean at HEAD and across all history: the only pattern hit is a masked `gho_****…` token inside a quoted `gh auth status` transcript in a historical report, the one tracked `.pem` is an 18-certificate CA trust bundle with no private key, and no `.env`/key/credential files are tracked. Items 2–5 are informational: the repo is not a GitHub fork (`isFork: false`), the releases-repo name appears in 135 places of which 4 are compiled firmware code (three URL constants in `dial_ota.c` plus one comment), the tree is 250 files with nothing over 5 MB and no unexpected binaries, and no absolute home paths remain outside the historical reports.

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
HEAD at audit time: `68b854de1fe5fd450a8c2b4354a2f7e87ae8549e` = `somnus/main` (tree clean, nothing untracked)

Nothing was modified, staged, committed or pushed during the audit itself; the only write in this task is the commit carrying this report and its `docs/REPORTS.md` line.

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

## Item 1 — secret scan

Raw output of every command, HEAD first, then history, then scanner availability. A `(exit 1)` after a `git grep` means no matches.

```
$ git grep -n -iE "(ghp_[A-Za-z0-9]{20,}|github_pat_|gho_|ghs_|glpat-|xox[bp]-|AKIA[0-9A-Z]{16}|sk-[A-Za-z0-9]{20,})"
docs/REPORT-releases-repo-readme.md:19:  - Token: gho_************************************
(exit 0)

$ git grep -n -E "BEGIN (RSA |EC |OPENSSH |DSA )?PRIVATE KEY"
(exit 1)

$ git grep -n -iE "(password|passwd|passphrase|secret|api[_-]?key|token)\s*[:=]\s*['\"][^'\"]{4,}" -- . ':!docs' ':!CHANGELOG*' ':!THIRD_PARTY_LICENSES.md'
(exit 1)

$ git grep -n -iE "CONFIG_(EXAMPLE_)?WIFI_(SSID|PASSWORD)|ESP_WIFI_(SSID|PASS)" -- firmware/
(exit 1)

$ git ls-files | grep -iE "(^|/)\.env($|\.)|\.pem$|\.key$|\.p12$|\.pfx$|id_rsa|id_ed25519|\.netrc|\.npmrc|\.pypirc"
firmware/dial-idf/components/dial_ota/trust_roots.pem
(exit 0)

=== .pem identification ===
-- firmware/dial-idf/components/dial_ota/trust_roots.pem --
BEGIN markers: 18  CERTIFICATE: 18  PRIVATE KEY: 0  size:    28198 bytes
# Curated CA trust anchors for dial_ota's embedded TLS verification
# (api.github.com + objects.githubusercontent.com, GitHub's release-asset
# CDN).

$ git log --all -p --no-color | grep -n -iE "(ghp_[A-Za-z0-9]{20,}|github_pat_|BEGIN (RSA |EC |OPENSSH |DSA )?PRIVATE KEY|AKIA[0-9A-Z]{16})" | head -20
(exit 0)

$ which gitleaks trufflehog
(exit 1)

```

Reading of the results:

- **Pattern 1 (token prefixes):** one hit, `docs/REPORT-releases-repo-readme.md:19`. It is the line `- Token: gho_************************************` quoted from `gh auth status` output in the 2026-09-04 report. The `gh` CLI masks the token in that output; only the four-character prefix and asterisks are present, so nothing usable is in the file. Not a secret.
- **Pattern 2 (private-key PEM headers):** no matches.
- **Pattern 3 (password/secret/api-key/token assignments, excluding docs, CHANGELOG and the licenses file):** no matches.
- **Pattern 4 (ESP-IDF Wi-Fi SSID/password Kconfig symbols under firmware/):** no matches.
- **Pattern 5 (credential-shaped filenames):** one hit, `firmware/dial-idf/components/dial_ota/trust_roots.pem`. It is the expected CA trust bundle: 18 `BEGIN CERTIFICATE` blocks, 0 `PRIVATE KEY` markers, 28,198 bytes, header comment "Curated CA trust anchors for dial_ota's embedded TLS verification". Public root certificates; fine to publish.
- **History (`git log --all -p`):** no matches for token prefixes, private-key headers or AWS key IDs anywhere in any commit on any ref.
- **Scanners:** neither `gitleaks` nor `trufflehog` is installed (`which` returned nothing, exit 1). Not installed, per the task.

**SECRET SCAN CLEAN.** (The one regex hit is listed above with file and line so it can be re-checked; it is a masked value, not a credential.)

## Item 2 — is it a fork

```
$ gh repo view matthewclaude/bedknob-for-somnus --json isFork,parent,visibility,defaultBranchRef,hasIssuesEnabled,hasWikiEnabled,hasProjectsEnabled
{"defaultBranchRef":{"name":"main"},"hasIssuesEnabled":true,"hasProjectsEnabled":true,"hasWikiEnabled":false,"isFork":false,"parent":null,"visibility":"PRIVATE"}
(exit 0)

$ git remote -v
origin	https://github.com/chris023/orion-waveshare-rotary-dial.git (fetch)
origin	no_push (push)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (fetch)
somnus	git@github.com:matthewclaude/bedknob-for-somnus.git (push)
(exit 0)

$ gh api repos/matthewclaude/bedknob-for-somnus --jq .security_and_analysis

(exit 0)

$ gh api repos/matthewclaude/bedknob-for-somnus --jq "{private,visibility,fork,archived,default_branch}"
{"archived":false,"default_branch":"main","fork":false,"private":true,"visibility":"private"}
(exit 0)
```

`isFork` is **false** and `parent` is null: GitHub does not treat `matthewclaude/bedknob-for-somnus` as a fork of anything, so flipping visibility will not expose it on upstream's fork network and its issues/PRs stay its own. The relationship to `chris023/orion-waveshare-rotary-dial` exists only as the local `origin` fetch remote (push URL `no_push`) and in the README/licence text. Visibility is currently PRIVATE; issues and projects are enabled, wiki disabled.

## Item 3 — where the releases repo is named

Full raw output (135 hits), then per-file counts:

```
.github/workflows/release.yml:12:# the public matthewclaude/somnus-dial-releases repo instead (see
.github/workflows/release.yml:74:          # matthewclaude/somnus-dial-releases (see top-of-file comment),
.github/workflows/release.yml:84:            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); \`somnus-dial-merged.bin\` below is the same image for flashing manually from offset 0x0."
.github/workflows/release.yml:129:          repository: matthewclaude/somnus-dial-releases
.github/workflows/release.yml:178:          external_repository: matthewclaude/somnus-dial-releases
CHANGELOG.md:30:the browser at https://matthewclaude.github.io/somnus-dial-releases/, join
README.md:53:**Open [matthewclaude.github.io/somnus-dial-releases](https://matthewclaude.github.io/somnus-dial-releases/)
README.md:212:[matthewclaude/somnus-dial-releases](https://github.com/matthewclaude/somnus-dial-releases),
docs/ARCHITECTURE.md:198:`matthewclaude/somnus-dial-releases` over the GitHub API — `/releases/latest`
docs/NAMING.md:46:   `BedknobMac`. GitHub redirects the old names. `somnus-dial-releases` stays: its URL is
docs/REPORT-0.1.5-beta.5-ci-check.md:3:**Verdict: RELEASE CONFIRMED BUILT AND PUBLISHED. `somnus-v0.1.5-beta.5` is live as a prerelease on `matthewclaude/somnus-dial-releases` with both firmware assets, and the gh-pages beta channel serves the new merged image. Workflow run 33932029987 on the private fork confirmed via authenticated gh: `completed` / `success`.**
docs/REPORT-0.1.5-beta.5-ci-check.md:43:## Public releases repo (`matthewclaude/somnus-dial-releases`)
docs/REPORT-0.1.5-beta.5-ci-check.md:45:Polled `GET /repos/matthewclaude/somnus-dial-releases/releases/tags/somnus-v0.1.5-beta.5`:
docs/REPORT-0.1.5-beta.5-ci-check.md:56:url: https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.5-beta.5
docs/REPORT-0.1.5-beta.5-ci-check.md:82:HEAD https://matthewclaude.github.io/somnus-dial-releases/firmware/beta/somnus-dial-merged.bin
docs/REPORT-0.1.5-beta.5-tag-push.md:80:object and gh-pages artifacts into the public `matthewclaude/somnus-dial-releases` repo
docs/REPORT-0.1.5-beta.5-tag-push.md:84:- https://github.com/matthewclaude/somnus-dial-releases/releases
docs/REPORT-0.1.5-ci-check.md:3:**Verdict: RELEASE BUILT AND PUBLISHED AS STABLE. Workflow run 33935053719 completed/success in 5m20s; `somnus-v0.1.5` exists on `matthewclaude/somnus-dial-releases` with `isPrerelease: false`, `isDraft: false`, both assets attached; `/releases/latest` now returns `somnus-v0.1.5`.**
docs/REPORT-0.1.5-ci-check.md:67:## 3. `gh release view somnus-v0.1.5 --repo matthewclaude/somnus-dial-releases`
docs/REPORT-0.1.5-ci-check.md:78:url:	https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.5
docs/REPORT-0.1.5-ci-check.md:97:## 4. `gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq '.tag_name'`
docs/REPORT-0.1.5-tag-push.md:63:`matthewclaude/somnus-dial-releases` and the browser flasher's default manifest starts
docs/REPORT-0.1.5-tag-push.md:68:gh release list --repo matthewclaude/somnus-dial-releases --limit 3
docs/REPORT-0.1.6-graduation.md:323:`matthewclaude/somnus-dial-releases` is `isPrerelease: false`, `isDraft: false`, both assets
docs/REPORT-0.1.6-graduation.md:328:the public `somnus-dial-releases` repo (see Deviations 1).
docs/REPORT-0.1.6-graduation.md:514:🎉 Release ready at https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.6
docs/REPORT-0.1.6-graduation.md:526:| Release URL | https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.6 |
docs/REPORT-0.1.6-graduation.md:540:**Against the repo the workflow actually publishes to (`matthewclaude/somnus-dial-releases`, per
docs/REPORT-0.1.6-graduation.md:545:{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362458","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:a353d2cbfc111f4b6a7820ad7c0c4f613b9a9249021c312c30c53452fa06a3d9","downloadCount":0,"id":"RA_kwDOULeAcc4gvpsa","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362457","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:89449a1e66999d43a73ecd95efec158d2853fec85cd2aeb7bf36f0908a523666","downloadCount":0,"id":"RA_kwDOULeAcc4gvpsZ","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"tagName":"somnus-v0.1.6"}
docs/REPORT-0.1.6-graduation.md:561:$ curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin | head -8
docs/REPORT-0.1.6-graduation.md:574:`gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json body --jq .body`
docs/REPORT-0.1.6-graduation.md:586:> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
docs/REPORT-0.1.6-graduation.md:662:For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
docs/REPORT-0.1.6-graduation.md:669:1. **Verification ran against `matthewclaude/somnus-dial-releases`, not `bedknob-for-somnus`, for
docs/REPORT-0.1.6-graduation.md:673:   the public `somnus-dial-releases` repo (header comment lines 10–19, `repository:` on the
docs/REPORT-1.0.0-commit.md:32:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
docs/REPORT-1.0.0-commit.md:34:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
docs/REPORT-1.0.0-commit.md:36:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
docs/REPORT-1.0.0-commit.md:58:the browser at https://matthewclaude.github.io/somnus-dial-releases/, join
docs/REPORT-1.0.0-commit.md:193:the browser at https://matthewclaude.github.io/somnus-dial-releases/, join
docs/REPORT-1.0.0-tag-push.md:121:## Step 5 — published release (releases repo matthewclaude/somnus-dial-releases)
docs/REPORT-1.0.0-tag-push.md:124:$ gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,publishedAt,assets
docs/REPORT-1.0.0-tag-push.md:127:$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
docs/REPORT-1.0.0-tag-push.md:130:$ curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin | head -1
docs/REPORT-1.0.0-tag-push.md:142:`gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json body --jq .body` saved, and diffed against the CHANGELOG section extracted with release.yml's awk (`ver=1.0.0`):
docs/REPORT-1.0.0-tag-push.md:149:> For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); `somnus-dial-merged.bin` below is the same image for flashing manually from offset 0x0.
docs/REPORT-1.0.0-tag-push.md:159:84:            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); \`somnus-dial-merged.bin\` below is the same image for flashing manually from offset 0x0."
docs/REPORT-handoff.md:19:both assets on `matthewclaude/somnus-dial-releases`; `/releases/latest` is
docs/REPORT-readme-license-fix.md:185:- **Public releases repo README (`matthewclaude/somnus-dial-releases`).** Not on disk. Checked instead with `gh api repos/matthewclaude/somnus-dial-releases/readme` (blob sha `046158dfc671edb88ee8b5d26b535f1e078d66ac`): the served `README.md` is **byte-identical** to this repo's `web-flasher/RELEASES-README.md` (`diff` clean). So it carries the same already-correct text: Somnus Lab disclaimer, "License and attribution" section, no commercial-license offer, no Contributing section, and the line "If you want the source, ask the maintainer." (that is "maintainer", not the task's "project maintainer" string, and it is about obtaining the source, not licensing). No change is needed there for this task; it would need its own hand-maintained update if the owner later changes the wording here.
docs/REPORT-release-0.1.6-beta.1.md:1574:## Release — `gh release view somnus-v0.1.6-beta.1 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets`
docs/REPORT-release-0.1.6-beta.1.md:1580:      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546402673",
docs/REPORT-release-0.1.6-beta.1.md:1591:      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.1/somnus-dial-merged.bin"
docs/REPORT-release-0.1.6-beta.1.md:1594:      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546402674",
docs/REPORT-release-0.1.6-beta.1.md:1605:      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.1/somnus-dial.bin"
docs/REPORT-release-0.1.6-beta.1.md:1624:$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
docs/REPORT-release-0.1.6-beta.2.md:1525:## Release — `gh release view somnus-v0.1.6-beta.2 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets`
docs/REPORT-release-0.1.6-beta.2.md:1531:      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546487234",
docs/REPORT-release-0.1.6-beta.2.md:1542:      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.2/somnus-dial-merged.bin"
docs/REPORT-release-0.1.6-beta.2.md:1545:      "apiUrl": "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/546487233",
docs/REPORT-release-0.1.6-beta.2.md:1556:      "url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.2/somnus-dial.bin"
docs/REPORT-release-0.1.6-beta.2.md:1575:$ gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq .tag_name
docs/REPORT-release-0.1.6-beta.3.md:154:$ gh release view somnus-v0.1.6-beta.3 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,isDraft,assets
docs/REPORT-release-0.1.6-beta.3.md:155:{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/547256069","contentType":"application/octet-stream","createdAt":"2026-09-06T13:56:13Z","digest":"sha256:4c830d5ee8fdfc05bfab9e1846955be8f70f615e54b8bf7b90e769b5abafbf0d","downloadCount":0,"id":"RA_kwDOULeAcc4gnncF","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-06T13:56:14Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.3/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/547256070","contentType":"application/octet-stream","createdAt":"2026-09-06T13:56:13Z","digest":"sha256:ca8d3f5f10da43582d8d767ae624460f6c42674d626d6df735723fd5a75f4239","downloadCount":0,"id":"RA_kwDOULeAcc4gnncG","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-06T13:56:14Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6-beta.3/somnus-dial.bin"}],"isDraft":false,"isPrerelease":true,"tagName":"somnus-v0.1.6-beta.3"}
docs/REPORT-release-latest-pointer.md:7:**YES.** somnus-v1.0.0 is the release GitHub serves as "latest" for matthewclaude/somnus-dial-releases.
docs/REPORT-release-latest-pointer.md:24:### 1. `gh release list --repo matthewclaude/somnus-dial-releases --limit 20`
docs/REPORT-release-latest-pointer.md:46:### 2. `gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,name,isLatest,isPrerelease,isDraft,createdAt,publishedAt,targetCommitish,assets`
docs/REPORT-release-latest-pointer.md:77:`gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,name,isPrerelease,isDraft,createdAt,publishedAt,targetCommitish,assets`
docs/REPORT-release-latest-pointer.md:80:{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/553715364","contentType":"application/octet-stream","createdAt":"2026-09-09T22:40:27Z","digest":"sha256:b5a6702e87bc75403806d00810be168a8b9c340979b84e390e6458569999021a","downloadCount":0,"id":"RA_kwDOULeAcc4hAQak","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-09T22:40:27Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.0/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/553715363","contentType":"application/octet-stream","createdAt":"2026-09-09T22:40:27Z","digest":"sha256:9ee90ab07cb32f630da3ffdc3ecfdfde2ec253688171d64e810bdaa6d3d06a9a","downloadCount":1,"id":"RA_kwDOULeAcc4hAQaj","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-09T22:40:27Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.0/somnus-dial.bin"}],"createdAt":"2026-09-05T02:41:05Z","isDraft":false,"isPrerelease":false,"name":"somnus-v1.0.0","publishedAt":"2026-09-09T22:40:28Z","tagName":"somnus-v1.0.0","targetCommitish":"main"}
docs/REPORT-release-latest-pointer.md:84:### 3. `gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json tagName,isLatest,isPrerelease,createdAt,publishedAt,targetCommitish`
docs/REPORT-release-latest-pointer.md:115:`gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,createdAt,publishedAt,targetCommitish,assets`
docs/REPORT-release-latest-pointer.md:118:{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362458","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:a353d2cbfc111f4b6a7820ad7c0c4f613b9a9249021c312c30c53452fa06a3d9","downloadCount":1,"id":"RA_kwDOULeAcc4gvpsa","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362457","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:89449a1e66999d43a73ecd95efec158d2853fec85cd2aeb7bf36f0908a523666","downloadCount":2,"id":"RA_kwDOULeAcc4gvpsZ","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial.bin"}],"createdAt":"2026-09-05T02:41:05Z","isPrerelease":false,"publishedAt":"2026-09-07T20:49:39Z","tagName":"somnus-v0.1.6","targetCommitish":"main"}
docs/REPORT-release-latest-pointer.md:122:### 4. `gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq '.tag_name, .created_at, .published_at, .target_commitish'`
docs/REPORT-release-latest-pointer.md:132:### 5. `gh api repos/matthewclaude/somnus-dial-releases/tags --paginate --jq '.[].name'`
docs/REPORT-release-latest-pointer.md:154:### 6. `curl -s https://matthewclaude.github.io/somnus-dial-releases/manifest.json`
docs/REPORT-release-latest-pointer.md:173:### 7. `curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin`
docs/REPORT-release-latest-pointer.md:210:$ curl -s -o $S/latest.bin https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin && wc -c < $S/latest.bin && shasum -a 256 $S/latest.bin
docs/REPORT-release-latest-pointer.md:252:280:      <a href="https://github.com/matthewclaude/somnus-dial-releases">Releases</a>
docs/REPORT-release-latest-pointer.md:254:282:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/LICENSE">License</a>
docs/REPORT-release-latest-pointer.md:256:288:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES">third-party
docs/REPORT-release-latest-pointer.md:261:304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
docs/REPORT-release-latest-pointer.md:274:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
docs/REPORT-release-latest-pointer.md:338:304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
docs/REPORT-release-latest-pointer.md:343:URL: `https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest`, displaying
docs/REPORT-release-latest-pointer.md:385:- **Without write access / repo contents:** the gh-pages branch of somnus-dial-releases was not
docs/REPORT-releases-repo-readme.md:4:Target: https://github.com/matthewclaude/somnus-dial-releases (public), branch `main` only.
docs/REPORT-releases-repo-readme.md:23:$ gh repo view matthewclaude/somnus-dial-releases --json name
docs/REPORT-releases-repo-readme.md:24:{"name":"somnus-dial-releases"}
docs/REPORT-releases-repo-readme.md:31:$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases
docs/REPORT-releases-repo-readme.md:32:Cloning into '/tmp/somnus-dial-releases'...
docs/REPORT-releases-repo-readme.md:48: # somnus-dial-releases
docs/REPORT-releases-repo-readme.md:89: Each [release](https://github.com/matthewclaude/somnus-dial-releases/releases)
docs/REPORT-releases-repo-readme.md:559:17:Open **[the flasher page](https://matthewclaude.github.io/somnus-dial-releases/)**
docs/REPORT-releases-repo-readme.md:600:To https://github.com/matthewclaude/somnus-dial-releases.git
docs/REPORT-releases-repo-readme.md:624:$ gh api repos/matthewclaude/somnus-dial-releases/readme --jq .content | base64 -d | head -5
docs/REPORT-releases-repo-readme.md:625:# somnus-dial-releases
docs/REPORT-releases-repo-readme.md:632:First line is `# somnus-dial-releases`, followed by the Bedknob paragraph.
docs/REPORT-releases-repo-readme.md:649:- The clone at `/tmp/somnus-dial-releases` was left in place.
docs/REPORT-third-party-licenses-restore.md:3:**DONE** — the per-holder MIT copyright list and stb's "ALTERNATIVE B - Public Domain" text were recovered verbatim from the releases repo's `THIRD_PARTY_LICENSES` at `4097b6e` and restored into the firmware repo's `THIRD_PARTY_LICENSES.md` (commit `2207b1d` on `main`; the Apache-2.0 and OFL 1.1 blocks re-hash to their previous values), and the identical file was pushed to `matthewclaude/somnus-dial-releases` as `21c239d`. No firmware file touched. Firmware-repo push and CI: see the post-push addendum at the end.
docs/REPORT-third-party-licenses-restore.md:20:$ gh api "repos/matthewclaude/somnus-dial-releases/contents/THIRD_PARTY_LICENSES?ref=4097b6e" \
docs/REPORT-third-party-licenses-restore.md:141:$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases-mirror
docs/REPORT-third-party-licenses-restore.md:142:clone exit 0; origin = https://github.com/matthewclaude/somnus-dial-releases.git; HEAD 70c62990b171f1899f6b95a51bf3812667d57df5
docs/REPORT-third-party-licenses-restore.md:150:To https://github.com/matthewclaude/somnus-dial-releases.git
docs/REPORT-third-party-licenses-restore.md:155:Clone directory removed afterwards (confirmed absent). Post-push network check: `https://raw.githubusercontent.com/matthewclaude/somnus-dial-releases/main/THIRD_PARTY_LICENSES` is byte-identical to the firmware file (`cmp` clean).
docs/REPORT-third-party-licenses.md:3:**DONE** — `THIRD_PARTY_LICENSES.md` gains an `## ESP-IDF` section (v6.0, Espressif Systems (Shanghai) CO LTD, Apache-2.0) ahead of the managed-components table and a closing `## License texts` section with the full Apache License 2.0, SIL Open Font License 1.1 and MIT License texts, each taken from a file on disk or the named URL and hashed (commit `b0b0797` on `main`); the identical file was pushed to `matthewclaude/somnus-dial-releases` as `THIRD_PARTY_LICENSES` (commit `70c6299`, superseding a copy that had already diverged). No firmware file touched. Firmware-repo push and CI: see the post-push addendum at the end.
docs/REPORT-third-party-licenses.md:146:$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases-mirror
docs/REPORT-third-party-licenses.md:147:clone exit 0; remote origin = https://github.com/matthewclaude/somnus-dial-releases.git
docs/REPORT-third-party-licenses.md:207:$ git push origin main        (that clone's origin = matthewclaude/somnus-dial-releases)
docs/REPORT-third-party-licenses.md:208:To https://github.com/matthewclaude/somnus-dial-releases.git
docs/REPORT-third-party-licenses.md:213:Clone directory `/tmp/somnus-dial-releases-mirror` removed afterwards (confirmed absent).
docs/REPORT-third-party-licenses.md:245:- **Live flasher footer.** Checked over the network, not from disk: the served page at `https://matthewclaude.github.io/somnus-dial-releases/` (and the `gh-pages` branch's `index.html`, line 288) links the footer's "third-party" text to `https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES`, which returned HTTP 200 before the mirror push. Since the file kept its extensionless name and lives on `main`, that link now resolves to the updated file; the addendum records a post-push check of the raw content.
docs/REPORT-third-party-licenses.md:286:Public releases repo, checked over the network after the mirror push: `https://raw.githubusercontent.com/matthewclaude/somnus-dial-releases/main/THIRD_PARTY_LICENSES` is byte-identical to this repo's `THIRD_PARTY_LICENSES.md` (`cmp` clean; `## ESP-IDF` at line 105, `## License texts` at line 168), and the flasher footer's link target `https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES` returns HTTP 200. This resolves the first "Cannot verify from disk" item.
docs/REPORTS.md:51:- `REPORT-third-party-licenses.md` — 2026-09-10 — REPORT: third-party licenses — ESP-IDF attribution; Apache-2.0, OFL 1.1 and MIT texts appended; mirrored to the releases repo — **DONE** — `THIRD_PARTY_LICENSES.md` gains an ESP-IDF section (v6.0, Apache-2.0) and a closing License texts section with the full Apache-2.0 (from `~/esp/esp-idf/LICENSE`), OFL 1.1 (Montserrat's `OFL.txt`, header dropped) and MIT (LVGL's `LICENCE.txt`, placeholder copyright) texts, each sha256-recorded, as `b0b0797`; the identical file pushed to `somnus-dial-releases` as `70c6299`, superseding a public copy that had already diverged.
docs/REPORTS.md:52:- `REPORT-third-party-licenses-restore.md` — 2026-09-10 — REPORT: third-party licenses restore — per-holder MIT notices and stb Alternative B text, re-mirrored — **DONE** — the per-holder MIT copyright list and stb's public-domain Alternative B text, recovered verbatim from the releases repo's `4097b6e`, restored into `THIRD_PARTY_LICENSES.md` as `2207b1d` (Apache and OFL blocks re-hash unchanged) and mirrored byte-identical to `somnus-dial-releases` as `21c239d`.
docs/SPEC-ota-readiness.md:6:> flasher publish to the public `matthewclaude/somnus-dial-releases` via
docs/SPEC-ota-readiness.md:264:`matthewclaude/somnus-dial-releases` binaries-only repo. That board queried
docs/SPEC-ota-readiness.md:1115:delete the prerelease from `matthewclaude/somnus-dial-releases`.
docs/SPEC-ota-readiness.md:1236:`somnus-dial-releases` marked **Pre-release**, both assets attached. A dial
docs/SPEC-ota-readiness.md:1244:published. Every release in `somnus-dial-releases` points at the same commit
docs/SPEC-ota-readiness.md:1356:for the first five entries of the release list. `somnus-dial-releases`
firmware/dial-idf/README.md:13:[https://matthewclaude.github.io/somnus-dial-releases/](https://matthewclaude.github.io/somnus-dial-releases/)
firmware/dial-idf/README.md:116:[matthewclaude/somnus-dial-releases](https://github.com/matthewclaude/somnus-dial-releases))
firmware/dial-idf/README.md:224:  [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/)
firmware/dial-idf/components/dial_ota/dial_ota.c:35:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
firmware/dial-idf/components/dial_ota/dial_ota.c:43:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
firmware/dial-idf/components/dial_ota/dial_ota.c:47:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
firmware/dial-idf/components/dial_ota/dial_ota.c:301:    // matthewclaude/somnus-dial-releases repo, not a check failure. Report
web-flasher/README.md:4:(https://matthewclaude.github.io/somnus-dial-releases/). This project's
web-flasher/README.md:7:the public `matthewclaude/somnus-dial-releases` repo instead — see
web-flasher/RELEASES-README.md:1:# somnus-dial-releases
web-flasher/RELEASES-README.md:18:Open **[the flasher page](https://matthewclaude.github.io/somnus-dial-releases/)**
web-flasher/RELEASES-README.md:29:Each [release](https://github.com/matthewclaude/somnus-dial-releases/releases)
web-flasher/index.html:280:      <a href="https://github.com/matthewclaude/somnus-dial-releases">Releases</a>
web-flasher/index.html:282:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/LICENSE">License</a>
web-flasher/index.html:288:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES">third-party
web-flasher/index.html:304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
(exit 0; 135 lines)

$ git grep -c "somnus-dial-releases"
.github/workflows/release.yml:5
CHANGELOG.md:1
README.md:2
docs/ARCHITECTURE.md:1
docs/NAMING.md:1
docs/REPORT-0.1.5-beta.5-ci-check.md:5
docs/REPORT-0.1.5-beta.5-tag-push.md:2
docs/REPORT-0.1.5-ci-check.md:4
docs/REPORT-0.1.5-tag-push.md:2
docs/REPORT-0.1.6-graduation.md:12
docs/REPORT-1.0.0-commit.md:5
docs/REPORT-1.0.0-tag-push.md:7
docs/REPORT-handoff.md:1
docs/REPORT-readme-license-fix.md:1
docs/REPORT-release-0.1.6-beta.1.md:6
docs/REPORT-release-0.1.6-beta.2.md:6
docs/REPORT-release-0.1.6-beta.3.md:2
docs/REPORT-release-latest-pointer.md:21
docs/REPORT-releases-repo-readme.md:13
docs/REPORT-third-party-licenses-restore.md:6
docs/REPORT-third-party-licenses.md:8
docs/REPORTS.md:2
docs/SPEC-ota-readiness.md:6
firmware/dial-idf/README.md:3
firmware/dial-idf/components/dial_ota/dial_ota.c:4
web-flasher/README.md:2
web-flasher/RELEASES-README.md:3
web-flasher/index.html:4
```

Classification (every hit falls in exactly one class):

| Class | Count | Where |
|---|---|---|
| (a) code the firmware compiles and executes | **4** | `firmware/dial-idf/components/dial_ota/dial_ota.c` lines 35, 43, 47 (string constants), 301 (comment) |
| (b) CI / workflow config | **5** | `.github/workflows/release.yml` lines 12, 74 (comments), 84 (release-notes text echoed into the GitHub Release body), 129 (`repository:` input of `softprops/action-gh-release`), 178 (`external_repository:` input of `peaceiris/actions-gh-pages`) |
| (c) the flasher page or its manifests | **4** | `web-flasher/index.html` lines 280, 282, 288 (footer links to the repo, its LICENSE and THIRD_PARTY_LICENSES) and 304 (the page's `fetch()` of `/releases/latest`). `web-flasher/manifest.json` and `manifest-beta.json` contain no hit (their firmware paths are relative). |
| (d) prose in a doc, README, changelog or report | **122** | `README.md` (2), `CHANGELOG.md` (1), `firmware/dial-idf/README.md` (3), `web-flasher/README.md` (2), `web-flasher/RELEASES-README.md` (3), `docs/ARCHITECTURE.md` (1), `docs/NAMING.md` (1), `docs/SPEC-ota-readiness.md` (6), `docs/REPORTS.md` (2), and 101 across 17 `docs/REPORT-*.md` files |

4 + 5 + 4 + 122 = 135.

**What a later repoint spec has to change (class a), exactly:** three preprocessor string constants in `firmware/dial-idf/components/dial_ota/dial_ota.c`, each carrying the owner/repo path inside a GitHub API URL:

- `GITHUB_API_URL` (line 34–35) — `"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"`, the stable channel's request.
- `GITHUB_API_URL_TAGS` (line 42–43) — `"https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"`, the beta channel's tag scan.
- `GITHUB_API_URL_RELEASE_BY_TAG_FMT` (line 46–47) — `"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"`, the beta channel's per-tag lookup.

The comment at line 301 names the repo but compiles to nothing. Any repoint also has to touch class (b) (release.yml lines 129 and 178, which decide where assets and Pages land) and class (c) (index.html line 304, which the flasher uses to show the latest version, plus the three footer links); class (d) is prose only. This repo name is not itself a publication blocker: the releases repo is already public.

## Item 4 — repo hygiene

```
$ du -sh .git
 17M	.git
(exit 0)

$ git count-objects -vH
warning: garbage found: .git/objects/pack/tmp_idx_sfYN8m
warning: garbage found: .git/objects/pack/tmp_pack_zZpI8e
count: 1251
size: 9.31 MiB
in-pack: 2224
packs: 3
size-pack: 6.95 MiB
prune-packable: 0
garbage: 2
size-garbage: 621.12 KiB
(exit 0)

$ git ls-files | xargs -I{} du -k "{}" 2>/dev/null | sort -rn | head -15
248	firmware/dial-idf/components/dial_ui/dial_font_num_140.c
108	firmware/dial-idf/components/dial_ui/dial_font_num_88.c
88	firmware/dial-idf/main/main.c
84	docs/SPEC-ota-readiness.md
80	firmware/dial-idf/components/dial_ui/scr_dial.c
72	simulator/vendor/stb_image_write.h
64	firmware/dial-idf/components/dial_state/dial_state.h
60	docs/REPORT-release-0.1.6-beta.1.md
60	docs/REPORT-rails-fix.md
56	docs/REPORT-release-0.1.6-beta.2.md
56	docs/REPORT-battery-pct-about-redesign.md
52	docs/REPORT-layout-tier-bc.md
48	simulator/main.c
48	docs/SPEC-pad-discovery.md
48	docs/REPORT-layout-a1b.md
(exit 0)

$ ls -la .github/workflows/
total 48
drwxr-xr-x  5 matthew  staff   160 Sep  1 22:06 .
drwxr-xr-x  3 matthew  staff    96 Aug 12 12:16 ..
-rw-r--r--  1 matthew  staff  5414 Sep  1 22:06 certs.yml
-rw-r--r--  1 matthew  staff   371 Aug 12 12:16 ci.yml
-rw-r--r--  1 matthew  staff  8582 Sep  2 21:11 release.yml
(exit 0)

$ git ls-files | wc -l
     250
(exit 0)

=== tracked files over 5 MB (bytes) ===
(end)

=== tracked files git treats as binary (git diff --numstat vs empty tree shows '-') ===
   20542  docs/screens/about-battery-pct.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20650  docs/screens/about-battery-usb.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   23232  docs/screens/about-wifi-real.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   24196  docs/screens/about-wifi-worst.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   18358  docs/screens/about.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   25019  docs/screens/adjust-mode.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   19139  docs/screens/brightness-clock-off.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   21921  docs/screens/brightness-clock.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20139  docs/screens/brightness-menu.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20015  docs/screens/brightness.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   14039  docs/screens/connecting.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26578  docs/screens/dial-celsius.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   25180  docs/screens/dial-night-water.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   25951  docs/screens/dial-relative-max.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   25182  docs/screens/dial-relative.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26114  docs/screens/dial-update.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   25134  docs/screens/dial.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20975  docs/screens/menu.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   18723  docs/screens/netpick.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   16192  docs/screens/night-face.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   17916  docs/screens/night-mode.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   21666  docs/screens/pad-address.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   19561  docs/screens/pad-degraded-real.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   19370  docs/screens/pad-discovery.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   17244  docs/screens/pad-unreachable.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20657  docs/screens/passkey.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26044  docs/screens/rails-420-up-down.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26847  docs/screens/rails-420-up.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26847  docs/screens/rails-423-up.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   27850  docs/screens/rails-423.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26955  docs/screens/rails-drag-337-live.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   26574  docs/screens/rails-drag-337.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   22925  docs/screens/settings-pad.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   22595  docs/screens/settings-standby-face.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   24187  docs/screens/settings-timezone-raw.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20899  docs/screens/settings.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   15393  docs/screens/sidepick.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   17468  docs/screens/standby-face.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   20168  docs/screens/standby-update.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   19138  docs/screens/standby.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   17164  docs/screens/timezone.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   21747  docs/screens/update-failed.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   21987  docs/screens/update-prompt.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   22790  docs/screens/update.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   22933  docs/screens/updating.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   16447  docs/screens/welcome.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   22426  docs/screens/wifi-confirm.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   18040  docs/screens/wifi-info.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
   21603  docs/screens/wifi-portal.png  [PNG image data, 360 x 360, 8-bit/color RGBA, non-interlaced]
(end)
```

- **No tracked file is over 5 MB.** The largest tracked file is 248 KB (`dial_font_num_140.c`, a generated LVGL font as C source); the next is 108 KB (`dial_font_num_88.c`). Everything else is under 90 KB.
- **Tracked binaries:** exactly 49 files, all `docs/screens/*.png` simulator screenshots (360×360 RGBA, 14–28 KB each). No other binary is tracked; `trust_roots.pem` is text. Nothing to flag.
- **Workflows:** `certs.yml`, `ci.yml`, `release.yml` — three files, all read-only listed here, nothing unexpected.
- **`.git` is 17 MB** for 250 tracked files; `count-objects` reports 2,224 in-pack objects (6.95 MiB) plus 1,251 loose (9.31 MiB). It also warns of two leftover temporary pack files (`tmp_idx_sfYN8m`, `tmp_pack_zZpI8e`, 621 KiB) under `.git/objects/pack/` — harmless debris from an interrupted `git gc`/fetch. Not touched (read-only task); a `git gc` would remove them. None of this is pushed, so it has no publication effect.

## Item 5 — leftover personal paths

```
$ git grep -n -E "/Users/[a-z]+/|/home/[a-z]+/" -- . ':!docs/REPORT-*' ':!docs/REVIEW-*' ':!docs/TEST-*'
(exit 1)

=== per-file counts ===

=== for context: hits inside the excluded historical reports (count only) ===
files:       17
```

**No hits** outside the excluded historical reports: no tracked code, script, workflow, README or SPEC contains an absolute `/Users/<name>/` or `/home/<name>/` path. For context only, 17 `docs/REPORT-*`/`REVIEW-*`/`TEST-*` files do contain such paths in quoted terminal output (bench-log captures, build commands); they are the historical record this repo deliberately keeps and are not fixed here.

## HEAD before and after, push result

- HEAD before the report commit: `68b854de1fe5fd450a8c2b4354a2f7e87ae8549e`
- HEAD after the report commit, and the push result: reported in the chat reply that delivered this file. A commit cannot carry its own SHA, and this task allows exactly one commit, so there is no addendum commit this time.

## Deviations

- **HEAD-after and the push result are not in this file** (see the previous section). The task asked for both in the report and also for a single commit; the two cannot both hold, so the single-commit rule won and the two values are in the chat reply.
- The item-3 raw listing is reproduced by re-running the same `git grep` at report-writing time rather than pasted from the earlier capture; the two runs are on the same HEAD and identical. Once this report is committed, the same grep will return additional hits inside this file itself (class d).
- Otherwise none: every listed command ran as written, nothing was installed, nothing outside this report and its index line was written.

## Cannot verify from disk

- **GitHub secret scanning and push protection.** `gh api repos/matthewclaude/bedknob-for-somnus --jq .security_and_analysis` returned an empty value (raw output above, exit 0): the API exposes no `security_and_analysis` object for this repo while it is private on a personal account, so neither secret scanning nor push protection can be confirmed on or off from here. Both become available (and secret scanning is enabled by default for public repos on github.com) once the visibility flips; worth confirming in Settings → Code security after the flip.
- **Whether GitHub's own secret scanning would flag the masked `gho_****` line.** It should not (the value is asterisks), but that is GitHub's classifier, not a local check.
- **The full-history scan relied on the five regexes only.** No dedicated scanner ran (none installed). Anything shaped differently from the listed patterns — a bare high-entropy string, a non-GitHub provider token — would not have been caught.
- **Whether the two temporary pack files in `.git/objects/pack/` hide anything.** They are local, unpushed and unreachable from any ref; `git log --all` cannot see them. They are not part of what gets published.
