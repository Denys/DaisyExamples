# Texture [FunBox]

Texture [FunBox] adapts `GuitarML/FunBox` into the bundle through
`DaisyDelayFxCore::ProcessFunBox`.

The algorithm combines several delay behaviors in one texture engine: normal
stereo taps, a grain tap, a reverse accent tap, slow drift modulation, density,
smear, freeze or hold, and cross-feedback writes.

Important entities:

- Driven stereo input.
- Slow drift LFO.
- Normal left and right delay taps.
- Grain delay tap.
- Reverse accent tap.
- Texture blend.
- Reverse amount.
- Freeze amount.
- Cross-feedback writes.
- Wet left and wet right outputs.

Texture is the most interactive and expressive algorithm in the bundle. A static
flowchart explains its parts, but an interactive graph is more comprehensible
because the reader can isolate normal, grain, reverse, and feedback behavior.
