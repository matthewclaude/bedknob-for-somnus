# somnus-dial-releases

Firmware releases and the browser flasher for **Bedknob for Somnus** — a bedside
dial that turns a Waveshare ESP32-S3 round touch-LCD knob into a standalone
temperature control for a Somnus Pad. Not affiliated with, endorsed by, or
supported by Somnus Lab or Waveshare.

**This repo holds binaries, a manifest, and this flasher page only — it is
not the firmware's source.** The source lives in a private repository.
That's a deliberate choice about this project's development, not a
statement about the license below: the firmware is source-available
under the same PolyForm Noncommercial terms it always has been, this repo
just isn't where the buildable source happens to live. If you want the
source, ask the maintainer.

## Install

Open **[the flasher page](https://matthewclaude.github.io/somnus-dial-releases/)**
in Chrome or Edge on a desktop computer, plug the dial in over USB-C, and
click Install. No software to install locally — the page flashes the board
directly from your browser over Web Serial.

Already have a dial running this firmware? You don't need this page again —
updates arrive over the air (Menu → Update on the dial itself, or
automatically overnight if you've turned that on).

## What's in a release

Each [release](https://github.com/matthewclaude/somnus-dial-releases/releases)
carries two files:

- `somnus-dial.bin` — the app-only image. This is what a dial already
  running this firmware downloads and installs over the air; it must never
  be flashed directly at offset `0x0`.
- `somnus-dial-merged.bin` — the same firmware, bundled with the bootloader
  and partition table into one image flashable at offset `0x0` on a blank
  or already-flashed chip. This is what the browser flasher above uses, and
  what you'd use with `esptool` manually.

A release marked **pre-release** on GitHub is a beta build — visible only
to dials that have turned on "Beta builds" under Menu → Update, or to the
flasher page's beta checkbox.

## License and attribution

This firmware is a fork of
[chris023/orion-waveshare-rotary-dial](https://github.com/chris023/orion-waveshare-rotary-dial),
licensed under the **PolyForm Noncommercial License 1.0.0** — free for
personal, noncommercial use. See [`LICENSE`](LICENSE) for the full terms
and required notice, and [`THIRD_PARTY_LICENSES`](THIRD_PARTY_LICENSES)
for the hardware bring-up code, fonts, data, and libraries this project
builds on, each under its own terms.
