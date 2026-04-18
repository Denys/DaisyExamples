# CloudSeed Source Taxonomy (BKS Daisy Port)

Source analyzed:

- `bkshepherd/DaisySeedProjects`
- `Software/GuitarPedal/dependencies/CloudSeed`

This dependency is not a synthesizer voice library. It is a stereo reverb and delay-effect engine with diffusion, filtering, modulation, and seeded decorrelation.

## Signal Flow

`ReverbController` owns two `ReverbChannel` instances and applies stereo input crossmix (`InputMix`) before processing.

Per channel, the audio path is:

`input -> high-pass/low-pass -> pre-delay -> multitap early reflections -> optional early allpass diffusion -> parallel late delay lines -> optional post-diffusion inside each line -> post-EQ inside each line -> output mix`

The final per-channel output is:

`dryOut * dry + predelayOut * predelay + earlyOut * earlyStage + mainOut * lateLines`

## Parameter Surface

`Parameter2` is the full external control surface. The practical split below matches how the code uses it.

### Inputs

These shape the incoming signal and the audible reverb network.

- Input conditioning:
  - `InputMix`
  - `PreDelay`
  - `HighPass`
  - `LowPass`
- Early reflection block:
  - `TapCount`
  - `TapLength`
  - `TapGain`
  - `TapDecay`
  - `DiffusionEnabled`
  - `DiffusionStages`
  - `DiffusionDelay`
  - `DiffusionFeedback`
- Late reverb block:
  - `LineCount`
  - `LineDelay`
  - `LineDecay`
  - `LateDiffusionEnabled`
  - `LateDiffusionStages`
  - `LateDiffusionDelay`
  - `LateDiffusionFeedback`
- Post-frequency shaping:
  - `PostLowShelfGain`
  - `PostLowShelfFrequency`
  - `PostHighShelfGain`
  - `PostHighShelfFrequency`
  - `PostCutoffFrequency`
- Modulation controls:
  - `EarlyDiffusionModAmount`
  - `EarlyDiffusionModRate`
  - `LineModAmount`
  - `LineModRate`
  - `LateDiffusionModAmount`
  - `LateDiffusionModRate`

### Outputs

These are stage-output gains mixed into the final channel sum.

- `DryOut`
- `PredelayOut`
- `EarlyOut`
- `MainOut`

### Internal / Structural

These do not behave like direct tone or time controls. They select topology, decorrelation, or implementation behavior.

- Seed and decorrelation:
  - `TapSeed`
  - `DiffusionSeed`
  - `DelaySeed`
  - `PostDiffusionSeed`
  - `CrossSeed`
- Enable or routing switches:
  - `HiPassEnabled`
  - `LowPassEnabled`
  - `LowShelfEnabled`
  - `HighShelfEnabled`
  - `CutoffEnabled`
  - `LateStageTap`
  - `Interpolation`

## Class Groups By Audio Role

### Top-level effect engine

- `ReverbController`
  - Owns the global parameter array.
  - Scales normalized `Parameter2` values into real DSP values.
  - Routes left/right input and output.
- `ReverbChannel`
  - Owns the full per-channel DSP graph.
  - Applies input filtering, pre-delay, early reflections, diffusion, late lines, and output summing.

### Delay and reverb building blocks

- `ModulatedDelay`
  - Fractional delay with sine-based modulation.
  - Used for pre-delay and late lines.
- `MultitapDiffuser`
  - Early-reflection / multitap echo generator.
  - Produces the first dense tap cluster after pre-delay.
- `AllpassDiffuser`
  - Multi-stage allpass diffusion network.
  - Used both as the early diffuser and as optional late post-diffusion inside each delay line.
- `DelayLine`
  - Composite late-reverb line:
    - feedback delay
    - optional post-diffusion
    - low-shelf / high-shelf / low-pass tone shaping

### Filters and EQ

- `AudioLib::Hp1`
  - First-order high-pass input filter.
- `AudioLib::Lp1`
  - First-order low-pass filter.
  - Used both on input and as the per-line cutoff filter.
- `AudioLib::Biquad2`
  - Generic biquad implementation.
  - Used here as low-shelf and high-shelf filters in each late delay line.

### Modulators

- `ModulatedAllpass`
  - Allpass delay with internal sine modulation and optional interpolation.
- `ModulatedDelay`
  - Delay modulation source consumer for chorus-like movement in the pre-delay and late lines.
- `FastSin`
  - Lookup table used as the internal sine modulator source.

There are no user-facing oscillators, envelopes, or voice generators in this dependency. The only oscillator-like behavior is internal LFO modulation used to move delays and allpass stages.

### Randomization / decorrelation

- `AudioLib::ShaRandom`
  - Deterministic pseudo-random series from seeds.
  - Used to decorrelate tap positions, allpass delays, modulation depth/rate, and per-line timing.
- `AudioLib::ValueTables`
  - Curve tables for converting normalized parameters to meaningful ranges.
  - Used for frequency, gain, decay, and modulation scaling.

### Utilities / non-DSP support

- `CloudSeed::Utils`
  - Buffer ops and gain/db helpers.
- `SHA256`
  - Hash utility used by the random generator support code.

## Notes

- `LineCount` exists in `Parameter2`, but this Daisy fork statically caps the channel to `TotalLineCount = 3`.
- In this port, `LineCount` is treated more like a structural selector than a continuous musical parameter.
- The library is best described as:
  - `effects/reverb`
  - `delay network`
  - `diffusion`
  - `modulated delay`
  - `EQ/filtering`
  - `seeded decorrelation`

