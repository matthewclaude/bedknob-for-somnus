# Support notice in the README and the web flasher page

**Verdict: DONE** (one deliberate deviation: "open an issue" is plain text and not a link. See *Issue link* and *Deviations*.)

Date: 2026-10-11. Docs only. No git write commands were run.

## Where the text lives (Step 1)

| Path | Role | Changed? |
|---|---|---|
| `README.md` | Main repo README | Yes |
| `web-flasher/index.html` | Flasher page source. `.github/workflows/release.yml`, job `deploy-pages`, step "Assemble Pages site" copies it (`cp web-flasher/index.html web-flasher/manifest.json web-flasher/manifest-beta.json _site/`), and step "Deploy to gh-pages (this repo)" publishes it | Yes |
| `web-flasher/RELEASES-README.md` | README of the archived `matthewclaude/somnus-dial-releases` repo | No (see below) |
| `web-flasher/README.md` | Developer notes for the flasher directory. No workflow copies it | No |

**Correction to the task's premise:** the flasher is **not** published to
`matthewclaude/somnus-dial-releases` gh-pages. `release.yml` publishes
`_site` to the gh-pages branch of **this** repo (`matthewclaude/bedknob-for-somnus`,
the `somnus` remote) through `peaceiris/actions-gh-pages@v4` and
`GITHUB_TOKEN`. The workflow header says: "matthewclaude/somnus-dial-releases
(archived 2026-09-13) is the frozen pre-1.0.1 location … Nothing in this
workflow writes to it."

No workflow copies a README into the releases repo. `web-flasher/RELEASES-README.md`
is that repo's archived README ("This repository is archived and no longer
publishes anything"). It was pushed there by hand before the archive, and an
archived repo is read-only, so editing the file here would change nothing that
is live. I left it alone.

## Issue link (Step 2)

```
$ gh repo view matthewclaude/somnus-dial-releases --json hasIssuesEnabled,visibility
{"hasIssuesEnabled":true,"visibility":"PUBLIC"}
```

Extra read-only check:

```
$ gh repo view matthewclaude/somnus-dial-releases --json isArchived,url
{"isArchived":true,"url":"https://github.com/matthewclaude/somnus-dial-releases"}
```

**No issue link was added.** Both of the task's conditions pass, but the repo is
archived. GitHub makes archived repos read-only, so nobody can open a new issue
there. A link to `…/somnus-dial-releases/issues` would invite bug reports that
can't be filed. "open an issue" stays plain text in both files. No repo
setting was changed.

For reference, the live project repo would accept issues:

```
$ gh repo view matthewclaude/bedknob-for-somnus --json hasIssuesEnabled,visibility,isArchived
{"hasIssuesEnabled":true,"isArchived":false,"visibility":"PUBLIC"}
```

If you want the link, the target would be
`https://github.com/matthewclaude/bedknob-for-somnus/issues`. That URL isn't
in the task, so I didn't add it.

## Files changed and exact text inserted (Step 3)

### `README.md`

Added as a plain paragraph right after the title and the one-line
not-affiliated blockquote, before "Turn a knob…". The line wrap matches the
file's ~78-column wrap:

```markdown
A note before you build: Bedknob is a free hobby project, shared as-is. It
isn't a retail product, and there's no setup or how-to support. Bug reports
are welcome: if something in the firmware is broken, open an issue with what
you did, what you expected, and what happened.
```

### `web-flasher/index.html`

Added between the `.pitch` paragraph and `<div class="install-card">`, which
puts it above the flash button. It reuses the existing `disclaimer` class, the
same dim, left-ruled style as the not-affiliated note at the top of the page.
No CSS, layout, scripts or manifest attributes changed.

```html
  <p class="disclaimer">
    A note before you build: Bedknob is a free hobby project, shared as-is.
    It isn't a retail product, and there's no setup or how-to support. Bug
    reports are welcome: if something in the firmware is broken, open an
    issue with what you did, what you expected, and what happened.
  </p>
```

Neither insertion contains a surname.

## Checks (Step 4)

**HTML well-formedness.** I ran a stack-based tag-balance pass over
`web-flasher/index.html` using Python's `html.parser`, with void elements
excluded. Result: `unclosed: [] mismatched: []`. The new block is one
self-contained `<p>…</p>` with no inline markup.

**When the flasher change goes live.** It does **not** go live on its own.
`release.yml` triggers only on `push: tags: ['somnus-v*']`. Pushing a commit to
`main` runs `ci.yml` (branches `main`) and `certs.yml` (all branches), and
neither one touches gh-pages. The new page is deployed the next time any
`somnus-v*` tag is pushed to `somnus`, **beta or stable**. The
"Assemble Pages site" step copies `index.html` on both channels, and only the
firmware directory (`firmware/beta` vs `firmware/latest`) depends on the
channel. `keep_files: true` overwrites `index.html` in place.

The README change shows on GitHub as soon as the commit is pushed to `somnus`.

## git diff --stat (Step 5)

```
 README.md              | 5 +++++
 docs/REPORTS.md        | 2 ++
 web-flasher/index.html | 7 +++++++
 3 files changed, 14 insertions(+)
```

The `docs/REPORTS.md` change was already uncommitted before this task started
(it appears in the opening status snapshot). This task did not touch it. This
report is untracked, so it doesn't appear in the stat.

**No commit SHA:** the task forbids committing, so none exists.

## Deviations

1. The issue link was not added even though `hasIssuesEnabled` is true and the
   repo is public, because `somnus-dial-releases` is archived and won't accept
   new issues. (Details in *Issue link*.)
2. The task's premise is wrong in two places. The flasher's gh-pages is in
   `bedknob-for-somnus`, not `somnus-dial-releases`, and no workflow copies a
   README into the releases repo. `web-flasher/RELEASES-README.md` exists but
   was left alone because its repo is archived.
3. Not added to `docs/REPORTS.md`, as the task instructed.

## Commands for Matthew

```
git add README.md web-flasher/index.html
git commit -m "docs: hobby-project / no-setup-support notice in README and flasher page"
git push somnus main
```

Optional, if you want the issue link: change "open an issue" to
`[open an issue](https://github.com/matthewclaude/bedknob-for-somnus/issues)`
in `README.md` and to
`<a href="https://github.com/matthewclaude/bedknob-for-somnus/issues">open an issue</a>`
in `web-flasher/index.html` before committing. The flasher page then goes live
with the next `somnus-v*` tag push.
