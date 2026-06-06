# Field Delay Bundle Overview

The Field delay bundle is a Daisy Field hardware project that selects one of
four source-backed delay algorithms through A1-A4. The shared DSP engine is
`DaisyDelayFxCore`. The Field hardware adapter is
`FieldDelayFieldApp.h`.

The main entities are:

- Audio input and audio output.
- External MIDI keyboard input.
- B1-B8 white-key test input from C4 to C5.
- A1-A8 mode keys.
- SW1 and SW2 shifted knob layers.
- Eight physical knobs with an until-touched write gate.
- Per-algorithm parameter snapshots.
- OLED main values and short zoom view.
- Three-state key LED values.
- Four external delay lines.
- Four selected delay algorithms.
- Internal 8-voice pluck/pad synth.

The shared core exposes `DaisyDelayFxSource` values for MultiFX Pedal, Reverb
Playground, FunBox, and SDRAM Delaylines. The user-facing labels are Tape
[multifx], Tank [reverb], Texture [FunBox], and Long [sdram].

The audio callback calls `DaisyDelayFxCore::Process`. It combines audio input
with the internal synth, dispatches to the selected algorithm, then applies
dry/wet mix, bypass or wet-only state, clamp, and output gain.
