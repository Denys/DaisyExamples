# Field_ModalBells

A physics-based percussion synthesizer for Daisy Field using modal synthesis.

## Overview

Field_ModalBells creates bell, chime, marimba, and metallic percussion sounds using DaisySP's ModalVoice synthesis. The keyboard allows building complex harmonic patterns by toggling individual modes on/off, with LED feedback showing the active pattern.

## Features

- **8 Modal Voices:** Each representing a harmonic frequency
- **8 Strike Types:** Different mallet/striker characteristics  
- **LED Pattern Display:** Active modes shown on Row A LEDs
- **Real-time OLED Feedback:** Shows active parameter with auto-highlighting
- **Stereo Reverb:** Built-in reverb with size/decay control
- **Two Display Modes:** Standard (large param) and Compact (all 8)

## Controls

### Keyboard

**Row A (A1-A8) - Modal Voices:**
- Each key toggles a harmonic mode on/off
- LED shows which modes are currently active
- Pressing a key triggers that mode
- Build complex patterns by combining modes

**Row B (B1-B8) - Strike Types:**
- B1: Soft mallet
- B2: Medium mallet
- B3: Hard mallet
- B4: Metal striker
- B5: Brush
- B6: Mallet
- B7: Felt
- B8: Wood
- Only one strike type active at a time

### Knobs

| Knob | Parameter | Description |
|------|-----------|-------------|
| K1 | Brightness | Harmonic content of strike |
| K2 | Structure | Modal frequency spacing |
| K3 | Damping | How quickly modes decay |
| K4 | Accent | Strike hardness/velocity |
| K5 | Reverb Size | Simulated room size |
| K6 | Reverb Decay | Reverb tail length |
| K7 | Dry/Wet | Reverb mix amount |
| K8 | Master Level | Output volume |

### Switches

- **SW1:** Toggle between Standard and Compact display views
- **SW2:** Trigger all active modes simultaneously

## Display

### Standard View
```
ModalBells 4M Hard
> Brightness
0.72

---
0.72  0.45
```
Shows the last-adjusted parameter large with first two params below.

### Compact View
```
ModalBells 4M Hard
Bri:0.72  Siz:0.8
Str:0.45  Dcy:0.6
Dmp:0.30  Mix:0.5
Acc:0.85  Lvl:1.0
```
Shows all 8 parameters in two columns.

## Usage Tips

1. **Start Simple:** Activate just 2-3 modes (keys A1, A3, A5)
2. **Experiment with Damping:** Low values = sustained ringing, high values = short percussive
3. **Layer Strikes:** Try different strike types on the same mode pattern
4. **Build Patterns:** Toggle modes on/off to create evolving textures
5. **Use Reverb:** Adds depth and space to the bell sounds

## Modal Frequencies

The 8 modes are tuned to a bell-like harmonic series based on C4:
- Mode 1 (A1): C4 fundamental (261.63 Hz)
- Mode 2 (A2): G4 perfect 5th (392.00 Hz)
- Mode 3 (A3): C5 octave (523.25 Hz)
- Mode 4 (A4): E5 major 3rd (659.25 Hz)
- Mode 5 (A5): G5 perfect 5th (783.99 Hz)
- Mode 6 (A6): A5 major 6th (880.00 Hz)
- Mode 7 (A7): C6 two octaves (1046.5 Hz)
- Mode 8 (A8): E6 major 3rd (1318.5 Hz)

## Building

```bash
make clean
make
make program-dfu  # Flash to Daisy Field
```

## Dependencies

- libDaisy
- DaisySP
- field_defaults.h (in `foundation_examples/`)

## Implementation Notes

This project showcases the new `field_defaults.h` library:
- `FieldKeyboardLEDs` for toggle LED management
- `FieldOLEDDisplay` for auto-parameter tracking
- Standard LED/keyboard constant arrays

See `FIELD_DEFAULTS_USAGE.md` in `foundation_examples/` for details.

## Credits

Modal synthesis algorithm: DaisySP `ModalVoice`
Reverb: DaisySP `ReverbSc`
