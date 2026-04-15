# Pod MultiFX Tube Delay Reverb Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refactor `Pod_MultiFX_Chain` into a three-page Tube/Delay/Reverb pedal with Tube shift controls, short-delay ranges, simplified reverb, and persistent page state via soft takeover.

**Architecture:** Keep control state in a Daisy-independent helper header and drive the firmware entirely from stored state. The helper will own three pages plus a Tube shift-layer sub-state, while the firmware maps those stored values into DAFX `Tube`, short `Delay`, and simplified `ReverbSc` parameters.

**Tech Stack:** C++, libDaisy, DaisySP, DAFX `Tube`, Daisy Pod hardware, host-side MSVC test executable, Markdown documentation

---

### Task 1: Rewrite the host-side state test

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Write the failing test**

- Replace the four-page assumptions with a three-page `Tube -> Delay -> Reverb`
  model
- Add assertions for:
  - three-page wrap
  - Tube default-layer state persistence
  - Tube shifted-layer state persistence
  - `SW1` short press toggles Tube bypass
  - `SW1` hold enables Tube shift without toggling bypass on release

**Step 2: Run test to verify it fails**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- compile failure or assertion failure because the helper still implements the
  old four-page model

**Step 3: Write minimal helper changes**

- Reduce pages to Tube, Delay, Reverb
- Add Tube shift-layer state
- Add `SW1` hold/short-press state helpers
- Keep soft takeover independent per logical layer

**Step 4: Run test to verify it passes**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- clean compile
- executable exits `0`

### Task 2: Refactor firmware DSP and controls

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Makefile`
- Use: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Replace Overdrive and remove WahWah**

- Remove `Overdrive`
- Remove `WahWah`
- Add DAFX `Tube`
- Update `Makefile` sources/includes so `tube.cpp` is compiled instead of
  `wahwah.cpp`

**Step 2: Implement Tube page mappings**

- Default layer:
  - `Knob 1 = Drive` with exponential mapping
  - `Knob 2 = Mix` with linear mapping
- Shift layer while holding `SW1` on Tube:
  - `Knob 1 = Bias` with center-biased exponential mapping
  - `Knob 2 = Distortion` with exponential mapping

**Step 3: Simplify Delay and Reverb**

- Delay:
  - map `Knob 1` to `10 ms -> 100 ms`
  - map `Knob 2` to `0.0 -> 0.5`
- Reverb:
  - map `Knob 1` to simplified decay
  - map `Knob 2` to linear mix
  - keep fixed LP tone

**Step 4: Implement `SW1` short press vs hold**

- short press on Tube toggles Tube bypass
- hold on Tube enters shift while held
- release after hold exits shift without toggling bypass
- non-Tube pages keep normal short-press bypass behavior

**Step 5: Update firmware header comments**

- Rewrite the file header so the documented controls match the new Tube/Delay/
  Reverb design exactly

### Task 3: Update README

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/README.md`

**Step 1: Update pages and control table**

- Remove WahWah from active pages
- Document Tube shift behavior
- Document new Delay and Reverb parameter ranges

**Step 2: Update soft takeover text**

- Explain that Tube default and Tube shift layers each preserve their own values

**Step 3: Add revisit note**

- Note that DAFX `WahWah` is removed from the live pedal and marked for future
  revisit

### Task 4: Verify everything

**Files:**
- Test: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Test: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`

**Step 1: Run the host-side test**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- exit `0`

**Step 2: Run Daisy QAE lint**

Run:

```powershell
$env:PYTHONIOENCODING='utf-8'
py -3 ../../../DAISY_QAE/validate_daisy_code.py .
```

Expected:

- `0 error(s), 0 warning(s)`

**Step 3: Run a clean rebuild**

Run:

```powershell
if(Test-Path 'build'){Remove-Item -LiteralPath 'build' -Recurse -Force}
make
```

Expected:

- successful firmware build
- updated memory usage output
