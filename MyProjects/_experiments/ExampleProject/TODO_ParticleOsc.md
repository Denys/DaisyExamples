# TODO: ParticleOsc Synth for Daisy Field

## Session Status: 2025-12-14
**Build Status**: ✅ Compiles successfully

## What Was Done
- [x] Read daisy_expert_system_prompt.md
- [x] Created `ParticleOsc.cpp` - Particle oscillator with SVF filter
- [x] Created `Makefile_ParticleOsc`
- [x] Build successful: `ParticleOsc.bin` (99KB, 75% flash)

## Current Implementation

### Signal Flow
```
KEY (B1-B8) → PARTICLE → SVF FILTER → AUDIO OUTPUT
```

### Knob Mapping
| Knob | Parameter | Range |
|------|-----------|-------|
| K0 | Particle Density | 0.0 - 1.0 |
| K1 | Particle Spread | 0.0 - 1.0 |
| K2 | Particle Freq | 0 - 10kHz |
| K3 | Particle Resonance | 0.0 - 1.0 |
| K4 | Filter Cutoff | 20Hz - 20kHz |
| K5 | Filter Resonance | 0.0 - 0.95 |

### Keyboard Mapping (Monophonic C3-C4)
- B1: C3 (130.81 Hz)
- B2: D3 (146.83 Hz)
- B3: E3 (164.81 Hz)
- B4: F3 (174.61 Hz)
- B5: G3 (196.00 Hz)
- B6: A3 (220.00 Hz)
- B7: B3 (246.94 Hz)
- B8: C4 (261.63 Hz)

## Known Issues
- [ ] Used **Svf** filter instead of **MoogLadder** (MoogLadder not in DaisySP build)

## Next Steps
- [ ] Flash to Daisy Field and test: `make -f Makefile_ParticleOsc program-dfu`
- [ ] Consider adding MoogLadder if available in updated DaisySP
- [ ] Add CV modulation inputs if desired
- [ ] Tune envelope times for desired response

## Files
- `ParticleOsc.cpp` - Main source
- `Makefile_ParticleOsc` - Build configuration
- `build/ParticleOsc.bin` - Firmware binary

## Reference Diagram
Original request was based on visual patch diagram with:
- KEY block (Note 60/MIDI, Velo 0.80)
- 6 KNOBS (K0-K5) 
- PARTICLE block (Dens 0.50, Spre 0.50)
- MOOG LADDER block (Cuto 1000Hz, Reso 0.40)
- AUDIO OUTPUT
