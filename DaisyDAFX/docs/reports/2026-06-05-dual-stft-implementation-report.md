# Daisy Dual STFT Implementation Report

## Executive Summary

Implemented a dual STFT comparison path in DaisyDAFX with two shared-contract backends:

- `FastStftBackend`: Farmer2K5-style streaming STFT with fixed buffers, Hann windowing, CMSIS-backed RFFT/IFFT on Daisy firmware, portable FFT fallback on host, complex/magnitude-phase hooks, and squared-COLA correction.
- `DafxStftEnvBackend`: C++ identity port of DAFX `stftenv.m` using zero-phase frame placement, CMSIS-backed full-spectrum FFT/IFFT reconstruction on Daisy firmware, overlap-add, envelope accumulation, and stream-safe envelope division.

Both backends pass host identity tests for `512/128/64` and `1024/256/64`. The firmware builds for Daisy Seed and for the connected Daisy Field target via `STFT_BOARD_FIELD`; all four Daisy Field backend/profile variants build, flash, verify, and report serial CPU telemetry. Measured Daisy Field CPU for the required `512/128/64` profile is Fast `13.008%` max / `6.505%` avg and DAFX `16.060%` max / `8.183%` avg.

## Conclusions

The implementation satisfies the prompt completion gate: both backends compile, host identity tests pass, firmware builds and flashes, memory is reported, and runtime CPU is measured on the connected Daisy Field. It is still not fair to call the DAFX backend optimized; it is identity-correct and measured, but the Fast backend has lower peak and average CPU in both tested profiles.

## Personal Opinion

This is now good enough to start a controlled `VX_robot`-style magnitude-only experiment. I would keep pitch/time effects separate until a MagPhase passthrough benchmark is captured, because the 1024 DAFX identity path already peaks at `31.319%`.

## Source Evidence Used

| Source | Evidence Used |
| --- | --- |
| `Farmer2K5/daisy-fast-stft` commit `6eda9c43ccb771ae1598260bae13183d9f6b51e2` | Local source cache under `.tmp/source_evidence/daisy-fast-stft/`: `fast_stft.h`, `fast_rfft.h`, `fast_istft.h`, `fast_window.h`, `fast_spectral.h`, `ex_stft_mono.cpp`, `Performance.md` |
| `DAFX-MATLAB/M_files_chap10/stftenv.m` | Zero-phase placement, full-spectrum reconstruction, overlap-add, `yenv` compensation |
| `DAFX-MATLAB/M_files_chap10/stft.m` | Simpler identity comparison without final envelope compensation |
| `DAFX-MATLAB/M_files_chap07/VX_robot.m` | Magnitude-only first effect candidate after identity passes |
| `DAFX-MATLAB/M_files_chap07/VX_pv_nothing.m` | Magnitude/phase passthrough sanity reference |
| Weekly digest report and dashboard JSON | Confirmed the earlier decision to treat DAFX as algorithm-reference until measured |

## Implemented Files

| File | Purpose |
| --- | --- |
| `src/spectral/dual_stft.h` | Header-only STFT helpers, `FastStftBackend`, `DafxStftEnvBackend`, and runtime `DualStftProcessor` |
| `tests/test_dual_stft.cpp` | Host tests for windows, COLA/envelope, silence, sine identity, impulse boundedness, and backend switch contract |
| `examples/daisy_dual_stft/` | Daisy Seed / Daisy Field firmware example with compile-time board/backend/profile selection and `CpuLoadMeter` reporting |
| `docs/benchmarks/2026-06-05-dual-stft-results.json` | Machine-readable host RMS, firmware memory, source evidence, and measured Daisy runtime CPU fields |

## Algorithms Included

- Hann window generation and sum normalization.
- Squared-COLA gain calculation for the Fast-style backend.
- DAFX envelope-period calculation and thresholding.
- `Fast_RFFT` wrapper using CMSIS `arm_rfft_fast_f32` on Daisy firmware and DaisyDAFX `FFTHandler` on host.
- `Fast_ISTFT` overlap-add helper for inverse real FFT frame reconstruction.
- Magnitude/phase conversion helpers for future effects.
- Streaming input ring buffers and output overlap-add rings.
- DAFX zero-phase FFT placement and inverse zero-phase unwrap.
- Runtime backend switch wrapper for host/library use.
- Compile-time firmware board/backend/profile selection for realistic Seed and Field RAM use, including explicit `STFT_BACKEND_FAST` / `STFT_BACKEND_DAFX` and `STFT_PROFILE_512` / `STFT_PROFILE_1024` options.
- Windows ST-Link automation that resolves the programming/debug VCP (`COM3` on this machine) to the Daisy USB serial logger (`COM5` on this machine) before CPU capture.

## Verification Results

| Check | Result |
| --- | --- |
| Red test before implementation | Passed as expected: missing `spectral/dual_stft.h` compile failure |
| `cmake --build build_dual_stft --config Release --target unit_tests` | Passed |
| Focused STFT tests | Passed: 13/13 |
| `ctest --test-dir build_dual_stft -C Release --output-on-failure` | Passed: `1/1` test target, `100%` tests passed |
| `make` Seed Fast `512/128/64` | Passed after Field patch: FLASH `96108 B` `73.32%`, SRAM `51440 B` `9.81%`, RAM_D2 `16 KB` `5.56%`, SDRAM `0%` |
| `make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_512"` | Passed: FLASH `112040 B` `85.48%`, SRAM `91696 B` `17.49%`, RAM_D2 `17224 B` `5.84%`, SDRAM `0%` |
| `make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_512"` | Passed: FLASH `112360 B` `85.72%`, SRAM `100384 B` `19.15%`, RAM_D2 `17224 B` `5.84%`, SDRAM `0%` |
| `make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_FAST -DSTFT_PROFILE_1024"` | Passed: FLASH `116120 B` `88.59%`, SRAM `118320 B` `22.57%`, RAM_D2 `17224 B` `5.84%`, SDRAM `0%` |
| `make STFT_DEFS="-DSTFT_BOARD_FIELD -DSTFT_BACKEND_DAFX -DSTFT_PROFILE_1024"` | Passed: FLASH `116480 B` `88.87%`, SRAM `135712 B` `25.89%`, RAM_D2 `17224 B` `5.84%`, SDRAM `0%` |
| `tools\measure_dual_stft_field.ps1 -Port COM3 -Program -Include1024` | Passed: ST-Link programmed and OpenOCD verified all four variants; runtime telemetry captured from Daisy USB serial logger `COM5` |

## Daisy Field CPU Measurements

| Backend | Profile | Max CPU | Average CPU | Samples | Capture |
| --- | --- | --- | --- | --- | --- |
| `FastStftBackend` | `512/128/64` | `13.008%` | `6.505%` | `67` | `.tmp/dual-stft-cpu-capture-20260606-153216.json` |
| `DafxStftEnvBackend` | `512/128/64` | `16.060%` | `8.183%` | `67` | `.tmp/dual-stft-cpu-capture-20260606-153343.json` |
| `FastStftBackend` | `1024/256/64` | `25.719%` | `6.666%` | `67` | `.tmp/dual-stft-cpu-capture-20260606-153508.json` |
| `DafxStftEnvBackend` | `1024/256/64` | `31.319%` | `8.291%` | `67` | `.tmp/dual-stft-cpu-capture-20260606-153630.json` |

## Completion Audit

| Prompt Requirement | Current Evidence | Status |
| --- | --- | --- |
| Both STFT backends compile | Host tests and Field firmware builds include `FastStftBackend` and `DafxStftEnvBackend` | Met |
| Host-side identity tests pass | Focused dual-STFT tests cover 512 and 1024 sine identity, silence, impulse, window/COLA/envelope, and backend switch contract | Met |
| Firmware build succeeds | Daisy Field 512 and 1024 backend variants build and report memory usage | Met |
| CPU and memory reported for at least mono `512/128/64` | Memory and measured Daisy runtime CPU are reported for both Field 512 variants | Met |
| Report distinguishes measured runtime from source-only claims | Benchmark JSON labels Farmer2K5 CPU rows as source references and this port's CPU rows as measured Daisy runtime | Met |

## Host Reconstruction RMS

| Backend | Profile | RMS Error |
| --- | --- | --- |
| `FastStftBackend` | `512/128/64` | `2.006718347e-08` |
| `DafxStftEnvBackend` | `512/128/64` | `2.020931156e-08` |
| `FastStftBackend` | `1024/256/64` | `2.188945913e-08` |
| `DafxStftEnvBackend` | `1024/256/64` | `2.148411760e-08` |

## Not Measured

- Hardware audio quality, noise, and glitch behavior.
- MagPhase processing cost in this port.
- `VX_robot` or phase-vocoder effects.

## Reproduce Hardware Measurement

On this Windows machine the ST-Link programming/debug VCP appears as `COM3`, and the flashed Daisy USB CDC logger appears as `COM5`. The measurement runner resolves that automatically after programming.

```powershell
# Build-only check, no flashing.
powershell -ExecutionPolicy Bypass -File tools\measure_dual_stft_field.ps1

# Explicitly flash and capture the required 512/128/64 Field pair.
powershell -ExecutionPolicy Bypass -File tools\measure_dual_stft_field.ps1 -Port COM3 -Program

# Explicitly flash and capture all four Field benchmark variants.
powershell -ExecutionPolicy Bypass -File tools\measure_dual_stft_field.ps1 -Port COM3 -Program -Include1024

# Manual one-variant capture after flashing.
powershell -ExecutionPolicy Bypass -File tools\capture_dual_stft_cpu.ps1 -Port COMx -Seconds 65 -UpdateBenchmark

# If the serial monitor was captured outside this shell.
powershell -ExecutionPolicy Bypass -File tools\capture_dual_stft_cpu.ps1 -InputLog path\to\serial.log -UpdateBenchmark
```

The capture script only updates `docs/benchmarks/2026-06-05-dual-stft-results.json` after it sees real `CPU max/avg/min %` telemetry from the firmware.
