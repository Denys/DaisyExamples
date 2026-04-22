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
- apply a local MSVC portability fix to
  `third_party/CloudSeedCore/DSP/RandomBuffer.cpp` by replacing the upstream
  VLA scratch buffers with `std::vector`
- add helper-layer unit coverage for the automation bridge and effective-state
  snapshot contract, then extend the rerun host gate to `79/79` on 2026-04-22
- add `build_host.ps1` plus `build_host.cmd` as the standard local host-gate
  entrypoint, encapsulating the shell-specific `Path` / `PATH` normalization
  needed by MSBuild-backed commands in this checkout
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
