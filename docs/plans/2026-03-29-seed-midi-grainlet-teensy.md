# Seed MIDI Grainlet Teensy Controller Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a Teensy 4.0 sketch that sends raw UART MIDI to drive the Daisy Seed MIDI Grainlet proof of concept.

**Architecture:** Use a single Arduino sketch with `Serial1` at `31250` baud, three polled analog inputs, and one debounced pushbutton. Avoid external MIDI, debounce, or encoder libraries to keep setup friction minimal.

**Tech Stack:** Arduino sketch, Teensy 4.0, raw UART MIDI, arduino-cli

---

### Task 1: Create the sketch skeleton

**Files:**
- Create: `MyProjects/_projects/Seed_MIDI_Grainlet/teensy_controller/Seed_MIDI_Grainlet_Teensy40/Seed_MIDI_Grainlet_Teensy40.ino`
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/README.md`

**Step 1: Write the failing test**

Attempt to compile the sketch path before it exists.

**Step 2: Run test to verify it fails**

Run: `arduino-cli compile --fqbn teensy:avr:teensy40 <sketch path>`
Expected: failure because the sketch file is missing

**Step 3: Write minimal implementation**

Create the sketch folder and minimal `.ino` file.

**Step 4: Run test to verify it passes**

Run: `arduino-cli compile --fqbn teensy:avr:teensy40 <sketch path>`
Expected: compile proceeds with a valid sketch structure

### Task 2: Add raw MIDI output for pots and button

**Files:**
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/teensy_controller/Seed_MIDI_Grainlet_Teensy40/Seed_MIDI_Grainlet_Teensy40.ino`

**Step 1: Write the failing test**

Compile with references to helper functions and state not yet implemented.

**Step 2: Run test to verify it fails**

Run: `arduino-cli compile --fqbn teensy:avr:teensy40 <sketch path>`
Expected: compile errors for missing MIDI send or debounce helpers

**Step 3: Write minimal implementation**

Add:

- raw MIDI send helpers
- analog polling and MIDI CC transmission
- pushbutton debounce and fixed note on/off

**Step 4: Run test to verify it passes**

Run: `arduino-cli compile --fqbn teensy:avr:teensy40 <sketch path>`
Expected: clean Teensy compile

### Task 3: Document the controller mapping

**Files:**
- Modify: `MyProjects/_projects/Seed_MIDI_Grainlet/README.md`

**Step 1: Write the failing test**

Treat missing Teensy-side wiring and controller mapping as incomplete documentation.

**Step 2: Run test to verify it fails**

Inspect the README
Expected: missing Teensy-specific setup details

**Step 3: Write minimal implementation**

Add Teensy wiring, note mapping, and upload target notes.

**Step 4: Run test to verify it passes**

Re-read the README and re-run compile
Expected: documentation matches the compiled sketch
