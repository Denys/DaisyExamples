# Daisy Arpeggiator Project Plan

## Goal Description
Build a full-featured MIDI arpeggiator for **Daisy Field** informed by 18 documented arpeggio techniques spanning 11 authoritative sources. The outcome is:
1. A complete C++ implementation with MIDI I/O, OLED display, and Field controls.
2. A matching `.dvpe` block diagram for the DVPE visual editor.

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

### Category D — Symbolic/Text-based
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Python Audio Cookbook* Ch.10-11 | `Events`, `Score`, MML | Pattern iteration, sample-accurate timing |
| *Python Audio Cookbook* Ch.9 | 12-tone matrix | Schoenberg row, voice distribution |

### Category E — Delay/Pitch-Shift
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Hack Audio* Ch.15 | Pitch shifter | Fractional delay, linear interp |
| *DAFX 2ed* Ch.9.4.3 | Auto-tune | Yin f₀ analysis, semitone quantization |
| *Audio Effects: Theory & Impl.* Ch.2,12 | Delay line | Cubic interp, MIDI events |

### Category F — Synth-Embedded
| Source | Technique | Key Concepts |
|--------|-----------|--------------|
| *Computer Sound Design* Ch.8.17 | Vibra 1000 arp module | Active, Sync, Tempo, Range params |

---

## Proposed Implementation (3 Phases)

### Phase 1 — Core Engine (Week 1)
- [ ] MIDI note-on capture into ring buffer (8 notes max)
- [ ] Arp modes: Up, Down, UpDown, Random
- [ ] Metro-based clock with BPM knob (40–240 BPM)
- [ ] Gate length parameter
- [ ] OLED: current note, mode, BPM

### Phase 2 — Advanced Features (Week 2)
- [ ] Octave range (+0, +1, +2 octaves) with Knob 2
- [ ] Latch mode (Switch 1)
- [ ] External MIDI clock sync (24 PPQN)
- [ ] Swing/shuffle (±50%)
- [ ] OLED: pattern visualization (8-step bar)

### Phase 3 — Generative Extensions (Week 3)
- [ ] Probability per step (0–100%)
- [ ] Transpose knob (±12 semitones)
- [ ] Pattern length (1–16 steps)
- [ ] Optional: Genetic/Markov variation mode

---

## Proposed Block Diagram (DVPE)

```
┌──────────────┐     ┌──────────────┐
│  MIDI_Input  │────►│ Arpeggiator  │
└──────────────┘     │  (new block) │
                     └──────┬───────┘
       ┌───────────────────►│
       │                    ▼
┌──────┴────┐         ┌──────────┐
│   Metro   │────────►│   ADSR   │
│ (BPM knob)│         └────┬─────┘
└───────────┘              │
                           ▼
                     ┌──────────┐
                     │ Oscillator│
                     └────┬─────┘
                          │
                          ▼
                     ┌──────────┐
                     │   VCA    │
                     └────┬─────┘
                          │
                          ▼
                     ┌──────────┐
                     │  Output  │
                     └──────────┘
```

**Arpeggiator Block Inputs**:
- `midi_in` (MIDI note events)
- `clock` (Metro trigger)
- `mode_cv` (0–3: Up/Down/UpDown/Random)
- `octave_cv` (0–2)
- `gate_length_cv` (0–1)

**Arpeggiator Block Outputs**:
- `note_out` (float: MIDI note 0–127)
- `gate_out` (trigger)

---

## Verification Plan

### Automated Tests
```bash
npm run test src/core/blocks/definitions/arpeggiator.test.ts
```

### Manual Verification
1. Load `field_arpeggiator.dvpe` in DVPE GUI — confirm all wires render.
2. Export C++, compile with `make clean && make`.
3. Flash to Daisy Field, test with external MIDI keyboard.
4. Verify OLED displays note, mode, BPM.

---

## Files to Create/Modify

| Action | File |
|--------|------|
| **NEW** | `dvpe_CLD/src/core/blocks/definitions/arpeggiator.ts` |
| **MODIFY** | `dvpe_CLD/src/core/blocks/definitions/index.ts` (add export) |
| **MODIFY** | `dvpe_CLD/src/core/blocks/BlockRegistry.ts` (register) |
| **NEW** | `_block_diagrams_code/field_arpeggiator_v2.dvpe` |
| **NEW** | `DaisyExamples/MyProjects/_projects/FieldArpeggiator/` (C++ project) |

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
