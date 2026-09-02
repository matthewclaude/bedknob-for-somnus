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
| Somnus Dial / the dial | Bedknob for Somnus | Name settled; on-screen strings, README, flasher copy and repo name **not yet changed** |
| SomnusDialPreview | Bedknob for Mac | **Done Sep 2** — package, target, module, directories, app struct, Info.plist, build.sh, docs. Builds clean, 9/9 tests pass. GitHub repo still `matthewclaude/SomnusDialPreview`. |
| SomnusWidget | Bedknob Mini | **Not started** |

## What is still outstanding

1. **Firmware repo rename pass** — on-screen strings, root README, flasher page copy,
   `PROJECT_VER` metadata, and the repo names `somnus-waveshare-rotary-dial` and
   `somnus-dial-releases`. Do it **before 1.0.0**: the releases repo is public and the
   flasher is live, so this gets more expensive once people have flashed boards and
   bookmarked the page.
2. **Bedknob Mini** — the whole rename, same shape as the Mac app's.
3. **Two GitHub repo renames**, both manual and both the owner's to do.
4. **The Claude Project** — docs, instructions, description and title ("Bedknob family")
   all updated Sep 2 2026. Only the summary's *filename*
   (`somnus-dial-project-summary.md`) still carries the old name; fold that into the
   rename pass.

## Naming assets

`firmware/dial-idf/docs/brand/` — the mark, in day, night and small variants.
`docs/SPEC-brand-palette.md` — colors, authoritative.
