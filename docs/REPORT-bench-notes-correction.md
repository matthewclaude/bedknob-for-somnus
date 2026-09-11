# REPORT: bench-notes correction — Fahrenheit-only premise in REPORT-1.0.2-bench-items-2-3.md

**Verdict: BLOCKED by gate check 1 — `grep -c -i 'fahrenheit-only'` prints 6, the task requires exactly 5.** Nothing was changed. To unblock: the gate must accept 6, or the task must say which of the six passages is not in scope. The sixth occurrence is the final "Bench notes for those tasks, learned here" line (line 186), which the task's own item 3 targets for rewriting, so the count on disk is consistent with the task's edit list and the expected value of 5 appears to be off by one, not the file.

Date: 2026-09-11. Branch `main` at `cf733ce`. Documentation only; no firmware, bench, pad, version, tag or push actions taken or intended.

## 1. Gate — raw output of both checks

```
$ grep -c -i 'fahrenheit-only' docs/REPORT-1.0.2-bench-items-2-3.md
6
$ git --no-optional-locks status --short --untracked-files=no
(no output)
```

Check 1 failed (6 ≠ 5). Check 2 passed.

## 2. What was found

The six matching lines, with their line numbers on `cf733ce`, quoted so the next task can set its gate from them:

```
3:   **Verdict: PASS on both items, but the pad ends at 17.8 °C (64 °F), not 18.0 °C.** The Somnus app is Fahrenheit-only and has no value that maps to 18.0 °C; ...
152: The pad ends **off at 17.8 °C (64 °F)**. That is not 18.0 °C; it is the closest value the Fahrenheit-only Somnus app offers ...
157: - Step B, attempt 1: "Something else", then in chat "its in F" (the app is Fahrenheit-only; 20.0 °C was not enterable). ...
166: ... **The pad is restored to off at 64 °F / 17.8 °C, the Fahrenheit-only app's nearest value to 18.0 °C, as the owner chose. It is not at 18.0 °C.** ...
170: 1. **Restore is 17.8 °C, not 18.0 °C.** The Somnus app is Fahrenheit-only; no whole-°F value is 18.0 °C. ...
186: Bench notes for those tasks, learned here: the Somnus app is Fahrenheit-only and sets a target only while the bed is on; ...
```

Mapped to the task's edit list: line 3 is the verdict line, 152 is section (g), 157 is the step-B attempt-1 bullet, 166 is section 4 Step E, 170 is deviation 1 — the five places item 2 names — and 186 is the bench-notes line item 3 names. One further line asserts the same premise without the hyphenated phrase and would need the same correction: deviation 2 (line 171) says "the app change never happened (Fahrenheit)". Deviation 5 (the 20.6 °C reading) does not contain the phrase and is the passage item 4 extends.

The owner's quoted answers that must stay byte-unchanged were located and are untouched: `"its in F"` and `"Pad turned off; app stays in F"` on line 157, `"64 F (17.8 C)"` on line 159.

## 3. Before / after

None. No passage was changed.

## 4. Grep confirmation

Not applicable: no edit was made, so the six assertions still stand and the quoted answers are unchanged because the file is unchanged (`git --no-optional-locks status` clean before this report was written).

## 5. `git diff --stat` and SHA

No correction commit. `docs/REPORT-1.0.2-bench-items-2-3.md` is unchanged. The only commit from this task is the one carrying this report and its `docs/REPORTS.md` line; its SHA is given in chat.

## 6. Deviations

No deviations. The gate blocked and the task's stop rule was followed.

## 7. Could not be verified without the app or the bench

Nothing was attempted, so nothing was verified. Carried forward for the re-run: the app's Celsius and Relative modes, the relative scale (1.0 °C per level, level 0 = 27.0 °C, 18.0 °C = level −9), and whether 68 °F → 20.6 °C is the app sending 69 or the pad rounding are all owner statements or open questions that only the app or the bench can confirm.
