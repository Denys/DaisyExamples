# Field_MI_Plaits

## Overview

`Field_MI_Plaits` is a Daisy Field synth project with banked controls and pickup/catch behavior.
It is not a full original Plaits port. The current build is a reduced mono voice driven by external MIDI, with
stored parameter banks, OLED status pages, and one-hot key shortcuts.

The most important behavior change is that `SW1` now switches between independent main and alt parameter banks.
Switching banks does not copy or overwrite values. Knobs must be picked up before they start editing the active bank.
Knob LEDs show the stored logical parameter value, not the raw pot position.

`K8` is kept as a musically meaningful voice-level control for the synth behavior itself.
It is not treated as a generic hardware master volume.

## Control Map

### Main bank

| Control | Function |
|---|---|
| `K1` | Frequency / pitch center |
| `K2` | Harmonics |
| `K3` | Timbre |
| `K4` | Morph |
| `K5` | FM amount |
| `K6` | Timbre modulation amount |
| `K7` | Morph modulation amount |
| `K8` | Level / LPG-style voice level behavior |

### Alt bank

Hold `SW1` to edit the alt bank. The stored alt values remain separate from the main bank.

| Control | Function |
|---|---|
| `SW1 + K1` | LPG colour |
| `SW1 + K2` | Decay |
| `SW1 + K3` | Reserved (inactive) |
| `SW1 + K4` | Reserved (inactive) |
| `SW1 + K5` | Reserved (inactive) |
| `SW1 + K6` | Reserved (inactive) |
| `SW1 + K7` | Reserved (inactive) |
| `SW1 + K8` | Reserved (inactive) |

`SW1 + K3-K8` are intentionally disabled on this build. They do not capture, edit, or light in the alt bank.

### Keybed and switches

| Control | Function |
|---|---|
| `A1-A8` | Select engine slot when `SW2` is not held |
| `B1-B8` | Select engine slot when `SW2` is not held |
| `SW1` tap | Panic: clears the note/gate state and sustain |
| `SW1` hold | Switches to the alt parameter bank |
| `SW2` tap | Clears the temporary OLED zoom state |
| `SW2` hold | Opens the range page and turns the keybed into range controls |

Only mapped engine slots are active. Disabled slots intentionally do nothing.

### MIDI

| Input | Function |
|---|---|
| `NoteOn` | Starts the voice |
| `NoteOff` | Releases the held note |
| `CC64` | Sustain pedal |

External MIDI provides the notes, so the keybed is free for control and menu behavior.

## Pickup And Bank Switching

The main and alt banks are stored independently.

Behavior:

1. `K1-K8` always edit the currently active bank only.
2. Holding `SW1` changes the active bank to alt.
3. Releasing `SW1` returns to main.
4. Switching banks does not copy values between banks.
5. A knob must be picked up before it starts writing again.

LEDs make this easier to understand:

- uncaptured knobs show a dim LED at the stored value
- captured knobs show a bright LED at the stored value

## OLED Pages

### Overview

Shows the current engine slot, note state, range, parameter summary, MIDI/gate activity, and active bank.

### Zoom

Appears briefly after a knob move and shows the active parameter name, its stored value, and a bar display.

### Alt bank

Shown while `SW1` is held.
Displays the alt-bank values, with `K3-K8` marked as reserved and inactive on this build.

### Range

Shown while `SW2` is held.
Lets the keybed select the pitch range:

- `A1-A8` = `C0 +/- 7` through `C7 +/- 7`
- `B1` = full `C0-C8` style wide range

## Startup Defaults

| Item | Default |
|---|---|
| Main `K1` | 50% |
| Main `K2` | 40% |
| Main `K3` | 50% |
| Main `K4` | 50% |
| Main `K5` | 50% |
| Main `K6` | 50% |
| Main `K7` | 50% |
| Main `K8` | 85% |
| Alt `K1` | 50% |
| Alt `K2` | 50% |
| Alt `K3-K8` | 50% reserved placeholders |
| Engine slot | First active slot |
| Range | Full `C0-C8` style range |
| Zoom | Off |
| Sustain | Off |
| Active bank | Main |

## Panic And Reset

- `SW1` tap clears the current note, gate, and sustain state.
- `SW2` tap clears the OLED zoom state.
- If a note is stuck, `SW1` is the immediate recovery action.

## Notes For Maintenance

- Keep the README, `CONTROLS.md`, and `Dependencies.md` in sync whenever the control map changes.
- If a knob is repurposed, update the startup/default table as well.
- If a future build adds more meaningful alt-bank assignments, replace the reserved slots with real functions instead of reusing the old labels.
