# Field Template April

`Field_Template_April` is a clean Daisy Field starter synth for new display-based projects. It is intentionally a small mono voice, but it demonstrates the current Field control rules: banked knobs with pickup/catch, knob LEDs driven by stored values, keybed-as-controls, tri-state key LEDs, OLED overview plus zoom, and explicit startup/default documentation.

This template plays notes from external MIDI only. The Field keybed is reserved for parameter/state controls.

## Controls

| Control | Role | Notes |
|---|---|---|
| `K1` | `Cutoff` | Main bank |
| `K2` | `Reso` | Main bank |
| `K3` | `Attack` | Main bank |
| `K4` | `Decay` | Main bank |
| `K5` | `Sustain` | Main bank |
| `K6` | `Release` | Main bank |
| `K7` | `Drive` | Main bank |
| `K8` | `Color` | Main bank; not wasted on generic level |
| `SW1 + K1` | `EnvAmt` | Alt bank |
| `SW1 + K2` | `LfoRt` | Alt bank |
| `SW1 + K3` | `LfoDp` | Alt bank |
| `SW1 + K4` | `Glide` | Alt bank |
| `SW1 + K5` | `VelAmt` | Alt bank |
| `SW1 + K6` | `KeyTrk` | Alt bank |
| `SW1 + K7` | `Noise` | Alt bank |
| `SW1 + K8` | `Sub` | Alt bank |
| `SW2 + K8` | `Level` | Hidden internal output level trim |
| `A1-A4` | Waveform select | `SINE`, `TRI`, `SAW`, `SQR` |
| `A5` | Velocity mode | Cycles `FIX` -> `SCL` -> `PCH` |
| `A6` | Key tracking mode | Cycles `OFF` -> `HALF` -> `FULL` |
| `A7` | LFO target | Cycles `OFF` -> `PITCH` -> `FILT` |
| `A8` | Glide mode | Cycles `OFF` -> `LEG` -> `ON` |
| `B1-B4` | MIDI transpose select | `-12`, `0`, `+12`, `+24` semitones |
| `B5` | Panic | All notes off |
| `B6` | Reset main bank | Restores main defaults |
| `B7` | Reset alt bank | Restores alt defaults |
| `B8` | Reset all | Restores all defaults and state controls |
| MIDI input | Note control | `Note On`, `Note Off`, velocity, `CC64` sustain |

## LED States

### Knob LEDs

- Knob LEDs show the stored logical value of the currently active parameter bank.
- They do not show the raw physical potentiometer position.
- Bright LED = captured and live.
- Dim LED = stored value exists, but the knob has not yet re-captured after a bank/modifier switch.
- While holding `SW2`, `K8` shows hidden `Level` instead of main/alt `K8`.

### Key LEDs

The keybed supports three states:

- `Off`
- `Blink`
- `On`

In this template:

- `A1-A4`: one-hot waveform selection, selected key is `On`
- `A5-A8`: tri-state demo controls
  - `Off` = state 1
  - `Blink` = state 2
  - `On` = state 3
- `B1-B4`: one-hot transpose selection, selected key is `On`
- `B5`: note state indicator
  - `Off` = idle
  - `On` = note gate open
  - `Blink` = sustain held
- `B6-B8`: utility keys
  - `B6` and `B7` blink to mark bank-reset helpers
  - `B8` stays `On` to mark full reset/default recall

### Switch LEDs

- `SW1` LED brightens when the alt bank is active
- `SW2` LED brightens while the hidden `Level` modifier is held

## OLED Pages

The OLED uses two views: compact overview and edit zoom.

### Overview

Shown when no parameter has changed recently.

```text
APRIL MAIN|ALT
WAVE  TR:TRANSPOSE
A5:VELMODE  A6:KEYTRK
A7:LFOTGT   A8:GLIDE
P1:VALUE    P5:VALUE
P2:VALUE    P6:VALUE
P3:VALUE    P7:VALUE
P4:VALUE    P8:VALUE
```

Notes:

- `MAIN` or `ALT` reflects the active knob bank
- the lower grid always shows the active bank’s stored parameter values

### Edit Zoom

Shown for about `1.4 s` after a parameter or state change.

```text
EDIT MAIN|ALT
PARAMETER NAME
LARGE VALUE TEXT
SW1=Alt  SW2+K8=Level
```

Examples:

- `Cutoff` + `5320 Hz`
- `VelMode` + `PCH`
- `Level` + `80%`

## Hidden Banks / Pages

### Main vs Alt bank

- `K1-K8` = main bank
- `SW1 + K1-K8` = alt bank

Switching between them:

- preserves stored values
- uses pickup/catch
- does not let the physical knob overwrite the newly active parameter immediately
- updates knob LEDs to the stored value set for the active layer

### Hidden level trim

- `Level` is not on the main bank
- hold `SW2` and move `K8` to edit internal output level
- `Level` has its own stored value and its own pickup/catch state

## Startup / Default Values

### Main bank defaults

| Param | Default |
|---|---|
| `Cutoff` | `58%` |
| `Reso` | `12%` |
| `Attack` | `2%` |
| `Decay` | `22%` |
| `Sustain` | `78%` |
| `Release` | `24%` |
| `Drive` | `12%` |
| `Color` | `50%` |

### Alt bank defaults

| Param | Default |
|---|---|
| `EnvAmt` | `42%` |
| `LfoRt` | `18%` |
| `LfoDp` | `14%` |
| `Glide` | `10%` |
| `VelAmt` | `70%` |
| `KeyTrk` | `50%` |
| `Noise` | `0%` |
| `Sub` | `18%` |

### Hidden / state defaults

| Setting | Default |
|---|---|
| `Level` | `80%` |
| Waveform | `SAW` |
| Velocity mode | `SCL` |
| Key tracking | `HALF` |
| LFO target | `FILT` |
| Glide mode | `LEG` |
| MIDI transpose | `0` |
| Active bank at boot | `MAIN` |
| OLED page at boot | Overview |
| Note state at boot | idle, sustain off |

## Panic / Reset

- `B5` sends panic: closes gate, clears sustain, and drops the current note state
- `B6` restores the main knob bank defaults
- `B7` restores the alt knob bank defaults
- `B8` restores all defaults, including hidden `Level` and key-state controls
- Recalling defaults also clears pickup state for the affected controls, so knobs must re-capture

## MIDI / External Control

- external MIDI is the only note-entry method
- supported messages:
  - `Note On`
  - `Note Off`
  - velocity
  - `CC64` sustain pedal
- no parameter CC mapping is included in this template

## Maintenance Notes

- Keep the synth simple; this project is a starter shell, not a flagship instrument
- Preserve banked pickup/catch behavior when adding pages or modifiers
- Keep knob LEDs tied to stored values, never raw pot position
- Avoid spending main `K8` on generic output level unless the architecture truly requires it
- If you change OLED pages, key LED semantics, or startup defaults, update this README in the same change
