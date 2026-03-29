# Field MIDI OXI Diagnostic Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a minimal Daisy Field diagnostic polysynth that receives OXI ONE MIDI, plays simple polyphonic notes, and reports which MIDI features are present and working.

**Architecture:** Create a new custom project under `DaisyExamples/MyProjects/_projects/` derived from the stock `field/Midi` example. Keep the synth intentionally small: one oscillator, one envelope, one filter per voice, plus OLED and serial diagnostics for MIDI event classes, channel filtering, voice allocation, steals, and dropped notes.

**Tech Stack:** C++14, libDaisy, DaisySP, GNU Make, Daisy Field hardware MIDI (`hw.midi`)

---

### Task 1: Create the standalone project shell

**Files:**
- Create: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/Makefile`
- Create: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/midi_oxi.cpp`
- Create: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/README.md`

**Step 1:** Create a new custom project directory so the stock `field/Midi` example remains unchanged.

**Step 2:** Add a Makefile with a nested `_projects` path configuration and `TARGET = midi_oxi`.

**Step 3:** Add a firmware source file derived from the Field MIDI example and local Field conventions.

**Step 4:** Add a short README documenting purpose, controls, and OXI test scope.

### Task 2: Implement the minimal diagnostic synth

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/midi_oxi.cpp`

**Step 1:** Add a tiny poly voice type with oscillator, filter, ADSR, note number, velocity, gate, and age tracking.

**Step 2:** Add a voice manager with:
- free-voice search
- released-voice reuse
- oldest-voice stealing
- counters for steals and dropped notes

**Step 3:** Add a global diagnostics state with counters for:
- `NoteOn`
- `NoteOff`
- `ControlChange`
- `PitchBend`
- `ProgramChange`
- `SystemRealTime`
- `TimingClock`
- `Start`
- `Continue`
- `Stop`

**Step 4:** Add a single-channel filter for the OXI test path and standard velocity-0 note-off handling.

### Task 3: Add observable diagnostics

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/midi_oxi.cpp`

**Step 1:** Add OLED pages that show:
- active voices
- selected MIDI channel
- note and voice counters
- message counters

**Step 2:** Add low-rate serial logging in the main loop for the same counters.

**Step 3:** Add simple control bindings:
- `K1` cutoff
- `K2` resonance
- `K3` master volume
- `SW1` choke voices
- `SW2` cycle OLED page

### Task 4: Build verification

**Files:**
- Modify if required: `DaisyExamples/MyProjects/_projects/Midi_OXI_Field/*`

**Step 1:** Run `make` in the new project directory.

**Step 2:** Fix compile errors until the build succeeds.

**Step 3:** Record what is already supported at Daisy/library level versus what remains for later OXI/DVPE work.
