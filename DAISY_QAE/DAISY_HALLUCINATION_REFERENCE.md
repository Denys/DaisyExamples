# Daisy C++ Anti-Hallucination Reference

> **Purpose**: Paste this document into any AI prompt when generating Electrosmith Daisy C++ code.
> It grounds the LLM against real DaisySP APIs and prevents the most common hallucination patterns.

---

## ⚠️ Critical: Most Common LLM Hallucinations

These are **fabricated methods that do not exist** in DaisySP. Using them causes compile errors.

| ❌ Hallucinated (DO NOT USE) | ✅ Correct DaisySP API |
|:---|:---|
| `filter.SetCutoff(freq)` | `filter.SetFreq(freq)` |
| `filter.SetCutoffFrequency(freq)` | `filter.SetFreq(freq)` |
| `filter.SetQ(q)` | `filter.SetRes(res)` |
| `filter.SetResonance(res)` | `filter.SetRes(res)` |
| `env.SetAttack(time)` | `env.SetTime(ADSR_SEG_ATTACK, time)` |
| `env.SetDecay(time)` | `env.SetTime(ADSR_SEG_DECAY, time)` |
| `env.SetRelease(time)` | `env.SetTime(ADSR_SEG_RELEASE, time)` |
| `osc.SetFrequency(freq)` | `osc.SetFreq(freq)` *(note: Fm2 is the exception — see below)* |
| `osc.setFreq(freq)` | `osc.SetFreq(freq)` *(PascalCase!)* |
| `osc.SetWaveType(type)` | `osc.SetWaveform(waveform)` |
| `filt.setCutoffFrequency(f)` | `filt.SetFreq(f)` |
| **DAFX** `wah.SetFrequency(f)` | `wah.SetFreq(f)` *(DAFX headers normalized — old name removed)* |
| **DAFX** `wah.SetQ(q)` | `wah.SetRes(q)` *(WahWah only — renamed to match DaisySP)* |

---

## Critical Architectural Rules

```
❌ NEVER call hw.ProcessAllControls() inside AudioCallback
✅ ALWAYS call hw.ProcessAllControls() in the main while(1) loop

❌ NEVER call malloc, new, printf, or PrintLine inside AudioCallback
✅ Use flag variables — set flag in callback, act on it in main loop

❌ NEVER skip Init() for DSP modules
✅ ALWAYS call module.Init(sample_rate) before StartAudio()

❌ NEVER use std::vector, std::map, std::function, or lambdas with captures
✅ Use static arrays and simple C-style patterns

✅ Call StartAdc() BEFORE StartAudio()
```

---

## DaisySP API Quick Reference

### callRate tags

| Tag | Meaning |
|-----|---------|
| `[init]` | Call once in `main()` before `hw.StartAudio()` |
| `[control]` | Call in `ProcessKnobs()` / main loop at ~60Hz |
| `[audio]` | Call per-sample inside `AudioCallback` |

---

### Oscillators

```cpp
// Oscillator — standard analog-style oscillator
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
void SetAmp(float amp)            // 0.0 - 1.0                [control]
void SetWaveform(uint8_t waveform)                            // [control]
  // ^ WAVE_SIN | WAVE_TRI | WAVE_SAW | WAVE_RAMP | WAVE_SQUARE
  // ^ WAVE_POLYBLEP_TRI | WAVE_POLYBLEP_SAW | WAVE_POLYBLEP_SQUARE
void SetPw(float pw)              // 0.0 - 1.0 (pulse width)  [control]
float Process()                                               // [audio]

// BlOsc — band-limited oscillator
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
void SetAmp(float amp)            // 0.0 - 1.0                [control]
void SetWaveform(uint8_t waveform)                            // [control]
  // ^ WAVE_SAW | WAVE_SQUARE | WAVE_TRI | WAVE_OFF
float Process()                                               // [audio]

// Fm2 — TWO-OPERATOR FM (⚠ uses SetFrequency not SetFreq)
void Init(float sample_rate)                                  // [init]
void SetFrequency(float freq)     // 20.0 - 20000.0 Hz        [control]
  // ^ ⚠ EXCEPTION: SetFrequency (not SetFreq) — only class with this name
void SetRatio(float ratio)        // 0.0 - 10.0               [control]
void SetIndex(float index)        // 0.0 - 12.0               [control]
float Process()                                               // [audio]

// Phasor — ramp generator (0.0 to 1.0)
void Init(float sample_rate, float freq)  // freq: 0-20000 Hz [init]
void SetFreq(float freq)                                      // [control]
float Process()                  // returns 0.0-1.0 ramp      // [audio]

// WhiteNoise — white noise generator
void Init()                                                    // [init]
void SetAmp(float amp)            // 0.0 - 1.0                [control]
float Process()                                               // [audio]
```

---

### Filters

```cpp
// MoogLadder — 24dB/oct Moog-style low-pass filter
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 10.0 - 20000.0 Hz        [control]
  // ^ ⚠ SetFreq, NOT SetCutoff NOT SetCutoffFrequency
void SetRes(float res)            // 0.0 - 1.0                [control]
  // ^ ⚠ SetRes, NOT SetQ NOT SetResonance
float Process(float in)                                       // [audio]
// Requires: USE_DAISYSP_LGPL = 1 in Makefile

// Svf — State Variable Filter (multimode: LP/HP/BP/Notch/Peak)
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 10.0 - 20000.0 Hz        [control]
void SetRes(float res)            // 0.0 - 1.0                [control]
void SetDrive(float drive)        // 0.0 - 1.0                [control]
void Process(float in)                                        // [audio]
  // ^ ⚠ VOID return! Query outputs:
float Low()                                                   // [audio]
float High()                                                  // [audio]
float Band()                                                  // [audio]
float Notch()                                                 // [audio]
float Peak()                                                  // [audio]

// Biquad — general purpose biquad filter
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
void SetRes(float res)            // 0.0 - 1.0                [control]
float Process(float in)                                       // [audio]

// Tone — low-pass filter (1-pole)
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
float Process(float in)                                       // [audio]

// ATone — high-pass filter (1-pole)
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
float Process(float in)                                       // [audio]
```

---

### Envelopes

```cpp
// Adsr — Attack-Decay-Sustain-Release envelope
void Init(float sample_rate)                                  // [init]
void SetTime(Adsr::SegIndex seg, float time) // time: 0.001-10.0 s [control]
  // ^ ⚠ SetTime with segment enum — NOT SetAttack / SetDecay / SetRelease
  // ^ seg: ADSR_SEG_ATTACK | ADSR_SEG_DECAY | ADSR_SEG_RELEASE
void SetSustainLevel(float level) // 0.0 - 1.0                [control]
float Process(bool gate)                                      // [audio]
bool IsRunning()                                              // [audio]

// AdEnv — Attack-Decay (two-segment envelope, no sustain)
void Init(float sample_rate)                                  // [init]
void SetTime(AdEnv::SegIndex seg, float time)                 // [control]
  // ^ seg: ADENV_SEG_ATTACK | ADENV_SEG_DECAY
void SetCurve(float shape)        // -1.0 - 1.0               [control]
void SetMax(float max)            // 0.0 - 1.0                [control]
void SetMin(float min)            // 0.0 - 1.0                [control]
float Process(bool trig)                                      // [audio]
```

---

### Effects

```cpp
// ReverbSc — stereo reverb (requires USE_DAISYSP_LGPL = 1)
void Init(float sample_rate)                                  // [init]
void SetFeedback(float feedback)  // 0.0 - 1.0                [control]
void SetLpFreq(float lpfreq)      // 0.0 - 20000.0 Hz         [control]
void Process(float in1, float in2, float *out1, float *out2)  // [audio]
  // ^ ⚠ Void return — outputs via pointer arguments

// Chorus
void Init(float sample_rate)                                  // [init]
void SetLfoFreq(float freq)       // 0.0 - 10.0 Hz            [control]
void SetLfoDepth(float depth)     // 0.0 - 1.0                [control]
void SetDelay(float delay)        // 0.0 - 1.0                [control]
float Process(float in)                                       // [audio]
float GetLeft()                                               // [audio]
float GetRight()                                              // [audio]

// Phaser
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 0.0 - 20000.0 Hz         [control]
void SetFeedback(float feedback)  // 0.0 - 1.0                [control]
void SetLfoDepth(float depth)     // 0.0 - 1.0                [control]
void SetLfoFreq(float lfofreq)    // 0.0 - 20.0 Hz            [control]
float Process(float in)                                       // [audio]

// Flanger
void Init(float sample_rate)                                  // [init]
void SetFeedback(float feedback)  // 0.0 - 1.0                [control]
void SetLfoFreq(float lfofreq)    // 0.0 - 20.0 Hz            [control]
void SetLfoDepth(float depth)     // 0.0 - 1.0                [control]
float Process(float in)                                       // [audio]

// Tremolo
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 0.01 - 100.0 Hz          [control]
void SetDepth(float depth)        // 0.0 - 1.0                [control]
void SetWaveform(uint8_t waveform)                            // [control]
float Process(float in)                                       // [audio]

// Overdrive — ⚠ No sample_rate in Init!
void Init()                                                    // [init]
void SetDrive(float drive)        // 0.0 - 1.0                [control]
float Process(float in)                                       // [audio]

// Decimator — ⚠ No sample_rate in Init!
void Init()                                                    // [init]
void SetDownsampleFactor(float factor) // 0.0 - 1.0           [control]
void SetBitDepth(float bitdepth)  // 1.0 - 32.0 bits          [control]
float Process(float in)                                       // [audio]

// Bitcrush
void Init(float sample_rate)                                  // [init]
void SetBitDepth(float bitdepth)  // 1.0 - 32.0 bits          [control]
void SetCrushRate(float crushrate)// 0.0 - 1.0                [control]
float Process(float in)                                       // [audio]

// PitchShifter
void Init(float sample_rate)                                  // [init]
void SetTranspose(float transpose)// -24.0 - 24.0 semitones   [control]
void SetDelSize(size_t size)                                   // [init]
float Process(float in)                                       // [audio]

// Compressor
void Init(float sample_rate)                                  // [init]
void SetThreshold(float threshold)// -80.0 - 0.0 dBFS         [control]
void SetRatio(float ratio)        // 1.0 - 40.0               [control]
void SetAttack(float attack)      // 0.001 - 10.0 s           [control]
void SetRelease(float release)    // 0.001 - 10.0 s           [control]
void SetMakeup(float makeup)      // 0.0 - 80.0 dB            [control]
float Process(float in)                                       // [audio]
void ProcessStereo(float inL, float inR, float *outL, float *outR) // [audio]
```

---

### Physical Modeling

```cpp
// StringVoice — Karplus-Strong style string model (requires LGPL)
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
void SetAccent(float accent)      // 0.0 - 1.0                [control]
void SetStructure(float structure)// 0.0 - 1.0                [control]
void SetBrightness(float brightness) // 0.0 - 1.0            [control]
void SetDamping(float damping)    // 0.0 - 1.0                [control]
float Process(bool trig)                                      // [audio]

// ModalVoice — resonant modal bank (requires LGPL)
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 20000.0 Hz        [control]
void SetAccent(float accent)      // 0.0 - 1.0                [control]
void SetStructure(float structure)// 0.0 - 1.0                [control]
void SetBrightness(float brightness) // 0.0 - 1.0            [control]
float Process(bool trig)                                      // [audio]
void Trig()                                                   // [audio]

// AnalogBassDrum
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 1000.0 Hz         [control]
void SetAccent(float accent)      // 0.0 - 1.0                [control]
void SetDecay(float decay)        // 0.0 - 1.0                [control]
void SetTone(float tone)          // 0.0 - 1.0                [control]
void SetAttackFmAmount(float amt) // 0.0 - 1.0                [control]
void SetSelfFmAmount(float amt)   // 0.0 - 1.0                [control]
float Process(bool trig)                                      // [audio]

// AnalogSnareDrum
void Init(float sample_rate)                                  // [init]
void SetFreq(float freq)          // 20.0 - 1000.0 Hz         [control]
void SetAccent(float accent)      // 0.0 - 1.0                [control]
void SetDecay(float decay)        // 0.0 - 1.0                [control]
void SetSnappy(float snappy)      // 0.0 - 1.0                [control]
void SetTone(float tone)          // 0.0 - 1.0                [control]
float Process(bool trig)                                      // [audio]
```

---

### Utilities

```cpp
// Metro — metronome trigger
void Init(float freq, float sample_rate)  // freq: 0.01-1000 Hz [init]
  // ^ ⚠ freq is FIRST argument (reversed from most DaisySP Init signatures)
void SetFreq(float freq)                                      // [control]
bool Process()           // returns true on tick only         // [audio]

// CrossFade
void Init()                                                    // [init]
void SetPos(float pos)            // 0.0=full A, 1.0=full B   [control]
void SetCurve(CrossFade::Type type)                           // [control]
  // ^ CROSSFADE_LIN | CROSSFADE_CPOW
float Process(float in1, float in2)                           // [audio]

// Port — slew limiter / portamento
void Init(float sample_rate, float htime)                     // [init]
void SetHtime(float htime)        // 0.0 - 1.0 (half-time)   [control]
float Process(float in)                                       // [audio]

// DcBlock — removes DC offset
void Init(float sample_rate)                                  // [init]
float Process(float in)                                       // [audio]
```

---

## Standard Code Template (Daisy Field)

```cpp
#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

DaisyField        hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay  display;

// ── DSP Modules ─────────────────────────────────────────────
// Declare here; never inside AudioCallback

// ── Parameters (shared between main loop and callback) ──────
volatile float param_freq    = 440.0f;  // volatile = safe cross-thread read
volatile float param_cutoff  = 2000.0f;
volatile float param_res     = 0.3f;

// ── Control Processing (main loop ONLY) ─────────────────────
void ProcessKnobs() {
    param_freq   = 20.0f + hw.knob[0].Process() * 4000.0f;
    param_cutoff = 100.0f + hw.knob[1].Process() * 10000.0f;
    param_res    = hw.knob[2].Process();
    // Update DSP with smoothed values
    // osc.SetFreq(param_freq);    // ← do this inside AudioCallback instead
}

// ── Audio Callback ───────────────────────────────────────────
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
    // Audio processing ONLY — no control reads, no alloc, no prints
    for(size_t i = 0; i < size; i++) {
        // Per-sample DSP here
        float sig = 0.0f;
        out[0][i] = out[1][i] = sig;
    }
}

// ── Main ─────────────────────────────────────────────────────
int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sr = hw.AudioSampleRate();

    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Project Name");

    // Initialize DSP — MUST be here, before StartAudio
    // osc.Init(sr);
    // filter.Init(sr);

    hw.StartAdc();           // Before StartAudio!
    hw.StartAudio(AudioCallback);

    while(1) {
        hw.ProcessAllControls();   // ← here, NOT in callback
        ProcessKnobs();

        keyLeds.Update();
        display.Update();
        System::Delay(16);         // ~60Hz
    }
}
```

---

## Makefile Checklist

```makefile
TARGET     = ProjectName
CPP_SOURCES = ProjectName.cpp

LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR  = ../../DaisySP

USE_DAISYSP_LGPL = 1   # Required for: ReverbSc, MoogLadder, ModalVoice, StringVoice

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
```

---

## Daisy Field Hardware (Confirmed Mappings)

```cpp
// Keyboard input indices
// A-row: hw.KeyboardRisingEdge(0)  to  hw.KeyboardRisingEdge(7)
// B-row: hw.KeyboardRisingEdge(8)  to  hw.KeyboardRisingEdge(15)

// LED output indices (ASYMMETRIC — common bug source!)
// A-row LEDs: index 15 (=A1) down to 8 (=A8)  ← REVERSED
// B-row LEDs: index 0  (=B1) up to  7 (=B8)   ← sequential
// Knob LEDs:  indices 16-23
// Switch LEDs: indices 24-25
```

---

## DAFX_2_Daisy_lib API Reference

> **Note:** The DAFX_2_Daisy_lib is a companion DSP library in the DVPE ecosystem.
> All DAFX classes use the `daisysp::` namespace and follow the same `Init` / `Process` pattern as DaisySP.
> Method names have been **normalized to match DaisySP conventions** (e.g. `SetFreq` not `SetFrequency`).

### DAFX Filters

```cpp
// HighShelving — high-frequency shelving EQ
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 20.0-20000.0 Hz      [control]
void SetGain(const float &gain)  // -20.0 to +20.0 dB   [control]
float Process(const float &in)                           // [audio]

// LowShelving — low-frequency shelving EQ
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 20.0-20000.0 Hz      [control]
void SetGain(const float &gain)  // -20.0 to +20.0 dB   [control]
float Process(const float &in)                           // [audio]

// PeakFilter — parametric EQ band
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 20.0-20000.0 Hz      [control]
void SetBandwidth(const float &bw) // 10.0-10000.0 Hz   [control]
void SetGain(const float &gain)  // -20.0 to +20.0 dB   [control]
float Process(const float &in)                           // [audio]
```

### DAFX Effects

```cpp
// WahWah — modulated bandpass wah effect
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 200.0-2000.0 Hz      [control]
  // ^ ⚠ SetFreq (NOT SetFrequency — old DAFX name removed)
void SetRes(const float &res)    // 1.0-20.0             [control]
  // ^ ⚠ SetRes (NOT SetQ — old DAFX name removed)
void SetDepth(const float &depth)// 0.0-1.0              [control]
float Process(const float &in)                           // [audio]

// ToneStack — guitar amplifier tone stack (bass/mid/treble)
void Init(float sample_rate)                              // [init]
void SetBass(const float &bass)  // -1.0 to +1.0         [control]
void SetMiddle(const float &middle) // -1.0 to +1.0      [control]
void SetTreble(const float &treble) // -1.0 to +1.0      [control]
float Process(const float &in)                           // [audio]

// Tube — asymmetric tube distortion / waveshaper
void Init(float sample_rate)                              // [init]
void SetDrive(const float &drive)                        [control]
void SetBias(const float &bias)                          [control]
void SetDistortion(const float &dist)                    [control]
void SetHighPassPole(const float &rh)                    [control]
void SetLowPassPole(const float &rl)                     [control]
void SetMix(const float &mix)    // 0.0-1.0              [control]
float Process(const float &in)                           // [audio]

// FDNReverb — 4-channel feedback delay network reverb
void Init(float sample_rate)                              // [init]
void SetDecay(float decay)       // 0.9-0.999            [control]
void SetMix(float mix)           // 0.0-1.0              [control]
void SetDamping(float damping)   // 0.0-0.99             [control]
void SetDelayScale(float scale)  // 0.1-4.0 (room size)  [control]
void SetReverbTime(float rt60)   // RT60 in seconds      [control]
float Process(float in)                                  // [audio]
void ProcessStereo(float in_l, float in_r, float *out_l, float *out_r) // [audio]
void Clear()                     // remove reverb tail   [control]
```

### DAFX Modulation

```cpp
// Vibrato — modulated delay-line vibrato
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 0.1-20.0 Hz          [control]
  // ^ ⚠ SetFreq (NOT SetFrequency — old DAFX name removed)
void SetWidth(const float &width)// 0.0001-0.1 s         [control]
  // ^ width is modulation depth in seconds
float Process(const float &in)                           // [audio]

// RingModulator — ring modulation (input × sine carrier)
void Init(float sample_rate)                              // [init]
void SetFreq(const float &freq)  // 1.0-10000.0 Hz       [control]
  // ^ ⚠ SetFreq (NOT SetFrequency — old DAFX name removed)
void SetDepth(const float &depth)// 0.0-1.0              [control]
float Process(const float &in)                           // [audio]
```

### DAFX Dynamics

```cpp
// NoiseGate — threshold gate with hold/attack/release
void Init(float sample_rate)                              // [init]
void SetThreshold(const float &thresh_db) // -60.0-0.0 dB [control]
void SetHoldTime(const float &hold_time)  // 0.001-1.0 s  [control]
void SetAttackTime(const float &attack_time) // 0.0001-0.1 s [control]
void SetReleaseTime(const float &release_time) // 0.001-1.0 s [control]
void SetAlpha(const float &alpha) // 0.0-1.0 (smoothing)  [control]
float Process(const float &in)                           // [audio]

// CompressorExpander — RMS compressor + downward expander with lookahead
void Init(float sample_rate)                              // [init]
  // ^ Template: CompressorExpander<N> (default N=256 lookahead samples)
void SetCompThreshold(float threshold_db)                [control]
void SetCompRatio(float ratio)   // 1.0+ e.g. 4.0 = 4:1  [control]
void SetExpThreshold(float threshold_db)                 [control]
void SetExpRatio(float ratio)    // 1.0+                  [control]
void SetAttackTime(float time_sec) // 0.001-1.0 s        [control]
void SetReleaseTime(float time_sec)// 0.001-5.0 s        [control]
void SetLookahead(size_t samples)                        [control]
float Process(float in)                                  // [audio]
float GetCurrentGainDb()         // gain reduction in dB  [audio]
```

### DAFX Utility

```cpp
// EnvelopeFollower — peak or RMS envelope detector
void Init(float sample_rate)                              // [init]
void SetAttackTime(float attack_time)  // 0.0001-1.0 s   [control]
void SetReleaseTime(float release_time)// 0.001-5.0 s    [control]
void SetMode(EnvelopeMode mode)                           [control]
  // ^ EnvelopeMode::Peak (fast) | EnvelopeMode::RMS (smoother)
float Process(float in)          // returns linear envelope [audio]
float ProcessDB(float in)        // returns envelope in dB  [audio]
void Reset()                                              [control]

// YinPitchDetector — YIN algorithm monophonic pitch detector
void Init(float sample_rate)                              // [init]
  // ^ Template: YinPitchDetector<N> (default N=1024). Aliases: Yin1024, Yin2048
  // ^ CPU cost: ~10-15% at 48kHz
void SetTolerance(float tolerance)  // 0.1-0.5 (default 0.15) [control]
void SetFrequencyRange(float f0_min, float f0_max)        [control]
bool ProcessSample(float sample)    // returns true on new frame [audio]
  // ^ Streaming mode: call every sample, check IsVoiced()/GetFrequency() when true
float Process(const float *input)   // block mode, returns Hz   [audio]
float GetFrequency() const          // Hz, or 0 if unvoiced    [control]
float GetConfidence() const         // 0-1 detection confidence  [control]
bool  IsVoiced() const                                    [control]
float GetMidiNote() const           // fractional MIDI note, or -1 [control]
```

### DAFX Makefile Additions

```makefile
# Add DAFX_2_Daisy_lib to your project Makefile:
C_INCLUDES += -I../../MyProjects/DAFX_2_Daisy_lib/src

# DAFX source files (compiled separately):
CPP_SOURCES += ../../MyProjects/DAFX_2_Daisy_lib/src/effects/wahwah.cpp \
               ../../MyProjects/DAFX_2_Daisy_lib/src/filters/peakfilter.cpp
# (add only what you use)
```

---

*Source: DaisyExamples/DAISY_QAE/ — Part of the Daisy Quality Assurance Ecosystem.*  
*Always cross-reference against the DaisySP source headers for latest signatures.*
