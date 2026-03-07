# Field_MFOS_NoiseToaster

`Field_MFOS_NoiseToaster` is a Daisy Field mono synth inspired by the MFOS Noise Toaster signal path and performance style. It keeps the patch compact and immediate: one oscillator, one noise source, one filter, one VCA envelope, and a small set of front-panel performance controls.

## What It Does

- Single VCO with three waveforms: saw, square, triangle
- White noise source with continuous oscillator/noise crossfade
- Fixed AR contour routed to either pitch or filter
- Shared LFO depth that simultaneously affects pitch vibrato and filter sweep
- Resonant SVF low-pass filter
- Fixed ADSR VCA envelope
- Daisy Field OLED status screen
- Stereo mirrored output

## Project Docs

- [CONTROLS.md](./CONTROLS.md)
- [DIAGRAMS.md](./DIAGRAMS.md)
- [GENERATION_PROMPT.md](./GENERATION_PROMPT.md)

## Analog Reference

The analog reference for this project comparison is the original MFOS Noise Toaster documentation in:

- [Make  Analog Synthesizers - Ray Wilson.pdf](./Make%20%20Analog%20Synthesizers%20-%20Ray%20Wilson.pdf)
- [4-2_Noise_Toaster_Block_Diagram.png](./4-2_Noise_Toaster_Block_Diagram.png)

The current Daisy Field patch is inspired by that signal path, but it is not yet a 1:1 digital replica of the analog control architecture.

## Architecture Summary

The synth runs as a single monophonic voice. Each key press selects one note from the Daisy Field A-row note map, then the audio path applies:

1. VCO waveform generation
2. Oscillator/noise blend
3. Low-pass SVF filtering
4. Fixed ADSR VCA envelope
5. Output gain

The AR contour is switchable between pitch modulation and filter modulation. The LFO does not have a routing selector in the current implementation; one depth control drives both pitch and filter modulation with fixed internal scaling.

## Quick Controls

### Knobs

- `K1`: coarse tune, approximately +/- 1 octave around the selected note
- `K2`: oscillator/noise mix
- `K3`: base filter cutoff
- `K4`: filter resonance
- `K5`: AR contour depth
- `K6`: LFO rate
- `K7`: shared LFO depth
- `K8`: output level

### Keyboard And Switches

- `A1..A8`: play notes `C3, D3, E3, F3, G3, A3, B3, C4`
- `B1/B2/B3`: select waveform `Saw / Square / Triangle`
- `B4`: toggle hold/drone
- `B5`: toggle AR destination `Pitch / Filter`
- `SW1`: duplicate hold toggle
- `SW2`: panic, gate off and hold off

## Defaults And Startup Behavior

- Boot waveform: `Saw`
- Boot AR destination: `Pitch`
- Hold starts `Off`
- Output is duplicated to left and right channels
- OLED shows waveform, hold state, and quick control reminders
- Serial boot log prints a short startup summary over the seed logger

The synth is intentionally simple:

- It is monophonic, not polyphonic
- The most recent played A-row note becomes the active pitch
- Releasing an older held key does not restore a previous note
- Envelope times are fixed in code; only depth, filter, mix, rate, and level are on the panel

## Fixed Internal Envelope Settings

These are hard-coded in the current firmware:

- VCA ADSR: attack `2 ms`, decay `120 ms`, sustain `0.75`, release `220 ms`
- AR contour: attack `5 ms`, decay `280 ms`

## Analog-Faithfulness Gaps

Compared with the original MFOS Noise Toaster described in the Ray Wilson reference, the current Field build still differs in several structural ways:

- It is driven from an `A1..A8` note-key layout instead of centering on `Manual Gate` and `Repeat/Manual` interaction.
- The AR envelope times are fixed in code instead of being front-panel `Attack` and `Release` controls.
- The LFO is fixed to sine instead of offering the analog `Square / Differentiated Square / Integrated Square` selection.
- The filter does not have a true `Mod Source` selector with dedicated modulation depth; LFO-to-filter is effectively always present.
- The VCA is driven by a separate fixed ADSR instead of being directly shaped by the same AREG block, and there is no VCA bypass control.
- VCO sync is not implemented.
- Oscillator and noise input selection are simplified into a continuous crossfade instead of the analog switch-based routing.

## Highest-Value Analog Improvements

After reviewing the Ray Wilson PDF, the best analog-faithful improvement per unit of work is now:

1. Expose live AREG `Attack` and `Release` controls.
2. Add `Repeat / Manual` behavior and a true `Manual Gate` action.
3. Replace the fixed sine LFO with selectable `Square / Differentiated Square / Integrated Square` modes.
4. Add a real VCF `Mod Source` selector and dedicated `Mod Depth`.
5. Add `VCA Bypass` and move amplitude behavior closer to the analog AREG-driven VCA model.
6. Add VCO sync behavior driven from the LFO square output.
7. Replace the current oscillator/noise crossfade with a more analog-like input-selection model if strict control fidelity is the goal.

## Build

```bash
cd MyProjects/_projects/Field_MFOS_NoiseToaster
make
```

`Makefile` follows the standard DaisyExamples project layout and expects local `libDaisy` and `DaisySP` checkouts at:

- `../../../libDaisy`
- `../../../DaisySP`

## Known Limitations

- No MIDI input handling in this project
- No patch memory or preset system
- No held-note stack or note priority recovery
- No dedicated LFO routing switch; pitch and filter modulation are driven together
- No manual gate / repeat behavior matching the analog Noise Toaster
- No exposed AR attack and release controls
- No LFO waveform selection, VCO sync, or VCA bypass
- Build validation depends on local libDaisy core files being present
