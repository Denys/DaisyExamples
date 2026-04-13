# Phase 2 Audio Skeleton Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add the narrow Phase 2 audio skeleton to `POD_EDGE_mono_DSP` without touching delay, reverb, limiter, freeze audio, or the control redesign.

**Architecture:** Enable only the first four DSP stages inside `main.cpp`: input gain, `DcBlock`, input `Svf` high-pass, and `Overdrive`. Keep ISR-safe double-buffer parameter reads, preserve current UI/control behavior, and smooth only the continuous parameters used by the active Phase 2 chain.

**Tech Stack:** Daisy Pod, libDaisy, DaisySP, C++14, GNU Make

---

### Task 1: Enable Phase 2 DSP modules

**Files:**
- Modify: `main.cpp`

**Step 1: Define the active Phase 2 module set**

- Enable `DcBlock`, input `Svf`, and `Overdrive`
- Leave delay, feedback filters, reverb, limiter, and wow modules disabled

**Step 2: Initialize the modules in `main()`**

- `dcb.Init(sr)`
- `svf_hp_in.Init(sr)` plus initial freq/res setup
- `overdrive.Init()`

**Step 3: Keep comments aligned with real phase state**

- Update nearby comments so they describe the active chain accurately

### Task 2: Add callback smoothing and processing

**Files:**
- Modify: `main.cpp`

**Step 1: Add lightweight smoothing state**

- Add static smoothed values for `input_gain`, `drive`, and `hp_hz`
- Seed them from defaults at startup

**Step 2: Apply parameter smoothing in the callback**

- Smooth the active parameters using `fonepole()`
- Clamp HP and drive inputs before using them

**Step 3: Implement the active Phase 2 chain**

- Read Pod left input
- Apply gain
- Apply `DcBlock`
- Apply input `Svf` high-pass
- Apply `Overdrive`
- Write dual-mono output

### Task 3: Update project memory/docs to reflect active Phase 2 state

**Files:**
- Modify: `MEMORY.md`
- Modify: `Settings/SETTINGS.md`
- Modify: `Settings/HISTORY.md`

**Step 1: Update phase wording**

- Move the project from pure passthrough wording to Phase 2 audio skeleton wording

**Step 2: Update current signal path description**

- Describe the active DSP stages only

**Step 3: Log the implementation**

- Add a concise history entry with files changed and build result

### Task 4: Verify with a fresh rebuild

**Files:**
- No source changes required

**Step 1: Run a clean rebuild**

- Default / Git Bash: `make clean && make`
- Codex / PowerShell-safe fallback:
  `if(Test-Path build){Remove-Item -LiteralPath build -Recurse -Force}; make`

**Step 2: Inspect the result**

- Confirm exit code is zero
- Record memory usage
- Confirm no unexpected warnings/errors
