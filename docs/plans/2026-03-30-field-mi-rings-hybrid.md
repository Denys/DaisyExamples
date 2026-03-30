# Field MI Rings Hybrid Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a reusable Daisy Field banked-parameter and pickup/catch control system, retrofit `Field_MI_Plaits` to use it, and create a new `Field_MI_Rings` hybrid standalone instrument with 2-voice polyphony and OXI One MIDI support.

**Architecture:** First create a shared Field helper layer that owns parameter-bank storage, pickup/catch behavior, knob LED rendering from stored values, and 3-state key LED support. Then use `Field_MI_Plaits` as the proving ground for the new control UX before creating `Field_MI_Rings` as a new sibling project that reuses the same app shell, control layer, OLED conventions, and documentation standard.

**Tech Stack:** C++, libDaisy, DaisySP, Daisy Field, stmlib, external TRS MIDI (`OXI One`)

---

### Task 1: Add the shared Field parameter-bank helper layer

**Files:**
- Create: `MyProjects/foundation_examples/field_parameter_banks.h`
- Modify: `MyProjects/foundation_examples/field_instrument_ui.h`
- Modify: `MyProjects/foundation_examples/FIELD_DEFAULTS_USAGE.md`

**Step 1: Write the failing test**

Update `MyProjects/_projects/Field_MI_Plaits/Field_MI_Plaits.cpp` locally to include `field_parameter_banks.h` and reference placeholder helper types such as `FieldParamBankSet` and `TriStateKeyLedBank` before those helpers exist.

**Step 2: Run test to verify it fails**

Run: `make` from `MyProjects/_projects/Field_MI_Plaits`
Expected: compile failure because the shared helper header and types do not exist yet

**Step 3: Write minimal implementation**

Create the new shared helper header with:

- per-knob stored values for `main` and `alt` banks
- per-bank captured state
- pickup/catch threshold logic
- helpers to read the active bank value without mutating it

Extend `field_instrument_ui.h` with a reusable 3-state key LED helper that supports:

- `Off`
- `Blink`
- `On`

Update `FIELD_DEFAULTS_USAGE.md` so the new helper layer is discoverable for future Field projects.

**Step 4: Run test to verify it passes**

Run: `make` from `MyProjects/_projects/Field_MI_Plaits`
Expected: compile succeeds again with the new shared helper layer available

**Step 5: Commit**

```bash
git add MyProjects/foundation_examples/field_parameter_banks.h MyProjects/foundation_examples/field_instrument_ui.h MyProjects/foundation_examples/FIELD_DEFAULTS_USAGE.md
git commit -m "feat: add shared Field parameter bank helpers"
```

### Task 2: Retrofit Field_MI_Plaits to banked parameters and pickup/catch

**Files:**
- Modify: `MyProjects/_projects/Field_MI_Plaits/Field_MI_Plaits.cpp`
- Create: `MyProjects/_projects/Field_MI_Plaits/README.md`
- Modify: `MyProjects/_projects/Field_MI_Plaits/CONTROLS.md`
- Modify: `MyProjects/_projects/Field_MI_Plaits/Dependencies.md`

**Step 1: Write the failing test**

Capture the current bug as the failure condition using the exact manual reproduction:

1. Set default `K1` to `50%`
2. Hold `SW1`
3. Change alt `K1` to `35%`
4. Release `SW1`

Expected for the bug reproduction today: default `K1` jumps away from `50%`

Also record the current `K8` behavior as a second failure condition:

- `K8` is assigned to generic `Level` even though Field already has master volume

**Step 2: Run test to verify it fails**

Run: `make`
Expected: build succeeds, but manual verification on device shows the K1 bank-switch jump and the current K8 `Level` assignment

**Step 3: Write minimal implementation**

Refactor `Field_MI_Plaits.cpp` to:

- store main and alt values independently
- require pickup/catch before editing after a bank switch
- render knob LEDs from stored logical values rather than raw pot position
- keep or remove the synth-level `Level` control only if it is musically required by Plaits behavior
- if `Level` is not musically required, reassign `K8` to a more useful parameter or macro

Create `README.md` with:

- control table
- alt-bank behavior
- LED behavior
- OLED page descriptions
- startup/default values
- panic/reset behavior

Use `CONTROLS.md` and `Dependencies.md` as source material and update them so they stay aligned with the new behavior.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: clean build

Manual verify on hardware:

- default and alt knob banks retain independent values
- releasing `SW1` does not overwrite the default bank
- alt bank is not overwritten when `SW1` is pressed
- knob LEDs track stored values
- `K8` is no longer wasted on generic volume unless intentionally kept for true Plaits voice behavior

**Step 5: Commit**

```bash
git add MyProjects/_projects/Field_MI_Plaits/Field_MI_Plaits.cpp MyProjects/_projects/Field_MI_Plaits/README.md MyProjects/_projects/Field_MI_Plaits/CONTROLS.md MyProjects/_projects/Field_MI_Plaits/Dependencies.md
git commit -m "feat: add banked controls to Field MI Plaits"
```

### Task 3: Add the shared README and OLED documentation standard

**Files:**
- Create: `MyProjects/foundation_examples/FIELD_DISPLAY_PROJECT_README_TEMPLATE.md`
- Modify: `MyProjects/foundation_examples/FIELD_DEFAULTS_README.md`
- Modify: `MyProjects/foundation_examples/FIELD_DEFAULTS_USAGE.md`

**Step 1: Write the failing test**

Treat the missing system-level documentation standard as the failure condition:

- no shared README template for display projects
- no explicit requirement for startup/default values
- no explicit requirement for OLED page replication

**Step 2: Run test to verify it fails**

Inspect the three files above
Expected: no dedicated display-project README template exists yet

**Step 3: Write minimal implementation**

Add a shared template/checklist that requires every display-based Field project to document:

- controls
- LED states
- OLED pages
- hidden banks/pages
- startup/default values
- panic/reset behavior

Update the shared Field docs to point to the template.

**Step 4: Run test to verify it passes**

Inspect the updated docs
Expected: the template exists and the shared docs reference it clearly

**Step 5: Commit**

```bash
git add MyProjects/foundation_examples/FIELD_DISPLAY_PROJECT_README_TEMPLATE.md MyProjects/foundation_examples/FIELD_DEFAULTS_README.md MyProjects/foundation_examples/FIELD_DEFAULTS_USAGE.md
git commit -m "docs: add Field display project README standard"
```

### Task 4: Create the Field_MI_Rings project skeleton

**Files:**
- Create: `MyProjects/_projects/Field_MI_Rings/Makefile`
- Create: `MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp`
- Create: `MyProjects/_projects/Field_MI_Rings/README.md`

**Step 1: Write the failing test**

Treat the missing project as the failure condition.

**Step 2: Run test to verify it fails**

Run: `make` from `MyProjects/_projects/Field_MI_Rings`
Expected: failure because the directory and project files do not exist yet

**Step 3: Write minimal implementation**

Create a standard Daisy Field project skeleton that:

- sets `TARGET = Field_MI_Rings`
- includes `../../../libDaisy` and `../../../DaisySP`
- includes the shared foundation headers
- initializes Daisy Field, MIDI, OLED, and LED helpers
- builds with a trivial placeholder audio callback

Seed `README.md` with placeholder sections matching the shared template.

**Step 4: Run test to verify it passes**

Run: `make` from `MyProjects/_projects/Field_MI_Rings`
Expected: successful skeleton build

**Step 5: Commit**

```bash
git add MyProjects/_projects/Field_MI_Rings/Makefile MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp MyProjects/_projects/Field_MI_Rings/README.md
git commit -m "feat: create Field MI Rings skeleton"
```

### Task 5: Add Rings v1 DSP, voice allocation, and MIDI mapping

**Files:**
- Modify: `MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp`
- Modify: `MyProjects/_projects/Field_MI_Rings/README.md`

**Step 1: Write the failing test**

Reference the intended v1 behavior in code before it exists:

- 2-voice allocator
- model family selection
- OXI CC handling for `CC74`, `CC71`, `CC16`, `CC17`
- internal exciter driven by note velocity

**Step 2: Run test to verify it fails**

Run: `make`
Expected: compile failures for missing voice state, missing model state, or missing MIDI handlers

**Step 3: Write minimal implementation**

Implement the smallest useful v1:

- 2-voice allocator
- note on/off and sustain handling
- velocity-driven excitation
- three Rings-style model families built from local DaisySP physical-model blocks
- main knob map (`Structure`, `Brightness`, `Damping`, `Position`, spread, exciter, macro, interaction)
- OXI CC mapping
- stereo voice placement

Keep the audio callback real-time safe and keep all control scanning and OLED updates in the main loop.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: successful build

Manual verify on hardware:

- 2-note polyphony works
- velocity changes excitation
- the four main OXI CCs move the intended parameters
- free keybed selects model/page states without note-entry conflicts

**Step 5: Commit**

```bash
git add MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp MyProjects/_projects/Field_MI_Rings/README.md
git commit -m "feat: add Rings hybrid voice and MIDI control"
```

### Task 6: Finish the Rings OLED pages and documentation

**Files:**
- Modify: `MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp`
- Modify: `MyProjects/_projects/Field_MI_Rings/README.md`

**Step 1: Write the failing test**

Treat missing UI clarity as the failure condition:

- OLED pages not fully described
- startup/default values not documented
- key LED `Off/Blink/On` meanings not documented

**Step 2: Run test to verify it fails**

Inspect `README.md` and run the build
Expected: README is incomplete relative to the display-project standard

**Step 3: Write minimal implementation**

Complete:

- OLED overview page
- OLED zoom page
- OLED alt/mode page descriptions
- startup/default table
- key LED state meaning table
- panic/reset section
- concise wiring and controller notes for OXI One

Update the code if any missing labels or state indicators are needed to match the docs.

**Step 4: Run test to verify it passes**

Run: `make`
Expected: clean build

Manual verify on hardware:

- OLED pages match the README
- startup/default values match the shipped state
- key LED meanings are visible and documented

**Step 5: Commit**

```bash
git add MyProjects/_projects/Field_MI_Rings/Field_MI_Rings.cpp MyProjects/_projects/Field_MI_Rings/README.md
git commit -m "docs: finalize Rings OLED and control documentation"
```
