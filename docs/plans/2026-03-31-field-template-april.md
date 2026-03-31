# Field Template April Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build `Field_Template_April` as a clean Daisy Field starter synth that demonstrates banked controls, stored-value LEDs, keybed state controls, and full display/docs conventions.

**Architecture:** Start from the simple Field template pattern, then layer in the shared parameter-bank helpers and display helpers already added under `MyProjects/foundation_examples`. Keep the voice intentionally simple so the project teaches the control framework rather than DSP complexity.

**Tech Stack:** C++, libDaisy, DaisySP, Daisy Field hardware helpers, shared Field foundation headers, Markdown README documentation

---

### Task 1: Scaffold the project

**Files:**
- Create: `MyProjects/_projects/Field_Template_April/Field_Template_April.cpp`
- Create: `MyProjects/_projects/Field_Template_April/Makefile`

**Step 1: Copy the standard Daisy Field project shape**

- Base the new project on `MyProjects/_projects/Field_Template_Std`
- Keep `TARGET` aligned with `Field_Template_April.cpp`

**Step 2: Build the minimal synth shell**

- Add a simple mono synth voice
- Initialize Daisy Field audio, ADC, MIDI, OLED, and LEDs
- Keep DSP simple and readable

**Step 3: Commit**

```bash
git add MyProjects/_projects/Field_Template_April/Field_Template_April.cpp MyProjects/_projects/Field_Template_April/Makefile
git commit -m "feat: scaffold Field Template April"
```

### Task 2: Add shared control behavior

**Files:**
- Modify: `MyProjects/_projects/Field_Template_April/Field_Template_April.cpp`

**Step 1: Add parameter banks**

- Use `field_parameter_banks.h`
- Define main and alt parameter defaults
- Preserve values across bank changes

**Step 2: Add pickup/catch**

- Require capture after switching banks
- Prevent raw knob position from overwriting stored values on entry

**Step 3: Add stored-value knob LEDs**

- Drive knob LEDs from the active bank’s stored values
- Keep LED state independent from physical pot position

**Step 4: Reserve level for `SW2 + K8`**

- Keep main `K8` musically useful
- Put internal level on secondary access only

**Step 5: Commit**

```bash
git add MyProjects/_projects/Field_Template_April/Field_Template_April.cpp
git commit -m "feat: add banked controls to Field Template April"
```

### Task 3: Add demo keybed state controls and OLED behavior

**Files:**
- Modify: `MyProjects/_projects/Field_Template_April/Field_Template_April.cpp`

**Step 1: Wire keybed as control surface**

- Use `A1-A8` / `B1-B8` as demo control/state buttons
- Demonstrate `Off / Blink / On`

**Step 2: Add OLED overview and zoom**

- Use the shared instrument UI helpers
- Show compact overview by default
- Show parameter zoom on edits

**Step 3: Add panic/reset**

- Define and document a simple safe reset path

**Step 4: Commit**

```bash
git add MyProjects/_projects/Field_Template_April/Field_Template_April.cpp
git commit -m "feat: add Field Template April UI behaviors"
```

### Task 4: Document the project

**Files:**
- Create: `MyProjects/_projects/Field_Template_April/README.md`

**Step 1: Write full README**

- Document controls, LEDs, OLED pages, hidden banks, startup/default values, MIDI behavior, and maintenance notes
- Include exact default values for every main and alt parameter

**Step 2: Commit**

```bash
git add MyProjects/_projects/Field_Template_April/README.md
git commit -m "docs: add Field Template April README"
```

### Task 5: Verify build

**Files:**
- Test: `MyProjects/_projects/Field_Template_April/Makefile`

**Step 1: Run build**

Run:

```bash
cd MyProjects/_projects/Field_Template_April
make
```

Expected:

- Successful firmware build
- Reported flash and RAM usage

**Step 2: Fix issues if needed**

- Resolve compile errors
- Re-run `make`

**Step 3: Commit final verification state**

```bash
git add MyProjects/_projects/Field_Template_April
git commit -m "feat: finalize Field Template April"
```
