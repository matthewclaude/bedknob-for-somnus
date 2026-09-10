# REPORT: release "latest" pointer verification

Date: 2026-09-10 (checks run ~13:46 UTC). Read-only verification run. No fix proposed or applied.

## VERDICT

**YES.** somnus-v1.0.0 is the release GitHub serves as "latest" for matthewclaude/somnus-dial-releases.
`gh release list` marks it `Latest`, `releases/latest` returns `somnus-v1.0.0`, and the byte-identical
binary at `firmware/latest/somnus-dial-merged.bin` hashes to the 1.0.0 asset digest.

## Gate check

This run made no writes to GitHub and no writes to the working tree other than this report file
(which the instructions required). No git write commands were run: no `git add`, `git commit`,
`git mv`, `git push`, `git checkout`, `git stash`, or plain `git status`. Git commands executed were
exactly `git --no-optional-locks status --short`, `git log --oneline -5`, `git rev-parse HEAD`, and
`git --no-optional-locks diff --stat`. The one downloaded file (the served firmware binary, for
hashing) went to the session scratchpad, not the repo.

Tooling: `gh version 2.100.0 (2026-09-03)`.

## RAW output

### 1. `gh release list --repo matthewclaude/somnus-dial-releases --limit 20`

```
somnus-v1.0.0	Latest	somnus-v1.0.0	2026-09-09T22:40:28Z
somnus-v0.1.6		somnus-v0.1.6	2026-09-07T20:49:39Z
somnus-v0.1.6-beta.3	Pre-release	somnus-v0.1.6-beta.3	2026-09-06T13:56:12Z
somnus-v0.1.6-beta.2	Pre-release	somnus-v0.1.6-beta.2	2026-09-06T00:13:11Z
somnus-v0.1.6-beta.1	Pre-release	somnus-v0.1.6-beta.1	2026-09-05T22:54:04Z
somnus-v0.1.5		somnus-v0.1.5	2026-09-05T01:09:56Z
somnus-v0.1.5-beta.5	Pre-release	somnus-v0.1.5-beta.5	2026-09-05T00:14:40Z
somnus-v0.1.5-beta.4	Pre-release	somnus-v0.1.5-beta.4	2026-09-04T02:09:06Z
somnus-v0.1.5-beta.3	Pre-release	somnus-v0.1.5-beta.3	2026-09-04T00:55:47Z
somnus-v0.1.5-beta.2	Pre-release	somnus-v0.1.5-beta.2	2026-09-03T20:26:54Z
somnus-v0.1.5-beta.1	Pre-release	somnus-v0.1.5-beta.1	2026-09-03T19:12:28Z
somnus-v0.1.4		somnus-v0.1.4	2026-09-03T16:17:47Z
somnus-v0.1.3		somnus-v0.1.3	2026-09-02T20:36:17Z
somnus-v0.1.2		somnus-v0.1.2	2026-09-02T03:27:57Z
somnus-v0.1.1		somnus-v0.1.1	2026-09-02T02:57:55Z
somnus-v0.1.0		somnus-v0.1.0	2026-09-02T02:26:51Z
exit=0
```

### 2. `gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,name,isLatest,isPrerelease,isDraft,createdAt,publishedAt,targetCommitish,assets`

Ran verbatim as instructed. It FAILED: `isLatest` is not a JSON field this gh version exposes.

```
Unknown JSON field: "isLatest"
Available fields:
  apiUrl
  assets
  author
  body
  createdAt
  databaseId
  id
  isDraft
  isImmutable
  isPrerelease
  name
  publishedAt
  tagName
  tarballUrl
  targetCommitish
  uploadUrl
  url
  zipballUrl
exit=1
```

**2b (supplementary, not in the instructions).** Same command with `isLatest` removed, so the
comparison section below can be computed:

`gh release view somnus-v1.0.0 --repo matthewclaude/somnus-dial-releases --json tagName,name,isPrerelease,isDraft,createdAt,publishedAt,targetCommitish,assets`

```
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/553715364","contentType":"application/octet-stream","createdAt":"2026-09-09T22:40:27Z","digest":"sha256:b5a6702e87bc75403806d00810be168a8b9c340979b84e390e6458569999021a","downloadCount":0,"id":"RA_kwDOULeAcc4hAQak","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-09T22:40:27Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.0/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/553715363","contentType":"application/octet-stream","createdAt":"2026-09-09T22:40:27Z","digest":"sha256:9ee90ab07cb32f630da3ffdc3ecfdfde2ec253688171d64e810bdaa6d3d06a9a","downloadCount":1,"id":"RA_kwDOULeAcc4hAQaj","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-09T22:40:27Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v1.0.0/somnus-dial.bin"}],"createdAt":"2026-09-05T02:41:05Z","isDraft":false,"isPrerelease":false,"name":"somnus-v1.0.0","publishedAt":"2026-09-09T22:40:28Z","tagName":"somnus-v1.0.0","targetCommitish":"main"}
exit=0
```

### 3. `gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json tagName,isLatest,isPrerelease,createdAt,publishedAt,targetCommitish`

Ran verbatim as instructed. It FAILED for the same reason as 2.

```
Unknown JSON field: "isLatest"
Available fields:
  apiUrl
  assets
  author
  body
  createdAt
  databaseId
  id
  isDraft
  isImmutable
  isPrerelease
  name
  publishedAt
  tagName
  tarballUrl
  targetCommitish
  uploadUrl
  url
  zipballUrl
exit=1
```

**3b (supplementary, not in the instructions).** Same command with `isLatest` removed and `assets`
added (needed for the byte-size comparison against 0.1.6):

`gh release view somnus-v0.1.6 --repo matthewclaude/somnus-dial-releases --json tagName,isPrerelease,createdAt,publishedAt,targetCommitish,assets`

```
{"assets":[{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362458","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:a353d2cbfc111f4b6a7820ad7c0c4f613b9a9249021c312c30c53452fa06a3d9","downloadCount":1,"id":"RA_kwDOULeAcc4gvpsa","label":"","name":"somnus-dial-merged.bin","size":1743152,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial-merged.bin"},{"apiUrl":"https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/assets/549362457","contentType":"application/octet-stream","createdAt":"2026-09-07T20:49:38Z","digest":"sha256:89449a1e66999d43a73ecd95efec158d2853fec85cd2aeb7bf36f0908a523666","downloadCount":2,"id":"RA_kwDOULeAcc4gvpsZ","label":"","name":"somnus-dial.bin","size":1612080,"state":"uploaded","updatedAt":"2026-09-07T20:49:38Z","url":"https://github.com/matthewclaude/somnus-dial-releases/releases/download/somnus-v0.1.6/somnus-dial.bin"}],"createdAt":"2026-09-05T02:41:05Z","isPrerelease":false,"publishedAt":"2026-09-07T20:49:39Z","tagName":"somnus-v0.1.6","targetCommitish":"main"}
exit=0
```

### 4. `gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq '.tag_name, .created_at, .published_at, .target_commitish'`

```
somnus-v1.0.0
2026-09-05T02:41:05Z
2026-09-09T22:40:28Z
main
exit=0
```

### 5. `gh api repos/matthewclaude/somnus-dial-releases/tags --paginate --jq '.[].name'`

```
somnus-v1.0.0
somnus-v0.1.6
somnus-v0.1.6-beta.3
somnus-v0.1.6-beta.2
somnus-v0.1.6-beta.1
somnus-v0.1.5
somnus-v0.1.5-beta.5
somnus-v0.1.5-beta.4
somnus-v0.1.5-beta.3
somnus-v0.1.5-beta.2
somnus-v0.1.5-beta.1
somnus-v0.1.4
somnus-v0.1.3
somnus-v0.1.2
somnus-v0.1.1
somnus-v0.1.0
exit=0
```

### 6. `curl -s https://matthewclaude.github.io/somnus-dial-releases/manifest.json`

```
{
  "name": "Bedknob for Somnus",
  "version": "latest",
  "new_install_prompt_erase": false,
  "builds": [
    {
      "chipFamily": "ESP32-S3",
      "parts": [
        { "path": "firmware/latest/somnus-dial-merged.bin", "offset": 0 }
      ]
    }
  ]
}
exit=0
```

### 7. `curl -sI https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin`

Content-Length: **1743152**. Last-Modified: **Wed, 09 Sep 2026 22:41:12 GMT**.

```
HTTP/2 200 
server: GitHub.com
content-type: application/octet-stream
last-modified: Wed, 09 Sep 2026 22:41:12 GMT
access-control-allow-origin: *
strict-transport-security: max-age=31556952
etag: "6aa1e088-1a9930"
expires: Thu, 10 Sep 2026 13:56:22 GMT
cache-control: max-age=600
x-proxy-cache: MISS
x-github-request-id: F3D2:37022F:15218E4:1747C73:6AA2B4AE
x-github-edge-region: iad
accept-ranges: bytes
age: 0
date: Thu, 10 Sep 2026 13:46:22 GMT
via: 1.1 varnish
x-served-by: cache-dfw-ktki8620029-DFW
x-cache: MISS
x-cache-hits: 0
x-timer: S1789047982.197946,VS0,VE190
vary: Accept-Encoding
x-fastly-request-id: 4813a134904b589a2ffadee6e6cd8f075ff65cfc
content-length: 1743152

exit=0
```

**7b (supplementary, not in the instructions).** Because the 1.0.0 and 0.1.6 merged assets have the
identical byte count (see comparison below), Content-Length alone cannot tell them apart. The served
file was downloaded (GET, to the scratchpad) and hashed:

```
$ curl -s -o $S/latest.bin https://matthewclaude.github.io/somnus-dial-releases/firmware/latest/somnus-dial-merged.bin && wc -c < $S/latest.bin && shasum -a 256 $S/latest.bin
 1743152
b5a6702e87bc75403806d00810be168a8b9c340979b84e390e6458569999021a
exit=0
```

### 8. Source repo working tree, read-only git

```
$ git --no-optional-locks status --short
 M docs/REPORT-handoff.md
exit=0

$ git log --oneline -5
11a00da docs: track housekeeping report
3fd1cc1 docs: report housekeeping — consistent 0.1.5 report names, TEST-F3 on disk, REPORTS.md index
195e77c docs: sim version/screens report
effb9c6 sim: read PROJECT_VER at build time; regenerate screens at 1.0.0
7c106fb chore: drop stale 1.0.0-reserved comment; track 1.0.0 commit report
exit=0

$ git rev-parse HEAD
11a00daededfc167cd81d0dc77d17c4a74101954
exit=0

$ git --no-optional-locks diff --stat
 docs/REPORT-handoff.md | 162 +++++++++++++++++++++++++++----------------------
 1 file changed, 90 insertions(+), 72 deletions(-)
exit=0
```

### 9. `web-flasher/index.html` version-string URL

Grep for URLs / manifest / version / fetch in the page:

```
$ grep -nE "https?://|manifest|version|\.json|fetch\(" web-flasher/index.html
179:  #latest-version {
199:    <esp-web-install-button manifest="./manifest.json">
222:    // Swapping the manifest is the whole channel switch: manifest-beta.json
231:        btn.setAttribute('manifest', box.checked ? './manifest-beta.json' : './manifest.json');
278:    <span id="latest-version">Latest firmware: checking&hellip;</span>
280:      <a href="https://github.com/matthewclaude/somnus-dial-releases">Releases</a>
281:      <a href="https://github.com/chris023/orion-waveshare-rotary-dial">Forked from Orion Dial</a>
282:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/LICENSE">License</a>
286:      <a href="https://polyformproject.org/licenses/noncommercial/1.0.0">PolyForm
288:      <a href="https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES">third-party
290:      <a href="https://esphome.github.io/esp-web-tools/">ESP Web Tools</a>.
295:      Required Notice: Copyright © 2026 Chris Meyer (<a href="https://github.com/chris023/orion-waveshare-rotary-dial">https://github.com/chris023/orion-waveshare-rotary-dial</a>)
300:<script type="module" src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"></script>
303:    var el = document.getElementById("latest-version");
304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
307:        return r.json();
exit=0
```

Surrounding lines of the version-fetch script, for context:

```
$ sed -n "300,315p" web-flasher/index.html
<script type="module" src="https://unpkg.com/esp-web-tools@10/dist/web/install-button.js?module"></script>
<script>
  (function () {
    var el = document.getElementById("latest-version");
    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
      .then(function (r) {
        if (!r.ok) throw new Error("bad response");
        return r.json();
      })
      .then(function (data) {
        if (data && data.tag_name) {
          el.textContent = "Latest firmware: " + data.tag_name;
        } else {
          throw new Error("no tag_name");
        }
      })
```

## Computed comparison: somnus-v1.0.0 vs somnus-v0.1.6

| field            | somnus-v1.0.0          | somnus-v0.1.6          |
|------------------|------------------------|------------------------|
| created_at       | 2026-09-05T02:41:05Z   | 2026-09-05T02:41:05Z   |
| published_at     | 2026-09-09T22:40:28Z   | 2026-09-07T20:49:39Z   |
| target_commitish | main                   | main                   |
| isPrerelease     | false                  | false                  |
| isDraft          | false                  | (not queried; listed as a normal release, not Draft/Pre-release) |

Working:

- `created_at` is **identical** for both releases (2026-09-05T02:41:05Z). GitHub documents a release's
  `created_at` as the date of the commit the release targets, not the publish date. Both releases
  target `main` in the releases repo, and that value shows `main` in the releases repo has not moved
  since 2026-09-05 (release commits and tags are created against the same commit).
- Because `created_at` ties, it cannot be the deciding field here. GitHub's `releases/latest`
  returned somnus-v1.0.0, whose `published_at` (09-09) is later than 0.1.6's (09-07), and whose
  release/asset ids are higher (asset ids 553715363/4 vs 549362457/8).
- Conclusion: with `created_at` tied, the "latest" ordering is effectively keying on
  `published_at` (equivalently, release id, which is monotonic with publish order). Both point to
  1.0.0. These values cannot distinguish `published_at` from release id as the tie-breaker, but the
  outcome is the same either way and it is the expected outcome.
- `target_commitish` is `main` for both, so it contributes nothing to the ordering.

## Byte size at firmware/latest/

| file                                              | bytes     | sha256 |
|---------------------------------------------------|-----------|--------|
| Pages `firmware/latest/somnus-dial-merged.bin`     | 1743152   | b5a6702e87bc75403806d00810be168a8b9c340979b84e390e6458569999021a |
| 1.0.0 asset `somnus-dial-merged.bin`               | 1743152   | b5a6702e87bc75403806d00810be168a8b9c340979b84e390e6458569999021a |
| 0.1.6 asset `somnus-dial-merged.bin`               | 1743152   | a353d2cbfc111f4b6a7820ad7c0c4f613b9a9249021c312c30c53452fa06a3d9 |

- The served size (1,743,152) matches **both** the 1.0.0 asset and the 0.1.6 asset (the 0.1.6
  figure given in the instructions, 1,743,152, is confirmed by the asset listing). Byte size is
  therefore inconclusive on its own. Merged images are padded to a fixed flash layout, so equal
  sizes across releases are expected.
- The sha256 of the served file **matches the 1.0.0 asset digest exactly** and does not match 0.1.6.
  The Pages ETag `"6aa1e088-1a9930"` also encodes 0x1a9930 = 1743152 bytes, and Last-Modified
  (09-09 22:41:12Z) is 44 seconds after the 1.0.0 publish time (22:40:28Z), consistent with the
  1.0.0 deploy writing it.
- Result: `firmware/latest/` is serving the **1.0.0** binary.

## Flasher page version-string URL

The page reads the version string from the GitHub Releases API "latest" endpoint, not from the
manifest or the Pages site. Quoted from `web-flasher/index.html`:

```
303:    var el = document.getElementById("latest-version");
304:    fetch("https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest")
...
311:          el.textContent = "Latest firmware: " + data.tag_name;
```

URL: `https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest`, displaying
`tag_name`. That endpoint returned `somnus-v1.0.0` in check 4, so the page will show
"Latest firmware: somnus-v1.0.0". The install button itself uses the relative `./manifest.json`
(line 199; `./manifest-beta.json` on the beta toggle, line 231), whose `version` field is the literal
string `latest` (check 6), so the version text and the flashed binary come from two different sources.

## git diff --stat and commit SHA

HEAD: `11a00daededfc167cd81d0dc77d17c4a74101954` (`11a00da docs: track housekeeping report`).

`git --no-optional-locks diff --stat` is **not empty**:

```
 docs/REPORT-handoff.md | 162 +++++++++++++++++++++++++++----------------------
 1 file changed, 90 insertions(+), 72 deletions(-)
```

That modification to `docs/REPORT-handoff.md` was already present before this run (it appears in the
git status snapshot taken at session start) and was not touched by this run. No other tracked files
differ. The only file this run created is `docs/REPORT-release-latest-pointer.md` (this report),
which will appear as untracked on the next status check.

## Deviations from instructions

1. Commands 2 and 3 were run verbatim and failed (`Unknown JSON field: "isLatest"` in gh 2.100.0).
   The raw failure output is preserved above. Supplementary commands 2b and 3b were run with that
   field removed (and `assets` added to 3b) so the required comparison and byte-size sections could
   be computed. The "is latest" fact was taken from commands 1 and 4 instead.
2. One additional read-only command not in the list (7b): the served `firmware/latest` binary was
   downloaded to the scratchpad and hashed with `shasum -a 256`, because both candidate assets have
   the same byte count and the instructions asked which one the served size matches.
3. `git --no-optional-locks diff --stat` was run (with the no-lock flag) because the report format
   requires the diff stat; the instruction list in step 8 did not name it, but the report section did.
4. `sed -n '300,315p'` on `web-flasher/index.html` was run to quote the lines around the fetch call
   with context; the grep in step 9 is the primary evidence.

No other deviations. Nothing on GitHub or on disk (other than this report file) was changed.

## Not verifiable in this run

- **Without hardware:** whether a dial running 0.1.6 actually sees and applies the 1.0.0 OTA, and
  whether the served `firmware/latest` binary boots on a dial. Only the served bytes were checked.
- **Without write access / repo contents:** the gh-pages branch of somnus-dial-releases was not
  inspected directly (only its served output over HTTPS). Whether `manifest-beta.json` and
  `firmware/beta/` exist or what they point to was not checked; the instructions only named
  `manifest.json` and `firmware/latest/`.
- Whether GitHub's tie-break is `published_at` or release id cannot be distinguished from these
  values alone (both give the same answer).
- The 1.0.0 workflow run in the source repo (bedknob-for-somnus) was not re-inspected; the
  instructions did not ask for it.
