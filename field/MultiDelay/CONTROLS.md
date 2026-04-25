# Field MultiDelay Controls

## Hardware Mapping

| Field control | Firmware behavior |
|---|---|
| K1 | MultiDelay dry/wet mix |
| K2 | Primary delay control |
| K3 | Secondary delay control |
| K4 | Feedback |
| K5 | Tertiary delay control |
| K6-K8 | Unused in this first adapter |
| CV1 | Tertiary delay CV input |
| CV2-CV4 | Unused in this first adapter |
| CV OUT 1 | Mirrors K5 as `0..5V` |
| CV OUT 2 | Held at `0V` |
| SW1 | Fires the MultiDelay impulse utility |
| SW2 | Toggles OLED between app display and Field hardware status |
| A/B key LEDs | Visual status only; no musical key behavior in this adapter |
| Knob LEDs | Mirror K1-K8 physical knob values |

## Manual Hardware Validation Checklist

Record date, build commit or diff state, and tester name before marking this
adapter hardware-validated.

- ST-Link detects the connected Daisy Field.
- `make program` flashes successfully.
- Audio input 1 reaches audio outputs 1/2 with a delay effect.
- K1-K5 audibly affect the documented parameters.
- K6-K8 do not unexpectedly affect DSP.
- CV1 affects tertiary delay.
- CV OUT 1 changes over `0..5V` with K5.
- CV OUT 2 remains `0V`.
- SW1 fires the impulse utility.
- SW2 toggles the OLED status page without changing DSP state.
- Knob LEDs, key LEDs, switch LEDs, and OLED update without audio dropouts.

## Notes

- Field controls are processed in the main loop, not in the audio callback.
- The audio callback only runs `MultiDelayCore::Process(...)` and writes audio
  buffers.
- This adapter intentionally keeps K6-K8, CV2-CV4, and musical key behavior out
  of scope for v1.
