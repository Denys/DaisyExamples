# Pod MultiFX Tube Delay Reverb Design

**Date:** 2026-04-14

## Goal

Refactor `Pod_MultiFX_Chain` into a three-page Daisy Pod pedal using DAFX
`Tube`, short `Delay`, and simplified `Reverb`, while preserving page-owned
state with soft takeover.

## Scope

- Remove DAFX `WahWah` from the live signal path
- Replace `Overdrive` with DAFX `Tube`
- Reduce delay time range to `10 ms -> 100 ms`
- Reduce delay feedback range to `0.0 -> 0.5`
- Simplify reverb to `Decay + Mix`
- Preserve encoder page switching, `SW1` page bypass, and `SW2` global bypass
- Add Tube-only `SW1` hold behavior for editing hidden parameters
- Update firmware header comments, README, tests, and saved implementation plan
- Mark DAFX `WahWah` as a revisit item in docs

## Control Rules

### Page Order

- Tube
- Delay
- Reverb

### Switches

- `SW1` short press:
  - Tube page: toggle Tube bypass
  - Delay page: toggle Delay bypass
  - Reverb page: toggle Reverb bypass
- `SW1` hold:
  - Tube page only: temporary shift layer while held
  - other pages: no alternate behavior
- `SW2`: toggle global bypass for the whole chain

### Encoder

- Encoder turn changes the selected page

### Knobs

- Delay and Reverb use one normal two-knob layer per page
- Tube uses two logical two-knob layers:
  - default layer:
    - `Knob 1 = Drive`
    - `Knob 2 = Mix`
  - shifted layer while `SW1` is held:
    - `Knob 1 = Bias`
    - `Knob 2 = Distortion`

## Parameter Mapping

### Tube

- `Mix`: linear response
- `Drive`: exponential response
- `Bias`: center-biased exponential response around `0`
- `Distortion`: exponential response

Tube uses warm fixed defaults for internal filter poles and starts from a
musical default bias/distortion setup even before the shift layer is used.

### Delay

- `Time`: `10 ms -> 100 ms`
- `Feedback`: `0.0 -> 0.5`

This intentionally shifts the delay page toward slapback and resonant short
echoes rather than long ambient repeats.

### Reverb

- `Decay`: one macro decay control
- `Mix`: linear wet/dry control

The LP tone is fixed to keep the reverb page simpler than the previous
`Feedback + LP frequency` design.

## State Model

The pedal state should be page-owned and independent from the raw knob
positions.

Required stored state:

- selected page index
- global bypass flag
- per-page bypass flags
- Delay page knob values and capture flags
- Reverb page knob values and capture flags
- Tube default-layer knob values and capture flags
- Tube shifted-layer knob values and capture flags
- `SW1` hold state for Tube shift behavior

This allows:

- page switching without losing settings
- Tube shift entry without `Drive/Mix` overwriting `Bias/Distortion`
- short press and hold behavior to coexist on `SW1`

## Soft Takeover Rule

Soft takeover, also called pickup/catch, means:

- the selected logical parameter set keeps using its stored values immediately
- switching page or layer does not jump to the raw knob positions
- each knob edits only after crossing the stored value within a small threshold

For this pedal, Tube default and Tube shifted layers each need their own
soft-takeover state.

## Audio Behavior

Signal chain becomes:

- Tube -> Delay -> Reverb

Behavior rules:

- global bypass returns dry mono input to stereo output
- page bypass still works when global bypass is off
- all DSP parameters are always applied from stored state
- Tube shift only changes which stored Tube parameters the knobs edit, not the
  signal-chain order or bypass semantics

## LED Behavior

- LED color tracks the selected page:
  - Red = Tube
  - Green = Delay
  - Blue = Reverb
- LED brightness dims when:
  - global bypass is active, or
  - the selected page is bypassed

## Documentation Notes

- Firmware header comment must reflect:
  - Tube / Delay / Reverb only
  - Tube shift behavior
  - `SW1` short press vs hold behavior
  - soft takeover
- README should note that DAFX `WahWah` was removed from the live pedal and is a
  revisit candidate rather than an active page

## Verification Strategy

- Update the host-side test first so it fails on:
  - three-page wrap
  - Tube shift-layer storage
  - Tube short-press vs hold behavior
  - Delay/Reverb state persistence
- Re-run host-side test after helper and firmware updates
- Run Daisy QAE lint
- Run clean firmware rebuild
