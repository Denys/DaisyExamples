# Local validation

Date: 2026-08-16

This file records checks actually executed against this bootstrap package in the generation runtime.

## Environment

- CMake: detected and used successfully
- C++ compiler: GNU 14.2.0
- Host target: Linux container
- C++ standard: C++17

## Results

| Check | Result | Evidence |
|---|---|---|
| CMake configure, Release | VERIFIED / PASS | `cmake -S . -B build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Release` |
| Native build | VERIFIED / PASS | four test executables built |
| CTest | VERIFIED / PASS | 4/4 tests passed |
| Repository validator | VERIFIED / PASS | `repository structure: PASS` |
| Provenance validator | VERIFIED / PASS | `provenance validation: PASS` |
| Python validator tests | VERIFIED / PASS | 2/2 tests passed |
| ASan + UBSan configure/build | VERIFIED / PASS | GCC sanitizer build completed |
| ASan + UBSan CTest | VERIFIED / PASS | 4/4 tests passed |
| ARM/libDaisy build | NOT_RUN | no target toolchain/dependency checkout executed in this bootstrap runtime |
| Daisy Field hardware | NOT_RUN | no physical hardware attached to this runtime |
| GitHub Actions | NOT_RUN | repository remains empty and nothing was pushed by this task |

## Native tests executed

- `test_pattern16`
- `test_step_sequencer`
- `test_sample_clock`
- `test_control_pickup`

The pattern test includes a deterministic 20,000-operation state stress sequence. The sample-clock test checks 1,024 non-integer-BPM step events against ideal sample-domain timing and requires each event to remain within one sample.

## Scope of claim

These results verify the generated R0 host core and repository automation in this runtime. They do not verify ARM behavior, libDaisy integration, callback timing on STM32H7, or any analog/electrical/audio performance of Daisy Field.
