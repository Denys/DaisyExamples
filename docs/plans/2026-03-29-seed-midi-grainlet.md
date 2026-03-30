# Seed MIDI Grainlet Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a new Daisy Seed project that plays a monophonic Grainlet oscillator and responds to UART MIDI note and CC messages from an external Teensy controller.

**Architecture:** Reuse the stock Seed Grainlet example for the audio path and the libDaisy UART MIDI example for message handling. Keep the audio callback real-time safe by applying only precomputed control state there while MIDI parsing and parameter updates run in the main loop.

**Tech Stack:** C++, libDaisy, DaisySP, Daisy Seed, UART MIDI

---

### Task 1: Create the project skeleton

**Files:**
- Create: `MyProjects/_projects/Seed_MIDI_Grainlet/Makefile`
- Create: `MyProjects/_projects/Seed_MIDI_Grainlet/Seed_MIDI_Grainlet.cpp`
- Create: `MyProjects/_projects/Seed_MIDI_Grainlet/README.md`

**Step 1: Write the failing test**

Treat the initial missing project as the failure condition. Attempt to build the target directory before files exist.

**Step 2: Run test to verify it fails**

Run: `make` from `MyProjects/_projects/Seed_MIDI_Grainlet`
Expected: failure because the directory and source files do not exist yet

**Step 3: Write minimal implementation**

Create the project directory, add a standard Makefile using `../../../../libDaisy` and `../../../../DaisySP`, and add a minimal source file and README.

**Step 4: Run test to verify it passes**

Run: `make` from `MyProjects/_projects/Seed_MIDI_Grainlet`
Expected: compile proceeds far enough to validate the skeleton

### Task 2: Add monophonic Grainlet audio

**Files:**
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/Seed_MIDI_Grainlet.cpp`

**Step 1: Write the failing test**

Build with references to audio state and DSP objects that are not yet implemented.

**Step 2: Run test to verify it fails**

Run: `make`
Expected: compile errors for missing oscillator, envelope, or callback logic

**Step 3: Write minimal implementation**

Add one `GrainletOscillator`, a simple envelope, and a stereo audio callback.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: compile succeeds with the monophonic audio path present

### Task 3: Add UART MIDI note and CC handling

**Files:**
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/Seed_MIDI_Grainlet.cpp`
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/README.md`

**Step 1: Write the failing test**

Reference UART MIDI setup and message handlers before implementing them.

**Step 2: Run test to verify it fails**

Run: `make`
Expected: compile errors for missing MIDI config or handlers

**Step 3: Write minimal implementation**

Initialize `MidiUartHandler`, call `StartReceive()`, parse `NoteOn`, `NoteOff`, and the selected CC messages, and update shared synth state.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: compile succeeds with MIDI control integrated

### Task 4: Document controls and verify build

**Files:**
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/README.md`

**Step 1: Write the failing test**

Treat missing documentation and missing final build verification as failure conditions.

**Step 2: Run test to verify it fails**

Inspect README and run `make`
Expected: missing control mapping or unverified build state

**Step 3: Write minimal implementation**

Document wiring, MIDI mappings, and expected behavior.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: clean build with updated README
