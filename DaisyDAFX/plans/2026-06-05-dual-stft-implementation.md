# Daisy Dual STFT Implementation Plan

> **For Codex:** REQUIRED SUB-SKILL: Use superpowers:executing-plans style task-by-task execution when continuing this plan.

**Goal:** Add a portable dual STFT comparison path with a Farmer2K5-style backend and a DAFX `stftenv.m` identity backend.

**Architecture:** Keep the core algorithms header-only under `src/spectral/` so DaisyDAFX remains a host-testable library. Add a separate Daisy Seed firmware example that uses the same backend API and reports CPU load without putting logging in the audio callback.

**Tech Stack:** C++17, DaisyDAFX `FFTHandler`, Google Test, CMake, libDaisy Make firmware example.

---

### Task 1: Source Evidence

**Files:**
- Read: `.tmp/source_evidence/daisy-fast-stft/include/DaisyFarm/STFT/*.h`
- Read: `.tmp/source_evidence/daisy-fast-stft/ex_stft_mono.cpp`
- Read: `.tmp/source_evidence/daisy-fast-stft/Performance.md`
- Read: `DAFX-MATLAB/M_files_chap10/stftenv.m`
- Read: `DAFX-MATLAB/M_files_chap10/stft.m`
- Read: `DAFX-MATLAB/M_files_chap07/VX_robot.m`
- Read: `DAFX-MATLAB/M_files_chap07/VX_pv_nothing.m`

**Steps:**
1. Record source commit and paths in the implementation report.
2. Use Farmer2K5 code for streaming STFT structure, window/COLA, packed spectral helper behavior, and CPU reference labels.
3. Use DAFX code for zero-phase placement, full-spectrum reconstruction, and envelope compensation semantics.

### Task 2: Host Tests First

**Files:**
- Create: `tests/test_dual_stft.cpp`
- Modify: `tests/CMakeLists.txt`

**Steps:**
1. Add tests for deterministic Hann windows and envelope/COLA stability.
2. Add silence, impulse boundedness, sine identity, and backend switch contract tests.
3. Run the focused CMake build and confirm the test fails because the new backend header does not exist yet.

### Task 3: Library Implementation

**Files:**
- Create: `src/spectral/dual_stft.h`

**Steps:**
1. Add `StftBackendKind`, processing mode tags, spectral conversion helpers, and fixed-size FFT frame buffers.
2. Add `FastStftBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE>` using normal frame extraction, Hann analysis/synthesis windows, FFT/IFFT, complex processing hook, and COLA correction.
3. Add `DafxStftEnvBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE>` using DAFX-style zero-phase placement, full-spectrum reconstruction, overlap-add, envelope accumulation, and stream-safe threshold compensation.
4. Add a compile-time selector and a runtime switching wrapper with the same block contract.

### Task 4: Firmware Example

**Files:**
- Create: `examples/daisy_dual_stft/Makefile`
- Create: `examples/daisy_dual_stft/daisy_dual_stft.cpp`
- Create: `examples/daisy_dual_stft/README.md`

**Steps:**
1. Use Daisy Seed as the first firmware target unless a stronger local board rule appears.
2. Support `STFT_BACKEND_FAST` / `STFT_BACKEND_DAFX` and `STFT_PROFILE_512` / `STFT_PROFILE_1024` compile-time options.
3. Keep the audio callback DSP-only and report CPU once per second from the main loop.

### Task 5: Benchmark And Report

**Files:**
- Create: `docs/reports/2026-06-05-dual-stft-implementation-report.md`
- Create: `docs/benchmarks/2026-06-05-dual-stft-results.json`

**Steps:**
1. Record host reconstruction RMS values from tests.
2. Record firmware memory from `make` when the build is available.
3. Mark CPU as measured only if hardware runtime data is actually captured; otherwise keep runtime CPU null and name the next action.

### Task 6: Verification

**Commands:**
- `cmake -S DaisyDAFX -B DaisyDAFX/build -DBUILD_EXAMPLES=OFF`
- `cmake --build DaisyDAFX/build --config Release --target unit_tests`
- `ctest --test-dir DaisyDAFX/build -C Release --output-on-failure`
- `make` from `DaisyDAFX/examples/daisy_dual_stft`

**Acceptance:**
- Host tests pass.
- Firmware source builds or exact toolchain/build failure is reported.
- Report distinguishes measured runtime, source-inferred data, and unmeasured data.
