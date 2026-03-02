# Pod Fuzz Distortion

Aggressive fuzz distortion effect with 4-voice modal synth, tone shaping, and stereo ReverbSc, built for the Daisy Pod.

## Features

- **4-voice modal synth** via MIDI (TRS jack) — modular voice architecture for easy swapping
- Hard clip and soft clip (tanh) modes, toggled via Button 1
- Drive gain from clean (1x) to extreme fuzz (20x)
- SVF lowpass tone control (200 Hz - 12 kHz)
- ReverbSc stereo reverb with feedback, LP freq, and dry/wet mix
- 4-mode encoder for parameter access across 8 controls with only 2 knobs
- Synth enable/disable via Button 2

## Controls

| Mode | Encoder | Knob 1 | Knob 2 | LED1 | LED2 |
|------|---------|--------|--------|------|------|
| Fuzz | Pos 0 | Drive | Tone | Red (drive) | Red/Green (clip) |
| Reverb | Pos 1 | Feedback | LP Freq | Yellow | Off |
| Mix | Pos 2 | Rev Mix | Damping | Off | Cyan |
| Synth | Pos 3 | Structure | Brightness | Purple | Cyan (voices) |

| Button | Function |
|--------|----------|
| Button 1 | Toggle Hard/Soft clip |
| Button 2 | Toggle synth on/off |
| Encoder push | Reserved (future sequencer) |

## Signal Chain

```
MIDI In -> Modal Voice (4-voice) --+
                                   |--> Mix -> Drive -> Clip -> SVF LP -> ReverbSc -> Audio Out L+R
Audio In ----------------------+
```

## Modular Voice Architecture

Voice implementations are swappable via the `Voice` interface (`voice.h`):

1. Create `new_voice.h/.cpp` implementing `Voice`
2. Change `#include` and type in main `.cpp`
3. Add `.cpp` to Makefile `CPP_SOURCES`

Current voice: `ModalSynth` (4-voice modal synthesis with Plaits-derived resonators)

## Build

```bash
make clean && make
make program
```

## Source

Concept from `_concepts/pod_ideas_2602-gmn.md` (Section 3: Fuzz Distortion).
DSP: DaisySP (Svf, ModalVoice) + DaisySP-LGPL (ReverbSc).
