# Field_AdditiveSynth_2 - Controls

## Overview

`Field_AdditiveSynth_2` is the comparison version of `Field_AdditiveSynth`.
It keeps the same Daisy Field workflow, MIDI behavior, OLED views, knob pages,
LFO, chorus, reverb, and voice allocation, but each voice now uses
`OscillatorBank` instead of `HarmonicOscillator<16>`.

This means Page B behaves essentially the same, while Page A keeps the same
musical intent with a different underlying oscillator model.

SW1 = Page A (tone + ADSR + reverb)
SW2 = Page B (LFO + chorus + volume)

**Knob Pickup/Catch**: On page switch, all knobs lock. A locked knob's stored
value persists until the physical knob crosses that value (within 0.02). Locked
knob LED dims to 5%.

## Control Difference vs. Field_AdditiveSynth

The original project used 16 direct harmonic amplitudes. This project uses the
7 fixed `OscillatorBank` registrations:

1. Saw 8'
2. Square 8'
3. Saw 4'
4. Square 4'
5. Saw 2'
6. Square 2'
7. Saw 1'

Because of that:

- `K1 Rolloff` still acts like a brightness tilt, but no longer controls all harmonics independently.
- `K2 Even` now biases the sound toward the saw registrations and stronger body.
- `K3 Odd` now biases the sound toward the square registrations and a more hollow tone.
- The A-row presets are comparison presets, not exact recreations of the original harmonic spectra.
- The OLED spectrum view now shows 7 registration bars instead of 16 harmonic bars.

## Page A (SW1, default)

| Knob | Parameter | Range | Behavior in Field_AdditiveSynth_2 |
|------|-----------|-------|------------------------------------|
| K1 | Rolloff | 0.0-2.0 | Tilts energy from low registrations to higher ones |
| K2 | Even | 0.0-1.0 | Emphasizes saw registrations and fuller body |
| K3 | Odd | 0.0-1.0 | Emphasizes square registrations and hollow character |
| K4 | Attack | 1ms-2000ms (log) | ADSR attack |
| K5 | Decay | 1ms-2000ms (log) | ADSR decay |
| K6 | Sustain | 0.0-1.0 | ADSR sustain |
| K7 | Release | 1ms-4000ms (log) | ADSR release |
| K8 | Reverb Mix | 0.0-1.0 | Global reverb wet/dry mix |

## Page B (SW2)

| Knob | Parameter | Range | Notes |
|------|-----------|-------|-------|
| K1 | LFO Rate | 0.1-20 Hz (log) | Same as original project |
| K2 | LFO Depth | 0.0-1.0 | Same as original project |
| K3 | LFO Target | 0 = pitch, 1 = amp | Same as original project |
| K4 | Chorus Rate | 0.1-5 Hz | Same as original project |
| K5 | Chorus Depth | 0.0-1.0 | Same as original project |
| K6 | Chorus Delay | 0.0-1.0 | Same as original project |
| K7 | Chorus Mix | 0.0-1.0 | Same as original project |
| K8 | Master Volume | 0.0-1.0 | Same as original project |

## Switches

| SW | Function | SW LED |
|----|----------|--------|
| SW1 | Select Page A | Bright = Page A |
| SW2 | Select Page B | Bright = Page B |

## Keyboard A Row - Comparison Presets

Selecting a preset loads `rolloff`, `even`, and `odd`, then locks K1-K3 on Page A.
Moving any unlocked K1-K3 control releases the preset and returns the synth to
custom mode.

| Key | Preset | rolloff | even | odd | Comparison intent |
|-----|--------|---------|------|-----|-------------------|
| A1 | Sine | 5.0 | 0.0 | 0.0 | Smoothest available bank voicing |
| A2 | Tri | 2.0 | 0.0 | 0.7 | Softer, hollow comparison tone |
| A3 | Saw | 1.0 | 0.8 | 0.8 | Bright full-spectrum comparison tone |
| A4 | Sqr | 1.0 | 0.0 | 0.8 | Hollow square-leaning comparison tone |
| A5 | Hollow | 1.5 | 0.9 | 0.1 | Airy, saw-forward comparison tone |
| A6 | Bell | 0.5 | 0.2 | 0.9 | Bright, sharper bank tone |
| A7 | Organ | 0.0 | 0.8 | 0.6 | Closest to native OscillatorBank territory |
| A8 | Buzz | 0.1 | 0.8 | 0.8 | Rich, bright comparison tone |

## Keyboard B Row - LFO + Performance

| Key | Function | Mode | LED |
|-----|----------|------|-----|
| B1 | LFO Waveform: Sine | radio | Lit = active |
| B2 | LFO Waveform: Tri | radio | Lit = active |
| B3 | LFO Waveform: Saw | radio | Lit = active |
| B4 | LFO Waveform: S and H | radio | Lit = active |
| B5 | LFO Sync on NoteOn | toggle | Lit = on |
| B6 | Reverb bypass | toggle | Lit = bypassed |
| B7 | Chorus bypass | toggle | Lit = bypassed |
| B8 | Sustain pedal | toggle | Lit = held |

## MIDI (TRS hardware)

- 8-voice polyphonic note handling
- last-note stealing when all voices are active
- velocity scales each voice level

## OLED Display

**Normal view**:
- Row 0: active notes + page indicator
- Row 1: 7 registration bars relative to the current max
- Row 2: preset name and bypass flags

**Zoom view**:
- Row 0: parameter name, with `(L)` shown for locked knobs
- Row 1: value readout
- Row 2: full-width progress bar
