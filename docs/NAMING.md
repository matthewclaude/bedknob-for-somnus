# Naming — the Bedknob family

**Settled Sep 2 2026. Authoritative.** Where any other document still uses an older name,
this one is right and that one is stale.

## The names

| Thing | Name | Short form | Identifier |
|---|---|---|---|
| The physical rotary dial + firmware | **Bedknob for Somnus** | Bedknob | — |
| The interactive Mac app | **Bedknob for Mac** | — | `BedknobMac` |
| The menu bar reader | **Bedknob Mini** | — | `BedknobMini` |

**Why "for Somnus" and not "Somnus Bedknob".** The preposition marks the project as
third-party rather than implying an endorsement by Somnus Lab, whose trademark it is.
Anything public-facing also carries *"Not affiliated with Somnus Lab."*

**Short form on the device.** The 360×360 display cannot fit the full name. The boot splash
and the Settings header use **Bedknob** alone.

**"for Mac" vs "for Somnus" mean different things** — platform in one, pad brand in the
other. Accepted deliberately; "Bedknob for Mac" reads naturally enough that the parallel
construction is not worth losing.

## Old names → new

| Was | Now | Status |
|---|---|---|
| Somnus Dial / the dial | Bedknob for Somnus | **Copy surfaces done Sep 3** — root README, firmware READMEs, ARCHITECTURE.md, flasher page (title, H1, description, button, manifests' `name` field), SoftAP name `Bedknob-XXXX`, portal page title and heading. GitHub repo still `matthewclaude/somnus-waveshare-rotary-dial` (default branch `main` = the ported code since Sep 5); binary, tag and URL identifiers stay by design (see below). |
| SomnusDialPreview | Bedknob for Mac | **Done Sep 2** — package, target, module, directories, app struct, Info.plist, build.sh, docs. Builds clean, 9/9 tests pass. GitHub repo still `matthewclaude/SomnusDialPreview`. |
| SomnusWidget | Bedknob Mini | **Done Sep 5** — target/module/executable `BedknobMini`, bundle `Bedknob Mini.app`, id `com.matthew.bedknobmini`, hardcoded home IP scrubbed to the spec's `192.168.1.100`; rename only, no behavior change. Put under git the same day and pushed to a new private repo `matthewclaude/BedknobMini`. Local folder still `~/Projects/SomnusWidget` on purpose (paths in specs). |

## What is still outstanding

1. **Firmware repo rename pass** — **done Sep 3 2026** for every copy surface: root
   README, firmware READMEs, ARCHITECTURE.md, the flasher page (title, H1, description,
   button, the manifests' `name` field), the SoftAP name (`Bedknob-XXXX`, **seen on the air Sep 3 evening** after a
   factory-blank provision), and the portal page's title and heading. **Deliberately not renamed, and staying that way:**
   the binary names `somnus-dial.bin` / `somnus-dial-merged.bin`, the tag prefix
   `somnus-v`, the CMake project name, and the releases repo URL — shipped dials resolve
   updates by exactly these identifiers, so changing any of them would strand every
   flashed board.
2. ~~**Bedknob Mini** — the whole rename.~~ **Done Sep 5 2026** (see table).
3. **Two GitHub repo renames** (`somnus-waveshare-rotary-dial`, `SomnusDialPreview`),
   both manual and both the owner's to do. `somnus-dial-releases` stays: its URL is one
   of the identifiers above.
4. **The Claude Project** — docs, instructions, description and title ("Bedknob family")
   all updated Sep 2 2026. Only the summary's *filename*
   (`somnus-dial-project-summary.md`) still carries the old name; fold that into the
   rename pass.
5. **Sep 5 2026 audit of what is actually published — and the fixes.** Found: the
   firmware repo's GitHub default branch `main` still pointed at the un-ported Orion
   code (`4a32427`), which is why the GitHub repo page showed the Orion README while
   the ported firmware sat on `firmware/somnus-port`. **Fixed Sep 5:** the work branch
   was renamed to `main` and force-pushed over the old one (`4a32427..5228de3`);
   `firmware/somnus-port` is deleted on the remote. There is one branch now. The Mac
   repo's rename commit `d762ba6` ("Rename to Bedknob for Mac …") had never been
   pushed, so GitHub still showed SomnusDialPreview — pushed the same day.
   `~/Projects/SomnusWidget` is not a git repository at all (Bedknob Mini, item 2).
   Check going forward: `git log <remote>/main..HEAD` empty and
   `git --no-optional-locks status --short` empty means GitHub matches disk.

## Naming assets

`firmware/dial-idf/docs/brand/` — the mark, in day, night and small variants.
`docs/SPEC-brand-palette.md` — colors, authoritative.
