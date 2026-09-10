# REPORT: third-party licenses restore — per-holder MIT notices and stb Alternative B text, re-mirrored

**DONE** — the per-holder MIT copyright list and stb's "ALTERNATIVE B - Public Domain" text were recovered verbatim from the releases repo's `THIRD_PARTY_LICENSES` at `4097b6e` and restored into the firmware repo's `THIRD_PARTY_LICENSES.md` (commit `2207b1d` on `main`; the Apache-2.0 and OFL 1.1 blocks re-hash to their previous values), and the identical file was pushed to `matthewclaude/somnus-dial-releases` as `21c239d`. No firmware file touched. Firmware-repo push and CI: see the post-push addendum at the end.

Date: 2026-09-11
Repo: `~/Projects/somnus-waveshare-rotary-dial` (matthewclaude/bedknob-for-somnus), branch `main`
Starting HEAD: `74f0feff0ded1498071d7410799ca77cb94bc88f` = `somnus/main` (tree clean)

## Gate check

| Check | Command | Result |
|---|---|---|
| 1 | `sed -n '21p' firmware/dial-idf/CMakeLists.txt` | `set(PROJECT_VER "1.0.0")` — PASS |
| 2 | `git tag -l 'somnus-v1.0.0'` | `somnus-v1.0.0` — PASS |
| 3 | `git --no-optional-locks status --short --untracked-files=no` | (empty) — PASS |

## STEP 1 — recover the source

```
$ gh api "repos/matthewclaude/somnus-dial-releases/contents/THIRD_PARTY_LICENSES?ref=4097b6e" \
    -H "Accept: application/vnd.github.raw" > /tmp/tpl-old.txt
fetch exit 0
$ wc -l /tmp/tpl-old.txt
     520 /tmp/tpl-old.txt
$ shasum -a 256 /tmp/tpl-old.txt
cf9f686f88039487af55edf7f06be706aca65a2d84902c1494561956f690dde3
```

Contains "Alternative B" (1 occurrence) and a `### MIT License` heading at line 361. Headings in the recovered file: `## Full license texts` at 145, `### Apache License 2.0` at 150, `### MIT License` at 361, `### SIL Open Font License 1.1` at 421.

## STEP 2 — the two blocks, verbatim

**2a. Per-holder list** — `/tmp/tpl-old.txt` lines 363–373 (the lead-in and the five items; sha256 of the block with one trailing newline `d7ba601c4ce66ec92a305a922fefc0c356ee90137e5dcd43fa63c735abf8fc48`):

```
Applies to, with each project's copyright line as published in its own
`LICENSE` file:

- LVGL — Copyright (c) 2021 LVGL Kft
- cJSON — Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
- Bootstrap Icons — Copyright (c) 2019-2024 The Bootstrap Authors
- `stb_image_write.h` — Copyright (c) 2017 Sean Barrett (MIT is
  "Alternative A" of stb's dual license; the public-domain alternative is
  reproduced after it)
- `posix_tz_db` — the upstream `LICENSE` file is the MIT template with its
  copyright placeholder left as `[year] [fullname]`; reproduced as published.
```

**2b. stb Alternative B** — lead-in at line 399, the text at lines 402–418 (the old file wrapped it in a ``` fence at lines 401 and 419; sha256 of the 17 text lines with one trailing newline `0eca45c095aa3f457f0ee3fe032649ddd932da86dcbdb840c4ad79d74d0bcf1b`):

```
stb's Alternative B, at the user's option instead of MIT:

ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## STEP 3 — the edit

Both replacements were exact single-occurrence string operations (asserted). Line count 482 → 513; headings unchanged except the one addition at the end (`### stb_image_write.h - public domain (Alternative B)` at line 493).

**3a. `### MIT License` subsection as it now reads** (the `MIT licence` line and the placeholder copyright line are gone; the rest of the MIT text is byte-for-byte what LVGL's `LICENCE.txt` carries, curly quotes included):

```
### MIT License

Applies to, with each project's copyright line as published in its own
`LICENSE` file:

- LVGL — Copyright (c) 2021 LVGL Kft
- cJSON — Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
- Bootstrap Icons — Copyright (c) 2019-2024 The Bootstrap Authors
- `stb_image_write.h` — Copyright (c) 2017 Sean Barrett (MIT is
  "Alternative A" of stb's dual license; the public-domain alternative is
  reproduced after it)
- `posix_tz_db` — the upstream `LICENSE` file is the MIT template with its
  copyright placeholder left as `[year] [fullname]`; reproduced as published.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

**3b. New closing subsection as it now reads** (plain text, no fence, matching the file's convention for the other license texts):

```
### stb_image_write.h - public domain (Alternative B)

stb's Alternative B, at the user's option instead of MIT:

ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

**3c. Checks:**

| Check | Result |
|---|---|
| "Permission is hereby granted, free of charge" occurs exactly twice (OFL + MIT) | 2 — PASS |
| "Alternative B" occurs at least once | 2 (once in the list item, once in the new heading) — PASS |
| Apache block re-extracted between `### Apache License 2.0` and `### SIL Open Font License 1.1`, sha256 | `cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30` — matches — PASS |
| OFL block re-extracted between `### SIL Open Font License 1.1` and `### MIT License`, sha256 | `e564f06d018e7b95bc3594c96a17f1d41865af4038c375e7aa974dd69df38602` — matches — PASS |

Nothing else in the file changed: the diff is 33 insertions, 2 deletions, all inside the last 41 lines.

## STEP 4 — re-mirror

```
$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases-mirror
clone exit 0; origin = https://github.com/matthewclaude/somnus-dial-releases.git; HEAD 70c62990b171f1899f6b95a51bf3812667d57df5
$ cp THIRD_PARTY_LICENSES.md <clone>/THIRD_PARTY_LICENSES        (name kept)
$ cmp <clone>/THIRD_PARTY_LICENSES THIRD_PARTY_LICENSES.md
identical
 THIRD_PARTY_LICENSES | 35 +++++++++++++++++++++++++++++++++--
 1 file changed, 33 insertions(+), 2 deletions(-)
mirror commit: 21c239d37550fc4a6f066bd0d57220c5041d69f9  docs: restore per-holder MIT notices and stb Alternative B text
$ git push origin main         (that clone's origin = the releases repo)
To https://github.com/matthewclaude/somnus-dial-releases.git
   70c6299..21c239d  main -> main
git ls-remote origin refs/heads/main → 21c239d37550fc4a6f066bd0d57220c5041d69f9
```

Clone directory removed afterwards (confirmed absent). Post-push network check: `https://raw.githubusercontent.com/matthewclaude/somnus-dial-releases/main/THIRD_PARTY_LICENSES` is byte-identical to the firmware file (`cmp` clean).

## Commits — `git diff --stat` per firmware-repo commit, every SHA

**Commit A (STEP 3)** — `2207b1da6d02ef1d5da6508a53ab73ea487cbd2e` — "docs: restore per-holder MIT notices and stb Alternative B text" (body says both were recovered from the releases repo's `4097b6e`)

```
 THIRD_PARTY_LICENSES.md | 35 +++++++++++++++++++++++++++++++++--
 1 file changed, 33 insertions(+), 2 deletions(-)
```

**Commit B (this report + its `docs/REPORTS.md` line)** — SHA in the addendum below and in the chat reply.

**Commit C (post-push addendum)** — SHA in the chat reply only.

**Releases repo:** `21c239d37550fc4a6f066bd0d57220c5041d69f9` on `main` (previous HEAD `70c6299`).

## Firmware-repo push and CI

See the post-push addendum at the end of this report.

## Deviations

- **The old file's ``` fence around the Alternative B text was not carried over.** The task said to add the 2b text verbatim; the fence markers are Markdown wrapping, not text, and the firmware file's License texts section deliberately holds every license as plain text (per the previous pass). The 17 text lines are byte-identical to the source (hash above).
- **The `MIT licence` line was removed as instructed**, so the MIT subsection now opens with the "Applies to" lead-in rather than a title line. That is what 3a asked for; noted only because it is a deletion in a file whose other passes were insert-only.
- **Three commits after STEP 3 rather than two**, as instructed (addendum pattern). The addendum push triggers one further ci.yml run not tracked here.
- Otherwise none: the `gh api` fetch succeeded on the first try, both blocks were found where the task expected, all 3c checks passed, nothing pushed to the firmware repo's `origin`.

## Cannot verify from disk

- **Whether the five copyright lines in the per-holder list are still what each upstream publishes today** (LVGL Kft 2021; Dave Gamble 2009-2017; The Bootstrap Authors 2019-2024; Sean Barrett 2017; posix_tz_db's `[year] [fullname]` placeholder). They were restored exactly as the 2026-09-04 pass wrote them; the RULE for this task forbade any other source, so none was re-checked against upstream.
- **GitHub's rendering** of the restored plain-text blocks was not looked at visually. The raw file matches; how the flush-left Alternative B paragraph and the bulleted list render is as specified, not checked.
- **CI at the time of writing** — see addendum.
