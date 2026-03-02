# Daisy Pod Architecture Ideas

*16 project concepts for Daisy Pod hardware, organized by input type and complexity.*

## Table of Contents

### Part 1: Line In Projects (1-6)
1. [Simple Tremolo](#1-simple-tremolo) â˜…â˜…â˜†â˜†â˜†â˜†â˜†â˜†
2. [Bitcrusher](#2-bitcrusher) â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†
3. [Delay Pedal](#3-delay-pedal) â˜…â˜…â˜…â˜…â˜†â˜†â˜†â˜†
4. [Chorus + Flanger](#4-chorus--flanger) â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†
5. [Multi-FX Chain](#5-multi-fx-chain) â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†
6. [Reverb + Shimmer](#6-reverb--shimmer) â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜…

### Part 2: MIDI IN Projects (7-12)
7. [Mono Synth](#7-mono-synth) â˜…â˜…â˜†â˜†â˜†â˜†â˜†â˜†
8. [FM Synth](#8-fm-synth) â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†
9. [Pluck Synth](#9-pluck-synth) â˜…â˜…â˜…â˜…â˜†â˜†â˜†â˜†
10. [Poly Synth 4-Voice](#10-poly-synth-4-voice) â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†
11. [Drum Synth](#11-drum-synth) â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†
12. [Physical Model Synth](#12-physical-model-synth) â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜…

### Part 3: Line In + MIDI Projects (13-16)
13. [Vocoder Lite](#13-vocoder-lite) â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†
14. [MIDI-Controlled Filter](#14-midi-controlled-filter) â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†
15. [Harmonizer](#15-harmonizer) â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†
16. [Synth + FX Workstation](#16-synth--fx-workstation) â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜…

---

## Pod Hardware Reference

```
â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
â”‚              DAISY POD                  â”‚
â”‚    â”Œâ”€â”€â”€â”                    â”Œâ”€â”€â”€â”       â”‚
â”‚    â”‚ K1â”‚                    â”‚ K2â”‚       â”‚  K1, K2 = Knobs
â”‚    â””â”€â”€â”€â”˜                    â””â”€â”€â”€â”˜       â”‚
â”‚         â”Œâ”€â”€â”€â”        â”Œâ”€â”€â”€â”              â”‚
â”‚         â”‚ B1â”‚        â”‚ B2â”‚              â”‚  B1, B2 = Buttons
â”‚         â””â”€â”€â”€â”˜        â””â”€â”€â”€â”˜              â”‚
â”‚              â—‰ RGB LED                  â”‚
â”‚  AUDIO IN â”€â”€â–º         â”€â”€â–º AUDIO OUT     â”‚
â”‚  MIDI IN â”€â”€â–º                            â”‚
â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
```

---

# Part 1: Line In Projects

---

## 1. Simple Tremolo
**Complexity: â˜…â˜…â˜†â˜†â˜†â˜†â˜†â˜†**

Classic amplitude modulation effect.

```
AUDIO IN â”€â”€â–º TREMOLO â”€â”€â–º AUDIO OUT
             â”‚             â”‚
             â”œâ”€â”€ Rate â—„â”€â”€ K1
             â””â”€â”€ Depth â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†**

Lo-fi destruction: reduce bit depth and sample rate.

```
AUDIO IN â”€â”€â–º BITCRUSH â”€â”€â–º DECIMATOR â”€â”€â–º AUDIO OUT
             â”‚             â”‚
             Bits â—„â”€â”€ K1   Downsample â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜†â˜†â˜†â˜†**

Digital delay with tap tempo.

```
AUDIO IN â”€â”€â–º DELAY LINE â”€â”€â”¬â”€â”€â–º AUDIO OUT
             â–²            â”‚
             â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜ (feedback)

Time â—„â”€â”€ K1 (10ms-1s)    Feedback â—„â”€â”€ K2 (0-95%)
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†**

Dual modulation with mode switch.

```
AUDIO IN â”€â”€â–º [CHORUS / FLANGER] â”€â”€â–º STEREO OUT

Chorus: Depth â—„â”€â”€ K1, Rate â—„â”€â”€ K2
Flanger: Depth â—„â”€â”€ K1, Feedback â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†**

3-stage serial: Overdrive â†’ Delay â†’ Reverb.

```
AUDIO IN â”€â”€â–º OVERDRIVE â”€â”€â–º DELAY â”€â”€â–º REVERB â”€â”€â–º AUDIO OUT
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜…**

Lush reverb with pitch-shifted feedback.

```
AUDIO IN â”€â”€â–º REVERB â”€â”€â–º PITCH SHIFT (+12) â”€â”€â”
                  â–²                          â”‚
                  â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
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
**Complexity: â˜…â˜…â˜†â˜†â˜†â˜†â˜†â˜†**

Simple monophonic synthesizer.

```
MIDI IN â”€â”€â–º OSCILLATOR â”€â”€â–º SVF FILTER â”€â”€â–º ENV â”€â”€â–º AUDIO OUT

Cutoff â—„â”€â”€ K1    Resonance â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†**

Two-operator FM synthesis.

```
MIDI IN â”€â”€â–º FM2 â”€â”€â–º ADSR â”€â”€â–º AUDIO OUT
            â”‚
            â”œâ”€â”€ Index â—„â”€â”€ K1
            â””â”€â”€ Ratio â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜†â˜†â˜†â˜†**

Karplus-Strong plucked string synthesis.

```
MIDI IN â”€â”€â–º PLUCK â”€â”€â–º AUDIO OUT

Decay â—„â”€â”€ K1 (0.8-0.99)    Damping â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†**

4-voice polyphonic with voice stealing.

```
MIDI IN â”€â”€â–º VOICE MANAGER â”€â”€â–º MIX â”€â”€â–º AUDIO OUT
            â”‚
            â”œâ”€â”€ Voice 1 (OSC+FLT+ENV)
            â”œâ”€â”€ Voice 2
            â”œâ”€â”€ Voice 3
            â””â”€â”€ Voice 4

Cutoff â—„â”€â”€ K1 (all)    Res â—„â”€â”€ K2 (all)
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

**Modules:** `Oscillator`Ã—4, `Svf`Ã—4, `Adsr`Ã—4

---

## 11. Drum Synth
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†**

MIDI-triggered drums (C1-B1 = different drums).

```
MIDI IN â”€â”€â–º DRUM MAPPER â”€â”€â–º MIX â”€â”€â–º STEREO OUT
            â”‚
            â”œâ”€â”€ C1: Kick (AnalogBassDrum)
            â”œâ”€â”€ D1: Snare (SynthSnareDrum)
            â”œâ”€â”€ E1: Closed HiHat
            â”œâ”€â”€ F1: Open HiHat
            â””â”€â”€ G1: Clap

Tune â—„â”€â”€ K1    Decay â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜…**

StringVoice + ModalVoice physical modeling.

```
MIDI IN â”€â”€â–º [STRING / MODAL] â”€â”€â–º REVERB â”€â”€â–º STEREO OUT

Brightness â—„â”€â”€ K1    Structure â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜†â˜†â˜†â˜†â˜†**

Audio = modulator, MIDI = carrier.

```
AUDIO IN â”€â”€â–º Envelope Follower â”€â”€â”
                                 â–¼
MIDI IN â”€â”€â–º Carrier OSC â”€â”€â–º VCA â”€â”€â–º Filter â”€â”€â–º OUT

Bands â—„â”€â”€ K1 (4/8/16)    Carrier Mix â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜†â˜†â˜†**

Filter audio with MIDI note = cutoff.

```
AUDIO IN â”€â”€â–º MOOG LADDER â”€â”€â–º AUDIO OUT
             â”‚
             â””â”€â”€ Cutoff â—„â”€â”€ MIDI Note (mtof)
                 Res â—„â”€â”€ MIDI Velocity

Offset â—„â”€â”€ K1    Env Follow â—„â”€â”€ K2
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
**Complexity: â˜…â˜…â˜…â˜…â˜…â˜…â˜†â˜†**

Pitch shift audio based on held MIDI notes.

```
AUDIO IN â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€ DRY â”€â”€â”
           â”‚                           â”‚
           â”œâ”€â”€â–º PitchShift (int 1) â”€â”€â”€â”¤
           â”‚                           â”œâ”€â”€â–º MIX â”€â”€â–º OUT
           â””â”€â”€â–º PitchShift (int 2) â”€â”€â”€â”˜

Dry/Wet â—„â”€â”€ K1    Detune â—„â”€â”€ K2
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

## 16. Synth + FX Workstation (No Audio In)
**Complexity: **********

Internal synth workstation with no external audio input.

### Suggested Oscillator Voice (Grainlet + Particle + Dust)

- `Grainlet`: main pitched body from MIDI note.
- `Particle`: noisy micro-burst layer for texture and attack grit.
- `Dust`: sparse random impulses to excite movement and organic transients.
- Blend suggestion:
  - `voice = 0.62 * Grainlet + 0.24 * Particle + 0.14 * DustEnv`
  - Then `voice -> SVF -> ADSR -> MIXER`.

```
MIDI IN -> VOICE CORE (Grainlet + Particle + Dust) -> MIXER -> FX BUS -> DISTORTION -> STEREO OUT
                  |                                                |
       Timbre/Blend <- K1                                      +-> Chorus
                                                                +-> Delay
                                                                +-> Reverb

FX Amount / Drive <- K2
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> VOX["VOICE CORE: Grainlet + Particle + Dust"]:::audio
    VOX --> FLT["SVF"]:::audio
    FLT --> ENV["ADSR"]:::ctrl
    ENV --> MIX["MIXER"]:::math
    MIX --> FX["FX BUS"]:::audio
    FX --> DIST["DISTORTION"]:::audio
    DIST --> OUT["STEREO OUT"]:::ui

    CH["Chorus"]:::audio
    DLY["Delay"]:::audio
    RVB["Reverb"]:::audio
    FX --> CH
    FX --> DLY
    FX --> RVB
    CH --> DIST
    DLY --> DIST
    RVB --> DIST

    K1["K1: Voice Blend / Timbre"]:::ui --> VOX
    K2["K2: FX Amount / Dist Drive"]:::ui --> DIST

    B1["B1: Voice mode / randomize Dust"]:::ui --> VOX
    B2["B2: FX preset cycle"]:::ui --> FX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

**Modules:** `Grainlet`, `Particle`, `Dust`, `Svf`, `Adsr`, `Chorus`, `DelayLine<>`, `ReverbSc`, `Overdrive`

---
## Summary Table

| # | Project | Complexity | Input | Key Modules |
|---|---------|------------|-------|-------------|
| 1 | Tremolo | â˜…â˜…â˜† | Line | Tremolo |
| 2 | Bitcrusher | â˜…â˜…â˜…â˜† | Line | Bitcrush |
| 3 | Delay | â˜…â˜…â˜…â˜…â˜† | Line | DelayLine |
| 4 | Chorus/Flanger | â˜…â˜…â˜…â˜…â˜…â˜† | Line | Chorus, Flanger |
| 5 | Multi-FX | â˜…â˜…â˜…â˜…â˜…â˜…â˜† | Line | OD, Delay, Reverb |
| 6 | Shimmer Reverb | â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜… | Line | ReverbSc, PitchShift |
| 7 | Mono Synth | â˜…â˜…â˜† | MIDI | Osc, Svf |
| 8 | FM Synth | â˜…â˜…â˜…â˜† | MIDI | Fm2 |
| 9 | Pluck Synth | â˜…â˜…â˜…â˜…â˜† | MIDI | Pluck |
| 10 | Poly Synth | â˜…â˜…â˜…â˜…â˜…â˜† | MIDI | OscÃ—4 |
| 11 | Drum Synth | â˜…â˜…â˜…â˜…â˜…â˜…â˜† | MIDI | DrumSynths |
| 12 | Physical Model | â˜…â˜…â˜…â˜…â˜…â˜…â˜…â˜… | MIDI | StringVoice |
| 13 | Vocoder | â˜…â˜…â˜…â˜† | Both | Osc, EnvFollow |
| 14 | MIDI Filter | â˜…â˜…â˜…â˜…â˜…â˜† | Both | MoogLadder |
| 15 | Harmonizer | â˜…â˜…â˜…â˜…â˜…â˜…â˜† | Both | PitchShifter |
| 16 | Synth+FX (No Audio In) | ********** | MIDI | Grainlet, Particle, Dust, FX, Distortion |

---

**Generated per DAISY_EXPERT_SYSTEM_PROMPT_v5.2 guidelines**

