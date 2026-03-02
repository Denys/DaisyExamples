# Genetic Step Sequencer for Daisy Field

A 16-step MIDI sequencer that uses genetic algorithms to evolve musical patterns.

## Features
- Population-based sequence evolution
- Fitness evaluation (melodic contour, density, range)
- Tournament selection, crossover, mutation
- MIDI keyboard seeding
- OLED visualization
- LED feedback on keyboard keys

## Controls

### Knobs (K1-K8)
| Knob | Parameter | Range |
|------|-----------|-------|
| K1 | Mutation Rate | 0-50% |
| K2 | Crossover Rate | 50-100% |
| K3 | Population Size | 10-50 |
| K4 | Contour Bias | -1 to +1 |
| K5 | Target Density | 25-100% |
| K6 | Tempo | 40-240 BPM |
| K7 | Gate Length | 10-100% |
| K8 | Swing | 0-50% |

### KEY_A Row (Function Keys)
| Key | Function |
|-----|----------|
| A1 | Evolve (single generation) |
| A2 | Auto-Evolve toggle |
| A3 | Randomize population |
| A4 | Seed from MIDI |
| A5 | Play/Stop |
| A6 | Select Best |
| A7 | Undo |
| A8 | (Reserved) |

### KEY_B Row (Step Control)
| Key | Function |
|-----|----------|
| B1-B8 | Toggle steps 1-8 active/inactive |

### LED Feedback
- **KEY_A LEDs**: Show function state (Auto mode, Playing, MIDI capture)
- **KEY_B LEDs**: Show sequence steps (bright = current step, dim = active)

## Build & Flash

```bash
make clean && make
make program-dfu
```

## Architecture

```
GeneticSequencer/
├── GeneticSequencer.cpp     # Main + Audio (DaisyField)
├── genetic_algorithm.h/cpp  # GA Engine
├── sequencer.h/cpp          # Step Sequencer
└── Makefile
```
