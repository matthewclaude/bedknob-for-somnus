# SPEC — Voice: spoken confirmation and voice commands

**Status: analysis / proposal, Sep 3 2026. Not built, not scheduled, not v1.** Written to answer "what would it take," not to green-light it. See §0 before anything else.

**Owner's ask, verbatim in spirit:** two things. (A) After the knob sets a temperature and the dial confirms it, a gentle voice says "68 degrees. Sleep well." — for people who cannot read the display at night. (B) Voice commands: "Set temperature to 68 degrees", "Turn on".

---

## 0. Two things to know before reading further

**0.1 The board has no speaker.** The Waveshare wiki's onboard-resources list is: a **PCM5100A stereo DAC** on I2S, a **3.5 mm headphone jack**, and a **digital (PDM) microphone**. Every audio demo Waveshare ships (Music Player, Bluetooth Music Player, Audio_Test) says *"External speakers or headphones are required to 3.5mm headphone jack."* The PCM5100A is a line-level DAC (~2 V RMS), not an amplifier — a bare speaker on the jack will be nearly inaudible. So feature (A) needs **an external powered speaker, or an amp board plus speaker, plugged into the jack**, and that is a bill-of-materials, enclosure and "new user can easily add their pad" problem before it is a firmware one. The mic half of the premise holds: there is a PDM mic on `GPIO45/46` (already in `SPEC-power-sensing.md` §1's pin inventory) and I2S output on `GPIO39–41`.

Consequence: **the speaker path must be strictly optional and default off.** A dial with nothing in the jack must behave exactly as 0.1.4 does. Anything else breaks the stranger's path in `V1-scope.md`.

**0.2 The scope rule.** The Sep 1 rule was "no new features until v1 is functional and a new user can easily add their pad." `0.1.4` satisfies that literally, and the night window has already gone out on the beta channel under the owner's ruling. But this is not a night-window-sized feature. It is a new component with two always-running audio tasks, ~4 MB of PSRAM, a **partition-table change** (§4.3, which OTA cannot deliver), a proprietary model dependency, and external hardware. It is the largest thing the project has considered, larger than dial-side scheduling. Recommendation: **spec now, build after `1.0.0`**, and build (A) before (B) — see §7.

---

## 1. What "can't read the display at night" actually needs

Worth separating the problem from the proposed solution, because the cheapest fix is not audio.

- **Haptic confirmation.** The DRV2605 is already wired and driven (`HAPTIC_CONFIRM`, the soft-stop pulse, the power-button breathe in §14.6 of the bring-up log). A haptic *count* — one tick per level away from a reference, a long buzz at level 0 — is silent to a sleeping partner, works with the dial in the hand under the covers, and needs no new hardware. It does not say "68," but on a device whose realistic nightly band is ten detents (log §2), "three clicks warmer than neutral" is the thing a half-asleep person actually wants to know.
- **A night face.** The night palette exists (`SPEC-night-window.md`). A night face that shows *only* the numeral at a much larger size, high contrast, no pill, no caption, is a UI change with zero hardware risk.
- **Spoken confirmation (A).** The only one of the three that reads the number out. It is also the only one that is audible to the person on the other side of the bed at 3 a.m., which is the exact hour it is for. That tension is the design problem in §3, and it is why (A) must be gated on volume, on a setting, and on the night window rather than fired unconditionally.

Recommendation: do the haptic count and the night face first regardless; they cost a day and solve most of the stated problem. Then (A) on top for the people who add a speaker.

---

## 2. Hardware inventory (verified against the wiki and the pin map already in the project)

| Part | What it is | Pins | Status |
|---|---|---|---|
| Digital MIC | PDM microphone, mono | `GPIO45/46` (clk/data) | On board, unused by this firmware |
| PCM5100A | Stereo I2S DAC, line-level out | I2S `GPIO39–41` | On board, unused; drives only the jack |
| 3.5 mm jack | Line out | — | The only audio output |
| DRV2605 | Haptic driver, I2C | shared I2C | Already driven |
| Companion ESP32 | Factory firmware, untouched | UART `GPIO48/38` | Wiki says all audio demos run on the **S3**; the companion is not in the audio path |

ESP32-S3 I2S supports PDM RX natively and can deliver 16 kHz 16-bit mono directly, which is the format ESP-SR requires. No codec configuration on the mic side; the PCM5100A needs no I2C either (it is a hardware-configured DAC). Bring-up risk on the audio hardware itself is low. Two tasks worth doing first, in this order, with nothing else changed: (1) record 3 s from the PDM mic to a buffer and dump it over serial; (2) play a 1 kHz tone out the jack into headphones. Each is an afternoon and settles the pin map without touching the product code.

**Open hardware question:** the PDM mic's placement relative to the knob and the enclosure. A mic under a hand that is turning the knob hears the hand. Only measurable on the bench.

---

## 3. Feature A — spoken confirmation

### 3.1 Design

**Trigger:** not the knob event. The existing path is knob → `post_temp()` → debounced `CMD_SET_TEMP` → worker writes `target_t` → response parsed → state updated. Speak **on the successful write**, from the worker task's result, so the voice reports what the bed accepted, not what the knob asked for. This is the same "does what it wrote actually land" rule the bring-up log's pattern two insists on (§12). If the write fails, say nothing (or a short failure phrase — see 3.3); never announce a number the pad did not take.

**What it says:** `<number> <unit>. Sleep well.` where number/unit follow the dial's own display mode: "68 degrees" on absolute °F, "minus 3" / "plus 2" / "neutral" on relative, °C as "20 degrees" when absolute °C. The dial already has this rendering logic; the voice must read the same state the numeral does, never a separately computed value. Power writes get "Bed on" / "Bed off". Off-side and single-zone guards are upstream of this hook, so a refused write never speaks.

**Tail phrase by time of day:** "Sleep well" is right at bedtime and wrong at 7 a.m. Tie it to the night window that already exists: inside the window, "Sleep well"; outside, nothing after the number. This reuses `dial_time_valid()` and the night-window state; no new clock logic.

### 3.2 The voice — how to get a "gentle, sweet" one on an ESP32

On-device English text-to-speech is the wrong tool. Espressif's `esp-tts` is **Chinese only**. The community PicoTTS port runs on the S3 but sounds like 2005; it is not the voice being asked for.

**Pre-rendered clips** are the right answer and they are cheap. Render every phrase once at build time with any good neural TTS voice, store them in flash, and concatenate at runtime. Inventory:

- Absolute °F: 54–108 → 55 clips, rendered as whole phrases ("sixty-eight degrees") so the prosody is natural rather than "sixty" + "eight" + "degrees" stitched.
- Absolute °C: 12–42 → 31 clips.
- Relative: "minus fifteen" … "plus fifteen", "neutral" → 31 clips.
- Tails and states: "Sleep well", "Bed on", "Bed off", "Couldn't reach the bed" → ~5 clips.

About 120 clips, ~0.8 s average. At 16 kHz 16-bit mono that is ~3 MB raw; as IMA-ADPCM (4:1, trivial to decode) ~800 KB; as MP3 with a decoder ~300 KB. **ADPCM in a dedicated read-only partition** is the recommendation: no decoder dependency, no licensing, and clips can be swapped by reflashing one partition. Playback is a few hundred lines: open the I2S TX channel, stream the clip, close.

Playing needs the I2S TX channel and PSRAM buffers only while a clip is playing. Idle cost is zero. **This is the property that makes (A) reasonable to ship and (B) hard** — see §4.

### 3.3 Settings (and the recurring-bug check)

Three rows, all under a "Voice" group so they can be hidden as one when no speaker is present:

- **Spoken confirmation:** Off / On. **Default Off.** The stranger's path must not change.
- **Voice volume:** a few steps. Applied as a PCM gain before the DAC — the PCM5100A has no volume register.
- **Night volume:** the volume used inside the night window, default lower. This is the partner-asleep control. Off is a valid choice here, giving "speaks in the evening, silent at 3 a.m."

Check against the pattern in `V1-scope.md`: *can it be changed from the state the device will be in, and does anything read it?* Yes on both — the rows are on Settings, reachable from any sticky screen, and the single reader is the speak-on-write hook. If the jack is empty the user hears nothing and turns it off; there is no way to detect jack insertion on this board, so no auto-hide.

### 3.4 Cost

New component `dial_audio` (I2S TX, ADPCM decode, clip table, volume), a partition for clips, three Settings rows, the hook in the worker's write-result path, a clip-generation script checked into `tools/`. Two to four days of firmware work after the bench checks in §2, plus the speaker hardware question that is not firmware at all. PSRAM: one clip buffer, tens of KB. Flash: ~1 MB partition. No always-on task, no new steady-state CPU load, no battery impact when idle.

---

## 4. Feature B — voice commands

### 4.1 The engine

The only serious on-device option is Espressif's **ESP-SR**: WakeNet (wake word) + AFE (front end: noise suppression, VAD, optional AEC) + **MultiNet7** (command phrases, English supported, phoneme-defined, up to 200 commands, runtime `esp_mn_commands_add()`). Everything else is either cloud or a hobby port. Cloud is rejected out of hand — this project deleted the cloud pipeline on purpose, and a bedside microphone streaming to a server is not something to ask a user to accept for a temperature knob.

ESP-SR documents that **command recognition is meant to run after a wake word** — MultiNet listens for a bounded window after WakeNet fires, then times out. Two ways to open that window:

- **A wake word.** Espressif ships a fixed set of English wake words; "Hey Bedknob" is not one of them, and a custom wake word is a **paid Espressif service with lead time**. Shipping "Hi ESP" as the wake word of a bedside product is not acceptable. And an always-on wake-word engine costs on the order of 20 % of one core and a live mic 24 h a day, on a device that routinely runs on battery and sits next to a sleeping person.
- **Push-to-talk.** Press the knob (or a long press, or a tap on the face), the dial opens a listening window of a few seconds, MultiNet runs on that window only. No wake word, no always-on mic, no false wakes at 3 a.m., no Espressif contract, near-zero idle cost. **This is the recommendation**, and it also fits the device: someone who can reach the knob to press it can say the number instead of finding the right detent.

### 4.2 Command grammar

"Set temperature to 68 degrees" is one phrase per number: 55 phrases for °F, plus the relative and °C variants, plus power, plus "warmer"/"cooler". Under the 200 cap, but two known weaknesses, both worth stating before anyone builds it:

- **Adjacent numbers are the hardest thing for a small command model.** "sixty-six" / "sixty-eight" / "sixty-seven" differ by one syllable. Expect misrecognitions. Mitigation: every recognized set is spoken back by feature (A) before it is written — "sixty-eight degrees, say yes" — or written immediately and spoken, with "undo" as a command. Without (A) there is no feedback loop, which is why (A) must ship first.
- **Unit mismatch.** A user on relative mode saying "68 degrees" is asking for something the dial can honor (convert °F → °C → level) but should confirm in its own unit. The grammar should carry the unit in the phrase ("sixty-eight degrees", "level minus three") so the intent is unambiguous.

Relative commands ("warmer", "cooler", "a bit warmer") are more robust than absolute numbers and match how the knob is used. Start there.

### 4.3 What it costs — this is the part that moves it past 1.0

From Espressif's own ESP32-S3 benchmarks: WakeNet9 ~324 KB PSRAM and 16 KB internal RAM; **MultiNet7 ~2.9 MB PSRAM** and 18 KB internal, ~11 ms per 32 ms frame (~35 % of a core while listening); AFE ~0.8 MB PSRAM and ~10 % of a core each for its feed and fetch tasks. With push-to-talk, WakeNet is skipped, and AFE + MultiNet only run inside the listening window — but **the ~3.7 MB of PSRAM is allocated for the life of the process** in every ESP-SR example, because model load is slow. That is nearly half of the 8 MB, on a firmware that already runs LVGL with a 360×360 framebuffer out of PSRAM. Heap growth across nights is *already* on the unverified list (log §13); this makes that question urgent rather than academic.

**Flash and the partition table.** ESP-SR models live in a dedicated `model` partition (several MB for MultiNet7 English plus AFE). The 16 MB part has room — `ota_0`/`ota_1` are 4 MB each — but adding a partition means **rewriting the partition table at `0x8000`, which OTA does not touch.** Every existing dial needs a wire flash (`idf.py flash`, NVS preserved) or a browser flash (NVS wiped). Today that is one unit on one desk, so it is cheap. The day a second unit leaves the desk, the same argument `SPEC-ota-readiness.md` §7 makes about rollback applies here. **If (B) is ever going to happen, the partition table change should go out with the 1.0.0 wire flash, empty, so later firmware can fill it over the air.** That is the single most important sequencing decision in this document.

**Licensing.** ESP-SR's models are Espressif-licensed binaries, not Apache; a row in `THIRD_PARTY_LICENSES` and a read of their terms before shipping. Compatible with PolyForm NC as far as a non-lawyer can tell; note it in `LICENSING.md` §5 when the time comes.

**Threading.** ESP-SR wants feed and fetch tasks pinned to different cores at priority 5. The firmware already runs the LVGL task, `worker_task`, `power_task`, Wi-Fi and lwIP. Nothing about this is impossible; it is a tuning job that has to be done on hardware with the display animating, because a stuttering knob is a worse product than a knob with no voice.

**Integration is the easy part.** A recognized command posts the same `CMD_SET_TEMP` / power command the knob and the drag handle post. Same queue, same debounce, same off-side guard, same single-zone guard, same write path — so a voice command cannot write anything a knob turn could not. Do not add a second write path.

### 4.4 Cost summary

Bench proof of the mic and ESP-SR on this board: a week, most of it memory and task tuning. Product integration on top of (A): another week. Plus the partition-table flash for existing units, plus the hardware question in §0.1 for the spoken feedback that (B) needs to be usable.

---

## 5. Battery and the night

Two facts from the bring-up log shape both features:

- The dial runs on battery as a normal condition (§1) and its radio spends ~40 % of some boots in modem sleep (§18.5). An always-listening mic is incompatible with that. Push-to-talk keeps (B) at zero idle cost; (A) is already zero idle cost.
- The feature exists for 3 a.m. The person next to the user does not want to hear "sixty-eight degrees, sleep well." Night volume (§3.3) with Off as an option, and the haptic count from §1, are how both people in the bed get what they want.

---

## 6. Risks, stated plainly

- No speaker on the board. The feature is not "the Bedknob talks"; it is "the Bedknob can talk if you plug something into it." Decide whether that is a product you want before writing code.
- The PSRAM cost of MultiNet is large and permanent for as long as it is loaded; it has to be measured against LVGL on the real board before anyone believes it fits.
- Partition table change is a wire-flash event for every unit in the field.
- Number recognition accuracy on a small model is mediocre by construction; the feature needs spoken read-back to be trustworthy.
- The bring-up log's recurring shape applies twice: a Voice row that appears with no speaker present is a "control that looks live" (pattern two); and a voice command that lands while the dial is in the connect loop must not sit in the queue — `CMD_SET_TEMP` is *not* one of the immediate commands F3 made drainable. Refuse voice commands (and say so, if (A) is on) unless the phase is `PH_READY`.
- The companion ESP32 still runs its factory image and "may independently drive audio or the vibration motor" (log §13). Before trusting the jack, confirm the companion is not also driving the I2S lines. One scope trace or one bench test.

---

## 7. Recommended order

1. Nothing before `1.0.0`. The housekeeping list in `V1-scope.md` stands.
2. With the 1.0.0 wire flash, add an empty `model` partition and a `voice` clip partition to the table, so (A) and (B) can later arrive over the air. Costs nothing now; saves a field reflash later.
3. Haptic level count and the night face (§1). Small, no hardware, solves most of the stated problem, and are worth doing even if voice never happens.
4. Bench: PDM capture and tone playback (§2). Settles the pin map and the companion-ESP32 question.
5. Feature (A), spoken confirmation, default off, with night volume. Ships on the beta channel like the night window did.
6. Only then, feature (B), push-to-talk, relative commands first ("warmer", "cooler", "bed on", "bed off"), absolute numbers after the read-back loop is proven.

---

## 9. Owner's ruling, Sep 3 2026 — speaker out; commands with haptic + display feedback?

Spoken confirmation (feature A) is **dropped**: no onboard speaker, and requiring an external one is not the product. The question became whether voice commands (B) stand on their own with haptic and display feedback instead of read-back.

**Relative commands: yes.** "Warmer", "cooler", "much warmer", "bed on", "bed off" need no number read-back. Feedback is the existing haptic vocabulary plus one new pulse for "heard and written" (fired on write success, same hook §3.1 described), and the display updates as it already does. A misheard "warmer" as "cooler" is one level in the wrong direction, visible the next time the face is looked at, and undone with one detent. Small blast radius.

**Absolute numbers: no, not without read-back.** A misheard "sixty-six" for "sixty-eight" lands on the bed silently; haptics cannot say which number was heard, and the display is the thing the user cannot read at night, which is the premise. That is pattern two from the bring-up log (a control that looks live and writes the wrong thing) built in on purpose. If absolute commands are ever wanted, the display must show the recognized phrase *before* the write, with a confirm (press) and a timeout that cancels — and that only helps people who can read the screen.

**Everything in §4.3 still applies.** Push-to-talk, ~3.7 MB PSRAM held while the model is loaded, a `model` partition (wire flash for existing units; add it empty at 1.0.0), Espressif model license, task tuning against LVGL on real hardware, `PH_READY` gating. Dropping the speaker removes the hardware question and the clip partition; it removes none of the recognition cost. Order in §7 stands with step 5 deleted.

## 8. Sources

Waveshare wiki, ESP32-S3-Knob-Touch-LCD-1.8 (onboard resources: PCM5100A, 3.5 mm jack, digital MIC; every audio demo requires external speakers/headphones). Espressif ESP-SR documentation for ESP32-S3: command-word recognition (MultiNet5/6/7 English, phoneme definition, 200-command cap, WakeNet precondition), audio front end (16 kHz 16-bit, single-mic supported, feed/fetch tasks pinned per core), and the resource-occupancy benchmark (WakeNet9, MultiNet7, AFE figures quoted in §4.3). `esp-tts` speech synthesis page (Chinese only). `docs/SPEC-power-sensing.md` §1 for the PDM and I2S pin assignments already inventoried in this project.
