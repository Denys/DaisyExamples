# Pod MultiFX Reattach and Slew Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace `Pod_MultiFX_Chain` soft takeover with stored page positions, first-move reattach, and a short slew to the new control target, while downgrading the Daisy QAE soft-takeover rule to a recommendation.

**Architecture:** Keep all page and layer ownership in `pod_multifx_pages.h`, but swap the old capture-based logic for detached knobs with movement-anchor detection and fixed-rate slewing. The firmware continues to read raw Pod controls in the audio callback and now consumes the helper’s effective smoothed values rather than soft-takeover-captured values.

**Tech Stack:** C++, libDaisy, DaisySP, DAFX `Tube`, Daisy Pod hardware, Markdown documentation, host-side MSVC test executable

---

### Task 1: Replace the host-side behavior test

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Write the failing test**

- Replace soft-takeover capture assertions with:
  - detached state after page/layer switch
  - first update recording a movement anchor
  - reattach when movement exceeds threshold
  - intermediate slew value after partial elapsed time
  - arrival at target after enough elapsed time

**Step 2: Run test to verify it fails**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- compile failure or assertion failure because the helper still exposes capture
  flags and does not implement detached reattach with slew

**Step 3: Write minimal helper changes**

- remove soft-takeover capture logic
- add detached anchor state and effective-target slewing
- keep page memory and shift-layer logic intact

**Step 4: Run test to verify it passes**

Run:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

Expected:

- clean compile
- executable exits `0`

### Task 2: Refactor the firmware to use effective slewed values

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`
- Use: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Change control update calls**

- pass per-block elapsed time into the helper
- keep analog processing in the audio callback

**Step 2: Use helper effective values everywhere**

- Tube
- Delay
- Reverb
- LFO coarse/fine effective values

**Step 3: Keep interaction semantics unchanged**

- page selection
- encoder push on LFO
- SW1 hold on Tube and LFO
- page bypass
- global bypass
- LEDs

### Task 3: Update standards and docs

**Files:**
- Modify: `DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/README.md`
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`

**Step 1: Downgrade the QAE rule**

- replace mandatory soft-takeover wording with recommendation/promemoria wording
- mention alternative safe patterns such as first-move reattach with slew

**Step 2: Update firmware header comments**

- remove soft-takeover wording
- document stored position recall and smooth reattach behavior

**Step 3: Update README**

- replace the soft-takeover section
- document movement reattach and short slew behavior
- refresh troubleshooting text

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
