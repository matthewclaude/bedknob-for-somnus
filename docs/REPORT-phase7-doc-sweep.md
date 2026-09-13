# REPORT: SPEC-repo-consolidation Phase 7 — doc sweep, point the repo's own docs at the single repo — 2026-09-13

**DONE** — seven files reworded so nothing a reader would follow today points at `somnus-dial-releases`: both READMEs send a first-time installer to `matthewclaude.github.io/bedknob-for-somnus/` and link this repo's Releases page, `ARCHITECTURE.md` describes the OTA client as querying `bedknob-for-somnus` with the archived repo kept as the `1.0.0` migration path, `SPEC-ota-readiness.md` line 6 carries the dual-publish history (`1.0.1-beta.1`–`1.0.1` dual, `1.0.2-beta.1` on single-repo), `release.yml` and `NAMING.md` note the archive date, and the `dial_ota.c` header comment no longer claims dual-publishing. The `dial_ota.c` change is comment-only (proved below). Committed as `b53b06e`, pushed to `somnus`; CI run 34778960227 `success`. 39 mentions of the old repo remain outside reports and the CHANGELOG, every one listed below with why it stays. No build, no tag, no `PROJECT_VER` change. Two deviations, neither affecting the outcome.

## 1. Gate (raw, all four)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2-beta.1")
$ git --no-optional-locks status --short --untracked-files=no
(no output)
$ git --no-optional-locks log --oneline -1
7ff0c71 docs: Phase 6b complete - releases repo archived, frozen-redirect README, Pages redirect, secret and PAT gone
$ gh repo view matthewclaude/somnus-dial-releases --json isArchived
{"isArchived":true}
```

All four match expectations. Every line number named in the block matched disk before editing (README.md 53 and 212; firmware/dial-idf/README.md 13, 116, 224; docs/ARCHITECTURE.md 198; docs/SPEC-ota-readiness.md 6; .github/workflows/release.yml 13; docs/NAMING.md 46; dial_ota.c 33 and 35). No line number in the block was wrong on disk.

## 2. FIX items — before and after, verified against disk

Each "before" is the line as `sed -n` showed it before editing; each "after" is the committed text. Where the sentence containing the named line wrapped onto neighbouring lines, the whole sentence was reworded and the neighbours are shown.

### README.md line 53 — first-install flasher URL

Before:
```
**Open [matthewclaude.github.io/somnus-dial-releases](https://matthewclaude.github.io/somnus-dial-releases/)
```
After:
```
**Open [matthewclaude.github.io/bedknob-for-somnus](https://matthewclaude.github.io/bedknob-for-somnus/)
```

### README.md line 212 — where releases live (sentence spans lines 211–213)

Before:
```
Firmware is published from this repository to
[matthewclaude/somnus-dial-releases](https://github.com/matthewclaude/somnus-dial-releases),
which also hosts the browser flasher.
```
After:
```
Firmware is published as GitHub Releases of this repository
([bedknob-for-somnus/releases](https://github.com/matthewclaude/bedknob-for-somnus/releases)),
which also hosts the browser flasher.
```

### firmware/dial-idf/README.md line 13 — flasher URL

Before:
```
[https://matthewclaude.github.io/somnus-dial-releases/](https://matthewclaude.github.io/somnus-dial-releases/)
```
After:
```
[https://matthewclaude.github.io/bedknob-for-somnus/](https://matthewclaude.github.io/bedknob-for-somnus/)
```

### firmware/dial-idf/README.md line 116 — releases-repo link (sentence spans 115–117)

Before:
```
Each GitHub Release (tag `somnus-vX.Y.Z`, published to
[matthewclaude/somnus-dial-releases](https://github.com/matthewclaude/somnus-dial-releases))
carries two images:
```
After:
```
Each GitHub Release (tag `somnus-vX.Y.Z`, published on this repository's
[Releases page](https://github.com/matthewclaude/bedknob-for-somnus/releases))
carries two images:
```

### firmware/dial-idf/README.md line 224 — recovery flasher URL

Before:
```
  [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/)
```
After:
```
  [browser flasher](https://matthewclaude.github.io/bedknob-for-somnus/)
```

### docs/ARCHITECTURE.md line 198 — OTA endpoint (sentence opens on line 197; now lines 197–200)

Before:
```
`dial_ota` checks the **public releases repo**
`matthewclaude/somnus-dial-releases` over the GitHub API — `/releases/latest`
```
After:
```
`dial_ota` checks **this repository's own Releases**,
`matthewclaude/bedknob-for-somnus`, over the GitHub API (since `1.0.1-beta.1`;
a dial still on `1.0.0` polls the archived `somnus-dial-releases` repo, whose
`1.0.1` Release is its migration path onto this one) — `/releases/latest`
```
The rest of the paragraph (beta channel, `/tags?per_page=50`, `esp_https_ota`, TLS) is untouched.

### docs/SPEC-ota-readiness.md line 6 — reader's-guide status line, line 6 only

Before:
```
> flasher publish to the public `matthewclaude/somnus-dial-releases` via
```
After:
```
> flasher publish to this repo's own Releases and Pages, `matthewclaude/bedknob-for-somnus` (as-built history: releases were dual-published to `somnus-dial-releases` from `1.0.1-beta.1` to `1.0.1` and single-repo from `1.0.2-beta.1` on; that repo is archived as of 2026-09-13), with no
```
Line 7 (which begins `> ` then `SOMNUS_RELEASES_TOKEN` in backticks, `somnus-v0.1.3` is current, and a real OTA) is unchanged, so the sentence now reads "... with no `SOMNUS_RELEASES_TOKEN`, `somnus-v0.1.3` is current, ...". See deviation 1.

### .github/workflows/release.yml line 13 — comment only

Before:
```
# matthewclaude/somnus-dial-releases is the frozen pre-1.0.1 location that
```
After:
```
# matthewclaude/somnus-dial-releases (archived 2026-09-13) is the frozen pre-1.0.1 location that
```
No workflow logic changed; the diff for this file is one comment line.

### docs/NAMING.md line 46 — the "stays" assertion, unchanged in meaning

Before:
```
   `BedknobMac`. GitHub redirects the old names. `somnus-dial-releases` stays: its URL is
```
After:
```
   `BedknobMac`. GitHub redirects the old names. `somnus-dial-releases` stays (archived 2026-09-13, still served read-only): its URL is
```

## 3. dial_ota.c — full diff, comment lines only

```
commit b53b06e9e8dc3a52e22595185789f4ce7505bd14
Author: Matthew Montgomery <mattseattle@icloud.com>
Date:   Sun Sep 13 14:51:22 2026 -0500

    docs: Phase 7 sweep - point docs at the single repo, note the archived releases repo
    
    Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_01SFSm5UsqdHg6foNT7besks

diff --git a/firmware/dial-idf/components/dial_ota/dial_ota.c b/firmware/dial-idf/components/dial_ota/dial_ota.c
index 8d15813..2434da3 100644
--- a/firmware/dial-idf/components/dial_ota/dial_ota.c
+++ b/firmware/dial-idf/components/dial_ota/dial_ota.c
@@ -31,10 +31,12 @@ static const char *TAG = "ota";
 // as of 1.0.1-beta.1 (docs/SPEC-repo-consolidation.md). Before that, every
 // shipped dial resolved updates from the binaries-only
 // matthewclaude/somnus-dial-releases repo (docs/SPEC-ota-readiness.md §5),
-// by exactly the URL compiled in here -- so during the migration releases
-// are dual-published to somnus-dial-releases as well, which is where a
-// dial still on 1.0.0 finds this build. Nothing here falls back to the old
-// repo: a dial running this code polls only the URLs below.
+// by exactly the URL compiled in here. Releases were dual-published there
+// through 1.0.1 and are single-repo (this repo only) from 1.0.2-beta.1 on;
+// that repo is now archived, read-only, and its 1.0.1 Release is where a
+// dial still on 1.0.0 finds the build that repoints it here. Nothing here
+// falls back to the old repo: a dial running this code polls only the URLs
+// below.
 #define GITHUB_API_URL \
     "https://api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/latest"
 // Beta channel only (docs/SPEC-ota-readiness.md §9.7, 2026-09-03 finding): GitHub's
```

**Every changed line is a comment.** The four removed lines and the six added lines all begin with `//`. Checked mechanically: `git diff -U0` on the file, filtered to `+`/`-` lines and then to lines not beginning with `+//` or `-//`, returned nothing. No `#define` changed, `PROJECT_VER` is untouched (`1.0.2-beta.1`), nothing was built, nothing tagged.

## 4. Sweep grep — every surviving hit and why it stays

```
$ git --no-optional-locks grep -n "somnus-dial-releases" -- . ':!docs/REPORT-*' ':!docs/REPORTS.md' ':!CHANGELOG.md'
.github/workflows/release.yml:13:# matthewclaude/somnus-dial-releases (archived 2026-09-13) is the frozen pre-1.0.1 location that
docs/ARCHITECTURE.md:199:a dial still on `1.0.0` polls the archived `somnus-dial-releases` repo, whose
docs/NAMING.md:46:   `BedknobMac`. GitHub redirects the old names. `somnus-dial-releases` stays (archived 2026-09-13, still served read-only): its URL is
docs/NAMING.md:66:   `somnus-dial-releases` still carried pre-rename descriptions using "Somnus" as the
docs/SPEC-ota-readiness.md:6:> flasher publish to this repo's own Releases and Pages, `matthewclaude/bedknob-for-somnus` (as-built history: releases were dual-published to `somnus-dial-releases` from `1.0.1-beta.1` to `1.0.1` and single-repo from `1.0.2-beta.1` on; that repo is archived as of 2026-09-13), with no
docs/SPEC-ota-readiness.md:264:`matthewclaude/somnus-dial-releases` binaries-only repo. That board queried
docs/SPEC-ota-readiness.md:1115:delete the prerelease from `matthewclaude/somnus-dial-releases`.
docs/SPEC-ota-readiness.md:1236:`somnus-dial-releases` marked **Pre-release**, both assets attached. A dial
docs/SPEC-ota-readiness.md:1244:published. Every release in `somnus-dial-releases` points at the same commit
docs/SPEC-ota-readiness.md:1356:for the first five entries of the release list. `somnus-dial-releases`
docs/SPEC-repo-consolidation.md:3:Status: **Shipped as `somnus-v1.0.1-beta.1` on 2026-09-10 and hardware-confirmed. §2–§4 are built and released in both repos. Phase 6 (§5) is complete: Phase 6a cut the release workflow and the browser flasher over to this repo and shipped with `somnus-v1.0.2-beta.1` (`docs/REPORT-phase6a.md`); Phase 6b on 2026-09-13 archived `somnus-dial-releases` behind its frozen-redirect README and meta-refresh Pages index, deleted `SOMNUS_RELEASES_TOKEN` and revoked its PAT (`docs/REPORT-phase6b.md`), which closed the dual-publish window of §3.3. Still open: the §7 gate re-run against `1.0.1` stable. This work jumped the queue in the 1.0.x line — it shipped BEFORE the standby-poll cadence change (`SPEC-standby-poll.md`) because it has a closing window and that spec does not.** The window is the one in §3.3 and §6: every dial that installs any release while both repos are publishing migrates itself over the air; every dial that does not is stranded on the old repo the day the old repo stops publishing. The standby-poll change has no such clock, so it waited, and still queues behind this.
docs/SPEC-repo-consolidation.md:12:- `matthewclaude/somnus-dial-releases` — a binaries-only repo. Every `somnus-v*` tag pushed here has its Release object created *there* by `release.yml`, and the browser flasher is deployed *there* as `https://matthewclaude.github.io/somnus-dial-releases/`. This arrangement exists only because the source repo used to be private and GitHub 404s everything under a private repo to the unauthenticated clients (the dial's OTA check, ESP Web Tools) that need to read it.
docs/SPEC-repo-consolidation.md:14:The private-repo reason is gone. What remains is that **a dial only ever polls the repo URL compiled into its firmware.** `dial_ota.c` has three URL constants (§2) and all three name `somnus-dial-releases`. A user who flashes `1.0.0` today — from either repo's copy of the binary, it makes no difference where the bytes came from — gets a dial that follows OTA from the old repo forever, until it installs a build whose constants say otherwise.
docs/SPEC-repo-consolidation.md:24:**`firmware/dial-idf/components/dial_ota/dial_ota.c` — three URL constants.** As of this writing (`grep -n 'somnus-dial-releases'` on that file) the old repo name appears on lines 32, 37, 45, 49 and 303: one header comment, the three `#define` continuation lines, and one comment inside `check_stable()`.
docs/SPEC-repo-consolidation.md:28:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
docs/SPEC-repo-consolidation.md:30:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
docs/SPEC-repo-consolidation.md:32:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
docs/SPEC-repo-consolidation.md:59:- The `check_stable()` comment, lines 301–305, says the 404 on `/releases/latest` is "expected right now for the freshly created matthewclaude/somnus-dial-releases repo, not a check failure". That sentence becomes true again for the new repo, from the moment the beta ships until the first stable Release is published there (§3.1). Reword it to describe the new repo and that window; keep the behaviour it documents (404 → `OTA_IDLE`, "don't fall back to any other repo").
docs/SPEC-repo-consolidation.md:60:- The header comment above `GITHUB_API_URL`, lines 30–35, currently explains why releases *keep* publishing to `somnus-dial-releases` ("every shipped dial resolves updates by exactly this URL; consolidating releases into the source repo is a separate, planned step (the OTA repoint)"). This beta is that step. Rewrite it for the new state: the constants name this repo; `somnus-dial-releases` is the frozen pre-1.0.1 location that `1.0.0` dials still poll; pointer to this spec.
docs/SPEC-repo-consolidation.md:80:The difference between the repos is what the tags endpoint returns. In `somnus-dial-releases` the tags exist only because Release objects were created against them, so tags and Releases are one-to-one. In `bedknob-for-somnus` the tags endpoint returns **every `somnus-v*` tag ever pushed to the source repo** — 16 today (`git tag -l 'somnus-v*' | wc -l`; 0.1.0 through 1.0.0 including eight betas), none of which has a Release in this repo. Consequences:
docs/SPEC-repo-consolidation.md:90:- in `somnus-dial-releases`, so the `1.0.0` dial (Beta builds on) finds it at step 1, fetches its Release at step 3, downloads `somnus-dial.bin` and installs it;
docs/SPEC-repo-consolidation.md:95:**Fleet size, recorded 2026-09-11.** Everything above was written before the repoint shipped, and it is now overstated; this section should say why. There is exactly one dial in existence: the bench unit, running `1.0.1`, which polls the new repo. Both browser flashers serve the `1.0.1` merged image, and that image is itself repointed, so a board flashed from either page comes up polling the new repo; the only way left to make a pre-repoint unit is to flash an older release asset on purpose. The two further Waveshare boards inbound will be flashed from a flasher and arrive repointed too. So the window has no population left to protect, and the stranding argument here and in §6 is precautionary rather than load-bearing. The standing rule stays — the `1.0.1` Release on `somnus-dial-releases` is never deleted — but as cheap insurance against a board that turns up later, not as a live dependency. This stops being true the day GitHub shows activity from outside the project: the repo is public, and the fleet is assumed to be one unit only until then. The signals are the release assets' cumulative `downloadCount`, which every tag-push report already quotes (as of 2026-09-11: 2 and 1 on the old repo's `1.0.1-beta.1` assets, both the bench dial's own pull; 0 on the new repo), and the repository's Insights traffic — clones, unique cloners, unique visitors — a 14-day rolling window GitHub does not retain, so it has to be looked at or captured while it is there. Once either shows pulls or clones that are not the owner's own, this fleet-size assumption is stale and the stranding reasoning in this section and §6 is operative again.
docs/SPEC-repo-consolidation.md:101:- line 129 `repository: matthewclaude/somnus-dial-releases` — the `softprops/action-gh-release@v2` step, authenticated with `token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}` (line 130), a fine-grained PAT scoped to the old repo;
docs/SPEC-repo-consolidation.md:102:- line 178 `external_repository: matthewclaude/somnus-dial-releases` — the `peaceiris/actions-gh-pages@v4` deploy, `personal_token` from the same secret (line 177), `publish_dir: ./_site`, `publish_branch: gh-pages`, `keep_files: true`;
docs/SPEC-repo-consolidation.md:103:- line 84, the release-notes footer: `For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); …`;
docs/SPEC-repo-consolidation.md:118:- **(d) The four comments describing "this repo stays private"** get rewritten to describe the dual-publish window. Verified by `grep -n private`: lines 11 and 14 (the top-of-file comment, "THIS repo, which stays private -- but the Release object and the gh-pages site are published into the public matthewclaude/somnus-dial-releases repo instead"), and lines 73 and 76 (the release-notes step's comment explaining why there is no compare link — "THIS (private) repo … a compare link built from `${GITHUB_REPOSITORY}` would point at a private repo"). The compare-link reasoning is worth a second look while there: with Releases in the source repo a `compare/<prev>...<tag>` link *would* resolve. Adding one is not part of this beta, but the comment should stop claiming it cannot work.
docs/SPEC-repo-consolidation.md:124:- line 304 `fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")` — the "Latest firmware: …" label. On the new repo this is 404 until a stable Release exists there, and the page's `.then` throws on `!r.ok`, so a repointed page would show its error text for the whole beta period.
docs/SPEC-repo-consolidation.md:125:- lines 280, 282 and 288 — the footer's Releases, License and third-party-notices links, all into `github.com/matthewclaude/somnus-dial-releases`.
docs/SPEC-repo-consolidation.md:135:5. Archive `somnus-dial-releases`. Archive, not delete, not rename (§1).
docs/SPEC-repo-consolidation.md:137:**Ordering constraint, recorded 2026-09-11** now that the precondition is met — `1.0.1` dual-published on 2026-09-11, §7.1 bench pass the same day (`docs/REPORT-1.0.1-bench-gate.md`). `release.yml`'s `on:` block is `push:` of tags matching `somnus-v*`, and nothing else. So a commit to `main` that repoints `web-flasher/index.html`, or that drops the old-repo publish steps, changes nothing that is live: the flasher pages at `matthewclaude.github.io/somnus-dial-releases/` and `matthewclaude.github.io/bedknob-for-somnus/` are whatever the last release run deployed to `gh-pages`, and only the next tag push redeploys them. Items 1, 2 and the workflow edits in item 4 are inert until a release carries them. The consequence for items 3 and 5: the old repo must not be frozen or archived until a release has actually deployed the repointed flasher and that page has been opened and checked live. Freezing it early would not error — an archived repo still answers every read, so the live flasher would keep fetching the old repo's `/releases/latest`, which stops moving the day dual-publishing stops, and first-install users would silently keep being offered the last build that repo ever saw. Given the fleet size in §3.3, though, this ordering is tidiness, not a correctness risk: the only harm of freezing early is the old flasher page staying pinned at `1.0.1` for whoever finds it, and as of 2026-09-11 nobody but the owner has.
docs/SPEC-repo-consolidation.md:141:- **6a — on disk, landing with the next release.** Items 1, 2 and the workflow half of item 4: repoint `index.html`'s `/releases/latest` fetch and its three footer links, add a link to the source repo itself, repoint the release-notes footer, remove the old-repo `action-gh-release` step and the old-repo `peaceiris` deploy, and rewrite the workflow's stale comments — the top-of-file block and the changelog-extractor note that still describe dual-publishing into `somnus-dial-releases` and the token that does it. These go live with `somnus-v1.0.2-beta.1`, the standby-poll release (`SPEC-standby-poll.md`) already queued behind this, rather than needing a release of their own.
docs/SPEC-repo-consolidation.md:152:- **The one-way door, spelled out** (in the CHANGELOG too, in a sentence). A dial on `1.0.0` that never installs any release during the dual-publish window — never `1.0.1-beta.1`, never `1.0.1` — is still polling `somnus-dial-releases` after Phase 6, and that repo never publishes again. Whether it is *stranded* depends on what Phase 6 leaves behind. An archived repo is read-only but still answers every read: its `/releases/latest` still returns `1.0.1`, and its `browser_download_url` still serves the asset. So as long as the dual-published `1.0.1` Release is left in place on the archived repo, a `1.0.0` dial that checks for updates a year from now still finds `1.0.1` there, installs it, and comes out repointed — the archived repo's last Release *is* the migration path, permanently. What strands such a dial is any of: the old repo being deleted or renamed (§1 says never), the `1.0.1` Release being removed from it, or a future GitHub change to what archived repos serve. In every one of those cases the only way back is one wire flash from the new flasher page; there is no over-the-air route. That is why the window matters — it is the period during which the migration is *actively* served rather than resting on an archived repo's continued good behaviour — and why this spec jumps the queue: the fewer dials that arrive at Phase 6 still on `1.0.0`, the less rests on that. Phase 6 item 5 therefore carries a standing rule: **the `1.0.1` Release on `somnus-dial-releases` is never deleted.**
docs/SPEC-repo-consolidation.md:158:1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo → installs → reboots showing `1.0.1-beta.1` under Menu → About. The evidence is the Release fetch for `somnus-v1.0.1-beta.1` succeeding — the serial log's `ota: latest 1.0.1-beta.1, running 1.0.0 -- update available` line — and the reboot into `1.0.1-beta.1`. No log line carries a URL, but a dial still running `1.0.0` polls only the old repo, so only `somnus-dial-releases` could have served it that release; that is what makes the observation meaningful.
docs/SPEC-repo-consolidation.md:159:2. Same dial, now on `1.0.1-beta.1` → Menu → Update → set Beta builds **off** → Check for updates. The serial log must show, verbatim, `ota: no releases published yet (HTTP 404)`. That line is what tells the two repos apart: with Beta builds off the dial asks `/releases/latest`, and `bedknob-for-somnus` has no non-prerelease Release yet, so GitHub answers 404 — whereas the same check pointed at `somnus-dial-releases` would find its `1.0.0` stable Release and report up to date, never logging that line.
docs/SPEC-repo-consolidation.md:175:  $ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
firmware/dial-idf/components/dial_ota/dial_ota.c:33:// matthewclaude/somnus-dial-releases repo (docs/SPEC-ota-readiness.md §5),
web-flasher/README.md:9:`matthewclaude/somnus-dial-releases` repo, which served this page and the
web-flasher/RELEASES-README.md:1:# somnus-dial-releases — archived
web-flasher/RELEASES-README.md:24:release published here, [`somnus-v1.0.1`](https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v1.0.1),
```

39 hits (25 of them in `docs/SPEC-repo-consolidation.md`):

- `.github/workflows/release.yml:13` — FIX target of this block; the comment names the archived repo on purpose (frozen pre-1.0.1 location, now marked archived).
- `docs/ARCHITECTURE.md:199` — FIX target of this block; the migration-path clause the block asked to keep (1.0.0 dials still poll the archived repo).
- `docs/NAMING.md:46` — FIX target of this block; the assertion "stays" is still correct, now annotated as archived.
- `docs/NAMING.md:66` — LEAVE, named in the block: as-built narrative about pre-rename descriptions.
- `docs/SPEC-ota-readiness.md:6` — FIX target of this block; the as-built dual-publish history the block asked to add names the old repo.
- `docs/SPEC-ota-readiness.md:264` — LEAVE, named in the block: as-built narrative.
- `docs/SPEC-ota-readiness.md:1115` — LEAVE, named in the block: as-built narrative.
- `docs/SPEC-ota-readiness.md:1236` — LEAVE, named in the block: as-built narrative.
- `docs/SPEC-ota-readiness.md:1244` — LEAVE, named in the block: as-built narrative.
- `docs/SPEC-ota-readiness.md:1356` — LEAVE, named in the block: as-built narrative.
- `docs/SPEC-repo-consolidation.md:3` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:12` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:14` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:24` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:28` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:30` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:32` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:59` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:60` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:80` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:90` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:95` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:101` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:102` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:103` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:118` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:124` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:125` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:135` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:137` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:141` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:152` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:158` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:159` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `docs/SPEC-repo-consolidation.md:175` — LEAVE, named in the block: SPEC-repo-consolidation body is an as-built record (its status line was fixed in 7ff0c71).
- `firmware/dial-idf/components/dial_ota/dial_ota.c:33` — Header comment's history clause ("before that, every shipped dial resolved updates from ... somnus-dial-releases"); still true, kept; lines 34-40 reworded around it.
- `web-flasher/README.md:9` — Fixed in the Phase 6b docs commit (7ff0c71); describes the old repo as the former host of the page, in the past tense. True.
- `web-flasher/RELEASES-README.md:1` — In-tree source of the frozen-redirect README that is live on the archived repo's main (e0f0b51); must keep matching what was pushed there.
- `web-flasher/RELEASES-README.md:24` — Same file; links the old repo's own 1.0.1 Release, which is the migration path and still serves.

`CHANGELOG.md` was not opened. Every `docs/REPORT-*.md` was not opened.

## 5. CI

- Run id **34778960227**, workflow `ci`, head `b53b06e`, triggered by the push at 19:51:25Z.
- Conclusion **success**; job `build` 3m8s; the only annotation is GitHub's standing Node.js 20 deprecation notice on `actions/checkout@v4`, unrelated to this change.
- The preceding run 34778834355 (head `7ff0c71`, the Phase 6b docs commit) was still in progress when this one started; not waited on, not this block's push.

## 6. Commit

```
$ git --no-optional-locks diff --stat   (as committed)
b53b06e9e8dc3a52e22595185789f4ce7505bd14
docs: Phase 7 sweep - point docs at the single repo, note the archived releases repo

 .github/workflows/release.yml                    |  2 +-
 README.md                                        |  6 +++---
 docs/ARCHITECTURE.md                             |  6 ++++--
 docs/NAMING.md                                   |  2 +-
 docs/SPEC-ota-readiness.md                       |  2 +-
 firmware/dial-idf/README.md                      |  8 ++++----
 firmware/dial-idf/components/dial_ota/dial_ota.c | 10 ++++++----
 7 files changed, 20 insertions(+), 16 deletions(-)
```

Staged by `git add -u`; staged set was exactly those seven files. Pushed `7ff0c71..b53b06e main -> main` to `somnus`; `git log --oneline somnus/main..HEAD` empty immediately after.

## 7. Original Phase 7 plan item already done

The "If you want the source, ask the maintainer" item in the original Phase 7 plan needed no work here. `git grep 'ask the maintainer'` over the tree (reports excluded) returns nothing; the phrase was removed by `ec8ce37` "docs: the source is public now" on 2026-09-10.

## 8. Deviations

1. **SPEC-ota-readiness.md line 6 leaves a run-on with line 7.** The block said line 6 only. Line 7 begins with `` `SOMNUS_RELEASES_TOKEN`, `somnus-v0.1.3` is current ``, so line 6 was ended with "with no" to make the join read as a list item ("... with no `SOMNUS_RELEASES_TOKEN`, `somnus-v0.1.3` is current, and a real OTA ... is proven"). The rest of that reader's guide (dated 2026-09-02, `somnus-v0.1.3` current) is stale in the same way and was not touched. If the owner wants the whole blockquote brought to date, that is a separate edit.
2. **Sentences, not lines, were reworded in three places.** README.md 212, firmware README 116 and ARCHITECTURE.md 198 each sat inside a sentence wrapped across neighbouring lines, so the neighbours changed too (README 211, firmware README 115, ARCHITECTURE 197). Shown in §2. This is the block's "reword references" rule applied, not extra scope.

## 9. Not verified without hardware

Nothing in this block touches runtime behaviour: the only firmware file changed is a comment block in `dial_ota.c`, the three URL `#define`s are byte-identical, and the workflow change is a comment. There is nothing hardware could verify. CI's build of the same tree is the only executable check and it passed.

## 10. Closing state

What still names the archived repo, and why: the 39 hits in §4 — the `SPEC-repo-consolidation.md` body and the five `SPEC-ota-readiness.md` narrative lines as as-built record; the reworded lines in `release.yml`, `ARCHITECTURE.md`, `NAMING.md`, `SPEC-ota-readiness.md` line 6 and `dial_ota.c` line 33 because the archived repo is still, truthfully, the pre-1.0.1 location and the `1.0.0` migration path; `web-flasher/README.md` line 9 in the past tense; and `web-flasher/RELEASES-README.md` because it is the in-tree source of the redirect README live on the archived repo. Plus every report and the CHANGELOG, excluded by rule.

`main` = `somnus/main` = `b53b06e` at the end of the block. This report is untracked and rides the next docs commit, so after it is written `main` is 0 commits ahead of `somnus/main` with one untracked file.
