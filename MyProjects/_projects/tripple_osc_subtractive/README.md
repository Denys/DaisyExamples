# tripple_osc_subtractive

## Overview

`tripple_osc_subtractive` is a Daisy Field concept synth focused on a **3-oscillator subtractive voice** controlled from an **external MIDI keyboard or sequencer**.

The project uses a banked control model to expand 8 physical knobs into **24 stored parameters**:

- `DEFAULT` bank: core oscillator/filter/envelope tone shaping
- `SW1` bank: modulation and filter behavior
- `SW2` bank: output voicing/performance utility controls

The LED key matrix (`A1-A8`, `B1-B8`) is used as a tri-state control surface:

- **OFF** = unrelated in current mode
- **BLINK** = available but not selected
- **ON** = currently selected

OLED pages are mode-aware and provide a zoomed parameter display while controls are moving.

## Core Feature Set

- 3 primary oscillators (`osc1`, `osc2`, `osc3`) + noise + derived sub component
- Subtractive filter path with selectable LP/BP/HP response
- Separate amplitude and filter ADSR envelopes
- LFO with selectable waveform and routable targets
- External MIDI note/velocity control with channel filtering or OMNI
- Glide/portamento, drive, stereo spread, keytracking, transpose

## Banked Knob Workflow (24 controls)

There are always 8 active knobs (`K1-K8`), but assignments depend on mode.

### Mode switching

- Press `SW1` to switch active bank to `SW1`
- Press `SW2` to switch active bank to `SW2`
- Press `SW1` while `SW2` is held to return to `DEFAULT`

### Why this is useful

- No parameter jump from page changes (stored values per parameter)
- Fast panel access to deep synth controls without menu diving
- Hardware feedback stays consistent via ring LEDs and OLED zoom

## OLED behavior

OLED always shows:

1. Project header and current mode (`DEFAULT`, `SW1`, `SW2`)
2. Mode summary line for key selections
3. **Zoom view** when a parameter changes (name + large value)
4. Fallback mini-summary when idle

## MIDI behavior

The voice is intended for external MIDI sequencing/keyboard play.

- `NoteOn` with velocity > 0: gate on + note/velocity update
- `NoteOn` velocity 0 or `NoteOff`: note release
- Last-note priority over currently held notes
- Optional channel filter via LED-key function (`SW1` mode), including `OMNI`

## LED key functional layers

The same LED keys expose different controls based on active mode:

- `DEFAULT`: LFO waveform, oscillator waveform, hard-sync toggle, legato toggle
- `SW1` (alt1): keytrack amount, filter mode, MIDI channel/OMNI
- `SW2` (alt2): global transpose and LFO target routing

This enables **direct hands-on switching** with visual tri-state feedback.

## Build

From this folder:

```bash
make
```

Expected repo layout dependencies:

- `../../../libDaisy`
- `../../../DaisySP`

## Notes

- Project name intentionally uses `tripple` to match folder naming requested for this project.
- This is a concept voice architecture and can be extended to polyphony, MIDI CC mapping, or preset storage in future passes.
