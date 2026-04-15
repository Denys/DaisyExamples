# CloudSeedCore Interactive DSP Block Diagram (Daisy Bridge)

This schematic describes an interactive CloudSeedCore-style reverb architecture for Daisy Seed/Field integration.

- Shows **audio signal flow** from input to output.
- Breaks down the engine into main FX stages.
- Expands each stage into DSP primitives/classes.
- Documents control/parameter ownership by block.

---

## 1) Top-Level Signal Flow

```mermaid
flowchart LR
    IN_L[Input L] --> INMIX
    IN_R[Input R] --> INMIX

    subgraph PRE[Input + Preconditioning]
      INMIX[InputMix]
      HPF[LowCut / HPF]
      LPF[HighCut / LPF]
      INMIX --> HPF --> LPF
    end

    LPF --> TAPS
    LPF --> EARLY

    subgraph EARLY_PATH[Early Reflection Path]
      TAPS[Multitap Reflections]
      EARLYDIFF[Early Diffusion Network]
      TAPS --> EARLYDIFF
    end

    EARLYDIFF --> LATE

    subgraph LATE_PATH[Late Reverb Path]
      LATE[Late FDN / Delay Matrix]
      POSTDIFF[Post/Late Diffusion]
      MOD[Modulation Layer]
      LATE --> POSTDIFF --> MOD
    end

    MOD --> EQ

    subgraph TONE[Output EQ / Spectral Shaping]
      EQ[Low Shelf + High Shelf + Lowpass]
    end

    EQ --> WETMIX

    subgraph MIXOUT[Mix / Routing]
      DRY[DryOut Gain]
      EOUT[EarlyOut Gain]
      LOUT[LateOut Gain]
      WETMIX[Wet Bus]
      SUM[Output Summing]
      WETMIX --> EOUT --> SUM
      WETMIX --> LOUT --> SUM
      INMIX --> DRY --> SUM
    end

    SUM --> OUT_L[Output L]
    SUM --> OUT_R[Output R]
```

---

## 2) Interactive Control Plane (Parameter Domains)

```mermaid
flowchart TB
    UI[Hardware UI: Seed / Field / MIDI] --> CM[ControlModel 0..1 normalized]
    CM --> PERF[CloudSeedPerformance]

    PERF --> G[CloudSeedGlobal]
    PERF --> T[CloudSeedTap]
    PERF --> ED[CloudSeedEarlyDiffusion]
    PERF --> LR[CloudSeedLateReverb]
    PERF --> EQ[CloudSeedEq]
    PERF --> S[CloudSeedSeeds]

    G --> DSP[CloudSeed DSP Engine]
    T --> DSP
    ED --> DSP
    LR --> DSP
    EQ --> DSP
    S --> DSP
```

---

## 3) Expandable DSP Primitives by Main FX Block

## 3.1 Input + Preconditioning (Global)

```mermaid
flowchart LR
    X[Input stereo/mono] --> MIX[InputMix]
    MIX --> HP[HPF one-pole/bq]
    HP --> LP[LPF one-pole/bq]
    LP --> Y[Conditioned input]
```

**DSP primitives/classes**
- Gain stage (`InputMix`).
- High-pass filter (`LowCutEnabled`, `LowCut`).
- Low-pass filter (`HighCutEnabled`, `HighCut`).

**Controls (CloudSeedGlobal)**
- `INTERPOLATION_PARAM`
- `INPUT_MIX_PARAM`
- `LOW_CUT_ENABLED_PARAM`, `LOW_CUT_PARAM`
- `HIGH_CUT_ENABLED_PARAM`, `HIGH_CUT_PARAM`
- `DRY_OUT_PARAM`, `EARLY_OUT_PARAM`, `LATE_OUT_PARAM`

---

## 3.2 Multitap Reflections (Tap Block)

```mermaid
flowchart LR
    X[Conditioned input] --> TD[Tap delay lines]
    TD --> TG[Tap gain envelope / decay law]
    TG --> TSUM[Tap sum]
```

**DSP primitives/classes**
- Parallel short delay lines.
- Per-tap gain/decay scaling.
- Optional tap predelay.

**Controls (CloudSeedTap)**
- `TAP_ENABLED_PARAM`
- `TAP_COUNT_PARAM`
- `TAP_DECAY_PARAM`
- `TAP_PREDELAY_PARAM`
- `TAP_LENGTH_PARAM`

---

## 3.3 Early Diffusion Network

```mermaid
flowchart LR
    X[Tap sum / direct feed] --> AP1[Allpass stage 1]
    AP1 --> AP2[Allpass stage 2]
    AP2 --> AP3[Allpass stage N]
    AP3 --> FB[Feedback mix]
    FB --> OUT[Early diffuse out]
    MOD[Slow delay modulation] --> AP1
    MOD --> AP2
    MOD --> AP3
```

**DSP primitives/classes**
- Cascaded allpass diffusers.
- Diffusion feedback path.
- Delay-time modulation (LFO/random).

**Controls (CloudSeedEarlyDiffusion)**
- `EARLY_DIFFUSE_ENABLED_PARAM`
- `EARLY_DIFFUSE_COUNT_PARAM`
- `EARLY_DIFFUSE_DELAY_PARAM`
- `EARLY_DIFFUSE_MOD_AMOUNT_PARAM`
- `EARLY_DIFFUSE_FEEDBACK_PARAM`
- `EARLY_DIFFUSE_MOD_RATE_PARAM`

---

## 3.4 Late Reverb Core (FDN / Delay Matrix)

```mermaid
flowchart LR
    IN[Early diffuse in] --> DL1[Delay line 1]
    IN --> DL2[Delay line 2]
    IN --> DLN[Delay line N]

    DL1 --> MX[Feedback matrix / Householder / Hadamard]
    DL2 --> MX
    DLN --> MX

    MX --> DAMP[Damping filters per line]
    DAMP --> MOD[Line modulation]
    MOD --> DEC[Per-line decay gains]
    DEC --> OUT[Late tail bus]

    OUT --> FBRET[Feedback return]
    FBRET --> DL1
    FBRET --> DL2
    FBRET --> DLN
```

**DSP primitives/classes**
- Delay-bank (`LateLineCount`, `LateLineSize`).
- Feedback mixing matrix (`LateMode`).
- Per-line damping filters.
- Delay-line modulation (`LateLineModAmount`, `LateLineModRate`).
- Decay gains (`LateLineDecay`).
- Optional late diffusion chain.

**Controls (CloudSeedLateReverb)**
- `LATE_MODE_PARAM`
- `LATE_LINE_COUNT_PARAM`
- `LATE_DIFFUSE_ENABLED_PARAM`
- `LATE_DIFFUSE_COUNT_PARAM`
- `LATE_LINE_SIZE_PARAM`
- `LATE_LINE_MOD_AMOUNT_PARAM`
- `LATE_DIFFUSE_DELAY_PARAM`
- `LATE_DIFFUSE_MOD_AMOUNT_PARAM`
- `LATE_LINE_DECAY_PARAM`
- `LATE_LINE_MOD_RATE_PARAM`
- `LATE_DIFFUSE_FEEDBACK_PARAM`
- `LATE_DIFFUSE_MOD_RATE_PARAM`

---

## 3.5 Output EQ / Spectral Section

```mermaid
flowchart LR
    X[Wet tail] --> LS[Low shelf]
    LS --> HS[High shelf]
    HS --> LP[Lowpass]
    LP --> Y[EQ out]
```

**DSP primitives/classes**
- Shelving filters.
- Final low-pass roll-off.
- Cross-seed logic for parameter randomization coupling.

**Controls (CloudSeedEq)**
- `EQ_LOW_SHELF_ENABLED_PARAM`
- `EQ_HIGH_SHELF_ENABLED_PARAM`
- `EQ_LOWPASS_ENABLED_PARAM`
- `EQ_LOW_FREQ_PARAM`
- `EQ_HIGH_FREQ_PARAM`
- `EQ_CUTOFF_PARAM`
- `EQ_LOW_GAIN_PARAM`
- `EQ_HIGH_GAIN_PARAM`
- `EQ_CROSS_SEED_PARAM`

---

## 3.6 Seed/Randomization Control Layer

```mermaid
flowchart LR
    TRIG[Randomize trigger] --> RNG[Seed generator]
    RNG --> ST[SeedTap]
    RNG --> SDIF[SeedDiffusion]
    RNG --> SDL[SeedDelay]
    RNG --> SPD[SeedPostDiffusion]

    ST --> TAPCFG[Tap config]
    SDIF --> EDCFG[Early diffusion config]
    SDL --> LCFG[Late delay config]
    SPD --> PDCFG[Post diffusion config]
```

**DSP primitives/classes**
- Deterministic seedable RNG.
- Block-local seed routing (tap/diffusion/delay/post-diffusion).

**Controls (CloudSeedSeeds)**
- `SEED_TAP_PARAM`
- `SEED_DIFFUSION_PARAM`
- `SEED_DELAY_PARAM`
- `SEED_POST_DIFFUSION_PARAM`

---

## 4) Daisy Performance Macro Mapping (Current 8 Knobs)

| Performance Param | Suggested CloudSeed Domain |
|---|---|
| `MIX_PARAM` | `INPUT_MIX_PARAM` / wet blend macro |
| `SIZE_PARAM` | `LATE_LINE_SIZE_PARAM` |
| `DECAY_PARAM` | `LATE_LINE_DECAY_PARAM` |
| `DIFFUSION_PARAM` | `EARLY_DIFFUSE_FEEDBACK_PARAM` + `LATE_DIFFUSE_FEEDBACK_PARAM` macro |
| `PRE_DELAY_PARAM` | `TAP_PREDELAY_PARAM` |
| `DAMPING_PARAM` | `HIGH_CUT_PARAM` + damping filters macro |
| `MODULATION_PARAM` | `LATE_LINE_MOD_AMOUNT_PARAM` + `EARLY_DIFFUSE_MOD_AMOUNT_PARAM` |
| `MODULATION_RATE_PARAM` | `LATE_LINE_MOD_RATE_PARAM` + `EARLY_DIFFUSE_MOD_RATE_PARAM` |

---

## 5) Practical Class/Module Decomposition (for implementation)

- `InputConditioner`
  - Gain + HP/LP filters.
- `TapReflections`
  - Multitap delays and decay curves.
- `EarlyDiffuser`
  - Cascaded allpass network + modulation.
- `LateReverbCore`
  - Delay-bank, feedback matrix, damping, modulation, decay.
- `OutputEq`
  - Shelf + lowpass chain.
- `SeedRouter`
  - Seeded randomization and per-block dispatch.
- `CloudSeedEngineAdapter`
  - Bridges `ControlModel` / per-class enums to DSP internals.

This decomposition aligns with the enum classes in `CloudSeedInteractiveParameters.h` and keeps Seed/Field control mapping independent from DSP internals.
