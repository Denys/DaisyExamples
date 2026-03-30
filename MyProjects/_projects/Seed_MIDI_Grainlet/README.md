# Seed MIDI Grainlet

Minimal Daisy Seed proof of concept for controlling a `GrainletOscillator` from an external Teensy over UART MIDI.

## Scope

- Monophonic Grainlet voice
- MIDI note on/off for pitch and gate
- Three MIDI CC controls for timbre
- Stereo output with the same signal on both channels

## Wiring

- Teensy `TX1` -> Daisy Seed `D14` (`USART1_RX`)
- Teensy `GND` -> Daisy `GND`

This project assumes a direct 3.3V UART MIDI link between the two boards.

## MIDI Map

- `Note On`: set Grainlet pitch and trigger envelope
- `Note Off`: release envelope
- `CC 14`: `shape`
- `CC 15`: `formant frequency`
- `CC 16`: `bleed`

## Defaults

- Shape: `0.35`
- Formant frequency: `1200 Hz`
- Bleed: `0.25`

## Build

From this directory:

```sh
make
```

## Teensy Companion

A matching `Teensy 4.0` controller sketch lives at [Seed_MIDI_Grainlet_Teensy40.ino](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\teensy_controller\Seed_MIDI_Grainlet_Teensy40\Seed_MIDI_Grainlet_Teensy40.ino).

Suggested Teensy wiring:

- `A0` pot -> `CC 14` -> shape
- `A1` pot -> `CC 15` -> formant frequency
- `A2` pot -> `CC 16` -> bleed
- `Pin 2` pushbutton to ground -> fixed `Note On/Off`
- `Serial1 TX` -> Daisy `D14`
- `GND` -> Daisy `GND`

The sketch targets `Teensy 4.0` and sends raw UART MIDI at `31250` baud with no extra Arduino libraries.
