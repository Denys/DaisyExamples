# DaisyHost AGENTS

Local agent instructions for `DaisyHost/`.

Read this file after the repo-level [AGENTS.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/AGENTS.md) and before editing `DaisyHost/`.

## Start Here

1. Read [README.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/README.md) for the workspace overview.
2. Read [CHECKPOINT.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/CHECKPOINT.md) for the current verified state, artifact paths, control map, and open issues.
3. Read [CHANGELOG.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/CHANGELOG.md) before editing user-facing behavior or versioned release notes.
4. If multiple agents are active, stay inside your ownership slice. Do not touch another slice's files unless the user explicitly reassigns ownership or an integrator asks for a conflict-resolution edit.

## Workspace Map

- `include/daisyhost/`: shared host-side interfaces and data models
- `src/apps/`: portable extracted Daisy app cores
- `src/juce/`: JUCE processor/editor implementation
- `tests/`: host-side unit coverage
- `build/`: generated host-side outputs; use it for artifact checks, not as source of truth

## Parallel Ownership Slices

### Worker 1: Core, Parameters, Menu, Firmware

Owns:

- `include/daisyhost/HostedAppCore.h`
- `include/daisyhost/DisplayModel.h`
- `include/daisyhost/apps/MultiDelayCore.h`
- `src/apps/MultiDelayCore.cpp`
- `tests/test_multidelay_core.cpp`
- `../patch/MultiDelay/MultiDelay.cpp`

Expected outcomes:

- shared `ParameterDescriptor` and `MenuModel`
- final Patch control hierarchy
- encoder/menu behavior shared by host and firmware
- menu-editable access to all five DSP parameters
- `last touch wins` arbitration across knob, CV, and menu sources

Required verification before handoff:

- targeted `MultiDelayCore` tests pass
- `make` passes in `../patch/MultiDelay`

### Worker 2: Patch UI, Board Layout, Icon, Visual Fidelity

Owns:

- `include/daisyhost/BoardProfile.h`
- `src/BoardProfile.cpp`
- `src/juce/DaisyHostPluginEditor.h`
- `src/juce/DaisyHostPluginEditor.cpp`
- `assets/*`
- `tests/test_board_profile.cpp`

Expected outcomes:

- Patch-faithful vector layout
- visible control positions that match the final hierarchy
- reduced right-hand drawer that mirrors the shared menu model
- standalone icon assets

Required verification before handoff:

- board-profile tests pass
- standalone app launches and renders without layout corruption

### Worker 3: Repo Awareness, Docs, Version Tracking

Owns:

- `../AGENTS.md`
- `../LATEST_PROJECTS.md`
- `AGENTS.md`
- `CHECKPOINT.md`
- `CHANGELOG.md`
- `README.md`

Expected outcomes:

- DaisyHost treated as a pinned first-party workspace
- local docs for current state, parallel ownership, version history, and known issues
- docs aligned with actual artifact names and build targets already present in the repo

Required verification before handoff:

- reread all six owned docs for consistency
- confirm current artifact names in `build/DaisyHostPatch_artefacts/Release/`
- confirm the host test target remains `unit_tests`
- do not claim code behavior that is still only a planned `0.2.0` target

### Main Integrator: Host Plumbing, Inputs, Version Surfacing

Owns:

- `CMakeLists.txt`
- `src/juce/DaisyHostPluginProcessor.h`
- `src/juce/DaisyHostPluginProcessor.cpp`
- `include/daisyhost/HostSessionState.h`
- `src/HostSessionState.cpp`
- `include/daisyhost/HostStartupPolicy.h`
- `src/HostStartupPolicy.cpp`
- `tests/test_host_startup_policy.cpp`
- new targeted tests for saw input, version surfacing, or session wiring

Expected outcomes:

- `0.2.0` version bump
- `Saw` test input and `Host In` startup default
- version/about surfacing in the app
- final integration between shared menu state and the mirrored host drawer

Required verification before handoff:

- startup-policy tests pass
- saw-input behavior is tested
- session/version wiring is tested or manually validated

## Shared Behavior Contract

- Final Patch control hierarchy:
  - `CTRL 1` = dry/wet mix
  - `CTRL 2` = primary delay control
  - `CTRL 3` = secondary delay control
  - `CTRL 4` = feedback
  - `CV 1` = tertiary delay control
  - `ENC 1` rotate + push = menu navigation and edit/confirm
- For current `MultiDelayCore`, the delay priority order is:
  - primary = old delay 1 target
  - secondary = old delay 2 target
  - tertiary = old delay 3 target
- Every DSP parameter must also be editable from the OLED menu.
- If knob, CV, or menu all target the same parameter, `last touch wins`.

## Completion Gate

Do not call the plugin complete until all of the following pass together:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

and:

```sh
make
```

from `../patch/MultiDelay/`.

Manual validation still required before final handoff:

- standalone launch stability
- VST3 load in a DAW
- standalone icon visible
- `Host In`, `Sine`, `Saw`, `Noise`, and `Impulse`
- mouse control of all visible controls
- computer-keyboard MIDI
- external MIDI note input
- CC learn
- MIDI tracker updates
- encoder menu and drawer sync
- all five DSP parameters editable from both menu and live controls
- host and Patch firmware menu behavior aligned at the feature level
