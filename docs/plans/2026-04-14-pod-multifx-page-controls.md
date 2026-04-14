# Pod MultiFX Page Controls Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refactor `Pod_MultiFX_Chain` so encoder turns select effect pages, `SW1` bypasses the current effect, `SW2` bypasses the full chain, and page-owned parameters persist with soft takeover.

**Architecture:** Move page and soft-takeover behavior into a small Daisy-independent helper header that owns selected-page, per-page parameter storage, per-page bypass, and global bypass. Use that helper from the existing firmware file so the audio callback keeps applying DSP parameters from stored state while the main loop handles page switching and bypass toggles.

**Tech Stack:** C++, libDaisy, DaisySP, Daisy Pod hardware, host-side `g++` test executable, Markdown documentation

---

### Task 1: Add host-side control-state test harness

**Files:**
- Create: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Create: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Write the failing test**

- Add a host-side test program covering:
  - wrapped page increment/decrement
  - page switch arms soft takeover
  - knobs do not overwrite stored values until capture
  - capture occurs after crossing saved value
  - per-page values persist when switching away and back
  - current-page bypass and global bypass toggle independently

**Step 2: Run test to verify it fails**

Run:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/pod_multifx_pages_test.cpp -o tests/pod_multifx_pages_test.exe
```

Expected:

- compile failure because `pod_multifx_pages.h` helpers do not exist yet

**Step 3: Write minimal helper implementation**

- Add pure C++ types and functions for:
  - page wrapping
  - page selection
  - per-page state storage
  - soft takeover capture
  - bypass toggling

**Step 4: Run test to verify it passes**

Run:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/pod_multifx_pages_test.cpp -o tests/pod_multifx_pages_test.exe
.\tests\pod_multifx_pages_test.exe
```

Expected:

- clean compile
- executable exits `0`

### Task 2: Refactor firmware to use stored page state

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`
- Use: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/pod_multifx_pages.h`

**Step 1: Replace direct current-page knob writes**

- Remove the current pattern where the selected page is driven directly from raw
  knob values
- Introduce stored normalized values for each page

**Step 2: Apply soft takeover in the audio callback**

- Keep `hw.ProcessAnalogControls()` in the callback
- Read raw knob values with `hw.knob1.Value()` and `hw.knob2.Value()`
- Feed them through the soft-takeover helper for the selected page only

**Step 3: Apply all effect parameters from stored state**

- Always set DSP parameters from stored page values
- Keep signal-chain order as:
  - Overdrive
  - Delay
  - WahWah
  - Reverb

**Step 4: Add global bypass**

- Short-circuit the chain to dry stereo output when global bypass is enabled

**Step 5: Move page selection to encoder and remap switches**

- Encoder increment changes selected page
- `button1` toggles bypass on the selected page
- `button2` toggles global bypass

**Step 6: Keep LED behavior aligned**

- LED color follows the selected page
- LED brightness dims for global bypass or current-page bypass

### Task 3: Update project documentation

**Files:**
- Modify: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/README.md`

**Step 1: Update control table**

- Document encoder page switching
- Document `SW1` current-page bypass
- Document `SW2` global bypass

**Step 2: Document soft takeover**

- Explain that page values persist
- Explain that knobs must cross saved values before editing after a page change

**Step 3: Update usage and troubleshooting**

- Add page-switch workflow
- Add troubleshooting note for apparent knob inactivity after page switch

### Task 4: Update DAISY_QAE standards

**Files:**
- Modify: `DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

**Step 1: Add general soft-takeover rule**

- State that Seed and Field projects with banked or page-switched parameters
  must use pickup/catch/soft takeover
- Make the rule explicit that stored values own the parameter after page/bank
  switch until the knob crosses the saved value

**Step 2: Keep terminology aligned**

- Mention that "pickup/catch" and "soft takeover" describe the same behavior

### Task 5: Verify everything

**Files:**
- Test: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/tests/pod_multifx_pages_test.cpp`
- Test: `DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain/Pod_MultiFX_Chain.cpp`
- Test: `DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

**Step 1: Run the host-side test**

Run:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/pod_multifx_pages_test.cpp -o tests/pod_multifx_pages_test.exe
.\tests\pod_multifx_pages_test.exe
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
