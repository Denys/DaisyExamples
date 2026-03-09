# Field_MFOS_NoiseToaster

`Field_MFOS_NoiseToaster` is a Daisy Field mono synth adapted from the MFOS Noise Toaster with a more analog-style control model than the first digital draft. The current build centers the patch around one `AREG`, repeat/manual behavior, manual retriggering, and three front-panel selector keys that cycle modes with LED on/blink/off feedback.

## What It Does

- Monophonic `A1..A8` note select plus trigger
- `B1` three-state VCO output cycling: `Saw / Square / Triangle`
- `B2` three-state LFO wave cycling: `Sine / Square / Triangle`
- `B3` three-state VCF modulation source cycling: `LFO / AREG / Off`
- Live `AREG` attack and release on `K7` and `K8`
- `B4` repeat/manual envelope behavior
- `SW1` manual gate retrigger
- `B5` VCA bypass
- Low-pass SVF with source-selected modulation
- OLED idle overview plus focused parameter zoom while a knob is moving
- Stereo mirrored output

## Project Docs

- [CONTROLS.md](./CONTROLS.md)
- [DIAGRAMS.md](./DIAGRAMS.md)
- [GENERATION_PROMPT.md](./GENERATION_PROMPT.md)

## Analog Reference

The analog comparison reference for this project is still:

- [Make  Analog Synthesizers - Ray Wilson.pdf](./Make%20%20Analog%20Synthesizers%20-%20Ray%20Wilson.pdf)
- [4-2_Noise_Toaster_Block_Diagram.png](./4-2_Noise_Toaster_Block_Diagram.png)

The current firmware is closer to that reference than the earlier Field version, but it is still an adaptation, not a literal panel-for-panel replica.

## Architecture Summary

The current firmware is an `AREG`-centric monophonic voice:

1. `A1..A8` select a note and trigger the contour
2. `K1` coarse-tunes the selected pitch
3. The VCO runs in one of three `B1`-selected waveforms
4. A fixed `18%` white-noise blend is mixed in before the filter
5. The SVF low-pass filter uses `K4` cutoff, `K5` resonance, and `K6` source-selected modulation depth
6. The same `AREG` modulates pitch and, when selected on `B3`, the filter
7. The same `AREG` also controls amplitude unless `B5` bypass is enabled
8. Output is sent to both channels at a fixed internal level

`B4` repeat mode retriggers the `AREG` automatically whenever it falls idle and a note is armed. In manual mode, a note press or `SW1` creates a single contour cycle.

## Quick Controls

### Knobs

- `K1`: VCO frequency / coarse tune
- `K2`: VCO LFO modulation depth
- `K3`: VCO AREG modulation depth
- `K4`: VCF cutoff
- `K5`: VCF resonance
- `K6`: VCF modulation depth for the source selected on `B3`
- `K7`: AREG attack
- `K8`: AREG release

### Keyboard And Switches

- `A1..A8`: select and trigger notes `C3, D3, E3, F3, G3, A3, B3, C4`
- `B1`: cycle VCO output `Saw -> Square -> Triangle`
- `B2`: cycle LFO wave `Sine -> Square -> Triangle`
- `B3`: cycle VCF mod source `LFO -> AREG -> Off`
- `B4`: toggle `Repeat / Manual`
- `B5`: toggle `VCA Bypass`
- `B6..B8`: unused in the current firmware
- `SW1`: manual gate retrigger for the armed note
- `SW2`: panic, clear the armed note and stop repeating activity

## Defaults And Startup Behavior

- Boot VCO wave: `Saw`
- Boot LFO wave: `Sine`
- Boot VCF mod source: `LFO`
- Boot repeat mode: `Manual`
- Boot VCA bypass: `Off`
- Boot state: no note armed
- Fixed internal LFO rate: `2.2 Hz`
- Fixed white-noise blend: `18%`
- Fixed output level: `72%`
- Output is duplicated to left and right channels
- OLED shows all knob functions in idle view and zooms the active parameter while a knob is moving
- Serial boot log prints the new mode mapping and fixed internal settings

## Fixed Internal Settings

These are hard-coded in the current pass:

- LFO rate: `2.2 Hz`
- White-noise blend: `18%`
- Output level: `72%`
- AREG curve: `-18.0`

`K7` and `K8` do control `AREG` timing live, but the `LFO rate` and `output level` are fixed in this version to make room for the new analog-style panel functions on the eight available Field knobs.

## Analog-Faithfulness Gaps

Compared with the original MFOS Noise Toaster, the current Field build still differs in these important ways:

- It still uses an `A1..A8` note-key adaptation instead of the pure analog `Freq knob + Manual Gate` interaction model.
- White noise is a fixed blend, not a discrete front-panel on/off source.
- The analog `VCF input select` behavior is still simplified into one selected VCO waveform plus a fixed noise blend.
- `VCO Sync` is still not implemented.
- `LFO Rate` is now fixed internally rather than being a live panel control.
- `Output Level` is fixed internally rather than exposed as a knob.
- The original speaker / amp / line-out switching is not represented on Daisy Field.

## Highest-Value Next Improvements

After this pass, the best analog-faithful improvements per unit of work are now:

1. Add white-noise on/off control instead of the fixed blend.
2. Restore a live `LFO Rate` control.
3. Restore a live `Output Level` control.
4. Add `VCO Sync` driven from the LFO reset path.
5. Move the pre-filter source routing closer to the analog `VCF input select` behavior.
6. Add an optional no-key mode that behaves more like pure manual-gate analog operation.

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
- No preset or patch memory system
- No note stack or last-note recovery
- `B6..B8` are currently unused
- No discrete white-noise switch yet
- No live LFO-rate control in this pass
- No live output-level control in this pass
- No VCO sync yet
- No literal analog speaker-stage replica
