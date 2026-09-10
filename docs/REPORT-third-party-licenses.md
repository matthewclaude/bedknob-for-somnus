# REPORT: third-party licenses — ESP-IDF attribution; Apache-2.0, OFL 1.1 and MIT texts appended; mirrored to the releases repo

**DONE** — `THIRD_PARTY_LICENSES.md` gains an `## ESP-IDF` section (v6.0, Espressif Systems (Shanghai) CO LTD, Apache-2.0) ahead of the managed-components table and a closing `## License texts` section with the full Apache License 2.0, SIL Open Font License 1.1 and MIT License texts, each taken from a file on disk or the named URL and hashed (commit `b0b0797` on `main`); the identical file was pushed to `matthewclaude/somnus-dial-releases` as `THIRD_PARTY_LICENSES` (commit `70c6299`, superseding a copy that had already diverged). No firmware file touched. Firmware-repo push and CI: see the post-push addendum at the end.

Date: 2026-09-10
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `1c8e37c9b2a579df9c9ed59e3303ad889e3dc3c5` = `somnus/main` (tree clean)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

## STEP 1 — starting state

```
$ wc -l THIRD_PARTY_LICENSES.md
     148 THIRD_PARTY_LICENSES.md
$ grep -n "^#" THIRD_PARTY_LICENSES.md
1:# Third-Party Licenses
20:## Rotary knob decoder — `bidi_switch_knob.c` / `.h`
35:## Waveshare demo-derived board bring-up
56:## Numeral font — `dial_font_num_88.c`
67:## Icon fonts — `dial_font_icons_16.c`, `dial_font_icons_20.c`
95:## Timezone table — `zones.csv`
105:## Build-time managed components (fetched by `idf.py`, not vendored)
118:## Image writer — `stb_image_write.h`
130:## `trust_roots.pem`
$ grep -n -iE "esp-idf|espressif" THIRD_PARTY_LICENSES.md
26:- **Origin:** Espressif's `iot_knob` decoder (the `knob` component in
31:  `SPDX-FileCopyrightText: 2016-2024 Espressif Systems (Shanghai) CO LTD` /
33:- **Link:** https://github.com/espressif/esp-iot-solution/tree/master/components/knob
44:  `08_LVGL_Test` / `04_Encoder_Test` ESP-IDF demo for the
108:Espressif Component Registry / upstream at build time — listed here for
114:| `espressif/cjson` (cJSON) | 1.7.19 | MIT |
115:| `espressif/esp_lcd_sh8601` | 2.0.1 | Apache-2.0 |
116:| `espressif/cmake_utilities` | 0.5.3 | Apache-2.0 |
(exit 0)
$ grep -n "esp_idf_version" .github/workflows/ci.yml .github/workflows/release.yml
.github/workflows/ci.yml:18:          esp_idf_version: v6.0
.github/workflows/release.yml:114:          esp_idf_version: v6.0
(exit 0)
```

Against the expectations: nine headings — yes. No ESP-IDF section and no appended license text — confirmed, so the premise holds. Espressif rows: the knob decoder section (lines 26–33) plus **four** managed-component table rows that mention Espressif (`espressif/cjson`, `espressif/esp_lcd_sh8601`, `espressif/cmake_utilities`, and the registry sentence at line 108); the task predicted only the knob decoder and cmake_utilities. Line 44 is a mention of Waveshare's "ESP-IDF demo", not an ESP-IDF attribution. None of this changes the work.

**release.yml finding:** it pins the same version by the same mechanism — `espressif/esp-idf-ci-action@v1` with `esp_idf_version: v6.0` at line 114 (ci.yml: line 18). The ESP-IDF section therefore names both workflows.

## STEP 2 — the three texts (none typed from memory)

| Text | Source used | sha256 of the text as appended | Verification strings found |
|---|---|---|---|
| Apache License 2.0 | `/Users/matthew/esp/esp-idf/LICENSE` (`$IDF_PATH` was unset in this shell; the `~/esp/esp-idf/LICENSE` candidate existed; that checkout reports `git describe` = `v6.0`). Copied verbatim, 202 lines, no edits. | `cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30` | "TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION" ×1, "END OF TERMS AND CONDITIONS" ×1 |
| SIL Open Font License 1.1 | `https://raw.githubusercontent.com/googlefonts/Montserrat/master/OFL.txt` (curl exit 0; raw file 93 lines, raw sha256 `41f82bb4d24b304f30f7136bc47abdd083782e4265c984160f5649d1e78ea49c`). Trimmed by dropping the header `Copyright 2011 The Montserrat Project Authors (https://github.com/JulietaUla/Montserrat)` plus its blank line; kept from the line "This Font Software is licensed under the SIL Open Font License, Version 1.1." (raw line 3) to the end. | `e564f06d018e7b95bc3594c96a17f1d41865af4038c375e7aa974dd69df38602` | "SIL OPEN FONT LICENSE Version 1.1" ×1, "PERMISSION & CONDITIONS" ×1 |
| MIT License | `firmware/dial-idf/managed_components/lvgl__lvgl/LICENCE.txt` (present on disk; raw 8 lines, raw sha256 `6a916130a36e83a79bb0dca6c6a7bfe60a67af7d118605e2b7b23bd5ad5c4eb0`). Its line 2, `Copyright (c) 2021 LVGL Kft`, replaced with exactly `Copyright (c) <year> <copyright holders> - see each MIT-licensed component's section above for its holder.`; its line 1 (`MIT licence`) and everything else kept verbatim, including LVGL's curly quotes around "Software" and "AS IS". | `3c5c1db1bcdbaae1f5daa3487ef38c7690c9d385a6d6183b3359c4b3bf7c3070` | "Permission is hereby granted, free of charge" ×1 in the MIT text |

Each hash is over the block as it sits in the file with one trailing newline, and was recomputed by re-extracting the block from the edited `THIRD_PARTY_LICENSES.md` between its `###` heading and the next heading (or end of file): all three re-extracted hashes match the values above, so the appended text is byte-identical to the prepared source.

## STEP 3 — the edit

**3a. ESP-IDF section, inserted immediately before `## Build-time managed components …`, in the existing sections' bullet shape:**

```
## ESP-IDF

- **What:** Espressif's IoT Development Framework — the operating system,
  network stack, drivers, HTTP/TLS client, OTA and NVS support that every
  shipped `somnus-dial.bin` is built on. The single largest body of code in
  the binary.
- **Version:** v6.0, as pinned by `esp_idf_version` in
  `.github/workflows/ci.yml` and `.github/workflows/release.yml` (both use
  `espressif/esp-idf-ci-action@v1` with the same pin).
- **Copyright:** Espressif Systems (Shanghai) CO LTD.
- **License:** Apache License 2.0 — full text appended below under
  "License texts".
- **Link:** https://github.com/espressif/esp-idf
- **Note:** ESP-IDF itself bundles further third-party code (FreeRTOS, lwIP,
  mbedTLS and others) under their own licenses; their notices live in the
  ESP-IDF distribution under `components/*/LICENSE` and are not reproduced
  here.
```

**3b. Appended at the end of the file:**

```
## License texts

These are the full texts of the licenses referenced above, reproduced so
that a copy of this file satisfies each license's requirement that the text
accompany the distribution.

### Apache License 2.0
<Apache text, verbatim, plain text, no fence>

### SIL Open Font License 1.1
<trimmed OFL text, verbatim, plain text, no fence>

### MIT License
<MIT text with the placeholder copyright line, verbatim, plain text, no fence>
```

**3c.** Nothing already in the file was removed or reworded; every existing link (`https://openfontlicense.org/`, the Montserrat, Bootstrap Icons, posix_tz_db, stb and esp-iot-solution links) is still in place. The diff is 334 insertions, 0 deletions.

**After the edit:**

```
$ wc -l THIRD_PARTY_LICENSES.md
     482 THIRD_PARTY_LICENSES.md
$ grep -n "^#" THIRD_PARTY_LICENSES.md
1:# Third-Party Licenses
20:## Rotary knob decoder — `bidi_switch_knob.c` / `.h`
35:## Waveshare demo-derived board bring-up
56:## Numeral font — `dial_font_num_88.c`
67:## Icon fonts — `dial_font_icons_16.c`, `dial_font_icons_20.c`
95:## Timezone table — `zones.csv`
105:## ESP-IDF
123:## Build-time managed components (fetched by `idf.py`, not vendored)
136:## Image writer — `stb_image_write.h`
148:## `trust_roots.pem`
168:## License texts
174:### Apache License 2.0
379:### SIL Open Font License 1.1
473:### MIT License
```

Line count: 148 before, 482 after (+334).

Verification-string counts in the whole file (`grep -c -F`):

| String | Count |
|---|---|
| TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION | 1 |
| END OF TERMS AND CONDITIONS | 1 |
| SIL OPEN FONT LICENSE Version 1.1 | 1 |
| PERMISSION & CONDITIONS | 1 |
| Permission is hereby granted, free of charge | **2** |

The MIT verification string appears twice because the OFL 1.1 text also opens its permission clause with the same words (file line 426, inside the OFL section; line 478 is the MIT one). That is inherent to the two texts, not a duplication of the MIT block — the MIT text itself is present exactly once. Listed under Deviations.

## STEP 4 — mirror to the public releases repo

```
$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases-mirror
clone exit 0; remote origin = https://github.com/matthewclaude/somnus-dial-releases.git
HEAD before: 4097b6e983e0037a41572325aac893656c453150  docs: Bedknob README, ESP-IDF attribution, full third-party license texts
```

**Pre-edit diff** (`diff <clone>/THIRD_PARTY_LICENSES <(git show HEAD:THIRD_PARTY_LICENSES.md)`; `<` = public copy, `>` = firmware copy before today's edit). The two had **already diverged**: the public copy was 520 lines, the firmware copy 148. Hunks, with the long license-text bodies abridged (the full removed text is recoverable as `git show 4097b6e:THIRD_PARTY_LICENSES` in the releases repo):

```
14,18c14,18
< 1.0.0 terms, © 2026 the Somnus Dial contributors. Both layers also build on
< hardware bring-up code, ... (same paragraph, older name and line-wrapping)
---
> 1.0.0 terms, © 2026 the Bedknob for Somnus contributors. Both layers also build on hardware
> bring-up code, ...
74c74,75
<   `DIAL_ICON_SNOW3` today. The two files are still listed as build sources in
---
>   `DIAL_ICON_SNOW3` today (confirmed by searching every `dial_ui`/`main`
>   source file). The two files are still listed as build sources in
76,77c77,79
<   build's ELF/map — the linker drops them since nothing references them, so
<   despite shipping as source, no icon glyph data currently reaches the
---
>   build's ELF/map (`somnus-dial.elf`/`.map`, spot-checked against a build
>   from this same day) — the linker drops them since nothing references them,
>   so despite shipping as source, no icon glyph data currently reaches the
79,80c81,82
<   source, and attribution doesn't depend on whether the linker happens to
<   keep something.
---
>   source, attribution doesn't depend on whether the linker happens to keep
>   something.
98c100,101
< - **Origin:** `nayarsystems/posix_tz_db`.
---
> - **Origin:** `nayarsystems/posix_tz_db`, cited in
>   `firmware/dial-idf/components/dial_time/dial_time.c`.
114d116
< | [`espressif/esp-idf`](https://github.com/espressif/esp-idf) — the ESP-IDF framework itself, Espressif Systems | v6.0 | Apache-2.0 |
136,520c138,148
<   (trust_roots paragraph in a shorter, path-free wording)
< 
< ## Full license texts
< ### Apache License 2.0        (in a ``` fence, with an "Applies to:" list)
< ### MIT License               (in a ``` fence: Bootstrap Authors copyright line, plus a
<                                per-holder list for LVGL / cJSON / Bootstrap Icons / stb /
<                                posix_tz_db, plus stb's "ALTERNATIVE B - Public Domain" text)
< ### SIL Open Font License 1.1 (in a ``` fence, including the Montserrat 2024 header)
---
>   (trust_roots paragraph in the firmware repo's longer wording naming
>    `components/dial_oauth/orion_root_ca.pem` and Orion's chain)
```

So the public copy, from the 2026-09-04 pass (`docs/REPORT-releases-repo-readme.md`), already had an ESP-IDF table row and fenced full texts; the firmware copy never got them. Per the task, the new firmware file supersedes both. Overwriting the public copy therefore **drops** these items that only it had: the ESP-IDF row in the managed-components table (replaced by the new section), the fenced formatting, the "Applies to" lists, the per-holder MIT copyright list, and stb's "Alternative B" public-domain text. It **adds** the firmware copy's fuller prose (Orion-chain detail in `trust_roots.pem`, the `dial_time.c` citation, the "Bedknob for Somnus contributors" wording). Flagged under Deviations for the owner; proceeded as instructed.

**Mirror commit and push** (in the clone; the firmware file was copied over `THIRD_PARTY_LICENSES`, name kept, `cmp` against the firmware copy reports identical):

```
 THIRD_PARTY_LICENSES | 162 ++++++++++++++++++++-------------------------------
 1 file changed, 62 insertions(+), 100 deletions(-)
mirror commit: 70c62990b171f1899f6b95a51bf3812667d57df5  docs: ESP-IDF attribution; append Apache-2.0, OFL 1.1 and MIT texts
$ git push origin main        (that clone's origin = matthewclaude/somnus-dial-releases)
To https://github.com/matthewclaude/somnus-dial-releases.git
   4097b6e..70c6299  main -> main
git ls-remote origin refs/heads/main → 70c62990b171f1899f6b95a51bf3812667d57df5
```

Clone directory `/tmp/somnus-dial-releases-mirror` removed afterwards (confirmed absent).

## Commits — `git diff --stat` per firmware-repo commit, every SHA

**Commit A (STEP 3)** — `b0b07970ce9282c5229db3499f7d6cef9805a142` — "docs: ESP-IDF attribution; append Apache-2.0, OFL 1.1 and MIT texts" (body names the three sources and their hashes)

```
 THIRD_PARTY_LICENSES.md | 334 ++++++++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 334 insertions(+)
```

**Commit B (this report + its `docs/REPORTS.md` line)** — SHA in the addendum below and in the chat reply.

**Commit C (post-push addendum)** — SHA in the chat reply only.

**Releases repo:** `70c62990b171f1899f6b95a51bf3812667d57df5` on `main` (previous HEAD `4097b6e`).

## Firmware-repo push and CI

See the post-push addendum at the end of this report.

## Deviations

- **The public releases copy had already diverged before today** (see STEP 4): it carried an ESP-IDF table row and fenced Apache/MIT/OFL texts from the 2026-09-04 pass, which the firmware copy never received. Proceeded as instructed; the mirror now equals the firmware file. The owner may want to restore, in the firmware file, two things the old public copy had and the new one lacks: the per-holder MIT copyright list (LVGL Kft; Dave Gamble and cJSON contributors; The Bootstrap Authors; Sean Barrett; posix_tz_db) and stb's public-domain "Alternative B" text. Not done here — outside the specified content.
- **The MIT verification string occurs twice in the finished file**, not once, because the OFL 1.1 text contains the same opening words. The MIT block itself is present once (re-extracted hash matches). The other four verification strings occur exactly once each.
- **STEP 1's expectation about "Espressif rows"** listed the knob decoder and cmake_utilities; the table also has `espressif/cjson` and `espressif/esp_lcd_sh8601` rows. No effect on the work.
- **The MIT text keeps LVGL's first line `MIT licence`** (British spelling) under the `### MIT License` heading, because the task said to replace only the copyright line. Cosmetic.
- **Three commits after STEP 3 rather than two**, as the task itself instructed (addendum pattern). The addendum push triggers one further ci.yml run not tracked here.
- Otherwise none: `$IDF_PATH` was unset but the second candidate path existed, so no fallback fetch was needed for Apache; the LVGL MIT file was on disk, so no fallback fetch was needed for MIT; nothing pushed to the firmware repo's `origin`.

## Cannot verify from disk

- **Live flasher footer.** Checked over the network, not from disk: the served page at `https://matthewclaude.github.io/somnus-dial-releases/` (and the `gh-pages` branch's `index.html`, line 288) links the footer's "third-party" text to `https://github.com/matthewclaude/somnus-dial-releases/blob/main/THIRD_PARTY_LICENSES`, which returned HTTP 200 before the mirror push. Since the file kept its extensionless name and lives on `main`, that link now resolves to the updated file; the addendum records a post-push check of the raw content.
- **Whether GitHub renders the plain-text license blocks acceptably.** The Apache text is indented, so GitHub's Markdown renderer will show it as an indented code block; the OFL and MIT texts are flush-left paragraphs. The task asked for plain text, not a fence, so this is as specified; not checked visually.
- **Whether ESP-IDF v6.0's `LICENSE` is byte-identical to `https://www.apache.org/licenses/LICENSE-2.0.txt`.** Not fetched (the preferred source was available). Its sha256 `cfc7749b…` is the widely published hash of the canonical Apache-2.0 text, but that equivalence was not verified here.
