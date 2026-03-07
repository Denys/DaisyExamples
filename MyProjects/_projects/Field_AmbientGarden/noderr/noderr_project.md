# Noderr Project: Field_AmbientGarden Firmware

**Version:** 1.0
**Date:** 2026-03-06
**Platform:** Daisy Field (Electro-Smith)
**Language:** C++17 / libDaisy / DaisySP

---

## 1. Project Purpose

A self-playing generative ambient synthesizer that requires no performer input after initialization. A Turing Machine shift register generates pseudo-random sequences, quantized to one of 8 musical scales, driving 4 Modal voices through stereo reverb. The system is probabilistic: each clock tick has a chance of triggering 1–4 voices at different harmonic intervals.

**Complexity Rating:** 9/10

---

## 2. Technology Stack

| Component | Library / Source | Notes |
|-----------|-----------------|-------|
| Hardware HAL | `libDaisy` (daisy_field.h) | Non-interleaved audio, 8 knobs, 16 keys, OLED |
| DSP Framework | `DaisySP` | Requires `USE_DAISYSP_LGPL = 1` |
| Modal Synthesis | `ModalVoice` (DaisySP LGPL) | 4 voices |
| Reverb | `ReverbSc` (DaisySP LGPL) | Stereo |
| Filtering | `Svf` (DaisySP) | Pre-reverb LPF, stereo pair |
| Generative Clock | `random_clock.h` (custom) | Probabilistic trigger rate |
| Sequence Generator | `turing_machine.h` (custom) | 8-bit shift register w/ probability |
| Scale Quantizer | `scale_quantizer.h` (custom) | 8 musical scales |
| Field UI Helpers | `field_defaults.h` | FieldKeyboardLEDs, FieldOLEDDisplay |

---

## 3. Scope & Key Features

### In Scope
- Generative sequencing via TuringMachine + ScaleQuantizer
- 4-voice probabilistic polyphony with harmony spread
- 8 voice character presets (Glass, Marimba, Metal, Soft Pad, Gamelan, WindChime, Steel Drum, Temple)
- 8 musical scale modes (Pentatonic Major through Chromatic)
- Stereo reverb with wet/dry mix
- OLED parameter zoom on knob change (2s timeout)
- A-row exclusive scale selection, B-row exclusive preset selection
- SW1 freeze/run toggle, SW2 stereo width toggle
- LED feedback: knob brightness, voice trigger pulses, scale/preset indicators

### Out of Scope
- MIDI input (no external keyboard — this is fully generative)
- USB MIDI
- CV/Gate I/O (no modular integration planned)
- Preset save/recall to persistent storage

---

## 4. Architecture Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Audio buffer format | Non-interleaved (`out[0][i]`, `out[1][i]`) | Daisy Field platform requirement |
| Block size | `kRecommendedBlockSize` (48 from field_defaults.h) | Balances latency vs CPU efficiency |
| Parameter communication | `Params` struct (main→callback) | Avoids calling Process() in callback |
| OLED update location | Main loop only | OLED is not real-time safe in callback |
| Control reads | Main loop only (after ProcessAllControls) | Prevents double-processing |
| Random generation | xorshift32 PRNG (inline, lock-free) | `rand()` is NOT callback-safe |
| Modal voice smoothing | Raw params at 60Hz main loop rate | 60Hz is sufficient; avoids dead code |

---

## 5. Known Technical Debt

| ID | Description | Priority | Tracker Ref |
|----|-------------|----------|-------------|
| TD-1 | FieldUX library not integrated (using field_defaults.h helpers) | Low | NC-1 |
| TD-2 | Frozen OLED state not fully implemented (CONTROLS.md mismatch) | Low | MISMATCH-1 |
| TD-3 | Custom `Smooth()` instead of DaisySP `fonepole()` | Cosmetic | NC-2 |

---

## 6. File Structure

```
Field_AmbientGarden/
├── Field_AmbientGarden.cpp   ← Main source
├── turing_machine.h          ← GEN_TuringMachine
├── scale_quantizer.h         ← GEN_ScaleQuantizer
├── random_clock.h            ← GEN_RandomClock
├── Makefile                  ← USE_DAISYSP_LGPL = 1 required
├── CONTROLS.md               ← Control mapping documentation
├── DIAGRAMS.md               ← Mermaid block diagrams
├── Field_AmbientGarden.dvpe  ← DVPE block diagram
├── README.md
└── noderr/                   ← This Noderr tracking instance
    ├── noderr_project.md
    ├── noderr_architecture.md
    ├── noderr_tracker.md
    ├── noderr_log.md
    └── specs/
        ├── FW_AudioCallback.md
        ├── FW_ParamBridge.md
        ├── FW_ClockTrigger.md
        ├── DSP_ModalVoices.md
        └── GEN_TuringMachine.md
```

---

## 7. Session Startup Protocol

At the start of every firmware development session:
1. Read `noderr_project.md` (this file) — project context
2. Read `noderr_tracker.md` — current node statuses and active WorkGroupIDs
3. Read `noderr_log.md` (last 3 entries) — recent changes
4. Read `noderr_architecture.md` — system topology
5. Identify the target NodeID(s) for the session goal
6. Follow the Noderr Loop (see root `noderr/noderr/noderr_loop.md`)

---

## 8. Build & Flash

```bash
# Build
cd DaisyExamples/MyProjects/_projects/Field_AmbientGarden
make clean && make

# Flash (ST-Link)
make program

# Flash (DFU)
make program-dfu
```

**Required Makefile flag:** `USE_DAISYSP_LGPL = 1` (for ModalVoice + ReverbSc)
