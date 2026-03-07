# Field_MFOS_NoiseToaster Documentation Expansion Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Expand `Field_MFOS_NoiseToaster` with accurate project documentation and add a reusable prompt that requests code plus companion docs together.

**Architecture:** This is a documentation-only change. `README.md` becomes the project entry point, `CONTROLS.md` captures detailed hardware/runtime behavior, `DIAGRAMS.md` captures architecture visually, and `GENERATION_PROMPT.md` captures the reusable AI-generation specification. All content is grounded in the existing C++ implementation and Makefile.

**Tech Stack:** Markdown, Mermaid, git, Daisy Field C++ source as reference

---

### Task 1: Record The Approved Design

**Files:**
- Create: `docs/plans/2026-03-07-field-mfos-noisetoaster-docs-design.md`

**Step 1: Write the design doc**

Include:

- Scope
- Deliverables
- Source-of-truth files
- Key implementation behaviors that the docs must preserve
- Verification approach

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: design file is listed

### Task 2: Expand The README

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/Makefile`

**Step 1: Replace the short README with a fuller operator-facing version**

Cover:

- Overview
- Architecture summary
- Quick controls
- Defaults and startup behavior
- Build instructions
- Links to detailed docs
- Known limitations

**Step 2: Re-read the C++ file and confirm every feature claim matches the implementation**

Run: `rg -n "KeyboardRisingEdge|sw\\[|knob\\[|SetWaveform|StartLog|WriteString|Process" MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
Expected: all control claims in the README are traceable to code

### Task 3: Add Detailed Controls Documentation

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Add hardware mapping tables**

Document:

- K1-K8
- A1-A8 note map
- B1-B5 behavior
- SW1/SW2 behavior

**Step 2: Add runtime behavior notes**

Document:

- Monophonic last-trigger behavior
- Hold/drone behavior
- Panic behavior
- AR destination toggle behavior
- OLED and serial boot behavior

### Task 4: Add Diagrams

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Add a block diagram**

Show:

- Daisy Field controls
- DSP blocks
- OLED and stereo output

**Step 2: Add signal/control flow diagrams**

Show:

- Audio path from oscillator/noise through filter and VCA
- Control/event path from keyboard, switches, and knobs into synth state

### Task 5: Add The Reusable Generation Prompt

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`

**Step 1: Write a copy-pastable prompt**

Require the generated deliverable set to include:

- C++ source
- Makefile
- README
- CONTROLS
- DIAGRAMS

**Step 2: Make the prompt explicit about consistency**

Require:

- Docs must match code exactly
- No placeholder sections
- Real Daisy Field control mapping
- Real-time-safe audio callback expectations

### Task 6: Verify Documentation Changes

**Files:**
- Verify: `docs/plans/2026-03-07-field-mfos-noisetoaster-docs-design.md`
- Verify: `docs/plans/2026-03-07-field-mfos-noisetoaster-docs.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`

**Step 1: Check for whitespace and patch issues**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: no output

**Step 2: Review the diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: only documentation files are added or modified

**Step 3: Do not claim build validation**

Note:

- This task does not change firmware behavior.
- Local firmware build remains environment-dependent on `libDaisy/core/Makefile`.
