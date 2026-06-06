# Long [sdram]

Long [sdram] adapts `Farmer2K5/daisy-sdram-delaylines` into the bundle through
`DaisyDelayFxCore::ProcessSdramDelaylines`.

The algorithm emphasizes reusable long external-buffer delay behavior. It uses
long base delay time, fractional left and right reads, slow LFO modulation,
secondary warp taps, cross-feedback, and smear blending.

Important entities:

- Long base delay.
- External delay storage.
- Slow LFO modulation.
- Primary left and right reads.
- Secondary warp taps.
- Ping-pong cross feedback.
- Smear blend.
- Wet left and wet right outputs.

This is the best algorithm for explaining memory and delay-line structure
because it is closer to the primitive than the more colored Tape, Tank, and
Texture algorithms.
