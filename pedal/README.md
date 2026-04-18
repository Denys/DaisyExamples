# Daisy Pedal

Programmable guitar pedal examples for the `DaisyPedal` board helper.

Phase 1 targets the GuitarPedal125b control surface:

- Stereo audio I/O
- 6 knobs
- 2 footswitches
- Rotary encoder
- 2 LEDs
- OLED display
- MIDI in/out
- Relay bypass and mute control

## Included Examples

- `PassthruBypass` - relay bypass, mute timing, OLED status, and tap-tempo heartbeat
- `NoiseGate` - simple stereo gate built on the `DaisySP::NoiseGate` module
- `PitchDrop` - mono guitar pitch drop using `PitchShifter` and `BypassFader`
- `PolyOctave` - mono octave effect using `Multirate` and `OctaveGenerator`
