# Simple FM Synthesizer for Daisy Field

**Platform**: Electrosmith Daisy Field  
**Architecture**: Monophonic FM Synth with Chorus  
**Signal Path**: Fm2 → VCA → Chorus → Output  
**Date**: 2026-01-04

---

## Overview

A simple, monophonic FM synthesizer featuring:
- **Fm2 synthesis core** (2-operator FM)
- **ADSR envelope** controlling VCA (percussive mode)
- **Chorus effect** for stereo width and shimmer
- **8-knob control** with real-time OLED visualization
- **Quantized ratio** (floor function for harmonic overtones)

Based on `Simple_FM_Chorus_Diagrams.md` specifications.

---

## Signal Flow

```
Keyboard Trigger
      ↓
   ADSR Envelope
      ↓ (gate)
      │
K1 ──┼──► Fm2 Synth ───► FM Audio ───► VCA (×) ───► Shaped Audio
K2 ──┤                                   ▲
K3 ──┘                                   │
                                    Env Signal
      │
      ▼
K5 ──┐
K6 ──┼──► Chorus Effect ───► Chorused Audio ───► Output (L/R)
K7 ──┤
K8 ──┘
```

---

## Controls

### Knobs

| Knob | Parameter | Range | Description |
|------|-----------|-------|-------------|
| **K1** | Frequency | 20-2000 Hz | Base frequency of FM carrier |
| **K2** | Ratio | 0.5-8.0 (floor) | Modulator frequency ratio (quantized) |
| **K3** | FM Index | 0-10 | Modulation depth |
| **K4** | Decay | 0.01-2.0 s | ADSR decay time |
| **K5** | Chorus Depth | 0-1.0 | LFO modulation depth |
| **K6** | Chorus Rate | 0.1-5 Hz | LFO frequency |
| **K7** | Chorus Delay | 5-50 ms | Delay line time |
| **K8** | Chorus Feedback | 0-0.9 | Feedback amount |

### Keyboard

| Keys | Function |
|------|----------|
| **Any key** | Trigger note (gate on) |
| **Release all** | Gate off |

**Note**: This is a simple trigger version. Any key press triggers the envelope.

---

## OLED Display

### Default View
```
FM Simple
Freq: 440Hz
Ratio: 2:1
Index: 5.0
Gate: OFF
```

### Zoomed View (1.2s on knob change)
```
FM Index

50% (5.0)

▓▓▓▓▓▓▓▓░░░░
```

---

## Architecture Details

### FM2 Synthesizer
- **Carrier**: Base oscillator at `K1` frequency
- **Modulator**: Oscillator at `K1 × floor(K2)` frequency
- **Algorithm**: Modulator → Carrier (classic FM)
- **Ratio Quantization**: floor() ensures integer ratios (harmonic)

### ADSR Envelope
- **Attack**: 5ms (fixed, percussive)
- **Decay**: Variable (K4: 0.01-2.0s)
- **Sustain**: 0% (fixed, percussive)
- **Release**: 100ms (fixed)

### Chorus Effect
- **Type**: Stereo chorus with LFO modulation
- **Delay Line**: Single delay with feedback
- **LFO**: Sine wave modulation
- **Mix**: 50% dry / 50% wet (internal)

---

## Build Instructions

### Compile
```bash
cd MyProjects/fm_synth_field_simple
make clean && make
```

### Program to Field
```bash
make program-dfu
```

---

## Example Patches

### Patch 1: Classic FM Bell
```
K1 (Freq):     50% (500 Hz)
K2 (Ratio):    38% (3:1)     ← floor(3.x) = 3
K3 (Index):    50% (5.0)
K4 (Decay):    30% (0.6s)
K5 (Depth):    40%
K6 (Rate):     20% (1 Hz)
K7 (Delay):    30% (18ms)
K8 (Feedback): 30%

Result: Bell-like tone with chorus shimmer
```

### Patch 2: Percussive Bass
```
K1 (Freq):     10% (100 Hz)
K2 (Ratio):    13% (1:1)     ← floor(1.x) = 1
K3 (Index):    80% (8.0)
K4 (Decay):    5% (0.1s)     ← Very short!
K5 (Depth):    10%           ← Minimal chorus
K6 (Rate):     10%
K7 (Delay):    10% (9ms)
K8 (Feedback): 0%

Result: Tight, punchy bass hit
```

### Patch 3: Ambient Pad
```
K1 (Freq):     25% (250 Hz)
K2 (Ratio):    50% (4:1)     ← floor(4.x) = 4
K3 (Index):    40% (4.0)
K4 (Decay):    100% (2.0s)   ← Long decay
K5 (Depth):    80%           ← Deep chorus
K6 (Rate):     40% (2 Hz)
K7 (Delay):    70% (35ms)
K8 (Feedback): 60%

Result: Lush, evolving synth pad
```

### Patch 4: Harmonic Organ
```
K1 (Freq):     40% (400 Hz)
K2 (Ratio):    25% (2:1)     ← floor(2.x) = 2 (octave)
K3 (Index):    20% (2.0)     ← Low index
K4 (Decay):    60% (1.2s)
K5 (Depth):    50%
K6 (Rate):     15% (0.8 Hz)
K7 (Delay):    40% (22ms)
K8 (Feedback): 40%

Result: Organ-like tone with movement
```

---

## Technical Details

### Ratio Quantization (floor function)

The `K2` knob implements integer quantization:

```cpp
float ratio_raw = knob_value * 7.5 + 0.5;  // Range: 0.5 - 8.0
float ratio = floor(ratio_raw);            // Quantize: 0, 1, 2, 3, ...8
```

**Why?** Integer ratios produce **harmonic overtones** (musical intervals):
- 1:1 = Fundamental
- 2:1 = Octave
- 3:1 = Octave + Fifth
- 4:1 = Two octaves
- 5:1 = Two octaves + Major third

### DSP Chain

```cpp
// Per sample:
float fm_out = fm_synth.Process();           // 1. FM synthesis
float env_val = envelope.Process(gate);      // 2. ADSR envelope
float vca_out = fm_out * env_val;            // 3. VCA
float chorus_out = chorus_fx.Process(vca_out); // 4. Chorus
out[0][i] = out[1][i] = chorus_out;          // 5. Output
```

---

## Memory Usage

Expected build stats:
- **Flash**: ~30-40 KB (simple architecture)
- **RAM**: ~10-15 KB (no polyphony)
- **CPU**: ~20-30% (monophonic + chorus)

---

## Troubleshooting

### No Sound
- Check K1 (frequency) is not at minimum
- Check K4 (decay) is not too short
- Trigger with keyboard

### Too Much Distortion
- Reduce K3 (FM index)
- Reduce K8 (chorus feedback)

### No Chorus Effect
- Increase K5 (chorus depth)
- Increase K7 (delay time)

### Ratio Not Changing
- Remember: K2 uses floor() - only integer values
- Values quantized to: 0, 1, 2, 3, 4, 5, 6, 7, 8

---

## Code Structure

```cpp
// Main modules
Fm2    fm_synth;     // FM synthesis
Adsr   envelope;     // Envelope generator
Chorus chorus_fx;    // Chorus effect

// Control flow
AudioCallback() {
    1. Read 8 knobs
    2. Update Fm2 (freq, ratio, index)
    3. Update ADSR (decay)
    4. Update Chorus (4 params)
    5. Process audio chain
}

// Display (60 FPS)
CheckParameterChanges()  // Detect knob changes
UpdateOLED()             // Zoom or default view
```

---

## Future Enhancements

### Possible Additions
- [ ] Multiple trigger modes (hold, retrigger)
- [ ] Velocity sensitivity
- [ ] Pitch tracking from keyboard
- [ ] LFO for vibrato
- [ ] Second envelope for filter
- [ ] Preset storage

---

## Credits

- **Developer**: DK + Lumina (ESA)
- **Platform**: Electrosmith Daisy Field
- **Libraries**: DaisySP, libDaisy
- **Based On**: Simple_FM_Chorus_Diagrams.md
- **Date**: 2026-01-04

---

## License

MIT License (same as DaisySP/libDaisy)

---

**Enjoy simple FM synthesis with chorus!** 🎹✨
