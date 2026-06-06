# Tape [multifx]

Tape [multifx] adapts `balazsbencs/daisy-multifx-pedal` into the bundle through
`DaisyDelayFxCore::ProcessMultiFx`.

The algorithm is based on a tape-style circular delay line. It uses smoothed
delay time, stereo read offsets, flutter modulation, feedback tone filtering,
FastTanh saturation, grit blending, and freeze-aware delay writes.

Important entities:

- Driven mono input.
- Delay-time smoothing.
- Flutter LFO.
- Left and right delay reads.
- Tone one-pole state.
- Grit saturation.
- Feedback amount.
- Freeze amount.
- Delay line writes.
- Wet left and wet right outputs.

This is the clearest pedal-style algorithm in the bundle because its sound
identity maps directly to common controls: delay time, feedback, tone, grit,
modulation, width, rhythm, mix, input drive, and output gain.
