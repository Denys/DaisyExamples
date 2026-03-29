# Controls - Field_Template_Std_2

## Intended baseline

Template baseline for Daisy Field projects with:
- External MIDI keyboard note input (TRS MIDI via `hw.midi`)
- 8-knob parameter mapping with smoothed audio-side response
- OLED overview plus temporary zoom on knob movement
- Key and switch handling patterns for mono/poly synths and drum machines

## Knobs

| Knob | Parameter |
|------|-----------|
| K1   | Filter cutoff |
| K2   | Filter resonance |
| K3   | Envelope attack |
| K4   | Envelope decay |
| K5   | Envelope sustain |
| K6   | Envelope release |
| K7   | Drive |
| K8   | Output level |

## Keyboard / switches

- A1-A4: one-hot waveform select (`Sine`, `Tri`, `Saw`, `Square`)
- A5-A8: reserved for future synth/drum shortcuts
- B1-B8: reserved for future synth/drum shortcuts
- SW1: panic/all-notes-off.
- SW2: clear OLED zoom/focus state.

## External MIDI keyboard

- `NoteOn`/`NoteOff` mapped to synth voice gate.
- CC64 (sustain pedal) example included.

## Notes

- `Field_Template_Std` remains untouched; this fork is the new external-MIDI Field starter.
- Runtime behavior intentionally avoids broad changes to `field_defaults.h`.
