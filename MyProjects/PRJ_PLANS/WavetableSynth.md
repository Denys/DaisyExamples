# Wavetable Synth with Morphing Implementation Plan

**Status**: ✅ Compiles (SDRAM migration complete 2026-01-23)
**Migrated to**: PLANNING/projects/ (2026-01-23)
**Relationship to DVPE**: Standalone Daisy Project (NOT an integrated DVPE feature)
**Location**: `DaisyExamples/MyProjects/_projects/field_wavetable_morph_synth/`

---

## Overview
This implementation plan outlines the development of a wavetable synthesizer with morphing capabilities for the Daisy Field platform, based on section 16 of the field_concepts_OPUS_2.md specifications. The synth features custom wavetables, position-based morphing, MIDI integration, and real-time audio processing optimized for low latency.

## Specifications Analysis
- **Complexity**: ★★★★☆ (High)
- **Core Features**:
  - Custom wavetables with position morphing
  - Wavetable bank selection (8 banks via A1-A8 keys)
  - Morph curve selection (8 curves via B1-B8 keys)
  - SVF filter + ADSR envelope
  - Stereo FX processing
  - MIDI input for note control
  - LFO modulation for wavetable position

## Build Status

| Date | Status | Notes |
|------|--------|-------|
| 2026-01-22 | ⚠️ 75% | SRAM overflow (113%) |
| 2026-01-23 | ✅ Compiles | SDRAM migration complete |

**Final Build**:
- FLASH: 99KB (76%)
- SDRAM: 512KB for wavetable data

## Detailed Controls Mapping

### Knobs (8 available on Daisy Field)
1. **Knob 1**: Wavetable Position (0-1, modulates table index with LFO)
2. **Knob 2**: Morph Speed (controls rate of position changes, 0-10 Hz)
3. **Knob 3**: Filter Cutoff (SVF cutoff frequency, 20Hz - 20kHz)
4. **Knob 4**: ADSR Attack (0-5 seconds)
5. **Knob 5**: ADSR Decay (0-5 seconds)
6. **Knob 6**: ADSR Sustain (0-1)
7. **Knob 7**: ADSR Release (0-10 seconds)
8. **Knob 8**: FX Amount (dry/wet mix for stereo FX, 0-1)

### Keys (16 available: A1-A8, B1-B8)
- **A1-A8**: Wavetable Bank Select
  - A1: Sine waves bank
  - A2: Sawtooth waves bank
  - A3: Square waves bank
  - A4: Triangle waves bank
  - A5: Custom wavetable 1 bank
  - A6: Custom wavetable 2 bank
  - A7: Custom wavetable 3 bank
  - A8: Custom wavetable 4 bank
- **B1-B8**: Morph Curve Select
  - B1: Linear morphing
  - B2: Exponential morphing
  - B3: Logarithmic morphing
  - B4: Sine morphing
  - B5: Triangle morphing
  - B6: Step morphing
  - B7: Random morphing
  - B8: Custom curve morphing

### Switches (2 available: SW1, SW2)
- **SW1**: LFO On/Off for position modulation
- **SW2**: FX Bypass

### OLED Display
- Real-time visualization of:
  - Current wavetable waveform
  - Morph position indicator
  - Filter response curve
  - ADSR envelope state
  - FX parameters

## Code Structure

### File Organization
```
field_wavetable_morph_synth/
├── main.cpp              # Main application entry point
├── Makefile              # Build configuration
├── README.md             # Documentation
├── CONTROLS.md           # Control mapping documentation
├── wavetable_osc.h       # Wavetable oscillator class
├── wavetable_osc.cpp
├── morph_processor.h     # Morphing logic
├── morph_processor.cpp
├── voice.h               # Synth voice class
├── voice.cpp
├── midi_handler.h        # MIDI processing
├── midi_handler.cpp
├── ui_handler.h          # UI and controls
├── ui_handler.cpp
├── wavetables.h          # Wavetable data definitions
└── wavetables.cpp
```

## Performance Targets
- CPU usage: <50% at 48kHz
- Latency: <5ms
- Polyphony: Monophonic (expandable to polyphonic if needed)
- Memory usage: 512KB SDRAM for wavetables
