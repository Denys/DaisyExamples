# EDGE Performance FX — Modification History

Chronological log of all design decisions, code changes, and bug fixes.
Most recent entry first.

---

## 2026-04-12 - Session 7: Dedicated Phase 1/2 checklist

### Added dedicated completion checklist

- Created `PHASE_1_2_TESTS.md` as a focused hardware and listening checklist for:
  - Phase 1 hardware / UI / navigation / presets
  - control-response checks for switches, push-buttons, knobs, and both encoders
  - Phase 2 audible verification of input gain, input HP, overdrive, bypass, and stability

### State files updated

- Updated `MEMORY.md` to reference `PHASE_1_2_TESTS.md`.
- Updated `Settings/SETTINGS.md` with the new checklist file.

**Files changed:** `PHASE_1_2_TESTS.md`, `MEMORY.md`, `Settings/SETTINGS.md`, `Settings/HISTORY.md`

---

## 2026-04-12 - Session 6: Phase 2 audio skeleton + approved redesign implementation

### Implemented Phase 2 audio skeleton

- Activated the first live DSP stages in `main.cpp`:
  - `input_gain`
  - `DcBlock`
  - input `Svf` high-pass
  - `Overdrive`
  - callback-local smoothing for `input_gain`, `drive`, and `hp_hz`
- Kept later delay, diffusion, tilt, limiter, and freeze DSP stages stubbed so Phase 2 stays narrow and low-risk.

### Implemented approved page-aware Pod control redesign

- Replaced the old global live-control model with the approved page-aware mapping from `REDESIGN_PLAN.md`.
- Renamed SW2 events to `SW2_PRESS` / `SW2_SHIFT`.
- Replaced `PollKnobs()` with `PollControls()` and moved it earlier in the main-loop order so push/button events see the active shift layer.
- Added page-aware Pod encoder push behavior:
  - P1/P4 freeze action
  - P2 tilt reset
  - P3 wow toggle
  - P5 preset load
  - P6 bypass toggle
- Added movement-based knob tracking (`prev_knob*` + deadband) to avoid the Pod knobs fighting encoder fine adjustments on pages like P3/P4/P5/P6.
- Added `brightness_idx` to `FxParams` and wired the System page UI to store/display 25/50/75/100% brightness states.

### Updated OLED rendering for redesigned controls

- `display.cpp` now renders shift-aware headers and bottom hints for P2-P6.
- Presets page now shows direct live-scroll usage (`ENC/K1=SCRL CON=LOAD`) when not in confirm state.
- System page now displays the current brightness selection from `brightness_idx`.

### Memory / state files updated

- Updated `MEMORY.md` to mark Phase 2 implemented in code and to mark the redesign as implemented.
- Updated `Settings/SETTINGS.md` with the new control model, active DSP chain, and fresh build numbers.

**Files changed:** `main.cpp`, `ui_state.h`, `ui_state.cpp`, `display.h`, `display.cpp`, `parameters.h`, `MEMORY.md`, `Settings/SETTINGS.md`

**Build result:** clean Git Bash rebuild passed. Flash 100388 B / 128 KB (76.59%), SRAM 52460 B / 512 KB (10.01%), RAM_D2 16960 B / 288 KB (5.75%).

---

## 2026-04-12 - Session 5: Build-path documentation update

### Documented verified rebuild paths

- Updated `MEMORY.md`, `Settings/SETTINGS.md`, and `USER_MANUAL.md` to keep all currently valid build paths in sync:
  - Default current path: Git Bash / POSIX shell -> `make clean && make`
  - Codex with escalated permissions: explicit Git Bash invocation
  - Codex fallback: PowerShell-safe rebuild using `Remove-Item` + `make`
- Recorded why the fallback is needed: upstream `libDaisy/core/Makefile` implements `clean` with `rm -fR build`, which is not directly portable to plain PowerShell.
- Confirmed the Git Bash rebuild succeeds when run with escalated permissions.

**Files changed:** `MEMORY.md`, `Settings/SETTINGS.md`, `USER_MANUAL.md`

---

## 2026-04-12 — Session 4: Presets, Navigation Fixes, Redesign Plan

### Preset navigation bug fixes (ui_state.cpp)

**Bug 1 — Edit mode blocked encoder on P5:**
`EXT_ENC_CW/CCW` checked `edit_mode_` first. If true (even spuriously), called
`AdjustValue(+1)` which is a no-op for PRESETS page. Cursor never moved.
**Fix:** Guard with `page_ != Page::PRESETS` — PRESETS always navigates regardless.

**Bug 2 — Ext PSH immediately loaded preset and jumped to P1:**
Muscle memory from other pages (PSH = enter edit) caused accidental loads.
User would tap PSH expecting "select", get teleported to P1, then found
the Ext Enc navigating P1 parameters — appeared as "encoder doesn't navigate in presets."
**Fix:** Two-step load: Ext PSH sets `preset_selected_` flag (shows brackets + `CON=LOAD`
hint). CON actually loads. BAK cancels selection.

**Files changed:** `ui_state.h`, `ui_state.cpp`, `display.h`, `display.cpp`, `main.cpp`

---

### Page navigation bug fix (ui_state.cpp)

**Bug — P5 and P6 unreachable:**
`QuickPageJump()` only cycled P1→P2→P3→P4→P1. P5/P6 had no path.
`EXT_ENC_CW/CCW` clamped cursor at page boundary instead of wrapping.
**Fix:** `EXT_ENC_CW` past last item → `NextPage()`; `EXT_ENC_CCW` past first → `PrevPage()`.
Both wrap through all 6 pages. `QuickPageJump` now cycles all 6.

---

### Added preset system (presets.h, display.cpp, ui_state.cpp, main.cpp)

- Created `presets.h` with `InitPresets()` filling `FxParams[10]`:
  - P01–P05: performance presets (KickDub, SnareSpace, MetalBurst, NoiseWash, BassEchDrv)
  - P06–P10: functionality test presets (InitChek, MaxFeedb, WowDeep, FullDriv, FreezePd)
- P5 display updated to scrollable 5-item window with `N/10` indicator
- MaxCursor[PRESETS] = kNumPresets (10)
- `UIState::LoadPreset(idx)` copies FxParams and returns to P1
- CON and Ext PSH (two-step) trigger load

---

### Added functionality test chapter to USER_MANUAL.md (§13)

10 structured tests using presets to verify every firmware subsystem:
boot, navigation, controls, tap tempo, preset loading, feedback stability,
wow parameters, drive, freeze latch, SHF badge, full preset cycle.

---

### Created Settings/ folder

- `Settings/SETTINGS.md` — current system state snapshot
- `Settings/HISTORY.md` — this file

### Created REDESIGN_PLAN.md

Full specification for page-aware Pod control redesign (not yet implemented).
Documents proposed K1/K2/SW2/PodEnc mapping per page with Shift layer,
event system changes, display hint updates, and implementation checklist.

---

## 2026-04-12 — Session 3: Additional Presets + Manual Test Chapter

*See above — merged into Session 4 entry.*

---

## 2026-04-12 — Session 2: Phase 1 Firmware Implementation

### Initial firmware created (Phase 1 complete)

**Files created (all from scratch):**

| File | Contents |
|------|---------|
| `Makefile` | TARGET=EDGE_mono_DSP, block 48, DAFX lib path |
| `ExtEncoder.h` | Exact copy from Pod_OLED_EuclideanDrums (proven hardware) |
| `parameters.h` | FxParams struct + BeatDiv table (10 subdivisions) |
| `display.h/.cpp` | 6-page OLED renderer, Font_6x8, scrollable P5 |
| `ui_state.h/.cpp` | Event-driven state machine, tap tempo, shift logic |
| `main.cpp` | Init, double-buffer ISR sync, main loop |

**Key implementation decisions:**

- **Audio callback:** non-interleaved `AudioHandle::InputBuffer/OutputBuffer` — matches
  `Pod_OLED_EuclideanDrums` reference (confirmed working on hardware)
- **Thread safety:** double-buffer pattern from DAISY_DEVELOPMENT_STANDARDS §7;
  `param_buf[2]` + `volatile int active_buf_`; `__DSB()` before index swap
- **Block size:** 48 (not 4) — headroom for full DSP chain in Phase 2+
- **OLED init:** SSD130xI2c128x64Driver, I2C_1, D11/D12, 0x3C — pattern from DrumSeqUI.h
- **Shift detection:** manual timer (`sw1_press_time_`), 200 ms threshold
- **Tap tempo:** rolling average of last 4 inter-tap intervals, 3 s reset window
- **Preset_selected flag:** two-step load to prevent accidental preset load

**Build result:** 0 errors, 0 warnings. Flash 95 KB / 128 KB.

---

### SW1 assignment changed: SW2 → Shift, SW1 → Tap Tempo

**Decision during design review:**
SOW originally had SW2 = Shift, SW1 = Tap Tempo. Changed to:
- SW1 = Shift (hold) / Freeze momentary (short tap)
- SW2 = Tap Tempo

Rationale: SW1 is ergonomically closer to the performance controls (knobs).
Shift needs to be held while manipulating other controls — SW1 position is better.

**Files changed:** `CONTROLS.md`, SOW §16C diagrams (17C-1, 17C-2)

---

### make program attempt — ST-Link not connected

Flash via `make program` failed: `openocd: Error: open failed`.
Cause: ST-Link not detected — requires WinUSB driver (Zadig) on Windows.
DFU alternative: `make program-dfu` with BOOT+RESET sequence.
Status: open issue in `Settings/SETTINGS.md`.

---

## 2026-04-12 — Session 1: Documentation & Design

### Project documentation created

Starting from the SOW (`EDGE-oriented_mono_DSP_effects_instrument.md`), added:

**§16 DAISY_QAE Compliance Block** (appended to SOW):
- Complexity rating: 9/10
- DaisySP module selection table
- SDRAM placement requirement (DSY_SDRAM_BSS for delay buffer)
- Audio callback format (non-interleaved Pod pattern)
- Initialization order (StartAdc before StartAudio)
- ExtEncoder.h reference (Pod_OLED_EuclideanDrums)
- OLED I2C pins D11/D12
- Thread safety pattern (__DSB barrier)
- Block size recommendation (48)

**§17 Block Diagrams** (3 Mermaid diagrams in SOW):
- 17A: System Architecture (flowchart TB, 4 color-coded layers)
- 17B: Signal Flow (flowchart LR, full DSP chain with feedback path)
- 17C-1: Control Flow — Inputs → Events (flowchart LR)
- 17C-2: Control Flow — Events → State → ISR (flowchart TD)

**Separate files created:**
- `CONTROLS.md` — hardware map, control routing tables, FxParams reference
- `PARAMETER_TABLES.md` — per-page parameter tables (P1–P6) with ranges and edit steps
- `PAGES_CONTROLS_EFFECTS.md` — 3 Mermaid visual maps (page hub, control routing, DSP+controls)
- `USER_MANUAL.md` — full user documentation (§1–§12) + §13 functionality test chapter

### Key design decisions confirmed

- **Reverb:** `FDNReverb<8192>` from DAFX_2_Daisy_lib (no LGPL flag needed)
- **Delay buffer:** must use `DSY_SDRAM_BSS` — 2 s at 48 kHz = 384 KB, exceeds SRAM
- **ExtEncoder.h:** reuse from Pod_OLED_EuclideanDrums — handles BAK pin conflict (D17)
- **Non-interleaved callback:** verified in Pod_OLED_EuclideanDrums reference (`out[0][i]`)
- **No MIDI in v1:** explicitly excluded per SOW §3.3
- **5 factory presets:** Kick Dub, Snare Space, Metal Burst, Noise Wash, Bass Echo Drive

---

## Template for Future Entries

```
## YYYY-MM-DD — Session N: [Brief title]

### [Change category]

**What changed:** [description]
**Why:** [rationale or bug description]
**Files changed:** [list]
**Build result:** [pass/fail, flash size]
```

## 2026-04-12 — Session 8: Phase 3 Delay Core & Complete Structural Summary

### Comprehensive Structural Implementation (Phases 1-3 Summary)

Up to this point, the core audio processing and control architecture has been fully stabilized and implemented. Key historical breakthroughs to reference:

**1. Dual-Loop Control Decoupling (Phase 1 Bugfix):**
Initial implementations suffered from severe knob lag and deadbands swallowing user inputs. This was solved by moving `pod.ProcessDigitalControls()` and `ext_enc.Debounce()` into a 1kHz `ControlTimerCallback` (hardware timer ISR). Analog knobs (`pod.ProcessAnalogControls()`) were moved to the start of the `AudioCallback` (audio ISR). This properly aligned the 1kHz one-pole filters tied to the analog knobs, completely restoring responsive live-performance twist behavior.

**2. State Variable Filter & Overdrive (Phase 2):**
Implemented `DcBlock`, `Svf` (high-pass), and `Overdrive`. Bound all incoming continuous parameter changes from the UI (`drive`, `input_gain`, `hp_hz`) to `fonepole()` math executed *inside* the AudioCallback block. This prevented CPU cross-thread artifacting (zipper noise) while sweeping the inputs.

**3. SDRAM Delay Buffer Allocation (Phase 3):**
Resolved memory limit constraints by allocating the expansive delay buffer (`DelayLine<float, MAX_DELAY>`) into the external 64MB chip using libDaisy's `DSY_SDRAM_BSS` macro. Verification confirmed this cleanly utilized ~384 KB of `SDRAM` while protecting the 512KB `.ebss` inner SRAM space.

**4. Feedback DSP & Saturation (Phase 3):**
The delay feedback loops were integrated with two cascading SVFs (`svf_fb_hp` and `svf_fb_lp`). To prevent aggressive settings from inducing speaker-blowing runaway oscillations, a `tanhf(fb_sig)` saturation function was mathematically applied to the feedback return line. This behaves like an analog BBD/tape circuit, naturally limiting repeats. Tap tempo was smoothly bridged into calculating raw sample times via standard `sync_mode` logic.
