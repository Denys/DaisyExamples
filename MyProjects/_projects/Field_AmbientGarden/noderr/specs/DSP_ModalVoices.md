# Node Specification: DSP_ModalVoices - 4× ModalVoice Physical Synthesis

**Version:** 1.0
**Date:** 2026-03-06
**Author:** AI-Agent (Draft — retrofit)
**Classification:** Complex

## 1. Purpose
* **Goal:** Provide the synthesis core: 4 instances of DaisySP's `ModalVoice` (physical modeling resonator), each independently triggered by `FW_ClockTrigger` with different frequencies and voice roles. Summed with stereo panning before the DSP chain.

## 2. Dependencies & Triggers
* **Prerequisite NodeIDs:** `HW_Platform` (sample rate for Init), `FW_ClockTrigger` (SetFreq/SetAccent/Trig calls), `FW_KnobProcessor` (brightness/damping/structure at 60Hz), `CFG_VoicePresets` (preset parameters)
* **Library:** `DaisySP` — requires `USE_DAISYSP_LGPL = 1` in Makefile
* **Input Data/State:** Per-trigger: frequency (Hz), accent (0–1), brightness, damping, structure. Per-block: brightness/damping/structure from ProcessKnobs()

## 3. Interfaces

```cpp
ModalVoice modal[4]; // global array

// Initialization (main, before StartAudio)
modal[v].Init(sample_rate);
modal[v].SetFreq(220.0f);
modal[v].SetBrightness(preset.brightness);
modal[v].SetStructure(preset.structure);
modal[v].SetDamping(preset.damping);
modal[v].SetAccent(preset.accent);

// On clock trigger (from AudioCallback via FW_ClockTrigger)
modal[v].SetFreq(hz);
modal[v].SetAccent(accent);
modal[v].Trig(); // must call Trig() after SetFreq/SetAccent

// Per-sample (within AudioCallback inner loop)
float sig = modal[v].Process();
```

## 4. Core Logic & Processing Steps

1. Init all 4 voices with sample rate and Glass Bells (preset 0) defaults
2. On each clock trigger: voice 0 always fires; voices 1-3 fire probabilistically
3. Per-sample: call `modal[v].Process()` for all 4 voices regardless of activity (produces silence when decayed)
4. Sum voices with stereo panning:
   - Mono: each voice 0.25× into both channels
   - Wide: V0,V2 → 0.35L/0.15R; V1,V3 → 0.15L/0.35R

## 5. Data Structures

| Voice | Role | Probability | Freq Relationship |
|-------|------|-------------|-------------------|
| 0 | Melody (primary) | 100% | Base frequency from quantizer |
| 1 | Harmony low | 60% | Base × 2^(-spread×12/12) |
| 2 | Harmony high | 40% | Base × 2^(spread×7/12) |
| 3 | Accent chime | 20% | Base × 2.0 (octave up) |

**ModalVoice parameter ranges (DaisySP API):**
- `SetFreq(hz)`: 20–8000 Hz recommended
- `SetBrightness(0–1)`: 0 = dark/woody, 1 = bright/glassy
- `SetStructure(0–1)`: 0 = metallic, 1 = woody
- `SetDamping(0–1)`: 0 = long ring, 1 = short pluck
- `SetAccent(0–1)`: excitation amplitude
- `Trig()`: triggers new excitation (must be called after SetFreq/SetAccent)

## 6. Error Handling & Edge Cases
* Frequencies below 30 Hz or above 8000 Hz clamped in FW_ClockTrigger before reaching here
* Always call `Trig()` AFTER `SetFreq()` and `SetAccent()` (order matters in DaisySP ModalVoice)
* `Process()` called every sample even on idle voices — returns near-zero, safe

## 7. ARC Verification Criteria

* **Functional:**
  * ARC_FUNC_01: Distinct timbral character between all 8 presets (Glass vs Temple should be clearly audible)
  * ARC_FUNC_02: K4 (spread) at max creates wide harmonic spread; at min all voices cluster near base pitch
  * ARC_FUNC_03: K5 (brightness) audibly controls brightness of voice character
  * ARC_FUNC_04: K6 (damping) audibly controls decay length (short pluck vs long ring)
* **LGPL:**
  * ARC_LGPL_01: Makefile contains `USE_DAISYSP_LGPL = 1` — build fails without it
  * ARC_LGPL_02: `ModalVoice` resolves correctly at link time (not undefined reference)

## 8. Notes & Considerations

* **LGPL requirement:** `ModalVoice` and `ReverbSc` are LGPL modules in DaisySP. The Makefile MUST include `USE_DAISYSP_LGPL = 1` or the linker will report undefined references.
* `Process()` on all 4 voices every sample is the dominant CPU cost (~60-70% of budget). Adding more DSP effects risks overflow on Cortex-M7.
* Voices 1-3 retain their last `SetBrightness/Damping/Structure` settings between triggers. Preset switches apply these params immediately via FW_KnobProcessor's preset handling in main loop.
