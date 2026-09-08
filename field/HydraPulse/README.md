# HydraPulse Field | 0.2.0 candidate

A direct, four-voice synthesized drum machine for stock Electro-Smith Daisy Field.
**Select a drum → tap 16 steps → edit the sound → Play.**

This subtree is designed for `Denys/DaisyExamples/field/HydraPulse`. It is an additive
development candidate, not a hardware-qualified release. The original 29-file R0 package
is preserved without changes under `references/r0`; the old dedicated repository is not deleted.

## What is implemented

Two separate firmware sources share a host-testable foundation:

| Image | Purpose | Source |
|---|---|---|
| HydraPulseTruth | Stereo passthrough, raw key events, knobs/CV/Gate/MIDI telemetry, momentary output tests, OLED/LED diagnostics and callback instrumentation | `firmware/FieldTruth.cpp` |
| HydraPulseBeat | Hammer kick, Crack snare, Steel metallic hat, Arc phase-modulated percussion; 16-step A/B/Fill banks, accents, mutes, swing, drive, soft pickup, stop/panic and eight original tutorials | `firmware/HydraPulse.cpp` |

One Home screen and a momentary Shift palette replace a nested menu tree. All state
changes that affect sound are owned by the audio callback. Display and telemetry are
foreground-only. The DSP uses original bounded algorithms and no DaisySP runtime dependency.

Current integration evidence is recorded in [the integration log](docs/INTEGRATION_2026-09-08.md).
The supplied package's historical results remain in [candidate validation](docs/VALIDATION.md).
A software or flash verification result does not qualify physical controls or audio.

## Start with the host build

Requirements: CMake 3.20+, a C++17 compiler, Python 3.10+, and Git. The scripts use only
Python's standard library. Linux/WSL is the reference command environment.

```sh
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host --parallel 2
ctest --test-dir build-host --output-on-failure
python3 -S -m unittest discover -s tests/python -v
python3 -S tools/validate_repo.py
python3 -S tools/generate_presets.py --check
./build-host/render_scenarios renders/first
./build-host/render_scenarios renders/repeat
python3 -S tools/validate_audio.py renders/first --repeat renders/repeat
```

The host adapter object builds use an explicit stub. They catch C++ integration mistakes,
not compatibility with an actual libDaisy revision, interrupt behavior or electrical behavior.

## Build the two target images

Use DaisyExamples' existing root libDaisy at `../../libDaisy`, verified against
`deps.lock.json`. Run qualification in a clean isolated checkout; the helper rebuilds
the library with explicit qualification flags. See [Codex execution](docs/CODEX_EXECUTION.md).

```sh
python3 -S tools/verify_dependencies.py ../../libDaisy
python3 -S tools/build_target.py --libdaisy ../../libDaisy
```

The default build is `BOOT_NONE`. Successful builds must fit the project's 128 KiB
internal-flash qualification policy. Oversized images fail; changing boot layout requires
a deliberate bootloader decision. `BOOT_SRAM` and `BOOT_QSPI` are explicit build options,
not assumptions about the board in front of you. The build tool never flashes hardware.

Do not use a host executable as a firmware image. Do not use an old successful CI result
for the new source. The build helper records its transcript during the build and emits a successful
qualification manifest with source/image hashes, tool versions and dependency gitlinks
only after both real ARM images pass its checks.

## Development and playing references

- [Detailed plan](docs/00_EXECUTION_PLAN.md) and [foundational findings](docs/01_FOUNDATION_REVIEW.md).
- [Eight-file Mermaid system reference](docs/system/README.md), covering ownership,
  algorithms, timing, controls, hardware, presets and evidence gates.
- [User manual](docs/USER_MANUAL.md), [tutorial presets](docs/PRESETS.md) and
  [quick reference](docs/QUICK_REFERENCE.md).
- [Automated and bench QA](docs/QA_PLAN.md), [Codex integration](docs/CODEX_EXECUTION.md),
  [DVPE decision](docs/DVPE_INTEGRATION.md).
- [Source ledger](third_party/manifest.json), [dependency pin](deps.lock.json) and
  [R0 preservation inventory](references/r0_manifest.json).

## Deliberately not implemented

No persistent user save, sample engine, 64-step paging, deep parameter locks, external
clock following, external audio lane, USB MIDI transport or Hydrasynth-specific
integration. Beat's CV and Gate outputs are commanded off and its audio inputs are ignored.
Those ports are exercised by Field Truth first. A/B/Fill is present as a candidate;
P0 hardware acceptance remains open.

P0 must establish controls, audio, CV/Gate, MIDI and timing on the actual Field. Only then
promote Beat to the P1 hardware trial. Longer patterns, more voices and two-box integration
follow measured standalone performance, not the number of folders in this repository.

## License and references

The owner's root license choice remains unselected; this package does not invent one.
See `LICENSE_STATUS.md`. Commercial manuals, their diagrams and factory patterns are
not redistributed. Their instructional approaches informed the original manual and
tutorials, not the implementation of their products.
