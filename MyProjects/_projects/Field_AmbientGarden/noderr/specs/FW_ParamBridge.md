# Node Specification: FW_ParamBridge - Main→Callback Parameter Contract

**Version:** 1.0
**Date:** 2026-03-06
**Author:** AI-Agent (Draft — retrofit)
**Classification:** Standard

## 1. Purpose
* **Goal:** Define the data contract between the main loop (which reads hardware) and the AudioCallback (which processes audio). Two structs act as the communication channel: `Params` (written by main, read by callback) and `SmoothedParams` (maintained entirely within callback).

## 2. Dependencies & Triggers
* **Prerequisite NodeIDs:** `HW_Knobs` (populates Params), `FW_KnobProcessor` (writes Params), `FW_AudioCallback` (reads Params, maintains SmoothedParams)
* **Input Data/State:** Raw knob values (0–1) from FW_KnobProcessor

## 3. Interfaces

```cpp
// Written by: FW_KnobProcessor (main loop, ~60Hz)
// Read by:    FW_AudioCallback (DMA interrupt, ~1000Hz blocks)
struct Params {
    float density;      // K1: clock density [0–1]
    float root_knob;    // K2: scale root raw [0–1]
    float reverb_size;  // K3: reverb size [0–1]
    float spread;       // K4: harmony spread + TM probability [0–1]
    float brightness;   // K5: ModalVoice brightness + LPF cutoff [0–1]
    float damping;      // K6: ModalVoice damping [0–1]
    float structure;    // K7: ModalVoice structure [0–1]
    float wet_dry;      // K8: reverb wet/dry mix [0–1]
} params;

// Written and read by: FW_AudioCallback only (never touched by main loop)
struct SmoothedParams {
    // NOTE: brightness/damping/structure fields were removed (dead code BUG-2)
    float lpf_cutoff;   // Smoothed LPF cutoff freq [800–12000 Hz]
    float rev_decay;    // Smoothed reverb feedback [0.70–0.99]
    float rev_lpfreq;   // Smoothed reverb LP freq [2000–16000 Hz]
    float wet_dry;      // Smoothed wet/dry mix [0–1]
} smoothed;

// Float array cached by FW_KnobProcessor for OLED display
float knob_values[8];
```

## 4. Core Logic & Processing Steps

**Main loop side (FW_KnobProcessor writes):**
1. Call `hw.knob[i].Process()` for i=0..7 → store in `knob_values[]` and `params.*`
2. Apply derived parameters (rclock density, quantizer root, TM probability)
3. Set modal voice timbre params directly (brightness, damping, structure)

**Callback side (reads):**
4. Read `params.*` at block start
5. Compute smoothed targets and advance `smoothed.*` via first-order IIR
6. Apply smoothed values to DSP module setters

## 5. Data Structures
* See struct definitions in §3 above.
* `kSmoothCoeff = 0.01f` — per-block smoothing coefficient (higher = faster response, more zipper noise)

## 6. Error Handling & Edge Cases
* All `params.*` values are raw 0–1 floats from knobs — no validation needed (ADC is bounded)
* `smoothed.*` tracks `params.*` with lag; initial values set to safe defaults matching first preset
* No mutex required on Cortex-M7 for 32-bit float writes to aligned addresses (atomic in practice)

## 7. ARC Verification Criteria

* **Functional:**
  * ARC_FUNC_01: Moving any knob results in audible change within 50ms (smoothing lag acceptable)
  * ARC_FUNC_02: `knob_values[]` correctly reflects current knob position in OLED display
  * ARC_FUNC_03: Rapid knob movement produces smooth param change (no zipper noise on reverb/LPF)
* **Safety:**
  * ARC_SAF_01: No direct hardware calls (knob.Process()) from AudioCallback

## 8. Notes & Considerations

* **After BUG-2 fix:** `SmoothedParams` no longer has `brightness`, `damping`, `structure` fields. These were removed as dead code. ModalVoice timbre is updated at ~60Hz from ProcessKnobs() — sufficient resolution for these slow-changing timbral params.
* `kSmoothCoeff = 0.01f` is applied per block (48 samples at 48kHz ≈ 1ms blocks). This gives a time constant of ~100ms (1/0.01 blocks × 1ms = 100ms), suitable for reverb tail and LPF sweeps.
