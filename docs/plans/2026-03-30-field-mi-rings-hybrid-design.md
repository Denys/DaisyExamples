# Field MI Rings Hybrid Design

## Goal

Create a new Daisy Field project named `Field_MI_Rings` that delivers a standalone,
external-MIDI-played, 2-voice polyphonic Rings-style instrument. The design should
reuse the working Daisy Field application shell from `Field_MI_Plaits`, adopt a
shared banked-parameter and pickup/catch control system, and establish a repo-level
documentation standard for all display-based Field projects.

## Scope

This first pass stays intentionally focused:

- New project: `MyProjects/_projects/Field_MI_Rings`
- Standalone instrument with internal exciter
- External MIDI input from `OXI One`
- 2-voice polyphony
- Three Rings-style resonator model families
- Small OXI-friendly MIDI CC set
- OLED overview, zoom, and mode pages
- Free keybed used for mode/page/model controls rather than notes
- Shared banked-parameter and pickup/catch system reusable by multiple projects
- Retrofit the same banked-parameter behavior into `Field_MI_Plaits`
- Detailed README files for both `Field_MI_Plaits` and `Field_MI_Rings`

Out of scope for v1:

- External audio input processing for Rings
- Full line-by-line DSP port of the original Rings code
- More than 2 voices of polyphony
- Preset storage
- USB MIDI host support
- Reworking unrelated projects in the repository

## Architecture

The design deliberately separates the problem into two layers:

1. A shared Daisy Field UX/control layer
2. A Rings-specific voice application

The shared layer is the more important architectural change. Current projects read
the physical knob directly into whichever parameter bank is active. That creates
parameter jumps whenever a switch-hold page or alternate parameter bank is entered
or exited. The new shared layer stores parameter values independently per bank and
requires the physical knob to catch the stored value before it can edit it.

The `Field_MI_Rings` application should reuse the proven Field app structure from
`Field_MI_Plaits`: MIDI handled in the main loop, audio callback kept DSP-only,
OLED updated in the main loop, and Field LEDs used as mode indicators. That keeps
the app shell stable while allowing the DSP layer to change.

## Shared Field UX Policy

The following rules should be treated as system-level behavior for display-based
Field projects:

### Parameter Banks

- Each physical knob may control different parameters on different banks/pages
- Each bank stores its parameter values independently
- Switching banks never overwrites stored values

### Pickup / Catch

- Physical knob position is not the parameter value
- When a bank becomes active, the stored parameter value remains authoritative
- A knob only begins editing after it catches the stored value within a threshold

### Knob LEDs

- Knob LEDs represent stored logical parameter values, not raw pot position
- This makes pickup/catch behavior visually understandable
- Optional refinement:
  - uncaptured state = dim brightness
  - captured state = full brightness

### Keybed As Controls

- When notes come from external MIDI, `A1-A8` and `B1-B8` are available for
  project state and menu controls
- Key LEDs should support three states when useful:
  - `Off`
  - `Blink`
  - `On`

This allows discrete multi-state control without wasting OLED space.

### Documentation Policy

Every Field project with a display should include a `README.md` that documents:

- Overview and project intent
- Full control mapping
- Alternate/hidden pages
- OLED pages and what each page shows
- LED behavior
- MIDI mappings
- Panic/reset behavior
- Explicit startup/default values for all parameters and important state

## Rings v1 DSP Strategy

This design uses a hybrid approach rather than a full direct port of the original
Rings firmware.

### Why Hybrid

- Lower flash/code risk than another large monolithic port
- Faster path to a polished instrument
- Reuses local DaisySP physical-modeling blocks already present in the repo
- Preserves the Rings user concept even if the implementation is not a byte-for-byte
  copy of the original Mutable code

### DSP Basis

The Rings-style models should be built from existing local DSP building blocks:

- `daisysp::Resonator`
- `daisysp::ModalVoice`
- `daisysp::StringVoice`

The three v1 model families should feel like Rings, not generic synth voices:

- `Modal`
- `String`
- `Sympathetic-style`

The exact hybrid DSP details can evolve during implementation, but the user-facing
control language should stay close to Rings.

## Rings v1 Application Design

### Voice Model

- 2 voices
- External MIDI drives pitch and note events
- Velocity drives exciter strength/accent
- Internal exciter means the instrument works without external audio input
- Stereo voice placement keeps the two notes spatially separated rather than summed
  to the center

### Main Knob Map

Recommended v1 map:

- `K1` = `Structure`
- `K2` = `Brightness`
- `K3` = `Damping`
- `K4` = `Position`
- `K5` = stereo/poly spread
- `K6` = exciter strength baseline
- `K7` = model-specific macro
- `K8` = interaction/resonance macro

Important rule:

- `K8` should not be used for generic output volume by default
- Daisy Field already has hardware-level master volume
- A synth-level level control should only be used if it is musically important to
  the engine, not merely because it is a common placeholder

### Alt Knob Bank

`SW1 + knob` should expose secondary controls without destroying main-bank values.
Suggested secondary functions:

- internal exciter color
- internal exciter decay/noise
- reverb or space amount if included in v1
- strum/retrigger behavior
- voice interaction details
- output/aux behavior or a reserved global slot

### Keybed Usage

Since the keybed is not used for note entry in this project, it should be used for
discrete state control:

- model family selection
- page selection
- discrete mode toggles
- preset-like behavior sets if needed
- panic or quick utility functions where helpful

The three-state LED rule should be available when a control has more than two
meaningful states.

## OXI One MIDI Mapping

The MIDI mapping should stay intentionally small and musically focused.

Recommended v1 map:

- `Note On` / `Note Off`
- velocity -> exciter strength / accent
- `CC74` -> `Brightness`
- `CC71` -> `Damping`
- `CC16` -> `Structure`
- `CC17` -> `Position`
- optional `CC1` -> performance macro

Rationale:

- OXI One is very good at sequencing a compact set of expressive CC lanes
- The most attack-critical behavior should come from note velocity rather than a CC
  lane
- Discrete state changes such as model selection or mode changes should stay on the
  Field keybed rather than on CC

## Plaits Retrofit

Before treating `Field_MI_Plaits` as the reference shell for newer Field instruments,
it should be updated to use the same shared banked-parameter and pickup/catch system.

That retrofit should include:

- main-bank and alt-bank value separation
- knob pickup/catch behavior
- knob LEDs driven from stored values
- review of `K8` usage so it is not kept as generic level unless it is truly part of
  Plaits voice behavior
- improved documentation with full OLED and startup/default descriptions

`Field_MI_Plaits` should become the first project proving the shared UX layer works.

## OLED / README Standard

The README and OLED documentation should be treated as deliverables, not afterthoughts.

Every display-based project README should explicitly document:

- control table
- alternate bank behavior
- key LED meanings
- OLED overview page
- OLED zoom page
- OLED alt or mode pages
- hidden settings
- panic/reset behavior
- exact startup/default values for all parameters and major state

## Success Criteria

- Shared banked-parameter and pickup/catch behavior is specified clearly enough to
  implement once and reuse
- `Field_MI_Plaits` can be retrofitted to the new behavior without changing its
  musical intent
- `Field_MI_Rings` has a clear v1 feature boundary
- OXI One integration is defined around a compact set of useful CCs
- Documentation expectations are explicit and reusable

## Recommended Rollout Order

1. Build the shared Field parameter-bank, pickup/catch, and LED-state helpers
2. Retrofit `Field_MI_Plaits` to use them
3. Add full README/OLED documentation to `Field_MI_Plaits`
4. Create `Field_MI_Rings`
5. Implement the Rings v1 app on top of the shared helper layer
6. Add full README/OLED documentation to `Field_MI_Rings`
