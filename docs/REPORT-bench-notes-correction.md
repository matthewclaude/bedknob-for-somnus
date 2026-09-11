# REPORT: bench-notes correction — "Fahrenheit-only" premise in REPORT-1.0.2-bench-items-2-3.md

**Verdict: DONE.** `docs/REPORT-1.0.2-bench-items-2-3.md` corrected in one commit, `f085388`: a dated correction note under the verdict line, six "Fahrenheit-only" passages and deviation 2's parenthetical reworded to "in Fahrenheit mode", deviation 5 extended with the off-grid 20.6 °C observation, and the bench-notes line rewritten for items 4–7. Measurements, log lines and the owner's quoted answers are byte-unchanged. No other file touched by that commit. This report supersedes the BLOCKED version written when the earlier task's gate expected 5 occurrences.

Date: 2026-09-11. Branch `main`, base `e5e920e`. Documentation only; no firmware, bench, pad, version, tag or push actions.

## 1. Gate — raw output of both checks

```
$ grep -c -i 'fahrenheit-only' docs/REPORT-1.0.2-bench-items-2-3.md
6
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

Both passed.

## 2. Before / after of every passage changed

Line numbers are before → after (the correction note adds two lines).

### 2.1 Verdict line (3 → 3)

Before:

```
**Verdict: PASS on both items, but the pad ends at 17.8 °C (64 °F), not 18.0 °C.** The Somnus app is Fahrenheit-only and has no value that maps to 18.0 °C; the owner chose 64 °F (17.8 °C) for the restore, the pad is off, and the last poll of the capture reads `on=0 set=17.8C`. Item 2: …
```

After:

```
**Verdict: PASS on both items, but the pad ends at 17.8 °C (64 °F), not 18.0 °C.** The Somnus app was in Fahrenheit mode for this run and no whole-°F value maps to 18.0 °C; the owner chose 64 °F (17.8 °C) for the restore rather than switch units (see the correction below), the pad is off, and the last poll of the capture reads `on=0 set=17.8C`. Item 2: …
```

The rest of the line (items 2 and 3, errors, two attempts) is unchanged.

### 2.2 New correction note (inserted as line 5, directly under the verdict line)

Before: (no such paragraph)

After:

```
> **Correction, 2026-09-11 (same day, after the run):** this report's conclusion that the Somnus app is "Fahrenheit-only" is wrong. The app supports Celsius and Relative as well as Fahrenheit; it was simply left in Fahrenheit for this run. The owner's "app stays in F" meant he was leaving the display unit alone, not that the app had no other unit. 18.0 °C was reachable directly in Celsius mode, or as relative level −9 (1.0 °C per level, level 0 = 27.0 °C). The pad's end state, off at 17.8 °C (64 °F), is therefore the owner's choice, not a device limit. The passages below that drew the wrong conclusion are reworded to say the app was in Fahrenheit mode during this run; the measurements, log lines and quoted answers are unchanged. Recorded in `docs/REPORT-bench-notes-correction.md`.
```

### 2.3 Section (g), first and last `side A:` lines (152 → 154)

Before:

```
The pad ends **off at 17.8 °C (64 °F)**. That is not 18.0 °C; it is the closest value the Fahrenheit-only Somnus app offers (64 °F = 17.78 °C; 65 °F = 18.33 °C) and the one the owner chose. The pad was on for …
```

After:

```
The pad ends **off at 17.8 °C (64 °F)**. That is not 18.0 °C; it is the closest whole-°F value with the Somnus app left in Fahrenheit mode (64 °F = 17.78 °C; 65 °F = 18.33 °C) and the one the owner chose; 18.0 °C itself was reachable in Celsius or as relative level −9 (see the correction at the top). The pad was on for …
```

### 2.4 Section (h), step-B attempt-1 bullet (157 → 159)

Before:

```
- Step B, attempt 1: "Something else", then in chat "its in F" (the app is Fahrenheit-only; 20.0 °C was not enterable). Asked which input woke the dial: "Single tap on the screen". Asked to turn the pad off from the app and whether the app could switch to Celsius: "Pad turned off; app stays in F". Then in chat: "lets start over and i'll make sure to pay attention :)".
```

After:

```
- Step B, attempt 1: "Something else", then in chat "its in F" (the app was in Fahrenheit mode; 20.0 °C was not directly enterable without switching units). Asked which input woke the dial: "Single tap on the screen". Asked to turn the pad off from the app and whether the app could switch to Celsius: "Pad turned off; app stays in F" — which meant he was leaving the display unit alone, not that Celsius was unavailable (see the correction at the top). Then in chat: "lets start over and i'll make sure to pay attention :)".
```

### 2.5 Section 4, Step E (166 → 168)

Before:

```
The last `side A:` line of the capture of record reads `on=0 set=17.8C`. **The pad is restored to off at 64 °F / 17.8 °C, the Fahrenheit-only app's nearest value to 18.0 °C, as the owner chose. It is not at 18.0 °C.** The owner was told in chat.
```

After:

```
The last `side A:` line of the capture of record reads `on=0 set=17.8C`. **The pad is restored to off at 64 °F / 17.8 °C, the nearest whole-°F value to 18.0 °C with the app left in Fahrenheit mode, as the owner chose. It is not at 18.0 °C; that is his choice, not a device limit.** The owner was told in chat.
```

### 2.6 Deviation 1 (170 → 172)

Before:

```
1. **Restore is 17.8 °C, not 18.0 °C.** The Somnus app is Fahrenheit-only; no whole-°F value is 18.0 °C. The owner chose 64 °F. The dial could have set exactly 18 °C but the task forbids dial writes. Stated in the verdict line.
```

After:

```
1. **Restore is 17.8 °C, not 18.0 °C.** The Somnus app was left in Fahrenheit mode, and no whole-°F value is 18.0 °C. The owner chose 64 °F rather than switch the app to Celsius (where 18.0 °C is directly enterable) or Relative (level −9); the end state is his choice, not a limit of the app. The dial could also have set exactly 18 °C but the task forbids dial writes. Stated in the verdict line.
```

### 2.7 Deviation 2, the parenthetical (171 → 173)

Before:

```
… and the app change never happened (Fahrenheit); the owner then turned the bed off on the dial …
```

After:

```
… and the app change never happened (the app was in Fahrenheit mode and 20.0 °C was not directly enterable without switching units); the owner then turned the bed off on the dial …
```

The rest of deviation 2, the two-attempt account, is unchanged.

### 2.8 Deviation 5, extended (174 → 176)

Before (whole item):

```
5. **68 °F arrived as 20.6 °C**, i.e. 69.1 °F, not the 20.0 °C the task named. Either the app sent 69 or the pad rounds; not investigated, outside this task. The owner read "68" on the face; the face shows whole °F and the dial's own °F rendering of 20.6 °C would be 69. The measurement (a change, seen within 3 s) does not depend on the value.
```

After (the original text kept, this appended):

```
… does not depend on the value. Sharper reading, added 2026-09-11: 20.6 °C is off the whole-degree Celsius grid and off the Relative grid (1.0 °C per level from 27.0 °C, so every level is a whole °C), while the dial's own writes are integer Celsius (`{"target_t":24}` in the attempt-1 log). The app in Fahrenheit mode can therefore set a target the dial cannot itself produce, and the dial then renders a setpoint that sits at a fractional relative level. Nothing is broken; it is a fact about the two clients. Recorded as an observation for a later look, explicitly not something this beta chases: the no-new-features rule is in force and this is not a 1.0.2 concern.
```

### 2.9 Bench-notes line (186 → 188)

Before:

```
Bench notes for those tasks, learned here: the Somnus app is Fahrenheit-only and sets a target only while the bed is on; wake the dial with one brief fingertip tap and no second contact; one action per message to the owner.
```

After:

```
Bench notes for those tasks, learned here: the Somnus app supports Fahrenheit, Celsius and Relative — set bench stimuli in Celsius or Relative so the target is exact and the log is unambiguous; the app was observed once not to send a target while the bed was off, noted here and not established as a rule; wake the dial with one brief fingertip tap and no second contact; one action per message to the owner.
```

## 3. Grep confirmations

No assertion of "Fahrenheit-only" survives:

```
$ grep -c -i 'fahrenheit-only' docs/REPORT-1.0.2-bench-items-2-3.md
1
$ grep -n -i 'fahrenheit-only' docs/REPORT-1.0.2-bench-items-2-3.md
5:> **Correction, 2026-09-11 (same day, after the run):** this report's conclusion that the Somnus app is "Fahrenheit-only" is wrong. …
```

The single remaining hit is the correction note quoting the phrase as the wrong conclusion, which item 1 of the task requires the note to state. Every other mention of Fahrenheit (lines 3, 154, 159, 168, 172, 173, 176, 188) now reads "in Fahrenheit mode" / "left in Fahrenheit mode" / "supports Fahrenheit, Celsius and Relative".

No other passage blames Fahrenheit for an inability:

```
$ grep -n -i 'not enterable\|cannot set\|could not set\|only offers\|no value that maps\|unavailable' docs/REPORT-1.0.2-bench-items-2-3.md
159:- Step B, attempt 1: … (the app was in Fahrenheit mode; 20.0 °C was not directly enterable without switching units). … not that Celsius was unavailable (see the correction at the top). …
```

Both hits on line 159 are the corrected wording ("not directly enterable without switching units"; "not that Celsius was unavailable").

The owner's quoted answers are byte-unchanged. Every double-quoted string in the file, before and after (excluding the two JSON log lines), was extracted with `grep -o '"[^"]*"'` and diffed:

```
$ diff q-before.txt q-after.txt
0a1,2
> "Fahrenheit-only"
> "app stays in F"
```

Two additions, both inside the new correction note; no removals and no changes. `"its in F"`, `"Pad turned off; app stays in F"`, `"64 F (17.8 C)"`, `"68 within ~3 s"`, `"3:43"`, `"Looked normal, not stale"` and the rest are exactly as committed in `cf733ce`.

## 4. `git diff --stat` and SHA

Commit `f085388` — `docs: REPORT-1.0.2-bench-items-2-3 — correct the "Fahrenheit-only" conclusion`

```
 docs/REPORT-1.0.2-bench-items-2-3.md | 18 ++++++++++--------
 1 file changed, 10 insertions(+), 8 deletions(-)
```

Nothing under `firmware/`, `.github/` or `web-flasher/` touched; `docs/SPEC-standby-poll.md` untouched. Not pushed.

## 5. Deviations

No deviations.

One thing seen and deliberately left, because the task says to change nothing else in the file: deviation 4 still contains the parenthetical "(the app only sets a target while on)", which states as a rule what the rewritten bench-notes line now records as a single observation. It is not a Fahrenheit claim and was outside the task's list. Worth a one-line reword in a later docs pass.

## 6. Could not be verified without the app or the bench

- That the app offers Celsius and Relative modes, and the Relative scale (1.0 °C per level, level 0 = 27.0 °C, 18.0 °C = level −9): owner's statement, taken as given.
- Whether 68 °F became 20.6 °C because the app sent 69 or because the pad rounds: still open, recorded as an observation and not a 1.0.2 concern.
- That the app does or does not send a target while the bed is off: observed once, now recorded as such.
