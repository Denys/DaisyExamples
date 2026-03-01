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

**Document Version**: 1.2
**Last Updated**: 2026-02-08

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.2 | 2026-02-08 | Added Priority Queue table, Owner/Last Activity fields to template, Lessons Learned section |
| 1.1 | 2026-02-08 | Added BUG-002, BUG-003; expanded investigation methodology |
| 1.0 | 2026-02-08 | Initial version: BUG-001 (LED mirroring) with full resolution |
