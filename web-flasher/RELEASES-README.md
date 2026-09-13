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
