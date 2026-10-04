# Synthesis

All four algorithms are original in this package, not ports of the manuals or unverified community DSP.
The shared 1,025-entry sine table is initialized once before audio starts. Noise is xorshift32
with per-voice deterministic seeds. No audio processing requests random bytes from a peripheral.
Hammer uses a unipolar exponentially decaying pitch transient; Character scales its amount.
Crack blends two sine modes with high-passed noise. Steel sums three products of six sine
partials and adds shaped noise. Arc uses feed-forward phase modulation; its Character changes
modulator ratio and index, not a feedback gain.

The attack is 1 ms for Hammer/Crack/Arc and 0.5 ms for Steel. Decay is a T60-like parameter:
coefficient = exp(-ln(1000)/(Fs*T60)). Release is a 5 ms linear ramp from the current envelope
value to exact zero. Retrigger starts from the current amplitude and retains oscillator phase.
The pitch/modulation transient has its own 1 ms attack from current state, avoiding an
instantaneous phase-modulation jump on Arc retrigger. Frequency, Character, Level and the
noise-filter coefficient use 5 ms ramps; changing a decay coefficient changes slope,
not the current envelope value. The noise-filter coefficient is bounded between zero and one.
The master is capped by the mathematical output path; this is not a true-peak/lookahead limiter.
Harmonics from phase modulation, ring products and saturation still require aliasing/listening QA.
No resonant filter or delay feedback loop is present in this candidate.

```mermaid
flowchart TB
    Trig["Step or MIDI trigger / velocity"] --> Env["Finite attack-decay envelope"]
    Params["Tune / Decay / Character / Level"] --> Smooth["Validated targets / 5 ms ramps"]
    Smooth --> H
    Smooth --> C
    Smooth --> S
    Smooth --> A
    subgraph H["Hammer"]
        HP["Exponential pitch transient"] --> HS["Sine + small second harmonic"]
    end
    subgraph C["Crack"]
        CT["Two inharmonic sine modes"] --> CM["Body-noise mix"]
        CN["Seeded noise / one-pole highpass"] --> CM
    end
    subgraph S["Steel"]
        SO["Six sine oscillators"] --> SR["Three pairwise products"]
        SN["Seeded highpass noise"] --> SM["Metal-noise mix"]
        SR --> SM
    end
    subgraph A["Arc"]
        AM["Sine modulator / bounded ratio"] --> AP["Decaying phase-modulation index"]
        AP --> AC["Sine carrier"]
    end
    H --> Amp["Envelope x level x mute ramp"]
    C --> Amp
    S --> Amp
    A --> Amp
    Env --> Amp
    Amp --> Pan["Fixed linear pan / stereo sum"]
    Pan --> Gain["Drive gain"]
    Gain --> DC["10 Hz DC blocker"]
    DC --> Sat["Bounded feed-forward saturator"]
    Sat --> Master["Master + preset fade"]
    Master --> LR["StereoFrame"]
```
