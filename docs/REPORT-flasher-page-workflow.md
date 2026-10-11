# Report: flasher-page workflow (page-only gh-pages redeploy)

**Verdict: DONE.** All three gate checks passed. `.github/workflows/flasher-page.yml` was created. It parses as valid YAML, and its guard step passed and failed in the right cases when run locally. Nothing was committed and the workflow has not been run.

## Gate output

1. `sed -n 21p firmware/dial-idf/CMakeLists.txt`
```
set(PROJECT_VER "1.0.4")
```
PASS.

2. `.github/workflows/release.yml` exists. In its `deploy-pages` job, the last step is:
```
      - name: Deploy to gh-pages (this repo)
        uses: peaceiris/actions-gh-pages@v4
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./_site
          publish_branch: gh-pages
          keep_files: true
```
PASS.

3. `git --no-optional-locks status --short --untracked-files=no` (run before any edit)
```
```
(no output, exit 0). PASS. The only untracked file before this task was `docs/REPORT-issue-link.md`, which this command does not list.

## New file: `.github/workflows/flasher-page.yml`

```yaml
# Redeploys the browser flasher PAGE only: web-flasher/index.html,
# web-flasher/manifest.json and web-flasher/manifest-beta.json, copied to the
# root of this repo's gh-pages branch. Run it by hand (workflow_dispatch) after
# a page-only change on main, so the page goes live without cutting a release.
#
# It never carries firmware. Firmware images reach gh-pages ONLY through
# release.yml on a somnus-v* tag (firmware/latest/ for stable, firmware/beta/
# for beta). The guard step below fails the run if _site holds anything under
# firmware/ or any file other than the three page files.
#
# keep_files: true is what preserves firmware/latest/ and firmware/beta/ on
# gh-pages across this deploy; it is the same setting release.yml uses. Never
# use force_orphan here: it rebuilds gh-pages from _site alone and would
# delete every firmware image the flasher serves.
name: flasher-page

on:
  workflow_dispatch:

permissions:
  contents: write

concurrency:
  group: gh-pages
  cancel-in-progress: false

jobs:
  deploy-page:
    if: github.ref == 'refs/heads/main'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Assemble Pages site (page files only)
        run: |
          mkdir -p _site
          cp web-flasher/index.html web-flasher/manifest.json web-flasher/manifest-beta.json _site/

      - name: Guard - page files only, nothing under firmware/
        run: |
          if [ -e _site/firmware ]; then
            echo "::error::_site contains firmware/; this workflow must never deploy firmware images"
            exit 1
          fi
          ACTUAL="$(cd _site && find . -mindepth 1 | sed 's|^\./||' | LC_ALL=C sort)"
          EXPECTED="$(printf '%s\n' index.html manifest-beta.json manifest.json)"
          if [ "$ACTUAL" != "$EXPECTED" ]; then
            echo "::error::_site must contain exactly index.html, manifest.json and manifest-beta.json"
            echo "Found:"
            printf '%s\n' "$ACTUAL"
            exit 1
          fi
          echo "_site contents OK:"
          printf '%s\n' "$ACTUAL"

      # Same settings as release.yml's deploy-pages step. keep_files leaves
      # firmware/latest/ and firmware/beta/ on gh-pages untouched.
      # (NOT force_orphan, which wipes the branch on every deploy.)
      - name: Deploy to gh-pages (this repo)
        uses: peaceiris/actions-gh-pages@v4
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./_site
          publish_branch: gh-pages
          keep_files: true
```

## Validation output

`actionlint` is not installed (`which actionlint` → `actionlint not found`), so I used the Python fallback.

The fallback command exactly as given, using the system `python3`:
```
$ python3 -c "import yaml,sys; yaml.safe_load(open(sys.argv[1]))" .github/workflows/flasher-page.yml
Traceback (most recent call last):
  File "<string>", line 1, in <module>
ModuleNotFoundError: No module named 'yaml'
exit=1
```

The same command run with a throwaway venv in the session scratch directory (PyYAML 6.0.3):
```
$ <scratch>/venv/bin/python3 -c "import yaml,sys; yaml.safe_load(open(sys.argv[1]))" .github/workflows/flasher-page.yml
exit=0
```

Parsed structure (also from the scratch venv). PyYAML follows YAML 1.1, so it reads the `on` key as boolean `True`. That is a PyYAML quirk; GitHub Actions reads the key as `on`:
```
triggers: {'workflow_dispatch': None}
perms: {'contents': 'write'}
concurrency: {'group': 'gh-pages', 'cancel-in-progress': False}
jobs: ['deploy-page']
if: github.ref == 'refs/heads/main' | runs-on: ubuntu-latest
deploy with: {'github_token': '${{ secrets.GITHUB_TOKEN }}', 'publish_dir': './_site', 'publish_branch': 'gh-pages', 'keep_files': True}
```

Local test of the guard step. I pulled its `run:` script out of the parsed YAML and ran it with `bash -e` against a scratch `_site`:
```
--clean
_site contents OK:
index.html
manifest-beta.json
manifest.json
exit=0
--firmware dir
::error::_site contains firmware/; this workflow must never deploy firmware images
exit=1
--extra file
::error::_site must contain exactly index.html, manifest.json and manifest-beta.json
Found:
extra
index.html
manifest-beta.json
manifest.json
exit=1
--missing file
::error::_site must contain exactly index.html, manifest.json and manifest-beta.json
Found:
index.html
manifest.json
exit=1
```

## git diff --stat

```
$ git --no-optional-locks diff --stat
```
(empty: the new workflow is untracked, so `git diff` does not show it.)

```
$ git --no-optional-locks status --short
?? .github/workflows/flasher-page.yml
?? docs/REPORT-issue-link.md
```
(This report, `docs/REPORT-flasher-page-workflow.md`, was written after these commands ran and is also untracked.)

## Deviations

- Validation: `actionlint` is not installed, and the system `python3` has no PyYAML, so the fallback command as given failed (raw output above). I re-ran the same one-liner with PyYAML installed in a temporary venv in the session scratch directory. No packages were installed into the system Python or the repo.
- Note, not a change: `release.yml` has no `concurrency:` block. So the `gh-pages` concurrency group only serialises runs of `flasher-page.yml` against each other. It does not serialise them against a tag-triggered release deploy. Both workflows use `keep_files: true`, so they write disjoint paths (page files at the root versus `firmware/<channel>/`). The worst case is that one push is rejected as non-fast-forward and needs a re-run. Per the rules, `release.yml` was not edited.

Otherwise none. No changes to `release.yml`, `ci.yml`, `certs.yml`, `CHANGELOG.md` or anything under `firmware/`. The workflow was not run and the gh-pages branch was not touched.

## Commit commands (for Matthew)

```
git add .github/workflows/flasher-page.yml
git commit -m "ci: manual flasher-page workflow - redeploy index.html + manifests to gh-pages without firmware"
git push somnus main
```

## Running it afterwards

GitHub only offers `workflow_dispatch` once the file is on the default branch, so run this after the push:
```
gh workflow run flasher-page.yml --repo matthewclaude/bedknob-for-somnus --ref main
```
