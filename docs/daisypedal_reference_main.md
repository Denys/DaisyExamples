# DaisyPedal Reference

[TOC]

# Overview

This reference covers the Phase 1 DaisyPedal assimilation work inside
`DaisyExamples/`.

The documented surface is intentionally narrow:

- the `DaisyPedal` board helper in `libDaisy`
- the hardware-independent DSP building blocks in `DaisySP/Source/DaisyPedal`
- the first-party `pedal/` example family that demonstrates those APIs

This reference is designed to mirror the generated-reference workflow already
used in the canonical `DaisyDAFX` workspace: Doxygen generates HTML and LaTeX,
then LaTeX is compiled into the final PDF artifact.

# Library Boundaries

## DaisyPedal Integration

The board-facing hardware contract lives in
[libDaisy/src/daisy_pedal.h](../libDaisy/src/daisy_pedal.h).

It covers:

- stereo audio startup and callback helpers
- six analog knobs
- two footswitches
- encoder and click switch
- two panel LEDs
- OLED setup
- MIDI UART setup
- bypass and mute relay control

## DaisySP Modules

The hardware-independent DSP modules live under
[DaisySP/Source/DaisyPedal](../DaisySP/Source/DaisyPedal).

Phase 1 includes:

| Module | Purpose |
| --- | --- |
| `BypassFader` | Pop-free dry/wet switching for pedal transitions |
| `TapTempo` | Audio-rate beat tracking from footswitch taps |
| `NoiseGate` | Envelope-following gate with hysteresis |
| `Multirate` | Lightweight low-rate conditioning helper |
| `OctaveGenerator` | Sub-octave and octave-up voice generation |

## Pedal Examples

The example family under [pedal/](../pedal/README.md) demonstrates how the new
board helper and DSP modules are intended to be used together.

Phase 1 examples:

| Example | Focus |
| --- | --- |
| `PassthruBypass` | Relay bypass, mute timing, OLED status, tap indicator |
| `NoiseGate` | Stereo gate using `DaisySP::NoiseGate` |
| `PitchDrop` | Mono guitar pitch drop using `PitchShifter` and `BypassFader` |
| `PolyOctave` | Mono octave effect using `Multirate` and `OctaveGenerator` |

# Recommended Documentation Contract

The generated PDF is most useful when the public headers answer three questions
quickly:

1. What problem does this type solve?
2. How is it initialized and processed?
3. What are the expected parameter ranges and control semantics?

For that reason, this reference is driven from headers and Markdown pages rather
than implementation files.

# Regeneration

Use the rebuild helpers in `util/`:

- Windows: `util/rebuild_daisypedal_docs.cmd`
- Unix-like shells: `util/rebuild_daisypedal_docs.sh`

Expected outputs:

- HTML: `build/docs/daisypedal/html/`
- LaTeX: `build/docs/daisypedal/latex/`
- PDF: `docs/daisypedal_reference.pdf`
