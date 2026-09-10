# REPORT: 1.0.1-beta.1 commit — OTA client polls bedknob-for-somnus; dual-publish releases

**DONE** — block 1 of 2: edits A–E applied, `idf.py build` clean, binary carries the three new URLs and no old repo name, committed. No tag, no push, no flash. Parent commit: `ba25b17`.

Date: 2026-09-10. Spec: `docs/SPEC-repo-consolidation.md`.

## Gate (all six passed)

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.0")
$ git tag -l 'somnus-v1.0.1*'
(no output)
$ git tag -l somnus-v1.0.0
somnus-v1.0.0
$ git --no-optional-locks status --short --untracked-files=no
(no output)
$ git rev-parse --abbrev-ref HEAD
main
$ git rev-parse --short HEAD
ba25b17
$ git remote get-url --push origin
no_push
$ test -f docs/SPEC-repo-consolidation.md && echo "spec present"
spec present
```

## Premise verification, before editing

### A — dial_ota.c

```
$ grep -n 'somnus-dial-releases' firmware/dial-idf/components/dial_ota/dial_ota.c
32:// publishing to somnus-dial-releases because every shipped dial resolves
37:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/latest"
45:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50"
49:    "https://api.github.com/repos/matthewclaude/somnus-dial-releases/releases/tags/%s"
303:    // matthewclaude/somnus-dial-releases repo, not a check failure. Report

$ sed -n '30,35p' firmware/dial-idf/components/dial_ota/dial_ota.c
// Points at the public binaries-only release repo (docs/SPEC-ota-readiness.md
// §5). The source repo is public too (since 2026-09-10), but releases keep
// publishing to somnus-dial-releases because every shipped dial resolves
// updates by exactly this URL; consolidating releases into the source repo
// is a separate, planned step (the OTA repoint), not a change this constant
// can make on its own.

$ sed -n '301,305p' firmware/dial-idf/components/dial_ota/dial_ota.c
    // GitHub 404s /releases/latest when the repo has zero published
    // releases -- expected right now for the freshly created
    // matthewclaude/somnus-dial-releases repo, not a check failure. Report
    // it exactly like "checked, nothing newer" rather than an error state,
    // and don't fall back to any other repo.
```

### B — dial_state.h

```
$ sed -n '353,361p' firmware/dial-idf/components/dial_state/dial_state.h
 * The repo line omits the "github.com/" host: at the error screen's sub
 * label (300px @ lv_font_montserrat_16) the full URL measures ~400px and
 * wraps awkwardly, while the bare "owner/repo" form fits on one line.
 */
#define DIAL_CERT_ERR_TITLE "Secure connection failed"
#define DIAL_CERT_ERR_MSG \
    DIAL_CERT_ERR_TITLE "\n" \
    "This firmware may be too old\n" \
    "matthewclaude/somnus-waveshare-rotary-dial"

$ printf '%s' 'matthewclaude/somnus-waveshare-rotary-dial' | wc -c
      42
$ printf '%s' 'matthewclaude/bedknob-for-somnus' | wc -c
      32
```

The new string is 32 characters against the old 42, so the width argument in the comment at lines 353–355 (bare `owner/repo` fits the 300 px label where the full URL does not) still holds with margin. The comment is untouched.

### C — CMakeLists.txt

```
$ sed -n '21p' firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.0")
```

### D — CHANGELOG.md and the extractor

```
$ sed -n '23,27p' CHANGELOG.md
"Beta builds" turned on.

## 1.0.0 — 2026-09-09

Bedknob for Somnus 1.0. This is the `0.1.6` build renumbered — no code

$ sed -n '60,64p' .github/workflows/release.yml
          BODY="$(awk -v ver="$VER" '
            $0 == "## " ver || index($0, "## " ver " ") == 1 { inside = 1; next }
            inside && /^## / { exit }
            inside { print }
          ' CHANGELOG.md)"
```

### E — release.yml

```
$ grep -n 'private' .github/workflows/release.yml
11:# private -- but the Release object and the gh-pages site are published into
14:# unauthenticated clients and GitHub 404s everything under a private repo to
73:          # THIS (private) repo, but the release/notes get published into
76:          # built from ${GITHUB_REPOSITORY} would point at a private repo

$ sed -n '27,28p' .github/workflows/release.yml
permissions:
  contents: write

$ sed -n '84p' .github/workflows/release.yml | cut -c1-120
            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/);

$ sed -n '126,135p' .github/workflows/release.yml
      - name: Publish GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          repository: matthewclaude/somnus-dial-releases
          token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
          prerelease: ${{ steps.channel.outputs.prerelease }}
          body: ${{ steps.notes.outputs.body }}
          files: |
            firmware/dial-idf/build/somnus-dial.bin
            firmware/dial-idf/build/somnus-dial-merged.bin

$ sed -n '170,184p' .github/workflows/release.yml
        uses: peaceiris/actions-gh-pages@v4
        with:
          # Same cross-repo PAT as the release-publish job above (see
          # top-of-file comment) -- external_repository + personal_token is
          # peaceiris/actions-gh-pages's documented way to push a gh-pages
          # branch into a different repo than the one the workflow runs in;
          # the default GITHUB_TOKEN can only ever push within this repo.
          personal_token: ${{ secrets.SOMNUS_RELEASES_TOKEN }}
          external_repository: matthewclaude/somnus-dial-releases
          publish_dir: ./_site
          publish_branch: gh-pages
          # NOT force_orphan: that wipes the branch on every deploy, which
          # would delete the OTHER channel's firmware image. keep_files
          # preserves firmware/latest/ when a beta publishes and vice versa.
          keep_files: true
```

Every line number and quotation in the block matched disk.

## After editing

```
$ grep -n 'somnus-dial-releases' firmware/dial-idf/components/dial_ota/dial_ota.c
33:// matthewclaude/somnus-dial-releases repo (docs/SPEC-ota-readiness.md §5),
35:// are dual-published to somnus-dial-releases as well, which is where a

$ grep -n 'private' .github/workflows/release.yml
(no output)

$ grep -n 'matthewclaude.github.io' .github/workflows/release.yml
86:            echo "For a first install, use the [browser flasher](https://matthewclaude.github.io/somnus-dial-releases/); ...
```

Both remaining `somnus-dial-releases` hits are inside the rewritten header comment above `GITHUB_API_URL` (the "before" sentence and the dual-publish sentence). No code line names the old repo. The flasher footer line moved from 84 to 86 (two lines added above it in the header comment) and is unchanged.

### Extractor output, `ver=1.0.1-beta.1`

```
$ awk -v ver="1.0.1-beta.1" '
    $0 == "## " ver || index($0, "## " ver " ") == 1 { inside = 1; next }
    inside && /^## / { exit }
    inside { print }
  ' CHANGELOG.md

The dial now checks for firmware updates at the project's own repository,
`matthewclaude/bedknob-for-somnus`, where the source code also lives.
Nothing else changes — same screens, same behaviour, same pad control.

A dial already on 1.0.0 with Beta builds turned on will be offered this
update as usual; after installing it, updates come from the new location
automatically. If you flash a dial from the browser flasher, it continues
to work exactly as before.

Internal: the three GitHub API URLs in `dial_ota.c` and the repository name
on the certificate-error screen updated; releases are published to both
repositories during the migration.

```

The new section only, non-empty; the workflow's trim step strips the leading and trailing blank lines.

### YAML parse

```
$ python -c "import yaml,sys; yaml.safe_load(open('.github/workflows/release.yml')); print('release.yml: parsed OK')"
release.yml: parsed OK

build-and-release ['actions/checkout@v4', 'Verify tag matches PROJECT_VER', 'Extract release notes from CHANGELOG.md', 'Classify release channel', 'Build firmware and merge flashable image (firmware/dial-idf)', 'Upload merged image for the Pages job', 'Publish GitHub Release', 'Publish GitHub Release (this repo)']
deploy-pages ['actions/checkout@v4', 'Download merged image', 'Channel directory', 'Assemble Pages site', 'Deploy to gh-pages', 'Deploy to gh-pages (this repo)']
```

(`python` here is the ESP-IDF environment's interpreter, which ships PyYAML; the system `python3` has no `yaml` module and nothing was installed.)

## Build

`get-idf; cd firmware/dial-idf; idf.py build`, exit 0. Last 40 lines, verbatim except that the home directory is written as `~`:

```
[ 98%] Building C object esp-idf/dial_ui/CMakeFiles/__idf_dial_ui.dir/scr_standby_face.c.obj
[100%] Linking C static library libdial_ui.a
[100%] Built target __idf_dial_ui
[100%] Building C object esp-idf/main/CMakeFiles/__idf_main.dir/main.c.obj
[100%] Linking C static library libmain.a
[100%] Built target __idf_main
[100%] Generating esp-idf/esp_system/ld/sections.ld
NOTE: ~/esp/esp-idf/components/bt/host/nimble/Kconfig.in:1420: 
BT_NIMBLE_MESH_PROVISIONER: 'default 0' is not a valid bool value (only 'y' and 
'n' are allowed). Value is treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:230: FATFS_PRINT_LLI: 
'default 0' is not a valid bool value (only 'y' and 'n' are allowed). Value is 
treated as 'n'.
NOTE: ~/esp/esp-idf/components/fatfs/Kconfig:235: 
FATFS_PRINT_FLOAT: 'default 0' is not a valid bool value (only 'y' and 'n' are 
allowed). Value is treated as 'n'.
[100%] Built target __ldgen_output_sections.ld
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated ~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x189930 bytes. Smallest app partition is 0x400000 bytes. 0x2766d0 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
or
 idf.py -p PORT flash
or
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x19000 build/ota_data_initial.bin 0x20000 build/somnus-dial.bin
or from the "~/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build" directory
 python -m esptool --chip esp32s3 -b 460800 --before default-reset --after hard-reset write-flash "@flash_args"
build exit 0
```

No errors, no warnings from project code; the three `NOTE:` lines are ESP-IDF's own Kconfig complaints about upstream components and appear on every build.

### Strings in the binary

```
$ strings build/somnus-dial.bin | grep -c 'repos/matthewclaude/bedknob-for-somnus'
3
$ strings build/somnus-dial.bin | grep -c 'somnus-dial-releases'
0
$ strings build/somnus-dial.bin | grep -c 'somnus-waveshare-rotary-dial'
0
$ strings build/somnus-dial.bin | grep -c '1.0.1-beta.1'
1
```

All four as expected (3, 0, 0, ≥1). The three hits are the full `/releases/latest`, `/tags?per_page=50` and `/releases/tags/%s` URLs. Not flashed.

## Diff

```
$ git diff --stat
 .github/workflows/release.yml                      | 61 +++++++++++++++++-----
 CHANGELOG.md                                       | 15 ++++++
 firmware/dial-idf/CMakeLists.txt                   |  2 +-
 firmware/dial-idf/components/dial_ota/dial_ota.c   | 28 +++++-----
 .../dial-idf/components/dial_state/dial_state.h    |  2 +-
 5 files changed, 79 insertions(+), 29 deletions(-)
```

Parent SHA: `ba25b17`. The new commit's SHA goes in block 2's report.

## Deviations

1. **Build tail redaction.** The block asked for the tail "unfiltered"; the public-repo rule forbids home paths. The tail is verbatim except that the home directory is written as `~` in the six lines that carried it. Nothing else was altered.
2. **Two header-comment hits for the old repo name, not one.** The block expected `grep -n 'somnus-dial-releases'` to show "only the header comment's dual-publish sentence, if any". The rewritten header names the old repo twice, both in that comment: once saying where shipped dials resolved updates before this build, once saying releases are dual-published there. Both are prose in the same comment block; no code line names it.
3. **`check_stable()` comment says "(non-prerelease)".** Added to the reworded 404 sentence because the beta's own Release is a prerelease and `/releases/latest` excludes prereleases, so the 404 lasts until the first *stable* Release — the spec's §3.1 wording. The block's wording ("until the first stable Release exists") is preserved.
4. **PyYAML not installed.** The block said to install it if missing; the ESP-IDF python environment already has it, so the parse ran there instead of adding a package to the system interpreter.

Spec vs block: no disagreements found. The second Release step and second Pages deploy match §4(a) and §4(b), including the old-repo-first ordering the spec asks for; the footer stays on the old flasher URL per §4(c); the four "private" sentences are reworded in place per §4(d); `web-flasher/` is untouched per §5.

## False premises

None. Every line number, every quoted string and both character counts matched disk before editing.

## Noticed, not touched

- **`DIAL_CERT_ERR_MSG` is never used by code.** Its only references outside `dial_state.h` are two comments in `scr_connecting.c` (lines 46 and 93). The string is not in the binary — `strings … | grep -c 'bedknob-for-somnus'` is 3, all three the API URLs — and the "expect 0" check for the old source-repo name would have passed on `1.0.0`'s binary too. Change B is a source-only correction; the width argument in its comment describes a label nothing currently draws. The header comment above it already says as much ("Kept, not pulled"). Left as is; it is what the spec asked for and it costs nothing.
- **`docs/SPEC-repo-consolidation.md` cites `release.yml` line numbers** (129, 178, 84, 27–28) that this edit shifts. The spec is dated 2026-09-10 and describes the pre-edit file; not updated.
- **The peaceiris comment at what is now lines 175–179** ("the default GITHUB_TOKEN can only ever push within this repo") is still true and is now the mechanism the second deploy relies on. Left as is.
- **`web-flasher/README.md` and `web-flasher/RELEASES-README.md`** were not opened; both are Phase 6 material under the spec's §5 and outside this block's CHANGES.
- **The 50-tag page cap** (spec §3.2(c)) is now one tag closer: 17 `somnus-v*` tags once this beta is tagged. Not this spec.

## Not verifiable without hardware

Everything in `docs/SPEC-repo-consolidation.md` §7, verbatim, for block 2 to check off:

> The beta tag itself is gated on the beta's own build (`idf.py build` clean; the simulator unaffected, since none of the four strings is rendered by it) and a bench pass of items 1–3 below against the *beta*. Tagging `1.0.1` **stable** repeats the gate against the stable build. Both passes use the one bench unit.
>
> 1. Bench dial on `1.0.0`, Beta builds **on** → Menu → Update → Check for updates → finds `1.0.1-beta.1` in the **old** repo (the serial log shows the tags request going to `api.github.com/repos/matthewclaude/somnus-dial-releases/tags?per_page=50` and the Release fetch for `somnus-v1.0.1-beta.1` returning 200) → installs → reboots showing `1.0.1-beta.1` under Menu → About.
> 2. Same dial → Check for updates → the serial log must show the request going to `api.github.com/repos/matthewclaude/bedknob-for-somnus/…` and the result `latest 1.0.1-beta.1, running 1.0.1-beta.1 -- up to date`.
> 3. **A serial capture of item 2 is required** and goes into the bring-up record. A silent "up to date" against the wrong host is indistinguishable from success on the screen — that was the failure shape of 2026-09-03 (`docs/REPORT-ota-beta-not-found.md`: eight `ota:` lines, every one "up to date", while the newer beta sat unseen on the server). Capture with the cat-based serial method from the bring-up notes, not `idf.py monitor`.
> 4. Not inside the ~6-minute tag-to-Release window of §3.2(b). Confirm with `gh run watch` that the release run for the tag has completed **and** that both Releases exist (`gh release view somnus-v1.0.1-beta.1 --repo <each>`) before touching the dial.
> 5. Rollback, if the repointed build cannot see the new repo (item 2 fails, or ends `OTA_FAILED`): wire-flash the `1.0.0` merged image from the **old** flasher page, which stays live throughout the beta and the cut-over. Five minutes, one unit. Then the beta is withdrawn (delete the two Releases; the tag stays, which is harmless per §3.2(a) once a newer tag with a Release exists) and this spec gets a §9 saying what was wrong.

The "build clean" half of the gate's preamble is satisfied by this report. Items 1–5 need the tag, the two Releases, and the bench unit.
