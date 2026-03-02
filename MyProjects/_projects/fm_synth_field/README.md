# FM Synthesizer for Daisy Field

**Platform**: Electrosmith Daisy Field  
**Architecture**: 8-Voice Polyphonic FM Synthesizer  
**Algorithms**: 8 FM algorithms (Stack, Parallel, Bell, Brass, E-Piano, etc.)  
**Date**: 2026-01-04

---

## Overview

A polyphonic FM synthesizer featuring:
- **8-voice polyphony** with voice allocation
- **8 FM algorithms** ranging from classic stacks to harmonic organ tones
- **4 FM operators** per voice (using 2× Fm2 modules)
- **ADSR envelopes** per operator
- **SVF filter** per voice
- **OLED visualization** with parameter zoom (sequencer_pod pattern)

Based on:
- **Field Midi.cpp** VoiceManager template
- **sequencer_pod** OLED visualization pattern
- **FM_architetcture.png** synthesis architecture

---

## Controls

### Knobs

| Knob | Parameter | Range | Description |
|------|-----------|-------|-------------|
| **K1** | OP1 Level | 0.0-1.0 | Operator 1 output amplitude |
| **K2** | OP2 Level | 0.0-1.0 | Operator 2 output amplitude |
| **K3** | OP3 Level | 0.0-1.0 | Operator 3 output amplitude |
| **K4** | OP4 Level | 0.0-1.0 | Operator 4 output amplitude |
| **K5** | Mod Index | 0.0-10.0 | FM modulation depth |
| **K6** | Algorithm | 0-7 | FM routing algorithm |
| **K7** | Filter Cutoff | 20Hz-20kHz | SVF filter frequency |
| **K8** | Resonance | 0.0-1.0 | SVF filter resonance |

### Switches

| Switch | Function |
|--------|----------|
| **SW1** | Octave Down (-1 octave) |
| **SW2** | Octave Up (+1 octave) |

### Keyboard

| Keys | Function | Notes |
|------|----------|-------|
| **Keys 0-15** | Chromatic keyboard | 16-note chromatic scale |
| **Octave Range** | C3 to C5 (±2 octaves) | Controlled by SW1/SW2 |

---

## FM Algorithms

### Algorithm 0: Stack
Classic cascaded FM (OP1→OP2→OP3→OP4)
- **Use**: Electric pianos, bells
- **Timbre**: Rich, complex harmonics

### Algorithm 1: Parallel
All operators directly to output
- **Use**: Additive synthesis, organs
- **Timbre**: Bright, harmonic

### Algorithm 2: 2+2 Split
Two FM pairs mixed
- **Use**: Dual timbres, layered sounds
- **Timbre**: Rich, chorused

### Algorithm 3: Harmonic (Organ)
Fixed harmonic ratios (1:1, 2:1)
- **Use**: Organ tones, pads
- **Timbre**: Warm, full

### Algorithm 4: Bell
1.4:1 and 3.5:1 ratios (DX7-style)
- **Use**: Metallic bells, mallets
- **Timbre**: Bright, inharmonic

### Algorithm 5: Brass
High modulation index stack
- **Use**: Brass, reeds
- **Timbre**: Sharp attack, buzzy

### Algorithm 6: E-Piano
Standard FM piano with high harmonic
- **Use**: Electric piano, vibes
- **Timbre**: Percussive, tine-like

### Algorithm 7: Bass
Very high modulation index
- **Use**: Bass, sub-bass
- **Timbre**: Deep, growling

---

## OLED Display

### Default View
```
FM Synth
Algo 5: Brass
Voices: 3/8
Octave: +0
OP: 10 8 6 4
```

### Zoomed Parameter View (1.2s on change)
```
Mod Index

50% (5.0)

▓▓▓▓▓▓▓▓▓▓▓▓░░░
```

**Features**:
- ✅ Parameter name + value
- ✅ Percentage + units
- ✅ Progress bar visualization
- ✅ Auto-clear after 1.2 seconds

---

## Signal Flow

```
Keyboard Input
    ↓
Voice Manager (8 voices)
    ↓
For Each Voice:
    ├─ Fm2 (OP1+OP2) ──→ ADSR Env 1,2
    ├─ Fm2 (OP3+OP4) ──→ ADSR Env 3,4
    ├─ Algorithm Matrix
    ├─ SVF Filter
    └─ Voice Output
    ↓
Voice Mixer (sum)
    ↓
Audio Output (L/R)
```

---

## Build Instructions

### Prerequisites
- **libDaisy** installed at `../../../../libDaisy`
- **DaisySP** installed at `../../../../DaisySP`
- **ARM GCC toolchain** configured

### Compile
```bash
cd MyProjects/fm_synth_field
make clean && make
```

### Program to Field
```bash
make program-dfu
```

---

## Technical Details

### Voice Architecture
- **Polyphony**: 8 voices
- **Voice Allocation**: First-free policy
- **Envelopes**: 4 ADSR per voice
    - Attack: 5ms
    - Decay: 100ms  
    - Sustain: 70%
    - Release: 300ms

### FM Engine
- **Operators**: 2× Fm2 (DaisySP)
- **Modulation**: Per-algorithm routing
- **Frequency Ratios**: Algorithm-dependent (1:1, 2:1, 3:1, etc.)

### Filter
- **Type**: SVF (State Variable Filter)
- **Modes**: Lowpass (default)
- **Per-Voice**: Independent filter per voice
- **Cutoff**: 20 Hz - 20 kHz
- **Resonance**: 0.0 - 1.0

### CPU Load
- **Estimated**: ~65-75%
- **Per Voice**: ~8%
- **OLED Updates**: ~5%
- **Headroom**: ~25%

---

## Example Patches

### Patch 1: Classic DX-Piano
- **Algorithm**: 6 (E-Piano)
- **OP Levels**: 1.0, 0.8, 0.6, 0.4
- **Mod Index**: 4.0
- **Filter**: Bypass (max cutoff)
- **Octave**: +0

### Patch 2: Warm Brass
- **Algorithm**: 5 (Brass)
- **OP Levels**: 1.0, 0.9, 0.7, 0.5
- **Mod Index**: 6.0
- **Filter**: 2 kHz cutoff, 0.3 res
- **Octave**: -1

### Patch 3: Bell Tones
- **Algorithm**: 4 (Bell)
- **OP Levels**: 1.0, 0.6, 0.8, 0.4
- **Mod Index**: 3.5
- **Filter**: 5 kHz cutoff, 0.2 res
- **Octave**: +1

### Patch 4: Deep Bass
- **Algorithm**: 7 (Bass)
- **OP Levels**: 1.0, 1.0, 0.5, 0.3
- **Mod Index**: 8.0
- **Filter**: 800 Hz cutoff, 0.7 res
- **Octave**: -2

---

## Code Structure

```cpp
// Main components:
class FMVoice              // Per-voice synthesis
    - Fm2 operators
    - ADSR envelopes
    - ProcessAlgorithm()

class FMVoiceManager       // Polyphony management
    - Voice allocation
    - Note on/off handling
    - Global parameter control

// OLED Functions:
CheckParameterChanges()    // Detect knob changes
DrawZoomedParameter()      // Zoomed param display
DrawDefaultView()          // Normal status display
UpdateOLED()               // Main display update

// Audio Callback:
AudioCallback()            // Process audio + controls
```

---

## Future Enhancements

### Phase 2 (Planned)
- [ ] Expand to 16 algorithms (full DX7-style)
- [ ] MIDI input support
- [ ] Per-operator envelope control
- [ ] LFO modulation
- [ ] Velocity sensitivity per operator

### Phase 3 (Optional)
- [ ] Preset storage/recall
- [ ] Global reverb/chorus
- [ ] Micro-tuning support
- [ ] Performance mode (quick access)

---

## Credits

- **Developer**: DK + Lumina (ESA)
- **Platform**: Electrosmith Daisy Field
- **Libraries**: DaisySP, libDaisy
- **Templates**: Field Midi.cpp, sequencer_pod
- **Date**: 2026-01-04

---

## License

MIT License (same as DaisySP/libDaisy)

---

**Enjoy FM synthesis on Daisy Field!** 🎹✨
