# Foundation review
## Verdict
R0 is a useful host-test bootstrap, not a hardware instrument or a target-qualified build.
Its existing tests miss numeric and runtime integration failures. Preserve its research;
repair those gaps before using it as a firmware foundation.

| ID | Evidence in supplied R0 | Concrete failure | Resolution in this candidate |
|---|---|---|---|
| F01 | SampleAccurateStepClock::SetTempoBpm unconditionally calls Reset | Repeated tempo updates can indefinitely postpone the next step | Retune preserves phase; add repeated-retune regression |
| F02 | Clock constructor accepts zero sample rate/zero step division; tiny BPM rounds to zero after mutation | Zero-threshold clock emits constantly or zero increment stalls | Validate construction, safe fallback and transactional setters |
| F03 | ControlPickup::Clamp uses std::clamp directly | NaN input can poison the held value and defeat capture comparisons | Ignore nonfinite physical updates; sanitize initialization |
| F04 | HPF_CHECK_NEAR only compares fabs(error)>tol | NaN comparison is false, so an invalid numerical result passes | Explicit finite/tolerance checks and negative self-test |
| F05 | Python tests run validators only on a valid repository | Removing a required file or corrupting provenance is not tested | Negative fixture tests for required files, presets and evidence |
| F06 | R0 has no firmware source, only field_truth/README.md | Host CI cannot detect a target API or linker failure | Two firmware entry points, explicit target-build script and separate gates |
| F07 | No UI/audio ownership contract exists in executable code | Concurrent display/MIDI edits could race DSP and corrupt state | Audio owns engine/UI; fixed-capacity one-producer/one-consumer snapshots and MIDI commands |
| F08 | No sound engine or rendered scenarios | Passing integer tests says nothing about runaway audio, tails or transitions | Deterministic DSP renderer; finite/bounds/decay/transition/no-allocation tests |
| F09 | Manifest validation checks only SHA syntax and equality | A well-formed but unfetched commit looks reproducible | Check actual git HEAD and submodule state in target preflight |
| F10 | P0 physical key order unmeasured | A diagram could label the wrong physical key or LED | One explicit mapping table and a bench acceptance gate; no measured-label claim |

## Integration judgment
Placing the project under field/HydraPulse improves compatibility with the two-level relative
library layout documented by upstream DaisyExamples. It does not grant tools access and does
not establish that Denys's fork contains a particular QA framework. That fork was not readable
through this run's public web route. No claims about its current tree are made.

libDaisy's inspected tests/CMakeLists.txt actually selects a limited set of sources with UNIT_TEST
and GoogleTest; it is not a simulator of the whole Daisy Field. CpuLoadMeter exists and measures
callback-body time with System ticks. The generic Makefile defaults to GNU++14 and BOOT_NONE:
both facts require explicit attention for a C++17 project and flash-size verification.

## What was not reproduced
No root license was selected. No commercial manual was copied into the repository.
The original APE_DRAFT ZIP and two .skill files are absent from this runtime: their contents
are not reconstructed from the handoff. The available 29-file R0 ZIP is preserved byte-for-byte.
No current remote merge, ARM build, DFU write, or physical measurement was performed here.

## Design versus proof
Soft ramps, finite envelopes and the absence of a resonant feedback path reduce specific failure
mechanisms. They are not a proof of click-free analog output or inaudible aliasing. Pulse timing,
codec behavior, boot transients, noise floor and analog levels remain measured-hardware gates.

## Implementation review and repaired deltas

F11. The first Arc candidate reset modulation-envelope depth at an active retrigger.
A matched-trajectory host test measured 1.42942 full-scale first-sample deviation.
Using a 1-ms envelope rise from the current depth reduced the tested maximum to
0.0320582. Noise-filter coefficient updates also became 5-ms ramps; the corresponding
parameter-event deviation fell from 0.210695 to 0.0263815. These are tested scenarios,
not a universal perceptual click guarantee. `test_transitions` guards the repairs.

F12. Repeated Stop could repeatedly restart a release ramp and postpone silence.
Envelope Release now ignores an already active zero-target release. The repeated-release
regression reaches exact silence within its fixed test window.

F13. Long telemetry records can exceed libDaisy's documented `LOGGER_BUFFER=128`.
Boot, output, timing and queue records are now split, with bounded string widths.
A conservative format-width test covers full 32-bit numeric values including CR/LF/NUL.
Source: https://electro-smith.github.io/libDaisy/logger_8h_source.html

F14. The upstream core Makefile's OpenOCD `program` recipe hardcodes `./build/`,
unlike `program-dfu`, which uses `BUILD_DIR`. This project uses separate build-truth
and build-beat directories. The integration guide therefore uses the verified
DFU recipe only after a boot-layout decision, never the incompatible `make program`.
Source: https://raw.githubusercontent.com/electro-smith/libDaisy/master/core/Makefile

F15. Showing only the Shift legend hid tutorial selection while its modifier was held.
The Shift display now keeps the pending tutorial name and load action visible.
No persistent nested menu was introduced.

F16. The inspected upstream application core Makefile adds `-fshort-enums`, while the
inspected library Makefile does not. Cross-library structures containing public enums
must not depend on inconsistent compiler layouts. This project explicitly uses
default-width enums in both builds, appends `-fno-short-enums` after the upstream
application rules, and asserts the public MIDI enum width. The ARM build remains
required to verify the complete ABI; source inspection alone does not establish it.
