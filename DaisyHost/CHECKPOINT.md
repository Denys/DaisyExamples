# DaisyHost Checkpoint

## Snapshot

- Date: 2026-04-25
- Workspace: `DaisyHost/`
- Current CMake version in source: `0.2.0`
- Active refresh target: `0.2.0`
- Scope: host-side Daisy Patch plugin and standalone app with a multi-app host
  layer, `MultiDelay` as the default regression fixture, `Torus` as the first
  second app, first-class `CloudSeed`, `Braids`, `Harmoniqs`, and `VA Synth`
  support, named MetaControllers for `multidelay` and `cloudseed`, plus a
  visible two-node live rack, board factory seam, Field board-support shell,
  host-side Field native controls, Field extended host surface support,
  forward/reverse two-node audio-chain proof paths, and the first
  flash-verified Daisy Field firmware adapter under `field/MultiDelay`

This checkpoint records the current source-backed state plus the last recorded
runtime verification. `PROJECT_TRACKER.md` is now the active ordered roadmap
and per-iteration testing ledger for DaisyHost.

## Companion Tracking Files

- `PROJECT_TRACKER.md`: current ordered roadmap, per-iteration testing ledger,
  and concurrent-thread coordination
- `SKILL_PLAYBOOK.md`: dedicated skill-selection, `Expected UF` /
  `Observed UF`, evidence, and validation log
- `CHANGELOG.md`: canonical durable release and workflow policy history
- `README.md`: workspace overview and local-doc entrypoint

## DaisyBrain NotebookLM Notebook

Curated repo knowledge notebook created for Daisy and DaisyExamples
architecture questions, planning, and design recall:

- Notebook title: `DaisyBrain`
- Notebook id: `8084395c-2d50-464c-967b-7569926fe771`
- Preferred local CLI home: `C:\Users\denko\.notebooklm-daisybrain`

Current seeded sources:

- `docs/notebooklm/DaisyBrain-knowledge-pack.txt`
- repo `AGENTS.md`
- `LATEST_PROJECTS.md`
- `DaisyHost/README.md`
- `DaisyHost/CHECKPOINT.md`
- `DaisyHost/CHANGELOG.md`
- `docs/plans/2026-04-18-daisyhost-book-roadmap.md`
- `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

CLI-first usage notes:

```sh
$env:NOTEBOOKLM_HOME = "$HOME\.notebooklm-daisybrain"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" ask -n 8084395c-2d50-464c-967b-7569926fe771 "question"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" source list -n 8084395c-2d50-464c-967b-7569926fe771 --json
```

Operational note:

- Prefer the NotebookLM CLI over MCP for this notebook when local automation is
  needed.
- Use the notebook for architectural recall, DaisyHost roadmap questions,
  Daisy-vs-host tradeoffs, and future book-feature planning.

## Current Build Commands

Preferred local entrypoint from `DaisyHost/`:

```sh
.\build_host.cmd
```

That wrapper launches `build_host.ps1`, normalizes the `Path` / `PATH`
environment split for MSBuild-backed commands in this shell, injects a fresh
`DAISYHOST_UNIT_TEST_RUN_TAG` for the Release unit-test payload path, and
reruns the full host gate.

Underlying raw commands:

```sh
cmake -S . -B build
cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir build -C Release --output-on-failure
```

`ctest` now includes:

- `unit_tests`
- `DaisyHostStandaloneSmoke`
- `DaisyHostRenderSmoke`

Current shell note:

- the rebuilt `unit_tests` target is now emitted as a Release payload under
  `build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
- `ctest` launches unit cases through `tests/run_unit_test_payload.py`, which
  receives that payload path and attempts to run a fresh temporary copy
- the wrapper-driven full host gate reran green on 2026-04-25, so the current
  baseline in this checkout is again a fully green Release host gate rather
  than only partial smoke/debug proof
- after local Debug rebuilds, direct `ctest -C Debug -R ...` can still point at
  a stale unbuilt run-tag path in this checkout; when that happens, the current
  safe targeted fallback is direct execution through
  `py -3 tests/run_unit_test_payload.py <fresh-payload> --gtest_filter=...`
- `tests/run_smoke.py` now uses a wider process-query timeout for standalone
  smoke so slower Windows process-path discovery does not produce a false
  timeout on an otherwise healthy launch
- result: the rack freeze gate is no longer blocked in this checkout, the
  Daisy Field board-support shell, host-side Field native controls, and Field
  extended host surface support are implemented, the latest Field ergonomics
  polish and board-generic UI cleanup gate passed `159/159`, and the first
  Field firmware adapter (`field/MultiDelay`) is build-verified and
  ST-Link flash-verified; its manual audio/control/CV checklist is still
  pending

Rebuild the Patch firmware reference targets only when DaisyHost shared cores or
firmware adapters change:

```sh
make
```

from `patch/MultiDelay/`.

and:

```sh
make
```

from `patch/Torus/`.

## Last Recorded Runtime Verification

- Last fully green DaisyHost host build/test verification rerun from this
  checkout: 2026-04-25
- Verified commands/results in the current 2026-04-25 Sprint F3 Field
  hardware/firmware parity adapter pass:
  - manager-readable result: DaisyHost now has its first hardware-facing Daisy
    Field firmware adapter, `field/MultiDelay`. This uses the same shared
    `MultiDelayCore` regression fixture as the host and Patch firmware adapter,
    so Field hardware proof can begin without changing the DaisyHost rack,
    session, render, or DAW behavior.
  - implementation scope:
    - `field/MultiDelay/Makefile`
    - `field/MultiDelay/MultiDelay.cpp`
    - `field/MultiDelay/README.md`
    - `field/MultiDelay/CONTROLS.md`
  - red proof:
    - `cd ..\field\MultiDelay; make` first failed as expected because
      `MultiDelay.cpp` did not exist yet:
      `No rule to make target 'build/MultiDelay.o'`
  - firmware build notes:
    - first full link with default optimization overflowed Field flash by
      `7800 bytes`
    - adding `OPT = -Os` in `field/MultiDelay/Makefile` reduced firmware size
      enough for Daisy Field flash
  - `cd ..\field\MultiDelay; make`: passed
  - `$env:PYTHONIOENCODING='utf-8'; py -3 ..\..\DAISY_QAE\validate_daisy_code.py .`:
    passed, `0 error(s), 0 warning(s)`
  - `cd ..\field\MultiDelay; make program`: passed; OpenOCD detected STLINK
    `V3J7M2`, target voltage `3.271571`, programmed the STM32H7 target, and
    reported `** Verified OK **`
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(MultiDelayCoreTest|BoardControlMappingTest|RenderRuntimeTest)"`:
    passed, `52/52`
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
      passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed,
      `159/159`
  - caveats:
    - ST-Link flash/verify is proven, but full manual Field hardware
      validation is not yet claimed
    - remaining manual checklist: audio input 1 to outputs 1/2 with delay,
      K1-K5 behavior, K6-K8 no unexpected DSP effect, CV1 tertiary-delay
      input, CV OUT 1 `0..5V`, CV OUT 2 `0V`, SW1 impulse, SW2 OLED status
      page toggle, LEDs/OLED stability, and no audio dropouts
    - no DaisyHost host runtime change, mixed-board rack behavior, routing
      change, DAW/VST3 validation, or DAW automation expansion belongs to this
      pass
- Verified commands/results in the current 2026-04-25 Field ergonomics polish
  and board-generic UI cleanup full-gate pass:
  - manager-readable result: DaisyHost Field now behaves more intentionally
    without opening new architecture. Field K5-K8 ranked extras avoid
    duplicating the real K1-K4 parameter targets where hosted apps expose that
    metadata, and the Field editor uses board-profile target metadata for
    knobs, keys, and switches instead of Field-only string construction. This
    improves operator value and future board-support maintainability while
    keeping `daisy_patch` as the default and the rack topology frozen.
  - red proof:
    - `cmake --build build --config Debug --target unit_tests` first failed on
      missing `HostedAppPatchBindings::knobParameterIds`
    - `ctest --test-dir build -C Debug --output-on-failure -R "BoardProfileTest"`
      failed `10/11` because Field surface controls did not yet expose target
      ids usable by generic editor lookup
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|RenderRuntimeTest|HostSessionStateTest|LiveRackTopologyTest)"`: passed, `70/70`
  - raw `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`:
    first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Release target build:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`:
    passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including Field native controls, Field extended surface, and selected-node Field surface scenarios
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `159/159`
  - caveats:
    - manual DAW/VST3 validation remains deferred
    - this pass does not add Field firmware, real hardware voltage output,
      mixed-board racks, routing changes, or DAW automation expansion
- Verified commands/results in the current 2026-04-25 knob-drag crash fix
  full-gate pass:
  - manager-readable result: DaisyHost fixed a board/app-independent crash
    reported while moving knobs. The root cause was shared hosted-app core
    state being accessed from both the audio thread and message-thread
    control/UI refresh paths during live knob drags. The fix serializes hosted
    core access and selected-node/core pointer handoff, so the same protection
    applies to all hosted apps on both `daisy_patch` and `daisy_field`.
  - implementation scope:
    `src/juce/DaisyHostPluginProcessor.h` and
    `src/juce/DaisyHostPluginProcessor.cpp`
  - `cmake --build build --config Debug --target unit_tests`: passed
  - raw `cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Debug standalone build rerun:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Debug --target DaisyHostPatch_Standalone`:
    passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `66/66`
  - visible Debug standalone validation:
    - launched `DaisyHost Patch.exe --board daisy_field --app multidelay`,
      dragged Field knobs, and confirmed the process remained alive/responding
    - launched `DaisyHost Patch.exe --board daisy_patch --app multidelay`,
      dragged Patch knobs, and confirmed the process remained alive/responding
  - sanitized Release target build:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`:
    passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `155/155`
  - caveats:
    - no automated red regression test was added because the crash is a JUCE
      audio/message-thread race outside the current unit-test harness
    - DAW/VST3 manual knob-drag validation remains deferred
    - this fix does not change `daisy_patch` default behavior, rack topology,
      Field firmware status, or real hardware validation status
- Verified commands/results in the current 2026-04-25 Field refinement closeout
  full-gate pass:
  - manager-readable result: DaisyHost now proves Field selection and selected
    node behavior more clearly. Hub `Play / Test` launch planning writes the
    supported startup request, Field render/smoke includes selected-node
    surface evidence for `node1`, and the Field UI copy no longer describes
    Field controls as Patch controls.
  - red proof:
    `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"`
    first failed because Hub play plans still used command-line args only and
    Field surface manifests still reported `node0` for a selected `node1`
    scenario
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(HubSupportTest|RenderRuntimeTest)"`: passed, `39/39`
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardProfileTest|BoardControlMappingTest|HubSupportTest|HostSessionStateTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `76/76`
  - raw `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_Standalone`: first hit the known duplicate `Path` / `PATH` MSBuild issue in this shell
  - sanitized Release build rerun:
    `$env:Path = $env:PATH; Remove-Item Env:PATH -ErrorAction SilentlyContinue; cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_Standalone`: passed
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including the new `training/examples/field_node_target_surface_smoke.json`
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `155/155`
  - this rerun verifies host-side Field selection/startup-request planning and
    selected-node Field surface evidence; it does not verify Field firmware,
    real hardware voltage output, mixed-board racks, or manual DAW/VST3 loading
- Verified commands/results in the current 2026-04-25 Field extended host
  surface full-gate pass:
  - manager-readable result: DaisyHost now automatically tests `daisy_field`
    as a functioning host-side Field board surface. This closes the desktop
    model gap between "Field can be selected and played" and "Field exposes
    repeatable host-side outputs/switches/LEDs" while keeping the rack frozen
    and `daisy_patch` as the default.
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|BoardProfileTest|HostSessionStateTest|EffectiveHostStateSnapshotTest|RenderRuntimeTest|LiveRackTopologyTest)"`: passed, `68/68`
  - `cmake --build build --config Release --target DaisyHostRender DaisyHostPatch_Standalone`: passed after normalizing the shell-local duplicate `Path` / `PATH` environment through a Python subprocess wrapper
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed for `daisy_patch` and `daisy_field`
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed, including the Field shell, Field native controls, and Field extended surface scenarios
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `152/152`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the render smoke harness now covers the Field board-support shell scenario,
    the Field native controls scenario, and
    `training/examples/field_extended_surface_smoke.json`
  - this rerun verifies host-side Field outputs/switches/LEDs from this
    checkout; it does not verify Field firmware, real hardware voltage output,
    mixed-board racks, or manual DAW/VST3 loading
- Verified commands/results in the earlier 2026-04-25 full-gate pass:
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `144/144`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the render smoke harness now covers the Field board-support shell scenario
    and the Field native controls scenario
  - this rerun verifies host-side Field native controls from this checkout
- Verified commands/results in the earlier 2026-04-24 full-gate pass:
  - `cmd /c build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `136/136`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - the aggregate now includes the first-class hosted-app wave through:
    - `multidelay`
    - `torus`
    - `cloudseed`
    - `braids`
    - `harmoniqs`
    - `vasynth`
  - this rerun verified the Daisy Field board-support shell from this checkout
- Verified commands/results in the earlier 2026-04-23 full-gate pass:
  - `.\build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `89/89`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - targeted WS6/WS7 proof reruns included:
    - `cmake --build build --config Release --target unit_tests`: passed
    - `ctest --test-dir build -C Release --output-on-failure -R "(MultiDelayCoreTest|CloudSeedCoreTest|HostSessionStateTest|RenderRuntimeTest|EffectiveHostStateSnapshotTest)"`: passed, `44/44`
- Later 2026-04-23 visible-rack sprint verification from this same checkout:
  - `cmake -S . -B build`: passed
  - `cmake --build build --config Release --target unit_tests`: passed
  - `cmake --build build --config Release --target DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
  - `ctest --test-dir build -C Release --output-on-failure -R "DaisyHost(Standalone|Render)Smoke"`: passed, `2/2`
  - `py -3 tests/run_smoke.py --mode standalone --build-dir build --source-dir . --config Release`: passed
  - `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release`: passed
  - direct `DaisyHostRender.exe` runs against temporary forward and reverse
    two-node rack scenarios: passed, with manifests recording the expected
    `boardId`, `selectedNodeId`, `entryNodeId`, `outputNodeId`, `nodes[]`, and
    `routes[]`
  - later same-day Braids hosted-app verification added:
    - `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release`: passed
    - checked-in `braids_smoke.json` render path: passed as part of the render smoke run
  - blocker observed in that pass: `DaisyHostUnitTestsRunner.exe` could
    disappear or become
    externally locked after link in this shell, so the rebuilt unit-test binary
    cannot currently be executed and the full host gate has not yet been rerun
    against the final visible-rack plus Braids code
- Resumed 2026-04-24 Braids verification from this same checkout:
  - `cmd /c build_host.cmd`: configured and rebuilt the Release app target set,
    then failed during full `ctest -C Release` because CTest could not launch
    the Release `DaisyHostUnitTestsRunner.exe` path
  - `cmake --build build --config Debug --target unit_tests`: passed
  - `ctest --test-dir build -C Debug --output-on-failure -R "(AppRegistryTest|BraidsCoreTest|DaisyBraidsCoreTest|RenderRuntimeTest)"`: passed, `27/27`
  - `ctest --test-dir build -C Debug --output-on-failure -E "DaisyHost(Standalone|Render)Smoke"`: passed, `110/110`
  - `py -3 tests/run_smoke.py --mode all --build-dir build --source-dir . --config Release --timeout-seconds 120`: passed
  - blocker observed in that pass: full Release `ctest` could not launch the
    Release unit-test runner in that shell, so that was not a fully green
    Release host gate
- Later 2026-04-24 rack-freeze resume diagnostics from this same checkout:
  - `.\build_host.cmd`: first exposed and then confirmed the fixed
    `build_host.ps1` PATH-normalization path; the wrapper now reaches the real
    configure/build/test loop
  - direct `probe.exe` creation in `build/test_bin/Release`: passed
  - safe cleanup of `build/test_bin/Release` and `build/unit_tests.dir/Release`:
    passed
  - `.\build_host.cmd`: rebuilt the full Release target set again, including
    `unit_tests`, then failed in `ctest -C Release` because the Release runner
    remained `resource busy or locked`
  - direct launch of the Release runner with a single-test filter: failed
    because the file was in use by another process
  - copied `DaisyHostRender.exe` executed successfully from
    `build/test_bin/Release`, isolating the issue to the Release unit-test
    runner rather than the output directory
  - attempted Defender exclusion for
    `DaisyHost/build/test_bin/Release`: failed due insufficient permissions
  - result: the canonical Release host gate is still not green, so the Daisy
    Field readiness line has not been reached
- Later 2026-04-24 wrapper-mitigation follow-up from this same checkout:
  - exact-name write probes proved `build/test_bin/Release` was not generally
    broken: `DaisyHostUnitTestsRunner.exe` was denied, while nearby names such
    as `DaisyHostUnitTests.exe` and `unit_tests.exe` were writable
  - scratch-directory checks proved the same friendly unit-test name was
    writable and runnable outside the original path
  - `build_host.ps1` now passes a fresh
    `-DDAISYHOST_UNIT_TEST_RUN_TAG=<timestamp>` into configure so each wrapper
    run uses a new Release payload path
  - `unit_tests` now builds as
    `build/unit_test_bin/<run-tag>/Release/DaisyHostTestPayload.bin`, and
    `ctest` launches it through `tests/run_unit_test_payload.py`
  - `cmd /c build_host.cmd` rebuilt the full Release target set again and then
    ran `ctest -C Release`; `DaisyHostStandaloneSmoke` and
    `DaisyHostRenderSmoke` both passed, but the remaining `110/112` unit tests
    failed because the launcher got `PermissionError` when reading the freshly
    linked Release payload
  - attempted Defender exclusion for `build/unit_test_bin` and
    `build/unit_test_run`: failed due insufficient permissions
  - result: the rack freeze gate is still blocked by a machine-level read lock
    on the freshly linked Release payload itself, and Daisy Field
    implementation has still not started
- Later 2026-04-24 Harmoniqs + VA Synth hosted-app verification from this same
  checkout:
  - red step:
    `cmake --build build --config Debug --target unit_tests` first failed after
    the new tests/CMake wiring because the `DaisyHarmoniqsCore`,
    `HarmoniqsCore`, `DaisyVASynthCore`, and `VASynthCore` surfaces did not yet
    exist
  - green targeted proof:
    `cmake --build build --config Debug --target unit_tests`: passed
  - direct Debug payload proof:
    `py -3 tests/run_unit_test_payload.py <fresh Debug payload> --gtest_filter="AppRegistryTest.*:DaisyHarmoniqsCoreTest.*:HarmoniqsCoreTest.*:DaisyVASynthCoreTest.*:VASynthCoreTest.*:RenderRuntimeTest.ProducesDeterministicOfflineRenderForHarmoniqs:RenderRuntimeTest.ProducesDeterministicOfflineRenderForVASynth"`:
    passed, `18/18`
  - current Release render-path proof:
    `cmake --build build --config Release --target DaisyHostRender`: passed
  - current Release render smoke:
    `py -3 tests/run_smoke.py --mode render --build-dir build --source-dir . --config Release --timeout-seconds 120`:
    passed, now covering `multidelay`, `torus`, `cloudseed`, `braids`,
    `harmoniqs`, and `vasynth`
- Firmware note: no `make` rerun was required in this 2026-04-23 pass because
  the WS6/WS7 package stayed host-side; the last recorded firmware reference
  reruns remain the successful 2026-04-22 `patch/MultiDelay/` and `patch/Torus/`
  builds.
- Historical note: the prior full runtime verification recorded in local docs
  before this rerun was `2026-04-19`
- In this Codex shell, raw MSBuild commands still require a sanitized one-path
  environment because both `Path` and `PATH` are exported by default, but
  `build_host.cmd` now encapsulates that repair.

## Last Recorded Artifact Paths

The following paths were verified from this checkout on 2026-04-23:

- Launcher hub:
  - `DaisyHost/build/DaisyHostHub_artefacts/Release/DaisyHost Hub.exe`
- Render CLI:
  - `DaisyHost/build/Release/DaisyHostRender.exe`
- VST3 bundle:
  - `DaisyHost/build/DaisyHostPatch_artefacts/Release/VST3/DaisyHost Patch.vst3`
- Standalone app:
  - `DaisyHost/build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe`
- Host test target:
  - `unit_tests`
  - executable payload currently built as:
    `DaisyHost/build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
  - `ctest` currently launches unit cases through
    `tests/run_unit_test_payload.py`
  - last fully green `ctest` aggregate is now the 2026-04-25
    `155/155` pass, including `DaisyHostStandaloneSmoke` and
    `DaisyHostRenderSmoke`

## Current Hosted Apps

- default app id: `multidelay`
- additional app id: `torus`
- additional app id: `cloudseed`
- additional app id: `braids`
- additional app id: `harmoniqs`
- additional app id: `vasynth`

Current Phase 2 host behavior:

- DaisyHost instantiates hosted apps by stable `appId`
- the selected app persists in `HostSessionState`
- the processor/editor bind to the active app's metadata and patch bindings
- the processor now exposes a fixed five-slot DAW automation bank with stable
  ids `daisyhost.slot1` .. `daisyhost.slot5`
- those five slots rebind to the active app's top-ranked automatable canonical
  parameters without changing the saved slot ids
- `multidelay` and `cloudseed` now expose named MetaControllers through the
  shared menu/drawer path while canonical parameter ids remain the only saved
  truth
- session persistence remains canonical by app parameter id rather than DAW slot
  id
- `GetEffectiveHostStateSnapshot()` now exposes live canonical parameters,
  mapped automation slots, selected-node identity, node count, board/topology
  fields, active-node MetaControllers, node summaries, routes, CV state, gate
  state, and current audio-input state for processor/UI/debug tooling
- the live plugin and standalone app now run exactly two hosted nodes
  (`node0`, `node1`) with selected-node-targeted Patch controls, drawer/menu
  actions, CV/gate/test-input state, keyboard MIDI, and five-slot DAW
  automation
- live rack topology is currently limited to four audio-only presets:
  - `node0_only`
  - `node1_only`
  - `node0_to_node1`
  - `node1_to_node0`
- board creation now flows through a board-id-based factory seam with
  `daisy_patch` as the default board and `daisy_field` as a Field
  board-support package
- Field native controls are host-side only:
  - K1-K4 mirror the selected node's current Patch page bindings
  - K5-K8 map to the next four automatable selected-node parameters by
    `importanceRank`, with missing parameters disabled
  - CV1-CV4 and Gate In reuse the existing host CV/gate input paths
  - A1-B8 emit 16 chromatic MIDI notes from the selected node's current
    keyboard octave
- Field extended host surface support is also host-side only:
  - CV OUT 1-2 are derived monitor outputs that mirror the K5/K6 mapped
    parameters as `0..5V`
  - SW1/SW2 are momentary selected-app utility triggers
  - key LEDs, switch LEDs, Gate In LED, and Gate Out LED are derived indicators
  - snapshots and render manifests carry final Field surface evidence for
    automated checks
  - the rack topology, hosted-app DSP behavior, selected-node routing
    semantics, Field firmware, real hardware voltage output, mixed-board racks,
    and manual DAW/VST3 validation remain out of scope
- `HostSessionState` is now v5 with rack globals:
  - `boardId`
  - `selectedNodeId`
  - `entryNodeId`
  - `outputNodeId`

Current `CloudSeed` supported-app behavior:

- hosted app id: `cloudseed`
- shared portable wrapper: `DaisyCloudSeedCore`
- named MetaControllers:
  - `Blend`
  - `Space`
  - `Motion`
  - `Tone`
- performance page model:
  - `Space` = `Mix`, `Size`, `Decay`, `Diffusion`
  - `Motion` = `Pre-Delay`, `Damping`, `Mod Amt`, `Mod Rate`
- encoder/menu sections:
  - `Pages`
  - `Program`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Bypass`
  - `Clear Tails`
  - `Randomize Seeds`
  - `Interpolation`
- DAW automation priority for the first five slots is:
  - `mix`
  - `size`
  - `decay`
  - `diffusion`
  - `pre_delay`

Current `Braids` supported-app behavior:

- hosted app id: `braids`
- shared portable wrapper: `DaisyBraidsCore`
- MIDI-first trigger source with optional `gate_in_1` rising-edge strike alias
- mono synth voice duplicated to stereo outputs 1/2
- no host audio input in v1
- performance page model:
  - `Drum` = `Tune`, `Timbre`, `Color`, `Model`
  - `Finish` = `Resolution`, `Sample Rate`, `Signature`, `Level`
- model subset currently ships:
  - `Kick`
  - `Snare`
  - `Cymbal`
  - `Drum`
  - `Bell`
  - `Filtered Noise`
- encoder/menu sections:
  - `Pages`
  - `Model`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Randomize Model`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `model`
  - `tune`
  - `timbre`
  - `color`
  - `signature`

Current `Harmoniqs` supported-app behavior:

- hosted app id: `harmoniqs`
- shared portable wrapper: `DaisyHarmoniqsCore`
- additive mono synth voice duplicated to stereo outputs 1/2
- MIDI-first trigger source with optional `gate_in_1` retrigger alias
- no host audio input in v1
- performance page model:
  - `Spectrum` = `Brightness`, `Tilt`, `Odd/Even`, `Spread`
  - `Envelope` = `Attack`, `Release`, `Detune`, `Level`
- stateful expert spectrum lanes currently ship:
  - `harmonic_1` through `harmonic_8`
- encoder/menu sections:
  - `Pages`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Init Spectrum`
  - `Randomize Spectrum`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `brightness`
  - `tilt`
  - `spread`
  - `odd_even`
  - `attack`

Current `VA Synth` supported-app behavior:

- hosted app id: `vasynth`
- shared portable wrapper: `DaisyVASynthCore`
- fixed seven-voice polyphonic synth with MIDI-first triggering
- optional `gate_in_1` rising-edge trigger alias
- stereo output on Patch outputs 1/2 with toggleable `Stereo Sim`
- no host audio input in v1
- performance page model:
  - `Osc` = `Mix`, `Detune`, `Osc 1`, `Osc 2`
  - `Filter` = `Cutoff`, `Resonance`, `Env Amt`, `Level`
  - `Motion` = `LFO Rate`, `LFO Amt`, `Attack`, `Release`
- encoder/menu sections:
  - `Pages`
  - `Oscillators`
  - `Utilities`
  - `Info`
- utilities currently expose:
  - `Audition`
  - `Init Patch`
  - `Stereo Sim`
  - `Panic`
- DAW automation priority for the first five slots is:
  - `osc_mix`
  - `detune`
  - `osc1_wave`
  - `osc2_wave`
  - `filter_cutoff`

Current Phase 3 render behavior:

- `DaisyHostRender` loads scenario JSON by `appId`
- each render run resets app state to a fixed seed, restores canonical
  parameters, replays a timeline, and renders offline
- legacy single-app scenarios remain valid, while optional `nodes[]` and
  `routes[]` contracts plus rack-level board/selection/topology fields now
  support forward and reverse two-node audio-chain proofs
- each run writes `audio.wav` plus `manifest.json`
- multi-node manifests now include recorded `boardId`, `selectedNodeId`,
  `entryNodeId`, `outputNodeId`, per-node summaries, and recorded routes while
  the top-level app fields continue mirroring `node0`
- `training/render_dataset.py` expands sweep jobs into multiple run folders and
  writes `dataset_index.json`
- checked-in smoke scenarios now include:
  - `multidelay_smoke.json`
  - `torus_smoke.json`
  - `cloudseed_smoke.json`
  - `braids_smoke.json`
  - `harmoniqs_smoke.json`
  - `vasynth_smoke.json`

Current hub behavior:

- `DaisyHost Hub` is a small front door for:
  - `Play / Test`
  - `Render`
  - `Train`
- v1 board selection currently exposes `daisy_patch` and `daisy_field`
- the hub persists its own launcher profile separately from plugin/session
  state
- `Play / Test` writes a small startup request so the standalone host opens the
  selected app directly

## Final Control Map For The Active Refresh

Agreed Patch control hierarchy for the current `0.2.0` refresh:

- `CTRL 1` = dry/wet mix
- `CTRL 2` = primary delay control
- `CTRL 3` = secondary delay control
- `CTRL 4` = feedback
- `CV 1` = tertiary delay control
- `ENC 1` rotate = menu navigation / value edit
- `ENC 1` push = menu open, enter edit, confirm, or exit edit

For the current `MultiDelayCore`, the delay importance order is:

- primary = old delay 1 target
- secondary = old delay 2 target
- tertiary = old delay 3 target

Shared behavior contract:

- every DSP parameter must also be editable from the OLED menu
- if a knob, `CV 1`, or the menu all target the same parameter, `last touch
  wins`

## Menu And Input Targets For The Refresh

Shared top-level menu structure:

- `Params`
- `Input`
- `MIDI`
- `Tracker`
- `About`

`Params` must expose:

- `Dry/Wet`
- `Delay 1`
- `Delay 2`
- `Feedback`
- `Delay 3`

Input/test-source target behavior for the refresh and renderer:

- standalone default = `Host In`
- alternate sources = `Sine`, `Triangle`, `Square`, `Saw`, `Noise`, `Impulse`

Version/change tracking target behavior:

- `About` page mirrors the current version and recent changelog bullets
- `CHANGELOG.md` is the canonical written release history

## Current Notes

- The last fully green `build_host.cmd` host-gate rerun from this checkout is
  now the 2026-04-25 Field ergonomics polish and board-generic UI cleanup
  `159/159` pass.
- The resumed 2026-04-24 Braids verification pass and the later same-day
  Harmoniqs + VA Synth proof wave are now folded into that fully green Release
  aggregate.
- The standalone smoke harness now tolerates slower Windows process-path
  discovery by using a wider process-query timeout before declaring a false
  launch timeout.
- Workstreams 4 and 5 are now source-backed in this checkout:
  - DaisyHost exposes a fixed five-slot DAW automation bridge that maps stable
    slot ids onto the active app's canonical automatable parameters
  - the processor exposes an in-process effective-state snapshot/readback API
    for parameters, mapped slots, CV, gate, and audio-input state
- Workstream 6 is now source-backed in this checkout:
  - `multidelay` and `cloudseed` both expose named MetaControllers that write
    back into canonical parameters and are surfaced through the mirrored
    menu/drawer path
  - `cloudseed` and `braids` are now treated as first-class supported apps
    rather than only pilots
- Visible Workstream 7 rack runtime is now source-backed, build-backed, and
  smoke-backed in this checkout:
  - `HostSessionState` now serializes `version 5` sessions with rack globals,
    `node` records, and `route` records while keeping legacy single-node
    sessions readable
  - the live plugin/standalone now run exactly two hosted nodes with
    selected-node-targeted controls and four audio-only topology presets
  - the editor now shows a visible rack header with per-node app selectors,
    selected-node highlighting, topology controls, Field board-support shell
    rendering, interactive Field K1-K8 controls, and momentary Field A1-B8 keys
  - `daisy_field` now has host-side Field native controls plus Field extended
    host surface support: derived CV OUT 1-2 monitor values, SW1/SW2 momentary
    utility triggers, and key/switch/gate LEDs in snapshots, render manifests,
    smoke, and the panel UI
  - Field refinement closeout adds Hub startup-request launch planning,
    selected-node Field surface render/smoke evidence, and board-aware
    selected-node mirror copy
  - Field ergonomics/UI cleanup adds explicit Patch-page knob parameter ids for
    hosted apps, makes Field K5-K8 avoid duplicate K1-K4 targets, and lets the
    Field editor resolve knobs, keys, and switches through board-profile target
    metadata
  - Field firmware, real hardware voltage output, Field-specific app
    ergonomics, mixed-board rack behavior, and manual DAW/VST3 validation
    remain deferred
  - board creation now flows through a board-id factory seam instead of
    hardcoded Patch construction in the processor path
  - render scenario and manifest contracts now accept rack globals plus
    `targetNodeId` for multi-node non-ID-scoped events
  - the render runtime now proves forward and reverse two-node audio chains
    while rejecting cross-node CV, gate, and MIDI routes with clear errors
- `tests/run_smoke.py` now provides the direct-entrypoint smoke harness:
  - `DaisyHostStandaloneSmoke` launches `DaisyHost Patch.exe` with
    `--board daisy_patch --app torus`, requires a crash-free 5s warmup window,
    then terminates the process cleanly; the process-path query now has a
    wider timeout budget for slower shells
  - `DaisyHostRenderSmoke` runs `DaisyHostRender.exe` against the checked-in
    `multidelay`, `torus`, `cloudseed`, `braids`, `harmoniqs`, `vasynth`, Field
    board-support shell, Field native controls, Field extended surface, and
    Field selected-node surface scenarios, verifies `audio.wav` plus
    `manifest.json`, checks repeated `multidelay` `audioChecksum`
    determinism, and validates final Field surface evidence for CV OUT, SW,
    LED state, and selected-node Field surface state
- `cloudseed` continues to wrap `third_party/CloudSeedCore` through a portable
  `DaisyCloudSeedCore`, and page changes refresh the JUCE processor's active
  patch bindings so DaisyHost control labels and control ids follow the active
  `Space` / `Motion` page instead of staying stale after menu navigation
- `../third_party/CloudSeedCore/DSP/RandomBuffer.cpp` now has a local MSVC-safe
  dynamic-buffer fix because the upstream VLA form does not compile in this
  Windows C++17 build
- `build_host.cmd` and `build_host.ps1` now provide the preferred local host
  gate entrypoint for this checkout and encapsulate the `Path` / `PATH`
  normalization needed by raw MSBuild commands here.
- `tools/write_vst3_manifest.ps1` now drives VST3 manifest generation from a
  PowerShell helper invocation because the direct MSBuild / `cmd` launch path
  for `juce_vst3_helper.exe` failed in this checkout with a `LoadLibraryW`
  path-resolution error.
- The standalone package uses the compact DaisyHost icon asset through
  `ICON_SMALL`.
- The standalone input-mute warning banner is suppressed in the app UI while
  the underlying host-input mute behavior remains unchanged.
- `patch/MultiDelay` requires a Windows-friendly clean rebuild when stale host
  objects are present: remove `build/` with PowerShell, then run `make`.
- `patch/Torus` also requires a PowerShell `build/` removal for clean rebuilds
  in this Windows shell because `make clean` delegates to `rm`.
- The Torus pilot inside DaisyHost is intentionally a DaisyHost-native wrapper
  with Torus/Rings semantics and menu/control assignment behavior. It does not
  compile the original `patch/Torus` firmware entrypoint or Daisy UI layer into
  the host.
- Phase 3 scenario/dataset examples live under:
  - `training/examples/multidelay_smoke.json`
  - `training/examples/torus_smoke.json`
  - `training/examples/cloudseed_smoke.json`
  - `training/examples/braids_smoke.json`
  - `training/examples/harmoniqs_smoke.json`
  - `training/examples/vasynth_smoke.json`
  - `training/examples/field_cloudseed_shell_smoke.json`
  - `training/examples/field_vasynth_native_controls_smoke.json`
  - `training/examples/field_extended_surface_smoke.json`
  - `training/examples/field_node_target_surface_smoke.json`
  - `training/examples/dataset_job_example.json`
- `WORKSTREAM_TRACKER.md` now holds the dedicated post-WS7 portfolio tracker,
  and the same tracker content is mirrored into `PROJECT_TRACKER.md`.
- Full DAW-side VST3 validation remains intentionally deferred until a
  post-Workstream-7 manual pass; no automated DAW load test is wired into this
  workspace yet.

## Final Verification Gate Before Calling The Plugin Complete

Use the following commands for the next full rerun:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

If shared-core or firmware-facing code changed, also run:

```sh
make
```

from `patch/MultiDelay/` and/or `patch/Torus/` as appropriate.

## Last Recorded Render Checks

The following direct-entrypoint render smoke command was last recorded as
executed on 2026-04-25:

```sh
py -3 DaisyHost/tests/run_smoke.py --mode render --build-dir DaisyHost/build --source-dir DaisyHost --config Release
```

The following historical manual render commands were last recorded as executed
on 2026-04-19:

```sh
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/multidelay_smoke.json --output-dir DaisyHost/training/out/multidelay_smoke
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/torus_smoke.json --output-dir DaisyHost/training/out/torus_smoke
py -3 DaisyHost/training/render_dataset.py DaisyHost/training/examples/dataset_job_example.json
```

## Last Recorded Manual Checks

Automated note:

- direct-entrypoint standalone startup smoke and render smoke were rerun on
  2026-04-23 through `tests/run_smoke.py`
- the remaining checks below are still manual / historical unless separately
  rerun

The following manual checks were last recorded on 2026-04-19:

- standalone launch stability: passed via 5s smoke launch
- VST3 load in a DAW: not verified in that session
- visible standalone icon: not visually rechecked after the final build in that
  session
- `Host In`, `Sine`, `Saw`, `Noise`, and `Impulse`: build/test covered; manual
  audio pass not repeated after the final UI patch
- app selection and app restore:
  - host/session serialization covered by tests
  - no manual GUI click-through for `multidelay` -> `torus` -> restore was run
    in that session
- render CLI:
  - `multidelay` scenario: rendered successfully
  - `torus` scenario: rendered successfully
  - repeated `multidelay` scenario: checksum matched on rerun
  - dataset sweep job: expanded and rendered successfully
- hub:
  - `DaisyHost Hub.exe`: built successfully
  - manual GUI click-through not run in that session
- mouse access to every visible control: editor wiring implemented; not
  exhaustively re-clicked after the final build
- computer keyboard MIDI: previously verified in-session
- external MIDI note input: previously verified in-session
- CC learn: previously verified in-session
- MIDI tracker updates: previously verified in-session
- encoder menu and side drawer sync: implemented in the refreshed editor;
  manual recheck still recommended
- all five DSP parameters editable from both menu and live controls: shared core
  tests pass; final end-to-end manual pass still recommended
- host and Patch firmware menu behavior aligned at the feature level: shared
  core plus firmware adapter build verified
