# Daisy Arpeggiator Project Plan

**Status**: ✅ Phase 1 & 2 Complete (compiles successfully)
**Migrated to**: PLANNING/projects/ (2026-01-23)
**Relationship to DVPE**: Standalone Daisy Project (NOT an integrated DVPE feature)
**Location**: `DaisyExamples/MyProjects/_projects/FieldArpeggiator/`

---

## Goal Description
Build a full-featured MIDI arpeggiator for **Daisy Field** informed by 18 documented arpeggio techniques spanning 11 authoritative sources. The outcome is:
1. A complete C++ implementation with MIDI I/O, OLED display, and Field controls.
2. A matching `.dvpe` block diagram for the DVPE visual editor.

## Build Status

| Phase | Status | Notes |
|-------|--------|-------|
| Phase 1 | ✅ Complete | MIDI buffer, 4 modes, Metro clock, gate length, swing, OLED, LEDs |
| Phase 2 | ✅ Complete | External MIDI clock sync (24 PPQN), 8-step pattern visualization, A5 clock toggle |
| Phase 3 | ⏳ Planned | Generative extensions |

**Final Build**:
- FLASH: 116KB (89%)
- SRAM: 104KB (20%)

---

## Analysis of Arpeggio Methods (Books Reference)

### Category A — MIDI Note-Buffer Sequencing
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Sound & Music Projects for Eurorack* Ch.10 | `ArpEngine` class | Note buffer, bubble sort, up/down/random, MIDI clock sync |
| *Introduction to Digital Music with Python* Ch.8 | `arpeggio()` function | List iteration, pitch offsets, beats |
| *Arduino for Musicians* Ch.15 | `StepSequence` class | Fixed-length patterns, timer ISR, shift register |
| *Arduino for Musicians* Ch.7 | `trackLoop()` | Major chord arrays, potentiometer tempo |

### Category B — LFO/Step Quantization
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Designing Software Synth Plugins* Ch.8 | Stepped LFO | `quantizeBipolarValue`, lane probability |
| *DAFX 2ed* Ch.10.4.3 | Harmonizer | Parallel pitch shifters, interval mapping |
| *Designing Audio Effect Plugins* Ch.13-14 | LFO + Delay | Modulo counter, ring buffer, BPM sync |

### Category C — Markov/Generative
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Build AI-Enhanced Audio Plugins* Ch.20-25 | Markov chains | State transitions, IOI, `ChordDetector` |
| *Sound & Music Projects for Eurorack* Ch.11 | Genetic algorithms | Chromosome mutation, note lookup |

---

## Implementation Phases

### Phase 1 — Core Engine ✅ COMPLETE
- [x] MIDI note-on capture into ring buffer (8 notes max)
- [x] Arp modes: Up, Down, UpDown, Random
- [x] Metro-based clock with BPM knob (40–240 BPM)
- [x] Gate length parameter
- [x] Swing parameter
- [x] OLED: current note, mode, BPM
- [x] LED visualization

### Phase 2 — Advanced Features ✅ COMPLETE
- [x] Octave range (+0, +1, +2 octaves)
- [x] Latch mode
- [x] External MIDI clock sync (24 PPQN)
- [x] 8-step pattern visualization
- [x] A5 key for clock source toggle

### Phase 3 — Generative Extensions ⏳ PLANNED
- [ ] Probability per step (0–100%)
- [ ] Transpose knob (±12 semitones)
- [ ] Pattern length (1–16 steps)
- [ ] Optional: Genetic/Markov variation mode

---

## DVPE Integration

**Note**: A DVPE `arpeggiator.ts` block definition exists, but this project is a standalone C++ implementation that demonstrates the concept. The two are related but separate.

### Arpeggiator Block Inputs (DVPE):
- `midi_in` (MIDI note events)
- `clock` (Metro trigger)
- `mode_cv` (0–3: Up/Down/UpDown/Random)
- `octave_cv` (0–2)
- `gate_length_cv` (0–1)

### Arpeggiator Block Outputs (DVPE):
- `note_out` (float: MIDI note 0–127)
- `gate_out` (trigger)

---

## Files

| File | Description |
|------|-------------|
| `main.cpp` | Main application (690+ lines) |
| `Makefile` | Build configuration |
| `README.md` | User documentation |
| `Arpeggiator_Project_Plan.md` | This file (original location) |

---

## References (Sources Key)
1. *Sound and Music Projects for Eurorack and Beyond*
2. *Introduction to Digital Music with Python Programming*
3. *Arduino for Musicians* (2016)
4. *Designing Software Synthesizer Plugins in C++ 2ed* (2021)
5. *Build AI-Enhanced Audio Plugins with C++*
6. *The Python Audio Cookbook*
7. *DAFX: Digital Audio Effects 2ed*
8. *Computer Sound Design*
9. *Hack Audio: DSP in MATLAB*
10. *Designing Audio Effect Plugins in C++ 2ed*
11. *Audio Effects: Theory, Implementation and Application*
