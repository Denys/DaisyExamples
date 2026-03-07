# Node Specification: FW_AudioCallback - Audio Processing Entrypoint

**Version:** 1.0
**Date:** 2026-03-06
**Author:** AI-Agent (Draft — retrofit)
**Classification:** Critical

## 1. Purpose
* **Goal:** Main real-time audio processing function. Called by libDaisy DMA engine once per audio block (~48 samples at 48kHz). Advances DSP chain per sample, applies parameter smoothing per block, and writes stereo output.

## 2. Dependencies & Triggers
* **Prerequisite NodeIDs:** `HW_Platform` (provides sample rate, calls StartAudio), `FW_ParamBridge` (reads Params struct), `GEN_RandomClock` (per-sample clock), `FW_ClockTrigger` (on clock fire), `DSP_ModalVoices`, `DSP_PreLPF`, `DSP_Reverb`, `DSP_SoftClip`
* **Input Data/State:** `params.*` (written by main loop via ProcessKnobs), `stereo_wide` flag, audio block `in` buffer and `size`
* **Trigger:** Called by libDaisy audio DMA interrupt, NOT by user code

## 3. Interfaces
* **Signature:** `void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)`
* **Outputs:** `out[0][i]` (left), `out[1][i]` (right) — non-interleaved, range ~±1.0f after soft clip
* **Audio format:** Non-interleaved (Daisy Field platform requirement)

## 4. Core Logic & Processing Steps

Per-block (before sample loop):
1. Smooth reverb and LPF target parameters using `Smooth()` (custom first-order IIR)
2. Apply smoothed params to `preLpfL/R.SetFreq()`, `reverb.SetFeedback()`, `reverb.SetLpFreq()`

Per-sample (inner loop, i = 0..size-1):
3. Call `rclock.Process()` → if returns true, call `OnClockTrigger()`
4. Sum 4 modal voice outputs with stereo panning (mono or wide depending on `stereo_wide`)
5. Run `preLpfL/R.Process()` → take `.Low()` output
6. Run `reverb.Process(filt_l, filt_r, &rev_l, &rev_r)`
7. Mix dry (filt_l/r) and wet (rev_l/r) by `smoothed.wet_dry`
8. Soft clip: `out[ch][i] = tanhf(mix * 1.5f) * 0.8f`

## 5. Data Structures

```cpp
// Read-only from callback (written by main loop)
struct Params {
    float density, root_knob, reverb_size, spread;
    float brightness, damping, structure, wet_dry;
};

// Maintained within callback (never touched by main loop)
struct SmoothedParams {
    float brightness, damping, structure; // ⚠️ BUG-2: computed but unused
    float lpf_cutoff, rev_decay, rev_lpfreq, wet_dry;
};
```

## 6. Error Handling & Edge Cases
* No malloc, new, printf, or blocking calls permitted inside callback
* `rand()` must NOT be called here (see BUG-1 in FW_ClockTrigger spec)
* If `size` is 0: inner loop does not execute — safe
* Stereo width flag `stereo_wide` is only read, never written — no race condition

## 7. ARC Verification Criteria

* **Functional:**
  * ARC_FUNC_01: Audio output present with no static/silence at K1 max density
  * ARC_FUNC_02: Reverb applies (audible difference between K8=0 and K8=1)
  * ARC_FUNC_03: Stereo width changes when SW2 toggled
  * ARC_FUNC_04: No audio dropout at full 4-voice polyphony (K1=max, K4=max)
* **Real-time Safety:**
  * ARC_RT_01: No `rand()`, `malloc()`, `new`, or `printf` anywhere in callback call tree
  * ARC_RT_02: Callback completes within block period (no underrun LEDs on Daisy)
* **Error Handling:**
  * ARC_ERR_01: `size=0` does not crash

## 8. Notes & Considerations

* **BUG-2 (RESOLVED in fix-20260306-BUG2):** Lines 282-286 computed `smoothed.brightness/damping/structure` but never applied them to modal voices. These 3 lines were dead code and have been removed. ModalVoice timbre params are set at 60Hz via ProcessKnobs() which is sufficient.
* `tanhf()` per-sample is CPU-heavy; acceptable on Cortex-M7 with FPU but leaves little headroom for adding more DSP modules.
* `stereo_wide` is a `bool` read from main loop without mutex — safe on Cortex-M7 (atomic single-byte read), but technically undefined behavior per C++.
