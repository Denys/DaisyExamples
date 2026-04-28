# DaisyHost Field OLED Interaction Rules Design

Date: 2026-04-28

## Purpose

Refine the Daisy Field OLED behavior for Subharmoniq and future Field-native
apps so the display is useful during performance, not just a static status
panel. The OLED should explain compact abbreviations, temporarily zoom into the
control being touched, and surface clear confirmation for button actions.

This design is host-side only. It does not change firmware, DAW/VST3 behavior,
DSP algorithms, routing presets, or hardware voltage behavior.

## Current Problem

The current Subharmoniq OLED status can show compact text such as `Q 12-JI`.
That is technically meaningful but unclear in performance context:

- `Q` means quantize.
- `12-JI` means 12-note Just Intonation.
- The compact form does not explain itself.

The current OLED also keeps showing the general status while knobs or keys are
being changed. That makes important live feedback hard to see, for example the
actual cutoff frequency while K8 controls cutoff, or the new transport state
after pressing B7.

Separately, the user reported that audio can stop after a couple of notes. That
must be treated as an audio/runtime regression, not as an OLED feature. The OLED
can optionally expose a diagnostic hint, but tests must prove the audio path.

## Display Modes

### 1. Compact Status

This is the normal idle display.

Recommended Subharmoniq status fields:

- title/page: `Subharmoniq Seq/Rhy`, `Subharmoniq VCO`, `Subharmoniq VCF`, or
  `Subharmoniq VCA/Mix`
- transport: `Play` or `Stop`
- quantize: `Quant 12-JI`, `Quant 12-ET`, `Quant 8-JI`, `Quant 8-ET`, or
  `Quant Off`
- active sequence steps: `S1:1 S2:1`
- rhythm routing summary: `R1/S1 R2/S2`, `R3/Both R4/Off`
- navigation hint only when space allows: `SW1<- SW2->`

Rule: avoid bare `Q` in the status line unless the display has no other viable
space. Prefer `Quant` because it removes ambiguity.

### 2. Touch Zoom

Any Field control interaction temporarily takes over the OLED with a larger,
single-purpose readout.

Trigger sources:

- K1-K8 knob movement
- CV/mapped parameter movement when it changes a visible/effective parameter
- A1-B8 key press or key menu action
- SW1/SW2 page navigation
- encoder/menu value editing

Timing:

- show the zoom view immediately when the control changes
- keep it visible for 2.0 seconds after the last change
- if a button/key is held, keep the zoom visible while held
- after release, keep it visible for 2.0 seconds, then return to compact status
- repeated movement refreshes the 2.0 second timer

Knob zoom should show:

- control label: `K8 Cutoff`
- large current value: for example `1.84 kHz`
- optional normalized bar
- optional page/context line if it fits

Button zoom should show:

- key label: for example `B7 Play`
- large resulting value: for example `Running`
- one compact consequence/context line, such as current steps or rhythm target

### 3. Detail Wording

Compact status and zoom/detail wording should differ:

- compact: `Quant 12-JI`
- detail: `12-note Just`
- compact: `Quant 8-ET`
- detail: `8-note Equal`
- compact: `Seq Oct 5`
- detail: `5 octave range`
- compact: `R2/S2`
- detail: `Rhythm 2 -> Seq2`

The goal is that compact status remains dense, while zoom views teach the
abbreviations without adding permanent clutter.

## Subharmoniq Examples

### Knob

When K8 controls cutoff:

```text
K8 Cutoff
1.84 kHz
[value bar]
returns after 2.0s
```

The formatter should use musical/native units where available. For cutoff-like
parameters, prefer Hz/kHz over raw normalized values.

### Transport Key

When B7 is pressed:

```text
B7 Play
Running
Step S1:2 S2:2
Quant: 12-note Just
```

If the same key stops playback:

```text
B7 Stop
Stopped
Step S1:2 S2:2
```

### Quantize Key

When B5 cycles quantize:

```text
B5 Quantize
12-note Just
LED: on
```

The LED line is optional; include it only if it does not crowd the main value.

### Rhythm Target Key

When A6 cycles Rhythm 2:

```text
A6 Rhythm 2
Seq2
R2 routes to Sequencer 2
```

When target is Both:

```text
A6 Rhythm 2
Both
R2 advances both sequencers
```

## Audio Dropout Handling

The report "after a couple of notes, audio stops reproducing" is a separate
runtime bug. The implementation plan should add a regression test before any
fix:

- launch/play Subharmoniq on `daisy_field`
- trigger multiple notes or clock pulses across enough time to reproduce the
  "couple notes" case
- assert output remains finite and non-silent unless `panic`, explicit hard
  mute, or an intentional zero-output parameter state is active
- compare MIDI-note path, B7 play/clock path, and Field key/menu action path

If a live diagnostic is cheap and reliable, the OLED may show:

```text
Audio: silent
Check gate/mute/output
```

Only show this diagnostic when the host can distinguish unintended silence from
intentional mute/stop. Do not add noisy false alarms to the performance display.

## Architecture

Add a small host-side display-event layer rather than baking all behavior into
static status rendering.

Suggested units:

- `FieldOledTransient` or equivalent lightweight state:
  - active flag
  - source control/key id
  - title
  - large value
  - detail lines
  - expiry time
  - held flag
- formatter helpers:
  - quantize compact/detail labels
  - seq octave compact/detail labels
  - rhythm target compact/detail labels
  - parameter native value labels
- app wrapper integration:
  - Subharmoniq control setters record transient display events
  - `TickUi` expires transient events
  - `BuildDisplay` chooses transient zoom first, compact status second

Keep this independent enough that other Field-native hosted apps can reuse the
same pattern later.

## Testing

Write tests before implementation.

Required tests:

- compact status uses `Quant 12-JI` rather than bare `Q 12-JI`
- quantize detail formatter expands `12-JI` to `12-note Just`
- moving a mapped knob creates a transient OLED zoom with the control label and
  current value
- repeated movement refreshes the 2.0 second timeout
- held key keeps the transient visible while held and expires 2.0 seconds after
  release
- pressing B7 shows `B7 Play` / `Running` or `B7 Stop` / `Stopped`
- pressing B5 shows quantize detail text
- pressing A5-A8 shows rhythm target detail text
- audio regression proves multi-note / multi-pulse Subharmoniq output remains
  finite and non-silent unless intentionally muted

## Non-Goals

- no firmware changes
- no DAW/VST3 behavior changes
- no hardware flashing
- no new routing presets
- no Patch board behavior changes unless a shared test proves an existing
  contract was broken
- no permanent OLED tutorial text beyond clearer compact labels

## Approval State

Approved user decisions:

- use a temporary OLED zoom model
- zoom hold duration is 2.0 seconds after the last change
- held buttons keep the zoom visible while held
- `Q 12-JI` should be clarified as quantize / 12-note Just Intonation
- audio stopping after a couple notes must be investigated with tests as a
  separate runtime bug path
