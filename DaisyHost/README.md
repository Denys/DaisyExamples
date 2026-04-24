# DaisyHost

`DaisyHost` is a Windows-first desktop host for Daisy apps. The current target
is a visible two-node virtual `Daisy Patch` rack that can host multiple
DaisyHost app cores inside the same `VST3` and standalone shell, plus a small
launcher hub.

## Read Local Docs First

If you are editing this workspace, use the local docs before relying on older
thread context:

- [AGENTS.md](AGENTS.md)
- [PROJECT_TRACKER.md](PROJECT_TRACKER.md)
- [WORKSTREAM_TRACKER.md](WORKSTREAM_TRACKER.md)
- [CHECKPOINT.md](CHECKPOINT.md)
- [CHANGELOG.md](CHANGELOG.md)

Review these high-priority tracking files on every implementation iteration.
`PROJECT_TRACKER.md` is the current roadmap, recommended work order, and
per-iteration testing ledger; always update it at the end of each iteration,
and update the other tracking files when their truth changed.
`WORKSTREAM_TRACKER.md` is the dedicated forward-looking portfolio tracker for
the post-WS7 parallel workstreams and is mirrored back into
`PROJECT_TRACKER.md`.

If the task depends on skill selection, reusable workflow choice, or
`Expected UF` / `Observed UF` updates, also read
[SKILL_PLAYBOOK.md](SKILL_PLAYBOOK.md).

Current source version is `0.2.0`. This README describes the refreshed
workspace at a high level and points to the local docs that track verification
status and current follow-ups.

For Phase 3 offline rendering and dataset generation, also read:

- [training/README.md](training/README.md)

## Scope

The current workspace intentionally models board semantics instead of firmware
internals:

- Patch controls: four knobs, encoder, encoder button
- Patch ports: audio, CV, gate, and MIDI jacks exposed as typed virtual ports
- Patch display: OLED text/bar model rendered by the desktop UI
- Host interaction: mouse-friendly controls plus a virtual MIDI keyboard that
  can be played with the mouse or from the computer keyboard using
  `A/W/S/E/D...` with `Z/X` octave shifts
- MIDI preview: note input from the on-screen keyboard, computer keyboard, or a
  connected external MIDI keyboard is rendered as a smoothed standalone preview
  tone and fed into `MultiDelay`
- MIDI learn: control learn is CC-based; use controller knobs/sliders rather
  than note keys
- MIDI tracking: the Host Tools panel shows current standalone MIDI input status
  plus a rolling log of recent note/CC/program messages; in standalone, external
  MIDI devices must still be enabled in the JUCE `Settings...` dialog
- Current standalone default stays on `Host In`; alternate internal sources are
  available from the mirrored host drawer and shared menu when live input is
  muted or unavailable
- The JUCE standalone mute banner is suppressed in-app; live input mute behavior
  still follows the standalone host safety setting
- Shared app cores:
  - `MultiDelayCore` remains the deterministic regression fixture and continues
    to back the firmware adapter in [patch/MultiDelay](../patch/MultiDelay/README.md)
  - `TorusCore` is the first nontrivial second hosted app, using a DaisyHost-native
    Patch wrapper with Torus-style semantics and menu/control assignment
  - `CloudSeedCore` is now a first-class supported hosted app, built on top of a
    portable `DaisyCloudSeedCore` wrapper around the imported
    `third_party/CloudSeedCore` with a performance-first Patch control model
  - `BraidsCore` is now a first-class supported hosted app, built on top of a
    portable `DaisyBraidsCore` wrapper around the imported Braids oscillator
    sources with a percussion-first MIDI/gate control model
  - `HarmoniqsCore` is now a first-class supported hosted app, built on top of
    a portable `DaisyHarmoniqsCore` additive wrapper with `Spectrum` /
    `Envelope` page remapping and MIDI/gate triggering
  - `VASynthCore` is now a first-class supported hosted app, built on top of a
    portable `DaisyVASynthCore` seven-voice subtractive wrapper with `Osc` /
    `Filter` / `Motion` pages and MIDI-first polyphonic triggering
- Multi-app host:
  - app selection persists in host session state
  - the Patch shell and mirror drawer bind to app metadata and active patch bindings
  - the processor now exposes a fixed five-slot DAW automation bank with stable
    ids `daisyhost.slot1` .. `daisyhost.slot5`
  - those slots rebind to the active app's top-ranked automatable canonical
    parameters while session persistence remains canonical by app parameter id
  - `multidelay` and `cloudseed` now expose named MetaControllers through the
    existing mirrored menu/drawer path, so one macro can steer multiple
    canonical parameters without adding a second persistence layer
  - the processor also exposes an in-process effective-state snapshot for live
    parameter, mapped-slot, MetaController, CV, gate, and audio-input inspection
  - the live plugin and standalone app now run exactly two hosted nodes
    (`node0`, `node1`) at a time with:
    - selected-node-targeted Patch controls, drawer/menu actions, CV/gate/test
      inputs, keyboard MIDI, and five-slot DAW automation
    - four operator-facing audio-only topology presets:
      `node0_only`, `node1_only`, `node0_to_node1`, `node1_to_node0`
    - a visible two-node rack header with per-node app selectors and role
      labels
    - a board-id-based factory seam while still shipping only `daisy_patch`
- Headless rendering:
  - `DaisyHostRender.exe` loads a scenario JSON, renders offline, and writes
    `audio.wav` plus `manifest.json`
  - scenarios drive apps through canonical parameter ids, typed Patch ports,
    and optional compatibility menu actions
  - optional `nodes[]` / `routes[]` contracts now allow internal two-node audio
    render proofs while keeping legacy single-app scenarios valid
  - `training/render_dataset.py` expands sweep jobs into multiple run folders
    and writes a `dataset_index.json`
- Launcher hub:
  - `DaisyHost Hub.exe` selects board, app, and activity
  - `Play / Test` dispatches to the standalone host
  - `Render` dispatches to `DaisyHostRender.exe`
  - `Train` dispatches to `training/render_dataset.py`

## 0.2.0 Control Model

The active refresh target changes the Patch interaction model to:

- `CTRL 1` = dry/wet mix
- `CTRL 2` = primary delay control
- `CTRL 3` = secondary delay control
- `CTRL 4` = feedback
- `CV 1` = tertiary delay control
- `ENC 1` rotate + push = menu navigation and edit/confirm

For the current `MultiDelayCore`, that delay priority order is:

- primary = old delay 1 target
- secondary = old delay 2 target
- tertiary = old delay 3 target

Implemented `0.2.0` additions:

- every DSP parameter editable from the OLED menu
- `last touch wins` arbitration between knob, CV, and menu edits
- shared top-level menu pages: `Params`, `Input`, `MIDI`, `Tracker`, `About`
- Patch-faithful vector panel layout
- version/build identity and recent changelog bullets surfaced through the app
  mirror/About flow
- `Saw` added as a test input
- standalone default test input restored to `Host In`
- multi-app host plumbing with `multidelay` as the default app and `torus` as
  app #2
- `cloudseed` promoted to a first-class supported hosted app with:
  - a portable `DaisyCloudSeedCore` shared wrapper
  - `Space` and `Motion` performance pages that remap the four Patch knobs
  - named `Blend`, `Space`, `Motion`, and `Tone` MetaControllers surfaced
    through the shared menu/drawer path
  - utility actions for `Bypass`, `Clear Tails`, `Randomize Seeds`, and
    `Interpolation`
  - checked-in `cloudseed_smoke.json` render coverage
- `braids` promoted to a first-class supported hosted app with:
  - a portable `DaisyBraidsCore` shared wrapper
  - a six-model percussion-first subset:
    `Kick`, `Snare`, `Cymbal`, `Drum`, `Bell`, `Filtered Noise`
  - `Drum` and `Finish` pages that remap the four Patch knobs
  - MIDI-first triggering plus `gate_in_1` rising-edge strike aliasing
  - utility actions for `Audition`, `Randomize Model`, and `Panic`
  - checked-in `braids_smoke.json` render coverage
- `harmoniqs` promoted to a first-class supported hosted app with:
  - a portable `DaisyHarmoniqsCore` shared wrapper
  - an additive MIDI/gate instrument voice with eight stateful harmonic lanes
  - `Spectrum` and `Envelope` pages that remap the four Patch knobs
  - utility actions for `Audition`, `Init Spectrum`, `Randomize Spectrum`, and
    `Panic`
  - checked-in `harmoniqs_smoke.json` render coverage
- `vasynth` promoted to a first-class supported hosted app with:
  - a portable `DaisyVASynthCore` shared wrapper
  - fixed seven-voice polyphony with MIDI-first note handling and gate alias
  - `Osc`, `Filter`, and `Motion` pages in the shared canonical model, with
    the current DaisyHost menu surfacing `Osc` / `Filter` page switching
  - utility actions for `Audition`, `Init Patch`, `Stereo Sim`, and `Panic`
  - checked-in `vasynth_smoke.json` render coverage
- `multidelay` now also exposes named `Blend`, `Space`, and `Regen`
  MetaControllers through the shared menu/drawer path
- app-aware top-panel control labels and persisted app selection
- fixed five-slot DAW automation bridge with stable ids
  `daisyhost.slot1` .. `daisyhost.slot5`
- processor-side effective-state snapshot/readback for canonical parameters,
  mapped automation slots, selected-node identity, node count,
  board/topology fields, active-node MetaControllers, node summaries, routes,
  CV, gate, and audio-input state
- app-generic headless render runtime with:
  - scenario JSON loading
  - deterministic offline rendering
  - `WAV + JSON manifest` output
  - timeline support for parameter, CV, gate, MIDI, audio-input, impulse, and
    compatibility menu events
  - node/route-aware render contracts with optional per-node summaries,
    rack-level board/selection/topology fields, and `targetNodeId` for
    multi-node non-ID-scoped events
  - forward and reverse two-node audio-chain proofs for internal rack
    validation
- `HostSessionState` v5 with board choice, selected node, rack entry/output
  topology fields, node metadata, and routes while keeping legacy single-node
  sessions backward-compatible
- `DaisyHostRender` CLI target and Python dataset sweep orchestration under
  `training/`

Non-goals for v1:

- full STM32/libDaisy emulation
- Patch SM / Field / Pod / custom board runtime support
- freeform or arbitrary multi-node rack graph editing inside one plugin
- mixed-board racks or Daisy Field runtime support

The abstractions are node-scoped so future work can compose multiple boards in a
single host without redesigning IDs or state formats.

## Build

The host uses CMake with `FetchContent` for JUCE and GoogleTest.

From `DaisyHost/`, prefer the checked-in wrapper:

```sh
.\build_host.cmd
```

It runs the full host gate from the local workspace root, normalizes the
problematic `Path` / `PATH` split for MSBuild-backed commands in this Codex
shell, injects a fresh `DAISYHOST_UNIT_TEST_RUN_TAG` for the current
Release-unit-test payload path, and bypasses local PowerShell execution-policy
friction by launching `build_host.ps1` through `build_host.cmd`.

The underlying raw commands are still:

```sh
cmake -S . -B build
cmake --build build --config Release --target unit_tests DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir build -C Release --output-on-failure
```

`ctest` now includes the normal host unit coverage plus the direct-entrypoint
smoke tests:

- `DaisyHostStandaloneSmoke`
- `DaisyHostRenderSmoke`

The smoke harness lives at `tests/run_smoke.py`. If you run the raw
MSBuild-backed commands directly instead of the wrapper, sanitize `Path` /
`PATH` first.

Checked-in render smoke scenarios now include:

- `training/examples/multidelay_smoke.json`
- `training/examples/torus_smoke.json`
- `training/examples/cloudseed_smoke.json`
- `training/examples/braids_smoke.json`
- `training/examples/harmoniqs_smoke.json`
- `training/examples/vasynth_smoke.json`

Outputs:

- `DaisyHost Hub.exe`
- `DaisyHostRender.exe`
- `DaisyHost Patch.vst3`
- `DaisyHost Patch.exe`
- `unit_tests` target, currently emitted as a payload under
  `build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
  and launched through `tests/run_unit_test_payload.py`

Current local verification caveat:

- the wrapper-driven full host gate reran green on 2026-04-24:
  `cmd /c build_host.cmd` passed and `ctest` passed `128/128`
- `tests/run_smoke.py` now uses a wider process-query timeout for standalone
  smoke so slower Windows process-path discovery does not produce a false
  timeout on an otherwise healthy launch
- Daisy Field implementation readiness is now satisfied in this checkout; the
  next post-WS7 portfolio is tracked in `WORKSTREAM_TRACKER.md`

## Architecture

- `include/daisyhost/`: stable host-side abstractions
- `src/apps/MultiDelayCore.cpp`: portable extracted app core
- `src/apps/TorusCore.cpp`: DaisyHost-native Torus pilot app core
- `include/daisyhost/DaisyCloudSeedCore.h` / `src/DaisyCloudSeedCore.cpp`:
  portable CloudSeed wrapper and canonical state mapper
- `include/daisyhost/DaisyBraidsCore.h` / `src/DaisyBraidsCore.cpp`:
  portable Braids wrapper and canonical percussion-state mapper
- `include/daisyhost/DaisyHarmoniqsCore.h` / `src/DaisyHarmoniqsCore.cpp`:
  portable Harmoniqs wrapper and canonical additive-state mapper
- `include/daisyhost/DaisyVASynthCore.h` / `src/DaisyVASynthCore.cpp`:
  portable VA synth wrapper and canonical polyphonic-state mapper
- `src/apps/CloudSeedCore.cpp`: DaisyHost-native CloudSeed supported app core
- `src/apps/BraidsCore.cpp`: DaisyHost-native Braids supported app core
- `src/apps/HarmoniqsCore.cpp`: DaisyHost-native Harmoniqs supported app core
- `src/apps/VASynthCore.cpp`: DaisyHost-native VA Synth supported app core
- `src/AppRegistry.cpp`: app registry and factory layer
- `src/HubSupport.cpp`: launcher hub registries, profiles, launch planning, and startup requests
- `src/HostAutomationBridge.cpp`: stable five-slot DAW automation mapping
- `src/HostSessionState.cpp`: backward-compatible session serialization with
  rack globals plus node and route records
- `src/LiveRackTopology.cpp`: visible rack topology preset expansion,
  validation, and reverse inference
- `src/EffectiveHostStateSnapshot.cpp`: processor-side live-state snapshot model
  with selected-node, board/topology fields, route summaries, and
  MetaController readback
- `src/RenderRuntime.cpp`: headless render runtime, scenario loading, manifest
  emission, node/route validation, node-targeted event routing, and WAV writing
- `tools/write_vst3_manifest.ps1`: PowerShell-backed VST3 manifest helper shim
  used by the Windows CMake build
- `src/hub/`: JUCE launcher hub
- `src/juce/`: JUCE plugin and standalone wrapper
- `tools/render_app.cpp`: command-line render entrypoint
- `training/`: dataset orchestration, schema notes, and example scenarios
- `tests/`: unit tests for the abstractions and the extracted core
- `AGENTS.md`, `PROJECT_TRACKER.md`, `SKILL_PLAYBOOK.md`, `CHECKPOINT.md`,
  `CHANGELOG.md`: local project guidance, roadmap/order tracking, skill
  validation, and verification history

The firmware adapter remains in [patch/MultiDelay/MultiDelay.cpp](../patch/MultiDelay/MultiDelay.cpp).
