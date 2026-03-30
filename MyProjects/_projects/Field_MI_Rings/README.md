# Field_MI_Rings

## Purpose

`Field_MI_Rings` is a Daisy Field hybrid Rings-style instrument for external MIDI control.
This v1 build is a standalone `2-voice poly` voice with an internal exciter, not an
external-audio resonator processor yet.

The sound engine uses local DaisySP physical-model blocks:

- `ModalVoice` for struck resonant bodies
- `StringVoice` for plucked and bowed string behavior
- `Resonator` plus a custom exciter for a sympathetic-style model

## Main Controls

| Control | Function | Notes |
|---|---|---|
| `K1` | `Structure` | Sets stiffness / nonlinearity / body character |
| `K2` | `Brightness` | Global tone brightness |
| `K3` | `Damping` | Decay length / damping |
| `K4` | `Position` | Pickup / exciter mix emphasis |
| `K5` | `Spread` | Stereo width amount |
| `K6` | `Exciter` | Internal strike intensity baseline |
| `K7` | `Macro` | Model-dependent tone macro |
| `K8` | `Interaction` | Cross-voice coupling / resonance interaction |

`K8` is intentionally not a generic output level control. Hardware volume already covers that role.

## MIDI / OXI One Mapping

| MIDI | Target |
|---|---|
| `Note On / Note Off` | 2-voice note allocation |
| Velocity | Internal exciter strength |
| `CC16` | `Structure` |
| `CC74` | `Brightness` |
| `CC71` | `Damping` |
| `CC17` | `Position` |
| `CC64` | Sustain pedal / sustained excitation latch |

When an external CC changes one of the mapped parameters, the stored value is updated and the
corresponding hardware knob must pass through that value before it takes ownership again.

## Keybed Controls

The Field keybed is used for controls, not note entry.

| Control | Function | LED State |
|---|---|---|
| `A1` | Select `Modal` model | `On` when active |
| `A2` | Select `String` model | `On` when active |
| `A3` | Select `Sympathetic` model | `On` when active |
| `A4` | Cycle spread mode | `Off = Narrow`, `Blink = Wide`, `On = Split` |
| `B1` | OLED main page | `On` when active |
| `B2` | OLED MIDI page | `On` when active |
| `B3` | OLED help page | `On` when active |
| `B7` | MIDI activity indicator | `Blink` on recent RX |
| `B8` | Panic | `On` while sustain pedal is latched |
| `SW1` | Cycle OLED page | Utility shortcut |
| `SW2` | Panic | Stops held and latched voices |

## Knob LEDs

- Knob LEDs show the stored logical parameter values.
- Full brightness means the knob currently owns that parameter.
- Dim brightness means the parameter was moved externally and the knob is waiting to recapture it.

## OLED Pages

### `B1` Main

- Title: `RINGS MODAL`, `RINGS STRING`, or `RINGS SYMP`
- Compact view of all eight parameters
- Temporary zoom view when a knob changes

### `B2` MIDI

- Active model name
- Last received note name
- Last received velocity
- OXI CC mapping reminder
- Sustain pedal and MIDI activity state

### `B3` Help

- A-row model shortcuts
- A4 spread-mode reminder
- B-row page shortcuts
- B8 panic reminder

## Startup / Default Values

| Item | Default |
|---|---|
| Model | `String` |
| Page | `Main` |
| Spread mode | `Wide` |
| `K1 Structure` | `45%` |
| `K2 Brightness` | `68%` |
| `K3 Damping` | `52%` |
| `K4 Position` | `35%` |
| `K5 Spread` | `58%` |
| `K6 Exciter` | `72%` |
| `K7 Macro` | `42%` |
| `K8 Interaction` | `28%` |
| Sustain pedal | Off |
| Last MIDI note | `C4` / `60` |

## Panic / Reset

- `B8` or `SW2` performs panic
- Panic clears held notes, sustain latch, and current resonator tails

## Internal Audio Notes

- Polyphony is fixed at `2 voices` in v1
- Voice stealing prefers inactive voices first, then released tails, then the oldest held voice
- Stereo placement is always active; `A4` changes how wide the pair is spread
- This build is intentionally a compact hybrid, not a direct full Mutable Rings DSP port

## Current Limitations

- No external audio input resonator mode yet
- No alternate knob bank yet
- No preset system yet
- The sympathetic model is Rings-inspired rather than a literal clone
