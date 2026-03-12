# Field_MFOS_NoiseToaster Dual-Oscillator LFO Bank Design

## Goal

Redesign the `Field_MFOS_NoiseToaster` control surface so `MOD0` becomes a dual-oscillator performance bank and `MOD1` becomes an LFO-edit bank.

This design supersedes the earlier sparse `MOD1` concept that assigned a dedicated `Output Level` knob.

## Approved Direction

Use `SW1` to switch between two persistent control banks:

- `MOD0` for the main dual-oscillator voice
- `MOD1` for LFO-related controls only
- `SW2` remains `Panic`

The intent is:

- keep oscillator controls grouped together
- move only LFO-related modulation controls to `MOD1` for now
- avoid wasting a dedicated knob on output level

## MOD0 Layout

### Oscillator 1 Group

- `K1` = `OSC1` tune
- `B1` = `OSC1` wave

### Oscillator 2 Group

- `K2` = `OSC2` tune / detune
- `B2` = `OSC2` wave

`OSC2` is not just a fine detune trim. It should support:

- center = unison with `OSC1`
- small offsets = fine detune
- larger offsets = interval / coarse offset

### Shared Voice Controls

- `K3` = oscillator mixer
- `K4` = `AREG -> VCO` depth
- `K5` = `VCF` cutoff
- `K6` = `VCF` resonance
- `K7` = `AREG` attack
- `K8` = `AREG` release

- `B3` = `VCF` mod source (`LFO / AREG / Off`)
- `B4` = `Repeat / Manual`
- `B5` = `VCA Bypass`
- `B6..B8` = reserved, stateful

## MOD1 Layout

`MOD1` owns only the LFO edit surface.

### LFO Knobs

- `K1` = `LFO Rate`
- `K2` = `LFO -> VCO` depth
- `K3` = `LFO -> VCF` depth
- `K4..K8` = reserved, stateful

### LFO Buttons

- `B1` = `LFO` wave
- `B2` = `VCO Sync`
- `B3..B8` = reserved, stateful

## Signal Path

The voice becomes:

`armed note -> OSC1 + OSC2 tuning -> OSC mixer -> existing noise path -> VCF -> VCA/output`

Behavior:

- `OSC1` and `OSC2` both track the same armed note
- the oscillator mixer is pre-filter and oscillator-only
- the existing white-noise path remains separate for now

## Mixer Definition

`K3` is a true crossfade mixer between the two oscillators:

- full left = `OSC1` at `100%`
- center = `OSC1/OSC2` at `50/50`
- full right = `OSC2` at `100%`

This is not an output-volume control and not a noise blend control.

## Modulation Ownership

For this pass, only LFO-related controls move to `MOD1`.

That means:

- `LFO Rate` moves to `MOD1`
- `LFO -> VCO` depth moves to `MOD1`
- `LFO -> VCF` depth moves to `MOD1`
- `LFO` wave moves to `MOD1`
- `VCO Sync` moves to `MOD1`

These stay on `MOD0` for now:

- `AREG -> VCO` depth
- `VCF` mod source
- `AREG` attack/release
- `Repeat / Manual`
- `VCA Bypass`

## Bank Semantics

- `SW1` switches `MOD0/MOD1`
- both banks keep their own saved `K1..K8` values
- both banks keep their own saved `B1..B8` states
- reserved controls must still preserve state and LED behavior

`MOD1` LFO settings remain active after returning to `MOD0`.

That means the player edits LFO behavior in `MOD1`, then performs from `MOD0` while those LFO settings continue to affect the voice.

## Knob Pickup

Knob pickup / soft takeover remains required.

On bank switch:

- the synth uses the saved values of the selected bank
- knobs must not jump immediately to the raw hardware position
- each knob must reattach only after crossing the saved value of the selected bank

## State Examples

Examples that must remain true:

- `MOD0:B6 = Blink` and `MOD1:B6 = On` must restore exactly
- `MOD0:K1` can target `OSC1` tune while `MOD1:K1` targets `LFO Rate`
- switching banks must not corrupt the inactive bank's stored values

## Controls Explicitly Not Assigned

- `Output Level` must not occupy a dedicated knob
- future non-LFO modulation moves are deferred
- additional future voice features may use reserved controls later

## DSP Constraints

- keep control scanning and bank switching in the main loop
- keep live DSP-object mutation callback-owned
- keep the implementation real-time safe
- keep `SW2` panic unchanged
- keep `A1..A8` note-entry behavior unchanged

## Verification Targets

The implementation should verify:

- bank switching helpers
- per-bank button-state restore
- knob pickup behavior
- dual-oscillator mixer math
- firmware build success
- updated docs and reference artifact text
