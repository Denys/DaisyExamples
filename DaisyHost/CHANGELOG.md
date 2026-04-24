# Changelog

Canonical change tracker for `DaisyHost`.

## [Unreleased]

- add `tests/run_smoke.py` as a direct-entrypoint smoke harness for
  `DaisyHost Patch.exe` and `DaisyHostRender.exe`
- wire `DaisyHostStandaloneSmoke` and `DaisyHostRenderSmoke` into CTest so the
  normal host gate now covers standalone startup stability plus render-output
  smoke and `multidelay` checksum determinism
- add a fixed five-slot DAW automation bridge with stable ids
  `daisyhost.slot1` .. `daisyhost.slot5`, mapped onto the active app's
  top-ranked automatable canonical parameters
- add processor-side effective-state snapshot/readback for canonical
  parameters, mapped automation slots, CV state, gate state, and current
  audio-input configuration
- extend the effective-state snapshot with `selectedNodeId`, `nodeCount`, and
  active-node MetaController readback
- expand the effective-state snapshot again for the visible rack with `boardId`,
  `entryNodeId`, `outputNodeId`, `routes`, and lightweight `nodeSummaries`
- add named MetaControllers for `multidelay` and `cloudseed`, surfaced through
  the existing mirrored menu/drawer path and derived from canonical parameters
- promote `cloudseed` from a Workstream-6 pilot to a first-class supported
  hosted app
- add `HostSessionState` v4 with `node` and `route` records while keeping
  legacy single-node session text backward-compatible
- extend render scenario and manifest contracts with optional `nodes[]` /
  `routes[]` plus per-node summaries
- add a hidden two-node audio-chain proof path to the render runtime and reject
  unsupported cross-node CV, gate, and MIDI routes with clear errors
- add `DaisyCloudSeedCore` as a portable shared wrapper around the imported
  `third_party/CloudSeedCore` with canonical performance/expert state,
  deterministic seed handling, bypass/clear/randomize utilities, and effective
  raw-parameter readback
- add `cloudseed` as the first Workstream-6 hosted-app pilot with:
  - stereo host input/output on Patch channels 1/2
  - `Space` / `Motion` performance pages that remap the four Patch knobs
  - encoder/menu sections for `Pages`, `Program`, `Utilities`, and `Info`
  - utility actions for `Bypass`, `Clear Tails`, `Randomize Seeds`, and
    `Interpolation`
  - fixed DAW automation priority for `mix`, `size`, `decay`, `diffusion`, and
    `pre_delay`
- refresh the JUCE processor's active patch bindings during core snapshot
  updates so page-based control remaps change live DaisyHost knob labels and
  control ids instead of staying cached from app creation
- add `training/examples/cloudseed_smoke.json`, extend render/runtime coverage
  for `cloudseed`, and include the new scenario in `DaisyHostRenderSmoke`
- add `DaisyBraidsCore` as a portable shared wrapper around vendored Braids
  oscillator code with a curated percussion-first model subset, `24`-sample
  inner rendering, deterministic trigger/state handling, and canonical
  parameter capture/restore
- add `braids` as a first-class supported hosted app with:
  - MIDI-first triggering plus optional `gate_in_1` strike alias
  - `Drum` / `Finish` performance pages that remap the four Patch knobs
  - encoder/menu sections for `Pages`, `Model`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Randomize Model`, and `Panic`
  - fixed DAW automation priority for `model`, `tune`, `timbre`, `color`, and
    `signature`
- add `training/examples/braids_smoke.json`, extend render/runtime smoke
  coverage for `braids`, and include the new scenario in
  `DaisyHostRenderSmoke`
- add `DaisyHarmoniqsCore` as a portable shared additive wrapper with
  deterministic harmonic-state capture/restore, MIDI/gate triggering, and
  `Spectrum` / `Envelope` page bindings
- add `harmoniqs` as a first-class supported hosted app with:
  - MIDI-first triggering plus optional `gate_in_1` retrigger alias
  - `Spectrum` / `Envelope` knob-page remapping
  - encoder/menu sections for `Pages`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Init Spectrum`, `Randomize Spectrum`, and
    `Panic`
- add `DaisyVASynthCore` as a portable shared seven-voice subtractive wrapper
  with deterministic polyphonic state, MIDI/gate triggering, and `Osc` /
  `Filter` / `Motion` page bindings
- add `vasynth` as a first-class supported hosted app with:
  - seven-voice MIDI-first note handling plus optional `gate_in_1` trigger
    alias
  - `Osc` / `Filter` knob-page remapping surfaced through the current
    DaisyHost menu flow, with `Motion` kept in the shared canonical model
  - encoder/menu sections for `Pages`, `Oscillators`, `Utilities`, and `Info`
  - utility actions for `Audition`, `Init Patch`, `Stereo Sim`, and `Panic`
- add `training/examples/harmoniqs_smoke.json` and
  `training/examples/vasynth_smoke.json`, extend render/runtime smoke coverage
  for both apps, and include the new scenarios in `DaisyHostRenderSmoke`
- add DaisyHost-local `include/stmlib/...` compatibility shims so the vendored
  Braids sources build under the Windows MSVC host toolchain without modifying
  the shared repo `stmlib/` copy
- apply a local MSVC portability fix to
  `third_party/CloudSeedCore/DSP/RandomBuffer.cpp` by replacing the upstream
  VLA scratch buffers with `std::vector`
- add helper-layer unit coverage for the automation bridge and effective-state
  snapshot contract, then extend the rerun host gate to `79/79` on 2026-04-22
- add helper-layer and contract coverage for MetaControllers plus rack-ready
  session/render state, then rerun the full host gate to `89/89` on 2026-04-23
- add `LiveRackTopology` as the shared helper for the four visible audio-only
  rack presets, reverse inference, and legal-route validation
- add the first visible WS7 live rack MVP:
  - exactly two live hosted nodes (`node0`, `node1`)
  - selected-node-targeted Patch controls, drawer/menu state, test inputs,
    keyboard MIDI, and five-slot DAW automation
  - editor rack header with per-node app selectors, selected-node highlight,
    role labels, and topology selector
  - operator-facing audio-only topology presets:
    `node0_only`, `node1_only`, `node0_to_node1`, `node1_to_node0`
- add a board factory seam so the runtime creates board profiles by `boardId`
  instead of hardcoding Patch construction in the processor path
- move session persistence to `HostSessionState` v5 with rack globals
  `boardId`, `selectedNodeId`, `entryNodeId`, and `outputNodeId`
- extend render timeline events with `targetNodeId` for multi-node non-ID-
  scoped events and prove both forward and reverse two-node serial audio chains
  directly through `DaisyHostRender.exe`
- change unit-test registration from runtime discovery to source-scanned
  `gtest_add_tests`, keep the target name `unit_tests`, and launch unit cases
  through `tests/run_unit_test_payload.py`
- change the Release unit-test payload path again so wrapper-driven configure
  runs emit a fresh per-run payload under
  `build/unit_test_bin/<run-tag>/<config>/DaisyHostTestPayload.bin`
- make `build_host.ps1` PATH normalization accept shells that expose only
  `Path` as well as shells that expose the duplicate `PATH` key
- make `build_host.ps1` also inject a fresh
  `DAISYHOST_UNIT_TEST_RUN_TAG` on each configure so wrapper-driven reruns do
  not collide with stale quarantined Release payload paths in this checkout
- fix the Braids render-runtime unit scenario duration so its final gate event
  is inside the render window
- add `build_host.ps1` plus `build_host.cmd` as the standard local host-gate
  entrypoint, encapsulating the shell-specific `Path` / `PATH` normalization
  needed by MSBuild-backed commands in this checkout
- widen the standalone smoke process-query timeout so slower Windows
  process-path discovery does not cause a false timeout on a healthy launch
- rerun the full wrapper-driven host gate on 2026-04-24 and pass `128/128`
- route Windows VST3 manifest generation through
  `tools/write_vst3_manifest.ps1` so `juce_vst3_helper.exe` is invoked through
  PowerShell instead of the failing direct MSBuild / `cmd` launch path
- document the smoke harness commands and update local verification docs to
  reflect automated standalone/render smoke coverage
- document that external DAW/VST3 load validation is intentionally deferred
  until a post-Workstream-7 manual pass
- promote `PROJECT_TRACKER.md` to a first-class DaisyHost tracking document and
  per-iteration testing ledger
- require mandatory per-iteration testing evidence for docs and code work, with
  targeted test expectations before handoff and the full host gate before
  milestone or merge handoff
- define the concurrent-thread protocol: `AGENTS.md` stays authoritative for
  ownership slices and contracts, while `PROJECT_TRACKER.md` stays authoritative
  for active work order, testing entries, and handoffs
- add `ROADMAP.d2` plus a compiled `ROADMAP.svg` for the current governance,
  workstream, and parallelization roadmap
- add `WORKSTREAM_TRACKER.md` as the dedicated post-WS7 portfolio tracker and
  mirror the same next-workstream plan into `PROJECT_TRACKER.md`
- add `SKILL_PLAYBOOK.md` as the dedicated DaisyHost file for skill-related
  activities and split skill ratings into `Expected UF` vs evidence-based
  `Observed UF`
- relabel stale verification counts and artifact assertions in local docs so
  unrerun runtime claims stay dated instead of implied-current

## [0.2.0] - 2026-04-18

Implemented refresh:

- promote `DaisyHost/` to a pinned first-party workspace in repo guidance
- add DaisyHost-local agent docs, checkpointing, and a canonical changelog flow
- remap Patch controls to:
  - `CTRL 1` = dry/wet
  - `CTRL 2` = primary delay
  - `CTRL 3` = secondary delay
  - `CTRL 4` = feedback
  - `CV 1` = tertiary delay
  - `ENC 1` = menu navigation and edit/confirm
- make every DSP parameter editable from the OLED menu with `last touch wins`
- redesign the host UI toward a more clearly Patch-like vector layout
- add `Saw` as a test input and restore `Host In` as the standalone default
- surface version/build identity and recent change history through the app mirror/About flow
- add standalone icon assets, wire the compact icon into `ICON_SMALL`, and apply a runtime standalone window icon
- hide the JUCE standalone yellow mute banner while keeping live input muted by default
- generalize DaisyHost into a multi-app Patch host with an app registry/factory layer
- persist selected hosted app in session state
- keep `MultiDelay` as the default deterministic fixture
- add `Torus` as the first nontrivial second hosted app with:
  - DaisyHost-native canonical parameter and menu model
  - dynamic knob-assignment menu
  - stereo output on Patch outputs 1/2
  - gate-triggered strum and audio-input exciter semantics
- rebind the processor and editor to active app metadata and patch bindings instead of hardcoded `MultiDelay` ids
- add Phase 3 headless render support with:
  - `DaisyHostRender` CLI target
  - scenario JSON loading and validation
  - deterministic offline `WAV + JSON manifest` output
  - timeline support for parameter, CV, gate, MIDI, audio-input, impulse, and
    compatibility menu events
- add `training/render_dataset.py`, dataset job examples, and render workflow
  docs under `training/`
- add `DaisyHost Hub` as a small GUI launcher for board/app/activity selection
- add hub-local profile persistence separate from plugin/session state
- add a startup-request handoff so `Play / Test` opens the standalone host with
  the selected app

## [0.1.0] - Existing Workspace Baseline

Observed baseline already present in the repo before the `0.2.0` refresh lands:

- `DaisyHost` builds a `VST3` plugin and standalone app around the extracted `patch/MultiDelay` core
- current built artifacts exist under:
  - `build/DaisyHostPatch_artefacts/Release/VST3/DaisyHost Patch.vst3`
  - `build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe`
- host-side test target is `unit_tests`
- host UI already includes mouse-accessible controls, a virtual keyboard, computer-keyboard MIDI input, CC learn support, and a MIDI tracker
- the current source version remains `0.1.0` until the main integrator slice updates `CMakeLists.txt`
