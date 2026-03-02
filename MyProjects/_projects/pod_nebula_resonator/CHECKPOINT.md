# Checkpoint: Nebula-Resonator — Daisy Pod
## Last Updated: 2026-02-10

## Status: COMPLETE (Build Verified + Optimized)

### Project Summary
Standalone MIDI-driven granular texture synthesizer for Daisy Pod.
Internal synth drone (BlOsc + WhiteNoise) fills a 2-second SDRAM freeze buffer.
When frozen, a 4-tap granular engine with jittered read heads creates evolving
stereo cloud textures. An 8-step parameter sequencer animates scan position.

### Platform Target
[ ] Seed  [ ] Field  [x] Pod

### DAISY_QAE Workflow
- [x] Step 1: CONCEPT — Distilled from 3 reference documents
- [x] Step 2: BLOCK DIAGRAMS — 3 Mermaid diagrams in CONTROLS.md
- [x] Step 3: CONTROLS.md — Full parameter mapping + defaults
- [x] Step 4: IMPLEMENTATION — All source files created
- [x] Step 5: VERIFY — Build succeeds (0 errors, 0 warnings)

### Completed Steps
- [x] Document analysis (Nebula-Resonator.docx, FSM_SuperController.pdf, meta-controller.pdf)
- [x] CONTROLS.md with 3 Mermaid block diagrams (System Architecture, Signal Flow, Control Flow)
- [x] freeze_buffer.h — SDRAM circular buffer, conditional write, interpolated read, pre-fill
- [x] texture_voice.h — 4-tap granular engine with Jitter, stereo panning
- [x] pod_nebula_resonator.cpp — AudioCallback, 3-page FSM, soft takeover, MIDI, sequencer
- [x] Makefile — USE_DAISYSP_LGPL=1, 3-level paths
- [x] README.md — Overview + quick reference
- [x] Build verified: `make clean && make` — 0 errors, 0 warnings
- [x] Code optimization pass (while→if wrapping, constexpr float, dead code removal, cache reverb mix)

### Build Output (Post-Optimization)
| Region | Used | Total | % |
|--------|------|-------|---|
| FLASH | 87,956 B | 128 KB | 67.11% |
| SRAM | 463,108 B | 512 KB | 88.33% |
| SDRAM | 384,008 B | 64 MB | 0.57% |

### Optimizations Applied
| Change | File | Rationale |
|--------|------|-----------|
| `while` → single `if` in `Read()` | freeze_buffer.h | Deterministic timing in hottest path (4 calls/sample) |
| `constexpr float FREEZE_BUFFER_SIZE_F` | freeze_buffer.h | Avoid int→float cast per Read() call |
| Remove upper-bound while in Read() | freeze_buffer.h | Dead — delay clamped by callers guarantees read_f < SIZE |
| Use `FREEZE_BUFFER_SIZE_F` | texture_voice.h | Eliminates 2 static_cast per grain per sample |
| Remove unused `Port scan_slew` | .cpp | Dead code |
| `% 8` → `& 7` for seq_idx | .cpp | Branchless on signed int |
| `while` → `if` for scan wrap | .cpp | Both addends [0,1], sum ≤ 2, one pass always sufficient |
| Cache `1.0f - smooth_rev_mix` | .cpp | Eliminate redundant subtraction per stereo pair |
| Remove `dry_l`/`dry_r` aliases | .cpp | Fewer register moves |

### State Variables
| Variable | Value |
|----------|-------|
| sample_rate | 48000 |
| block_size | 4 |
| buffer_format | Interleaved (Pod) |
| lgpl_modules | true (ReverbSc) |
| freeze_buffer_size | 96000 (2 sec) |
| num_grains | 4 |
| sequencer_steps | 8 |
| fsm_pages | 3 (Source/Granular/Motion) |

### Project Files
```
pod_nebula_resonator/
├── pod_nebula_resonator.cpp   # Main: AudioCallback, FSM, MIDI, Sequencer
├── freeze_buffer.h            # FreezeBuffer class (96k SDRAM)
├── texture_voice.h            # TextureVoice (4-tap granular)
├── Makefile                   # USE_DAISYSP_LGPL=1, 3-level paths
├── CONTROLS.md                # 3 Mermaid block diagrams + parameter tables
├── README.md                  # Overview + quick reference
├── CHECKPOINT.md              # This file
├── documentation/             # Reference documents
│   ├── Nebula-Resonator.docx
│   ├── FSM_SuperController.pdf
│   ├── meta-controller.pdf
│   └── Granular_Time-Texture_Freeze
└── build/                     # Compiled output (make clean && make)
    ├── pod_nebula_resonator.bin
    ├── pod_nebula_resonator.hex
    └── pod_nebula_resonator.elf
```

### DaisySP Modules Used
| Module | Purpose | LGPL? |
|--------|---------|-------|
| BlOsc | Internal oscillator (saw/square via PW) | No |
| WhiteNoise | Noise source for timbre blend | No |
| Jitter (x4) | Brownian grain position randomization | No |
| Metro | Internal sequencer clock (4 Hz) | No |
| ReverbSc | Stereo reverb output stage | **Yes** |

### Known Issues / Notes
- `source_decay` parameter (Knob 2, Page 0) is read from hardware but not yet
  wired to any envelope — currently a no-op placeholder
- MIDI clock sync and internal Metro can both advance seq_idx — no conflict
  guard (works fine in practice since MIDI clock disables internal advance)
- LED state computed every audio block (12 kHz) — acceptable for Pod but could
  be throttled if CPU budget tightens

### Next Steps (If Continuing)
- [ ] Flash to hardware: `make program` (ST-Link)
- [ ] Wire `source_decay` to an AdEnv envelope on the internal source
- [ ] Hardware test: Verify 3 encoder pages, freeze toggle, sequencer
- [ ] Optional: Add encoder-push to edit sequencer step values
