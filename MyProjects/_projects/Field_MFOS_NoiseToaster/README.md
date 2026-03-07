# Field_MFOS_NoiseToaster

A Daisy Field synth inspired by the MFOS Noise Toaster architecture:

- Single VCO (saw/square/triangle)
- White noise source with continuous crossfade
- AR contour that can modulate **pitch** or **filter**
- LFO routable to pitch and filter depth
- Resonant low-pass VCF
- VCA ADSR envelope

## Controls

### Knobs
- **K1**: Coarse tune (+/- 1 octave)
- **K2**: Oscillator/noise mix
- **K3**: Filter cutoff
- **K4**: Filter resonance
- **K5**: AR modulation depth
- **K6**: LFO rate
- **K7**: LFO modulation depth
- **K8**: Output level

### Field keyboard / switches
- **A1..A8**: Play notes (C3..C4)
- **B1/B2/B3**: Select waveform (Saw/Square/Triangle)
- **B4** (or **SW1**): Toggle hold/drone
- **B5**: Toggle AR destination (Pitch vs Filter)
- **SW2**: Panic (release gate + disable hold)

## Build

```bash
cd MyProjects/_projects/Field_MFOS_NoiseToaster
make
```
