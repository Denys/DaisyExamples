# Daisy Field Architecture Ideas

## Table of Contents

### Part 1: Keyboard-Based Concepts (1-10)
1. [Drone Station](#1-drone-station-beginner) ★☆☆☆☆
2. [Subtractive Monosynth](#2-subtractive-monosynth-beginner) ★★☆☆☆
3. [Mini Drum Machine](#3-mini-drum-machine-intermediate) ★★☆☆☆
4. [Delay + Reverb FX](#4-delay--reverb-effect-unit-intermediate) ★★★☆☆
5. [FM Synthesizer](#5-fm-synthesizer-intermediate) ★★★☆☆
6. [StringVoice Synth](#6-physical-modeling-stringvoice-intermediate) ★★★☆☆
7. [Granular Texture](#7-granular-texture-generator-advanced) ★★★★☆
8. [Poly Modal Synth](#8-polyphonic-modal-synthesizer-advanced) ★★★★☆
9. [Formant Vowel Synth](#9-formant-vowel-synthesizer-advanced) ★★★★☆
10. [Performance Workstation](#10-complete-performance-workstation-expert) ★★★★★

### Part 3: Modular Synthesizer Integration (21-23)
21. [Basic Modular Voice](#21-basic-modular-voice-complexity-) ★★★☆☆
22. [Modular FX + Mixer Hub](#22-modular-fx--mixer-hub-complexity-) ★★★★★☆
23. [Complete Modular Workstation](#23-complete-modular-workstation-complexity-) ★★★★★★★★☆☆

---

# Part 1: Keyboard-Based Concepts


## 1. Drone Station (Beginner)
**Complexity: ★☆☆☆☆**

8 oscillators controlled by keyboard. Press keys to layer drones.

```
┌──────────────────────────────────────────────────────┐
│                   KEYBOARD (16 keys)                 │
└───────────────────────┬──────────────────────────────┘
                         │ KeyState[0-15]
                         ▼
┌──────────────────────────────────────────────────────┐
│  VOICE BANK (8 oscillators)                          │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐            │
│  │ OSC │ │ OSC │ │ OSC │ │ OSC │ │...  │            │
│  └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘ └──┬──┘            │
│     └───────┴───────┴───────┴───────┘                │
│                   │ Mix                              │
└───────────────────┼──────────────────────────────────┘
                    ▼
              ┌───────────┐
              │  OUTPUT   │
              └───────────┘
```
```mermaid
flowchart TD
    KB["KEYBOARD (16 keys)"]:::ui --> STATE["KeyState[0-15]"]:::math
    STATE --> VB["VOICE BANK (8 oscillators)"]:::audio
    VB --> MIX["Mix"]:::math
    MIX --> OUT["OUTPUT"]:::ui

    K1["K1: Volume"]:::ui --> VB
    K2["K2: Detune"]:::ui --> VB
    K3["K3: Waveform"]:::ui --> VB
    K4["K4: Pan Spread"]:::ui --> MIX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 2. Subtractive Monosynth (Beginner)
**Complexity: ★★☆☆☆**

Classic synth: Osc → Filter → Amp Envelope.

```
┌──────────────────────────────────────────────────────┐
│                    KEYBOARD                          │
└───────────────────────┬──────────────────────────────┘
                         │ Note
         ┌───────────────┴───────────────┐
         ▼                               ▼
   ┌───────────┐                   ┌───────────┐
   │   OSC 1   │                   │   OSC 2   │
   │   (SAW)   │                   │  (PULSE)  │
   └─────┬─────┘                   └─────┬─────┘
         │                               │
         └───────────┬───────────────────┘
                     │ MIX
                     ▼
           ┌───────────────────┐
           │     SVF FILTER    │◄── Cutoff (K1), Res (K2)
           └─────────┬─────────┘
                     ▼
           ┌───────────────────┐
           │       ADSR        │◄── A(K3) D(K4) S(K5) R(K6)
           └─────────┬─────────┘
                     ▼
               ┌───────────┐
               │  OUTPUT   │
               └───────────┘
```
```mermaid
flowchart TD
    KB["KEYBOARD"]:::ui --> NOTE["Note"]:::ctrl
    NOTE --> OSC1["OSC 1 (SAW)"]:::audio
    NOTE --> OSC2["OSC 2 (PULSE)"]:::audio
    OSC1 --> MIX["MIX"]:::math
    OSC2 --> MIX
    MIX --> SVF["SVF FILTER"]:::audio
    SVF --> ADSR["ADSR"]:::ctrl
    ADSR --> OUT["OUTPUT"]:::ui

    K1["K1: Cutoff"]:::ui --> SVF
    K2["K2: Res"]:::ui --> SVF
    K3["K3: A"]:::ui --> ADSR
    K4["K4: D"]:::ui --> ADSR
    K5["K5: S"]:::ui --> ADSR
    K6["K6: R"]:::ui --> ADSR

    SW1["SW1: Octave shift"]:::ui --> OSC1
    SW2["SW2: Octave shift"]:::ui --> OSC2

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 3. Mini Drum Machine (Intermediate)
**Complexity: ★★☆☆☆**

4-voice drums with pattern sequencer.

```
┌──────────────────────────────────────────────────────┐
│                    METRO CLOCK                       │
│                  (Tempo: Knob 7)                     │
└───────────────────────┬──────────────────────────────┘
                         │ Tick
         ┌───────────────┼───────────────┐
         ▼               ▼               ▼
   ┌───────────┐   ┌───────────┐   ┌───────────┐
   │ PATTERN   │   │ PATTERN   │   │ PATTERN   │
   │   KICK    │   │  SNARE    │   │   HAT     │
   └─────┬─────┘   └─────┬─────┘   └─────┬─────┘
         ▼               ▼               ▼
   ┌───────────┐   ┌───────────┐   ┌───────────┐
   │ AnalogBD  │   │ SynthSnr  │   │  HiHat    │
   └─────┬─────┘   └─────┬─────┘   └─────┬─────┘
         └───────────────┼───────────────┘
                         │ MIX
                         ▼
                   ┌───────────┐
                   │  OUTPUT   │
                   └───────────┘
```
```mermaid
flowchart TD
    CLK["METRO CLOCK (Tempo: K7)"]:::ctrl --> TICK["Tick"]:::ctrl
    TICK --> PAT_K["PATTERN KICK"]:::math
    TICK --> PAT_S["PATTERN SNARE"]:::math
    TICK --> PAT_H["PATTERN HAT"]:::math
    PAT_K --> V_K["AnalogBD"]:::audio
    PAT_S --> V_S["SynthSnr"]:::audio
    PAT_H --> V_H["HiHat"]:::audio
    V_K --> MIX["MIX"]:::math
    V_S --> MIX
    V_H --> MIX
    MIX --> OUT["OUTPUT"]:::ui

    K1["K1: Kick Decay"]:::ui --> V_K
    K2["K2: Snare Decay"]:::ui --> V_S
    K3["K3: Hat Decay"]:::ui --> V_H
    K8["K8: Swing"]:::ui --> CLK

    A1["A1-A8: Steps 1-8"]:::ui --> PAT_K
    B1["B1-B8: Steps 9-16"]:::ui --> PAT_K

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 4. Delay + Reverb Effect Unit (Intermediate)
**Complexity: ★★★☆☆**

Stereo FX processor for external audio.

```
┌─────────────┐               ┌─────────────┐
│  AUDIO IN L │               │  AUDIO IN R │
└──────┬──────┘               └──────┬──────┘
       │                             │
       ▼                             ▼
┌─────────────────────────────────────────────────────┐
│                   DELAY LINE                        │
│  Time ◄── K1,  Feedback ◄── K2,  Mix ◄── K3         │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│                    REVERB SC                        │
│  Decay ◄── K4,  LP Freq ◄── K5,  Mix ◄── K6         │
└───────────────────────┬─────────────────────────────┘
                   ┌─────┴─────┐
                   ▼           ▼
             ┌─────────┐ ┌─────────┐
             │ OUT L   │ │ OUT R   │
             └─────────┘ └─────────┘
```
```mermaid
flowchart TD
    INL["AUDIO IN L"]:::ui --> DLY["DELAY LINE"]:::audio
    INR["AUDIO IN R"]:::ui --> DLY
    DLY --> RVB["REVERB SC"]:::audio
    RVB --> OUTL["OUT L"]:::ui
    RVB --> OUTR["OUT R"]:::ui

    K1["K1: Time"]:::ui --> DLY
    K2["K2: Feedback"]:::ui --> DLY
    K3["K3: Mix"]:::ui --> DLY
    K4["K4: Decay"]:::ui --> RVB
    K5["K5: LP Freq"]:::ui --> RVB
    K6["K6: Mix"]:::ui --> RVB
    K7["K7: Input Gain"]:::ui --> DLY
    K8["K8: Output Level"]:::ui --> RVB

    SW1["SW1: Delay Bypass"]:::ui --> DLY
    SW2["SW2: Reverb Bypass"]:::ui --> RVB

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 5. FM Synthesizer (Intermediate)
**Complexity: ★★★☆☆**

Two-operator FM with envelope.

```
┌──────────────────────────────────────────────────────┐
│                    KEYBOARD                          │
└───────────────────────┬──────────────────────────────┘
                         │ Note + Velocity
                         ▼
           ┌───────────────────────────────┐
           │            FM2                │
           │  ┌─────────────────────────┐  │
           │  │     MODULATOR OSC       │◄─┼── Ratio (K2)
           │  └───────────┬─────────────┘  │
           │              │ FM             │
           │  ┌───────────▼─────────────┐  │
           │  │      CARRIER OSC        │◄─┼── Index (K1)
           │  └─────────────────────────┘  │
           └───────────────┬───────────────┘
                           ▼
           ┌───────────────────────────────┐
           │            ADSR               │◄── K3-K6
           └───────────────┬───────────────┘
                           ▼
           ┌───────────────────────────────┐
           │         SVF FILTER            │◄── Cutoff (K7)
           └───────────────┬───────────────┘
                           ▼
                     ┌───────────┐
                     │  OUTPUT   │◄── Level (K8)
                     └───────────┘
```
```mermaid
flowchart TD
    KB["KEYBOARD"]:::ui --> NOTE["Note + Velocity"]:::ctrl
    NOTE --> FM["FM2"]:::audio
    FM --> ADSR["ADSR"]:::ctrl
    ADSR --> SVF["SVF FILTER"]:::audio
    SVF --> OUT["OUTPUT"]:::ui

    K1["K1: Index"]:::ui --> FM
    K2["K2: Ratio"]:::ui --> FM
    K3["K3: A"]:::ui --> ADSR
    K4["K4: D"]:::ui --> ADSR
    K5["K5: S"]:::ui --> ADSR
    K6["K6: R"]:::ui --> ADSR
    K7["K7: Cutoff"]:::ui --> SVF
    K8["K8: Level"]:::ui --> OUT

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 6. Physical Modeling StringVoice (Intermediate+)
**Complexity: ★★★☆☆**

Plucked string with overdrive and reverb.

```
┌──────────────────────────────────────────────────────┐
│                    KEYBOARD                          │
└───────────────────────┬──────────────────────────────┘
                         │ Trigger + Pitch
                         ▼
           ┌───────────────────────────────┐
           │         StringVoice           │
           │  Brightness ◄── K1            │
           │  Structure ◄── K2             │
           │  Damping ◄── K3               │
           │  Accent ◄── K4                │
           └───────────────┬───────────────┘
                           ▼
           ┌───────────────────────────────┐
           │          OVERDRIVE            │
           │  Amount ◄── K5,  Mix ◄── K6   │
           └───────────────┬───────────────┘
                           ▼
           ┌───────────────────────────────┐
           │          REVERB SC            │
           │  Feedback ◄── K7, Mix ◄── K8  │
           └───────────────┬───────────────┘
                     ┌─────┴─────┐
                     ▼           ▼
               ┌─────────┐ ┌─────────┐
               │ OUT L   │ │ OUT R   │
               └─────────┘ └─────────┘
```
```mermaid
flowchart TD
    KB["KEYBOARD"]:::ui --> TRIG["Trigger + Pitch"]:::ctrl
    TRIG --> STR["StringVoice"]:::audio
    STR --> OVR["OVERDRIVE"]:::audio
    OVR --> RVB["REVERB SC"]:::audio
    RVB --> OUTL["OUT L"]:::ui
    RVB --> OUTR["OUT R"]:::ui

    K1["K1: Brightness"]:::ui --> STR
    K2["K2: Structure"]:::ui --> STR
    K3["K3: Damping"]:::ui --> STR
    K4["K4: Accent"]:::ui --> STR
    K5["K5: Amount"]:::ui --> OVR
    K6["K6: Mix"]:::ui --> OVR
    K7["K7: Feedback"]:::ui --> RVB
    K8["K8: Mix"]:::ui --> RVB

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 7. Granular Texture Generator (Advanced)
**Complexity: ★★★★☆**

Evolving textures via grain modulation.

```
┌─────────────────────────────────────────────────────┐
│                    LFO BANK                         │
│  ┌─────┐   ┌─────┐   ┌─────┐   ┌─────┐              │
│  │LFO 1│   │LFO 2│   │LFO 3│   │LFO 4│              │
│  └──┬──┘   └──┬──┘   └──┬──┘   └──┬──┘              │
└─────┼────────┼────────┼────────┼────────────────────┘
      │        │        │        │
      ▼        ▼        ▼        ▼
┌─────────────────────────────────────────────────────┐
│              GRAINLET OSCILLATOR                    │
│  Freq ◄── Keyboard                                  │
│  Shape ◄── LFO1,  Formant ◄── LFO2                  │
│  Bleed ◄── K3                                       │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│                   REVERB SC                         │
│  Decay ◄── K7,  Mix ◄── K8                          │
└───────────────────────┬─────────────────────────────┘
                         ▼
                   ┌───────────┐
                   │  OUTPUT   │
                   └───────────┘
```
```mermaid
flowchart TD
    KB["Keyboard"]:::ui --> FREQ["Freq"]:::ctrl
    LFO1["LFO 1"]:::ctrl --> GRN["GRAINLET OSCILLATOR"]:::audio
    LFO2["LFO 2"]:::ctrl --> GRN
    LFO3["LFO 3"]:::ctrl --> GRN
    LFO4["LFO 4"]:::ctrl --> GRN
    GRN --> RVB["REVERB SC"]:::audio
    RVB --> OUT["OUTPUT"]:::ui

    FREQ --> GRN
    K1["K1: Grain Size"]:::ui --> GRN
    K2["K2: Formant"]:::ui --> GRN
    K3["K3: Bleed"]:::ui --> GRN
    K4["K4: Density"]:::ui --> GRN
    K5["K5: LFO Rate"]:::ui --> LFO1
    K6["K6: LFO Rate"]:::ui --> LFO2
    K7["K7: Decay"]:::ui --> RVB
    K8["K8: Mix"]:::ui --> RVB

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 8. Polyphonic Modal Synthesizer (Advanced)
**Complexity: ★★★★☆**

8-voice poly with VoiceManager pattern.

```
┌──────────────────────────────────────────────────────┐
│                  MIDI / KEYBOARD                     │
└───────────────────────┬──────────────────────────────┘
                         │ Note On/Off
                         ▼
┌──────────────────────────────────────────────────────┐
│                  VOICE MANAGER                       │
│  ┌────────┐ ┌────────┐ ┌────────┐      ┌────────┐   │
│  │ Modal  │ │ Modal  │ │ Modal  │      │ Modal  │   │
│  │Voice 1 │ │Voice 2 │ │Voice 3 │ ...  │Voice 8 │   │
│  └───┬────┘ └───┬────┘ └───┬────┘      └───┬────┘   │
│      └──────────┴──────────┴───────────────┘        │
│                        │ SUM                        │
└────────────────────────┼────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │    MASTER SVF     │◄── Cutoff (K5)
               └─────────┬─────────┘
                         ▼
               ┌───────────────────┐
               │      CHORUS       │◄── Depth (K6)
               └─────────┬─────────┘
                         ▼
               ┌───────────────────┐
               │    REVERB SC      │◄── Mix (K7)
               └─────────┬─────────┘
                         ▼
                   ┌───────────┐
                   │  OUTPUT   │◄── Level (K8)
                   └───────────┘
```
```mermaid
flowchart TD
    MIDI["MIDI / KEYBOARD"]:::ui --> NOTE["Note On/Off"]:::ctrl
    NOTE --> VM["VOICE MANAGER"]:::math
    VM --> SUM["SUM"]:::math
    SUM --> MSVF["MASTER SVF"]:::audio
    MSVF --> CH["CHORUS"]:::audio
    CH --> RVB["REVERB SC"]:::audio
    RVB --> OUT["OUTPUT"]:::ui

    V1["Modal Voice 1"]:::audio
    V2["Modal Voice 2"]:::audio
    V3["Modal Voice 3"]:::audio
    V8["Modal Voice 8"]:::audio
    VM --> V1
    VM --> V2
    VM --> V3
    VM --> V8
    V1 --> SUM
    V2 --> SUM
    V3 --> SUM
    V8 --> SUM

    K1["K1: Brightness"]:::ui --> SUM
    K2["K2: Structure"]:::ui --> SUM
    K3["K3: Damping"]:::ui --> SUM
    K4["K4: Accent"]:::ui --> SUM
    K5["K5: Cutoff"]:::ui --> MSVF
    K6["K6: Depth"]:::ui --> CH
    K7["K7: Mix"]:::ui --> RVB
    K8["K8: Level"]:::ui --> OUT

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 9. Formant Vowel Synthesizer (Advanced)
**Complexity: ★★★★☆**

Speech synthesis with vowel morphing.

```
┌─────────────────────────────────────────────────────┐
│              VOWEL TABLE (A, E, I, O, U)            │
│  A: F1=730Hz  F2=1090Hz                             │
│  E: F1=530Hz  F2=1840Hz                             │
│  I: F1=270Hz  F2=2290Hz                             │
│  O: F1=570Hz  F2=840Hz                              │
│  U: F1=300Hz  F2=870Hz                              │
└───────────────────────┬─────────────────────────────┘
                         │ Interpolated F1/F2
                         ▼
┌─────────────────────────────────────────────────────┐
│              FORMANT OSCILLATORS                    │
│  ┌───────────────────┐    ┌───────────────────┐     │
│  │  Formant Osc 1    │    │  Formant Osc 2    │     │
│  │  (Low F1)         │    │  (High F2)        │     │
│  └─────────┬─────────┘    └─────────┬─────────┘     │
│            └────────────┬───────────┘               │
│                         │ MIX                       │
└─────────────────────────┼───────────────────────────┘
                           ▼
                 ┌───────────────────┐
                 │       ADSR        │◄── Keyboard Gate
                 └─────────┬─────────┘
                           ▼
                 ┌───────────────────┐
                 │      PHASER       │◄── Depth (K6)
                 └─────────┬─────────┘
                           ▼
                     ┌───────────┐
                     │  OUTPUT   │
                     └───────────┘
```
```mermaid
flowchart TD
    KB["Keyboard"]:::ui --> VWL["VOWEL TABLE (A, E, I, O, U)"]:::math
    VWL --> F1["Formant Osc 1 (Low F1)"]:::audio
    VWL --> F2["Formant Osc 2 (High F2)"]:::audio
    F1 --> MIX["MIX"]:::math
    F2 --> MIX
    MIX --> ADSR["ADSR"]:::ctrl
    ADSR --> PHS["PHASER"]:::audio
    PHS --> OUT["OUTPUT"]:::ui

    K1["K1: Carrier Freq"]:::ui --> F1
    K2["K2: Vowel Morph"]:::ui --> VWL
    K3["K3: Vibrato"]:::ui --> ADSR
    K4["K4: Attack"]:::ui --> ADSR
    K5["K5: Release"]:::ui --> ADSR
    K6["K6: Phaser Depth"]:::ui --> PHS
    K7["K7: FX"]:::ui --> PHS
    K8["K8: FX"]:::ui --> PHS

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 10. Complete Performance Workstation (Expert)
**Complexity: ★★★★★**

Multi-mode: Synth + Drums + FX.

```
┌─────────────────────────────────────────────────────────────┐
│                    MODE SELECT (SW1 + SW2)                  │
│  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐             │
│  │ SYNTH  │  │ DRUMS  │  │   FX   │  │ LOOPER │             │
│  └────────┘  └────────┘  └────────┘  └────────┘             │
└─────────────────────────────────────────────────────────────┘

┌─ SYNTH MODE ────────────────────────────────────────────────┐
│  KEYBOARD ──► StringVoice ──► Overdrive ──► FX Bus          │
└─────────────────────────────────────────────────────────────┘

┌─ DRUMS MODE ────────────────────────────────────────────────┐
│  METRO ──► 16-Step Sequencer ──► 6 Drums ──► FX Bus         │
│  KEYS: Pattern programming                                  │
└─────────────────────────────────────────────────────────────┘

┌─ FX MODE ───────────────────────────────────────────────────┐
│  AUDIO IN ──► Serial FX Chain ──► FX Bus                    │
└─────────────────────────────────────────────────────────────┘

┌─ LOOPER MODE ───────────────────────────────────────────────┐
│  AUDIO IN ──► 8-slot Looper ──► FX Bus                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       FX BUS                                │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐                      │
│  │ CHORUS  │──► DELAY   │──► REVERB  │                      │
│  └─────────┘  └─────────┘  └─────────┘                      │
└───────────────────────────┬─────────────────────────────────┘
                            ▼
                    ┌───────────────┐
                    │  OUTPUT L/R   │
                    └───────────────┘
```
```mermaid
flowchart TD
    MODE["MODE SELECT (SW1 + SW2)"]:::ui --> MODE_SEL["SYNTH / DRUMS / FX / LOOPER"]:::math

    KB["KEYBOARD"]:::ui --> STR["StringVoice"]:::audio
    STR --> OVR["Overdrive"]:::audio
    OVR --> FXB["FX BUS"]:::audio

    CLK["METRO"]:::ctrl --> SEQ["16-Step Sequencer"]:::math
    SEQ --> DRUMS["6 Drums"]:::audio
    DRUMS --> FXB

    AUDIN["AUDIO IN"]:::ui --> FXCH["Serial FX Chain"]:::audio
    FXCH --> FXB

    LOOPER["AUDIO IN"]:::ui --> LOOP["8-slot Looper"]:::audio
    LOOP --> FXB

    FXB --> CH["CHORUS"]:::audio
    CH --> DLY["DELAY"]:::audio
    DLY --> RVB["REVERB"]:::audio
    RVB --> OUT["OUTPUT L/R"]:::ui

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---


# Part 3: Modular Synthesizer Concepts

*Designed for integration with Eurorack/modular systems. External MIDI keyboard for notes, Field keys as modular control surface, CV I/O for patching.*

---

## 21. Basic Modular Voice (Complexity ★★★☆☆)

A single VCO-VCF-VCA voice module with CV/Gate inputs. Perfect starting point for modular integration.

### Signal Flow
```
┌─────────────────────────────────────────────────────────────────────┐
│                        CV/GATE INPUTS                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐             │
│  │ CV IN 1  │  │ CV IN 2  │  │ CV IN 3  │  │ CV IN 4  │             │
│  │  V/Oct   │  │  Filter  │  │  VCA CV  │  │  Gate    │             │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘             │
│       │             │             │             │                   │
└───────┼─────────────┼─────────────┼─────────────┼───────────────────┘
        │             │             │             │
        ▼             │             │             │
┌───────────────────┐ │             │             │
│        VCO        │ │             │             │
│  ┌─────────────┐  │ │             │             │
│  │ Waveform    │◄─┼─┼─ KEYS A1-A4 (Saw/Sq/Tri/Sin)
│  │ Sub Octave  │◄─┼─┼─ KEYS A5: Sub -1 oct      │
│  │ PWM Depth   │◄─┼─┼─ Knob 1                   │
│  │ Detune      │◄─┼─┼─ Knob 2                   │
│  └─────────────┘  │ │             │             │
└─────────┬─────────┘ │             │             │
          │           │             │             │
          ▼           ▼             │             │
┌─────────────────────────────────┐ │             │
│             VCF                 │ │             │
│  ┌─────────────────────────┐    │ │             │
│  │ Cutoff ◄── Knob 3 + CV2 │    │ │             │
│  │ Resonance ◄── Knob 4    │    │ │             │
│  │ Env Depth ◄── Knob 5    │    │ │             │
│  │ Type ◄── KEYS A6-A8     │    │ │             │
│  │   (LP / BP / HP)        │    │ │             │
│  └─────────────────────────┘    │ │             │
└─────────────────┬───────────────┘ │             │
                  │                 │             │
                  ▼                 ▼             ▼
┌───────────────────────────────────────────────────┐
│                    VCA + ENV                      │
│  ┌─────────────────────────────────────────────┐  │
│  │ Attack ◄── Knob 6                           │  │
│  │ Decay ◄── Knob 7                            │  │
│  │ Sustain/Release ◄── Knob 8                  │  │
│  │ CV3 = VCA modulation input                  │  │
│  │ CV4 = Gate trigger                          │  │
│  └─────────────────────────────────────────────┘  │
└───────────────────────────┬───────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────┐
│                  CV OUTPUTS                       │
│  ┌──────────┐  ┌──────────┐                       │
│  │ CV OUT 1 │  │ CV OUT 2 │                       │
│  │ Envelope │  │ Gate Out │                       │
│  └──────────┘  └──────────┘                       │
└───────────────────────────┬───────────────────────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  AUDIO OUT    │
                    │    L / R      │
                    └───────────────┘
```
```mermaid
flowchart TD
    CV1["CV IN 1 (V/Oct)"]:::ctrl --> VCO["VCO"]:::audio
    CV2["CV IN 2 (Filter)"]:::ctrl --> VCF["VCF"]:::audio
    CV3["CV IN 3 (VCA CV)"]:::ctrl --> VCA["VCA + ENV"]:::ctrl
    CV4["CV IN 4 (Gate)"]:::ctrl --> VCA
    VCO --> VCF
    VCF --> VCA
    VCA --> OUT["AUDIO OUT L/R"]:::ui

    VCA --> CVO1["CV OUT 1 (Envelope)"]:::ctrl
    VCA --> CVO2["CV OUT 2 (Gate Out)"]:::ctrl

    K1["K1: PWM Depth"]:::ui --> VCO
    K2["K2: Detune"]:::ui --> VCO
    K3["K3: Cutoff"]:::ui --> VCF
    K4["K4: Resonance"]:::ui --> VCF
    K5["K5: Env Depth"]:::ui --> VCF
    K6["K6: Attack"]:::ui --> VCA
    K7["K7: Decay"]:::ui --> VCA
    K8["K8: Sustain/Release"]:::ui --> VCA

    A1["A1-A4: Waveform"]:::ui --> VCO
    A5["A5: Sub Octave"]:::ui --> VCO
    A6["A6-A8: Filter Type"]:::ui --> VCF

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 22. Modular FX + Mixer Hub (Complexity ★★★★★☆☆☆☆☆)

A utility module: 2-channel mixer with send/return FX, CV-controllable parameters, clock output for sequencers.

### Signal Flow
```
┌─────────────────────────────────────────────────────────────────────┐
│                     AUDIO INPUTS (from modular)                     │
│  ┌─────────────┐  ┌─────────────┐                                   │
│  │ AUDIO IN L  │  │ AUDIO IN R  │                                   │
│  │  (Ch 1)     │  │  (Ch 2)     │                                   │
│  └──────┬──────┘  └──────┬──────┘                                   │
│         │                │                                          │
└─────────┼────────────────┼──────────────────────────────────────────┘
          │                │
          ▼                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                        CHANNEL STRIP                                │
```
```mermaid
flowchart TD
    INL["AUDIO IN L (Ch 1)"]:::ui --> CH1["CHANNEL STRIP 1"]:::math
    INR["AUDIO IN R (Ch 2)"]:::ui --> CH2["CHANNEL STRIP 2"]:::math
    CH1 --> MIX["MAIN MIX"]:::math
    CH2 --> MIX
    MIX --> SENDFX["SEND FX BUS"]:::audio
    SENDFX --> RVB["REVERB"]:::audio
    RVB --> RET["FX RETURN"]:::audio
    RET --> MIX

    MIX --> OUTL["OUTPUT L"]:::ui
    MIX --> OUTR["OUTPUT R"]:::ui

    K1["K1: Ch 1 Gain"]:::ui --> CH1
    K2["K2: Ch 2 Gain"]:::ui --> CH2
    K3["K3: Send Level"]:::ui --> SENDFX
    K4["K4: Return Level"]:::ui --> RET
    K5["K5: Pan 1"]:::ui --> CH1
    K6["K6: Pan 2"]:::ui --> CH2
    K7["K7: Master"]:::ui --> MIX
    K8["K8: FX Mix"]:::ui --> MIX

    CLK["CLOCK OUT"]:::ctrl --> CLKOUT["Clock Output"]:::ui
    TRIG["TRIG OUT"]:::ctrl --> TRIGOUT["Trig Output"]:::ui

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ CH1: Gain(K1) ─► Pan(K2) ─► FX Send(K3) ─┬─► Main Bus         │  │
│  │ CH2: Gain(K4) ─► Pan(K5) ─► FX Send(K6) ─┤                    │  │
│  │                                          │                    │  │
│  │ KEYS A1-A4: CH1 routing (Pre/Post/Mute/Solo)                  │  │
│  │ KEYS A5-A8: CH2 routing (Pre/Post/Mute/Solo)                  │  │
│  └───────────────────────────────────────────┼───────────────────┘  │
└──────────────────────────────────────────────┼──────────────────────┘
                                               │
          ┌────────────────────────────────────┘
          │
          ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      FX PROCESSOR                                   │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │                                                               │  │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────┐                    │  │
│  │  │ DELAY   │───►│ REVERB  │───►│ RETURN  │                    │  │
│  │  └─────────┘    └─────────┘    └─────────┘                    │  │
│  │       │              │                                        │  │
│  │  Time ◄── CV1   Decay ◄── CV2                                 │  │
│  │  Feedback◄──K7  Mix ◄── CV3                                   │  │
│  │                                                               │  │
│  │  KEYS B1-B4: FX1 select (Delay/Chorus/Flanger/Phaser)         │  │
│  │  KEYS B5-B8: FX2 select (Reverb/Plate/Hall/Shimmer)           │  │
│  └───────────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────┬─────────────────────┘
                                                │
┌───────────────────────────────────────────────┼─────────────────────┐
│                   MASTER OUTPUT + CLOCK                             │
│  ┌───────────────────────────────────────────────────────────────┐  │
│  │ Main Mix ◄── Knob 8                                           │  │
│  │                                                               │  │
│  │ CV OUT 1: LFO (synced to tempo)                               │  │
│  │ CV OUT 2: Clock pulses (ppqn selectable)                      │  │
│  │                                                               │  │
│  │ SW1: Tap tempo                                                │  │
│  │ SW2: Clock start/stop                                         │  │
│  └───────────────────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────┬─────────────────────┘
                                                │
                                                ▼
                                    ┌───────────────────┐
                                    │    AUDIO OUT      │
                                    │      L / R        │
                                    └───────────────────┘
```

### Key Assignments
| Key Row | Function |
|---------|----------|
| A1-A2 | CH1: Mute, Solo |
| A3-A4 | CH1: Pre-fader FX, Post-fader FX |
| A5-A6 | CH2: Mute, Solo |
| A7-A8 | CH2: Pre-fader FX, Post-fader FX |
| B1-B4 | FX1 type: Delay, Chorus, Flanger, Phaser |
| B5-B8 | FX2 type: Reverb, HallVerb, Plate, Shimmer |

### OLED Display Layout
```
┌────────────────────────────────┐
│ MIXER HUB        BPM: 120.0    │
│ CH1:▓▓▓▓▓▓░░  CH2:▓▓▓▓░░░░     │
│ FX: Delay→Reverb  Send: 45%    │
│ CLK: ● ○ ○ ○  LFO: ∿∿∿∿        │
└────────────────────────────────┘

Line 1: Mode, current tempo
Line 2: Channel meters (real-time VU)
Line 3: Active FX chain, send level
Line 4: Clock visualization, LFO wave
```

### DaisySP Modules
- `DelayLine<>` (tempo-synced)
- `Chorus` / `Flanger` / `Phaser`
- `ReverbSc`
- `Metro` (clock)
- `Oscillator` (LFO for CV out)

---

## 23. Complete Modular Workstation (Complexity ★★★★★★★★☆☆)

Full-featured modular companion: dual VCO, multimode filter, dual envelope, LFO bank, sequencer, FX, and comprehensive modulation matrix.

### Signal Flow
```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              CV INPUTS (4)                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐                         │
│  │ CV1     │  │ CV2     │  │ CV3     │  │ CV4     │                         │
│  │ V/Oct   │  │ Mod     │  │ Mod     │  │ Gate    │                         │
│  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘                         │
└───────┼───────────┼───────────┼───────────┼─────────────────────────────────┘
        │           │           │           │
        │           └───────────┴───────────┘
        │                       │
        ▼                       ▼
┌─────────────────────────────────────────────────────────────────────────────┐
│                          SOUND ENGINE                                       │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                        DUAL VCO                                       │  │
│  │  ┌─────────────────┐        ┌─────────────────┐                       │  │
│  │  │      VCO 1      │        │      VCO 2      │                       │  │
│  │  │ Wave: SAW/SQ/TRI│        │ Wave: SAW/SQ/TRI│                       │  │
│  │  │ Tune: ±24 semi  │        │ Tune: ±24 semi  │                       │  │
│  │  │ PWM: 0-100%     │        │ FM Input: VCO1  │                       │  │
│  │  └────────┬────────┘        └────────┬────────┘                       │  │
│  │           │                          │                                │  │
│  │           └──────────┬───────────────┘                                │  │
│  │                      │ MIX (Knob 1)                                   │  │
│  └──────────────────────┼────────────────────────────────────────────────┘  │
│                         │                                                   │
│                         ▼                                                   │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                    MULTIMODE FILTER                                   │  │
│  │  ┌─────────────────────────────────────────────────────────────────┐  │  │
│  │  │ Mode: LP12 / LP24 / BP / HP / Notch / Formant                   │  │  │
│  │  │ Cutoff ◄── Knob 2 + CV2 + Env1                                  │  │  │
│  │  │ Resonance ◄── Knob 3                                            │  │  │
│  │  │ Drive ◄── Knob 4                                                │  │  │
│  │  └─────────────────────────────────────────────────────────────────┘  │  │
│  └──────────────────────┬────────────────────────────────────────────────┘  │
│                         │                                                   │
│                         ▼                                                   │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │                     VCA + ENVELOPES                                   │  │
│  │  ┌────────────────────────┐  ┌────────────────────────┐               │  │
│  │  │        ENV 1           │  │        ENV 2           │               │  │
│  │  │ (VCF Modulation)       │  │ (VCA Amplitude)        │               │  │
│  │  │ A: Knob 5  D: Knob 6   │  │ A: Knob 5  D: Knob 6   │               │  │
│  │  │ S: Knob 7  R: Knob 8   │  │ S: Knob 7  R: Knob 8   │               │  │
│  │  └────────────────────────┘  └────────────────────────┘               │  │
│  │                                                                       │  │
│  │  SW1 (hold): Edit ENV1        SW2 (hold): Edit ENV2                   │  │
│  └──────────────────────┬────────────────────────────────────────────────┘  │
│                         │                                                   │
└─────────────────────────┼───────────────────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────────────────┐
│                    LFO BANK + MODULATION MATRIX                             │
│                         │                                                   │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  LFO 1: Rate(K1), Shape(A1-A4), Dest(B1-B4)                           │  │
│  │  LFO 2: Rate(K2), Shape(A5-A8), Dest(B5-B8)                           │  │
│  │                                                                       │  │
│  │  Shapes: Sine, Tri, Saw, Square, S&H, Random                          │  │
│  │                                                                       │  │
│  │  Destinations:                                                        │  │
│  │    VCO1 Pitch, VCO2 Pitch, VCO1 PWM, VCO2 PWM                         │  │
│  │    Filter Cutoff, Filter Res, VCA, Pan                                │  │
│  │                                                                       │  │
│  │  Depth controlled by Knobs when destination selected                  │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                         │                                                   │
└─────────────────────────┼───────────────────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────────────────┐
│                      8-STEP SEQUENCER                                       │
│                         │                                                   │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ┌────┬────┬────┬────┬────┬────┬────┬────┐                            │  │
│  │  │ S1 │ S2 │ S3 │ S4 │ S5 │ S6 │ S7 │ S8 │  ◄── KEYS A1-A8 select    │  │
│  │  └────┴────┴────┴────┴────┴────┴────┴────┘                            │  │
│  │                                                                       │  │
│  │  KEYS B1-B8: Step gate on/off for current step                        │  │
│  │  Knob in Seq mode: Pitch for selected step                            │  │
│  │                                                                       │  │
│  │  Output: V/Oct CV, Gate CV, Accent CV                                 │  │
│  │  Clock: Internal (tap tempo) or External CV4                          │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                         │                                                   │
└─────────────────────────┼───────────────────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────────────────┐
│                      FX SECTION                                             │
│                         │                                                   │
│  ┌───────────────────────────────────────────────────────────────────────┐  │
│  │  ┌─────────┐    ┌─────────┐    ┌─────────┐                            │  │
│  │  │OVERDRIVE│───►│  DELAY  │───►│ REVERB  │                            │  │
│  │  │  Mix:K  │    │ Time:K  │    │ Decay:K │                            │  │
│  │  └─────────┘    └─────────┘    └─────────┘                            │  │
│  │                                                                       │  │
│  │  FX Bypass: SW1 tap                                                   │  │
│  │  FX type cycle: SW2 tap                                               │  │
│  └───────────────────────────────────────────────────────────────────────┘  │
│                         │                                                   │
└─────────────────────────┼───────────────────────────────────────────────────┘
                          │
┌─────────────────────────┼───────────────────────────────────────────────────┐
│                    OUTPUTS                                                  │
│                         │                                                   │
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐ │
│  │  AUDIO OUT L  │  │  AUDIO OUT R  │  │  CV OUT 1     │  │  CV OUT 2     │ │
│  │    (Main)     │  │    (Main)     │  │  (Env/LFO)    │  │  (Seq V/Oct)  │ │
│  └───────────────┘  └───────────────┘  └───────────────┘  └───────────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Mode System (SW1 + SW2 combinations)
| SW1 | SW2 | Mode | Keys Function |
|-----|-----|------|---------------|
| Off | Off | PLAY | A=Waveform select, B=Filter type |
| On  | Off | LFO  | A=LFO1 shape, B=LFO1 routing |
| Off | On  | SEQ  | A=Step select, B=Gate toggle |
| On  | On  | FX   | A=FX type, B=FX routing |

### OLED Display - Multi-Page System
```
PAGE 1 - OSCILLATORS:
┌────────────────────────────────┐
│ VCO1: SAW ▸▸▸▸  VCO2: SQR ████ │
│ Mix: ████▒▒░░  Tune: +3 semi   │
│ PWM1: 45%  FM: VCO1→VCO2       │
│ ─────────────────────────────  │
└────────────────────────────────┘

PAGE 2 - FILTER + ENV:
┌────────────────────────────────┐
│ FILTER: LP24  Cutoff: 2.4kHz   │
│ Res: ▓▓▓▓░░░  Drive: ▓▓░░░░░   │
│ ENV1: ╱‾‾╲__  ENV2: ╱‾\_       │
│ A:30 D:150 S:60% R:200         │
└────────────────────────────────┘

PAGE 3 - LFO + MOD:
┌────────────────────────────────┐
│ LFO1: ∿∿∿ 0.5Hz → Cutoff  25%  │
│ LFO2: ▁▂▃▄ 2Hz → VCO1 PWM 40%  │
│ Mod Matrix: 4 active routes    │
│ [View Matrix]                  │
└────────────────────────────────┘

PAGE 4 - SEQUENCER:
┌────────────────────────────────┐
│ SEQ: ●○●●○●○●  BPM: 120        │
│ Step 3: C#4  Gate: 75%         │
│ ▓░░░░░░░ Playing               │
│ [PLAY] [STOP] [REC]       