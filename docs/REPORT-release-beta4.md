# Release report — 0.1.5-beta.4

**Verdict: RELEASED.** Gate passed clean on the first run. Commit A (the
doubled-tag log fix) and commit B (the release) both built successfully and
landed as exactly the files specified. No tag, no push.

## Gate check (verbatim)

```
$ git log --oneline -1
8245021 power: battery glyph and 3 s plug-in glyph on dial face and standby (section 10, commit 2)

$ grep -n PROJECT_VER firmware/dial-idf/CMakeLists.txt
22:set(PROJECT_VER "0.1.5-beta.3")

$ git tag --list 'somnus-v*' | sort -V | tail -1
somnus-v0.1.5-beta.3

$ git --no-optional-locks status --short | grep -v '^??'
(nothing — grep exit 1)
```

All four checks passed. No stop needed.

## Commit A diff (log fix)

```diff
--- a/firmware/dial-idf/components/dial_power/dial_power.c
+++ b/firmware/dial-idf/components/dial_power/dial_power.c
@@ -159,7 +159,7 @@ static void pwr_sample_and_classify(void)
     // this series once the curve has been captured, per the task's own
     // instruction to keep it only "for the bench only and only in this
     // commit series".
-    ESP_LOGD(TAG, "power: sample %u mV", mv);
+    ESP_LOGD(TAG, "sample %u mV", mv);
 
     if (s_pwr_ring_n < PWR_BOOT_MIN_SAMPLES) return;   // still UNKNOWN (§10.2)
 
@@ -179,9 +179,9 @@ static void pwr_sample_and_classify(void)
     s_power_src = vote;
     dial_state_commit(mut_power_src, &vote);
     switch (vote) {
-    case PWR_PLUGGED: ESP_LOGI(TAG, "power: plugged (%u mV)", mv); break;
-    case PWR_BATTERY: ESP_LOGI(TAG, "power: battery (%u mV)", mv); break;
-    default:          ESP_LOGI(TAG, "power: unknown"); break;
+    case PWR_PLUGGED: ESP_LOGI(TAG, "plugged (%u mV)", mv); break;
+    case PWR_BATTERY: ESP_LOGI(TAG, "battery (%u mV)", mv); break;
+    default:          ESP_LOGI(TAG, "unknown"); break;
     }
 }
```

Exactly the four transition/sample log strings; nothing else touched. With
`TAG = "power"`, `ESP_LOGI(TAG, "plugged (...)")` now renders as
`I (...) power: plugged (4772 mV)` instead of the doubled
`power: power: plugged (...)`.

## Build — commit A (raw tail)

```
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x1886a0 bytes. Smallest app partition is 0x400000 bytes. 0x277960 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
```

No compiler warnings from the touched file.

## Build — commit B (raw tail)

```
[100%] Linking CXX executable somnus-dial.elf
[100%] Built target somnus-dial.elf
[100%] Generating binary image from built executable
esptool v5.3.1
Creating ESP32-S3 image...
Merged 2 ELF sections.
Successfully created ESP32-S3 image.
Generated /Users/matthew/Projects/somnus-waveshare-rotary-dial/firmware/dial-idf/build/somnus-dial.bin
[100%] Built target gen_somnus-dial_binary
[100%] Built target gen_project_binary
somnus-dial.bin binary size 0x1886a0 bytes. Smallest app partition is 0x400000 bytes. 0x277960 bytes (62%) free.
[100%] Built target app_check_size
[100%] Built target app

Project build complete. To flash, run:
 idf.py flash
```

Identical size to commit A's build — expected, since the CHANGELOG/version
string bump is the only change and doesn't alter code size meaningfully at
this granularity. No compiler warnings.

## Binary version-string check

```
$ strings firmware/dial-idf/build/somnus-dial.bin | grep -m1 '0\.1\.5-beta\.4'
0.1.5-beta.4
```

Confirmed.

## git diff --stat and commit SHAs

**Commit A** — `6f76af1` "power: drop doubled tag in transition log lines"

```
 firmware/dial-idf/components/dial_power/dial_power.c | 8 ++++----
 1 file changed, 4 insertions(+), 4 deletions(-)
```

**Commit B** — `179a351` "release: 0.1.5-beta.4"

```
 CHANGELOG.md                     | 12 ++++++++++++
 firmware/dial-idf/CMakeLists.txt |  2 +-
 2 files changed, 13 insertions(+), 1 deletion(-)
```

Commit B touches exactly the two authorized files. No tag created. No push
performed. Gate re-verified clean (no unstaged tracked changes) after both
commits.

## Deviations

None.

## What could not be verified without hardware

- **The OTA pull itself** for this release — a device on the beta channel
  actually discovering, downloading, and applying 0.1.5-beta.4 — was not
  exercised in this pass; that requires the tag/push (owner's step) and a
  real device checking for updates afterward.
