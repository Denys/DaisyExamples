# Field Wavetable Drone Lab

**Complexity**: 7/10  
**Concept**: A self-evolving drone generator using wavetable morphing and multi-stage modulation. Uses LFOs to slowly sweep through spectral content without user input.

## Overview

This synth creates evolving ambient textures by combining multiple oscillator sources with LFO-driven morphing. The Field keyboard selects drone "presets" while knobs control the modulation depths and filter characteristics.

## Audio Path
1. **Oscillator Bank** (8 harmonically-related oscillators) → provides rich harmonic base
2. **Variable Shape Oscillator** → adds morphable waveform content  
3. **Mixer** → combines with LFO-controlled crossfade
4. **Dual SVF** (parallel LP/HP) → spectral shaping with envelope mod
5. **Stereo Output** → with subtle width from filter mix

## Required DaisySP Modules
| Module | Purpose |
|--------|---------|
| `OscillatorBank` | 8-voice harmonic drone base |
| `VariableShapeOsc` | Morphable waveform source |
| `Oscillator` (x2) | LFO1 (morph) and LFO2 (level) |
| `Svf` (x2) | Parallel lowpass and highpass |
| `Adsr` | Envelope for filter cutoff modulation |

## Control Philosophy

The drone is **always running** - no MIDI trigger required. Keys A/B select morphing presets. All 8 knobs provide continuous real-time control over the drone evolution.
