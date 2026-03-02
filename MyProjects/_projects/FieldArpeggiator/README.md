# Field Arpeggiator

A full-featured MIDI arpeggiator for **Daisy Field** with OLED display and comprehensive LED feedback.

## Features

### Phase 1 - Core Engine ✅
- **MIDI Note Buffer**: Captures up to 8 held notes
- **4 Arp Modes**: Up, Down, Up/Down, Random
- **Multi-Octave**: 1-3 octave range
- **Gate Length**: 10-100% adjustable
- **Swing**: 0-50% shuffle
- **Latch Mode**: Hold notes without keeping keys pressed
- **OLED Display**: Shows mode, BPM, note buffer
- **LED Feedback**: Mode selection, note activity, status

### Phase 2 - Advanced Features ✅
- **External MIDI Clock Sync**: 24 PPQN with auto BPM detection
- **8-Step Pattern Visualization**: Real-time step display on OLED
- **Clock Source Toggle**: Switch between Internal/External (Key A5)
- **MIDI Transport**: Responds to Start/Stop/Continue messages

## Controls

### Knobs
| Knob | Function | Range |
|------|----------|-------|
| 1 | BPM | 40-240 |
| 2 | Octave Range | 1-3 |
| 3 | Gate Length | 10-100% |
| 4 | Swing | 0-50% |
| 5 | Attack | 1-500ms |
| 6 | Decay | 10-500ms |
| 7 | Sustain | 0-100% |
| 8 | Release | 10-1000ms |

### Keys
| Key | Function |
|-----|----------|
| A1 | Mode: Up |
| A2 | Mode: Down |
| A3 | Mode: Up/Down |
| A4 | Mode: Random |
| B1 | Octave: -1 |
| B2 | Octave: 0 |
| B3 | Octave: +1 |
| B4 | Octave: +2 |

### Switches
| Switch | Function |
|--------|----------|
| SW1 | Latch Mode Toggle |
| SW2 | Pause/Play |

## Building

```bash
make clean && make
```

## Flashing

```bash
# Using DFU
make program-dfu

# Using ST-Link
make program
```

## MIDI Setup

Connect a USB MIDI keyboard to the Daisy Field. Notes played will be captured and arpeggiated according to the selected mode and parameters.

## Signal Flow

```
MIDI In → Note Buffer → Arpeggiator → Oscillator → MoogLadder → ADSR → Output
                ↓
           Metro Clock
```

## References

Based on research from 11 authoritative sources including:
- *Sound and Music Projects for Eurorack and Beyond*
- *Arduino for Musicians*
- *Designing Software Synthesizer Plugins in C++ 2ed*
- *DAFX: Digital Audio Effects 2ed*

See `Arpeggiator_Project_Plan.md` for full analysis.

## Status

- [x] Phase 1: Core Engine (Complete)
- [ ] Phase 2: Advanced Features (Latch, External sync, Swing improvements)
- [ ] Phase 3: Generative Extensions (Probability, Transpose patterns)
