# Pod_MultiFX_Chain

Platform: Daisy Pod
Category: Multi-effect pedal
Complexity: Moderate-High

## Overview

`Pod_MultiFX_Chain` is a four-stage serial effect chain for Daisy Pod:

`Input -> Overdrive -> Delay -> WahWah -> Reverb -> Output`

The encoder selects the edit page. Each page owns its own stored knob values and
bypass state. Switching pages does not overwrite those stored values because the
project uses soft takeover.

Important distinction:

- Signal-chain order: Overdrive -> Delay -> WahWah -> Reverb
- Edit-page order: Overdrive -> Delay -> Reverb -> WahWah

## Controls

| Control | Function | Details |
|---------|----------|---------|
| Encoder turn | Change selected page | Overdrive -> Delay -> Reverb -> WahWah |
| Button 1 / SW1 | Toggle page bypass | Bypasses the currently selected effect page |
| Button 2 / SW2 | Toggle global bypass | Bypasses the full chain |
| Knob 1 | Parameter 1 | Depends on selected page, uses soft takeover |
| Knob 2 | Parameter 2 | Depends on selected page, uses soft takeover |
| RGB LED | Page indicator | Red=OD, Green=Delay, Blue=Reverb, Cyan=Wah |

### Per-page parameter map

#### Overdrive (Red)
- Knob 1: Drive amount (`0.0 -> 1.0`)
- Knob 2: Reserved / unused

#### Delay (Green)
- Knob 1: Delay time (`10 ms -> 1000 ms`)
- Knob 2: Feedback (`0.0 -> 0.95`)

#### Reverb (Blue)
- Knob 1: Feedback / decay (`0.50 -> 0.99`)
- Knob 2: Low-pass frequency (`1000 Hz -> 18000 Hz`)

#### WahWah (Cyan)
- Knob 1: Center frequency (`200 Hz -> 2000 Hz`)
- Knob 2: Depth (`0.0 -> 1.0`)

## Soft takeover

This project uses soft takeover, also called pickup/catch.

- Each page keeps its saved knob values when you leave it.
- Switching to another page does not jump immediately to the physical knob
  positions.
- A knob starts editing the new page only after the physical position crosses
  the saved page value.

This prevents sudden jumps when one pair of physical knobs is reused across
multiple pages.

## LED states

| Color | Brightness | Meaning |
|-------|------------|---------|
| Red | Full | Overdrive selected and active |
| Green | Full | Delay selected and active |
| Blue | Full | Reverb selected and active |
| Cyan | Full | WahWah selected and active |
| Selected page color | Dim | Selected page bypassed, or global bypass enabled |

## Implementation notes

- `Overdrive`, `DelayLine`, and `ReverbSc` come from DaisySP.
- `WahWah` is pulled from `../../DAFX_2_Daisy_lib/src/effects/wahwah.cpp`.
- `ReverbSc` requires `USE_DAISYSP_LGPL = 1` in the project `Makefile`.
- Delay storage uses `DelayLine<float, 48000>` in SDRAM for up to 1 second of
  delay at 48 kHz.
- Following `DAISY_QAE` BUG-005 guidance, analog controls are processed in the
  audio callback with `hw.ProcessAnalogControls()`, while encoder and button
  events are processed in the main loop with `hw.ProcessDigitalControls()`.
- Page control state is isolated in `pod_multifx_pages.h` so it can be tested on
  the host without Daisy hardware.

## Build

### Prerequisites

- ARM GCC toolchain installed
- `libDaisy` available at `../../../libDaisy`
- `DaisySP` available at `../../../DaisySP`
- `make` available in `PATH`

### Compile

```powershell
cd DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain
make clean
make
```

### Flash

```powershell
make program-dfu
```

## Verification

### Host-side control test

Use Visual Studio Build Tools to compile the page-state test on Windows:

```powershell
cmd /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" && cl /nologo /EHsc /std:c++17 /W4 tests\pod_multifx_pages_test.cpp /Fe:tests\pod_multifx_pages_test.exe'
tests\pod_multifx_pages_test.exe
```

### Daisy QAE lint

On Windows PowerShell, force UTF-8 console output before invoking the QAE
validator, otherwise the script can fail while printing Unicode separators.

```powershell
$env:PYTHONIOENCODING='utf-8'
py -3 ../../../DAISY_QAE/validate_daisy_code.py .
```

Fresh verification on 2026-04-14:

- Host-side page-control test: passed
- Daisy QAE linter: pending fresh run after final edits
- Clean rebuild: pending fresh run after final edits

## Usage

1. Power on the Daisy Pod. The project starts with Overdrive selected.
2. Turn the encoder to choose the effect page you want to edit.
3. Turn `Knob 1` and `Knob 2` to edit the selected page.
4. If the knobs appear inactive right after a page change, keep turning until
   they cross the saved page values and soft takeover reattaches them.
5. Press `SW1` to bypass or re-enable the selected page.
6. Press `SW2` to bypass or re-enable the whole chain.
7. Watch the LED color for page selection and brightness for bypass state.

## Typical settings

### Ambient clean

1. Select Overdrive and bypass it with `SW1`.
2. Select Delay and set a short time with low feedback.
3. Select Reverb and increase decay with a brighter LP frequency.
4. Select WahWah and keep depth low.

### Heavy lead

1. Select Overdrive and raise drive into the `0.7 -> 0.8` range.
2. Select Delay and set a medium repeat with moderate feedback.
3. Select Reverb and keep decay short to medium.
4. Select WahWah and add moderate depth for movement.

### Fast compare

1. Dial each page once.
2. Use the encoder to move between pages without losing those settings.
3. Use `SW1` to compare an individual stage.
4. Use `SW2` to compare the full processed chain against dry input.

## Troubleshooting

| Issue | Likely cause | Check |
|------|--------------|-------|
| Selected page does not respond right after switching | Soft takeover has not captured yet | Keep turning the knob until it crosses the saved page value |
| Selected effect has no audible change | The selected stage is bypassed | Press `SW1` and confirm the LED returns to full brightness |
| All processing disappears at once | Global bypass is enabled | Press `SW2` and confirm the LED returns to full brightness |
| Delay runs away | Feedback is too high | Lower Delay Knob 2 |
| Wah effect is too subtle | Depth is low | Select WahWah and raise Knob 2 |
| Knobs feel sluggish after code changes | Analog control polling moved out of the callback | Restore `hw.ProcessAnalogControls()` in `AudioCallback` and keep ADC started before audio |
| Build fails at link time with `ReverbSc` references | LGPL module flag missing | Confirm `USE_DAISYSP_LGPL = 1` is still present in the `Makefile` |

## Source files

- Main firmware: `Pod_MultiFX_Chain.cpp`
- Page-control helper: `pod_multifx_pages.h`
- Host-side test: `tests/pod_multifx_pages_test.cpp`
- Project build config: `Makefile`
- External Wah source: `../../DAFX_2_Daisy_lib/src/effects/wahwah.cpp`

## License

- Project code: MIT
- DaisySP: MIT
- `ReverbSc`: LGPL via `USE_DAISYSP_LGPL = 1`
