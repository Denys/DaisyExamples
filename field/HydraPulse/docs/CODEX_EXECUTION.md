# Build, qualify and operate HydraPulse in DaisyExamples

This integration uses the existing root libDaisy gitlink. Do not repin the shared
submodules or rebuild a dirty/shared dependency tree with qualification flags.
Use a clean isolated checkout of DaisyExamples; preserve the standalone repository.
Read the repository and project AGENTS.md and `INTEGRATION_2026-09-08.md` first.

## Host qualification

Run from `field/HydraPulse` with CMake, GCC/Clang and Python 3.10+:

```sh
python3 -S tools/validate_repo.py
python3 -S tools/generate_presets.py --check
python3 -S -m unittest discover -s tests/python -v
cmake -S . -B build-host -DCMAKE_BUILD_TYPE=Release
cmake --build build-host --parallel 4
ctest --test-dir build-host --output-on-failure
cmake -S . -B build-ubsan -DCMAKE_BUILD_TYPE=Debug -DHPF_UBSAN_ONLY=ON
cmake --build build-ubsan --parallel 4
ctest --test-dir build-ubsan --output-on-failure
CXX=clang++ cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DHPF_ENABLE_SANITIZERS=ON
cmake --build build-sanitize --parallel 4
ctest --test-dir build-sanitize --output-on-failure
./build-host/render_scenarios artifacts/renders-a
./build-host/render_scenarios artifacts/renders-b
python3 -S tools/validate_audio.py artifacts/renders-a --repeat artifacts/renders-b
```

Clang needs its platform ASan runtime (Ubuntu 24.04: `libclang-rt-18-dev`). A runtime
startup/link failure is not a sanitizer pass. The adapter stub checks source/API shapes;
it cannot establish actual BSP compatibility, interrupt behavior or physical results.
Use clang-format 10 on active C++; preserve `references/r0` byte for byte and regenerate
`Presets.generated.h` through the generator. QAE warnings in host test utilities are
not allocations in firmware; inspect firmware warnings against the adapter startup.

## ARM qualification

Inspect root `libDaisy` HEAD, status and recursive gitlinks. In a fresh checkout,
initialize only root libDaisy and its recorded `tests/googletest` submodule. This pin's
upstream host tests use `libDaisy/tests/Makefile`, not the newer CMake test layout.

```sh
python3 -S tools/verify_dependencies.py ../../libDaisy
python3 -S tools/build_target.py --libdaisy ../../libDaisy --app-type BOOT_NONE --jobs 4
```

The helper force-rebuilds the library and two independent images, records tool versions,
command lines, ELF/BIN/MAP, section sizes, hashes, and source/dependency identity.
Truth uses `build-truth/HydraPulseTruth.*`; the instrument uses
`build-beat/HydraPulseBeat.*`. No DaisySP is required. Keep default-width BSP enums,
C++17, finite-math checks, and the 128 KiB internal-flash size gate. Never substitute
host executables or bypass this gate by guessing a bootloader layout.

## Flash and physical validation

No CI workflow flashes hardware. Flash only when the user has authorized it, after
identifying the actual device and matching the image to its successful manifest.
For BOOT_NONE, internal flash starts at 0x08000000. Preserve a recovery image before
replacing the current application. Other boot layouts require a separate device/layout
decision. Preview `make -n APP=truth program` and verify its APP-specific ELF path.
The intentional local `program` recipe override replaces the pinned upstream hardcoded
`build/` path; GNU make reports an override warning. `make APP=truth program` is the
explicit diagnostic flash action. Use `APP=beat` only after the diagnostic gate.

Run Truth first. Record boot telemetry, OLED, 16 raw key indices, eight knobs, callback
load/overruns, safe CV/Gate state, low-level stereo passthrough, clipping and silence.
Do not flash the instrument until the physical diagnostic smoke test succeeds.
Then load First Pulse, test Play/Stop and decay, repeated transport, all four voice
parameters, tempo/steps/accent/voice/A-B/Fill/mutes/Panic in mono before enabling stereo.
Retain actual operator observations and audio captures; host/ARM/flash verification
cannot prove click-free analog sound or a complete P0 electrical qualification.

## Publication and continuity

Stage only HydraPulse and its narrowly necessary CI changes. Preserve the original
candidate manifest as provenance, record every delta, and keep historical package
results separate from current qualification. Publish through a normal branch/PR,
verify actual CI runs and merge only passing reviewed changes. No force push or
standalone repository changes. Report exact final commit, dependency, tests, artifacts,
CI and hardware status. Hardware without retained observations remains NOT_RUN.
