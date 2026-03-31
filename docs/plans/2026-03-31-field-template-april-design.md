# Field Template April Design

**Date:** 2026-03-31

## Goal

Create `MyProjects/_projects/Field_Template_April` as a clean Daisy Field starter project that demonstrates the current shared control, display, and documentation rules with a minimal mono synth voice.

## Scope

- Buildable Daisy Field project with a simple mono synth shell
- External MIDI note input with velocity and sustain
- Keybed used as control/state buttons, not piano-note input
- Main and alt knob banks with pickup/catch
- Knob LEDs show stored logical values, not raw pot positions
- `A1-A8` / `B1-B8` demonstrate `Off / Blink / On` key LED states
- OLED overview plus parameter zoom behavior
- Full README with explicit startup/default values and OLED/menu replica

## Control Rules

### Knobs

- `K1-K8` are the main parameter bank
- `SW1 + K1-K8` accesses the alt parameter bank
- Bank switching must preserve stored values
- Knobs must re-capture values after a bank switch before changing them
- Knob LEDs must visualize the stored value of the active parameter

### Level Placement

- Do not waste main `K8` on generic output level
- Internal level is available only on `SW2 + K8`

### Switches

- `SW1` acts as the alt-bank modifier
- `SW2` acts as a secondary modifier and panic/reset helper where appropriate

### Keybed

- `A1-A8` / `B1-B8` are control/state buttons
- LEDs support `Off`, `Blink`, and `On`
- The template should demonstrate state control patterns rather than note-entry behavior

## Audio / MIDI

- Minimal mono synth voice with clear, low-risk DSP
- External MIDI handles:
  - `Note On`
  - `Note Off`
  - velocity
  - `CC64` sustain
- No extra MIDI CC parameter mapping in the template

## OLED

The OLED should demonstrate the shared display behavior:

- compact overview page by default
- zoom/detail view when a parameter is actively edited
- labels and values aligned with the active bank

## Documentation Rules

The project README must include:

- purpose
- full control table
- LED semantics
- OLED page/menu replica
- hidden bank behavior
- exact startup/default values for all parameters and mode states
- panic/reset behavior
- MIDI behavior
- maintenance notes

## Implementation Notes

- Reuse `field_defaults.h`, `field_instrument_ui.h`, and `field_parameter_banks.h`
- Keep the audio callback real-time safe
- Process controls, MIDI, OLED, and LED refresh in the main loop
- Prefer clarity and starter-project readability over feature depth
