# Pod_MultiFX_Chain

Platform: Daisy Pod  
Category: Multi-effect pedal  
Complexity: Moderate

## Overview

`Pod_MultiFX_Chain` is a four-page Daisy Pod pedal with a front-of-chain LFO
modulation stage.

Signal chain:

`Input -> LFO -> Tube -> Delay -> Reverb -> Output`

Edit page order:

`Tube -> Delay -> Reverb -> LFO`

The encoder selects the current edit page. Each page owns its own stored values.
Tube has a hidden shift layer for `Bias / Distortion`, and the LFO page has a
hidden fine layer for `Amplitude / Rate`. Page and layer changes keep the
stored values active until the knob is moved, then drift smoothly to the new
physical target.

## Pages

- Tube
- Delay
- Reverb
- LFO

Note: DAFX `WahWah` has been removed from the active pedal and is marked as a
future revisit item.

## Controls

| Control | Function | Details |
|---------|----------|---------|
| Encoder turn | Change selected page | Tube -> Delay -> Reverb -> LFO |
| Encoder push on LFO | Change waveform | Sine -> Triangle -> Square |
| Button 1 / SW1 short press | Toggle page bypass | Bypasses the selected page |
| Button 1 / SW1 hold on Tube | Temporary shift | Tube edits Bias / Distortion |
| Button 1 / SW1 hold on LFO | Temporary fine layer | LFO edits Amplitude fine / Rate fine |
| Button 2 / SW2 | Toggle global bypass | Bypasses the full chain |
| Knob 1 | Parameter 1 | Depends on page and shift state |
| Knob 2 | Parameter 2 | Depends on page and shift state |
| RGB LED 1 | Page indicator | Red=Tube, Green=Delay, Blue=Reverb, Yellow=LFO |
| RGB LED 2 | LFO waveform and rate | Green=Sine, Blue=Triangle, Red=Square; brightness tracks LFO rate |

## Parameter map

### Tube (LED 1 Red)

Default layer:

- Knob 1: Drive
- Knob 2: Mix

Shift layer while `SW1` is held:

- Knob 1: Bias
- Knob 2: Distortion

Tube mapping rules:

- Mix is linear
- Drive is exponential
- Bias is center-biased exponential around `0`
- Distortion is exponential

### Delay (LED 1 Green)

- Knob 1: Time (`10 ms -> 100 ms`)
- Knob 2: Feedback (`0.0 -> 0.5`)

### Reverb (LED 1 Blue)

- Knob 1: Decay
- Knob 2: Mix

Reverb tone is intentionally fixed to keep the page simple.

### LFO (LED 1 Yellow, LED 2 waveform)

Default layer:

- Knob 1: Amplitude coarse
- Knob 2: Rate coarse

Fine layer while `SW1` is held:

- Knob 1: Amplitude fine
- Knob 2: Rate fine

LFO rules:

- Encoder push cycles waveform: `Sine -> Triangle -> Square`
- Amplitude maps exponentially from `0.0 -> 1.0`
- Rate is stored as period and maps exponentially from `5.0 s -> 0.1 s`
- LFO modulates the input as a unipolar tremolo-style gain stage before Tube

## Stored Positions and Smooth Reattach

This project keeps stored page values and uses movement-triggered reattach with
a short control slew.

- Each page keeps its saved values when you leave it.
- Tube default and Tube shift layers keep separate saved values.
- LFO coarse and LFO fine layers keep separate saved values.
- Switching page or logical layer leaves the saved value active immediately.
- The first meaningful knob movement reattaches that logical control.
- After reattach, the effective value slews toward the physical knob position
  over a short ramp instead of jumping abruptly.

## LED states

### LED 1

| Color | Brightness | Meaning |
|-------|------------|---------|
| Red | Full | Tube selected and active |
| Green | Full | Delay selected and active |
| Blue | Full | Reverb selected and active |
| Yellow | Full | LFO selected and active |
| Selected page color | Dim | Selected page bypassed, or global bypass enabled |

### LED 2

| Color | Meaning |
|-------|---------|
| Green | LFO waveform is Sine |
| Blue | LFO waveform is Triangle |
| Red | LFO waveform is Square |

LED 2 brightness follows the effective LFO rate. It dims heavily if the LFO
page is bypassed or global bypass is enabled.

## Implementation notes

- Tube uses DAFX `Tube` from `../../DAFX_2_Daisy_lib/src/effects/tube.cpp`.
- Delay uses `DelayLine<float, 48000>` in SDRAM.
- Reverb uses DaisySP `ReverbSc` with a fixed LP tone and page-controlled
  `Decay + Mix`.
- LFO uses DaisySP `Oscillator` and applies unipolar amplitude modulation
  before Tube.
- Following `DAISY_QAE` BUG-005 guidance, analog controls are processed in the
  audio callback with `hw.ProcessAnalogControls()`, while encoder and button
  events are processed in the main loop with `hw.ProcessDigitalControls()`.
- Page state, waveform state, shift-layer state, and smooth reattach logic live
  in `pod_multifx_pages.h` so they can be host-tested.

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

Fresh verification on 2026-04-15:

- Host-side page-control test: passed
- Daisy QAE linter: `0 error(s), 0 warning(s)`
- Clean rebuild: passed

Fresh build footprint on 2026-04-15:

- Flash: `82992 B / 128 KB` (`63.32%`)
- SRAM: `446640 B / 512 KB` (`85.19%`)
- RAM_D2: `16960 B / 288 KB` (`5.75%`)
- SDRAM: `192012 B / 64 MB` (`0.29%`)

## Usage

1. Power on the Daisy Pod. The project starts on the Tube page.
2. Turn the encoder to choose Tube, Delay, Reverb, or LFO.
3. Turn `Knob 1` and `Knob 2` to edit the selected page.
4. On the Tube page, hold `SW1` to temporarily edit `Bias` and `Distortion`.
5. On the LFO page, push the encoder to cycle `Sine`, `Triangle`, and `Square`.
6. On the LFO page, hold `SW1` to temporarily edit fine `Amplitude` and fine
   `Rate`.
7. Release `SW1` to return to the default logical layer on Tube or LFO.
8. Short-press `SW1` to bypass the selected page.
9. Press `SW2` to bypass or re-enable the whole chain.
10. If a page keeps its old value right after switching, move the knob
    deliberately. The stored value will stay active until movement is detected,
    then drift smoothly to the live knob position.

## Troubleshooting

| Issue | Likely cause | Check |
|------|--------------|-------|
| Selected page keeps its old value right after switching | The logical control is still waiting for movement reattach | Move the knob a bit farther so reattach is detected |
| Tube or LFO shift does not engage | `SW1` was tapped instead of held | Hold `SW1` for a short moment before turning the knobs |
| Page bypass toggles when trying to shift | `SW1` was released before hold threshold | Hold `SW1` longer before releasing |
| LFO waveform does not change | Encoder was turned instead of pressed | Push the encoder while the LFO page is selected |
| LED 2 is very dim | LFO rate is slow, or LFO/global bypass is active | Increase LFO rate or disable bypass |
| All processing disappears at once | Global bypass is enabled | Press `SW2` and confirm LED 1 returns to full brightness |
| Build fails at link time with `Tube` symbols missing | `tube.cpp` is not in the build | Confirm the `Makefile` includes `../../DAFX_2_Daisy_lib/src/effects/tube.cpp` |

## Source files

- Main firmware: `Pod_MultiFX_Chain.cpp`
- Page-control helper: `pod_multifx_pages.h`
- Host-side test: `tests/pod_multifx_pages_test.cpp`
- Project build config: `Makefile`
- DAFX Tube source: `../../DAFX_2_Daisy_lib/src/effects/tube.cpp`

## License

- Project code: MIT
- DaisySP: MIT
- DAFX `Tube`: see repository licensing for `DAFX_2_Daisy_lib`
- `ReverbSc`: LGPL via `USE_DAISYSP_LGPL = 1`
