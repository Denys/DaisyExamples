# Pod MultiFX LFO Page Design

**Date:** 2026-04-14

## Goal

Extend `Pod_MultiFX_Chain` with a fourth `LFO` page that adds front-of-chain
amplitude modulation, waveform selection from the encoder push, and dedicated
`led2` feedback, while preserving per-page state with soft takeover.

## Scope

- Add `LFO` as a fourth edit page
- Keep encoder turn for page selection
- Use encoder push on the `LFO` page to cycle `Sine -> Triangle -> Square`
- Use `led2` color to show LFO waveform:
  - Green = Sine
  - Blue = Triangle
  - Red = Square
- Use `led2` intensity to reflect the current LFO rate
- Add `SW1` short-press page bypass on the `LFO` page
- Add `SW1` hold on the `LFO` page for fine-edit controls
- Keep stored values per page and per logical layer with soft takeover
- Update tests, firmware header comments, README, and implementation plan

## Page Order

- Tube
- Delay
- Reverb
- LFO

## Signal Flow

The edit page order is not the same as the DSP order.

Actual DSP chain:

- LFO -> Tube -> Delay -> Reverb

This keeps the LFO acting as a tremolo-style input modulator while preserving
the existing page order for the three effect pages.

## Controls

### Encoder

- Turn: select edit page
- Push on LFO page: cycle waveform
- Push on non-LFO pages: no behavior change

### Switches

- `SW1` short press: toggle bypass for the selected page
- `SW1` hold on Tube: temporary shift layer for `Bias / Distortion`
- `SW1` hold on LFO: temporary fine layer for `Amplitude / Rate`
- `SW2`: global bypass for the entire chain

### Knobs

#### Tube

- Default:
  - `Knob 1 = Drive`
  - `Knob 2 = Mix`
- Shift while holding `SW1`:
  - `Knob 1 = Bias`
  - `Knob 2 = Distortion`

#### Delay

- `Knob 1 = Time`
- `Knob 2 = Feedback`

#### Reverb

- `Knob 1 = Decay`
- `Knob 2 = Mix`

#### LFO

- Default:
  - `Knob 1 = Amplitude coarse`
  - `Knob 2 = Rate coarse`
- Shift while holding `SW1`:
  - `Knob 1 = Amplitude fine`
  - `Knob 2 = Rate fine`

## LFO Parameter Model

### Waveforms

- Sine
- Triangle
- Square

### Amplitude

- Final amplitude range: `0.0 -> 1.0`
- Mapping: exponential

### Rate

The user requested `0.1s -> 5s`. Treat this as LFO period, not Hertz.

- Fastest period: `0.1 s`
- Slowest period: `5.0 s`
- Internal oscillator frequency:
  - fastest = `10 Hz`
  - slowest = `0.2 Hz`
- Mapping: exponential

### Coarse/Fine Combination

The LFO page stores coarse and fine values independently.

- Coarse knobs define the main target
- Fine knobs are centered trims around the coarse target
- Fine layer uses soft takeover just like Tube shift
- Switching between LFO coarse and fine layers does not jump the effective
  values to the raw knob positions

## Modulation Behavior

Use the LFO as a unipolar gain stage on the input signal.

- Convert waveform output from bipolar `[-1, 1]` to unipolar `[0, 1]`
- Apply tremolo-style gain:
  - `gain = (1 - depth) + depth * unipolar_wave`
- Depth `0` means no modulation
- Depth `1` means full waveform-controlled modulation

This keeps the dry floor stable and makes depth intuitive.

## LED Behavior

### `led1`

Continue to show selected page color.

- Red = Tube
- Green = Delay
- Blue = Reverb
- Yellow = LFO

Dim `led1` when the selected page is bypassed or global bypass is enabled.

### `led2`

Show the LFO waveform and rate.

- Green = Sine
- Blue = Triangle
- Red = Square
- Brightness tracks effective LFO rate

If the LFO page or whole chain is bypassed, dim `led2` heavily so the state is
still visible without implying active modulation.

## State Model

Required stored state additions:

- LFO page primary layer:
  - amplitude coarse
  - rate coarse
- LFO page shifted layer:
  - amplitude fine
  - rate fine
- LFO waveform index

The `SW1` hold helpers should become page-agnostic so they support both Tube and
LFO shift behavior.

## Verification Strategy

- Update the host-side state test first so it fails on:
  - four-page wrap
  - encoder push waveform cycling
  - LFO coarse/fine state persistence
  - LFO short-press vs hold behavior
  - LFO mapping ranges
- Re-run host-side test after helper and firmware updates
- Run Daisy QAE lint
- Run clean firmware rebuild
