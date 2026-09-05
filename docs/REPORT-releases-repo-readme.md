# REPORT: releases-repo README and third-party license fixes

Date: 2026-09-04
Target: https://github.com/matthewclaude/somnus-dial-releases (public), branch `main` only.
Run from: ~/Projects/somnus-waveshare-rotary-dial. Nothing committed in this repo.

## Verdict

**DONE.** README replaced with the Bedknob text (with the "open" phrase corrected to source-available), ESP-IDF row added, full Apache-2.0 / MIT / OFL-1.1 texts appended to the extensionless `THIRD_PARTY_LICENSES`, committed as `4097b6e` and pushed to `main`. gh-pages, releases and the repo name untouched. One deviation (a broken link fixed in the README) and one judgment call on the ESP-IDF row, both below.

## Gate

```
$ gh auth status
github.com
  ✓ Logged in to github.com account matthewclaude (keyring)
  - Active account: true
  - Git operations protocol: https
  - Token: gho_************************************
  - Token scopes: 'gist', 'read:org', 'repo', 'workflow'
exit=0

$ gh repo view matthewclaude/somnus-dial-releases --json name
{"name":"somnus-dial-releases"}
exit=0
```

## Step 1: clone

```
$ gh repo clone matthewclaude/somnus-dial-releases /tmp/somnus-dial-releases
Cloning into '/tmp/somnus-dial-releases'...
clone exit=0
```

Branches seen: `main` (HEAD, at `380a688 Update THIRD_PARTY_LICENSES`), `origin/gh-pages`. Files at root: `LICENSE`, `README.md`, `THIRD_PARTY_LICENSES` (extensionless, as the flasher footer at `web-flasher/index.html:288` expects).

## Step 2 + 4: README diff (full)

Source: `web-flasher/RELEASES-README.md` copied over `README.md`, then the step-4 phrase change and the link fix noted under Deviations. This is the diff of the committed result against the previous `main`:

```diff
diff --git a/README.md b/README.md
index c7fcc29..046158d 100644
--- a/README.md
+++ b/README.md
@@ -1,14 +1,17 @@
 # somnus-dial-releases
 
-Firmware releases and the browser flasher for **Somnus Dial** — a bedside
+Firmware releases and the browser flasher for **Bedknob for Somnus** — a bedside
 dial that turns a Waveshare ESP32-S3 round touch-LCD knob into a standalone
 temperature control for a Somnus Pad. Not affiliated with, endorsed by, or
-supported by Somnus or Waveshare.
+supported by Somnus Lab or Waveshare.
 
-**This repo holds binaries, a manifest, and the flasher page only — it is not
-the firmware's source.** The source lives in a private repository. That's a
-deliberate choice about how this project is developed, not a change to the
-license below. If you want the source, ask the maintainer.
+**This repo holds binaries, a manifest, and this flasher page only — it is
+not the firmware's source.** The source lives in a private repository.
+That's a deliberate choice about this project's development, not a
+statement about the license below: the firmware is source-available
+under the same PolyForm Noncommercial terms it always has been, this repo
+just isn't where the buildable source happens to live. If you want the
+source, ask the maintainer.
 
 ## Install
 
@@ -17,38 +20,33 @@ in Chrome or Edge on a desktop computer, plug the dial in over USB-C, and
 click Install. No software to install locally — the page flashes the board
 directly from your browser over Web Serial.
 
-Already have a dial running this firmware? You don't need this page again.
-The dial checks for updates periodically on its own and tells you when one is
-available, and you can check any time from Menu → Update on the dial itself.
-
-Note that flashing from this page **erases the dial's settings** — Wi-Fi
-credentials, timezone and pad address — because the image it writes covers
-the whole flash. That's fine for a first install. To update a dial you're
-already using, use the over-the-air update instead; it preserves everything.
+Already have a dial running this firmware? You don't need this page again —
+updates arrive over the air (Menu → Update on the dial itself, or
+automatically overnight if you've turned that on).
 
 ## What's in a release
 
 Each [release](https://github.com/matthewclaude/somnus-dial-releases/releases)
 carries two files:
 
-- `somnus-dial.bin` — the app-only image. This is what a dial already running
-  this firmware downloads and installs over the air; it must never be flashed
-  directly at offset `0x0`.
-- `somnus-dial-merged.bin` — the same firmware bundled with the bootloader and
-  partition table into one image flashable at offset `0x0` on a blank or
-  already-flashed chip. This is what the browser flasher uses, and what you'd
-  use with `esptool` manually.
+- `somnus-dial.bin` — the app-only image. This is what a dial already
+  running this firmware downloads and installs over the air; it must never
+  be flashed directly at offset `0x0`.
+- `somnus-dial-merged.bin` — the same firmware, bundled with the bootloader
+  and partition table into one image flashable at offset `0x0` on a blank
+  or already-flashed chip. This is what the browser flasher above uses, and
+  what you'd use with `esptool` manually.
 
-A release marked **pre-release** on GitHub is a beta build — offered only to
-dials with "Beta builds" turned on under Menu → Update, or via the flasher
-page's beta checkbox.
+A release marked **pre-release** on GitHub is a beta build — visible only
+to dials that have turned on "Beta builds" under Menu → Update, or to the
+flasher page's beta checkbox.
 
 ## License and attribution
 
 This firmware is a fork of
 [chris023/orion-waveshare-rotary-dial](https://github.com/chris023/orion-waveshare-rotary-dial),
 licensed under the **PolyForm Noncommercial License 1.0.0** — free for
-personal, noncommercial use. See [`LICENSE`](LICENSE) for the full terms and
-required notice, and [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md) for
-the hardware bring-up code, fonts, data, and libraries this project builds on,
-each under its own terms.
+personal, noncommercial use. See [`LICENSE`](LICENSE) for the full terms
+and required notice, and [`THIRD_PARTY_LICENSES`](THIRD_PARTY_LICENSES)
+for the hardware bring-up code, fonts, data, and libraries this project
+builds on, each under its own terms.
```

## Step 3: THIRD_PARTY_LICENSES diff (full)

3a. ESP-IDF row added to the managed-components table (it was absent — the file mentioned Espressif only via the knob decoder and two managed components).
3b. A new `## Full license texts` section appended with the complete Apache License 2.0, MIT License, stb's public-domain alternative, and SIL Open Font License 1.1 texts. Sources of the texts:

| Text | Taken from |
|---|---|
| Apache-2.0 | `~/esp/esp-idf/LICENSE` (the canonical 202-line text; byte-identical in wording to the esp-iot-solution and esp_lcd_sh8601 copies) |
| MIT | `twbs/icons` `LICENSE` from GitHub (standard MIT wording, © 2019-2024 The Bootstrap Authors); the other MIT holders' copyright lines are listed above the text, taken from their own LICENSE files: LVGL (`managed_components/lvgl__lvgl/LICENCE.txt`), cJSON (`managed_components/espressif__cjson/cJSON/LICENSE`), stb (`simulator/vendor/stb_image_write.h` license block), posix_tz_db (upstream `LICENSE` — its copyright placeholder is unfilled upstream, `[year] [fullname]`, and the file says so) |
| stb Alternative B | `simulator/vendor/stb_image_write.h` lines 1706-1722 |
| OFL 1.1 | `JulietaUla/Montserrat` `OFL.txt` from GitHub (includes the project's own copyright header line) |

```diff
diff --git a/THIRD_PARTY_LICENSES b/THIRD_PARTY_LICENSES
index 09a5ea0..16e3ee5 100644
--- a/THIRD_PARTY_LICENSES
+++ b/THIRD_PARTY_LICENSES
@@ -111,6 +111,7 @@ transparency, not shipped as source in this repository:
 | `espressif/cjson` (cJSON) | 1.7.19 | MIT |
 | `espressif/esp_lcd_sh8601` | 2.0.1 | Apache-2.0 |
 | `espressif/cmake_utilities` | 0.5.3 | Apache-2.0 |
+| [`espressif/esp-idf`](https://github.com/espressif/esp-idf) — the ESP-IDF framework itself, Espressif Systems | v6.0 | Apache-2.0 |
 
 ## Image writer — `stb_image_write.h`
 
@@ -140,3 +141,380 @@ transparency, not shipped as source in this repository:
   anchors are kept as valid roots for other well-known chains rather than
   pruned just because their original reason for inclusion no longer applies;
   see the file's own header comment for the full per-cert rationale.
+
+## Full license texts
+
+The sections above identify each component's license by name and link. The
+complete texts are reproduced here so that this file is self-contained.
+
+### Apache License 2.0
+
+Applies to: the rotary knob decoder (`bidi_switch_knob.c`/`.h`, © 2016-2024
+Espressif Systems (Shanghai) CO LTD), ESP-IDF, `espressif/esp_lcd_sh8601`,
+and `espressif/cmake_utilities`.
+
+```
+
+                                 Apache License
+                           Version 2.0, January 2004
+                        http://www.apache.org/licenses/
+
+   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
+
+   1. Definitions.
+
+      "License" shall mean the terms and conditions for use, reproduction,
+      and distribution as defined by Sections 1 through 9 of this document.
+
+      "Licensor" shall mean the copyright owner or entity authorized by
+      the copyright owner that is granting the License.
+
+      "Legal Entity" shall mean the union of the acting entity and all
+      other entities that control, are controlled by, or are under common
+      control with that entity. For the purposes of this definition,
+      "control" means (i) the power, direct or indirect, to cause the
+      direction or management of such entity, whether by contract or
+      otherwise, or (ii) ownership of fifty percent (50%) or more of the
+      outstanding shares, or (iii) beneficial ownership of such entity.
+
+      "You" (or "Your") shall mean an individual or Legal Entity
+      exercising permissions granted by this License.
+
+      "Source" form shall mean the preferred form for making modifications,
+      including but not limited to software source code, documentation
+      source, and configuration files.
+
+      "Object" form shall mean any form resulting from mechanical
+      transformation or translation of a Source form, including but
+      not limited to compiled object code, generated documentation,
+      and conversions to other media types.
+
+      "Work" shall mean the work of authorship, whether in Source or
+      Object form, made available under the License, as indicated by a
+      copyright notice that is included in or attached to the work
+      (an example is provided in the Appendix below).
+
+      "Derivative Works" shall mean any work, whether in Source or Object
+      form, that is based on (or derived from) the Work and for which the
+      editorial revisions, annotations, elaborations, or other modifications
+      represent, as a whole, an original work of authorship. For the purposes
+      of this License, Derivative Works shall not include works that remain
+      separable from, or merely link (or bind by name) to the interfaces of,
+      the Work and Derivative Works thereof.
+
+      "Contribution" shall mean any work of authorship, including
+      the original version of the Work and any modifications or additions
+      to that Work or Derivative Works thereof, that is intentionally
+      submitted to Licensor for inclusion in the Work by the copyright owner
+      or by an individual or Legal Entity authorized to submit on behalf of
+      the copyright owner. For the purposes of this definition, "submitted"
+      means any form of electronic, verbal, or written communication sent
+      to the Licensor or its representatives, including but not limited to
+      communication on electronic mailing lists, source code control systems,
+      and issue tracking systems that are managed by, or on behalf of, the
+      Licensor for the purpose of discussing and improving the Work, but
+      excluding communication that is conspicuously marked or otherwise
+      designated in writing by the copyright owner as "Not a Contribution."
+
+      "Contributor" shall mean Licensor and any individual or Legal Entity
+      on behalf of whom a Contribution has been received by Licensor and
+      subsequently incorporated within the Work.
+
+   2. Grant of Copyright License. Subject to the terms and conditions of
+      this License, each Contributor hereby grants to You a perpetual,
+      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
+      copyright license to reproduce, prepare Derivative Works of,
+      publicly display, publicly perform, sublicense, and distribute the
+      Work and such Derivative Works in Source or Object form.
+
+   3. Grant of Patent License. Subject to the terms and conditions of
+      this License, each Contributor hereby grants to You a perpetual,
+      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
+      (except as stated in this section) patent license to make, have made,
+      use, offer to sell, sell, import, and otherwise transfer the Work,
+      where such license applies only to those patent claims licensable
+      by such Contributor that are necessarily infringed by their
+      Contribution(s) alone or by combination of their Contribution(s)
+      with the Work to which such Contribution(s) was submitted. If You
+      institute patent litigation against any entity (including a
+      cross-claim or counterclaim in a lawsuit) alleging that the Work
+      or a Contribution incorporated within the Work constitutes direct
+      or contributory patent infringement, then any patent licenses
+      granted to You under this License for that Work shall terminate
+      as of the date such litigation is filed.
+
+   4. Redistribution. You may reproduce and distribute copies of the
+      Work or Derivative Works thereof in any medium, with or without
+      modifications, and in Source or Object form, provided that You
+      meet the following conditions:
+
+      (a) You must give any other recipients of the Work or
+          Derivative Works a copy of this License; and
+
+      (b) You must cause any modified files to carry prominent notices
+          stating that You changed the files; and
+
+      (c) You must retain, in the Source form of any Derivative Works
+          that You distribute, all copyright, patent, trademark, and
+          attribution notices from the Source form of the Work,
+          excluding those notices that do not pertain to any part of
+          the Derivative Works; and
+
+      (d) If the Work includes a "NOTICE" text file as part of its
+          distribution, then any Derivative Works that You distribute must
+          include a readable copy of the attribution notices contained
+          within such NOTICE file, excluding those notices that do not
+          pertain to any part of the Derivative Works, in at least one
+          of the following places: within a NOTICE text file distributed
+          as part of the Derivative Works; within the Source form or
+          documentation, if provided along with the Derivative Works; or,
+          within a display generated by the Derivative Works, if and
+          wherever such third-party notices normally appear. The contents
+          of the NOTICE file are for informational purposes only and
+          do not modify the License. You may add Your own attribution
+          notices within Derivative Works that You distribute, alongside
+          or as an addendum to the NOTICE text from the Work, provided
+          that such additional attribution notices cannot be construed
+          as modifying the License.
+
+      You may add Your own copyright statement to Your modifications and
+      may provide additional or different license terms and conditions
+      for use, reproduction, or distribution of Your modifications, or
+      for any such Derivative Works as a whole, provided Your use,
+      reproduction, and distribution of the Work otherwise complies with
+      the conditions stated in this License.
+
+   5. Submission of Contributions. Unless You explicitly state otherwise,
+      any Contribution intentionally submitted for inclusion in the Work
+      by You to the Licensor shall be under the terms and conditions of
+      this License, without any additional terms or conditions.
+      Notwithstanding the above, nothing herein shall supersede or modify
+      the terms of any separate license agreement you may have executed
+      with Licensor regarding such Contributions.
+
+   6. Trademarks. This License does not grant permission to use the trade
+      names, trademarks, service marks, or product names of the Licensor,
+      except as required for reasonable and customary use in describing the
+      origin of the Work and reproducing the content of the NOTICE file.
+
+   7. Disclaimer of Warranty. Unless required by applicable law or
+      agreed to in writing, Licensor provides the Work (and each
+      Contributor provides its Contributions) on an "AS IS" BASIS,
+      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
+      implied, including, without limitation, any warranties or conditions
+      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
+      PARTICULAR PURPOSE. You are solely responsible for determining the
+      appropriateness of using or redistributing the Work and assume any
+      risks associated with Your exercise of permissions under this License.
+
+   8. Limitation of Liability. In no event and under no legal theory,
+      whether in tort (including negligence), contract, or otherwise,
+      unless required by applicable law (such as deliberate and grossly
+      negligent acts) or agreed to in writing, shall any Contributor be
+      liable to You for damages, including any direct, indirect, special,
+      incidental, or consequential damages of any character arising as a
+      result of this License or out of the use or inability to use the
+      Work (including but not limited to damages for loss of goodwill,
+      work stoppage, computer failure or malfunction, or any and all
+      other commercial damages or losses), even if such Contributor
+      has been advised of the possibility of such damages.
+
+   9. Accepting Warranty or Additional Liability. While redistributing
+      the Work or Derivative Works thereof, You may choose to offer,
+      and charge a fee for, acceptance of support, warranty, indemnity,
+      or other liability obligations and/or rights consistent with this
+      License. However, in accepting such obligations, You may act only
+      on Your own behalf and on Your sole responsibility, not on behalf
+      of any other Contributor, and only if You agree to indemnify,
+      defend, and hold each Contributor harmless for any liability
+      incurred by, or claims asserted against, such Contributor by reason
+      of your accepting any such warranty or additional liability.
+
+   END OF TERMS AND CONDITIONS
+
+   APPENDIX: How to apply the Apache License to your work.
+
+      To apply the Apache License to your work, attach the following
+      boilerplate notice, with the fields enclosed by brackets "[]"
+      replaced with your own identifying information. (Don't include
+      the brackets!)  The text should be enclosed in the appropriate
+      comment syntax for the file format. We also recommend that a
+      file or class name and description of purpose be included on the
+      same "printed page" as the copyright notice for easier
+      identification within third-party archives.
+
+   Copyright [yyyy] [name of copyright owner]
+
+   Licensed under the Apache License, Version 2.0 (the "License");
+   you may not use this file except in compliance with the License.
+   You may obtain a copy of the License at
+
+       http://www.apache.org/licenses/LICENSE-2.0
+
+   Unless required by applicable law or agreed to in writing, software
+   distributed under the License is distributed on an "AS IS" BASIS,
+   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
+   See the License for the specific language governing permissions and
+   limitations under the License.
+```
+
+### MIT License
+
+Applies to, with each project's copyright line as published in its own
+`LICENSE` file:
+
+- LVGL — Copyright (c) 2021 LVGL Kft
+- cJSON — Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
+- Bootstrap Icons — Copyright (c) 2019-2024 The Bootstrap Authors
+- `stb_image_write.h` — Copyright (c) 2017 Sean Barrett (MIT is
+  "Alternative A" of stb's dual license; the public-domain alternative is
+  reproduced after it)
+- `posix_tz_db` — the upstream `LICENSE` file is the MIT template with its
+  copyright placeholder left as `[year] [fullname]`; reproduced as published.
+
+```
+The MIT License (MIT)
+
+Copyright (c) 2019-2024 The Bootstrap Authors
+
+Permission is hereby granted, free of charge, to any person obtaining a copy
+of this software and associated documentation files (the "Software"), to deal
+in the Software without restriction, including without limitation the rights
+to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
+copies of the Software, and to permit persons to whom the Software is
+furnished to do so, subject to the following conditions:
+
+The above copyright notice and this permission notice shall be included in
+all copies or substantial portions of the Software.
+
+THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
+IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
+FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
+AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
+LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
+OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
+THE SOFTWARE.
+```
+
+stb's Alternative B, at the user's option instead of MIT:
+
+```
+ALTERNATIVE B - Public Domain (www.unlicense.org)
+This is free and unencumbered software released into the public domain.
+Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
+software, either in source code form or as a compiled binary, for any purpose,
+commercial or non-commercial, and by any means.
+In jurisdictions that recognize copyright laws, the author or authors of this
+software dedicate any and all copyright interest in the software to the public
+domain. We make this dedication for the benefit of the public at large and to
+the detriment of our heirs and successors. We intend this dedication to be an
+overt act of relinquishment in perpetuity of all present and future rights to
+this software under copyright law.
+THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
+IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
+FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
+AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
+ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
+WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+```
+
+### SIL Open Font License 1.1
+
+Applies to: Montserrat (the source typeface for `dial_font_num_88.c`).
+Reproduced from the Montserrat repository's `OFL.txt`, which carries the
+project's own copyright header followed by the license text.
+
+```
+Copyright 2024 The Montserrat.Git Project Authors (https://github.com/JulietaUla/Montserrat.git)
+
+This Font Software is licensed under the SIL Open Font License, Version 1.1.
+This license is copied below, and is also available with a FAQ at:
+https://openfontlicense.org
+
+
+-----------------------------------------------------------
+SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
+-----------------------------------------------------------
+
+PREAMBLE
+The goals of the Open Font License (OFL) are to stimulate worldwide
+development of collaborative font projects, to support the font creation
+efforts of academic and linguistic communities, and to provide a free and
+open framework in which fonts may be shared and improved in partnership
+with others.
+
+The OFL allows the licensed fonts to be used, studied, modified and
+redistributed freely as long as they are not sold by themselves. The
+fonts, including any derivative works, can be bundled, embedded, 
+redistributed and/or sold with any software provided that any reserved
+names are not used by derivative works. The fonts and derivatives,
+however, cannot be released under any other type of license. The
+requirement for fonts to remain under this license does not apply
+to any document created using the fonts or their derivatives.
+
+DEFINITIONS
+"Font Software" refers to the set of files released by the Copyright
+Holder(s) under this license and clearly marked as such. This may
+include source files, build scripts and documentation.
+
+"Reserved Font Name" refers to any names specified as such after the
+copyright statement(s).
+
+"Original Version" refers to the collection of Font Software components as
+distributed by the Copyright Holder(s).
+
+"Modified Version" refers to any derivative made by adding to, deleting,
+or substituting -- in part or in whole -- any of the components of the
+Original Version, by changing formats or by porting the Font Software to a
+new environment.
+
+"Author" refers to any designer, engineer, programmer, technical
+writer or other person who contributed to the Font Software.
+
+PERMISSION & CONDITIONS
+Permission is hereby granted, free of charge, to any person obtaining
+a copy of the Font Software, to use, study, copy, merge, embed, modify,
+redistribute, and sell modified and unmodified copies of the Font
+Software, subject to the following conditions:
+
+1) Neither the Font Software nor any of its individual components,
+in Original or Modified Versions, may be sold by itself.
+
+2) Original or Modified Versions of the Font Software may be bundled,
+redistributed and/or sold with any software, provided that each copy
+contains the above copyright notice and this license. These can be
+included either as stand-alone text files, human-readable headers or
+in the appropriate machine-readable metadata fields within text or
+binary files as long as those fields can be easily viewed by the user.
+
+3) No Modified Version of the Font Software may use the Reserved Font
+Name(s) unless explicit written permission is granted by the corresponding
+Copyright Holder. This restriction only applies to the primary font name as
+presented to the users.
+
+4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
+Software shall not be used to promote, endorse or advertise any
+Modified Version, except to acknowledge the contribution(s) of the
+Copyright Holder(s) and the Author(s) or with their explicit written
+permission.
+
+5) The Font Software, modified or unmodified, in part or in whole,
+must be distributed entirely under this license, and must not be
+distributed under any other license. The requirement for fonts to
+remain under this license does not apply to any document created
+using the Font Software.
+
+TERMINATION
+This license becomes null and void if any of the above conditions are
+not met.
+
+DISCLAIMER
+THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
+EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
+MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
+OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
+COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
+INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
+DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
+FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
+OTHER DEALINGS IN THE FONT SOFTWARE.```
```

## Step 4: "open" phrase

Present in the source README. Before (two lines in the source, wrapped):

```
statement about the license below: the firmware is open under the same
terms it always has been, this repo just isn't where the buildable source
happens to live. If you want the source, ask the maintainer.
```

After:

```
statement about the license below: the firmware is source-available
under the same PolyForm Noncommercial terms it always has been, this repo
just isn't where the buildable source happens to live. If you want the
source, ask the maintainer.
```

Remaining occurrences of "open" in the README after the change:

```
17:Open **[the flasher page](https://matthewclaude.github.io/somnus-dial-releases/)**
```

That is the verb "Open" in the Install instructions, not a license claim. Left as is. `THIRD_PARTY_LICENSES` contains "An openly licensed set was chosen deliberately" about Bootstrap Icons, which is MIT and genuinely open; step 4 scopes to the README and that sentence is accurate, so it was left alone.

Note: `web-flasher/RELEASES-README.md` in THIS repo still carries the old "open under the same terms" wording and the `.md` link. Not touched here (no commits in this repo per spec); the owner may want to sync it so the next copy doesn't reintroduce both.

## Step 5: greps over the whole clone (excluding .git)

```
$ grep -rn --exclude-dir=.git '192\.168\.' .
exit=1   (no matches)

$ grep -rnE --exclude-dir=.git '[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}' .
exit=1   (no matches)
```

Nothing anywhere, license texts included.

## Step 6: commit and push

Pre-commit status in the clone:

```
 M README.md
 M THIRD_PARTY_LICENSES
```

Commit SHA in the releases repo: **4097b6e983e0037a41572325aac893656c453150**

```
docs: Bedknob README, ESP-IDF attribution, full third-party license texts

 README.md            |  54 ++++----
 THIRD_PARTY_LICENSES | 378 +++++++++++++++++++++++++++++++++++++++++++++++++++
 2 files changed, 404 insertions(+), 28 deletions(-)
```

Raw push output:

```
To https://github.com/matthewclaude/somnus-dial-releases.git
   380a688..4097b6e  main -> main
```

Remote heads after the push (gh-pages unmoved, only main advanced):

```
a2472966b73e17d71ac82174067fa00669d13bad	refs/heads/gh-pages
4097b6e983e0037a41572325aac893656c453150	refs/heads/main
```

Release list after the push (unchanged, latest still somnus-v0.1.5):

```
somnus-v0.1.5	Latest	somnus-v0.1.5	2026-09-05T01:09:56Z
somnus-v0.1.5-beta.5	Pre-release	somnus-v0.1.5-beta.5	2026-09-05T00:14:40Z
somnus-v0.1.5-beta.4	Pre-release	somnus-v0.1.5-beta.4	2026-09-04T02:09:06Z
somnus-v0.1.5-beta.3	Pre-release	somnus-v0.1.5-beta.3	2026-09-04T00:55:47Z
somnus-v0.1.5-beta.2	Pre-release	somnus-v0.1.5-beta.2	2026-09-03T20:26:54Z
```

## Step 7: README via the API

```
$ gh api repos/matthewclaude/somnus-dial-releases/readme --jq .content | base64 -d | head -5
# somnus-dial-releases

Firmware releases and the browser flasher for **Bedknob for Somnus** — a bedside
dial that turns a Waveshare ESP32-S3 round touch-LCD knob into a standalone
temperature control for a Somnus Pad. Not affiliated with, endorsed by, or
```

First line is `# somnus-dial-releases`, followed by the Bedknob paragraph.

## Deviations from spec

1. **README link fixed:** the source README links to `THIRD_PARTY_LICENSES.md`, but the file in the releases repo is extensionless `THIRD_PARTY_LICENSES` (and must stay so for the flasher footer). A verbatim copy would have published a 404 link, so the committed README links to `THIRD_PARTY_LICENSES`. No new content was introduced; the old README on `main` had the same broken link.
2. **Line rewrap in the step-4 paragraph:** the replacement sentence is longer than the original, so the paragraph was rewrapped to keep the file's ~76-column style. Wording is exactly what the spec asked for plus the surrounding sentence unchanged.
3. **ESP-IDF row placement and version:** the only table in the file is titled "Build-time managed components (fetched by idf.py, not vendored)". ESP-IDF is the framework rather than a managed component, so the row's first cell says so. The version column says `v6.0`, read from `~/esp/esp-idf` (`git describe` -> `v6.0`), the toolchain used to build the shipped firmware. The spec named no version.
4. **Commit message carries two trailer lines** (Co-Authored-By, Claude-Session) after the spec's title, per the session's attribution rule. Title line verbatim.
5. **MIT text reproduced once, not five times.** The five MIT components share one canonical text; it is reproduced once (Bootstrap Icons' copy) with every holder's copyright line listed above it rather than pasting five near-identical blocks.

Everything else: none.

## Not verified

- That the GitHub-rendered README displays the `THIRD_PARTY_LICENSES` link and the appended fenced license blocks as intended. Only the raw content was checked via the API.
- That the flasher footer link (`.../blob/main/THIRD_PARTY_LICENSES`) still renders from gh-pages; gh-pages was not touched and the file name is unchanged, so no reason to expect otherwise, but it was not opened in a browser.
- The ESP-IDF version actually compiled into the 0.1.5 binaries was inferred from the local toolchain, not read out of the firmware image.
- The clone at `/tmp/somnus-dial-releases` was left in place.
