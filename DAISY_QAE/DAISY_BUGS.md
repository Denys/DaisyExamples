# Daisy Embedded C++ Bug Tracker

> **Methodology**: Each bug must be analyzed systematically:
> 1. **Problem**: Clear description of observed behavior
> 2. **Expected**: What should happen
> 3. **Analysis**: Investigation findings
> 4. **Hypotheses**: Possible causes with reasoning
> 5. **Solutions**: Proposed fixes for each hypothesis
> 6. **Resolution**: What actually worked and why

---

## Active Issues — Priority Queue

| Bug | Severity | Status | Project | Last Activity | Next Action |
|-----|----------|--------|---------|---------------|-------------|
| [BUG-002](#bug-002-field_modalbells-b-row-key-crash) | Critical | 🟡 INVESTIGATING | Field_ModalBells | 2026-02-08 | Add serial debug checkpoints around Trig() loop |
| [BUG-003](#bug-003-field_modalbells-midi-not-working) | Medium | 🔴 OPEN | Field_ModalBells | 2026-02-08 | Verify MIDI init sequence and handler registration |
| [BUG-004](#bug-004-x0x-display-white-noise-controls-unresponsive) | High | 🟡 INVESTIGATING | x0x_drum_machine | 2026-02-27 | HiHat crash confirmed as root cause — replaced with WhiteNoise+ATone+AdEnv, awaiting flash |

> **Triage rule**: Critical/High bugs should have activity within 48 hours.
> Update "Last Activity" and "Next Action" every time you touch a bug.

---

## BUG-001: Keyboard LED Mirroring

**Date**: 2026-02-08  
**Project**: Field_WavetableDroneLab  
**Severity**: High  
**Status**: ✅ RESOLVED

### Problem
Pressing key B1 lights up LED B8, B2→B7, A1→A8, A2→A7 (mirrored within each row).

### Expected
Pressing key Bn should light LED Bn. Same for row A.

### Analysis

**Step 1: Trace the signal path**
```
Key Press → hw.KeyboardRisingEdge(index) → keyLeds.SetA/B(i) → kLedKeysA/B[i] → LED Driver
```

**Step 2: Check libDaisy source (`daisy_field.h` lines 52-81)**
```cpp
enum {
    LED_KEY_B1 = 0, LED_KEY_B2 = 1, ... LED_KEY_B8 = 7,   // B: 0-7
    LED_KEY_A8 = 8, LED_KEY_A7 = 9, ... LED_KEY_A1 = 15,  // A: 8-15 REVERSED!
}
```

**Step 3: Check working project (`field_wavetable_morph_synth`)**
```cpp
// Keys: A = indices 0-7, B = indices 8-15
if(hw_->KeyboardRisingEdge(i)) ...      // A keys
if(hw_->KeyboardRisingEdge(i + 8)) ...  // B keys

// LEDs: A = 15-i, B = i
int led_idx = 15 - active_bank_idx;     // A row
hw_->led_driver.SetLed(active_curve_idx, 1.0f);  // B row
```

### Hypotheses

| # | Hypothesis | Reasoning |
|---|-----------|-----------|
| 1 | LED arrays wrong | Our kLedKeysA/B don't match libDaisy enum |
| 2 | Key indices wrong | Our kKeyAIndices/B don't match actual hardware |
| 3 | Both wrong | Arrays based on incorrect pinout interpretation |

### Solutions Tested

| Attempt | kKeyAIndices | kKeyBIndices | kLedKeysA | kLedKeysB | Result |
|---------|--------------|--------------|-----------|-----------|--------|
| 1 | `{0-7}` | `{8-15}` | `{0-7}` | `{8-15}` | ❌ Mirrored |
| 2 | `{15-8}` | `{0-7}` | `{15-8}` | `{0-7}` | ❌ Mirrored |
| 3 | `{15-8}` | `{0-7}` | `{8-15}` | `{7-0}` | ❌ Mirrored |
| 4 | `{0-7}` | `{8-15}` | `{15-8}` | `{0-7}` | ✅ **WORKS** |

### Resolution

**Root Cause**: The keyboard input indices and LED indices follow DIFFERENT patterns:
- **Keyboard**: Row A = indices 0-7, Row B = indices 8-15
- **LEDs**: Row A = indices 15-8 (reversed), Row B = indices 0-7

**Correct Mapping**:
```cpp
kKeyAIndices  = {0, 1, 2, 3, 4, 5, 6, 7};       // A1-A8 input
kKeyBIndices  = {8, 9, 10, 11, 12, 13, 14, 15}; // B1-B8 input
kLedKeysA     = {15, 14, 13, 12, 11, 10, 9, 8}; // A1-A8 LEDs
kLedKeysB     = {0, 1, 2, 3, 4, 5, 6, 7};       // B1-B8 LEDs
```

**Lesson Learned**: Always verify against a known working implementation before making assumptions from documentation or pinout diagrams.

---

## BUG-002: Field_ModalBells B-Row Key Crash

**Date**: 2026-02-08  
**Project**: Field_ModalBells  
**Severity**: Critical  
**Status**: 🟡 INVESTIGATING

### Problem
1. System works initially (OLED shows "ModalBells 2M Soft", knobs work, A-row keys work)
2. Pressing ANY B-row key (B1-B8) causes **complete system stall**
3. After stall: OLED frozen, knobs unresponsive, LEDs stuck
4. External MIDI doesn't work from startup (separate issue?)

### Expected
B-row keys should select strike type (Soft/Medium/Hard/etc.) and trigger active modes.

### Analysis

**Step 1: Trace B-row key handler (lines 211-236)**
```cpp
// Row B: Select strike type
for(int i = 0; i < 8; i++)
{
    if(hw.KeyboardRisingEdge(kKeyBIndices[i]))  // kKeyBIndices = {8,9,...,15}
    {
        // Clear all B LEDs first
        for(int j = 0; j < 8; j++)
            keyLeds.SetB(j, false);  // Uses kLedKeysB[j] = {0,1,...,7}

        // Set new strike type
        strike_type = i;
        keyLeds.SetB(i, true);

        // Trigger all active modes with new strike type
        for(int m = 0; m < kNumModes; m++)
        {
            if(mode_active[m])
            {
                modal[m].SetBrightness(params.brightness * kStrikeTypes[i].brightness_mod);
                modal[m].SetAccent(kStrikeTypes[i].velocity);
                modal[m].Trig();  // <-- POTENTIAL ISSUE: Multiple Trig() calls?
            }
        }
    }
}
```

**Step 2: Check ModalVoice::Trig() behavior**
- Multiple simultaneous `Trig()` calls on all 8 voices
- Could cause audio buffer overrun or DSP overload

**Step 3: Check kKeyBIndices mapping**
- `kKeyBIndices = {8, 9, 10, 11, 12, 13, 14, 15}` (confirmed correct from BUG-001)
- Should trigger correctly

### Hypotheses

| # | Hypothesis | Reasoning |
|---|-----------|-----------|
| 1 | **Audio DSP overload** | Triggering multiple ModalVoice simultaneously = too much CPU |
| 2 | **Stack overflow** | Processing 8 modes with Trig() in tight loop |
| 3 | **LED driver conflict** | Array index out of bounds with kLedKeysB? |
| 4 | **kStrikeTypes array access** | Accessing `kStrikeTypes[i]` where i could be wrong |

### Solutions Tested

| Attempt | Change | Result |
|---------|--------|--------|
| 1 | Added SwapBuffersAndTransmit() for LEDs | Partial fix - system starts now |
| 2 | ... | Pending |

### Resolution
*Pending investigation*

---

## BUG-003: Field_ModalBells MIDI Not Working

**Date**: 2026-02-08  
**Project**: Field_ModalBells  
**Severity**: Medium  
**Status**: 🔴 OPEN

### Problem
External MIDI input doesn't work from startup.

### Expected
MIDI note input should trigger modal voices.

### Analysis
*Not yet investigated - likely missing MIDI initialization and handler*

### Resolution
*Pending investigation*

---

## Template for New Bugs

```markdown
## BUG-XXX: [Title]

**Date**: YYYY-MM-DD
**Project**: [Project Name]
**Severity**: Low/Medium/High/Critical
**Status**: 🔴 OPEN / 🟡 INVESTIGATING / ✅ RESOLVED
**Owner**: [who is investigating]
**Last Activity**: YYYY-MM-DD

### Problem
[Describe observed behavior]

### Expected
[Describe expected behavior]

### Analysis
[Investigation steps and findings]

### Hypotheses
| # | Hypothesis | Reasoning |
|---|-----------|-----------|
| 1 | ... | ... |

### Solutions Tested
| Attempt | Change | Result |
|---------|--------|--------|
| 1 | ... | ❌/✅ |

### Resolution
[What worked and why]

### Lessons Learned
[What can be generalized from this fix — update DEVELOPMENT_STANDARDS if applicable]
```

> **When closing a bug**: Update the Priority Queue table above (remove the row),
> and update `Last Activity` in the bug entry.

---

## Quality Assurance Ecosystem

This document is part of an interconnected quality assurance system:

| Document | Purpose | When to Use |
|----------|---------|-------------|
| [DAISY_TUTORIALS_KNOWLEDGE.md](DAISY_TUTORIALS_KNOWLEDGE.md) | Official API reference | Understanding GPIO/Audio/ADC |
| [DAISY_DEVELOPMENT_STANDARDS.md](DAISY_DEVELOPMENT_STANDARDS.md) | Workflow patterns | Starting a new project |
| [DAISY_DEBUG_STRATEGY.md](DAISY_DEBUG_STRATEGY.md) | Serial/hardware debugging | Investigating crashes |
| [DAISY_TECHNICAL_REPORT.md](DAISY_TECHNICAL_REPORT.md) | Complete process documentation | Deep reference |

**This document's role**: Tracks bugs with structured investigation methodology. When you solve a bug, document it here so the team learns from it.

---

---

## BUG-004: x0x Display White Noise + Controls Unresponsive

**Date**: 2026-02-27
**Project**: x0x_drum_machine
**Severity**: High
**Status**: 🟡 INVESTIGATING
**Last Activity**: 2026-02-27

### Problem (Original)

On hardware, display showed "mostly white noise." Turning knobs had no visible effect. Pressing keyboard keys produced no LED response.

### New Symptoms (After Fix Attempt 1)

After flashing fix attempt 1 (display pre-clear + 1ms delay + Delay(15)):

- Display shows "x0x INIT..." — boot noise is FIXED ✅
- Display shows NO further updates — `UpdateDisplay()` not running in main loop ❌
- Turning knobs: no change to params or knob LED intensity ❌
- Key LEDs A1, A6, B1, B6 lit from boot (expected A1, A5, B1, B5) ❌
- Pressing keys: no response ❌

### Expected

Display shows drum name + BPM + step pattern, updating in real time. Knobs change sound and display. A/B keys toggle steps. Key LEDs show kick pattern (A1, A5, B1, B5 for 4-on-floor).

### Analysis

#### Hardware Topology (Verified from libDaisy source)

**CRITICAL CORRECTION**: Previous analysis claimed OLED uses I2C — this is WRONG.

- **LED driver (PCA9685 ×2)**: I2C_1, PB8/PB9, 1 MHz, DMA async
- **OLED (SSD1306)**: 4-wire **SPI** (pins PB4/PB15 + separate SPI bus) — `SSD130x4WireSpi128x64Driver`
- These are on **completely separate hardware buses** — no I2C contention possible

The 1ms gap between `UpdateLeds()` and `UpdateDisplay()` added in Fix Attempt 1 was based on the wrong I2C contention theory and is therefore **irrelevant**.

#### Root Cause 1: Double Knob Processing (Confirmed)

```cpp
// Main loop:
hw.ProcessAllControls(); // ← internally calls hw.knob[i].Process() for all 8 knobs
ProcessControls();       // ← calls hw.knob[i].Process() AGAIN
```

`AnalogControl::Process()` applies a one-pole LP filter. Called twice per loop iteration, the effective time constant doubles. With the 16ms loop, LP filter sluggishness from double-processing may make knob response nearly imperceptible, explaining "knobs dead" symptom.

**Fix**: Change `ProcessControls()` to use `.Value()` instead of `.Process()`.

#### Root Cause 2: Display Never Reached (Working Hypothesis)

`UpdateLeds()` calls `hw.led_driver.SwapBuffersAndTransmit()` which starts an async I2C DMA transfer. If the DMA completion flag isn't set when `SwapBuffersAndTransmit()` is called again on the NEXT loop iteration, the function **blocks until DMA completes**. `UpdateDisplay()` coming after `UpdateLeds()` means a DMA stall would block the display update indefinitely.

The more likely explanation: **main loop MAY not be running at all** if the audio callback causes a hard fault (infinite loop in `HardFault_Handler`). Display would then be frozen on "x0x INIT..." indefinitely.

**Diagnostic Fix**: Move `UpdateDisplay()` BEFORE `UpdateLeds()`. If display still shows "x0x INIT...", the main loop is definitely not running → audio callback crash.

#### Root Cause 3: A5/A6 LED Offset (Suspected Non-Issue)

User observes kick-pattern LEDs at A1, **A6**, B1, **B6** instead of A1, A5, B1, B5.

- KICK pattern: steps 0, 4, 8, 12 → i=0 (A1) and i=4 (A5) for A-row
- The playhead at step 5 lights A6 at full brightness while A5 is dimmed — if user observes during playback this looks like A6 is "the active step"
- No code bug found for LED addressing; kLedKeysA mapping verified correct

### Solutions Tested

| # | Changes Applied                                                                                              | Outcome                                                          |
|---|--------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------|
| 1 | Pre-boot display clear + 1ms LED/display gap + Delay(15)                                                     | Boot noise FIXED; runtime display and knobs still dead           |
| 2 | `.Value()` in ProcessControls + display before LEDs + Delay(16)                                              | Zero effect — identical symptoms; confirmed main loop never runs |
| 3 | Replace `HiHat<>` with `WhiteNoise+OnePole+AdEnv`; revert `.Value()`→`.Process()`; `Delay(16)`→`Delay(2)`    | Build error: `ATone` and `AdEnv::Process(bool)` wrong → fixed    |
| 4 | Fix `ATone`→`OnePole` + `FILTER_MODE_HIGH_PASS`; `AdEnv::Trigger()`+`Process()` no-arg; `SetFrequency(f/sr)` | Build clean (114 KB flash). Awaiting hardware flash              |

### Resolution

*Pending hardware test of Fix Attempt 4 (clean build confirmed).*

### Lessons Learned (Interim)

- **`HiHat<>::Process()` in audio callback causes hard fault** — `HiHat<>` is NOT in the verified API list in DAISY_HALLUCINATION_REFERENCE.md. Its `Process()` call from the ISR-context audio callback triggers a hard fault on the first invocation, preventing the main loop from ever running. Replace with `WhiteNoise + OnePole + AdEnv` (all verified APIs).
- **`ATone` does not exist in DaisySP** — there is no `ATone` class. The 1-pole HP filter is `OnePole` with `SetFilterMode(OnePole::FILTER_MODE_HIGH_PASS)`. Freq is normalized (0–0.497): `SetFrequency(hz / sample_rate)`. No sample_rate arg in `Init()`.
- **`AdEnv::Process()` takes no arguments** — to trigger: call `hh_env.Trigger()` when the step fires; then call `hh_env.Process()` (no args) every sample. Do NOT pass the trig bool to `Process()`.
- **"Main loop changes have zero effect" = audio callback crash** — if display is frozen on the boot string and NO main-loop change has any visible effect, the audio callback crashes before the main loop starts. Diagnose the callback first.
- **Daisy Field OLED uses SPI, not I2C** — `SSD130x4WireSpi128x64Driver` is a 4-wire SPI driver. The PCA9685 LED chips use I2C. These buses are completely independent.
- **Never assume bus topology from chip type** — always verify from `daisy_field.cpp` `Init()` implementation.
- **Double `.Process()` is fine** — official examples (Chorus.cpp, Field_AnalogDrumCore.cpp, HALLUCINATION_REFERENCE template) all call `.Process()` in a helper after `hw.ProcessAllControls()`. The `.Value()` pattern is NOT required and should not be assumed.
- **Update OLED before LED driver** as standard ordering for Field projects (SPI is synchronous; I2C LED DMA runs async during the final Delay).
- **Reference `Field_AnalogDrumCore.cpp`** — confirmed working drum sequencer with identical architecture. Use `Delay(2)` not `Delay(16)`.

---

**Document Version**: 1.4
**Last Updated**: 2026-02-27

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.4 | 2026-02-27 | BUG-004: Fix Attempt 3 (HiHat<> root cause confirmed; replaced with WhiteNoise+ATone+AdEnv) |
| 1.3 | 2026-02-27 | Added BUG-004 (x0x display + controls) with I2C contention and boot-noise findings |
| 1.2 | 2026-02-08 | Added Priority Queue table, Owner/Last Activity fields to template, Lessons Learned section |
| 1.1 | 2026-02-08 | Added BUG-002, BUG-003; expanded investigation methodology |
| 1.0 | 2026-02-08 | Initial version: BUG-001 (LED mirroring) with full resolution |
