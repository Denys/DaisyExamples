# DaisyHost

`DaisyHost` is a Windows-first desktop host for Daisy apps. The current target
is a single virtual `Daisy Patch` node that can host multiple DaisyHost app
cores inside the same `VST3` and standalone shell.

## Read Local Docs First

If you are editing this workspace, use the local docs before relying on older
thread context:

- [AGENTS.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/AGENTS.md)
- [CHECKPOINT.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/CHECKPOINT.md)
- [CHANGELOG.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/DaisyHost/CHANGELOG.md)

Current source version is `0.2.0`. This README describes the refreshed
workspace at a high level and points to the local docs that track verification
status and current follow-ups.

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
- Multi-app host:
  - app selection persists in host session state
  - the Patch shell and mirror drawer bind to app metadata and active patch bindings
  - only one app runs at a time in this phase; there is still no multi-node rack

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
- app-aware top-panel control labels and persisted app selection

Non-goals for v1:

- full STM32/libDaisy emulation
- Patch SM / Field / Pod / custom board runtime support
- multi-node Daisy rack patching inside one plugin

The abstractions are node-scoped so future work can compose multiple boards in a
single host without redesigning IDs or state formats.

## Build

The host uses CMake with `FetchContent` for JUCE and GoogleTest.

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

Outputs:

- `DaisyHost Patch.vst3`
- `DaisyHost Patch.exe`
- `unit_tests`

## Architecture

- `include/daisyhost/`: stable host-side abstractions
- `src/apps/MultiDelayCore.cpp`: portable extracted app core
- `src/apps/TorusCore.cpp`: DaisyHost-native Torus pilot app core
- `src/AppRegistry.cpp`: app registry and factory layer
- `src/juce/`: JUCE plugin and standalone wrapper
- `tests/`: unit tests for the abstractions and the extracted core
- `AGENTS.md`, `CHECKPOINT.md`, `CHANGELOG.md`: local project guidance and
  version tracking

The firmware adapter remains in [patch/MultiDelay/MultiDelay.cpp](../patch/MultiDelay/MultiDelay.cpp).
