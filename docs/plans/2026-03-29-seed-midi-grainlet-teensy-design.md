# Seed MIDI Grainlet Teensy Controller Design

## Goal

Create a minimal Teensy 4.0 companion controller sketch for the Daisy `Seed_MIDI_Grainlet` proof of concept. The Teensy reads three potentiometers and one pushbutton, then sends raw UART MIDI directly to the Daisy Seed.

## Scope

The controller remains intentionally tiny:

- `Serial1` at standard MIDI baud `31250`
- Three analog pots
- One pushbutton
- Raw MIDI message generation without third-party libraries

Out of scope:

- Encoders
- Multiple notes
- Menus or displays
- USB MIDI
- Presets or banks

## Control Mapping

- `A0` -> `CC 14` -> Grainlet shape
- `A1` -> `CC 15` -> Grainlet formant frequency
- `A2` -> `CC 16` -> Grainlet bleed
- `Pin 2` pushbutton -> fixed note `60` note on/off

## Architecture

The sketch polls the three analog inputs, reduces them to MIDI `0..127`, and only transmits a control change when the value meaningfully changes. The pushbutton uses a simple software debounce and sends a fixed note on press and note off on release.

This is enough to prove:

- Teensy UART TX can drive Daisy UART RX
- Continuous MIDI CC control works
- Note gating works end to end

## Success Criteria

- Sketch compiles for `teensy:avr:teensy40`
- Button sends note on/off
- Pot changes send CC 14/15/16
- Daisy `Seed_MIDI_Grainlet` responds audibly
