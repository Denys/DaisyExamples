# Field Wavetable Morph Synth

A wavetable synthesizer with morphing capabilities for the Daisy Field platform, based on section 16 of the field_concepts_OPUS_2.md specifications.

## Overview

This project implements a monophonic wavetable synthesizer featuring:
- Custom wavetables with position-based morphing
- 8 wavetable banks selectable via A1-A8 keys
- 8 morph curves selectable via B1-B8 keys
- State Variable Filter (SVF) with ADSR envelope
- Stereo effects processing
- MIDI input support (DIN-5 and USB)
- Real-time audio processing optimized for low latency

## Hardware Requirements

- Daisy Field
- MIDI keyboard (external)
- USB cable for programming and MIDI
- Audio output connections

## Controls Mapping

### Knobs (8 available)
1. **Knob 1**: Wavetable Position (0-1, modulates table index with optional LFO)
2. **Knob 2**: Morph Speed (controls rate of position changes, 0-10 Hz)
3. **Knob 3**: Filter Cutoff (SVF cutoff frequency, 20Hz - 20kHz)
4. **Knob 4**: ADSR Attack (0-5 seconds)
5. **Knob 5**: ADSR Decay (0-5 seconds)
6. **Knob 6**: ADSR Sustain (0-1)
7. **Knob 7**: ADSR Release (0-10 seconds)
8. **Knob 8**: FX Amount (dry/wet mix for stereo FX, 0-1)

### Keys (16 available: A1-A8, B1-B8)
- **A1-A8**: Wavetable Bank Select
  - A1: Sine waves bank
  - A2: Sawtooth waves bank
  - A3: Square waves bank
  - A4: Triangle waves bank
  - A5: Custom wavetable 1 bank
  - A6: Custom wavetable 2 bank
  - A7: Custom wavetable 3 bank
  - A8: Custom wavetable 4 bank
- **B1-B8**: Morph Curve Select
  - B1: Linear morphing
  - B2: Exponential morphing
  - B3: Logarithmic morphing
  - B4: Sine morphing
  - B5: Triangle morphing
  - B6: Step morphing
  - B7: Random morphing
  - B8: Custom curve morphing

### Switches
- **SW1**: LFO On/Off for position modulation
- **SW2**: FX Bypass

## Building and Running

1. Ensure Daisy toolchain is installed
2. Navigate to the project directory
3. Run `make`
4. Flash to Daisy Field using `make program-dfu`

## Implementation Status

- [x] Project structure created
- [x] Basic hardware integration (main.cpp, Makefile)
- [ ] Wavetable engine implementation
- [ ] Morphing processor
- [ ] Voice architecture
- [ ] MIDI handling
- [ ] Control mapping
- [ ] OLED display integration
- [ ] Effects processing
- [ ] Low latency optimization

## Architecture

The synthesizer uses a modular architecture:
- **WavetableOsc**: Handles wavetable playback with morphing
- **MorphProcessor**: Applies morphing curves and LFO modulation
- **Voice**: Combines oscillator, filter, envelope, and FX
- **MidiHandler**: Processes MIDI messages
- **UiHandler**: Manages controls and display

## Performance Targets

- CPU usage: <50% at 48kHz
- Latency: <5ms
- Memory: <64KB for wavetables

## Dependencies

- libDaisy
- DaisySP

## Documentation

See `wavetable_synth_morph_plan.md` for detailed implementation plan and specifications.

## License

This project follows the same license as the Daisy Examples repository.