# SPEC — Bedknob identity palette

**Status: settled Sep 2 2026.** Authoritative for the mark, the boot splash, the README,
the flasher page and anything else carrying the name.

**Every color here already exists in `firmware/dial-idf/components/dial_ui/dial_palette.c`.
Nothing in this document is a new color.** The identity palette is a *selection* from the
firmware's token table, not a parallel brand system. If the two ever disagree,
`dial_palette.c` wins and this document is the thing that is wrong.

Assets: `firmware/dial-idf/docs/brand/`. Color rules: `firmware/dial-idf/docs/design-spec.md` §2.

---

## 1. The two hard rules

Both are inherited, not invented for the brand.

**Rule 1 — every value quantizes exactly to RGB565.** `R, B ≡ 0 mod 8` and `G ≡ 0 mod 4`.
The panel is 360×360 RGB565; a value that does not quantize exactly gets rounded somewhere
nobody chose.

**Rule 2 — in night mode, every value has blue channel ≤ `0x18` (24).** This is why the
identity has two variants rather than one. The day neutrals fail the cap outright.

Check any candidate before using it:

```sh
python3 - <<'EOF'
for h in ["C8A050", "888C88", "F0F0E8", "101418", "906818", "785018", "C87818", "100C08"]:
    r, g, b = int(h[0:2],16), int(h[2:4],16), int(h[4:6],16)
    ok565  = (r % 8 == 0) and (b % 8 == 0) and (g % 4 == 0)
    oknight = b <= 0x18
    print(f"#{h}  RGB565 {'PASS' if ok565 else 'FAIL'}   night(B<=0x18) {'PASS' if oknight else 'FAIL'}")
EOF
```

---

## 2. Day palette

Used everywhere except an on-device night-mode screen: README, flasher, social preview,
favicon, and the splash while `dial_palette_is_night()` is false.

| Element | Token | Hex | Why |
|---|---|---|---|
| Ball | `identity-home` | `#C8A050` | Brass. The token already means "your side" — a brass finial is not an addition to the system. |
| Stem + bead | `identity-home` | `#C8A050` | The waist is what separates a bedpost from a flagpole. Drawn from a photograph of a real one. |
| Collar + base step | `identity-home` | `#C8A050` | Flared bell, not a flat slab. The flare is the other half of reading as turned brass. |
| Post | `ink-secondary` | `#888C88` | Runs off the bottom edge, so it reads as a post that continues into a bed. |
| Pointer | `bg` | `#101418` | Knocked out, not painted — reads machined. The one thing a real finial does not have, and the only signal that this is a control. **Always points up.** |

Note the day mark uses **three** values, not four: there is no `ink-primary` element since the
index mark was removed with the detent ring (§6).

## 3. Night palette

Used **only** on-device, whenever `dial_palette_is_night()` is true. The device swaps token
tables wholesale at the night transition; anything on screen swaps with it.

| Element | Token | Hex |
|---|---|---|
| Ball | `identity` (night, both sides) | `#906818` |
| Stem + bead | `identity` | `#906818` |
| Collar + base step | `identity` | `#906818` |
| Post | `ink-secondary` | `#785018` |
| Pointer | `bg` | `#100C08` |

Brass survives the night cap at `#906818` (B = `0x18`), which is the reason the identity
works at all — it is the one distinctive color in the system that does not have to be
abandoned after dark.

---

## 4. What the identity must not use, and why

`accent-heat` `#E86018`, `accent-cool` `#3888C8`, `neutral-holding` `#587868`,
`neutral-standby` `#585858`, `warning` `#E82818` and `stale` `#C89838` are a **state
contract**. Per `design-spec.md` §2 each is carried in parallel by a shape channel, so hue
never works alone.

A logo has no state. Spending one of those colors on the identity weakens it in the place
it actually does work — the arc indicator, the state pill, the power ring, the presence
dots. Keep them out of the mark, the splash and the flasher chrome.

`identity-partner` `#98A0A8` (pewter) is likewise reserved: it means the *other* side of
the bed. The identity uses brass only.

---

## 5. Typography note

The device has one font family — the single 88px Montserrat face per `design-spec.md`, one
family everywhere, chosen specifically to avoid font-sourcing friction. **The wordmark's
web typography is not a device concern**; the splash uses the device font. Do not add a
font to the firmware for the name.

---

## 6. Provenance — read this before "correcting" anything

**The first published mark was a lightbulb.** It had a ring of ten detents radiating out
from the brass ball, which was reasoned about carefully — correct tokens, RGB565 clean, night
cap satisfied — and never once rendered before publishing. Gold ball plus radiating lines plus
a base reads as a bulb, or a sun, at every size. It was replaced by a mark drawn from a
photograph of a real brass bedpost finial: waisted stem, flared bell collar, stepped base,
post running off the edge. **Render a mark before shipping it.** Verifying the inputs is not
verifying the output.

An earlier draft still used **Bedknob for Mac's** coral/cyan/violet duotone
(`#FF6B54` / `#35C9DB` / `#8B7BE8`). That was wrong on four counts, and the mistake is
worth recording because it is easy to repeat:

1. Bedknob for Mac is a **separate Swift reimplementation**, explicitly a UI/UX preview
   and not the embedded code in a different shell. Its palette is not the firmware's.
2. None of those four values satisfy Rule 1. All fail RGB565 quantization.
3. `#35C9DB` has B = 219, violating Rule 2 by an order of magnitude — a cyan splash at 3am
   drives exactly the subpixels the night palette exists to keep dark.
4. It spent thermal-state colors on an identity (§4).

**When a Bedknob color is in doubt, read `dial_palette.c`, not the Swift.** The Swift is
easier to find and reads like a design system, which is precisely the trap.

As of 2026-09-02 the Swift preview no longer carries that palette — see §7.

---

## 7. Off-device surfaces — aligned 2026-09-02

Three codebases render Bedknob. All now derive from the same table.

**`web-flasher/index.html`** — was eight hand-picked values, none of them tokens, including
a `#e8590c` orange sitting four points away from `accent-heat` and a `#0b0b0c` background
five points from `bg`. That near-miss state is the one most likely to drift further,
because nothing looks wrong. Now:

| CSS var | Was | Now | Token |
|---|---|---|---|
| `--bg` | `#0b0b0c` | `#101418` | `bg` |
| `--bg-raised` | `#17171a` | `#181C20` | `surface` |
| `--border` | `#2a2a2e` | `#202830` | `track` |
| `--text` | `#f2efe9` | `#F0F0E8` | `ink-primary` |
| `--text-dim` | `#a8a29e` | `#888C88` | `ink-secondary` |
| `--accent` | `#e8590c` | `#C8A050` | `identity-home` |
| `--accent-text` | `#ff8a4c` | `#D8B868` | brass tint, §7.1 |
| button active | `#d14e0a` | `#A88038` | brass shade, §7.1 |
| `--accent-tint` | `rgba(232, 89, 12, 0.14)` | `rgba(200, 160, 80, 0.14)` | `accent` |

**Bedknob for Mac** (`Sources/BedknobMac/Theme.swift`) — state accents now
`accent-heat` / `accent-cool` / `neutral-holding`. Two further corrections beyond a hue
swap, both because the preview was showing behavior the device does not have:

- `currentReadingAccent` was coral-always. The device's numeral is `ink-primary`
  unconditionally — *"the one big number is a fact, never recolored by mood"*
  (design-spec.md §1). Now `#F0F0E8`.
- The background duotone was coral↔cyan, i.e. thermal hues used decoratively. It is now
  brass↔deep-brass: brand, not state. The gradient itself stays — it is a Mac affordance,
  and the device's flat `bg` exists for redraw budget, which does not apply on a Mac.

`DialView.swift`'s `differentialColor()` routes through `DialTheme.accent(for:)` and needed
no change — worth preserving that indirection.

### 7.0 Tokens are ground-dependent. Do not copy ink across.

**A token is a color plus the background it was chosen against.** The device has exactly one
ground per palette — `bg` `#101418` by day, `#100C08` at night — so `ink-primary` `#F0F0E8`
is near-white and never has to be anything else. **Bedknob for Mac has a light mode**, whose
card is near-white. Copying `ink-primary` straight across rendered the WATER readout
white-on-cream: the text was still laid out and still correct, and completely invisible.
Caught only by looking at a screenshot of the running app.

The fix is to *swap the two tokens by ground* rather than pick a third color:

```swift
static let currentReadingAccent = Color(light: Color(hex: 0x101418),   // bg, as ink
                                        dark:  Color(hex: 0xF0F0E8))   // ink-primary
```

**Rule for any future port of a device token to a light surface:** accent and state colors
(`accent-heat`, `accent-cool`, `neutral-holding`) are mid-tones and survive both grounds
unchanged. **Ink and background tokens do not** — `ink-primary`, `ink-secondary`, `bg`,
`surface` and `track` are all defined against a dark ground, and on a light surface the ink
and background tokens exchange roles. Check every one against the surface it will actually
sit on, in both appearances.

This is the same error as the coral/cyan one, one layer down: a color moved between contexts
without checking what it would be seen against.

**Shape channel — checked in source, not assumed.** The status pill already carries it:
`ZoneKind.symbolName` returns `arrow.up` / `arrow.down` / `minus`, which map cleanly onto
the device's `LV_SYMBOL_UP` ▲ / `DOWN` ▼ / `MINUS` ▬, and the pill renders that glyph
alongside `ZoneKind.word` ("HEATING" / "COOLING" / "HOLDING") and the accent color. Three
redundant channels. Nothing to add.

Two genuine gaps remain, both narrower than "no shape channel":

- **`ZoneKind` models only heating / cooling / holding.** The device's grammar also has
  **standby** (○ `LV_SYMBOL_STOP`, indicator at 30% opacity) and **offline**
  (× `LV_SYMBOL_CLOSE`, numeral at 45%, pill reading "OFFLINE" in `warning`). The preview
  expresses power-off and staleness by other means and has no pill equivalent for either,
  so those two glyphs have no counterpart to compare against.
- **The differential readout carries no glyph.** It is *not* color-only — the signed number
  ("−1.2 °F") states direction unambiguously, which is arguably a stronger redundant
  channel than a glyph. But it does not follow the device's glyph grammar, and the firmware
  port must adopt one: `SPEC-differential-firmware-port.md` §5.

**Not verified:** no Swift toolchain is reachable from a cloud session, so `Theme.swift`
has not been compiled or run since the change. Colors only, no structural edits — but
"compiles clean" has not been established, let alone "looks right."

### 7.1 Derived tints — web and Mac only

Interaction states need tones the device table does not carry. These three are derived from
`identity-home`, satisfy Rule 1 so they could migrate on-device if ever needed, and are the
**only** values in this document that are not straight from `dial_palette.c`:

| Name | Hex | Use |
|---|---|---|
| brass tint | `#D8B868` | hover, link text |
| brass shade | `#A88038` | active/pressed, gradient end (light) |
| brass shadow | `#503810` | gradient end (dark) |

Do not add a fourth without adding it here.

---

## 8. Related

- `firmware/dial-idf/docs/design-spec.md` §2 — the source of both rules and both tables.
- `firmware/dial-idf/components/dial_ui/dial_palette.c` — the tokens themselves.
- `firmware/dial-idf/docs/brand/README.md` — which asset file to use at which size.
- `docs/SPEC-differential-firmware-port.md` §5 — the one-meaning-per-color rule, stated in
  firmware tokens.
