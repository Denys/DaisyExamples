# Phantasmagoria [FuzzyLotus]

Phantasmagoria is an external reference project from
`FuzzyLotus/Phantasmagoria`, not currently one of the four Field delay bundle
firmware modes. Its README describes a spectral delay and echo chamber for Daisy
Seed / PedalPCB Terrarium with reverse delay, room mode, halo mode, freeze,
freeze evolution, reverb, and hi-fi dynamics.

Source extraction from `phantasmagoria.cpp` produced these named algorithm
blocks:

- Tri-LFO Tape Warble: three sine LFOs modulate delay read time for tape-like
  movement.
- Main Spectral Delay Line: dry plus feedback write into main SDRAM delay,
  smoothed forward read, and feedback return.
- Reverse Dual-Grain Reader: dual windowed grain read from the main delay buffer
  crossfaded with the forward delay by SW1.
- Smear Multi-Tap Diffusion: SW2 blends +10 ms and +25 ms taps into `delRd` so
  temporal smear is audible and feeds back.
- Erosion Repeat Aging Filter: SW3 sweeps a low-pass filter and attenuates
  `delRd` so repeats progressively darken.
- Echo Chamber Reverb Taps: separate reverb delay with fixed taps at 83, 151,
  227, and 311 ms.
- Freeze Voice Bank: three freeze buffers at 97, 149, and 199 ms hold or
  accumulate frozen audio.
- Freeze Evolution Drift: SW4 adds ultra-slow drift to frozen voices when freeze
  is active.
- Hi-Fi Dynamics Soft Clip: feedback and output soft saturation preserve smaller
  transients while limiting runaway levels.
- Constant-Power Dry/Wet Mixer: cosine/sine weighting combines dry and wet
  signals before final output.
