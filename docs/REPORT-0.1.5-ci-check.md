# REPORT: 0.1.5 stable release — CI / publish check

**Verdict: RELEASE BUILT AND PUBLISHED AS STABLE. Workflow run 33935053719 completed/success in 5m20s; `somnus-v0.1.5` exists on `matthewclaude/somnus-dial-releases` with `isPrerelease: false`, `isDraft: false`, both assets attached; `/releases/latest` now returns `somnus-v0.1.5`.**

Date: 2026-09-05 01:04–01:11 UTC
Read-only. No edits, no git writes.

## Confirmations

| # | Being confirmed | Result |
|---|---|---|
| A | Workflow run for the `somnus-v0.1.5` tag push completed with conclusion `success` | **PASS** — run 33935053719, `completed` / `success`, 5m20s (build-and-release 5m0s, deploy-pages 12s) |
| B | Release `somnus-v0.1.5` is stable, not a prerelease | **PASS** — `isPrerelease: false`, `isDraft: false` |
| C | `/releases/latest` points at 0.1.5 (moved off 0.1.4) | **PASS** — returns `somnus-v0.1.5` |

Assets: `somnus-dial-merged.bin` (1,740,944 bytes) and `somnus-dial.bin` (1,609,872 bytes)
both attached.

Note: the run was still `in_progress` (1m51s) when first listed; `gh run watch --exit-status`
was used to wait for completion (exited 0 at 01:10:29Z) before the remaining checks ran.

## 1. `gh run list --repo matthewclaude/somnus-waveshare-rotary-dial --limit 5`

```
completed	success	release: 0.1.5 — first stable release since 0.1.4, graduates beta.1-b…	release	somnus-v0.1.5	push	33935053719	5m20s	2026-09-05T01:04:55Z
completed	success	feat(power): battery percentage + About screen redesign (0.1.5-beta.5)	release	somnus-v0.1.5-beta.5	push	33932029987	5m28s	2026-09-05T00:09:26Z
completed	success	release: 0.1.5-beta.4	release	somnus-v0.1.5-beta.4	push	33828089795	5m44s	2026-09-04T02:03:36Z
completed	success	release: 0.1.5-beta.3	release	somnus-v0.1.5-beta.3	push	33823415650	5m27s	2026-09-04T00:50:37Z
completed	success	release: 0.1.5-beta.2	release	somnus-v0.1.5-beta.2	push	33801718212	6m16s	2026-09-03T20:20:52Z
```

Run triggered by the `somnus-v0.1.5` tag push: **33935053719**.

## 2. `gh run view 33935053719 --repo matthewclaude/somnus-waveshare-rotary-dial`

```
✓ somnus-v0.1.5 release · 33935053719
Triggered via push about 5 minutes ago

JOBS
✓ build-and-release in 5m0s (ID 101221230418)
✓ deploy-pages in 12s (ID 101222001984)

ANNOTATIONS
! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4, softprops/action-gh-release@v2. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
build-and-release: .github#2

! Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/download-artifact@v4. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/
deploy-pages: .github#2

ARTIFACTS
somnus-dial-merged

For more information about a job, try: gh run view --job=<job-id>
View this run on GitHub: https://github.com/matthewclaude/somnus-waveshare-rotary-dial/actions/runs/33935053719
```

`--json status,conclusion,createdAt,updatedAt,url`:

```
{"conclusion":"success","createdAt":"2026-09-05T01:04:55Z","status":"completed","updatedAt":"2026-09-05T01:10:15Z","url":"https://github.com/matthewclaude/somnus-waveshare-rotary-dial/actions/runs/33935053719"}
```

Duration: 5m20s (01:04:55Z → 01:10:15Z). The two annotations are Node 20 deprecation
warnings on third-party actions, not failures; they appear on every prior run too.

## 3. `gh release view somnus-v0.1.5 --repo matthewclaude/somnus-dial-releases`

```
title:	somnus-v0.1.5
tag:	somnus-v0.1.5
draft:	false
prerelease:	false
immutable:	false
author:	matthewclaude
created:	2026-09-02T02:48:24Z
published:	2026-09-05T01:09:56Z
url:	https://github.com/matthewclaude/somnus-dial-releases/releases/tag/somnus-v0.1.5
asset:	somnus-dial-merged.bin
asset:	somnus-dial.bin
--
[body = CHANGELOG.md "## 0.1.5 — 2026-09-05" section verbatim, followed by the
workflow's standard OTA/flasher footer and the Required Notice line — omitted here for
length; confirmed present and matching the committed CHANGELOG text.]
```

`--json isDraft,isPrerelease,tagName,publishedAt,assets`:

```
{"assets":[{"name":"somnus-dial-merged.bin","size":1740944},{"name":"somnus-dial.bin","size":1609872}],"isDraft":false,"isPrerelease":false,"publishedAt":"2026-09-05T01:09:56Z","tagName":"somnus-v0.1.5"}
```

(`created: 2026-09-02T02:48:24Z` is the date of the commit the tag points at in the
public repo — same as every release since 0.1.1 — not the publish time; see
SPEC-ota-readiness §9.7.)

## 4. `gh api repos/matthewclaude/somnus-dial-releases/releases/latest --jq '.tag_name'`

```
somnus-v0.1.5
```

## Supplementary (read-only, beyond the four requested commands)

**Release list order as the 0.1.4 beta-on code path sees it** (`/releases?per_page=5`):

```
somnus-v0.1.5  prerelease=false  created=2026-09-02T02:48:24Z  id=383090668
somnus-v0.1.4  prerelease=false  created=2026-09-02T02:48:24Z  id=382166589
somnus-v0.1.3  prerelease=false  created=2026-09-02T02:48:24Z  id=381557888
somnus-v0.1.2  prerelease=false  created=2026-09-02T02:48:24Z  id=380957827
somnus-v0.1.1  prerelease=false  created=2026-09-02T02:48:24Z  id=380948544
```

`0.1.5` landed **first**, inside the five-entry window. So a `0.1.4` dial with Beta builds
ON — still running the old list scan (SPEC §9.9) — will also discover `0.1.5`. This
closes the one caveat left open in `REPORT-0.1.5-tag-push.md`: both toggle positions on a
`0.1.4` dial now reach `0.1.5`. (Empirical for this repo's current state; the ordering is
still GitHub's tie-break, which is exactly why the fixed code no longer relies on it.)

**Stable Pages channel** (`firmware/latest/somnus-dial-merged.bin`):

```
HTTP/2 200
last-modified: Sat, 05 Sep 2026 01:10:35 GMT
content-length: 1740944
```

Size matches the `somnus-dial-merged.bin` release asset. The browser flasher's default
(stable) manifest now serves the 0.1.5 image.

## Follow-ups not performed here

- SPEC-ota-readiness §9.9's open item ("cut a new stable release carrying the fix?") is
  now answered by this release and can be closed out in the SPEC.
- On-device confirmation: a `0.1.4` unit (the bench dial captured in
  `REPORT-ota-beta-not-found.md`) should now log `latest 0.1.5, running 0.1.4 -- update
  available` on its next check. Not observed yet.
