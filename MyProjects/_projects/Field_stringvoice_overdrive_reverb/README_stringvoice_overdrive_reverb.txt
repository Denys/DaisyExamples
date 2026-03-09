# 8-Step Sequencer

## Author
Adapted from Patch version (5-step), expanded to 8 steps

## Description
Step sequencer with keyboard-based step selection. Uses the bottom keyboard row (8 keys) for step selection/toggling, with visual LED feedback showing sequence state. Features swing timing, variable length, and record mode.

## Controls

| Control | Description | Range |
|---------|-------------|-------|
| Knob 1 | Selected Step Pitch | 0-127 MIDI |
| Knob 2 | Selected Step Gate Length | 0-100% |
| Knob 3 | Tempo | 30-300 BPM |
| Knob 4 | Swing | 0-50% |
| Knob 5 | Pitch Offset | -12 to +12 semitones |
| Knob 6 | Gate Output Multiplier | |
| Knob 7 | Sequence Length | 1-8 steps |
| Knob 8 | Output Level | |
| SW1 | Play/Stop | Toggle |
| SW2 | Record Mode | Toggle step on/off |
| Keyboard 0-7 | Step Select/Toggle | Bottom row |
| CV Out 1 | Pitch | 1V/oct |
| CV Out 2 | Gate | 0V/5V |

## LED Feedback
- **Bottom Row (0-7)**: Sequence steps
  - Dim: Inactive step
  - Medium: Active step  
  - Bright: Selected step
  - Full: Current playing step
- **Top Row**:
  - Key 8: Play status
  - Key 9: Record status
  - Keys 10-15: Pitch level meter

## Modes
- **Edit Mode** (default): Touch keys to select step for editing
- **Record Mode** (SW2): Touch keys to toggle step active/inactive