# Brand assets — Bedknob for Somnus

**Colors are specified in `docs/SPEC-brand-palette.md`. That document is authoritative;
this one only says which file to reach for.**

Name: **Bedknob for Somnus**. Short form **Bedknob** — use it anywhere space is tight,
including the boot splash and the Settings header, where the full name does not fit 360x360.
The "for" is deliberate: it marks the project as third-party rather than implying an
endorsement by Somnus Lab.

The mark is a turned bedpost finial that is also a control knob — ball, collar, post, ten
detents and an index mark at top.

## Which file

| File | Use |
|---|---|
| `bedknob-mark.svg` | Default. Day palette. 40px and above. |
| `bedknob-mark-night.svg` | On-device only, whenever `dial_palette_is_night()` is true. |
| `bedknob-mark-small.svg` | 32px and below — favicon, GitHub avatar, menu bar. Detents dropped. |

Between 32 and 40px either the full or the small variant works. Above 40px use the full mark.

## The short version of the color rules

Full reasoning, the day and night tables, and a runnable conformance check are in
`docs/SPEC-brand-palette.md`. In brief:

1. Every value quantizes exactly to RGB565 — R,B = 0 mod 8, G = 0 mod 4.
2. Night values cap blue at 0x18. This is why there are two variants, not one.
3. No thermal-state color in the mark — `accent-heat`, `accent-cool`, `neutral-holding`,
   `warning`, `stale` are a state contract, and a logo has no state.
4. Flat fills, no gradients. LVGL 8.4 on the ESP32-S3 has no compositor.
5. The indicator points up, always. Never rotate it for effect.
6. Carry "not affiliated with Somnus Lab" wherever the name is public.

## If a color is ever in doubt

Read `components/dial_ui/dial_palette.c`, **not** Bedknob for Mac's Swift. The Swift is a
separate reimplementation, is easier to find, and reads like a design system — which is
exactly the trap. An early draft of this mark used its coral/cyan/violet; the cyan alone
(#35C9DB, B=219) violated the night blue cap by an order of magnitude. See
`docs/SPEC-brand-palette.md` §6.
