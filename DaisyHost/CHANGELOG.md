# Changelog

Canonical change tracker for `DaisyHost`.

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
