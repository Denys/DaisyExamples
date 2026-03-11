# Controls — Field_Template_Std

## Intended baseline

Template baseline for Daisy Field projects with:
- External MIDI keyboard note input (TRS MIDI via `hw.midi`)
- 8-knob parameter mapping
- OLED parameter visualization (`FieldOLEDDisplay`)
- Key and switch handling patterns

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

- A1-A8: waveform selection shortcuts with LED toggle feedback.
- B1-B8: generic toggle slots for project-specific functions.
- SW1: panic/all-notes-off.
- SW2: clear OLED active-parameter focus.

## External MIDI keyboard

- `NoteOn`/`NoteOff` mapped to synth voice gate.
- CC64 (sustain pedal) example included.
