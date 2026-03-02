# Generative Ambient Garden

## Author
DVPE/QAE Pipeline

## Description
A self-playing generative ambient synthesizer for Daisy Field. A 16-bit Turing Machine shift register generates pseudo-random values, quantized to selectable musical scales, driving 4 Modal synthesis voices through stereo reverb. The system produces evolving, musically coherent textures without user input. Users sculpt the garden's character via 8 knobs, scale/preset key selection, and freeze control.

**Complexity**: 9/10

**DSP Chain**: RandomClock -> TuringMachine -> ScaleQuantizer -> 4x ModalVoice -> SVF LP -> ReverbSc (stereo) -> Soft Clip -> Output

## Controls

| Control | Description |
| --- | --- |
| K1 | Density — event rate from sparse (~2s) to dense (~50ms) |
| K2 | Scale Root — transposes scale root C through B (12 steps) |
| K3 | Reverb Size — decay time and damping frequency |
| K4 | Harmony Spread — voice interval spread + Turing Machine probability |
| K5 | Brightness — modal voice brightness + pre-reverb LPF cutoff |
| K6 | Damping — modal voice damping (long ring to short pluck) |
| K7 | Structure — modal voice structure (metallic to woody) |
| K8 | Wet/Dry — reverb mix (dry to fully wet) |
| A1–A8 | Scale select: PentMaj, Major, Minor, Dorian, Mixolydian, PentMin, WholeTone, Chromatic |
| B1–B8 | Voice preset: Glass, Marimba, Metal, Soft Pad, Gamelan, WindChime, Steel Drum, Temple |
| SW1 | Freeze — pauses clock, voices ring out naturally |
| SW2 | Stereo Width — mono center or wide panning (V1,V3 left / V2,V4 right) |

## Source Files

| File | Purpose |
| --- | --- |
| Field_AmbientGarden.cpp | Main source — audio callback, controls, display |
| turing_machine.h | 16-bit shift register with probabilistic feedback |
| scale_quantizer.h | 8 musical scales, MIDI-to-Hz conversion |
| random_clock.h | Sample-accurate trigger generator with jitter |
| CONTROLS.md | Detailed parameter documentation and presets |
| Field_AmbientGarden.dvpe | DVPE visual patch (block diagram) |

## Build

```
make clean && make
```

Requires `USE_DAISYSP_LGPL = 1` (set in Makefile).
