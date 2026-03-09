# Field_MFOS_NoiseToaster MOD Banks Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a saved `MOD0/MOD1` bank system to `Field_MFOS_NoiseToaster` where `SW1` switches banks, `K1..K8` and `B1..B8` persist independently per bank, and `MOD1` exposes only the genuinely missing analog controls.

**Architecture:** Extend the pure helper layer in `noisetoaster_modes.h` so bank switching, button-state cycling, label lookup, and knob pickup decisions can be tested on the host first. Then refactor `Field_MFOS_NoiseToaster.cpp` so the main loop owns bank selection and per-bank state mutation, while the audio callback consumes a snapshot of the active bank without unsafe live object mutation. Finally, update the project docs and HTML reference artifact so they describe the new banked control model instead of the old `SW1` manual-gate behavior.

**Tech Stack:** C++, Daisy Field, libDaisy, DaisySP, host-side `g++` test, PowerShell, Markdown, HTML/CSS/JavaScript

---

### Task 1: Record The Approved Design

**Files:**
- Create: `docs/plans/2026-03-09-field-mfos-noisetoaster-mod-banks-design.md`

**Step 1: Write the design doc**

Capture:

- `SW1` as `MOD0/MOD1` switch
- sparse `MOD1` mapping (`K1 LFO Rate`, `K2 Output Level`, `B1 White Noise`, `B2 VCO Sync`, `B3 VCF Input Select`)
- per-bank persistence for both knobs and `B1..B8`
- exact requirement that states such as `MOD0:B6 = Blink` and `MOD1:B6 = On` restore correctly

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: `docs/plans/2026-03-09-field-mfos-noisetoaster-mod-banks-design.md` is listed

### Task 2: Write The Failing Bank Helper Test

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Modify later: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`

**Step 1: Write the failing test**

Add host-side coverage for:

- `ToggleBank(ModBank::Mod0) == ModBank::Mod1`
- `ToggleBank(ModBank::Mod1) == ModBank::Mod0`
- three-state button cycling for reserved bank buttons
- `KnobLabel(ModBank::Mod0, 0) == "FREQ"`
- `KnobLabel(ModBank::Mod1, 0) == "LFO RT"`
- `KnobLabel(ModBank::Mod1, 1) == "OUT"`
- pickup helper behavior so a bank-switched knob does not attach until the physical value crosses the saved target

Example test additions:

```cpp
static_assert(ToggleBank(ModBank::Mod0) == ModBank::Mod1);
static_assert(ToggleBank(ModBank::Mod1) == ModBank::Mod0);
static_assert(AdvanceButtonState(ThreeStateLed::On) == ThreeStateLed::Blink);
static_assert(AdvanceButtonState(ThreeStateLed::Blink) == ThreeStateLed::Off);
static_assert(AdvanceButtonState(ThreeStateLed::Off) == ThreeStateLed::On);
```

For pickup behavior, use a tiny runtime check in `main()` if a `static_assert` becomes awkward with floats.

**Step 2: Run the test to verify it fails**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: FAIL because the new bank helpers and labels do not exist yet

### Task 3: Implement The Pure MOD Bank Helper Layer

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`

**Step 1: Add bank and state helpers**

Add:

- `enum class ModBank { Mod0 = 0, Mod1 };`
- `constexpr ModBank ToggleBank(ModBank bank)`
- `constexpr ThreeStateLed AdvanceButtonState(ThreeStateLed state)`
- `inline const char* KnobLabel(ModBank bank, int index)`
- optional bank-specific short labels for `B1..B8` if that simplifies OLED rendering

**Step 2: Add pickup helper(s)**

Add a tiny pure helper such as:

```cpp
constexpr bool WithinPickupWindow(float physical, float saved, float epsilon)
{
    return (physical >= saved - epsilon) && (physical <= saved + epsilon);
}
```

or an equivalent helper that supports a soft-takeover decision in the firmware loop.

**Step 3: Run the helper test to verify it passes**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: PASS

### Task 4: Refactor Firmware State Around Saved Banks

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Introduce per-bank storage**

Add structs or arrays for:

- active bank (`ModBank`)
- saved knob values per bank: `float knob_bank_values[2][8]`
- pickup/capture flags per bank or per active bank transition
- saved `B1..B8` states per bank using a logical discrete state, not LED brightness

Keep `MOD0` initialized from the current defaults and seed `MOD1` with conservative analog-recovery defaults.

**Step 2: Replace the old `SW1` behavior**

Change `SW1` handling from:

- `QueueAregTrigger()`

to:

- toggle the active bank
- reset knob pickup status for the newly selected bank
- preserve both banks' saved `B1..B8` states

`SW2` stays panic.

**Step 3: Route `B1..B8` through the active bank**

Implement per-bank button behavior:

- `MOD0:B1` cycles VCO wave
- `MOD0:B2` cycles LFO wave
- `MOD0:B3` cycles VCF mod source
- `MOD0:B4` toggles Repeat / Manual
- `MOD0:B5` toggles VCA Bypass
- `MOD0:B6..B8` cycle saved discrete states only

- `MOD1:B1` toggles White Noise control
- `MOD1:B2` toggles VCO Sync
- `MOD1:B3` cycles VCF Input Select
- `MOD1:B4..B8` cycle saved discrete states only

Make sure a reserved button can still be `On`, `Blink`, or `Off` so the user's saved-state example is representable.

**Step 4: Route `K1..K8` through the active bank**

Implement:

- `MOD0:K1..K8` current mapping
- `MOD1:K1 = LFO Rate`
- `MOD1:K2 = Output Level`
- `MOD1:K3..K8` reserved and stateful, but no-op in DSP for now

Use soft takeover so the selected bank starts from its saved value until the physical knob crosses the pickup window.

**Step 5: Keep callback-owned DSP mutation**

Update the audio callback snapshot so it reads the active bank's saved values and used button states, then applies:

- current `MOD0` synth behavior when `MOD0` is active
- `MOD1` analog-recovery controls for live `LFO Rate`, live `Output Level`, White Noise state, VCO Sync state, and VCF Input Select when `MOD1` is active

Do not move expensive or unsafe work out of the callback-owned mutation path that was just stabilized.

### Task 5: Update LEDs And OLED For Banked Controls

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Render B-row LEDs from saved bank state**

Make the active bank's `B1..B8` LEDs reflect its saved discrete state exactly.

Examples:

- `Blink` must still blink after returning to that bank
- `On` must come back solid after switching away and back
- reserved buttons must still show their stored LED state

**Step 2: Make the OLED bank-aware**

Update the overview and focused parameter UI so it shows:

- active bank (`MOD0` or `MOD1`)
- active knob labels for the current bank
- current bank-specific selector summaries where relevant

Remove `SW1` manual-gate wording from the screen text.

### Task 6: Rebuild Docs And Reference Artifact

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

**Step 1: Update the Markdown docs**

Document:

- `SW1` now switches `MOD0/MOD1`
- `MOD0` mapping
- sparse `MOD1` mapping
- per-bank saved `B1..B8` states
- loss of dedicated manual-gate behavior in this revision

**Step 2: Update the HTML reference artifact**

Rewrite the relevant sections so the artifact shows:

- `SW1 -> MOD0/MOD1 switch`
- `MOD0` and `MOD1` control tables
- bank-aware `B1..B8` meanings
- reserved but stateful `B6..B8` behavior

Keep the artifact explicit that this is the next control model, not the previous firmware behavior.

**Step 3: Refresh the HTML regression check**

Add or replace representative string checks so the test verifies content such as:

- `SW1 -> MOD0/MOD1`
- `K1 -> LFO Rate`
- `K2 -> Output Level`
- `B1 -> White Noise`
- `B2 -> VCO Sync`
- `B3 -> VCF Input Select`

### Task 7: Verify Firmware, Docs, And Reference Artifact

**Files:**
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

**Step 1: Run the bank helper test**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: PASS

**Step 2: Build the firmware**

Run: `make -C MyProjects/_projects/Field_MFOS_NoiseToaster -j4`
Expected: compile succeeds and memory usage is reported

**Step 3: Run the HTML regression check**

Run: `powershell -ExecutionPolicy Bypass -File MyProjects\_projects\Field_MFOS_NoiseToaster\tests\check_reference_html.ps1`
Expected: PASS

**Step 4: Check patch integrity**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: no output

**Step 5: Review the diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: only the intended bank-system firmware, docs, and reference-artifact changes appear
