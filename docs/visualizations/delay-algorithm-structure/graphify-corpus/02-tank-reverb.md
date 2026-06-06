# Tank [reverb]

Tank [reverb] adapts `Farmer2K5/daisy-reverb-playground` into the bundle
through `DaisyDelayFxCore::ProcessReverbPlayground`.

The algorithm is based on a four-line feedback delay network. It reads four
scaled delay taps from base times of 37 ms, 53 ms, 71 ms, and 89 ms. Each tap is
damped by a one-pole filter. The damped taps are mixed with sum and difference
terms before being written back into the four delay lines.

Important entities:

- Driven mono input.
- Tank size.
- Four delay-line reads.
- Damping filter.
- Diffusion injection.
- Hadamard-style feedback matrix.
- Four feedback writes.
- Width control.
- Early output component.

This algorithm is best visualized as a network rather than as a simple linear
delay because the key behavior is the feedback matrix and the way the taps
reinject into one another.
