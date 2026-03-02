# Daisy Pod Architecture Ideas

*16 project concepts for Daisy Pod hardware, organized by input type and complexity.*

## Table of Contents

### Part 1: Line In Projects (1-6)
1. [Simple Tremolo](#1-simple-tremolo) ★★☆☆☆☆☆☆
2. [Bitcrusher](#2-bitcrusher) ★★★☆☆☆☆☆
3. [Delay Pedal](#3-delay-pedal) ★★★★☆☆☆☆
4. [Chorus + Flanger](#4-chorus--flanger) ★★★★★☆☆☆
5. [Multi-FX Chain](#5-multi-fx-chain) ★★★★★★☆☆
6. [Reverb + Shimmer](#6-reverb--shimmer) ★★★★★★★★

### Part 2: MIDI IN Projects (7-12)
7. [Mono Synth](#7-mono-synth) ★★☆☆☆☆☆☆
8. [FM Synth](#8-fm-synth) ★★★☆☆☆☆☆
9. [Pluck Synth](#9-pluck-synth) ★★★★☆☆☆☆
10. [Poly Synth 4-Voice](#10-poly-synth-4-voice) ★★★★★☆☆☆
11. [Drum Synth](#11-drum-synth) ★★★★★★☆☆
12. [Physical Model Synth](#12-physical-model-synth) ★★★★★★★★

### Part 3: Line In + MIDI Projects (13-16)
13. [Vocoder Lite](#13-vocoder-lite) ★★★☆☆☆☆☆
14. [MIDI-Controlled Filter](#14-midi-controlled-filter) ★★★★★☆☆☆
15. [Harmonizer](#15-harmonizer) ★★★★★★☆☆
16. [Synth + FX Workstation](#16-synth--fx-workstation) ★★★★★★★★

---

## Pod Hardware Reference

```
┌─────────────────────────────────────────┐
│              DAISY POD                  │
│    ┌───┐                    ┌───┐       │
│    │ K1│                    │ K2│       │  K1, K2 = Knobs
│    └───┘                    └───┘       │
│         ┌───┐        ┌───┐              │
│         │ B1│        │ B2│              │  B1, B2 = Buttons
│         └───┘        └───┘              │
│              ◉ RGB LED                  │
│  AUDIO IN ──►         ──► AUDIO OUT     │
│  MIDI IN ──►                            │
└─────────────────────────────────────────┘
```

---

# Part 1: Line In Projects

---

## 1. Simple Tremolo
**Complexity: ★★☆☆☆☆☆☆**

Classic amplitude modulation effect.

```
AUDIO IN ──► TREMOLO ──► AUDIO OUT
             │             │
             ├── Rate ◄── K1
             └── Depth ◄── K2
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> TREM["TREMOLO"]:::audio
    TREM --> OUT["AUDIO OUT"]:::ui

    K1["K1: Rate"]:::ui --> TREM
    K2["K2: Depth"]:::ui --> TREM

    B1["B1: Waveform"]:::ui --> TREM
    B2["B2: Bypass"]:::ui --> TREM

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Tremolo`

---

## 2. Bitcrusher
**Complexity: ★★★☆☆☆☆☆**

Lo-fi destruction: reduce bit depth and sample rate.

```
AUDIO IN ──► BITCRUSH ──► DECIMATOR ──► AUDIO OUT
             │             │
             Bits ◄── K1   Downsample ◄── K2
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> BC["BITCRUSH"]:::audio
    BC --> DEC["DECIMATOR"]:::audio
    DEC --> OUT["AUDIO OUT"]:::ui

    K1["K1: Bits"]:::ui --> BC
    K2["K2: Downsample"]:::ui --> DEC

    B1["B1: Mix toggle"]:::ui --> BC
    B2["B2: Bypass"]:::ui --> DEC

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Bitcrush`, `Decimator`

---

## 3. Delay Pedal
**Complexity: ★★★★☆☆☆☆**

Digital delay with tap tempo.

```
AUDIO IN ──► DELAY LINE ──┬──► AUDIO OUT
             ▲            │
             └────────────┘ (feedback)

Time ◄── K1 (10ms-1s)    Feedback ◄── K2 (0-95%)
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> DLY["DELAY LINE"]:::audio
    DLY --> OUT["AUDIO OUT"]:::ui
    DLY -->|Feedback| DLY

    K1["K1: Time"]:::ui --> DLY
    K2["K2: Feedback"]:::ui --> DLY

    B1["B1: Tap Tempo"]:::ui --> DLY
    B2["B2: Bypass"]:::ui --> DLY

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `DelayLine<float, MAX_DELAY>`

---

## 4. Chorus + Flanger
**Complexity: ★★★★★☆☆☆**

Dual modulation with mode switch.

```
AUDIO IN ──► [CHORUS / FLANGER] ──► STEREO OUT

Chorus: Depth ◄── K1, Rate ◄── K2
Flanger: Depth ◄── K1, Feedback ◄── K2
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> CF["CHORUS / FLANGER"]:::audio
    CF --> OUT["STEREO OUT"]:::ui

    K1["K1: Depth"]:::ui --> CF
    K2["K2: Rate/Feedback"]:::ui --> CF

    B1["B1: Mode toggle"]:::ui --> CF
    B2["B2: Bypass"]:::ui --> CF

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Chorus`, `Flanger`

---

## 5. Multi-FX Chain
**Complexity: ★★★★★★☆☆**

3-stage serial: Overdrive → Delay → Reverb.

```
AUDIO IN ──► OVERDRIVE ──► DELAY ──► REVERB ──► AUDIO OUT
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> OVR["OVERDRIVE"]:::audio
    OVR --> DLY["DELAY"]:::audio
    DLY --> RVB["REVERB"]:::audio
    RVB --> OUT["AUDIO OUT"]:::ui

    K1["K1/K2: Edit selected FX params"]:::ui --> OVR
    K1 --> DLY
    K1 --> RVB

    B1["B1: Cycle active FX"]:::ui --> OVR
    B2["B2: Bypass current FX"]:::ui --> OVR

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Overdrive`, `DelayLine<>`, `ReverbSc`

---

## 6. Reverb + Shimmer
**Complexity: ★★★★★★★★**

Lush reverb with pitch-shifted feedback.

```
AUDIO IN ──► REVERB ──► PITCH SHIFT (+12) ──┐
                  ▲                          │
                  └──────────────────────────┘
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> RVB["REVERB"]:::audio
    RVB --> PSH["PITCH SHIFT (+12)"]:::audio
    PSH --> RVB

    K1["K1: Decay"]:::ui --> RVB
    K2["K2: Shimmer Amount"]:::ui --> PSH

    B1["B1: Shimmer interval"]:::ui --> PSH
    B2["B2: Bypass"]:::ui --> PSH

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `ReverbSc`, `PitchShifter`

---

# Part 2: MIDI IN Projects

---

## 7. Mono Synth
**Complexity: ★★☆☆☆☆☆☆**

Simple monophonic synthesizer.

```
MIDI IN ──► OSCILLATOR ──► SVF FILTER ──► ENV ──► AUDIO OUT

Cutoff ◄── K1    Resonance ◄── K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> OSC["OSCILLATOR"]:::audio
    OSC --> SVF["SVF FILTER"]:::audio
    SVF --> ENV["ENV"]:::ctrl
    ENV --> OUT["AUDIO OUT"]:::ui

    K1["K1: Cutoff"]:::ui --> SVF
    K2["K2: Resonance"]:::ui --> SVF

    B1["B1: Waveform cycle"]:::ui --> OSC
    B2["B2: Octave shift"]:::ui --> OSC

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Oscillator`, `Svf`, `AdEnv`

---

## 8. FM Synth
**Complexity: ★★★☆☆☆☆☆**

Two-operator FM synthesis.

```
MIDI IN ──► FM2 ──► ADSR ──► AUDIO OUT
            │
            ├── Index ◄── K1
            └── Ratio ◄── K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> FM["FM2"]:::audio
    FM --> ADSR["ADSR"]:::ctrl
    ADSR --> OUT["AUDIO OUT"]:::ui

    K1["K1: Index"]:::ui --> FM
    K2["K2: Ratio"]:::ui --> FM

    B1["B1: Preset"]:::ui --> FM
    B2["B2: Octave"]:::ui --> FM

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Fm2`, `Adsr`

---

## 9. Pluck Synth
**Complexity: ★★★★☆☆☆☆**

Karplus-Strong plucked string synthesis.

```
MIDI IN ──► PLUCK ──► AUDIO OUT

Decay ◄── K1 (0.8-0.99)    Damping ◄── K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> PLUCK["PLUCK"]:::audio
    PLUCK --> OUT["AUDIO OUT"]:::ui

    K1["K1: Decay"]:::ui --> PLUCK
    K2["K2: Damping"]:::ui --> PLUCK

    B1["B1: Mode"]:::ui --> PLUCK
    B2["B2: Sustain toggle"]:::ui --> PLUCK

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Pluck`

---

## 10. Poly Synth 4-Voice
**Complexity: ★★★★★☆☆☆**

4-voice polyphonic with voice stealing.

```
MIDI IN ──► VOICE MANAGER ──► MIX ──► AUDIO OUT
            │
            ├── Voice 1 (OSC+FLT+ENV)
            ├── Voice 2
            ├── Voice 3
            └── Voice 4

Cutoff ◄── K1 (all)    Res ◄── K2 (all)
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> VMGR["VOICE MANAGER"]:::math
    VMGR --> MIX["MIX"]:::math
    MIX --> OUT["AUDIO OUT"]:::ui

    V1["Voice 1 (OSC+FLT+ENV)"]:::audio
    V2["Voice 2"]:::audio
    V3["Voice 3"]:::audio
    V4["Voice 4"]:::audio

    VMGR --> V1
    VMGR --> V2
    VMGR --> V3
    VMGR --> V4
    V1 --> MIX
    V2 --> MIX
    V3 --> MIX
    V4 --> MIX

    K1["K1: Cutoff (all)"]:::ui --> VMGR
    K2["K2: Res (all)"]:::ui --> VMGR

    B1["B1: Waveform"]:::ui --> VMGR
    B2["B2: Unison detune"]:::ui --> VMGR

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Oscillator`×4, `Svf`×4, `Adsr`×4

---

## 11. Drum Synth
**Complexity: ★★★★★★☆☆**

MIDI-triggered drums (C1-B1 = different drums).

```
MIDI IN ──► DRUM MAPPER ──► MIX ──► STEREO OUT
            │
            ├── C1: Kick (AnalogBassDrum)
            ├── D1: Snare (SynthSnareDrum)
            ├── E1: Closed HiHat
            ├── F1: Open HiHat
            └── G1: Clap

Tune ◄── K1    Decay ◄── K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> DMAP["DRUM MAPPER"]:::math
    DMAP --> MIX["MIX"]:::math
    MIX --> OUT["STEREO OUT"]:::ui

    KICK["C1: Kick"]:::audio
    SNARE["D1: Snare"]:::audio
    HH["E1: Closed HiHat"]:::audio
    OH["F1: Open HiHat"]:::audio
    CLAP["G1: Clap"]:::audio

    DMAP --> KICK
    DMAP --> SNARE
    DMAP --> HH
    DMAP --> OH
    DMAP --> CLAP
    KICK --> MIX
    SNARE --> MIX
    HH --> MIX
    OH --> MIX
    CLAP --> MIX

    K1["K1: Tune"]:::ui --> DMAP
    K2["K2: Decay"]:::ui --> DMAP

    B1["B1: Kit select"]:::ui --> DMAP
    B2["B2: Accent"]:::ui --> DMAP

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `AnalogBassDrum`, `SynthSnareDrum`, `HiHat<>`

---

## 12. Physical Model Synth
**Complexity: ★★★★★★★★**

StringVoice + ModalVoice physical modeling.

```
MIDI IN ──► [STRING / MODAL] ──► REVERB ──► STEREO OUT

Brightness ◄── K1    Structure ◄── K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> PHYS["STRING / MODAL"]:::audio
    PHYS --> RVB["REVERB"]:::audio
    RVB --> OUT["STEREO OUT"]:::ui

    K1["K1: Brightness"]:::ui --> PHYS
    K2["K2: Structure"]:::ui --> PHYS

    B1["B1: Model"]:::ui --> PHYS
    B2["B2: Sustain"]:::ui --> PHYS

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules (LGPL):** `StringVoice`, `ModalVoice`, `ReverbSc`

---

# Part 3: Line In + MIDI Projects

---

## 13. Vocoder Lite
**Complexity: ★★★☆☆☆☆☆**

Audio = modulator, MIDI = carrier.

```
AUDIO IN ──► Envelope Follower ──┐
                                 ▼
MIDI IN ──► Carrier OSC ──► VCA ──► Filter ──► OUT

Bands ◄── K1 (4/8/16)    Carrier Mix ◄── K2
```
```mermaid
flowchart TD
    AUD["AUDIO IN"]:::ui --> ENV["Envelope Follower"]:::ctrl
    MIDI["MIDI IN"]:::ui --> OSC["Carrier OSC"]:::audio
    OSC --> VCA["VCA"]:::audio
    VCA --> FIL["Filter"]:::audio
    FIL --> OUT["OUT"]:::ui
    ENV --> VCA

    K1["K1: Bands"]:::ui --> FIL
    K2["K2: Carrier Mix"]:::ui --> OSC

    B1["B1: Freeze"]:::ui --> ENV
    B2["B2: Bypass"]:::ui --> FIL

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Oscillator`, `Svf`

---

## 14. MIDI-Controlled Filter
**Complexity: ★★★★★☆☆☆**

Filter audio with MIDI note = cutoff.

```
AUDIO IN ──► MOOG LADDER ──► AUDIO OUT
             │
             └── Cutoff ◄── MIDI Note (mtof)
                 Res ◄── MIDI Velocity

Offset ◄── K1    Env Follow ◄── K2
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> MOOG["MOOG LADDER"]:::audio
    MOOG --> OUT["AUDIO OUT"]:::ui

    MIDI["MIDI IN"]:::ui --> MOOG
    MIDI -->|mtof| MOOG
    MIDI -->|Velocity| MOOG

    K1["K1: Offset"]:::ui --> MOOG
    K2["K2: Env Follow"]:::ui --> MOOG

    B1["B1: Filter type"]:::ui --> MOOG
    B2["B2: Key tracking"]:::ui --> MOOG

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `MoogLadder`, `Svf`

---

## 15. Harmonizer
**Complexity: ★★★★★★☆☆**

Pitch shift audio based on held MIDI notes.

```
AUDIO IN ──┬──────────────────── DRY ──┐
           │                           │
           ├──► PitchShift (int 1) ───┤
           │                           ├──► MIX ──► OUT
           └──► PitchShift (int 2) ───┘

Dry/Wet ◄── K1    Detune ◄── K2
```
```mermaid
flowchart TD
    IN["AUDIO IN"]:::ui --> SPL["Split"]
    SPL --> DRY["DRY"]:::audio
    SPL --> PSH1["PitchShift (int 1)"]:::audio
    SPL --> PSH2["PitchShift (int 2)"]:::audio
    PSH1 --> MIX["MIX"]:::math
    PSH2 --> MIX
    DRY --> MIX
    MIX --> OUT["OUT"]:::ui

    K1["K1: Dry/Wet"]:::ui --> MIX
    K2["K2: Detune"]:::ui --> PSH1
    K2 --> PSH2

    B1["B1: Mode"]:::ui --> PSH1
    B2["B2: Bypass"]:::ui --> MIX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `PitchShifter`

---

## 16. Synth + FX Workstation
**Complexity: ★★★★★★★★**

4-voice poly synth + audio input through shared FX.

```
AUDIO IN ──► Gate ──┐
                    ▼
MIDI IN ──► POLY ──► MIXER ──► FX BUS ──► STEREO OUT
                     │         │
          Balance ◄──K1       ├── Chorus
                              ├── Delay
                              └── Reverb

FX Amount ◄── K2
```
```mermaid
flowchart TD
    AUD["AUDIO IN"]:::ui --> GATE["Gate"]:::ctrl
    MIDI["MIDI IN"]:::ui --> POLY["POLY"]:::math
    POLY --> MIX["MIXER"]:::math
    GATE --> MIX
    MIX --> FX["FX BUS"]:::audio
    FX --> OUT["STEREO OUT"]:::ui

    CH["Chorus"]:::audio
    DLY["Delay"]:::audio
    RVB["Reverb"]:::audio

    FX --> CH
    FX --> DLY
    FX --> RVB
    CH --> OUT
    DLY --> OUT
    RVB --> OUT

    K1["K1: Balance"]:::ui --> MIX
    K2["K2: FX Amount"]:::ui --> FX

    B1["B1: Audio gate"]:::ui --> GATE
    B2["B2: FX select"]:::ui --> FX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Oscillator`×4, `Svf`×4, `Adsr`×4, `Chorus`, `DelayLine<>`, `ReverbSc`

---

## Summary Table

| # | Project | Complexity | Input | Key Modules |
|---|---------|------------|-------|-------------|
| 1 | Tremolo | ★★☆ | Line | Tremolo |
| 2 | Bitcrusher | ★★★☆ | Line | Bitcrush |
| 3 | Delay | ★★★★☆ | Line | DelayLine |
| 4 | Chorus/Flanger | ★★★★★☆ | Line | Chorus, Flanger |
| 5 | Multi-FX | ★★★★★★☆ | Line | OD, Delay, Reverb |
| 6 | Shimmer Reverb | ★★★★★★★★ | Line | ReverbSc, PitchShift |
| 7 | Mono Synth | ★★☆ | MIDI | Osc, Svf |
| 8 | FM Synth | ★★★☆ | MIDI | Fm2 |
| 9 | Pluck Synth | ★★★★☆ | MIDI | Pluck |
| 10 | Poly Synth | ★★★★★☆ | MIDI | Osc×4 |
| 11 | Drum Synth | ★★★★★★☆ | MIDI | DrumSynths |
| 12 | Physical Model | ★★★★★★★★ | MIDI | StringVoice |
| 13 | Vocoder | ★★★☆ | Both | Osc, EnvFollow |
| 14 | MIDI Filter | ★★★★★☆ | Both | MoogLadder |
| 15 | Harmonizer | ★★★★★★☆ | Both | PitchShifter |
| 16 | Synth+FX | ★★★★★★★★ | Both | All combined |

---

**Generated per DAISY_EXPERT_SYSTEM_PROMPT_v5.2 guidelines**
