# Field_MFOS_NoiseToaster Controls

## Overview

`Field_MFOS_NoiseToaster` is now a single-page Daisy Field patch with no menus or alternate pages. All eight knobs are live all the time, and the B-row is used as a small analog-style mode strip rather than as direct one-shot selectors.

The synth is monophonic. A note remains armed after an `A` key press until another note is selected or `SW2` panic clears it.

## Knobs

| Knob | Parameter | Behavior In Code | Practical Result |
|------|-----------|------------------|------------------|
| `K1` | VCO Frequency / Coarse Tune | `note * 2^((k - 0.5) * 2.0)` before modulation | About `-1` to `+1` octave around the armed note |
| `K2` | VCO LFO Depth | `0.0..1.0` depth into oscillator pitch | Controls vibrato or stepped pitch motion depending on the selected LFO wave |
| `K3` | VCO AREG Depth | `0.0..1.0` depth into oscillator pitch | Adds contour pitch sweep from the same AREG that drives the patch |
| `K4` | VCF Cutoff | About `50 + knob^2 * 8000 Hz` before modulation | Sets the base brightness of the low-pass filter |
| `K5` | VCF Resonance | `0.05 + 0.92 * knob` | Mild to strong resonance |
| `K6` | VCF Mod Depth | `0.0..1.0` depth for the source selected on `B3` | Sets how much the filter moves from LFO or AREG |
| `K7` | AREG Attack | `0.002 + knob^2 * 1.20 s` | About `2 ms` to `1202 ms` attack |
| `K8` | AREG Release | `0.010 + knob^2 * 1.80 s` | About `10 ms` to `1810 ms` release |

## A Row Note Map

| Key | MIDI Note | Pitch | Behavior |
|-----|-----------|-------|----------|
| `A1` | `48` | `C3` | Arms `C3` and triggers the AREG |
| `A2` | `50` | `D3` | Arms `D3` and triggers the AREG |
| `A3` | `52` | `E3` | Arms `E3` and triggers the AREG |
| `A4` | `53` | `F3` | Arms `F3` and triggers the AREG |
| `A5` | `55` | `G3` | Arms `G3` and triggers the AREG |
| `A6` | `57` | `A3` | Arms `A3` and triggers the AREG |
| `A7` | `59` | `B3` | Arms `B3` and triggers the AREG |
| `A8` | `60` | `C4` | Arms `C4` and triggers the AREG |

### Note Behavior

- A-row keys are trigger actions, not sustained gate-hold actions.
- Releasing an A-row key does not silence the synth.
- If `B4` is in `Manual`, the AREG runs once per key press unless retriggered again.
- If `B4` is in `Repeat`, the AREG keeps cycling on the armed note until `SW2` panic or a new note is selected.
- The currently armed note LED stays lit on the A-row.

## B Row And Switches

| Control | Function | States | LED Meaning |
|---------|----------|--------|-------------|
| `B1` | VCO output cycle | `Saw -> Square -> Triangle` | `On = Saw`, `Blink = Square`, `Off = Triangle` |
| `B2` | LFO wave cycle | `Sine -> Square -> Triangle` | `On = Sine`, `Blink = Square`, `Off = Triangle` |
| `B3` | VCF mod source cycle | `LFO -> AREG -> Off` | `On = LFO`, `Blink = AREG`, `Off = Off` |
| `B4` | Repeat / Manual | toggle | `On = Repeat`, `Off = Manual` |
| `B5` | VCA Bypass | toggle | `On = Bypass`, `Off = AREG VCA` |
| `B6` | Unused | none | `Off` |
| `B7` | Unused | none | `Off` |
| `B8` | Unused | none | `Off` |
| `SW1` | Manual Gate | retrigger | LED glows dimly when a note is armed |
| `SW2` | Panic | clear note and stop repeat | LED remains off in normal operation |

## Modulation And Envelope Behavior

### AREG

- The AREG is the central contour block in the current firmware.
- `K7` sets attack and `K8` sets release.
- Every A-row note press retriggers the AREG.
- `SW1` retriggers the AREG on the currently armed note.
- `B4` repeat mode retriggers the AREG automatically whenever it falls idle and a note is armed.

### Pitch Modulation

- `K2` sets `LFO -> VCO` depth.
- `K3` sets `AREG -> VCO` depth.
- Both pitch modulation paths are active together.

### Filter Modulation

- `B3` chooses the filter modulation source:
  - `LFO`
  - `AREG`
  - `Off`
- `K6` sets the modulation depth for that selected source.

### VCA Behavior

- With `B5` off, the low-pass signal is multiplied by the current AREG value.
- With `B5` on, the VCA is bypassed and the armed note becomes a continuous tone through the filter path.
- Even when `B5` bypass is on, the AREG can still modulate pitch or filter if the corresponding depths are up.

## Fixed Internal Settings

These are not exposed on the panel in the current pass:

| Block | Setting | Value |
|-------|---------|-------|
| LFO | Rate | `2.2 Hz` |
| Noise | Pre-filter blend | `18%` |
| Output | Level | `72%` |
| AREG | Curve | `-18.0` |
| Output | Routing | Same signal to left and right outputs |

## OLED Display

The OLED is now a live parameter display rather than a static instruction card.

### Idle Overview

When no knob is moving, the OLED shows:

- title
- current `VCO`, `LFO`, and `VCF mod source` modes
- current armed note
- `Repeat` and `Bypass` state
- all eight knob functions with live values
- fixed internal `LFO` rate and output level summary

### Focused Parameter View

When a knob moves far enough:

- the OLED zooms that parameter name
- shows a larger value readout
- shows the current mode summary underneath
- shows a progress bar
- returns to the overview after about `1.4 s` of inactivity

The old hint rows:

- `A1-A8 Notes`
- `B1-3 Wave`
- `B4 Hold`
- `SW2 Panic`

are intentionally removed from the display.

## Serial Boot Log

At boot, the project prints short status lines with the seed logger:

- `Field_MFOS_NoiseToaster ready`
- `A1-A8 note select+trigger | B1 VCO | B2 LFO | B3 VCF src`
- `B4 repeat | B5 bypass | SW1 manual gate | SW2 panic`
- fixed internal `LFO`, `OUT`, and `Noise` summary

## Analog Counterpart Controls Still Missing

Compared with the original MFOS Noise Toaster panel, these analog controls or routing behaviors are still absent or simplified:

- discrete white-noise on/off switching
- live `LFO Rate`
- live `Output Level`
- `VCO Sync`
- a closer replica of the analog `VCF input select` routing
- the original speaker / internal amp section
- a pure analog-style no-key interaction mode that removes the Daisy Field note-key adaptation

## Operational Notes

- If no note is armed, `SW1` manual gate does nothing.
- `SW2` clears the armed note and stops repeat behavior immediately.
- `B5` bypass does not bypass the filter; it bypasses only AREG amplitude shaping.
- White noise is always present at a fixed pre-filter blend in this pass.
