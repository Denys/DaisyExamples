# EDGE Performance FX — Current Settings & System State

**Firmware version:** v1.0.0
**Phase:** 3 (Delay core with SDRAM buffer + tap tempo + feedback filters implemented; hardware verification pending)
**Last updated:** 2026-04-12

---

## Hardware Configuration

| Component | Specification |
|-----------|--------------|
| Platform | Daisy Pod (old DIP-socket revision) |
| MCU | STM32H750IB (ARM Cortex-M7, 480 MHz) |
| SRAM | 512 KB (10% used at current build) |
| SDRAM | 64 MB (reserved for delay buffer — Phase 2) |
| Flash | 128 KB (77% used at current build) |
| Audio | Mono In (Pod L), Dual-Mono Out (Pod L+R) |
| Sample Rate | 48 kHz |
| Block Size | 48 samples |

### External Board Pin Map

| Signal | Seed Pin | Function |
|--------|----------|----------|
| TRA | D7 | Ext Encoder A-phase |
| TRB | D8 | Ext Encoder B-phase |
| PSH | D9 | Ext Encoder push button |
| CON | D10 | Confirm button |
| BAK | D22 | Back button |
| SCL | D11 | I2C clock (OLED) |
| SDA | D12 | I2C data (OLED) |
| VCC | 3V3 | Power |
| GND | GND | Ground |

**OLED:** SSD1306 128×64, I2C address 0x3C, I2C_1 bus, 400 kHz

---

## Source File Structure

```
POD_EDGE_mono_DSP/
├── Makefile              — build: TARGET=EDGE_mono_DSP, block size 48, DAFX lib included
├── ExtEncoder.h          — external board wrapper (copy from Pod_OLED_EuclideanDrums)
├── parameters.h          — FxParams struct + BeatDiv subdivision table (kNumDivs=10)
├── presets.h             — 10 presets: InitPresets(), kPresetNames[], kNumPresets=10
├── display.h / .cpp      — OLED renderer (6 pages, Font_6x8, scrollable P5)
├── ui_state.h / .cpp     — event-driven state machine + tap tempo + preset loading
├── main.cpp              — init, double-buffer ISR sync, main loop
├── Settings/
│   ├── SETTINGS.md       — this file
│   └── HISTORY.md        — modification log
├── REDESIGN_PLAN.md      — approved redesign spec; implemented in firmware
├── USER_MANUAL.md        — full user documentation + functionality test chapter
├── PHASE_1_2_TESTS.md    — dedicated hardware + hearing completion checklist
├── CONTROLS.md           — hardware control reference tables
├── PARAMETER_TABLES.md   — per-page parameter tables with ranges and edit steps
├── PAGES_CONTROLS_EFFECTS.md — 3 Mermaid visual maps (pages, controls, DSP chain)
└── EDGE-oriented_mono_DSP_effects_instrument.md — original SOW + §16 QAE compliance + §17 diagrams
```

---

## Current Control Assignments

> **Note:** These are the CURRENT assignments after the approved redesign.

### Global (all pages)

| Control | Function |
|---------|---------|
| SW1 short tap (<200 ms) | Freeze momentary (on while held) |
| SW1 hold (>200 ms) | Shift modifier active |
| Ext Encoder turn | Menu cursor navigation (or value edit in edit mode) |
| Ext PSH short | Enter/exit edit mode (or "select" on P5) |
| CON | Confirm / load preset (P5) |
| BAK | Exit edit mode / return to P1 |

### Pod Controls (PAGE-AWARE)

| Page | Normal layer | Shift layer |
|------|--------------|-------------|
| P1 Perform | K1 Time, K2 Feedback, Enc Wet, SW2 Tap, EncPush Freeze | K1 Subdiv, K2 FB LP, Enc Drive, SW2 Sync toggle |
| P2 Tone | K1 HP, K2 FB LP, Enc Tilt, SW2 Bypass, EncPush Tilt reset | K1 FB HP, K2 Damp, Enc Input Gain, SW2 Sync toggle |
| P3 Motion | K1 Wow Depth, K2 Wow Rate, Enc fine rate, SW2 Wow toggle, EncPush Wow toggle | K1 Feedback, K2 Wet, Enc Drive, SW2 Tap |
| P4 Freeze | K1 Freeze Size, K2 Feedback, Enc fine size, SW2 Loop/Hold, EncPush Freeze | K1 Delay Time, K2 Wet, Enc Damp, SW2 Latch-mode toggle |
| P5 Presets | K1 Scroll, Enc Scroll, SW2 Load, EncPush Load | Page jump only |
| P6 System | K1 Brightness, Enc Brightness, SW2 Bypass, EncPush Bypass | SW2 Encoder direction |

### Preset Page (P5) Special Controls

| Control | Function |
|---------|---------|
| Ext PSH | "Select" preset (shows CON=LOAD hint, brackets around name) |
| CON | Load selected preset → return to P1 |
| BAK (when selected) | Cancel selection, return to browse |

---

## FxParams Struct — All Fields

```cpp
struct FxParams {
    // P1 Perform
    float input_gain;        // 0.5–2.0  (default 1.0)
    float drive;             // 0.0–1.0  (default 0.25)
    int   subdiv_idx;        // 0–9      (default 6 = 1/4 note)
    float delay_time_ms;     // 10–2000  (default 500 ms)
    float feedback;          // 0.0–0.98 (default 0.50)
    float wet;               // 0.0–1.0  (default 0.40)
    float bpm;               // 40–240   (default 120)
    bool  sync_mode;         // (default true)
    bool  freeze_momentary;  // live flag
    bool  freeze_latched;    // (default false)
    bool  freeze_latch_mode; // (default false = momentary only)

    // P2 Tone
    float hp_hz;             // 20–500    (default 80 Hz)
    float fb_lp_hz;          // 500–18000 (default 6000 Hz)
    float fb_hp_hz;          // 20–500    (default 60 Hz)
    float diffuse_damping;   // 1000–20000 (default 8000 Hz)
    float output_tilt_db;    // -6 to +6  (default 0 dB)

    // P3 Motion
    float wow_depth;         // 0.0–1.0   (default 0.0)
    float wow_rate_hz;       // 0.1–5.0   (default 1.2 Hz)
    bool  wow_enabled;       // (default false)

    // P4 Freeze
    bool  freeze_loop_mode;  // (default true = loop)
    float freeze_size_ms;    // 50–2000   (default 500 ms)

    // P6 System
    uint8_t brightness_idx;  // 0..3      (default 2 = 75%)
    bool  bypass;            // (default false)
    bool  enc_flipped;       // (default false)
};
```

---

## Page Structure

| Page | ID | Items | Primary Contents |
|------|-----|-------|-----------------|
| PERFORM | 0 | 5 | Time, Feedback, Wet/Dry, Drive, Freeze |
| TONE | 1 | 5 | Input HP, FB LP, FB HP, Wet Tone, Out Tilt |
| MOTION | 2 | 3 | Wow Depth, Wow Rate, Mod Enable |
| FREEZE | 3 | 3 | Mode, Behavior, Loop Size |
| PRESETS | 4 | 10 | Preset slots P01–P10 (scrollable) |
| SYSTEM | 5 | 3 | Brightness, Enc Direction, Bypass |

---

## Preset Slots

| Slot | Name | Purpose |
|------|------|---------|
| P01 | KickDub | Performance — dotted-eighth, warm |
| P02 | SnareSpace | Performance — quarter-note room |
| P03 | MetalBurst | Performance — 16th, high feedback |
| P04 | NoiseWash | Performance — half-note ambient |
| P05 | BassEchDrv | Performance — driven bass echo |
| P06 | T:InitChek | Test — dry passthrough (wet=0, FB=0) |
| P07 | T:MaxFeedb | Test — feedback at 95% |
| P08 | T:WowDeep | Test — wow depth 80%, rate 2 Hz |
| P09 | T:FullDriv | Test — drive 100%, input gain 1.5 |
| P10 | T:FreezePd | Test — latch mode, freeze pad |

---

**Current active chain in `main.cpp`:**
`input_gain -> DcBlock -> Svf high-pass -> Overdrive -> DelayLine(SDRAM) -> feedback filters (LP/HP + Saturation) -> dual-mono out`

**Current inactive stages:** wow/flutter DSP, diffusion, output tilt, limiter.

Signal path intent (next phases):
```
In → Trim → DcBlock → Svf(HP) → Overdrive → DelayLine(SDRAM)
  → feedback: Svf(HP) → Svf(LP) → Phasor(wow) → tanh(sat) → back
  → wet: FDNReverb → CrossFade(mix)
  → dry: CrossFade(mix) → Biquad(tilt) → Limiter → Out L+R
```

DSP modules state:
- `DcBlock` — active
- `Svf` input HP — active
- `Overdrive` — active
- callback smoothing for `input_gain`, `drive`, `hp_hz`, `delay`, `feedback`, `fb filters` — active
- `Svf` feedback HP / LP — active
- `DelayLine<float,MAX>` — SDRAM delay buffer active
- `Phasor` — wow/flutter LFO
- `FDNReverb<8192>` — from DAFX_2_Daisy_lib (no LGPL)
- `Biquad` — output tilt EQ
- `Limiter` — output protection

---

## Build Commands

```bash
cd DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP
make program              # flash via ST-Link (requires WinUSB driver on Windows)
make program-dfu          # flash via USB DFU (hold BOOT + press RESET first)
```

### Rebuild paths

**Default current path — Git Bash / POSIX shell**

```bash
cd DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP
make clean && make
```

**Codex current path — Git Bash with escalated permissions**

```powershell
& 'C:\Program Files\Git\bin\bash.exe' -lc "cd '/c/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP' && make clean && make"
```

**Codex fallback — PowerShell-safe rebuild**

```powershell
Set-Location 'DaisyExamples\MyProjects\_projects\POD_EDGE_mono_DSP'
if(Test-Path build){Remove-Item -LiteralPath build -Recurse -Force}
make
```

> `make clean` comes from the upstream `libDaisy` Makefile and uses `rm -fR build`, so plain PowerShell cannot run that target directly without a POSIX shell.

**Current build stats (Phase 4):** Flash 104360 B / 128 KB (79.62%), SRAM 315036 B / 512 KB (60.09%), RAM_D2 16960 B / 288 KB (5.75%), SDRAM 384012 B / 64 MB (0.57%)

---

## Known Issues / Open Items

| Issue | Status | Notes |
|-------|--------|-------|
| ST-Link flash fails | Open | Needs WinUSB driver (Zadig) on Windows — `openocd: open failed` |
| Delay / feedback DSP not active | Closed | Phase 3 integrated: SDRAM delay buffer, sync/free time, feedback HP/LP paths active |
| Output tilt / limiter / diffusion not active | Closed | Phase 4 integrated: ToneStack Tilt EQ, Limiter, FDNReverb, Wow Phasor |
| Preset save to flash | Not implemented | Phase 6 |
| Erroneous noderr/ copy | Low priority | `DaisyExamples/MyProjects/_projects/POD_EDGE_mono_DSP/noderr/` should be deleted |
| OLED brightness control | Stub | UI stores `brightness_idx`, but OLED hardware contrast call is not wired |
| USER_MANUAL / control docs drift | Open | Reconcile docs with redesigned control map and active Phase 2 audio chain |
