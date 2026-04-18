# GranularSynth

## Author

OpenAI Codex

## Description

Polyphonic Field synth built around DaisySP's `GrainletOscillator`.

This is a granular-style synth voice rather than a sample-granulator: the Field
keyboard drives multiple `GrainletOscillator` voices while the 8 knobs shape
formant tone, articulation, stereo spread, filtering, and reverb.

## Controls

| Control | Function |
| --- | --- |
| Keyboard | Play notes |
| SW 1 / SW 2 | Octave down / up |
| Knob 1 | Shape |
| Knob 2 | Formant frequency |
| Knob 3 | Bleed |
| Knob 4 | Attack |
| Knob 5 | Release |
| Knob 6 | Low-pass cutoff |
| Knob 7 | Stereo spread |
| Knob 8 | Reverb amount |

## Notes

- The playable keys use the same 13-note layout as the stock `modalvoice` and
  `stringvoice` Field examples.
- Key LEDs follow active voices and knob LEDs mirror the current knob values.
