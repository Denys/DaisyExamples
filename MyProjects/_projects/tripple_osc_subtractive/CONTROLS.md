# Controls - tripple_osc_subtractive

## Physical controls summary

| Hardware | Role |
|---|---|
| `K1-K8` | 8 knobs mapped to active mode page |
| `SW1` | Select `SW1` control page |
| `SW2` | Select `SW2` control page |
| `A1-A8`, `B1-B8` | Tri-state function keys (mode-dependent) |
| OLED | Active mode + parameter zoom feedback |

Press the active switch again to return to `DEFAULT`:

- `SW1` toggles `DEFAULT <-> SW1`
- `SW2` toggles `DEFAULT <-> SW2`

## Knob pages

Total logical controls = **24** (`3 modes × 8 knobs`).

## DEFAULT page (`MODE_DEFAULT`)

| Knob | Parameter | Description |
|---|---|---|
| `K1` | `OSC1<->OSC2 MIX` | Crossfade between oscillator 1 and 2 |
| `K2` | `OSC3 LEVEL` | Level of oscillator 3 contribution |
| `K3` | `OSC2 DETUNE` | Oscillator 2 detune amount |
| `K4` | `OSC3 DETUNE` | Oscillator 3 detune amount |
| `K5` | `FILTER CUTOFF` | Base filter cutoff |
| `K6` | `FILTER RES` | Filter resonance |
| `K7` | `AMP ATTACK` | Amplitude envelope attack |
| `K8` | `AMP RELEASE` | Amplitude envelope release |

## SW1 page (`MODE_SW1`)

| Knob | Parameter | Description |
|---|---|---|
| `K1` | `FILT ENV AMT` | Filter envelope modulation depth |
| `K2` | `FILT DECAY` | Filter envelope decay time |
| `K3` | `FILT SUSTAIN` | Filter envelope sustain |
| `K4` | `DRIVE` | Pre-filter saturation amount |
| `K5` | `LFO RATE` | LFO speed |
| `K6` | `LFO->PITCH` | LFO depth to pitch |
| `K7` | `LFO->FILTER` | LFO depth to filter cutoff |
| `K8` | `GLIDE` | Portamento time |

## SW2 page (`MODE_SW2`)

| Knob | Parameter | Description |
|---|---|---|
| `K1` | `AMP DECAY` | Amplitude envelope decay |
| `K2` | `AMP SUSTAIN` | Amplitude envelope sustain |
| `K3` | `NOISE LEVEL` | Noise layer level |
| `K4` | `SUB LEVEL` | Sub component amount |
| `K5` | `PAN SPREAD` | Stereo spread amount |
| `K6` | `VELOCITY AMT` | Velocity-to-amplitude depth |
| `K7` | `LFO->AMP` | LFO amplitude modulation depth |
| `K8` | `MASTER VOL` | Master output level |

## LED key alternate functions

Tri-state display for active key groups:

- `ON` = selected value
- `BLINK` = selectable option in current group
- `OFF` = unused in current mode

## DEFAULT key functions

| Key | Function |
|---|---|
| `A1-A3` | LFO waveform select: `SINE`, `TRI`, `SQUARE` |
| `A4-A7` | Oscillator waveform select (all 3 oscillators) |
| `A8` | Hard-sync toggle |
| `B1` | Legato toggle |

## SW1 key functions (alt1)

| Key | Function |
|---|---|
| `A1-A4` | Filter keytracking amount selection |
| `A5-A7` | Filter mode: LP / BP / HP |
| `B1-B4` | MIDI channel select (`CH1`-`CH4`) |
| `B5` | MIDI `OMNI` mode |

## SW2 key functions (alt2)

| Key | Function |
|---|---|
| `A1-A4` | Transpose preset: `-12`, `0`, `+12`, `+24` semitones |
| `A5-A8` | LFO target mode: pitch / filter / amp / all |

## OLED pages and zoom

- OLED top line displays current control mode (`DEFAULT`, `SW1`, `SW2`).
- OLED middle line summarizes current key-mode selection state.
- When a knob is moved, the display zooms to the edited parameter name and value.
- When a mode key changes a toggle/selection, the display briefly shows that setting and its new value/state.
- After timeout, OLED returns to compact summary lines.

## MIDI notes

This synth expects external MIDI note/gate input. Daisy keybed is dedicated to control functions in this project.
