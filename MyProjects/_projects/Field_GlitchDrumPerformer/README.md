# Field_GlitchDrumPerformer (Daisy Pod)

Glitch/percussion performance patch adapted from Daisy Field to Daisy Pod.

## Engine

- Core drums: `AnalogBassDrum` + `AnalogSnareDrum`
- Texture: `Particle` + `GrainletOscillator`
- Random bursts: `Dust` gated by fill mode
- Saturation: `Overdrive` blend

## Pod Controls

- `Encoder turn`: Select pattern step (1-8)
- `Encoder press`: Cycle parameter page (1-4)
- `Button 1`: Toggle kick on selected step
- `Button 2`: Toggle snare on selected step
- `Button 1 + Button 2 hold`: Fill mode (Dust-triggered glitch bursts)
- `Knob 1/2`: Edit parameters in the active page

## Parameter Pages

- `Page 1`: Kick Tone / Kick Decay
- `Page 2`: Snare Tone / Snare Snappy
- `Page 3`: Texture / Grain Color
- `Page 4`: Tempo / Drive

## LED Feedback

- `LED1`: Clock pulse in active page color
- `LED2`: Kick/Snare step state at selected step, or bright fill color while fill is held

## Serial Logging

USB serial logging starts on terminal connection (`StartLog(true)`):

- Boot info
- Step/page changes
- Kick/snare step toggles
- Knob/page edits
- 1 Hz status line

## Build

```bash
cd DaisyExamples/MyProjects/_projects/Field_GlitchDrumPerformer
make clean
make
```
