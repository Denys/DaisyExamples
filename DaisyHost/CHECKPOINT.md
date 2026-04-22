# DaisyHost Checkpoint

## Snapshot

- Date: 2026-04-22
- Workspace: `DaisyHost/`
- Current CMake version in source: `0.2.0`
- Active refresh target: `0.2.0`
- Scope: host-side Daisy Patch plugin and standalone app with a multi-app host
  layer, `MultiDelay` as the default regression fixture, `Torus` as the first
  second app, `CloudSeed` as the first Workstream-6 app pilot, and a headless
  render/dataset workflow

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
environment split for MSBuild-backed commands in this shell, and reruns the
full host gate.

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

- Last full DaisyHost host build/test verification rerun from this checkout:
  2026-04-22
- Verified commands/results in the 2026-04-22 pass:
  - `.\build_host.cmd`: passed
  - underlying host-gate steps executed by the wrapper:
    - `cmake -S . -B build`: passed
    - `cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone`: passed
    - `ctest --test-dir build -C Release --output-on-failure`: passed, `79/79`
  - smoke tests included:
    - `DaisyHostStandaloneSmoke`
    - `DaisyHostRenderSmoke`
  - firmware reference reruns:
    - `make` from `patch/MultiDelay/`: passed
    - `make` from `patch/Torus/`: passed after a PowerShell `build/` removal,
      because `make clean` uses `rm` and is not Windows-safe in this checkout
- Historical note: the prior full runtime verification recorded in local docs
  before this rerun was `2026-04-19`
- In this Codex shell, raw MSBuild commands still require a sanitized one-path
  environment because both `Path` and `PATH` are exported by default, but
  `build_host.cmd` now encapsulates that repair.

## Last Recorded Artifact Paths

The following paths were verified from this checkout on 2026-04-22:

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
  - `ctest` aggregate (`79/79` passed on 2026-04-22, including `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke`)

## Current Hosted Apps

- default app id: `multidelay`
- additional app id: `torus`
- additional app id: `cloudseed`

Current Phase 2 host behavior:

- DaisyHost instantiates hosted apps by stable `appId`
- the selected app persists in `HostSessionState`
- the processor/editor bind to the active app's metadata and patch bindings
- the processor now exposes a fixed five-slot DAW automation bank with stable
  ids `daisyhost.slot1` .. `daisyhost.slot5`
- those five slots rebind to the active app's top-ranked automatable canonical
  parameters without changing the saved slot ids
- session persistence remains canonical by app parameter id rather than DAW slot
  id
- `GetEffectiveHostStateSnapshot()` now exposes live canonical parameters,
  mapped automation slots, CV state, gate state, and current audio-input state
  for processor/UI/debug tooling
- only one hosted app runs at a time in this phase

Current `CloudSeed` pilot behavior:

- hosted app id: `cloudseed`
- shared portable wrapper: `DaisyCloudSeedCore`
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

Current Phase 3 render behavior:

- `DaisyHostRender` loads scenario JSON by `appId`
- each render run resets app state to a fixed seed, restores canonical
  parameters, replays a timeline, and renders offline
- each run writes `audio.wav` plus `manifest.json`
- `training/render_dataset.py` expands sweep jobs into multiple run folders and
  writes `dataset_index.json`

Current hub behavior:

- `DaisyHost Hub` is a small front door for:
  - `Play / Test`
  - `Render`
  - `Train`
- v1 board selection currently exposes `daisy_patch`
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

- `unit_tests` plus the new smoke tests were rerun from this checkout on
  2026-04-22, and `ctest` passed `79/79`.
- `DaisyHostHub`, `DaisyHostRender`, and `DaisyHostPatch_VST3` rebuilt
  successfully on 2026-04-22.
- `DaisyHostPatch_Standalone` also rebuilt successfully on 2026-04-22 after
  the output-path lock was cleared.
- Workstreams 4 and 5 are now source-backed in this checkout:
  - DaisyHost exposes a fixed five-slot DAW automation bridge that maps stable
    slot ids onto the active app's canonical automatable parameters
  - the processor exposes an in-process effective-state snapshot/readback API
    for parameters, mapped slots, CV, gate, and audio-input state
- `tests/run_smoke.py` now provides the direct-entrypoint smoke harness:
  - `DaisyHostStandaloneSmoke` launches `DaisyHost Patch.exe` with
    `--board daisy_patch --app torus`, requires a crash-free 5s warmup window,
    then terminates the process cleanly
  - `DaisyHostRenderSmoke` runs `DaisyHostRender.exe` against the checked-in
    `multidelay`, `torus`, and `cloudseed` scenarios, verifies `audio.wav` plus
    `manifest.json`, and checks repeated `multidelay` `audioChecksum`
    determinism
- Workstream 6 now has its first landed app pilot in this checkout:
  - `cloudseed` wraps `third_party/CloudSeedCore` through a portable
    `DaisyCloudSeedCore`
  - page changes now refresh the JUCE processor's active patch bindings, so
    DaisyHost control labels and control ids follow the active `Space` /
    `Motion` page instead of staying stale after menu navigation
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
  - `training/examples/dataset_job_example.json`
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
executed on 2026-04-22:

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
  2026-04-22 through `tests/run_smoke.py`
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
