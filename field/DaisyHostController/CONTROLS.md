# DaisyHostController Controls

## USB MIDI Mapping

| Field control | MIDI output |
|---|---|
| K1-K8 | CC 20-27 |
| CV1-CV4 | CC 28-31 |
| A1-B8 | Note 60-75 |
| SW1-SW2 | CC 80-81 |

All messages use MIDI channel 1. Knobs and CV inputs send only when their
7-bit MIDI value changes after startup, so plugging the controller in does not
immediately overwrite learned DaisyHost parameters.

CV inputs are mapped as bipolar controller sources: `-1..+1` becomes
`0..127`, with center around `64`.

## DaisyHost Setup

1. Connect the Daisy Field USB port to the computer.
2. Open DaisyHost standalone.
3. Enable the Daisy USB MIDI input in the JUCE `Settings...` dialog.
4. Use DaisyHost MIDI learn on the target control.
5. Move a Field knob, switch, CV input, or key to bind it.

For VST3 use, route the Daisy Field MIDI input to `DaisyHost Patch.vst3` in the
DAW.

## Manual Hardware Validation Checklist

Record date, build commit or diff state, and tester name before marking this
target hardware-validated.

- `make` succeeds.
- `make program` flashes successfully through ST-Link.
- Local Field OLED feedback shows K1-K8 knob moves, SW1/SW2 button presses,
  and A1-B8 key events.
- The computer sees the Daisy Field as a USB MIDI device.
- K1-K8 emit CC 20-27 with useful `0..127` ranges.
- CV1-CV4 emit CC 28-31 and remain stable when idle.
- A1-B8 emit Note On and Note Off for notes 60-75.
- SW1/SW2 emit momentary CC 80/81 values `127` then `0`.
- DaisyHost standalone can MIDI-learn at least one knob CC.
- DaisyHost MIDI tracker shows Field note and CC activity.
- LEDs and OLED update without stuck notes or repeated CC flooding.

## Manual Validation Notes

- 2026-04-26: user tested all knobs, buttons, and keys on the flashed Field.
  The events displayed correctly on the Field screen. Field-to-computer USB
  MIDI was not checked in this pass.
