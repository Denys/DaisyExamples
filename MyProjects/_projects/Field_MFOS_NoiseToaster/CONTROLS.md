# Field_MFOS_NoiseToaster Controls

## Overview

This document describes the approved next control model for `Field_MFOS_NoiseToaster`.

It no longer describes the older `SW1 = Manual Gate` layout. The approved direction is:

- `SW1` switches between `MOD0` and `MOD1`
- `MOD0` keeps the current synth surface
- `MOD1` is a sparse analog-recovery bank
- `SW2` remains `Panic`

The synth remains monophonic. A note stays armed after an `A` key press until another note is selected or `SW2` clears it.

## Active Bank Summary

- `MOD0` is the main performance bank.
- `MOD1` exposes only the genuinely missing analog-original controls.
- `K1..K8` values are saved per bank.
- `B1..B8` states are saved per bank.
- Switching banks must restore the saved state of that bank instead of overwriting it with raw physical control positions.

## MOD0 Knobs

| Knob | Parameter | Practical Result |
|------|-----------|------------------|
| `K1` | VCO Frequency / Coarse Tune | About `-1` to `+1` octave around the armed note |
| `K2` | VCO LFO Depth | Controls vibrato or stepped pitch motion depending on the selected LFO wave |
| `K3` | VCO AREG Depth | Adds contour pitch sweep from the same AREG that drives the patch |
| `K4` | VCF Cutoff | Sets the base brightness of the low-pass filter |
| `K5` | VCF Resonance | Mild to strong resonance |
| `K6` | VCF Mod Depth | Sets how much the filter moves from LFO or AREG |
| `K7` | AREG Attack | About `2 ms` to `1202 ms` attack |
| `K8` | AREG Release | About `10 ms` to `1810 ms` release |

## MOD1 Knobs

`MOD1` is intentionally parsimonious.

| Knob | Parameter | Status |
|------|-----------|--------|
| `K1` | LFO Rate | Active analog-recovery control |
| `K2` | Output Level | Active analog-recovery control |
| `K3` | Reserved | Saved per bank for future features |
| `K4` | Reserved | Saved per bank for future features |
| `K5` | Reserved | Saved per bank for future features |
| `K6` | Reserved | Saved per bank for future features |
| `K7` | Reserved | Saved per bank for future features |
| `K8` | Reserved | Saved per bank for future features |

## A Row Note Map

| Key | MIDI Note | Pitch | Behavior |
|-----|-----------|-------|----------|
| `A1` | `48` | `C3` | Arms `C3` and triggers the contour |
| `A2` | `50` | `D3` | Arms `D3` and triggers the contour |
| `A3` | `52` | `E3` | Arms `E3` and triggers the contour |
| `A4` | `53` | `F3` | Arms `F3` and triggers the contour |
| `A5` | `55` | `G3` | Arms `G3` and triggers the contour |
| `A6` | `57` | `A3` | Arms `A3` and triggers the contour |
| `A7` | `59` | `B3` | Arms `B3` and triggers the contour |
| `A8` | `60` | `C4` | Arms `C4` and triggers the contour |

### Note Behavior

- A-row keys are trigger actions, not sustained gate-hold actions.
- Releasing an A-row key does not silence the synth.
- The currently armed note LED stays lit on the A-row.
- `SW1` no longer retriggers the contour directly; it switches banks.

## MOD0 B Row

| Control | Function | States | LED Meaning |
|---------|----------|--------|-------------|
| `B1` | VCO output cycle | `Saw -> Square -> Triangle` | `On = Saw`, `Blink = Square`, `Off = Triangle` |
| `B2` | LFO wave cycle | `Sine -> Square -> Triangle` | `On = Sine`, `Blink = Square`, `Off = Triangle` |
| `B3` | VCF mod source cycle | `LFO -> AREG -> Off` | `On = LFO`, `Blink = AREG`, `Off = Off` |
| `B4` | Repeat / Manual | toggle | `On = Repeat`, `Off = Manual` |
| `B5` | VCA Bypass | toggle | `On = Bypass`, `Off = AREG VCA` |
| `B6` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B7` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B8` | Reserved | saved state only | `On / Blink / Off` are restored per bank |

## MOD1 B Row

| Control | Function | States | LED Meaning |
|---------|----------|--------|-------------|
| `B1` | White Noise switch | toggle | `On = enabled`, `Off = disabled` |
| `B2` | VCO Sync switch | toggle | `On = enabled`, `Off = disabled` |
| `B3` | VCF Input Select | discrete routing cycle | bank-specific saved selector state |
| `B4` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B5` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B6` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B7` | Reserved | saved state only | `On / Blink / Off` are restored per bank |
| `B8` | Reserved | saved state only | `On / Blink / Off` are restored per bank |

## Switches

| Control | Function | Behavior |
|---------|----------|----------|
| `SW1` | MOD bank switch | Toggles between `MOD0` and `MOD1` |
| `SW2` | Panic | Clears the armed note and stops repeat behavior |

## State Persistence

The key requirement of the bank system is that each bank owns its own saved control state.

This applies to:

- `K1..K8` saved parameter values
- `B1..B8` saved logical button states

Example:

- `MOD0:B6 = Blink`
- `MOD1:B6 = On`

Switching away from `MOD0` and back must restore `B6` blinking again. Switching into `MOD1` must restore `B6` solid on. The firmware must save logical state, not just raw LED brightness.

## Knob Pickup Behavior

Each bank also owns its own saved `K1..K8` values.

When the active bank changes:

- the synth should use the saved values of the selected bank
- knobs should not jump immediately to their physical positions
- a knob should only take control after crossing the saved value for the selected bank

This makes `MOD0` and `MOD1` behave like real saved banks rather than temporary overlays.

## Analog-Recovery Controls Assigned To MOD1

The missing analog-original controls chosen for `MOD1` are:

- live `LFO Rate`
- live `Output Level`
- `White Noise`
- `VCO Sync`
- `VCF Input Select`

Other possible future additions should stay unassigned until they are explicitly designed.

## Operational Notes

- `SW1` is now a bank selector, not `Manual Gate`.
- `SW2` remains panic.
- `A1..A8` note-entry behavior is unchanged.
- Reserved controls do not need to affect audio yet, but their saved state and LED behavior must still survive bank switches.
