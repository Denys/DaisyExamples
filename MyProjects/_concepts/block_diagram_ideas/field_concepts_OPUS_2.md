# Daisy Field Architecture Ideas

## Table of Contents


### Part 2: MIDI + Keys as Control Surface (11-20)
11. [MIDI Poly Synth + XY Pad](#11-midi-poly-synth-with-xy-pad) ★★★☆☆
12. [Step Sequencer](#12-step-sequencer-with-midi-sync) ★★★☆☆
13. [Drum Machine Pro](#13-drum-machine-pro) ★★★☆☆
14. [Multi-FX Box A: Parallel](#14-multi-fx-box-a-parallel-mixer) ★★★☆☆
15. [Multi-FX Box B: Serial](#15-multi-fx-box-b-serial-chain) ★★★★☆
16. [Wavetable Synth](#16-wavetable-synth-with-morph) ★★★★☆
17. [Arpeggiator Synth](#17-arpeggiator-synth) ★★★★☆
18. [Looper + Sampler](#18-looper-sampler) ★★★★☆
19. [Modular-Style Patcher](#19-modular-style-semi-patcher) ★★★★★
20. [Live Performance Hub](#20-live-performance-hub) ★★★★★



---



# Part 2: MIDI Keyboard + Keys as Control Surface

*All concepts assume external MIDI keyboard for notes. Field keys A1-A8 and B1-B8 repurposed for controls.*

---

## 11. MIDI Poly Synth with XY Pad
**Complexity: ★★★☆☆**

Keys A1-A8 = X-axis zones, B1-B8 = Y-axis zones for real-time parameter morphing.

```
┌─────────────────────────────────────────────────────┐
│  MIDI IN (Notes)                                    │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   VoiceManager    │
               │   8× Oscillator   │
               │   + SVF + ADSR    │
               └─────────┬─────────┘
                         │
┌───────────────────────┼─────────────────────────────┐
│  FIELD KEYS AS XY PAD                               │
│  A1-A8: X-axis (Filter Cutoff interpolation)        │
│  B1-B8: Y-axis (Resonance / FX Mix)                 │
└───────────────────────┼─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   Chorus/Reverb   │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    MIDI["MIDI IN (Notes)"]:::ui --> VM["VoiceManager (8x Osc + SVF + ADSR)"]:::math
    VM --> FX["Chorus/Reverb"]:::audio
    FX --> OUT["OUTPUT"]:::ui

    A1["A1-A8: X-axis (Cutoff)"]:::ui --> VM
    B1["B1-B8: Y-axis (Resonance/FX)"]:::ui --> VM

    K1["K1: ADSR"]:::ui --> VM
    K2["K2: ADSR"]:::ui --> VM
    K3["K3: ADSR"]:::ui --> VM
    K4["K4: ADSR"]:::ui --> VM
    K5["K5: XY Smoothing"]:::ui --> VM
    K6["K6: FX"]:::ui --> FX
    K7["K7: FX"]:::ui --> FX
    K8["K8: FX"]:::ui --> FX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 12. Step Sequencer with MIDI Sync
**Complexity: ★★★☆☆**

16-step CV/Gate sequencer. Keys program steps, MIDI clock sync.

```
┌─────────────────────────────────────────────────────┐
│              MIDI CLOCK IN (Sync)                   │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│              16-STEP SEQUENCER                      │
│  ┌─────────────────────────────────────────────┐    │
│  │ Step: 1  2  3  4  5  6  7  8  ...  16       │    │
│  │ Note: C  -  E  -  G  -  C  -  ...  -        │    │
│  │ Gate: ●  ○  ●  ○  ●  ●  ●  ○  ...  ○        │    │
│  └─────────────────────────────────────────────┘    │
│                                                     │
│  KEYS A1-A8: Select step 1-8                        │
│  KEYS B1-B8: Select step 9-16                       │
│  KNOB 1: Note pitch for selected step               │
│  KNOB 2: Gate length                                │
│  KNOB 3: Velocity                                   │
└───────────────────────┬─────────────────────────────┘
                         │ Note/Gate
                         ▼
               ┌───────────────────┐
               │   INTERNAL SYNTH  │
               │   or MIDI OUT     │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    CLK["MIDI CLOCK IN (Sync)"]:::ctrl --> SEQ["16-STEP SEQUENCER"]:::math
    SEQ --> NOTE["Note/Gate"]:::ctrl
    NOTE --> SYNTH["INTERNAL SYNTH or MIDI OUT"]:::math
    SYNTH --> OUT["OUTPUT"]:::ui

    A1["A1-A8: Select step 1-8"]:::ui --> SEQ
    B1["B1-B8: Select step 9-16"]:::ui --> SEQ
    K1["K1: Note pitch"]:::ui --> SEQ
    K2["K2: Gate length"]:::ui --> SEQ
    K3["K3: Velocity"]:::ui --> SEQ

    SW1["SW1: Play/Stop"]:::ui --> SEQ
    SW2["SW2: Record mode"]:::ui --> SEQ

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 13. Drum Machine Pro
**Complexity: ★★★☆☆**

6-voice drums, 16-step pattern, 8 patterns stored.

```
┌─────────────────────────────────────────────────────┐
│              PATTERN MEMORY (8 patterns)            │
└───────────────────────┬─────────────────────────────┘
                         │
┌───────────────────────┼─────────────────────────────┐
│            16-STEP DRUM SEQUENCER                   │
│                                                     │
│  KEYS A1-A8: Steps 1-8 toggle (current drum)        │
│  KEYS B1-B8: Steps 9-16 toggle                      │
│  SW1: Drum select (cycle Kick/Snare/Hat/Tom/Clap/Rim)│
│  SW2: Pattern select (cycle 1-8)                    │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│              DRUM VOICES                            │
│  ┌────────┐ ┌────────┐ ┌────────┐                   │
│  │ Analog │ │ Synth  │ │ HiHat  │ ...               │
│  │BassDrum│ │SnareDrm│ │        │                   │
│  └───┬────┘ └───┬────┘ └───┬────┘                   │
│      └──────────┴──────────┘                        │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   MIXER + COMP    │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    PAT["PATTERN MEMORY (8 patterns)"]:::math --> SEQ["16-STEP DRUM SEQUENCER"]:::math
    SEQ --> DRUMS["DRUM VOICES"]:::audio
    DRUMS --> MIX["MIXER + COMP"]:::math
    MIX --> OUT["OUTPUT"]:::ui

    KICK["Kick"]:::audio
    SNARE["Snare"]:::audio
    HH["HiHat"]:::audio
    TOM["Tom"]:::audio
    CLAP["Clap"]:::audio
    RIM["Rim"]:::audio

    DRUMS --> KICK
    DRUMS --> SNARE
    DRUMS --> HH
    DRUMS --> TOM
    DRUMS --> CLAP
    DRUMS --> RIM
    KICK --> MIX
    SNARE --> MIX
    HH --> MIX
    TOM --> MIX
    CLAP --> MIX
    RIM --> MIX

    A1["A1-A8: Steps 1-8"]:::ui --> SEQ
    B1["B1-B8: Steps 9-16"]:::ui --> SEQ
    K1["K1: Kick tuning"]:::ui --> KICK
    K2["K2: Snare tuning"]:::ui --> SNARE
    K3["K3: Hat decay"]:::ui --> HH
    K4["K4: Tom tuning"]:::ui --> TOM
    K5["K5: Clap tuning"]:::ui --> CLAP
    K6["K6: Rim tuning"]:::ui --> RIM
    K7["K7: Tempo"]:::ui --> SEQ
    K8["K8: Swing"]:::ui --> SEQ

    SW1["SW1: Drum select"]:::ui --> SEQ
    SW2["SW2: Pattern select"]:::ui --> SEQ

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 14. Multi-FX Box A: Parallel Mixer
**Complexity: ★★★☆☆**

Two mono inputs mixed with parallel FX sends.

```
┌─────────────┐                     ┌─────────────┐
│  AUDIO IN L │                     │  AUDIO IN R │
└──────┬──────┘                     └──────┬──────┘
       │                                   │
       ▼                                   ▼
┌─────────────────────────────────────────────────────┐
│                    INPUT MIXER                      │
│  ┌─────────────────┐     ┌─────────────────┐        │
│  │ Input L Gain    │     │ Input R Gain    │        │
│  │ (Knob 1)        │     │ (Knob 2)        │        │
│  └────────┬────────┘     └────────┬────────┘        │
│           └──────────┬───────────┘                  │
│                      │ Stereo Bus                   │
└──────────────────────┼──────────────────────────────┘
                       │
       ┌───────────────┼───────────────┐
       ▼               ▼               ▼
┌────────────┐  ┌────────────┐  ┌────────────┐
│   CHORUS   │  │   DELAY    │  │   REVERB   │
│  (Send 1)  │  │  (Send 2)  │  │  (Send 3)  │
└─────┬──────┘  └─────┬──────┘  └─────┬──────┘
      │               │               │
      └───────────────┼───────────────┘
                      │ Return Mix
                      ▼
              ┌───────────────┐
              │  OUTPUT MIX   │
              │  Dry + Wet    │
              └───────┬───────┘
                      ▼
              ┌───────────────┐
              │   OUTPUT L/R  │
              └───────────────┘
```
```mermaid
flowchart TD
    INL["AUDIO IN L"]:::ui --> IMIX["INPUT MIXER"]:::math
    INR["AUDIO IN R"]:::ui --> IMIX
    IMIX --> BUS["Stereo Bus"]:::math
    BUS --> CH["CHORUS"]:::audio
    BUS --> DLY["DELAY"]:::audio
    BUS --> RVB["REVERB"]:::audio
    CH --> RET["Return Mix"]:::math
    DLY --> RET
    RVB --> RET
    RET --> OMIX["OUTPUT MIX (Dry + Wet)"]:::math
    OMIX --> OUTL["OUTPUT L"]:::ui
    OMIX --> OUTR["OUTPUT R"]:::ui

    K1["K1: Input L Gain"]:::ui --> IMIX
    K2["K2: Input R Gain"]:::ui --> IMIX
    K3["K3: Send 1 (Chorus)"]:::ui --> CH
    K4["K4: Send 2 (Delay)"]:::ui --> DLY
    K5["K5: Send 3 (Reverb)"]:::ui --> RVB
    K6["K6: FX params"]:::ui --> CH
    K7["K7: FX params"]:::ui --> DLY
    K8["K8: FX params"]:::ui --> RVB

    A1["A1-A4: FX1 params"]:::ui --> CH
    A5["A5-A8: FX2 params"]:::ui --> DLY
    B1["B1-B4: FX3 params"]:::ui --> RVB
    B5["B5-B8: Preset recall"]:::ui --> OMIX

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 15. Multi-FX Box B: Serial Chain
**Complexity: ★★★★☆**

Two mono inputs, 6 FX in configurable serial order.

```
┌─────────────┐                     ┌─────────────┐
│  AUDIO IN L │                     │  AUDIO IN R │
└──────┬──────┘                     └──────┬──────┘
       │                                   │
       ▼                                   ▼
┌─────────────────────────────────────────────────────┐
│              INPUT SECTION                          │
│  Gain L (K1) ──┬── Pan ──┬── Gain R (K2)            │
│                │  MIX    │                          │
└────────────────┼─────────┼──────────────────────────┘
                 └────┬────┘
                      ▼
┌─────────────────────────────────────────────────────┐
│            SERIAL FX CHAIN (Configurable)           │
│                                                     │
│  ┌──────┐   ┌──────┐   ┌──────┐   ┌──────┐          │
│  │SLOT 1│──►│SLOT 2│──►│SLOT 3│──►│SLOT 4│──► ...   │
│  └──────┘   └──────┘   └──────┘   └──────┘          │
│                                                     │
│  Available FX per slot:                             │
│  - Overdrive    - Chorus    - Flanger              │
│  - Phaser       - Delay     - Reverb               │
│  - Bitcrush     - Tremolo   - Autowah              │
│                                                     │
│  KEYS A1-A8: Select FX for current slot             │
│  KEYS B1-B4: Select slot 1-4 to edit                │
│  KEYS B5-B8: Bypass slot 1-4                        │
└───────────────────────┬─────────────────────────────┘
                        ▼
              ┌───────────────────┐
              │   OUTPUT L/R     │
              └───────────────────┘
```
```mermaid
flowchart TD
    INL["AUDIO IN L"]:::ui --> INSEC["INPUT SECTION"]:::math
    INR["AUDIO IN R"]:::ui --> INSEC
    INSEC --> FXCH["SERIAL FX CHAIN (Configurable)"]:::audio
    FXCH --> OUTL["OUTPUT L/R"]:::ui

    K1["K1: Gain L"]:::ui --> INSEC
    K2["K2: Gain R"]:::ui --> INSEC
    K3["K3: FX params"]:::ui --> FXCH
    K4["K4: FX params"]:::ui --> FXCH
    K5["K5: FX params"]:::ui --> FXCH
    K6["K6: FX params"]:::ui --> FXCH
    K7["K7: Output"]:::ui --> FXCH
    K8["K8: Output"]:::ui --> FXCH

    A1["A1-A8: Select FX for slot"]:::ui --> FXCH
    B1["B1-B4: Select slot 1-4"]:::ui --> FXCH
    B5["B5-B8: Bypass slot 1-4"]:::ui --> FXCH

    SW1["SW1: Slot select"]:::ui --> FXCH
    SW2["SW2: FX select within slot"]:::ui --> FXCH

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 16. Wavetable Synth with Morph
**Complexity: ★★★★☆**

Custom wavetables with position morphing.

```
┌─────────────────────────────────────────────────────┐
│                  MIDI IN                            │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│              WAVETABLE ENGINE                       │
│  ┌─────────────────────────────────────────────┐    │
│  │  Table: [Sine|Saw|Square|Custom1|Custom2]   │    │
│  │  Position ◄── Knob 1 + LFO                  │    │
│  │  Morph Speed ◄── Knob 2                     │    │
│  └─────────────────────────────────────────────┘    │
│                                                     │
│  KEYS A1-A8: Wavetable bank select                  │
│  KEYS B1-B8: Morph curve select                     │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   SVF + ADSR      │
               └─────────┬─────────┘
                         ▼
               ┌───────────────────┐
               │   Stereo FX       │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    MIDI["MIDI IN"]:::ui --> WAV["WAVETABLE ENGINE"]:::audio
    WAV --> SVF["SVF + ADSR"]:::audio
    SVF --> FX["Stereo FX"]:::audio
    FX --> OUT["OUTPUT"]:::ui

    K1["K1: Position"]:::ui --> WAV
    K2["K2: Morph Speed"]:::ui --> WAV
    K3["K3: Filter"]:::ui --> SVF
    K4["K4: ADSR"]:::ui --> SVF
    K5["K5: ADSR"]:::ui --> SVF
    K6["K6: ADSR"]:::ui --> SVF
    K7["K7: ADSR"]:::ui --> SVF
    K8["K8: FX"]:::ui --> FX

    A1["A1-A8: Wavetable bank select"]:::ui --> WAV
    B1["B1-B8: Morph curve select"]:::ui --> WAV

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 17. Arpeggiator Synth
**Complexity: ★★★★☆**

Hold chords on MIDI, keys control arp pattern/direction.

```
┌─────────────────────────────────────────────────────┐
│              MIDI IN (Held Notes)                   │
└───────────────────────┬─────────────────────────────┘
                         ▼
┌─────────────────────────────────────────────────────┐
│              ARPEGGIATOR ENGINE                     │
│  ┌─────────────────────────────────────────────┐    │
│  │ Direction: Up | Down | UpDown | Random      │    │
│  │ Octaves: 1-4                                 │    │
│  │ Pattern: Straight | Dotted | Swing          │    │
│  │ Gate: 25% | 50% | 75% | 100%                │    │
│  └─────────────────────────────────────────────┘    │
│                                                     │
│  KEYS A1-A8: Pattern/Octave (1-4, up, down, random) │
│  KEYS B1-B8: Rate (Free, 1/4, 1/8, 1/16, etc.)     │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   POLY SYNTH      │
               │   (Osc + VCF +    │
               │    VCA + ADSR)    │
               └─────────┬─────────┘
                         ▼
               ┌───────────────────┐
               │   FX BUS          │
               │ (Reverb / Delay)  │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    MIDI["MIDI IN (Held Notes)"]:::ui --> ARP["ARPEGGIATOR ENGINE"]:::math
    ARP --> SYN["POLY SYNTH (Osc + VCF + VCA + ADSR)"]:::audio
    SYN --> FX["FX BUS (Reverb / Delay)"]:::audio
    FX --> OUT["OUTPUT"]:::ui

    K1["K1: Rate"]:::ui --> ARP
    K2["K2: Gate"]:::ui --> ARP
    K3["K3: Filter"]:::ui --> SYN
    K4["K4: ADSR"]:::ui --> SYN
    K5["K5: ADSR"]:::ui --> SYN
    K6["K6: ADSR"]:::ui --> SYN
    K7["K7: ADSR"]:::ui --> SYN
    K8["K8: FX"]:::ui --> FX

    A1["A1-A8: Pattern/Octave"]:::ui --> ARP
    B1["B1-B8: Rate"]:::ui --> ARP

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 18. Looper + Sampler
**Complexity: ★★★★☆**

Record loops from audio input, trigger via keys.

```
┌─────────────┐                     ┌─────────────┐
│  AUDIO IN L │                     │  AUDIO IN R │
└──────┬──────┘                     └──────┬──────┘
       │                                   │
       └───────────────┬───────────────────┘
                       ▼
┌─────────────────────────────────────────────────────┐
│              LOOP RECORDER                          │
│  ┌─────────────────────────────────────────────┐    │
│  │ Buffer: 8 slots × 4 seconds each            │    │
│  │ Overdub: Layer on existing loops            │    │
│  └─────────────────────────────────────────────┘    │
│                                                     │
│  KEYS A1-A8: Trigger/Record slot 1-8               │
│  KEYS B1-B4: Playback mode (OneShot/Loop/Rev/Half) │
│  KEYS B5-B8: Mute slots 1-4                        │
│                                                     │
│  SW1 (hold): Record arm                             │
│  SW1 (tap): Start/Stop recording                    │
│  SW2: Clear selected slot                           │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   MIXER + FX      │
               └─────────┬─────────┘
                         ▼
                     OUTPUT
```
```mermaid
flowchart TD
    INL["AUDIO IN L"]:::ui --> LOOP["LOOP RECORDER"]:::math
    INR["AUDIO IN R"]:::ui --> LOOP
    LOOP --> MIX["MIXER + FX"]:::math
    MIX --> OUT["OUTPUT"]:::ui

    SLOT1["Slot 1"]:::audio
    SLOT2["Slot 2"]:::audio
    SLOT3["Slot 3"]:::audio
    SLOT4["Slot 4"]:::audio
    SLOT5["Slot 5"]:::audio
    SLOT6["Slot 6"]:::audio
    SLOT7["Slot 7"]:::audio
    SLOT8["Slot 8"]:::audio

    LOOP --> SLOT1
    LOOP --> SLOT2
    LOOP --> SLOT3
    LOOP --> SLOT4
    LOOP --> SLOT5
    LOOP --> SLOT6
    LOOP --> SLOT7
    LOOP --> SLOT8
    SLOT1 --> MIX
    SLOT2 --> MIX
    SLOT3 --> MIX
    SLOT4 --> MIX
    SLOT5 --> MIX
    SLOT6 --> MIX
    SLOT7 --> MIX
    SLOT8 --> MIX

    A1["A1-A8: Trigger/Record slot 1-8"]:::ui --> LOOP
    B1["B1-B4: Playback mode"]:::ui --> LOOP
    B5["B5-B8: Mute slots 1-4"]:::ui --> MIX

    K1["K1: Slot 1 vol"]:::ui --> SLOT1
    K2["K2: Slot 2 vol"]:::ui --> SLOT2
    K3["K3: Slot 3 vol"]:::ui --> SLOT3
    K4["K4: Slot 4 vol"]:::ui --> SLOT4
    K5["K5: Input"]:::ui --> LOOP
    K6["K6: FX Send"]:::ui --> MIX
    K7["K7: FX"]:::ui --> MIX
    K8["K8: FX"]:::ui --> MIX

    SW1["SW1: Record arm/Start/Stop"]:::ui --> LOOP
    SW2["SW2: Clear slot"]:::ui --> LOOP

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 19. Modular-Style Semi-Patcher
**Complexity: ★★★★★**

Keys select source→destination routing. Virtual patch cables.

```
┌─────────────────────────────────────────────────────┐
│                 MODULES                             │
│  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐  ┌─────┐       │
│  │ OSC │  │ LFO │  │ ENV │  │ FLT │  │ VCA │       │
│  │  1  │  │  1  │  │  1  │  │  1  │  │  1  │       │
│  └──┬──┘  └──┬──┘  └──┬──┘  └──┬──┘  └──┬──┘       │
│     │        │        │        │        │          │
└─────┼────────┼────────┼────────┼────────┼──────────┘
      │        │        │        │        │
      └────────┴────────┴────────┴────────┘
                       │
┌──────────────────────┼──────────────────────────────┐
│          PATCH MATRIX (Keys)                        │
│                                                     │
│  KEYS A1-A8: Select SOURCE module                   │
│    A1=OSC1 Out, A2=OSC2 Out, A3=LFO1, A4=LFO2      │
│    A5=ENV1, A6=ENV2, A7=Noise, A8=AudioIn          │
│                                                     │
│  KEYS B1-B8: Select DESTINATION                     │
│    B1=OSC1 Freq, B2=OSC2 Freq, B3=Filter Cutoff    │
│    B4=Filter Res, B5=VCA Amp, B6=LFO Rate          │
│    B7=Pan, B8=FX Send                               │
│                                                     │
│  Press A+B simultaneously = Create patch            │
│  Hold SW1 + Press A+B = Remove patch                │
└───────────────────────┬─────────────────────────────┘
                         ▼
               ┌───────────────────┐
               │   Audio Output    │
               └───────────────────┘
```
```mermaid
flowchart TD
    OSC1["OSC 1"]:::audio --> PATCH["PATCH MATRIX"]:::math
    OSC2["OSC 2"]:::audio --> PATCH
    LFO1["LFO 1"]:::ctrl --> PATCH
    LFO2["LFO 2"]:::ctrl --> PATCH
    ENV1["ENV 1"]:::ctrl --> PATCH
    ENV2["ENV 2"]:::ctrl --> PATCH
    NOISE["Noise"]:::audio --> PATCH
    AUDIN["AudioIn"]:::ui --> PATCH

    PATCH --> OUT["Audio Output"]:::ui

    K1["K1: Adjust param"]:::ui --> OSC1
    K2["K2: Adjust param"]:::ui --> LFO1
    K3["K3: Adjust param"]:::ui --> ENV1
    K4["K4: Adjust param"]:::ui --> FLT
    K5["K5: Adjust param"]:::ui --> VCA
    K6["K6: Adjust param"]:::ui --> LFO2
    K7["K7: Adjust param"]:::ui --> PATCH
    K8["K8: Adjust param"]:::ui --> PATCH

    A1["A1: OSC1 Out"]:::ui --> PATCH
    A2["A2: OSC2 Out"]:::ui --> PATCH
    A3["A3: LFO1"]:::ui --> PATCH
    A4["A4: LFO2"]:::ui --> PATCH
    A5["A5: ENV1"]:::ui --> PATCH
    A6["A6: ENV2"]:::ui --> PATCH
    A7["A7: Noise"]:::ui --> PATCH
    A8["A8: AudioIn"]:::ui --> PATCH

    B1["B1: OSC1 Freq"]:::ui --> PATCH
    B2["B2: OSC2 Freq"]:::ui --> PATCH
    B3["B3: Filter Cutoff"]:::ui --> PATCH
    B4["B4: Filter Res"]:::ui --> PATCH
    B5["B5: VCA Amp"]:::ui --> PATCH
    B6["B6: LFO Rate"]:::ui --> PATCH
    B7["B7: Pan"]:::ui --> PATCH
    B8["B8: FX Send"]:::ui --> PATCH

    SW1["SW1: Hold to remove patch"]:::ui --> PATCH

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 20. Live Performance Hub
**Complexity: ★★★★★**

Unified synth + drums + FX + sequencer + looper.

```
┌─────────────────────────────────────────────────────────────┐
│                    MODE SYSTEM                              │
│  SW1+SW2 combo selects mode:                                │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐                │
│  │ SYNTH  │ │ DRUMS  │ │  FX    │ │ LOOPER │                │
│  └────────┘ └────────┘ └────────┘ └────────┘                │
└─────────────────────────────────────────────────────────────┘

┌─ SYNTH MODE ────────────────────────────────────────────────┐
│  MIDI ──► 8-voice Poly ──► FX Bus                           │
│  KEYS A: Waveform select, KEYS B: Preset recall             │
└─────────────────────────────────────────────────────────────┘

┌─ DRUMS MODE ────────────────────────────────────────────────┐
│  Metro ──► 16-Step Seq ──► 6 Drums ──► FX Bus               │
│  KEYS A: Step edit, KEYS B: Pattern select                  │
└─────────────────────────────────────────────────────────────┘

┌─ FX MODE ───────────────────────────────────────────────────┐
│  Audio In L+R ──► Serial FX Chain ──► Output                │
│  KEYS A: FX slot select, KEYS B: FX type select             │
└─────────────────────────────────────────────────────────────┘

┌─ LOOPER MODE ───────────────────────────────────────────────┐
│  Audio In ──► 8-slot Looper ──► Mixer ──► Output            │
│  KEYS A: Record/Play slots, KEYS B: Playback modes          │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌───────────────────┐
                    │   MASTER OUTPUT   │
                    │   with Limiter    │
                    └───────────────────┘
```
```mermaid
flowchart TD
    MODE["MODE SELECT (SW1+SW2)"]:::ui --> MODE_SEL["SYNTH / DRUMS / FX / LOOPER"]:::math

    MIDI["MIDI"]:::ui --> POLY["8-voice Poly"]:::audio
    POLY --> FXB["FX Bus"]:::audio

    CLK["Metro"]:::ctrl --> SEQ["16-Step Seq"]:::math
    SEQ --> DRUMS["6 Drums"]:::audio
    DRUMS --> FXB

    AUDIN["Audio In L+R"]:::ui --> FXCH["Serial FX Chain"]:::audio
    FXCH --> FXB

    AUDIN2["Audio In"]:::ui --> LOOP["8-slot Looper"]:::audio
    LOOP --> MIX["Mixer"]:::math
    MIX --> FXB

    FXB --> OUT["MASTER OUTPUT (with Limiter)"]:::ui

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## Summary Table

| # | Project | Complexity | Key Modules | Keys Function |
|---|---------|------------|-------------|---------------|
| 1 | Drone Station | ★☆☆☆☆ | Oscillator | Note triggers |
| 2 | Subtractive Monosynth | ★★☆☆☆ | Osc+Svf+Adsr | Note triggers |
| 3 | Mini Drum Machine | ★★☆☆☆ | DrumSynths | Pattern steps |
| 4 | Delay + Reverb FX | ★★★☆☆ | DelayLine+Reverb | Bypass toggles |
| 5 | FM Synthesizer | ★★★☆☆ | Fm2 | Note triggers |
| 6 | StringVoice Synth | ★★★☆☆ | StringVoice | Note triggers |
| 7 | Granular Texture | ★★★★☆ | GrainletOsc | Param select |
| 8 | Poly Modal Synth | ★★★★☆ | ModalVoice | Param select |
| 9 | Formant Vowel Synth | ★★★★☆ | FormantOsc | Vowel select |
| 10 | Performance Workstation | ★★★★★ | All | Mode-dependent |
| 11 | MIDI Poly + XY Pad | ★★★☆☆ | Osc+Svf | XY pad zones |
| 12 | Step Sequencer | ★★★☆☆ | Metro+MIDI | Step select |
| 13 | Drum Machine Pro | ★★★☆☆ | DrumSynths | Step toggle |
| 14 | Multi-FX Parallel | ★★★☆☆ | Chorus+Delay+Reverb | FX select |
| 15 | Multi-FX Serial | ★★★★☆ | 9 FX types | Chain config |
| 16 | Wavetable Synth | ★★★★☆ | Custom tables | Bank select |
| 17 | Arpeggiator Synth | ★★★★☆ | Arp+Synth | Pattern select |
| 18 | Looper Sampler | ★★★★☆ | DelayLine buffers | Slot trigger |
| 19 | Modular Patcher | ★★★★★ | All modules | Patch matrix |
| 20 | Live Performance Hub | ★★★★★ | Everything | Mode-specific |

---

**Generated per DAISY_EXPERT_SYSTEM_PROMPT_v5.2 guidelines**
**All designs use OLED Zoom Visualization and fonepole smoothing**

---