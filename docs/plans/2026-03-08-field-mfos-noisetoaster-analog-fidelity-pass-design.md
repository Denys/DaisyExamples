# Field_MFOS_NoiseToaster Analog Fidelity Pass Design

## Goal

Move `Field_MFOS_NoiseToaster` materially closer to the original MFOS Noise Toaster front-panel interaction without abandoning the Daisy Field note-key adaptation.

This pass should prioritize the approved analog-style controls:

- `B1` three-state VCO output cycling with LED state encoding
- `B2` three-state LFO wave cycling with LED state encoding
- `B3` three-state VCF modulation source cycling with `Off` mapped to LED off
- `B4` repeat/manual behavior
- `B5` VCA bypass
- `SW1` manual gate
- `SW2` panic
- live `AREG` attack and release controls
- OLED idle overview plus focused parameter zoom

## User-Approved Interaction Rules

### Three-State Selector LED Semantics

The user approved a shared LED language for three-state selectors:

- LED on = first state
- LED blink = second state
- LED off = third state

For controls that include an actual `Off` function, the `Off` function must be the LED-off state.

Approved selector mappings:

- `B1` VCO output cycle:
  - on = `Saw`
  - blink = `Square`
  - off = `Triangle`
- `B2` LFO wave cycle:
  - on = `Sine`
  - blink = `Square`
  - off = `Triangle`
- `B3` VCF mod source cycle:
  - on = `LFO`
  - blink = `AREG`
  - off = `Off`

### OLED Rules

The user explicitly wants the display to stop acting as a static usage hint card.

Approved OLED behavior:

- remove the current hint rows for `A1-A8 Notes`, `B1-3 Wave`, `B4 Hold`, and `SW2 Panic`
- show all live knob functions in the idle view
- show a temporary zoomed view of the parameter being changed when a knob moves

## Scope

### Firmware

Modify `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp` so the current keyed synth becomes more analog-like:

- remove the current `hold` workflow
- replace direct `B1/B2/B3` waveform selection with cycling selectors
- replace `B5` AR destination toggle with the approved `VCF mod source` cycle
- replace the fixed ADSR-driven VCA behavior with an `AREG`-centric amplitude path plus `VCA bypass`
- add `Repeat / Manual` and `Manual Gate`
- expose live `AREG` attack and release on the panel
- keep `A1..A8` as pitch-selection/performance keys in this adaptation

### Documentation

Update the project docs so they remain exact reflections of the firmware:

- `README.md`
- `CONTROLS.md`
- `DIAGRAMS.md`

`DIAGRAMS.md` must also gain the still-missing analog reference `Audio Signal Flow` and `Control And Event Flow` Mermaid diagrams.

## Practical Daisy Field Adaptation

The original Noise Toaster has more front-panel knob functions than Daisy Field has physical knobs. The approved compromise for this pass is to spend the available knob budget on the new analog-fidelity controls rather than preserving every current convenience control.

The firmware should therefore expose these eight knob functions:

- `K1` VCO frequency / coarse tune
- `K2` VCO LFO modulation depth
- `K3` VCO AREG modulation depth
- `K4` VCF cutoff
- `K5` VCF resonance
- `K6` VCF modulation depth
- `K7` AREG attack
- `K8` AREG release

As a consequence, some previous controls become fixed internal settings in this pass. The most important examples are:

- LFO rate becomes fixed
- output level becomes fixed
- the old oscillator/noise crossfade is removed as a live knob control

These tradeoffs must be documented explicitly rather than implied.

## Behavioral Design

### Voice Model

The synth remains monophonic, but the interaction becomes more trigger-centric:

- pressing an `A` key selects the note and triggers the AREG
- `SW1` manual gate retriggers the current note if one is armed
- `B4` repeat mode continuously retriggers the AREG while a note is armed
- `SW2` panic clears the armed note and stops repeating activity

This is intentionally closer to the analog Noise Toaster than the previous hold-based workflow.

### Envelope Model

The current project already uses DaisySP `AdEnv`, and the local DaisySP build exposes `Trigger()` plus `Process()` with no gate argument.

For this pass:

- `AdEnv` remains the AREG block
- `K7` and `K8` drive attack and release times
- the AREG modulates pitch and, when selected by `B3`, the filter
- the same AREG also controls amplitude when `VCA bypass` is off

### Repeat Mode

The local `AdEnv` implementation does not provide built-in cycling. The practical repeat behavior should therefore be:

- if repeat mode is enabled
- and a note is armed
- and the AREG is idle
- retrigger it automatically

That produces a repeat/manual interaction without inventing a second envelope engine.

## Verification Strategy

This pass should still follow the repo's TDD preference where practical.

The most testable new behavior is the pure selector logic, so verification should include:

1. a small compile-time `static_assert` test for selector cycling and LED-state semantics
2. a firmware build with `make -C MyProjects/_projects/Field_MFOS_NoiseToaster -j4`
3. a doc integrity check with `git diff --check`

The docs must be updated only after the firmware behavior is settled, so the diagrams and control tables stay 1:1 with the code.
