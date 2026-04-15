# Pod MultiFX LFO Page Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a fourth LFO page to `Pod_MultiFX_Chain` with waveform selection on encoder push, LED2 waveform/rate feedback, SW1-held fine controls, and front-of-chain tremolo-style modulation.

**Architecture:** Keep page state in `pod_multifx_pages.h` and extend it to four pages plus generic page-owned shift layers. The firmware will translate the stored LFO coarse/fine values into effective amplitude and period, drive a DaisySP oscillator for the modulation stage, and update both Pod LEDs from the shared control state.

**Tech Stack:** C++, libDaisy, DaisySP `Oscillator`, DAFX `Tube`, Daisy Pod hardware, host-side MSVC test executable, Markdown documentation

---

### Task 1: Extend the host-side control-state test

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Write the failing test**

- Add expectations for:
  - four-page wrap
  - LFO waveform cycling
  - LFO coarse and fine layer persistence
  - `SW1` short press bypass on LFO
  - `SW1` hold fine-layer entry on LFO
  - LFO amplitude and period mappings

**Step 2: Run test to verify it fails**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- compile failure or assertion failure because the helper still implements only
  three pages and has no LFO state

**Step 3: Write minimal helper changes**

- Add `PAGE_LFO`
- Add LFO waveform state
- Generalize `SW1` hold behavior so any page with a shift layer can use it
- Add helpers for waveform cycling and LFO effective parameter mapping

**Step 4: Run test to verify it passes**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- clean compile
- executable exits `0`

### Task 2: Implement the LFO runtime

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`
- Use: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Add the modulation stage**

- Add DaisySP `Oscillator` for the LFO
- Set waveform from shared control state
- Place the tremolo-style gain stage before Tube in the audio callback

**Step 2: Add LFO parameter application**

- Convert stored coarse/fine controls into effective amplitude and period
- Map amplitude exponentially to `0.0 -> 1.0`
- Map period exponentially to `5.0 s -> 0.1 s`
- Convert period to oscillator frequency

**Step 3: Add encoder push waveform cycling**

- On `hw.encoder.RisingEdge()` while LFO page is selected, cycle:
  - Sine
  - Triangle
  - Square

**Step 4: Add LED2 waveform feedback**

- `led2`:
  - Green = Sine
  - Blue = Triangle
  - Red = Square
- Scale LED2 brightness from the effective LFO rate
- Dim LED2 when the LFO page or whole chain is bypassed

**Step 5: Keep existing behavior intact**

- Preserve Tube shift controls
- Preserve page bypass and global bypass semantics
- Preserve soft takeover across all pages and logical layers

### Task 3: Update documentation

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/README.md`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`

**Step 1: Update firmware header comment**

- Document the new LFO page, encoder push waveform cycling, LED2 behavior, and
  LFO coarse/fine controls

**Step 2: Update README page list and controls**

- Add the LFO page
- Document the LFO modulation stage as front-of-chain input modulation
- Document waveform colors and rate brightness on `led2`
- Document `SW1` hold fine edits on LFO

**Step 3: Update verification notes**

- Refresh the build footprint after the new code lands

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
