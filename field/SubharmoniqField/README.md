# SubharmoniqField

SubharmoniqField is a Daisy Field firmware instrument inspired by the
Subharmonicon concept. It is not a clone. Round 1 focuses on the transferable
musical structure: two oscillator families, four subharmonic voices, two
4-step sequencers, four integer rhythm dividers, quantization, Field-native
hands-on control, and a compact OLED menu.

The firmware uses the portable DaisyHost `DaisySubharmoniqCore` through a
Field adapter. Audio rendering stays in the audio callback; controls, MIDI,
CV/Gate, LEDs, and OLED updates run in the main loop.

The 2026-04-26 follow-ups fixed the build-verified no-audio path in two
steps: pressing `B7` now starts the portable core's internal tempo clock, and
the core now uses a real attack/decay envelope with louder playable defaults.
The Field adapter also uses pickup-style knob startup, so the initial physical
K1-K8 positions do not overwrite the safe sound before a knob is moved.

## Build

From this directory:

```sh
make
```

## Flash

With the Daisy Field connected through ST-Link:

```sh
make program
```

## Validation Status

This project can be marked hardware-validated only after:

- `make` succeeds
- QAE validation succeeds
- `make program` succeeds through ST-Link
- the manual checklist in `CONTROLS.md` is completed and dated

Until then, describe the result as build/QAE/ST-Link flash-verified only, not
fully hardware-validated.

Latest verification evidence from 2026-04-26: `make` was up to date after a
fresh firmware build with FLASH `124864 B` / `95.26%`; QAE passed with
`0 error(s), 0 warning(s)`; `make program` passed through ST-Link with STLINK
`V3J7M2`, target voltage `3.263618`, OpenOCD `** Verified OK **`, and target
reset. The manual checklist in `CONTROLS.md` is still pending.

## Round 1 Scope

- Six oscillator sources: `VCO1`, `VCO1 Sub1`, `VCO1 Sub2`, `VCO2`,
  `VCO2 Sub1`, `VCO2 Sub2`
- Subharmonic divisors clamped to `1..16`
- Two 4-step sequencers
- Four rhythm dividers with `off / seq1 / seq2 / both` routing
- Quantize modes: `Off`, `12-ET`, `8-ET`, `12-JI`, `8-JI`
- SVF-style low-pass filter only
- VCF and VCA AD envelopes
- MIDI note input and MIDI clock pulse handling
- CV/Gate patch-style inputs and two sequencer CV outputs

Round 2 should add selectable filter models/modes: `SVF LPF`, `SVF BPF`, and
ladder-style `LPF`.
