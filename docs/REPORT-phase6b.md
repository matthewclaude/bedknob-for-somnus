# REPORT: SPEC-repo-consolidation Phase 6b — secret, PAT, frozen-redirect README, Pages redirect, archive — 2026-09-13

**DONE** — `matthewclaude/somnus-dial-releases` is archived (`isArchived: true`), its `somnus-v1.0.1` Release and both assets are still there and still download (checked after archiving, SHA-256 of `somnus-dial.bin` matches the Release digest), `SOMNUS_RELEASES_TOKEN` is gone from `bedknob-for-somnus`, the owner confirms the PAT "somnus-dial-releases publish" is revoked, the old repo's `main` carries the frozen-redirect README (`e0f0b51`) and its `gh-pages` carries the meta-refresh redirect (`6aa3a12`), which the old flasher URL now serves. Nothing was deleted or renamed; the `gh-pages` branch, every Release and every asset survive. Phase 6 of `docs/SPEC-repo-consolidation.md` is closed. Eight deviations recorded below, none affecting the outcome. Nothing committed in this repo: this report, its `REPORTS.md` line, the new `web-flasher/RELEASES-README.md` and a two-paragraph fix to `web-flasher/README.md` ride the next docs commit.

Run from `~/Projects/somnus-waveshare-rotary-dial`, `main` at `9057d3f`. The old repo was touched only through a throwaway `gh repo clone` in the session scratchpad, removed afterwards. No push to `origin`; no push to `somnus`.

## The phase as a whole

Phase 6b is the second half of §5's Phase 6 in `docs/SPEC-repo-consolidation.md`: after `somnus-v1.0.2-beta.1` deployed the repointed flasher (Phase 6a, `docs/REPORT-phase6a.md`) and the new page was confirmed live, (1) delete the `SOMNUS_RELEASES_TOKEN` secret, (2) revoke the PAT behind it, (3) replace the old repo's README with a frozen redirect, (4) make the old Pages index a meta-refresh redirect, (5) archive, (6) verify. Steps 1 and 2 were assigned in an earlier session; this session started at step 2.5 (re-establish state), found step 1 had not actually happened, ran it, obtained the owner's word on step 2, and carried steps 3 through 6.

**Note on the gate.** The original entry gate for this phase was run and reported in an earlier session, whose chat is gone. The figures in step 2.5 below are a fresh re-run made in this session, not a transcription of what that session reported.

**Entry check (6b precondition, re-run here).** The new flasher page is live and fetches this repo:

```
--- https://matthewclaude.github.io/bedknob-for-somnus/
HTTP 200
api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/latest
0          <- grep -c 'somnus-dial-releases' in the served page
<title>Bedknob for Somnus — flash from your browser
```

## Step 2.5 — state re-established (raw output)

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.2-beta.1")

$ grep -rn "SOMNUS_RELEASES_TOKEN" .github/ ; echo "grep exit $?"
grep exit 1

$ git --no-optional-locks status --short --untracked-files=no
(no output)

$ gh release view somnus-v1.0.1 --repo matthewclaude/somnus-dial-releases --json tagName,isDraft,isPrerelease,publishedAt,assets
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/557784998","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:04Z","digest":"sha256:90b27f3b00d0ffa1d9f382b6f92eb587ae4fb8e53d8ebda44c35a5c824b9e00d","downloadCount":1,"id":"RA_kwDOULeAcc4hPx-m","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-11T18:34:05Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/557785000","contentType":"application/octet-stream","createdAt":"2026-09-11T18:34:04Z","digest":"sha256:c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782","downloadCount":1,"id":"RA_kwDOULeAcc4hPx-o","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-11T18:34:04Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-11T18:34:05Z","tagName":"somnus-v1.0.1"}

$ gh secret list --repo matthewclaude/bedknob-for-somnus
SOMNUS_RELEASES_TOKEN	2026-09-02T02:10:57Z
```

Gate result: PROJECT_VER `1.0.2-beta.1` ✔; grep finds nothing, exit 1 ✔; tracked tree clean ✔; `somnus-v1.0.1` present, `isDraft` false, `isPrerelease` false, both assets `uploaded` ✔. Fifth check: **`SOMNUS_RELEASES_TOKEN` was still listed** — step 1 had not happened.

Other state read before anything irreversible (all read-only): old repo `isArchived: false`, branches `gh-pages` and `main`, `main` at `306985b docs: the source is public now` with `LICENSE`, `README.md`, `THIRD_PARTY_LICENSES`; `gh-pages` at `3316ce3 deploy: matthewclaude/bedknob-for-somnus@f0aec29` with `.nojekyll`, `firmware/{beta,latest}/somnus-dial-merged.bin`, `index.html`, `manifest.json`, `manifest-beta.json`; Pages source `gh-pages` root, status `built`; 18 Releases, `somnus-v1.0.1` marked Latest. The old repo's `README.md` was byte-identical to this repo's `web-flasher/RELEASES-README.md` (`diff` → IDENTICAL), confirming that file is the source of record for it. This repo: `git rev-list --count somnus/main..main` → `0` (the owner pushed `9057d3f` since the last session; `somnus/main` = `9057d3f`).

## Step 1 — delete the secret (run in THIS session)

Step 1 was **not** already complete. Run here:

```
$ gh secret delete SOMNUS_RELEASES_TOKEN --repo matthewclaude/bedknob-for-somnus
(exit 0)
$ gh secret list --repo matthewclaude/bedknob-for-somnus
(exit 0)            <- empty list
```

## Step 2 — PAT revocation (owner's word, the only record)

Asked: "has he revoked the fine-grained PAT scoped to somnus-dial-releases, and what was it called?" The owner answered in two parts. First, choosing the offered option:

> Yes, revoked (name via Other)

then, asked for the name:

> somnus-dial-releases publish

So: PAT **"somnus-dial-releases publish"**, revoked per the owner. No `gh` command can verify a PAT revocation; this quote is the record.

## Step 3 — frozen-redirect README on the old repo's `main`

`web-flasher/RELEASES-README.md` in this repo rewritten (uncommitted, rides the next docs commit), then copied to `README.md` in the throwaway clone and pushed:

```
$ gh repo clone matthewclaude/somnus-dial-releases <scratchpad>/somnus-dial-releases
clone exit 0
* main   306985b docs: the source is public now
 README.md | 80 +++++++++++++++++++++++++++++++++------------------------------
 1 file changed, 42 insertions(+), 38 deletions(-)
e0f0b51 docs: frozen-redirect README — repo archived, everything is at bedknob-for-somnus
$ git push origin main
   306985b..e0f0b51  main -> main
push exit 0
```

(`origin` in that clone is the old repo, the only remote the clone has. This repo's `origin`, the upstream, was not touched.)

Full text of the new `web-flasher/RELEASES-README.md` (= the old repo's `README.md` at `e0f0b51`):

````markdown
# somnus-dial-releases — archived

**This repository is archived and no longer publishes anything.** It held the
firmware releases and the browser flasher for **Bedknob for Somnus** — a
bedside dial that turns a Waveshare ESP32-S3 round touch-LCD knob into a
standalone temperature control for a Somnus Pad — while the firmware's source
was still private. The source has been public since 2026-09-10, and from
`1.0.2-beta.1` onward releases are published alongside it. Not affiliated
with, endorsed by, or supported by Somnus Lab or Waveshare.

Everything now lives at
**[matthewclaude/bedknob-for-somnus](https://github.com/matthewclaude/bedknob-for-somnus)**:

- **Source** — <https://github.com/matthewclaude/bedknob-for-somnus>
- **Releases** — <https://github.com/matthewclaude/bedknob-for-somnus/releases>
- **Browser flasher** — <https://matthewclaude.github.io/bedknob-for-somnus/>

The flasher page that used to be served from this repository now redirects
to the new one.

## If your dial is still on 1.0.0

Dials on `1.0.0` check this repository for updates, not the new one. The last
release published here, [`somnus-v1.0.1`](https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v1.0.1),
is deliberately left in place and is never removed: a `1.0.0` dial that runs
Menu → Update → Check for updates still finds `1.0.1` here, installs it over
the air, and from then on checks the new repository. Everything after `1.0.1`
is found there.

If that does not happen — the dial never went online to check, or GitHub
changes what archived repositories serve — the fix is one wire flash: open
**[the new flasher page](https://matthewclaude.github.io/bedknob-for-somnus/)**
in Chrome or Edge on a desktop computer, plug the dial in over USB-C, and
click Install. Flashing from the page erases the dial's settings (Wi-Fi,
timezone, pad address); over-the-air updates never do.

## What is still here

The releases up to and including `somnus-v1.0.1` and their two files each:

- `somnus-dial.bin` — the app-only image a running dial downloads over the
  air; never flash it directly at offset `0x0`.
- `somnus-dial-merged.bin` — the same firmware with bootloader and partition
  table, flashable at offset `0x0` on a blank or already-flashed chip.

## License and attribution

This firmware is a fork of
[chris023/orion-waveshare-rotary-dial](https://github.com/chris023/orion-waveshare-rotary-dial),
licensed under the **PolyForm Noncommercial License 1.0.0** — free for
personal, noncommercial use. See [`LICENSE`](LICENSE) for the full terms
and required notice, and [`THIRD_PARTY_LICENSES`](THIRD_PARTY_LICENSES)
for the hardware bring-up code, fonts, data, and libraries this project
builds on, each under its own terms. The current versions of both files are
maintained in the source repository.
````

## Step 4 — meta-refresh redirect on the old repo's `gh-pages`

Only `index.html` replaced; `.nojekyll`, `firmware/beta/`, `firmware/latest/`, `manifest.json` and `manifest-beta.json` left in place (the manifests still point at `firmware/latest/somnus-dial-merged.bin`, the 1.0.1 image, so an ESP Web Tools client holding the old manifest URL still gets 1.0.1).

```
$ git checkout gh-pages       3316ce3 deploy: matthewclaude/bedknob-for-somnus@f0aec2998affc513026a68f16109a38b8832409a
 index.html | 322 +++----------------------------------------------------------
 1 file changed, 11 insertions(+), 311 deletions(-)
6aa3a12 redirect: flasher page moved to matthewclaude.github.io/bedknob-for-somnus
$ git push origin gh-pages
   3316ce3..6aa3a12  gh-pages -> gh-pages
push exit 0
gh-pages tree after: firmware/beta/somnus-dial-merged.bin  firmware/latest/somnus-dial-merged.bin  index.html  manifest-beta.json  manifest.json  (.nojekyll)

$ gh api repos/matthewclaude/somnus-dial-releases/pages/builds/latest
{"commit":"6aa3a12badaa9105089a517fc0164596bff047a2","created_at":"2026-09-13T19:36:54Z","status":"built","updated_at":"2026-09-13T19:37:09Z"}

$ curl https://matthewclaude.github.io/somnus-dial-releases/
HTTP 200 size 1000
http-equiv="refresh" content="0; url=https://matthewclaude.github.io/bedknob-for-somnus/"
<title>Bedknob for Somnus — flasher has moved
```

The owner also checked it in a browser and wrote (verbatim):

> Step 5 redirect confirmed in a real browser, not by fetch: a single tab loaded
> https://matthewclaude.github.io/somnus-dial-releases/ (200) and was then carried
> by the meta refresh to https://matthewclaude.github.io/bedknob-for-somnus/ (200),
> which rendered the full flasher page showing "Latest firmware: somnus-v1.0.1".
> Both requests are in the browser's own network log, in that order, same tab.

Full text of the redirect `index.html` (the old repo's `gh-pages` at `6aa3a12`; not kept as a file in this repo — see Deviations 6):

```html
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="0; url=https://matthewclaude.github.io/bedknob-for-somnus/">
<meta name="robots" content="noindex">
<link rel="canonical" href="https://matthewclaude.github.io/bedknob-for-somnus/">
<title>Bedknob for Somnus — flasher has moved</title>
<style>
  body { margin: 0; padding: 3rem 1.5rem; background: #101418; color: #F0F0E8;
         font: 16px/1.5 system-ui, -apple-system, "Segoe UI", sans-serif; text-align: center; }
  a { color: #D8B868; }
</style>
</head>
<body>
<p>The Bedknob for Somnus browser flasher has moved to<br>
<a href="https://matthewclaude.github.io/bedknob-for-somnus/">https://matthewclaude.github.io/bedknob-for-somnus/</a></p>
<p>You are being redirected. If nothing happens, follow the link above.</p>
<script>location.replace("https://matthewclaude.github.io/bedknob-for-somnus/");</script>
</body>
</html>
```

## Step 5 — archive (after a stop-and-ask)

Asked before archiving, with the state summarised. The owner's first reply was the browser confirmation quoted above; asked again for an explicit yes/no, the owner chose **"Yes, archive it"**.

```
$ gh repo archive matthewclaude/somnus-dial-releases --yes
(exit 0)
$ gh repo view matthewclaude/somnus-dial-releases --json name,isArchived,url
{"isArchived":true,"name":"somnus-dial-releases","url":"https://github.com/matthewclaude/somnus-dial-releases"}
```

## Step 6 — checks AFTER archiving

**`somnus-v1.0.1` on the old repo still exists with both assets** — checked after the archive:

```
$ gh release view somnus-v1.0.1 --repo matthewclaude/somnus-dial-releases --json tagName,isDraft,isPrerelease,publishedAt,assets
{"assets":[{"name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial-merged.bin"},{"name":"somnus-dial.bin","size":1612080,"state":"uploaded","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial.bin"}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-11T18:34:05Z","tagName":"somnus-v1.0.1"}

$ curl https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest      <- the endpoint a 1.0.0 dial polls
"tag_name": "somnus-v1.0.1"
"browser_download_url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial-merged.bin"
"browser_download_url": "https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.1/somnus-dial.bin"

$ curl -sSL .../somnus-v1.0.1/somnus-dial.bin                                                   <- the asset a 1.0.0 dial downloads
final HTTP 200  bytes 1612080
sha256 c381e13261cec624efc295a0ac658e9c196f8decedbf1d3128df70da76422782   == the Release's recorded digest

$ gh api repos/matthewclaude/somnus-dial-releases/branches --jq ".[].name"
gh-pages
main

$ curl https://matthewclaude.github.io/somnus-dial-releases/
HTTP 200 size 1000
http-equiv="refresh" content="0; url=https://matthewclaude.github.io/bedknob-for-somnus/"
<title>Bedknob for Somnus — flasher has moved

$ curl https://matthewclaude.github.io/bedknob-for-somnus/
HTTP 200 size 9879
<title>Bedknob for Somnus — flash from your browser
api.github.com/repos/matthewclaude/bedknob-for-somnus/releases/latest

$ gh secret list --repo matthewclaude/bedknob-for-somnus
(exit 0)            <- still empty

$ git fetch somnus; git rev-list --count somnus/main..main; git status --short
0
 M web-flasher/README.md
 M web-flasher/RELEASES-README.md
?? docs/REPORT-phase6b.md
```

## Build output

None. This phase built no firmware and changed no code: the only files touched in this repo are this report, its `REPORTS.md` line, `web-flasher/RELEASES-README.md` (a document that lives on another repo's `main`) and `web-flasher/README.md` (prose). `firmware/`, `.github/` and `web-flasher/index.html` are untouched, so there is nothing to build and no build output to show.

## Deviations

1. **Step 1 was not done in the earlier session.** The step 2.5 re-check found `SOMNUS_RELEASES_TOKEN` still listed; it was deleted in this session as the task block instructs. Recorded rather than a deviation from the block, but a deviation from what the earlier session's state implied.
2. **`web-flasher/README.md` edited** (not in the 6b list). It still said the release workflow publishes to the old repo's `gh-pages` and described `RELEASES-README.md` as "what ships there". Left as-is it would have contradicted the file next to it that now says the old repo is archived. Two paragraphs rewritten to say the workflow publishes to this repo's `gh-pages` and that `RELEASES-README.md` is the source of record for the archived repo's README. Uncommitted; easy to drop from the docs commit if unwanted.
3. **Step 2 took two questions, not one.** The owner's first answer was the offered option label "Yes, revoked (name via Other)" without a name; a second question obtained "somnus-dial-releases publish". Both quoted above.
4. **Step 5 took two questions.** The first reply described the browser check of the redirect and did not say yes or no to the archive; nothing was archived until the second, explicit "Yes, archive it".
5. **A stray `cat >` with no input hung the first file-writing command** for 120 s and it was backgrounded; the task was stopped. The README heredoc before it had already completed and the redirect `index.html` was rewritten in a clean command. No file on disk or on GitHub was affected.
6. **The redirect `index.html` is not kept as a file in this repo.** `web-flasher/index.html` is the live flasher and must stay so; the redirect exists on the old repo's `gh-pages` and in full above. If the owner wants a source copy in-tree, that is a separate decision.
7. **One extra download of the 1.0.1 asset.** Step 6 fetched `somnus-dial.bin` (1,612,080 bytes) once to prove the archived repo still serves it and to check its SHA-256; `downloadCount` on that asset moves from 1 to 2. A first `curl` write-out used a non-existent variable (`content_length`) and was re-run as a plain download.
8. **`docs/REPORTS.md` line added** for this report, per the standing docs convention (each new report gets a line). Uncommitted, riding the same docs commit.

## Not verifiable without hardware

Whether a dial still on `1.0.0` can actually migrate by installing the `1.0.1` Release from the archived repo. The bench dial is on `1.0.2-beta.1` and no `1.0.0` dial exists to test with. What this session shows is the server side only: the archived repo's `/releases/latest` answers `somnus-v1.0.1` and the app-only asset downloads intact with the right digest. The dial-side install from an archived repo was **not** demonstrated and this report does not claim it was.

## Closing state

- **Archived:** `matthewclaude/somnus-dial-releases` (`isArchived: true`). Not deleted, not renamed. Branches `main` (`e0f0b51`) and `gh-pages` (`6aa3a12`) both present. All 18 Releases present; `somnus-v1.0.1` (Latest) with `somnus-dial.bin` and `somnus-dial-merged.bin`, both `uploaded`, serving.
- **Gone:** the `SOMNUS_RELEASES_TOKEN` Actions secret on `bedknob-for-somnus` (deleted this session, list empty), and the fine-grained PAT "somnus-dial-releases publish" (revoked by the owner, per his word).
- **`https://matthewclaude.github.io/somnus-dial-releases/`** now serves the 1,000-byte meta-refresh redirect to the new flasher (HTTP 200, `<title>Bedknob for Somnus — flasher has moved`); its `manifest*.json` and `firmware/` images remain.
- **`https://matthewclaude.github.io/bedknob-for-somnus/`** serves the full flasher page (HTTP 200, 9,879 bytes) fetching `bedknob-for-somnus/releases/latest`; the owner saw it render "Latest firmware: somnus-v1.0.1".
- **This repo:** `main` = `9057d3f` = `somnus/main`, **0 commits ahead**. Working tree: modified `web-flasher/README.md`, `web-flasher/RELEASES-README.md`, `docs/REPORTS.md`; untracked `docs/REPORT-phase6b.md`. All ride the next docs commit; nothing committed, tagged or pushed in this block.
