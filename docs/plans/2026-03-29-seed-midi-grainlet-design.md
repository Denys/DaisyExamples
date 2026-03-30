# Seed MIDI Grainlet Design

## Goal

Create a new Daisy Seed project named `Seed_MIDI_Grainlet` that turns the stock Grainlet example into a minimal externally controlled proof of concept. The Daisy runs the sound engine. A Teensy sends MIDI over UART for pitch and basic timbre control.

## Scope

This first pass stays intentionally small:

- One `GrainletOscillator`
- Monophonic voice
- MIDI note on/off over UART
- Three MIDI CC controls
- Simple amplitude envelope to reduce clicks on note changes
- Stereo output with the same signal on left and right

Out of scope for the POC:

- Polyphony
- Sub-oscillator
- Onboard knobs, buttons, or OLED UI
- Presets
- Clock sync
- Sample-based granular processing

## Architecture

The project reuses the DSP core idea from the stock Seed `grainlet` example and the UART receive flow from the `MIDI_UART_Input` example. MIDI parsing and control updates run in the main loop. Audio rendering stays in the audio callback and only reads lightweight shared state.

The Teensy is expected to send:

- `Note On` and `Note Off` for pitch/gate
- `CC 14` for Grainlet shape
- `CC 15` for formant frequency
- `CC 16` for bleed

## Control Mapping

- `Note On`: set oscillator frequency from MIDI note and open envelope
- `Note Off`: release envelope
- `CC 14`: map `0..127` to `shape 0.0..1.0`
- `CC 15`: map `0..127` to `formant frequency 100..4000 Hz`
- `CC 16`: map `0..127` to `bleed 0.0..1.0`

## Wiring

- Teensy `TX1` -> Daisy Seed `D14` (`USART1_RX`)
- Teensy `GND` -> Daisy `GND`
- Shared 3.3V logic assumptions apply for direct UART connection

## Success Criteria

- Project builds cleanly with `make`
- Daisy receives UART MIDI from Teensy
- Notes trigger audible pitch changes
- The three CCs audibly change the Grainlet tone
- The project structure is isolated under `MyProjects/_projects/Seed_MIDI_Grainlet`
