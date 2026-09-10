# Spec: Repo consolidation — one public repo for source, releases and flasher

Status: **Draft, 2026-09-10. Not built. Supersedes the queue order in the 1.0.x line: this ships as `1.0.1-beta.1` BEFORE the standby-poll cadence change (`SPEC-standby-poll.md`), because it has a closing window and that spec does not.** The window is the one in §3.3 and §6: every dial that installs any release while both repos are publishing migrates itself over the air; every dial that does not is stranded on the old repo the day the old repo stops publishing. The standby-poll change has no such clock, so it waits.

> **Note for an on-disk reader:** `V1-scope.md`, `START-HERE.md`, `HARDWARE-bringup-log.md`, `LICENSING.md` and `somnus-dial-project-summary.md` are **not in this repo** — they live only in the Claude Project. Everything this spec needs is restated here.

## 1. The problem

Today the project is split across two GitHub repos under the same owner:

- `matthewclaude/bedknob-for-somnus` — this repo. The source, the CI, the release workflow, the CHANGELOG. Public since 2026-09-10, source-available (PolyForm NC). It holds **no Releases**, serves **no Pages site**, and has **no `gh-pages` branch** — `git ls-remote --heads somnus` on 2026-09-10 lists exactly one ref, `refs/heads/main`. (The local checkout's `origin` remote is the upstream repo this project forked from, and `origin/gh-pages` is *that* repo's flasher deploy; it is irrelevant here and must not be mistaken for this repo's. See §4(b).)
- `matthewclaude/somnus-dial-releases` — a binaries-only repo. Every `somnus-v*` tag pushed here has its Release object created *there* by `release.yml`, and the browser flasher is deployed *there* as `https://matthewclaude.github.io/somnus-dial-releases/`. This arrangement exists only because the source repo used to be private and GitHub 404s everything under a private repo to the unauthenticated clients (the dial's OTA check, ESP Web Tools) that need to read it.

The private-repo reason is gone. What remains is that **a dial only ever polls the repo URL compiled into its firmware.** `dial_ota.c` has three URL constants (§2) and all three name `somnus-dial-releases`. A user who flashes `1.0.0` today — from either repo's copy of the binary, it makes no difference where the bytes came from — gets a dial that follows OTA from the old repo forever, until it installs a build whose constants say otherwise.

Goal: one public repo, `bedknob-for-somnus`, serving the source, the Releases and the flasher. The old repo is then **frozen — archived, never deleted or renamed.** Its URL is inside every `1.0.0` binary and in every link ever shared (release notes, the flasher footer, the README that was mirrored there), and an archived repo still answers `GET /releases/latest` and still serves its Pages site read-only. A deleted or renamed one would turn every un-migrated dial's update check into a permanent failure.

This spec is the "spec on disk before code" step. It changes no code and no workflow. §2–§5 describe the `1.0.1-beta.1` change; §5's last paragraph lists the cut-over that follows `1.0.1` stable, out of scope here.

## 2. What changes in firmware — exactly four strings

Everything the beta changes in compiled code is four string literals. Nothing else in `firmware/` moves.

**`firmware/dial-idf/components/dial_ota/dial_ota.c` — three URL constants.** As of this writing (`grep -n 'somnus-dial-releases'` on that file) the old repo name appears on lines 32, 37, 45, 49 and 303: one header comment, the three `#define` continuation lines, and one comment inside `check_stable()`.

```c
#define GITHUB_API_URL \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
#define GITHUB_API_URL_TAGS \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
#define GITHUB_API_URL_RELEASE_BY_TAG_FMT \
    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
```

In each, the owner/repo segment becomes `matthewclaude/bedknob-for-somnus`. The paths, the `per_page=50` (whose reason is the comment above `GITHUB_API_URL_TAGS`: the releases list is ordered by `created_at`, every entry ties, so the tags endpoint is scanned in full instead), and the `/releases/tags/%s` format are unchanged.

**`firmware/dial-idf/components/dial_state/dial_state.h` — `DIAL_CERT_ERR_MSG`.** Lines 358–361 today:

```c
#define DIAL_CERT_ERR_MSG \
    DIAL_CERT_ERR_TITLE "\n" \
    "This firmware may be too old\n" \
    "matthewclaude/somnus-waveshare-rotary-dial"
```

The last line becomes `matthewclaude/bedknob-for-somnus`. Note that this string names neither repo of §1 today — it still carries the pre-rename source-repo name (NAMING.md item 6 deferred it to exactly this beta). The string deliberately omits `github.com/`: the comment above it (lines 353–355) records that at the error screen's sub label the full URL measures ~400 px at `lv_font_montserrat_16` against a 300 px label and wraps awkwardly, while bare `owner/repo` fits on one line. Keep that.

**Explicitly unchanged, and why each is a shipped identifier:**

- `ASSET_NAME "somnus-dial.bin"` (`dial_ota.c:50`). The dial matches a Release's assets by this exact name (`finish_from_release()`, line 269) and fails the check with "no somnus-dial.bin asset in latest release" if it is absent. A renamed asset would strand every dial on the old name.
- `TAG_PREFIX "somnus-v"` (`dial_ota.c:59`). `tag_version()` and `release_version()` strip it and `is_newer()` compares what is left; the comment above it spells out that a mismatch makes `sscanf` match zero fields and read as "not newer — no error, no log". `release.yml`'s tag trigger (`somnus-v*`) and its CHANGELOG-section extractor key off the same prefix.
- The merged image name `somnus-dial-merged.bin`. `release.yml` produces it (line 117), attaches it to the Release (line 135) and deploys it to the Pages site as `firmware/<channel>/somnus-dial-merged.bin` (line 166–167); the flasher manifests name it.
- The CMake project name, `project(somnus-dial)` (`CMakeLists.txt:22`). It is what makes the build output `somnus-dial.bin`.

**The asset download host does not change.** The dial does not build a download URL from the repo name. It reads `browser_download_url` out of the Release JSON (`dial_ota.c:270`) and follows it; for a GitHub Release asset that URL resolves to `objects.githubusercontent.com` regardless of which repo owns the Release. So `trust_roots.pem` needs no change, and `certs.yml` — the monthly cert sentinel (`cron: '17 6 1 * *'`), whose loop on line 52 checks exactly `api.github.com` and `objects.githubusercontent.com` — needs no change either.

**Comments to update in the same edit (comment-only):**

- The `check_stable()` comment, lines 301–305, says the 404 on `/releases/latest` is "expected right now for the freshly created matthewclaude/somnus-dial-releases repo, not a check failure". That sentence becomes true again for the new repo, from the moment the beta ships until the first stable Release is published there (§3.1). Reword it to describe the new repo and that window; keep the behaviour it documents (404 → `OTA_IDLE`, "don't fall back to any other repo").
- The header comment above `GITHUB_API_URL`, lines 30–35, currently explains why releases *keep* publishing to `somnus-dial-releases` ("every shipped dial resolves updates by exactly this URL; consolidating releases into the source repo is a separate, planned step (the OTA repoint)"). This beta is that step. Rewrite it for the new state: the constants name this repo; `somnus-dial-releases` is the frozen pre-1.0.1 location that `1.0.0` dials still poll; pointer to this spec.

## 3. Behaviour of each channel against the new repo

This is the part that needs thinking. The string swap is trivial; what the two channels do against a repo that has tags but (at first) no Releases is not.

### 3.1 Stable channel

`check_stable()` does one request: `GET /repos/matthewclaude/bedknob-for-somnus/releases/latest`. Until `1.0.1` stable is published in the new repo, GitHub answers 404 (there are no Releases there at all, and `/releases/latest` excludes prereleases by definition, so the beta's own Release will not satisfy it either). `check_stable()` maps `err == ESP_OK && status == 404` to `OTA_IDLE` with the log line "no releases published yet (HTTP 404)" — the same status as "checked, nothing newer", not `OTA_FAILED`.

Consequence: a repointed dial with Beta builds **off** reports "up to date" on every check until `1.0.1` stable exists in the new repo. For the one bench unit this is acceptable, and the hardware gate (§7) keeps Beta builds **on** so the gate actually exercises the new repo rather than a 404 that looks like success.

### 3.2 Beta channel

`check_beta()` does two bounded requests (its header comment, lines 347–357):

1. `GET /tags?per_page=50` and pick the single highest `somnus-v*` tag anywhere in the page by `is_newer()`; order is never assumed.
2. If that tag is not newer than the running version, stop — one request, no Release object fetched.
3. Otherwise `GET /releases/tags/<tag>`; on a 404 or a draft, fall back to the next-highest *untried* tag, re-scanned from the list, up to `OTA_BETA_CANDIDATE_CAP` attempts. The cap is **3** (`dial_ota.c:66`). If all three candidates fail, the check ends `OTA_FAILED` with "no usable release for newest tags" (line 502).

The difference between the repos is what the tags endpoint returns. In `somnus-dial-releases` the tags exist only because Release objects were created against them, so tags and Releases are one-to-one. In `bedknob-for-somnus` the tags endpoint returns **every `somnus-v*` tag ever pushed to the source repo** — 16 today (`git tag -l 'somnus-v*' | wc -l`; 0.1.0 through 1.0.0 including eight betas), none of which has a Release in this repo. Consequences:

- **(a) Historical tags with no Release are harmless as long as the newest tag has one.** Step 1 picks the highest tag; step 2 stops there if it is not newer; step 3 only ever fetches Release objects for tags newer than the running build. A dial on `1.0.1-beta.1` sees `1.0.1-beta.1` as the highest tag, finds it not newer, and never touches the fifteen tags below it. The fallback chain is only entered when the newest tag is newer *and* has no usable Release.
- **(b) A tag exists in the source repo before its Release does.** `release.yml` runs on the tag push and takes about six minutes (the `1.0.0` release run: 5m45s job time, 6m2s for the run). A beta check that lands inside that window sees the new tag at step 1, finds it newer, 404s at step 3, falls back to the next-highest tag — which in the new repo also has no Release — and again, and ends after the third attempt in `OTA_FAILED` "no usable release for newest tags". This is transient and self-healing: the next check, after the Release exists, succeeds. But it means **the hardware gate must not be run inside that window** (§7 item 4), because the failure it would produce is not the failure the gate is looking for.
- **(c) The 50-tag page cap.** At 16 tags today and roughly two to three per release cycle (one stable plus its betas), the tags list reaches 50 in something like a dozen releases. Past that, GitHub paginates and a single page can no longer be trusted to contain the newest tag — the same shape as the beta-not-found finding of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`; the `per_page=5` releases list that hid the sixth entry), just at 50 instead of 5. In the old repo this was far off because tags there only ever accumulate at the rate of Releases; in the new repo every beta tag counts. This is a note for a later fix (walk the `Link: rel="next"` pagination, or ask for `/tags?per_page=100`), **not this spec.** Record it; do not build it here.

### 3.3 The migration itself — the dual-publish window

A `1.0.0` dial polls the **old** repo. For it to migrate over the air, `1.0.1-beta.1` must exist in **both** repos:

- in `somnus-dial-releases`, so the `1.0.0` dial (Beta builds on) finds it at step 1, fetches its Release at step 3, downloads `somnus-dial.bin` and installs it;
- in `bedknob-for-somnus`, so that after the reboot the dial's first check — now against the new repo, because the new constants are in the running image — finds the same tag at step 1, compares it to the running `1.0.1-beta.1`, and says "up to date". If the tag were missing from the new repo the dial would see `1.0.0` as the highest tag, find it not newer, and also say "up to date" — indistinguishable on screen, which is why §7 item 3 requires the serial capture.

Same tag name in both repos, same asset bytes (the workflow uploads the same build artefact to both Releases). This is the dual-publish window. It opens at the `1.0.1-beta.1` tag and lasts through `1.0.1` stable, which must be dual-published for the same reason: a `1.0.0` dial with Beta builds *off* never sees a prerelease, and `1.0.1` stable in the old repo is its only over-the-air path onto the new constants.

## 4. What changes in `release.yml` — described, not implemented here

Current state, verified by `grep -n` on 2026-09-10:

- line 129 `repository: matthewclaude/somnus-dial-releases` — the `softprops/action-gh-release@v2` step, authenticated with `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}` (line 130), a fine-grained PAT scoped to the old repo;
- line 178 `external_repository: matthewclaude/somnus-dial-releases` — the `peaceiris/actions-gh-pages@v4` deploy, `personal_token` from the same secret (line 177), `publish_dir: ./_site`, `publish_branch: gh-pages`, `keep_files: true`;
- line 84, the release-notes footer: `For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); …`;
- line 27–28 `permissions: contents: write` is already declared at workflow level, which is what the default `GITHUB_TOKEN` needs to create a Release and push a branch **in this repo**.

The beta's workflow change, in the same commit as the four strings:

- **(a) A second `action-gh-release` step**, after the existing one, publishing the same `body` (`steps.notes.outputs.body`), the same `prerelease` flag and the same two files (`somnus-dial.bin`, `somnus-dial-merged.bin`) to `matthewclaude/bedknob-for-somnus`. No `repository:` key (the action defaults to the workflow's own repo) and no `token:` (it defaults to `GITHUB_TOKEN`, which `permissions: contents: write` already covers). The existing old-repo step stays exactly as it is. Order matters slightly: put the old-repo step first, so that if the new-repo step fails the old repo — which `1.0.0` dials actually poll — already has the Release.
- **(b) A second `peaceiris/actions-gh-pages` deploy** to *this* repo's `gh-pages` branch: `github_token: ${{ secrets.GITHUB_TOKEN }}`, no `external_repository`, no `personal_token`, the same `publish_dir: ./_site`, `publish_branch: gh-pages`, `keep_files: true`. This is what makes the new Pages site exist. Two things to know about the target branch. First, it does not exist yet. `matthewclaude/bedknob-for-somnus` has no `gh-pages` branch — verified 2026-09-10:

  ```
  $ git ls-remote --heads somnus
  2619c5738a7016594a1be69c848ed1597552549d	refs/heads/main
  ```

  (The `origin/gh-pages` visible in a local checkout belongs to the upstream repo, `chris023/orion-waveshare-rotary-dial`, which `origin` points at; it is the upstream project's own flasher deploy and has nothing to do with this repo.) So the first deploy creates `gh-pages` from `_site` alone, with nothing inherited. `keep_files: true` still matters from the second deploy on: each channel owns its own directory under `firmware/`, and a beta deploy must not delete `firmware/latest/` or vice versa; the spec does not propose `force_orphan` for the same reason the existing comment (lines 180–184) rejects it — it would wipe the other channel's image on every deploy. Second, **an owner-only prerequisite before the first dual-published tag: enable GitHub Pages on `bedknob-for-somnus`, source = `gh-pages` branch, root.** Nothing in the workflow can do that; a deploy to a branch that Pages is not serving publishes nothing.
- **(c) The release-notes footer keeps pointing at the old flasher URL** until `1.0.1` stable. The new Pages site does not exist until (b) has run once and the owner has enabled it, and the flasher page itself (§5) still reads the old repo; a note that sent first-time installers to a URL that might 404 would be worse than one that sends them to a page that works.
- **(d) The four comments describing "this repo stays private"** get rewritten to describe the dual-publish window. Verified by `grep -n private`: lines 11 and 14 (the top-of-file comment, "THIS repo, which stays private -- but the Release object and the gh-pages site are published into the public matthewclaude/somnus-dial-releases repo instead"), and lines 73 and 76 (the release-notes step's comment explaining why there is no compare link — "THIS (private) repo … a compare link built from `${GITHUB_REPOSITORY}` would point at a private repo"). The compare-link reasoning is worth a second look while there: with Releases in the source repo a `compare/<prev>...<tag>` link *would* resolve. Adding one is not part of this beta, but the comment should stop claiming it cannot work.

## 5. What does NOT change in the beta

`web-flasher/index.html` stays on the old repo, entirely:

- line 304 `fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")` — the "Latest firmware: …" label. On the new repo this is 404 until a stable Release exists there, and the page's `.then` throws on `!r.ok`, so a repointed page would show its error text for the whole beta period.
- lines 280, 282 and 288 — the footer's Releases, License and third-party-notices links, all into `github.com/matthewclaude/somnus-dial-releases`.

The flasher page is copied into `_site/` by the deploy job and so the same file, unchanged, is what the (b) deploy in §4 puts on the new Pages site. That is fine for the beta: it is a working page that happens to link to the old repo.

**Phase 6, out of scope here** — the cut-over, after `1.0.1` stable has been dual-published and the §7 gate has passed against the stable build. Listed so the sequence is on disk without being tasked:

1. Repoint `web-flasher/index.html`: the `/releases/latest` fetch and the three footer links to `bedknob-for-somnus`.
2. Repoint the release-notes footer (§4(c)) to `https://matthewclaude.github.io/bedknob-for-somnus/`.
3. Replace the old repo's README with a frozen-redirect README: this repo is archived; source, Releases and the flasher are at `bedknob-for-somnus`; dials on 1.0.0 that never updated need one wire flash from the new flasher (§6). Make the old Pages site's `index.html` a meta-refresh redirect to the new one (§8).
4. Remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy from `release.yml`, then delete the `SOMNUS_RELEASES_TOKEN` secret and revoke the PAT.
5. Archive `somnus-dial-releases`. Archive, not delete, not rename (§1).

Each of those is its own gated step with its own report. None of them happens in this beta.

## 6. Versioning and CHANGELOG

- Tag: `somnus-v1.0.1-beta.1`. `PROJECT_VER` in `firmware/dial-idf/CMakeLists.txt` line 21 becomes `1.0.1-beta.1` in the same commit as the four strings and the workflow change, because `release.yml` verifies the tag against `PROJECT_VER` and the CHANGELOG section.
- A `## 1.0.1-beta.1 — <date> (beta)` section at the top of `CHANGELOG.md`, in the file's user-facing register. The substance, in those words: the dial now checks for updates at the project's own repository, `github.com/matthewclaude/bedknob-for-somnus`, where the source lives; nothing else changes — no screen, no behaviour, no setting; a dial already on 1.0.0 picks this up over the air as usual (Beta builds on). The section is the release-notes body for both repos' Releases, so it must read correctly from either side.
- **The one-way door, spelled out** (in the CHANGELOG too, in a sentence). A dial on `1.0.0` that never installs any release during the dual-publish window — never `1.0.1-beta.1`, never `1.0.1` — is still polling `somnus-dial-releases` after Phase 6, and that repo never publishes again. Whether it is *stranded* depends on what Phase 6 leaves behind. An archived repo is read-only but still answers every read: its `/releases/latest` still returns `1.0.1`, and its `browser_download_url` still serves the asset. So as long as the dual-published `1.0.1` Release is left in place on the archived repo, a `1.0.0` dial that checks for updates a year from now still finds `1.0.1` there, installs it, and comes out repointed — the archived repo's last Release *is* the migration path, permanently. What strands such a dial is any of: the old repo being deleted or renamed (§1 says never), the `1.0.1` Release being removed from it, or a future GitHub change to what archived repos serve. In every one of those cases the only way back is one wire flash from the new flasher page; there is no over-the-air route. That is why the window matters — it is the period during which the migration is *actively* served rather than resting on an archived repo's continued good behaviour — and why this spec jumps the queue: the fewer dials that arrive at Phase 6 still on `1.0.0`, the less rests on that. Phase 6 item 5 therefore carries a standing rule: **the `1.0.1` Release on `somnus-dial-releases` is never deleted.**

## 7. Hardware gate

The beta tag itself is gated on the beta's own build (`idf.py build` clean; the simulator unaffected, since none of the four strings is rendered by it) and a bench pass of items 1–3 below against the *beta*. Tagging `1.0.1` **stable** repeats the gate against the stable build. Both passes use the one bench unit.

1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo (the serial log shows the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50` and the Release fetch for `somnus-v1.0.1-beta.1` returning 200) → installs → reboots showing `1.0.1-beta.1` under Menu → About.
2. Same dial → Check for updates → the serial log must show the request going to `api.github.com/repos/matthewclaude/bedknob-for-somnus/…` and the result `latest 1.0.1-beta.1, running 1.0.1-beta.1 -- up to date`.
3. **A serial capture of item 2 is required** and goes into the bring-up record. A silent "up to date" against the wrong host is indistinguishable from success on the screen — that was the failure shape of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`: eight `ota:` lines, every one "up to date", while the newer beta sat unseen on the server). Capture with the cat-based serial method from the bring-up notes, not `idf.py monitor`.
4. Not inside the ~6-minute tag-to-Release window of §3.2(b). Confirm with `gh run watch` that the release run for the tag has completed **and** that both Releases exist (`gh release view somnus-v1.0.1-beta.1 --repo <each>`) before touching the dial.
5. Rollback, if the repointed build cannot see the new repo (item 2 fails, or ends `OTA_FAILED`): wire-flash the `1.0.0` merged image from the **old** flasher page, which stays live throughout the beta and the cut-over. Five minutes, one unit. Then the beta is withdrawn (delete the two Releases; the tag stays, which is harmless per §3.2(a) once a newer tag with a Release exists) and this spec gets a §9 saying what was wrong.

## 8. Open questions for the owner, each with a recommended answer

1. **Do Dependabot or secret-scanning settings on `bedknob-for-somnus` need anything for Releases?** No. Releases are content, not dependencies; secret scanning already runs on the public source and does not look inside Release assets. Nothing to configure.
2. **Does the new Pages site need a custom 404 or a redirect for old deep links?** No, not now. The old site stays up, unchanged, until Phase 6; nobody has a deep link into the new site yet. At Phase 6 the *old* site becomes a meta-refresh redirect to the new one (Phase 6 item 3), which is the direction the deep links actually point.
3. **Does `certs.yml`'s host list need bumping?** No. §2: the dial talks to `api.github.com` for the check and follows `browser_download_url` to `objects.githubusercontent.com` for the download, in both repos. The sentinel already checks exactly those two hosts.
4. **(Found while writing) Enabling Pages.** Confirm the owner does it before the beta tag, not after — see §4(b). If it is done after, the deploy has still landed on the branch and enabling Pages then serves it; no re-run is needed. Recommended: before, so the gate can also open the new flasher URL and see the "Latest firmware" label fail gracefully (it will show its error text until `1.0.1` stable — §5 — which is expected and should be written down in the gate report so it is not mistaken for a defect).

## 9. Not in this spec

- Any pagination of the tags endpoint (§3.2(c)).
- The compare link in release notes (§4(d)).
- The flasher repoint, the old-repo README, the PAT removal, archiving — all Phase 6 (§5).
- The standby-poll cadence change; it queues behind this.
