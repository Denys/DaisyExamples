# DaisyHost Checkpoint

## Snapshot

- Date: 2026-04-19
- Workspace: `DaisyHost/`
- Current CMake version in source: `0.2.0`
- Active refresh target: `0.2.0`
- Scope: host-side Daisy Patch plugin and standalone app with a multi-app host
  layer, `MultiDelay` as the default regression fixture, `Torus` as the first
  second app, and a new headless render/dataset workflow

This checkpoint records the current verified artifact names, the agreed control
map for the `0.2.0` refresh, the Phase 2 multi-app host state, the Phase 3
headless render target, and the latest verification results.

## DaisyBrain NotebookLM Notebook

Curated repo knowledge notebook created for Daisy and DaisyExamples architecture
questions, planning, and design recall:

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

Configure and build the host workspace:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

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

## Current Verified Artifact Names In Repo Build Output

These names and paths were confirmed from the current `DaisyHost/build` tree:

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

This checkpoint now reflects a fresh host build, full host test pass, and
successful render smoke runs for both `multidelay` and `torus`.

## Current Hosted Apps

- default app id: `multidelay`
- additional app id: `torus`

Current Phase 2 host behavior:

- DaisyHost instantiates hosted apps by stable `appId`
- the selected app persists in `HostSessionState`
- the processor/editor bind to the active app's metadata and patch bindings
- only one hosted app runs at a time in this phase

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

- `unit_tests` currently passes `61/61`.
- Host outputs build at:
  - [DaisyHost Hub.exe](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/build/DaisyHostHub_artefacts/Release/DaisyHost%20Hub.exe)
  - [DaisyHostRender.exe](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/build/Release/DaisyHostRender.exe)
  - [DaisyHost Patch.vst3](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/build/DaisyHostPatch_artefacts/Release/VST3/DaisyHost%20Patch.vst3)
  - [DaisyHost Patch.exe](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost%20Patch.exe)
- The standalone package now uses the compact DaisyHost icon asset through `ICON_SMALL`.
- The standalone input-mute warning banner is suppressed in the app UI while
  the underlying host-input mute behavior remains unchanged.
- `patch/MultiDelay` requires a Windows-friendly clean rebuild when stale host
  objects are present: remove `build/` with PowerShell, then run `make`.
- The Torus pilot inside DaisyHost is intentionally a DaisyHost-native wrapper
  with Torus/Rings semantics and menu/control assignment behavior. It does not
  compile the original `patch/Torus` firmware entrypoint or Daisy UI layer into
  the host.
- Phase 3 scenario/dataset examples live under:
  - `training/examples/multidelay_smoke.json`
  - `training/examples/torus_smoke.json`
  - `training/examples/dataset_job_example.json`
- Full DAW-side VST3 validation is still manual; no automated DAW load test is
  wired into this workspace.

## Final Verification Gate Before Calling The Plugin Complete

Recorded automated commands:

```sh
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

Recorded manual/CLI render checks:

```sh
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/multidelay_smoke.json --output-dir DaisyHost/training/out/multidelay_smoke
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/torus_smoke.json --output-dir DaisyHost/training/out/torus_smoke
py -3 DaisyHost/training/render_dataset.py DaisyHost/training/examples/dataset_job_example.json
```

Recorded manual checks:

- standalone launch stability: passed via 5s smoke launch on 2026-04-18
- VST3 load in a DAW: not verified in this session
- visible standalone icon: not visually rechecked after the final build in this
  session
- `Host In`, `Sine`, `Saw`, `Noise`, and `Impulse`: build/test covered; manual
  audio pass not repeated after the final UI patch
- app selection and app restore:
  - host/session serialization covered by tests
  - no manual GUI click-through for `multidelay` -> `torus` -> restore was run
    in this session
- render CLI:
  - `multidelay` scenario: rendered successfully
  - `torus` scenario: rendered successfully
  - repeated `multidelay` scenario: checksum matched on rerun
  - dataset sweep job: expanded and rendered successfully
- hub:
  - `DaisyHost Hub.exe`: built successfully
  - manual GUI click-through not run in this session
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
  core + firmware adapter build verified
