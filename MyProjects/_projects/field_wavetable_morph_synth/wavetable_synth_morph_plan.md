# Wavetable Synth with Morphing Implementation Plan

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

## Hardware Integration for Daisy Field

### Audio I/O
- **Inputs**: None (MIDI-only control)
- **Outputs**: Stereo audio out (left/right channels)
- **Sample Rate**: 48kHz
- **Block Size**: 48 samples (1ms blocks for low latency)

### Control Interfaces
- **MIDI**: DIN-5 input for note messages, velocity, pitch bend
- **USB**: MIDI over USB for computer integration
- **CV/Gate**: Optional CV input for external modulation (if available)

### Power and Connectivity
- Powered via USB or external supply
- USB for programming and MIDI
- OLED display for UI feedback

## Software Architecture

### Core Components
1. **Wavetable Engine**
   - WavetableOsc class: Handles wavetable playback with morphing
   - WavetableBank class: Manages multiple wavetables per bank
   - MorphProcessor class: Applies morphing curves and LFO modulation

2. **Voice Architecture**
   - Single voice design (monophonic) for simplicity
   - Oscillator → Filter → Envelope → FX chain

3. **DSP Modules**
   - Oscillator: Wavetable-based with linear interpolation
   - Filter: State Variable Filter (SVF) with lowpass/bandpass/highpass modes
   - Envelope: ADSR generator with exponential curves
   - FX: Stereo chorus/reverb with configurable parameters

4. **Control System**
   - Parameter smoothing using one-pole filters
   - MIDI handler for note on/off, pitch, velocity
   - UI manager for knob/key processing

### Real-time Audio Processing
- **Audio Callback**: Processes 48 samples per block at 48kHz
- **Latency Optimization**:
  - Minimize buffer sizes
  - Use efficient interpolation algorithms
  - Pre-compute wavetable data
  - Optimize filter calculations

### MIDI/USB Integration
- **MIDI Input**: Handles note on/off, pitch bend, modulation
- **USB MIDI**: Class-compliant USB MIDI device
- **Message Processing**: Real-time parsing with minimal delay

## Code Structure

### File Organization
```
field_wavetable_morph_synth/
├── main.cpp              # Main application entry point
├── Makefile              # Build configuration
├── README.md             # Documentation
├── wavetable_synth_morph_plan.md  # Implementation plan
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

### Class Hierarchy
- `WavetableSynth` (main class)
  - `Voice` (contains oscillator, filter, envelope, FX)
    - `WavetableOsc`
    - `Svf`
    - `Adsr`
    - `StereoFx`
  - `MorphProcessor`
  - `MidiHandler`
  - `UiHandler`

### Key Classes
- **WavetableOsc**: Generates audio from wavetable with position control
- **MorphProcessor**: Handles morphing between wavetable positions using selected curves
- **Voice**: Combines all synthesis components into a complete voice
- **MidiHandler**: Processes MIDI messages and updates voice parameters

## Testing Approach

### Unit Testing
- Test individual DSP components (oscillator, filter, envelope)
- Verify wavetable morphing algorithms
- Test MIDI message parsing

### Integration Testing
- Full voice audio generation
- Real-time performance under load
- MIDI responsiveness

### Latency Testing
- Measure round-trip latency from MIDI input to audio output
- Target: <5ms total latency
- Profile CPU usage to ensure real-time operation

### Audio Quality Testing
- THD+N measurements
- Frequency response analysis
- Listening tests for morphing artifacts

## Implementation Steps
1. Set up basic Daisy Field project structure
2. Implement wavetable oscillator with basic playback
3. Add morphing functionality
4. Integrate filter and envelope
5. Add FX processing
6. Implement MIDI handling
7. Add UI controls and OLED display
8. Optimize for low latency
9. Testing and refinement

## Dependencies
- DaisySP library for DSP components
- libDaisy for hardware abstraction
- Standard C++ libraries

## Performance Targets
- CPU usage: <50% at 48kHz
- Latency: <5ms
- Polyphony: Monophonic (expandable to polyphonic if needed)
- Memory usage: <64KB for wavetables