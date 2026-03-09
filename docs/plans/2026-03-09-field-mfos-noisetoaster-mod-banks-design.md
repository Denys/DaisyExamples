# Field_MFOS_NoiseToaster MOD Bank Design

## Goal

Add a next-revision control-bank system to `Field_MFOS_NoiseToaster` where `SW1` switches between `MOD0` and `MOD1`, and each bank owns its own `K1..K8` values and `B1..B8` discrete states.

## Approved Direction

Use `SW1` as a bank switch instead of `Manual Gate`.

- `MOD0` remains the current live control surface.
- `MOD1` becomes a sparse analog-recovery bank that only exposes genuinely missing analog controls.
- `SW2` remains `Panic`.

This is intentionally no longer a strict description of the current firmware. It is the approved next control model.

## Bank Layout

### MOD0

`MOD0` keeps the current mappings:

- `K1` VCO Frequency / Coarse Tune
- `K2` VCO LFO Depth
- `K3` VCO AREG Depth
- `K4` VCF Cutoff
- `K5` VCF Resonance
- `K6` VCF Mod Depth
- `K7` AREG Attack
- `K8` AREG Release

- `B1` VCO wave cycle
- `B2` LFO wave cycle
- `B3` VCF mod source cycle
- `B4` Repeat / Manual
- `B5` VCA Bypass
- `B6..B8` reserved, but stateful

### MOD1

`MOD1` is intentionally parsimonious.

Knobs:

- `K1` LFO Rate
- `K2` Output Level
- `K3..K8` reserved for future features

Buttons:

- `B1` White Noise switch
- `B2` VCO Sync switch
- `B3` VCF Input Select
- `B4..B8` reserved, but stateful

## State Model

Each bank must persist independently:

- knob parameter values for `K1..K8`
- button states for `B1..B8`

The saved state is per bank, not global.

Example:

- `MOD0:B6 = Blink`
- `MOD1:B6 = On`

Switching banks must restore those exact states and LED behaviors. The firmware must save logical button state, not just momentary LED brightness.

## Button-State Representation

To support the required per-bank persistence, all `B1..B8` entries should use a bank-local discrete state representation.

That representation must be rich enough to preserve:

- three-state selectors such as current `B1/B2/B3`
- binary toggles such as current `B4/B5`
- reserved states such as `B6..B8` that may still be `On`, `Blink`, or `Off`

The current audio/control meaning for a button is derived from that stored state only when the active bank uses the button for a real function.

Reserved buttons do not affect audio, but their state and LED behavior are still restored exactly when switching banks.

## Knob Behavior

Each bank also owns saved values for `K1..K8`.

On bank switch:

- the active DSP targets change to the selected bank's saved values
- knob movement must not cause immediate parameter jumps based on raw physical position
- each knob should use soft takeover / pickup behavior before it starts writing into the newly selected bank

This makes `MOD0` and `MOD1` behave like real saved banks instead of temporary overlays.

## DSP And UI Rules

- Keep all live DSP-object mutation in callback-owned handling.
- Keep control scanning and bank-switch decisions in the main loop.
- Render `B` LEDs from the active bank's saved button state.
- Update the OLED so the active bank and active labels are visible.
- `A1..A8` note behavior stays unchanged.
- `SW2` panic behavior stays unchanged.

## Missing Analog Controls Assigned To MOD1

These are the missing analog-original controls selected for `MOD1`:

- live `LFO Rate`
- live `Output Level`
- `White Noise` switch control
- `VCO Sync`
- `VCF Input Select`

Other future analog-faithful or digital-extension features should remain unassigned until they are explicitly designed.

## Error Handling And Constraints

- Bank switching must not corrupt `MOD0` state while editing `MOD1`, or vice versa.
- Reserved controls must be safe no-ops in the audio path.
- If a bank feature is unimplemented in DSP, its control should still preserve state and LED indication without affecting sound.
- The design must remain Windows-buildable with the existing `make` caveat unchanged.

## Testing Strategy

Add host-side tests around pure helpers for:

- bank toggling
- three-state button cycling
- per-bank state helpers
- soft-takeover / pickup decisions

Then rebuild the firmware and update the docs so the project references match the new banked control model.
