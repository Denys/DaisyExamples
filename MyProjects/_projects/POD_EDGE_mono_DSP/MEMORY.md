# EDGE Performance FX — Agent Memory
**Read this first. Update this when anything changes.**
**Last updated:** 2026-04-12 | **Phase:** 2 — audio skeleton + page-aware controls implemented, hardware verification pending

---

## What This Project Is

Mono DSP effects instrument on **Daisy Pod** + **external I2C board** (OLED, encoder, BAK, CON).
Signal chain intent: Input Conditioning → Soft Drive → Tempo-Synced Delay → Dark Diffusion → Wet/Dry → Output Tilt → Limiter → Dual-Mono Out.
**Current audio state:** input gain, `DcBlock`, input `Svf` high-pass, `Overdrive`, and callback smoothing are active. Delay, diffusion, output tilt, limiter, and freeze DSP remain for later phases.

---

## Phase Status

| Phase | What | Status |
|-------|------|--------|
| 1 | Hardware bring-up + full UI state machine | ✅ Complete (builds, 0 warnings) |
| 2 | Audio skeleton: DcBlock + Svf HP + Overdrive + param smoothing | ✅ Implemented (clean build verified) |
| 3 | Delay core: DelayLine SDRAM + tap tempo + sync + feedback filters | ✅ Implemented (clean build 384KB SDRAM verified) |
| 4 | Wow/Flutter + FDNReverb + output tilt + Limiter | ✅ Implemented (clean build, valid memory bounds) |
| 5 | Freeze/stutter engine | ❌ Not started |
| 6 | Preset save/load to flash | ❌ Not started |

---

## Critical Facts — Always Remember

| Fact | Value |
|------|-------|
| Audio callback | NON-INTERLEAVED: `out[0][i] = out[1][i] = sig` |
| Block size | 48 (not 4 — heavy DSP headroom) |
| Sample rate | 48 kHz |
| Delay buffer | MUST be `DSY_SDRAM_BSS` (2 s = 384 KB, exceeds SRAM) |
| LGPL flag | NOT needed — using `FDNReverb` from DAFX lib, not ReverbSc |
| ExtEncoder.h | Copied from `Pod_OLED_EuclideanDrums/` — do NOT rewrite |
| OLED | I2C_1, SCL=D11, SDA=D12, 0x3C, `SSD130xI2c128x64Driver` |
| BAK pin | D22 via raw GPIO — D17 (LED2_R) is OUTPUT, must not use Switch on it |
| Init order | `hw.StartAdc()` BEFORE `hw.StartAudio()` |
| Thread safety | double-buffer `param_buf[2]` + `volatile active_buf_` + `__DSB()` |
| `Overdrive::Init()` | Takes NO sample_rate argument — just `Init()` |
| DAFX lib path | `../../DAFX_2_Daisy_lib/src/effects/` (relative from project root) |
| libDaisy path | `../../../libDaisy` (3 levels up from `_projects/NAME/`) |

---

## File Map — What Lives Where

```
POD_EDGE_mono_DSP/
│
├── MEMORY.md              ← YOU ARE HERE — read/update every session
├── REDESIGN_PLAN.md       ← Approved redesign spec; firmware implementation landed
│
├── Settings/
│   ├── SETTINGS.md        ← full current state: params, pins, build stats, issues
│   └── HISTORY.md         ← chronological change log (append new entries)
│
├── Firmware/
│   ├── Makefile           — TARGET=EDGE_mono_DSP, block 48, DAFX included
│   ├── ExtEncoder.h       — external board wrapper (copy, proven)
│   ├── parameters.h       — FxParams struct + BeatDiv[10] table
│   ├── presets.h          — 10 presets, InitPresets(), kPresetNames[]
│   ├── display.h/.cpp     — 6-page OLED renderer (Font_6x8, scrollable P5)
│   ├── ui_state.h/.cpp    — event state machine, tap tempo, preset load
│   └── main.cpp           — init, ISR, main loop
│
├── Documentation/
│   ├── USER_MANUAL.md     — §1–12 manual + §13 functionality test (10 tests)
│   ├── CONTROLS.md        — hardware pin map, control routing, FxParams ref
│   ├── PARAMETER_TABLES.md — per-page tables: parameter/range/control/edit-step
│   ├── PAGES_CONTROLS_EFFECTS.md — 3 Mermaid maps (pages, controls, DSP)
│   └── PHASE_1_2_TESTS.md — dedicated completion checklist for hardware + listening tests
│
└── SOW/
    └── EDGE-oriented_mono_DSP_effects_instrument.md — original brief + §16 QAE + §17 diagrams
```

> **Note:** Firmware files are currently at project root, not in `Firmware/` subfolder.
> The subfolder layout above is aspirational. Actual location: all `.h/.cpp/Makefile` at root.

---

## Current Control System (PAGE-AWARE)

| Control | Normal | Shift (SW1 hold >200 ms) |
|---------|--------|--------------------------|
| P1 K1 / K2 / Enc / SW2 | Time, Feedback, Wet, Tap | Subdiv, FB LP, Drive, Sync toggle |
| P2 K1 / K2 / Enc / SW2 | HP, FB LP, Tilt, Bypass | FB HP, Damp, Input Gain, Sync toggle |
| P3 K1 / K2 / Enc / SW2 | Wow Depth, Wow Rate, Fine Rate, Wow toggle | Feedback, Wet, Drive, Tap |
| P4 K1 / K2 / Enc / SW2 | Freeze Size, Feedback, Fine Size, Loop/Hold | Delay Time, Wet, Damp, Latch-mode toggle |
| P5 K1 / Enc / SW2 | Preset scroll, Preset scroll, Load | — |
| P6 K1 / Enc / SW2 | Brightness, Brightness, Bypass | —, —, Encoder direction |
| SW1 short (<200 ms) | Freeze momentary | — |
| SW1 hold | Shift modifier | — |
| Pod Enc push | Page-aware action on current page | Page Jump |
| Ext Enc turn | Menu cursor / value (edit mode) | — |
| Ext PSH | Edit mode toggle / preset select | — |
| CON | Confirm / load preset | — |
| BAK | Exit edit / go to P1 | — |

### Implementation Summary (Up to Phase 3)
1. **Phase 1 (UI & Controls):** Deeply refactored initial architecture. Decoupled analog and digital polling: encoders/switches process in a 1kHz Timer ISR, analog knobs sample directly in the `AudioCallback` at 1kHz. This completely resolved "sluggish/swallowed" knob turns and encoder unresponsiveness. `REDESIGN_PLAN.md` mapping implemented fully.
2. **Phase 2 (Audio Frontend):** Implemented DcBlock, Input High-Pass, and soft-clipping Overdrive. Bound all continuous parameters to `fonepole()` math inside the `AudioCallback` to prevent CPU cross-thread tearing and audio zipper artifacts during rapid UI knob sweeps.
3. **Phase 3 (Delay Core):** Successfully assigned a 384KB `DelayLine` array directly into the 64MB external memory chip via `DSY_SDRAM_BSS`, preserving precious `.ebss`. Configured a feedback path with discrete HP/LP filters, restrained runaway oscillations via `tanhf()` saturation, and tied Tap Tempo directly to the delay time subdiv-math.
---

## Pages

| Page | Ext Enc MaxCursor | Key parameters |
|------|-------------------|----------------|
| P1 PERFORM | 5 | Time, Feedback, Wet/Dry, Drive, Freeze |
| P2 TONE | 5 | InputHP, FbLP, FbHP, WetDamp, OutTilt |
| P3 MOTION | 3 | WowDepth, WowRate, ModEnable |
| P4 FREEZE | 3 | Mode, Behavior, LoopSize |
| P5 PRESETS | 10 | Scrollable 10 slots; Ext PSH = select, CON = load |
| P6 SYSTEM | 3 | Brightness, EncDir, Bypass |

**Navigation:** Ext Enc scrolls past boundary → next/prev page. BAK → P1. Shift+Pod Enc push → next page.

---

## Preset Slots

| # | Name | Notable values |
|---|------|----------------|
| P01 | KickDub | subdiv=1/8d, FB=40%, wet=45%, FbLP=3kHz |
| P02 | SnareSpace | subdiv=1/4, FB=35%, wet=40% |
| P03 | MetalBurst | subdiv=1/16, FB=65%, drive=30%, bpm=140 |
| P04 | NoiseWash | subdiv=1/2, FB=72%, wet=70%, FbLP=4kHz |
| P05 | BassEchDrv | subdiv=1/8, drive=65%, FbLP=4kHz |
| **P06** | **T:InitChek** | **wet=0, FB=0, drive=0 — baseline passthrough** |
| **P07** | **T:MaxFeedb** | **FB=95% — stability test** |
| **P08** | **T:WowDeep** | **wow=80%, rate=2Hz, enabled=true** |
| **P09** | **T:FullDriv** | **drive=100%, input_gain=1.5** |
| **P10** | **T:FreezePd** | **latch_mode=true, loop_size=800ms** |

---

## Open Issues

| Priority | Issue | Where | Action |
|----------|-------|--------|--------|
| 🔴 Must fix before hardware verification | ST-Link flash fails | Windows — WinUSB driver missing | Zadig → install WinUSB for STM32 STLink or use DFU |
| 🟢 Phase 5 required | Freeze engine implementation | `main.cpp` | Momentary/Latch loop micro-buffer logic |
| 🟡 Phase 6 | Preset save to flash | `ui_state.cpp` P5 stub | Implement flash persistence |
| 🟢 Low | OLED brightness not wired | `display.cpp` P6 | Add hardware contrast command path; UI already stores `brightness_idx` |
| 🟢 Low | `USER_MANUAL.md` / control docs drift | Docs | Reconcile manual tables with page-aware redesign + Phase 2 audio state |
| 🟢 Low | Erroneous noderr/ copy | `_projects/POD_EDGE_mono_DSP/noderr/` | Delete entire folder |

---

## FxParams Quick Reference

```
input_gain   0.5–2.0  (1.0)      hp_hz         20–500    (80)
drive        0.0–1.0  (0.25)     fb_lp_hz    500–18000  (6000)
subdiv_idx   0–9      (6=1/4)    fb_hp_hz      20–500    (60)
delay_ms     10–2000  (500)      diffuse_damp 1000–20000 (8000)
feedback     0–0.98   (0.50)     output_tilt  -6–+6 dB   (0)
wet          0–1.0    (0.40)     wow_depth    0–1.0      (0)
bpm          40–240   (120)      wow_rate     0.1–5.0    (1.2)
sync_mode    bool     (true)     wow_enabled  bool       (false)
freeze_*     bools    (false)    freeze_size  50–2000    (500)
bypass       bool     (false)    enc_flipped  bool       (false)
brightness   0..3     (2=75%)
```

---

## Phase 4 Unlock Checklist

Before Phase 4 diffusion / output work, verify:
- [ ] ST-Link flash working (hardware test required)
- [ ] Phase 3 delay/feedback features fully verified on hardware (run `PHASE_3_TESTS.md`)
- [ ] Phase 4 modules (Limiter, Biquad Tilt, FDNReverb) initialized in `main.cpp`
- [ ] Clean rebuild passes using one verified path:
  - Default / Git Bash: `make clean && make`
  - Codex with escalated Git Bash: `bash -lc "cd .../POD_EDGE_mono_DSP && make clean && make"`
  - Codex / PowerShell-safe fallback: `if(Test-Path build){Remove-Item -LiteralPath build -Recurse -Force}; make`

---

## How to Update This File

After every session, update:
1. **"Last updated"** date in the header
2. **Phase Status** table — tick completed phases
3. **Open Issues** table — add/close/reprioritize items
4. **Any changed facts** in Critical Facts or FxParams Quick Reference
5. Add a new entry in `Settings/HISTORY.md` with what changed and why
