# Field_AnalogDrumCore

Analog kick/snare drum machine for Daisy Field using:

- `AnalogBassDrum`
- `AnalogSnareDrum`
- 16-step sequencer
- `Overdrive` on master bus

## Build

```bash
cd DaisyExamples/MyProjects/_projects/Field_AnalogDrumCore
make clean && make
```

## Notes

- Follows DAISY_QAE callback rule: control processing stays in the main loop.
- Uses `field_defaults.h` for keyboard mappings, LED helpers, and OLED parameter display.
