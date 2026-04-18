# DaisyHost

`DaisyHost` is a Windows-first desktop host for Daisy apps. The v1 target is a
single virtual `Daisy Patch` node that runs the extracted `patch/MultiDelay`
engine as both a `VST3` and a standalone desktop app.

## Scope

v1 intentionally models board semantics instead of firmware internals:

- Patch controls: four knobs, encoder-style dry/wet control, encoder button
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
- Standalone safety: fresh standalone launches default to an internal sine test
  source so the app is audible even when JUCE mutes live input to avoid
  feedback
- Shared app core: the same `MultiDelayCore` is used by the desktop host and the
  firmware adapter in [patch/MultiDelay](../patch/MultiDelay/README.md)

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
cmake --build DaisyHost/build --config Release
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

Outputs:

- `DaisyHostPatch.vst3`
- `DaisyHostPatch Standalone`
- `unit_tests`

## Architecture

- `include/daisyhost/`: stable host-side abstractions
- `src/apps/MultiDelayCore.cpp`: portable extracted app core
- `src/juce/`: JUCE plugin and standalone wrapper
- `tests/`: unit tests for the abstractions and the extracted core

The firmware adapter remains in [patch/MultiDelay/MultiDelay.cpp](../patch/MultiDelay/MultiDelay.cpp).
