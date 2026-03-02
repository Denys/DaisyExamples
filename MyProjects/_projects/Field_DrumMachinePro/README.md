# Field_DrumMachinePro (Daisy Field)

16-step, 6-voice drum machine for Daisy Field.

## Field Controls

- `Keys A1-A8`: Toggle steps 1-8 for selected drum
- `Keys B1-B8`: Toggle steps 9-16 for selected drum
- `SW1`: Cycle drum voice (`Kick -> Snare -> CHat -> OHat -> Tom -> Clap`)
- `SW2`: Reserved/unused
- `Knob 1-6`: Per-drum synthesis parameters
- `Knob 7`: Tempo
- `Knob 8`: Swing

## Notes

- Patterns initialize with all steps off.
- USB serial logging starts on terminal connect (`StartLog(true)`).
- OLED shows drum, tempo/swing, and 16-step grid.

## Build

```bash
cd DaisyExamples/MyProjects/_projects/Field_DrumMachinePro
make clean
make
```
