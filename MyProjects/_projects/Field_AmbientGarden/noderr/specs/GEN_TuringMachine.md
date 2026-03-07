# Node Specification: GEN_TuringMachine - 8-bit Shift Register Sequence Generator

**Version:** 1.0
**Date:** 2026-03-06
**Author:** AI-Agent (Draft — retrofit)
**Classification:** Complex

## 1. Purpose
* **Goal:** Implement an 8-bit shift register (inspired by the Music Thing Modular Turing Machine) that generates pseudo-deterministic melodic sequences. At probability=0, the pattern loops; at probability=0.5, it evolves slowly; at probability=1, it is fully random. Output is an 8-bit value (0–255) fed to GEN_ScaleQuantizer.

## 2. Dependencies & Triggers
* **Prerequisite NodeIDs:** None (self-contained, callback-safe)
* **Input Data/State:** `probability` (0–0.5, set from K4 via FW_KnobProcessor), `freeze` flag (from SW1 via main loop)
* **Source:** `turing_machine.h` (custom header, local to project)
* **Trigger:** Called once per clock tick from `FW_ClockTrigger`

## 3. Interfaces

```cpp
TuringMachine tm; // global instance

// Initialization (main, before StartAudio)
tm.Init();

// From FW_KnobProcessor (main loop, ~60Hz)
tm.SetProbability(prob);   // 0 = locked, 0.5 = max chaos
tm.SetFreeze(freeze_bool); // if true, Process() returns same value

// From FW_ClockTrigger (AudioCallback, on clock tick)
uint8_t value = tm.Process(); // advance shift register, return 8-bit output
```

## 4. Core Logic & Processing Steps

Conceptual algorithm (from turing_machine.h):
1. Maintain an 8-bit shift register `reg`
2. On each `Process()` call:
   a. If frozen: return `reg & 0xFF` unchanged
   b. Shift register left by 1 bit
   c. With probability `p`: new LSB = random(0,1)
   d. With probability `1-p`: new LSB = `(reg >> 7) & 1` (feedback from MSB — creates loop)
   e. Return `reg & 0xFF`

## 5. Data Structures

```cpp
class TuringMachine {
    uint8_t  reg_;         // 8-bit shift register state
    float    probability_; // 0 = loop, 0.5 = chaos
    bool     freeze_;      // if true, hold current pattern
public:
    void    Init();
    void    SetProbability(float p);
    void    SetFreeze(bool freeze);
    uint8_t Process();
};
```

## 6. Error Handling & Edge Cases
* `probability` clamped to [0, 0.5] — values above 0.5 should behave as 0.5
* When frozen: `Process()` is still called but register does not advance
* Initial state: `reg_` should be non-zero (otherwise may stay stuck at 0)
* Uses the xorshift32 PRNG (after BUG-1 fix) — NOT `rand()`

## 7. ARC Verification Criteria

* **Functional:**
  * ARC_FUNC_01: With probability=0, same sequence loops indefinitely (deterministic, verifiable by ear)
  * ARC_FUNC_02: With probability=0.5, sequence evolves continuously (no fixed loop)
  * ARC_FUNC_03: SW1 freeze → melody locks at current pattern; all 4 voices ring out naturally
  * ARC_FUNC_04: K4 at center (0.5) → `prob = |0.5×2-1|×0.5 = 0` → maximum loop stability
  * ARC_FUNC_05: K4 at extremes (0 or 1) → `prob = 0.5` → maximum chaos
* **Real-time Safety:**
  * ARC_RT_01: `Process()` uses no heap, no locks, completes in < 1μs

## 8. Notes & Considerations

* The K4 mapping formula in FW_KnobProcessor is: `prob = fabsf(k4 * 2.0f - 1.0f) * 0.5f`
  - This creates a V-shape: center (0.5) = locked loop, extremes (0 or 1) = maximum chaos
  - Musically: center position creates stable repeating patterns, edges create evolving sequences
* The 8-bit output maps to one of the scale quantizer's pitch table entries, so full range (0-255) produces the full scale range set by GEN_ScaleQuantizer.
* `Init()` should seed with a non-zero value to avoid all-zero stuck state.
