# Daisy Platform Development Standards

> **Rule #1**: Development standards document FIRST, code AFTER logic.

This document establishes the required workflow for Daisy projects (Field, Pod, and Seed). Following this process prevents "AI-slop" and ensures working, maintainable code.

---

## 1. Development Workflow (Mandatory Steps)

```mermaid
flowchart LR
    A["1. CONCEPT"] --> B["2. BLOCK DIAGRAMS"]
    B --> C["3. CONTROLS.md"]
    C --> D["4. IMPLEMENTATION"]
    D --> E["5. VERIFY & ITERATE"]
    
    style A fill:#43A047,stroke:#1B5E20,color:#fff
    style B fill:#1E88E5,stroke:#0D47A1,color:#fff
    style C fill:#FB8C00,stroke:#E65100,color:#000
    style D fill:#8E24AA,stroke:#4A148C,color:#fff
    style E fill:#E53935,stroke:#B71C1C,color:#fff
```

### Step 1: CONCEPT
- Define the synth/effect purpose in 1-2 sentences
- List required DaisySP/DAFX algorithms
- Assign complexity rating (1-10)

### Step 2: BLOCK DIAGRAMS (3 Diagrams Required)
| Diagram | Purpose | Mermaid Type |
|---------|---------|--------------|
| **A. System Architecture** | Module hierarchy & relationships | `block-beta` |
| **B. Signal Flow Graph** | Audio DSP processing chain | `flowchart LR` |
| **C. Control Flow** | Event handling & UI logic | `flowchart TD` |

### Step 3: CONTROLS.md
- Parameter mapping table (all 8 knobs)
- Key assignments (A1-A8, B1-B8)
- Switch functions (SW1, SW2)
- Preset definitions with visual charts

### Step 4: IMPLEMENTATION
- Use `field_defaults.h` for LED/display helpers
- Keep control processing in main loop (NOT audio callback)
- Initialize all DSP modules with `Init(sample_rate)`
- **Start with a loaded preset** - initialize all parameters, filters, and smoothed values from preset defaults to avoid edge cases

### Step 5: VERIFY & ITERATE
- `make clean && make` → Exit 0
- Flash via ST-Link → Test all controls
- Update CONTROLS.md with any changes

---

## 2. Available DSP Components

### DaisySP Core Modules
| Category | Modules |
|----------|---------|
| **Oscillators** | `Oscillator`, `VariableShapeOsc`, `VariableSawOsc`, `OscillatorBank`, `FormantOsc`, `Phasor` |
| **Filters** | `Svf`, `OnePole`, `Biquad`, `NlFilt`, `Comb` |
| **Envelopes** | `Adsr`, `AdEnv`, `Line`, `Phasor` |
| **Effects** | `Chorus`, `Flanger`, `Phaser`, `Tremolo`, `Overdrive`, `Decimator` |
| **Reverb** | `ReverbSc` (requires LGPL flag) |
| **Modulation** | `Lfo` (uses Phasor internally), `Metro` |
| **Physical Modeling** | `ModalVoice`, `StringVoice`, `Pluck`, `Drip`, `Resonator` |
| **Utilities** | `Limiter`, `Compressor`, `CrossFade`, `DcBlock` |

### DAFX_2_Daisy_library (28 Modules)
| Category | Modules |
|----------|---------|
| **Effects** | `fdn_reverb`, `tube`, `wahwah`, `vibrato`, `tonestack`, `universal_comb` |
| **Filters** | `highshelving`, `lowshelving`, `peakfilter` |
| **Dynamics** | `compressor_expander`, `noisegate` |
| **Spectral** | `phase_vocoder`, `robotization`, `whisperization`, `spectral_filter` |
| **Modulation** | `ringmod`, `vibrato` |
| **Analysis** | `yin`, `envelopefollower` |
| **Utility** | `circularbuffer`, `fft_handler`, `windows`, `xcorr` |

---

## 3. Code Structure Template

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

// DSP Modules (declare here)
// [Your oscillators, filters, effects]

// Parameters
struct Params {
    float param1 = 0.5f;
    // ...all 8 knobs
} params;

void ProcessKnobs() {
    // Read knobs and update DSP parameters
    params.param1 = hw.knob[0].Process();
    // Update DSP modules with smoothed values
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
    // ONLY audio processing here - NO control reads
    for(size_t i = 0; i < size; i++) {
        float sig = 0.0f;
        // Process DSP chain
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    float sr = hw.AudioSampleRate();

    // Initialize helpers
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("Project Name");

    // Initialize DSP modules with sample rate
    // module.Init(sr);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1) {
        hw.ProcessAllControls();
        ProcessKnobs();
        
        // Handle keyboard input
        // Handle switches
        
        keyLeds.Update();
        display.Update();
        System::Delay(16);
    }
}
```

---

## 4. Common Pitfalls to Avoid

| ❌ Don't | ✅ Do |
|---------|-------|
| Call `hw.ProcessAllControls()` in audio callback | **Field**: Keep control processing in main loop. **Pod (OLED)**: Split Digital into 1kHz Timer ISR, Analog into AudioCallback. |
| Update `prev_value` unconditionally per frame | Implement proper hysteresis: only update `prev_value` when the deadband threshold is broken. |
| Use uninitialized DSP modules | Call `module.Init(sample_rate)` for all modules |
| Mix control and audio threads unsafely | Use atomic variables or simple flags for thread communication |
| Skip block diagrams | Create all 3 diagrams before writing code |
| Use `std::function` or heavy STL | Use simple C-style callbacks and arrays |

---

## 5. Platform Adaptation (Seed / Pod / Field)

The code template in Section 3 targets **Daisy Field**. When targeting Pod or Seed, these differences apply:

### Audio Buffer Format

| Platform | Buffer Type | Callback Signature | Sample Access |
|----------|-------------|-------------------|---------------|
| **Field** | Non-interleaved | `InputBuffer in, OutputBuffer out, size_t size` | `out[0][i]` (L), `out[1][i]` (R) |
| **Pod** | Interleaved | `float *in, float *out, size_t size` | `out[i*2]` (L), `out[i*2+1]` (R) |
| **Seed** | Interleaved | `float *in, float *out, size_t size` | `out[i*2]` (L), `out[i*2+1]` (R) |

### Pod Template (Key Differences)

```cpp
#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

void AudioCallback(float *in, float *out, size_t size) {
    // For Pods with I2C OLEDs stalling the main loop:
    // ONLY process analog controls here so the filter time constant matches AudioBlockRate.
    hw.ProcessAnalogControls();  
    
    for(size_t i = 0; i < size; i += 2) {
        float sig = 0.0f;
        // Process DSP
        out[i]     = sig;  // Left
        out[i + 1] = sig;  // Right
    }
}

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sr = hw.AudioSampleRate();

    // Configure 1kHz Timer ISR for digital debouncing if using heavy main-loop displays:
    // void ControlTimerCallback(void* data) { hw.ProcessDigitalControls(); }

    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) { 
        // Read hw.knob1.Value() here (NOT Process()) and apply hysteresis.
    }  
}
```

### Seed Template (Key Differences)

```cpp
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisySeed hw;

void AudioCallback(float *in, float *out, size_t size) {
    for(size_t i = 0; i < size; i += 2) {
        float sig = 0.0f;
        out[i]     = sig;
        out[i + 1] = sig;
    }
}

int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sr = hw.AudioSampleRate();

    // Seed: manual ADC config required
    // AdcChannelConfig adc_conf[2];
    // adc_conf[0].InitSingle(seed::A0);
    // hw.adc.Init(adc_conf, 2);

    hw.StartAudio(AudioCallback);
    while(1) { }
}
```

### Platform Selection Checklist

| Feature Needed | Use |
|----------------|-----|
| 8 knobs, 16-key keyboard, OLED | **Field** |
| 2 knobs, encoder, 2 RGB LEDs | **Pod** |
| Maximum GPIO flexibility, custom hardware | **Seed** |
| `field_defaults.h` helpers | **Field only** |
| Interleaved audio buffers | **Pod / Seed** |

---

## 6. OLED Parameter Visualization (Field)

Standard pattern for responsive knob feedback on the Field's 128x64 OLED display.

### Change Detection

```cpp
float prevKnob[8], currKnob[8];
int   zoomParam = -1;
uint32_t zoomStartTime = 0;

void CheckParameterChanges() {
    for(int i = 0; i < 8; i++) {
        currKnob[i] = hw.knob[i].Process();
        if(fabsf(currKnob[i] - prevKnob[i]) > 0.02f) {
            zoomParam = i;
            zoomStartTime = System::GetNow();
            prevKnob[i] = currKnob[i];
        }
    }
    // Auto-dismiss after 1.2 seconds of inactivity
    if(System::GetNow() - zoomStartTime > 1200)
        zoomParam = -1;
}
```

### Zoom Display with Progress Bar

```cpp
const char* kParamNames[8] = {
    "Freq", "Cutoff", "Resonance", "Attack",
    "Decay", "Mix", "Drive", "Level"
};

void DrawZoomedParameter() {
    if(zoomParam < 0) return;

    float val = currKnob[zoomParam];
    int percent = (int)(val * 100.f);

    // Line 1: Parameter name
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kParamNames[zoomParam], Font_11x18, true);

    // Line 2: Value with units
    char buf[32];
    sprintf(buf, "%d%%", percent);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(buf, Font_11x18, true);

    // Line 3: Progress bar (full width = 128px)
    hw.display.DrawRect(0, 50, (int)(val * 127.f), 58, true, true);
}
```

### Integration in Main Loop

```cpp
while(1) {
    hw.ProcessAllControls();
    CheckParameterChanges();

    // Normal display or zoom overlay
    display.StartDraw();
    if(zoomParam >= 0)
        DrawZoomedParameter();
    else
        display.DrawNormal();  // Your default screen
    display.EndDraw();

    keyLeds.Update();
    System::Delay(16);
}
```

> **Cross-reference**: This pattern is also documented in the `DAISY_EXPERT_SP_v5.2` system prompt. The `FieldOLEDDisplay` class in `field_defaults.h` provides a simpler auto-highlighting API if you prefer.

---

## 7. Thread Safety Patterns

The main loop (~60Hz) and AudioCallback (~1kHz at 48 samples/48kHz) run on the same core but at different interrupt priorities. The audio callback can preempt the main loop at any point.

### When `volatile` Is Sufficient

For simple flags and single-word parameter updates, `volatile` prevents the compiler from caching stale values:

```cpp
volatile bool  gate_on = false;     // Set in main loop, read in callback
volatile float target_freq = 440.f; // Set in main loop, read in callback
```

This works on Cortex-M7 for naturally-aligned 32-bit reads/writes because they are atomic at the hardware level.

### When You Need More: Memory Barriers

The STM32H750 has a D-cache. If you're passing multi-field structs between contexts, the compiler or cache can reorder operations:

```cpp
// UNSAFE: compiler/cache may reorder these writes
params.freq = new_freq;
params.resonance = new_res;
params_ready = true;  // Callback might see this before freq/res are updated

// SAFE: force all prior writes to complete
params.freq = new_freq;
params.resonance = new_res;
__DSB();               // Data Synchronization Barrier — flushes write buffer
params_ready = true;
```

| Barrier | Use Case |
|---------|----------|
| `__DSB()` | Ensure all memory writes complete before continuing |
| `__DMB()` | Ensure memory access ordering (lighter than DSB) |
| `__ISB()` | Flush instruction pipeline (rarely needed) |

### Recommended: Double-Buffered Parameters

For updating multiple parameters atomically without locks:

```cpp
struct ParamSet {
    float freq;
    float cutoff;
    float resonance;
    float level;
};

ParamSet param_buf[2];        // Two copies
volatile int active_buf = 0;  // Which buffer the callback reads

// Main loop: write to inactive buffer, then swap
void UpdateParams() {
    int write_buf = 1 - active_buf;
    param_buf[write_buf].freq      = 20.f + hw.knob[0].Process() * 2000.f;
    param_buf[write_buf].cutoff    = 100.f + hw.knob[1].Process() * 10000.f;
    param_buf[write_buf].resonance = hw.knob[2].Process();
    param_buf[write_buf].level     = hw.knob[3].Process();
    __DSB();
    active_buf = write_buf;  // Atomic pointer swap
}

// Audio callback: read from active buffer (never writes to it)
void AudioCallback(...) {
    const ParamSet& p = param_buf[active_buf];
    osc.SetFreq(p.freq);
    filter.SetFreq(p.cutoff);
    // ...
}
```

### When Simple `volatile` Fails

| Pattern | Safe? | Why |
|---------|-------|-----|
| Single `volatile float` | Yes | 32-bit aligned write is atomic on Cortex-M7 |
| `volatile bool` flag | Yes | Single-byte, naturally atomic |
| Updating 3 floats then a flag | Risky | Callback may read partial updates |
| Ring buffer with single producer/consumer | Yes | If head/tail are `volatile` and you use `__DSB()` |
| Anything with `malloc`/STL | No | Not async-signal-safe |

### Practical Rule of Thumb

For most Daisy projects, simple `volatile` floats per-parameter work fine. Use double buffering only when you need multiple parameters to change as a group (e.g., preset recall, multi-parameter modulation).

---

## 8. File Organization

```
MyProjects/_projects/ProjectName/
├── ProjectName.cpp          # Main implementation
├── Makefile                  # Build configuration
├── README.md                 # Project overview
├── CONTROLS.md              # Detailed control documentation
├── diagrams/
│   ├── system_architecture.svg
│   ├── signal_flow.svg
│   └── control_flow.svg
└── presets/                 # Optional preset files
```

---

## 9. Required Makefile Template

```makefile
TARGET = ProjectName
CPP_SOURCES = ProjectName.cpp

LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

USE_DAISYSP_LGPL = 1  # Enable for ReverbSc, etc.

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
```

---

## References

- [field/Midi/Midi.cpp](../field/Midi/Midi.cpp) - Base project for external MIDI-oriented Field projects
- [field_wavetable_morph_synth](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/field_wavetable_morph_synth/) - Reference implementation
- [field_defaults.h](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/foundation_examples/field_defaults.h) - Hardware helper library
- [DAFX_2_Daisy_library](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/DAFX_2_Daisy_lib/) - Extended DSP effects

---

## Quality Assurance Ecosystem

This document is part of an interconnected quality assurance system:

| Document | Purpose | When to Use |
|----------|---------|-------------|
| [DAISY_TUTORIALS_KNOWLEDGE.md](DAISY_TUTORIALS_KNOWLEDGE.md) | Official API reference | Understanding GPIO/Audio/ADC |
| [DAISY_DEBUG_STRATEGY.md](DAISY_DEBUG_STRATEGY.md) | Serial/hardware debugging | When something isn't working |
| [DAISY_TECHNICAL_REPORT.md](DAISY_TECHNICAL_REPORT.md) | Complete process documentation | Deep reference, onboarding |
| [DAISY_BUGS.md](DAISY_BUGS.md) | Bug tracking, investigation methodology | Documenting issues |

**This document's role**: Defines the development workflow and code standards. Follow these patterns to maximize first-attempt success.

---

**Document Version**: 1.2
**Last Updated**: 2026-02-08

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.2 | 2026-02-08 | Added Platform Adaptation (Sec 5), OLED Visualization (Sec 6), Thread Safety Patterns (Sec 7); renamed from "Field" to "Platform" scope |
| 1.1 | 2026-02-08 | Added common pitfalls table, file organization, Makefile template |
| 1.0 | 2026-02-08 | Initial version: workflow, DSP catalog, code template |
