# Field_MFOS_NoiseToaster Controls

## Overview

`Field_MFOS_NoiseToaster` is a direct, single-page Daisy Field patch. There are no alternate pages, presets, or menu layers. All knobs are live all the time.

The synth is monophonic. The most recently triggered A-row key becomes the active note.

## Knobs

| Knob | Parameter | Behavior In Code | Practical Result |
|------|-----------|------------------|------------------|
| `K1` | Coarse Tune | `note * 2^((k - 0.5) * 2.0)` | About `-1` to `+1` octave around the selected note |
| `K2` | Oscillator/Noise Mix | `0.0..1.0` crossfade | `0` is pure oscillator, `1` is pure white noise |
| `K3` | Filter Cutoff | Base cutoff term, about `40 + knob * 9000 Hz` before modulation | Sets the low-pass brightness center |
| `K4` | Resonance | `0.05 + 0.9 * knob` | Mild to strong SVF resonance |
| `K5` | AR Depth | `0.0..1.0` | Increases AR modulation amount to pitch or filter |
| `K6` | LFO Rate | `0.05 + 20 * knob^2` Hz | Slow drift to fast audio-rate-adjacent modulation |
| `K7` | LFO Depth | Shared modulation depth | Increases both pitch vibrato and filter sweep depth |
| `K8` | Output Level | Final gain scalar | Controls overall output level |

## A Row Note Map

| Key | MIDI Note | Pitch |
|-----|-----------|-------|
| `A1` | `48` | `C3` |
| `A2` | `50` | `D3` |
| `A3` | `52` | `E3` |
| `A4` | `53` | `F3` |
| `A5` | `55` | `G3` |
| `A6` | `57` | `A3` |
| `A7` | `59` | `B3` |
| `A8` | `60` | `C4` |

### Note Behavior

- Each A-row rising edge calls the monophonic note trigger.
- The latest played key becomes the active pitch.
- Releasing the active key ends the gate when hold is off.
- Releasing a non-active key has no effect.
- There is no note stack; an earlier held note is not restored automatically.

## B Row And Switches

| Control | Function | Behavior |
|---------|----------|----------|
| `B1` | Waveform select | Sets oscillator to saw |
| `B2` | Waveform select | Sets oscillator to square |
| `B3` | Waveform select | Sets oscillator to triangle |
| `B4` | Hold toggle | Toggles drone/latched gate |
| `B5` | AR destination toggle | Switches AR contour between pitch and filter |
| `SW1` | Hold toggle | Duplicates the `B4` hold function |
| `SW2` | Panic | Forces gate off and disables hold |

## Modulation Behavior

### AR Contour

- The AR contour is fixed-time and always running from the same internal envelope block.
- `B5` chooses the destination:
  - `Pitch`: AR depth raises oscillator pitch
  - `Filter`: AR depth opens filter cutoff

### LFO

- LFO waveform is fixed to sine in the current code.
- `K6` sets rate.
- `K7` sets shared depth.
- Pitch and filter are both modulated together with fixed internal scaling.
- There is no front-panel LFO routing selector in this project.

## OLED Display

The OLED is a compact status panel rather than a parameter editor.

| Row | Content |
|-----|---------|
| `0` | Title: `MFOS NOISE TOASTER` |
| `14` | Current waveform and hold state |
| `28` | Quick hint: `K1 Tune K2 Mix K3 Cut` |
| `42` | Quick hint: `A1-A8 Notes B1-3 Wave` |
| `54` | Quick hint: `B4 Hold SW2 Panic` |

The display updates continuously in the main loop.

## Serial Boot Log

At boot, the project prints short status lines with the seed logger:

- `Field_MFOS_NoiseToaster ready`
- `A1-A8 notes | B1/B2/B3 waveform | B4 hold | B5 AR target`

## Fixed Internal Settings

These are not exposed on the hardware controls in the current build:

| Block | Setting | Value |
|-------|---------|-------|
| VCA ADSR | Attack | `0.002 s` |
| VCA ADSR | Decay | `0.12 s` |
| VCA ADSR | Sustain | `0.75` |
| VCA ADSR | Release | `0.22 s` |
| AR Contour | Attack | `0.005 s` |
| AR Contour | Decay | `0.28 s` |
| LFO | Waveform | `Sine` |
| Output | Routing | Same signal to left and right outputs |

## Analog Counterpart Controls Not Yet Exposed

Compared with the original MFOS Noise Toaster panel, these analog-era controls or routing behaviors are still absent or simplified in the current Daisy Field firmware:

- `AREG Attack` and `AREG Release` are fixed internally instead of being live controls.
- `Repeat / Manual` and `Manual Gate` are replaced by the current note-key plus hold workflow.
- `LFO Waveform Select` is not implemented; the LFO is fixed to sine.
- `VCO Sync` is not implemented.
- `VCF Mod Source` is not a true `LFO / Off / AREG` selector.
- `VCF Mod Depth` is not independent from the current simplified modulation model.
- `VCA Bypass` is not implemented.
- `VCA` amplitude is shaped by a separate fixed ADSR rather than the same AREG block used for pitch/filter modulation.
- `VCF input selection` is simplified into the `K2` oscillator/noise crossfade instead of the analog switch-based source selection.

## Operational Notes

- `B4` and `SW1` are redundant by design. Both latch the same hold state.
- If hold is enabled, the VCA envelope continues to run from the held gate state.
- `SW2` panic clears the gate and disables hold, but it does not reset waveform or modulation mode.
- The filter is always used; there is no bypass path.
- The noise source is continuously available through `K2`, not a discrete on/off source.
