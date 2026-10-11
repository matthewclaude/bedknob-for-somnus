# REPORT: "open an issue" link in the README and the flasher page

**Verdict: DONE** — gate passed; "open an issue" is now a link to `https://github.com/matthewclaude/bedknob-for-somnus/issues` in `README.md` and `web-flasher/index.html`; the missing `REPORT-docs-touchup-1.0.4.md` line was added to `docs/REPORTS.md`; nothing staged, committed, tagged or pushed.

## Gate

```
$ sed -n 21p firmware/dial-idf/CMakeLists.txt
set(PROJECT_VER "1.0.4")

$ git merge-base --is-ancestor 1aa7f4c HEAD; echo "exit=$?"
exit=0

$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

All three checks passed.

The link target matches the `somnus` remote (`git@github.com:matthewclaude/bedknob-for-somnus.git`).

## Edit A — README.md (hobby-project note, lines 7–11)

Before:

```
A note before you build: Bedknob is a free hobby project, shared as-is. It
isn't a retail product, and there's no setup or how-to support. Bug reports
are welcome: if something in the firmware is broken, open an issue with what
you did, what you expected, and what happened.
```

After:

```
A note before you build: Bedknob is a free hobby project, shared as-is. It
isn't a retail product, and there's no setup or how-to support. Bug reports
are welcome: if something in the firmware is broken,
[open an issue](https://github.com/matthewclaude/bedknob-for-somnus/issues)
with what you did, what you expected, and what happened.
```

The wording is unchanged. The lines were rewrapped so the link sits on its own line.

## Edit B — web-flasher/index.html (second `<p class="disclaimer">`, lines 198–204)

Before:

```html
  <p class="disclaimer">
    A note before you build: Bedknob is a free hobby project, shared as-is.
    It isn't a retail product, and there's no setup or how-to support. Bug
    reports are welcome: if something in the firmware is broken, open an
    issue with what you did, what you expected, and what happened.
  </p>
```

After:

```html
  <p class="disclaimer">
    A note before you build: Bedknob is a free hobby project, shared as-is.
    It isn't a retail product, and there's no setup or how-to support. Bug
    reports are welcome: if something in the firmware is broken,
    <a href="https://github.com/matthewclaude/bedknob-for-somnus/issues">open an issue</a>
    with what you did, what you expected, and what happened.
  </p>
```

The wording is unchanged. The phrase that was split across the line break is now on one line inside the `<a>`.

### CSS check (no CSS changed)

**Yes, a link inside `.disclaimer` stands out from the text around it.**

- `.disclaimer` (line 61) sets `color: var(--text-dim)` = `#888C88` (grey).
- The global rule `a { color: var(--accent-text); }` (line 75) sets `#D8B868` (gold) on the `<a>` itself. A rule that targets the element wins over a colour it would inherit from `.disclaimer`, so the link shows gold, not grey.
- No rule removes the underline at rest, so the browser's default underline stays. `a:hover { text-decoration: none; }` (line 76) removes it on hover only.
- No `.disclaimer a` rule and no `prefers-color-scheme` block exist, so nothing overrides this.

Result: a gold, underlined link inside grey text.

## Edit C — docs/REPORTS.md

Before (end of the list):

```
- `REPORT-docs-sync-1.0.4.md` — 2026-10-11 — REPORT: docs sync to somnus-v1.0.4 — **DONE** — … five version-bearing screens regenerated at 1.0.4; nothing committed.

Regenerate this index when adding reports; it is hand-maintained, not built.
```

After:

```
- `REPORT-docs-sync-1.0.4.md` — 2026-10-11 — REPORT: docs sync to somnus-v1.0.4 — **DONE** — … five version-bearing screens regenerated at 1.0.4; nothing committed.
- `REPORT-docs-touchup-1.0.4.md` — 2026-10-11 — REPORT: docs touch-up after somnus-v1.0.4 — **DONE** — owner-confirmed charger note added to `SPEC-power-sensing.md` §9.3 and the `REPORT-docs-sync-1.0.4.md` index line corrected; committed as `1aa7f4c`.

Regenerate this index when adding reports; it is hand-maintained, not built.
```

The title in the new line is the report's H1. As instructed, this report has no line of its own.

## git diff --stat

```
 README.md              | 5 +++--
 docs/REPORTS.md        | 1 +
 web-flasher/index.html | 5 +++--
 3 files changed, 7 insertions(+), 4 deletions(-)
```

(`docs/REPORT-docs-touchup-1.0.4.md` and this report are untracked, so the stat doesn't list them.)

## Deviations

none

## Commit commands (for Matthew)

```
git add README.md web-flasher/index.html docs/REPORTS.md docs/REPORT-docs-touchup-1.0.4.md
git commit -m "docs: link 'open an issue' in README and flasher page; REPORTS.md docs-touchup-1.0.4 line"
```

This report (`docs/REPORT-issue-link.md`) is not committed. No tag or push is needed for a docs-only change. If you want the change on `somnus` now, run `git push somnus main`.
