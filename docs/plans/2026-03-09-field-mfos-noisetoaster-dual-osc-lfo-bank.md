# Field_MFOS_NoiseToaster Dual-Oscillator LFO Bank Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Redesign `Field_MFOS_NoiseToaster` so `MOD0` becomes a grouped dual-oscillator performance bank and `MOD1` becomes the persistent LFO-edit bank.

**Architecture:** Extend the pure helper layer first so bank switching, button-state persistence, knob labels, and pickup decisions can be tested on the host before touching the firmware. Then refactor the Daisy Field patch so the main loop owns bank selection and per-bank state, while the audio callback consumes a stable snapshot that drives a dual-oscillator signal path and persistent LFO settings. Finally, update the docs and HTML reference artifact so they describe the new dual-oscillator `MOD0` and LFO-only `MOD1` model instead of the earlier bank concept.

**Tech Stack:** C++, Daisy Field, libDaisy, DaisySP, host-side `g++` test, PowerShell, Markdown, HTML/CSS/JavaScript

---

### Task 1: Record The Approved Redesign

**Files:**
- Create: `docs/plans/2026-03-09-field-mfos-noisetoaster-dual-osc-lfo-bank-design.md`

**Step 1: Write the design doc**

Capture:

- grouped `OSC1` and `OSC2` controls in `MOD0`
- `K3` as the oscillator-only crossfade mixer
- `MOD1` owning only LFO-related controls
- explicit rejection of a dedicated `Output Level` knob

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: `docs/plans/2026-03-09-field-mfos-noisetoaster-dual-osc-lfo-bank-design.md` is listed

### Task 2: Write The Failing Helper Tests

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Modify later: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`

**Step 1: Add failing bank-label and helper tests**

Cover:

- `MOD0` labels for `OSC1`, `OSC2`, and mixer controls
- `MOD1` labels for `LFO Rate`, `LFO -> VCO`, and `LFO -> VCF`
- bank toggle behavior
- button-state cycling for reserved buttons
- soft-takeover helper behavior

Example additions:

```cpp
static_assert(ToggleBank(ModBank::Mod0) == ModBank::Mod1);
static_assert(ToggleBank(ModBank::Mod1) == ModBank::Mod0);
static_assert(AdvanceButtonState(ThreeStateLed::On) == ThreeStateLed::Blink);
static_assert(KnobLabel(ModBank::Mod0, 0) == nullptr); // temporary failing assertion to replace with real helper API
```

Use a tiny runtime `main()` assertion for float pickup helpers if needed.

**Step 2: Run the test to verify it fails**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: FAIL because the new bank-aware helper API does not exist yet

### Task 3: Implement The Pure Helper Layer

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`

**Step 1: Add bank-aware labels and helpers**

Provide:

- `enum class ModBank { Mod0 = 0, Mod1 };`
- `constexpr ModBank ToggleBank(ModBank bank)`
- `constexpr ThreeStateLed AdvanceButtonState(ThreeStateLed state)`
- `inline const char* KnobLabel(ModBank bank, int index)`
- `inline const char* ButtonLabel(ModBank bank, int index)`

`MOD0` knob labels should reflect:

- `OSC1`
- `OSC2`
- `MIX`
- `AR-VCO`
- `CUT`
- `RES`
- `ATT`
- `REL`

`MOD1` knob labels should reflect:

- `LFO RT`
- `LFO-VCO`
- `LFO-VCF`

**Step 2: Add pickup helper(s)**

Provide a small helper such as:

```cpp
constexpr bool WithinPickupWindow(float physical, float saved, float epsilon)
{
    return (physical >= saved - epsilon) && (physical <= saved + epsilon);
}
```

**Step 3: Run the helper tests to verify they pass**

Run: `g++ -std=c++17 -c MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp -I MyProjects/_projects/Field_MFOS_NoiseToaster -o build/noisetoaster_modes_test.o`
Expected: PASS

### Task 4: Introduce Per-Bank State In The Firmware

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Add the active bank and saved state**

Add storage for:

- current `ModBank`
- per-bank knob values: `float knob_bank_values[2][8]`
- per-bank button states for `B1..B8`
- per-bank knob pickup flags

Keep `SW1` as the bank switch and `SW2` as panic.

**Step 2: Update control scanning**

Implement:

- `SW1` toggles the bank
- the selected bank restores its saved `B1..B8` states
- the selected bank keeps its saved knob targets until pickup occurs

Keep reserved buttons stateful even if they are DSP no-ops.

### Task 5: Build The Dual-Oscillator Voice

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Add a second oscillator**

Introduce `OSC2` with:

- its own waveform state
- its own tuning derived from `K2`
- the same armed-note base as `OSC1`

**Step 2: Define the `OSC2` tuning rule**

Implement `K2` as bipolar around unison:

- center = unison with `OSC1`
- near center = fine detune
- farther from center = coarse interval offset

Document the exact range in code comments if the mapping is not obvious.

**Step 3: Add the oscillator mixer**

Implement `K3` as a crossfade:

- `0.0` -> `OSC1` only
- `0.5` -> `50/50`
- `1.0` -> `OSC2` only

Apply this before the existing filter path.

### Task 6: Move Only LFO Controls To MOD1

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Reassign the live controls**

`MOD0` should own:

- `K1` `OSC1` tune
- `K2` `OSC2` tune / detune
- `K3` mixer
- `K4` `AREG -> VCO`
- `K5` cutoff
- `K6` resonance
- `K7` attack
- `K8` release
- `B1` `OSC1` wave
- `B2` `OSC2` wave
- `B3` `VCF` mod source
- `B4` repeat/manual
- `B5` bypass

`MOD1` should own:

- `K1` `LFO Rate`
- `K2` `LFO -> VCO` depth
- `K3` `LFO -> VCF` depth
- `B1` `LFO` wave
- `B2` `VCO Sync`

**Step 2: Preserve the current non-LFO modulation placement**

Keep on `MOD0`:

- `AREG -> VCO`
- `VCF mod source`
- `AREG` attack/release
- repeat/manual
- bypass

Do not move AREG-related modulation into `MOD1` in this pass.

### Task 7: Make MOD1 LFO Settings Persistent

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Read LFO settings from the active bank state**

Ensure `MOD1` edits:

- live `LFO` rate
- live `LFO -> VCO` depth
- live `LFO -> VCF` depth
- live `LFO` waveform
- live sync state

**Step 2: Keep those settings active after returning to MOD0**

`MOD0` performance should continue using the last edited `MOD1` LFO settings until the user re-enters `MOD1` and changes them.

### Task 8: Update LEDs And OLED

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`

**Step 1: Render active-bank LED meanings**

Make `B1/B2/B3` show the meaning of the currently selected bank.

Examples:

- `MOD0:B1` shows the `OSC1` waveform state
- `MOD1:B1` shows the `LFO` waveform state
- reserved buttons restore `On / Blink / Off` exactly

**Step 2: Update OLED labels**

The OLED should show:

- active bank
- bank-specific knob labels
- oscillator grouping in `MOD0`
- LFO grouping in `MOD1`

Remove any wording that still claims `SW1 = Manual Gate`.

### Task 9: Rebuild Documentation And Reference Artifact

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

**Step 1: Update `CONTROLS.md`**

Document:

- grouped `OSC1` and `OSC2` controls in `MOD0`
- `K3` oscillator mixer behavior
- `MOD1` as the LFO-only bank
- the explicit decision not to dedicate a knob to output level

**Step 2: Update diagrams and README**

Make them reflect:

- dual oscillators
- pre-filter oscillator mixer
- LFO-only `MOD1`
- persistent bank-owned state

**Step 3: Update the HTML artifact and regression check**

Ensure the reference page and test look for representative content such as:

- `OSC1`
- `OSC2`
- `Mixer`
- `MOD1`
- `LFO Rate`
- `LFO -> VCO`
- `LFO -> VCF`
- `VCO Sync`

### Task 10: Verify Firmware, Tests, And Docs

**Files:**
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/noisetoaster_modes.h`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/noisetoaster_modes_test.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

**Step 1: Run the helper test**

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
Expected: only the intended dual-oscillator, LFO-bank, and documentation changes appear
