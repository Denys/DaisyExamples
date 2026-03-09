# Field_MFOS_NoiseToaster Analog Fidelity Pass Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Upgrade `Field_MFOS_NoiseToaster` toward the original analog Noise Toaster control model by adding selector cycling, repeat/manual behavior, manual gate, VCA bypass, live AREG timing, and a parameter-focused OLED UI.

**Architecture:** Extract the new selector cycling rules into a tiny pure C++ helper that can be checked with compile-time `static_assert` tests under the available Daisy toolchain, then update the firmware to use those helpers while simplifying the synth around an AREG-centric trigger model. After the firmware is stable, rewrite the project docs so `README.md`, `CONTROLS.md`, and `DIAGRAMS.md` match the code exactly and include the missing analog reference flow diagrams.

**Tech Stack:** C++, Daisy Field, libDaisy, DaisySP, PowerShell, host-side `g++` test, Markdown, Mermaid

---

### Task 1: Record The Approved Design

**Files:**
- Create: `docs/plans/2026-03-08-field-mfos-noisetoaster-analog-fidelity-pass-design.md`

**Step 1: Write the design doc**

Capture:

- the approved `B1/B2/B3` selector semantics
- the OLED idle/zoom rules
- the Daisy Field knob-budget compromise
- the AREG-centric repeat/manual interaction model

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: the new design file is listed

### Task 2: Write The Failing Selector Test

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Create later: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`

**Step 1: Write the failing test**

Write a simple compile-time test file with `static_assert` checks for:

- VCO waveform cycle order `Saw -> Square -> Triangle -> Saw`
- LFO waveform cycle order `Sine -> Square -> Triangle -> Sine`
- VCF mod source cycle order `LFO -> AREG -> Off -> LFO`
- LED-state mapping where the third state is LED off

**Step 2: Run the test to verify it fails**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: FAIL because the helper header and functions do not exist yet

### Task 3: Implement The Pure Selector Helper

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`
- Modify later: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Add the helper API**

Provide:

- enums for VCO wave mode, LFO wave mode, and VCF mod source
- functions to advance each selector by one step
- short label helpers for OLED text
- a small LED-state helper that returns full, blinking, or off brightness

**Step 2: Run the test to verify it passes**

Run:

- `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`

Expected: compile succeeds with all `static_assert` checks satisfied

### Task 4: Refactor The Firmware Around The New Control Model

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Remove superseded state**

Delete or replace:

- hold-related state
- separate ADSR VCA state
- direct `B1/B2/B3` waveform assignment logic
- old OLED hint-only layout

**Step 2: Add the new control state**

Add:

- selector states for VCO wave, LFO wave, and VCF mod source
- repeat/manual state
- VCA bypass state
- armed-note state
- knob movement tracking for OLED zoom timeout

**Step 3: Rebuild the AREG-driven synth behavior**

Implement:

- `A1..A8` note select + retrigger
- `SW1` manual gate retrigger
- `B4` repeat/manual with automatic AREG retrigger when idle
- `B5` VCA bypass
- `K7/K8` attack/release time mapping
- pitch modulation from LFO and AREG
- filter modulation from selected source only
- fixed internal LFO rate and fixed internal output level for this pass

**Step 4: Add LED feedback**

Implement:

- `B1/B2/B3` LED on/blink/off behavior
- `B4/B5` binary LED feedback
- note LED for the currently armed `A` key
- knob LEDs following current knob values

**Step 5: Replace the OLED UI**

Implement:

- idle overview showing all eight knob functions
- current mode summary for the selector states
- temporary focused parameter view while a knob is being edited

### Task 5: Rebuild The Documentation

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`

**Step 1: Update README.md**

Document:

- the new control architecture
- the knob-budget compromise
- fixed internal LFO rate and output level
- remaining analog gaps after this pass

**Step 2: Update CONTROLS.md**

Replace the old hold/waveform tables with:

- new knob table
- new B-row table
- manual gate / repeat behavior
- OLED idle/focus behavior
- fixed internal settings

**Step 3: Update DIAGRAMS.md**

Add:

- analog reference audio signal flow
- analog reference control/event flow

Update:

- current firmware block diagram
- current firmware audio signal flow
- current firmware control/event flow
- post-improvement notes so they match the new code

### Task 6: Verify Firmware And Docs

**Files:**
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`

**Step 1: Run the selector helper test**

Run:

- `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`

Expected: PASS

**Step 2: Build the firmware**

Run: `make -C MyProjects/_projects/Field_MFOS_NoiseToaster -j4`
Expected: compile succeeds and memory usage is reported

**Step 3: Check patch integrity**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: no output

**Step 4: Review the diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: only the intended firmware, helper, test, and Markdown files changed
