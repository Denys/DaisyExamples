# Daisy Field Project Ideas
*Generated from literature research - Complexity 5-10/10*
*Revised: CV/Gate controls replaced with Daisy Field-appropriate interfaces*

---

## Project 1: Quantized Pitch Arpeggiator (Complexity: 5/10)

### Description
A simple step sequencer that takes incoming MIDI notes and arpeggiates them through user-defined patterns. Based on discrete MIDI note sequencing methods from "Sound and Music Projects for Eurorack."

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Input notes to arpeggiate |
| Knob 1 | Tempo (BPM) |
| Knob 2 | Pattern selection (Up, Down, Up-Down, Random) |
| Knob 3 | Octave range (1-4) |
| Knob 4 | Gate length |
| Knob 5 | Note duration |
| Button 1 | Hold/Latch notes |
| Button 2 | Reset sequence |
| MIDI In | External note input |
| MIDI Out | Arpeggiated sequence output |

### Block Diagram
```mermaid
graph TB
    subgraph Inputs["User Controls & Interfaces"]
        KEY["🟢 Keyboard<br/>Note Input"]
        MIDI_IN["🟢 MIDI In"]
        K1["🟢 Knob 1<br/>Tempo"]
        K2["🟢 Knob 2<br/>Pattern"]
        K3["🟢 Knob 3<br/>Octave Range"]
        K4["🟢 Knob 4<br/>Gate Length"]
        BTN1["🟢 Button 1<br/>Hold"]
        BTN2["🟢 Button 2<br/>Reset"]
    end

    subgraph Processing["Utilities & Math"]
        BUF["🟣 Note Buffer<br/>Array Storage"]
        SORT["🟣 Note Sorter<br/>Pitch Order"]
        SEQ["🟣 Sequencer Logic<br/>Pattern Generator"]
        CLK["🟣 Clock Divider<br/>Tempo Control"]
    end

    subgraph Modulation["Envelopes & Modulation"]
        GATE["🟠 Gate Generator<br/>Note Triggers"]
    end

    subgraph Outputs["Audio & Interface"]
        MIDI_OUT["🟢 MIDI Out"]
        LED["🟢 LED Indicators<br/>Current Step"]
    end

    KEY --> BUF
    MIDI_IN --> BUF
    BUF --> SORT
    SORT --> SEQ
    K1 --> CLK
    K2 --> SEQ
    K3 --> SEQ
    K4 --> GATE
    CLK --> SEQ
    SEQ --> GATE
    GATE --> MIDI_OUT
    SEQ --> LED
    BTN1 --> BUF
    BTN2 --> SEQ

    style KEY fill:#90EE90
    style MIDI_IN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style BTN1 fill:#90EE90
    style BTN2 fill:#90EE90
    style BUF fill:#DDA0DD
    style SORT fill:#DDA0DD
    style SEQ fill:#DDA0DD
    style CLK fill:#DDA0DD
    style GATE fill:#FFA500
    style MIDI_OUT fill:#90EE90
    style LED fill:#90EE90
```

---

## Project 2: Wave Sequencing Synthesizer (Complexity: 6/10)

### Description
Multi-lane wave sequencing synthesizer inspired by Korg Wavestate, implementing concepts from "Designing Software Synthesizer Plugins in C++." Each lane can have different waveforms sequenced independently.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Pitch control |
| Knob 1 | Lane 1 wave select |
| Knob 2 | Lane 2 wave select |
| Knob 3 | Lane 3 wave select |
| Knob 4 | Sequence speed |
| Knob 5 | Filter cutoff |
| Knob 6 | Filter resonance |
| Knob 7 | Lane mix |
| Knob 8 | Master volume |
| MIDI CC 1 | Sequence speed modulation |
| MIDI CC 2 | Filter cutoff modulation |

### Block Diagram
```mermaid
graph TB
    subgraph Controls["User Controls"]
        KB["🟢 Keyboard"]
        K1["🟢 Knob 1<br/>Wave 1"]
        K2["🟢 Knob 2<br/>Wave 2"]
        K3["🟢 Knob 3<br/>Wave 3"]
        K4["🟢 Knob 4<br/>Speed"]
        K5["🟢 Knob 5<br/>Cutoff"]
        K6["🟢 Knob 6<br/>Resonance"]
        K7["🟢 Knob 7<br/>Mix"]
        MIDI_IN["🟢 MIDI In"]
        CC1["🟢 MIDI CC 1<br/>Speed Mod"]
        CC2["🟢 MIDI CC 2<br/>Cutoff Mod"]
    end

    subgraph Synth["Audio Generation"]
        OSC1["🔵 Oscillator 1<br/>Multi-Wave"]
        OSC2["🔵 Oscillator 2<br/>Multi-Wave"]
        OSC3["🔵 Oscillator 3<br/>Multi-Wave"]
    end

    subgraph Sequencing["Utilities"]
        SEQ1["🟣 Sequencer Lane 1"]
        SEQ2["🟣 Sequencer Lane 2"]
        SEQ3["🟣 Sequencer Lane 3"]
        MIX["🟣 Lane Mixer<br/>3-to-1"]
        QUANT["🟣 Pitch Quantizer"]
        SPEEDSUM["🟣 Speed Summer"]
        CUTSUM["🟣 Cutoff Summer"]
    end

    subgraph Filtering["Audio Effects"]
        FILT["🔵 SVF Filter<br/>Multi-mode"]
    end

    subgraph Output["Audio Output"]
        OUT["🟢 Audio Out<br/>Stereo"]
    end

    KB --> QUANT
    QUANT --> OSC1
    QUANT --> OSC2
    QUANT --> OSC3
    K1 --> SEQ1
    K2 --> SEQ2
    K3 --> SEQ3
    K4 --> SPEEDSUM
    CC1 --> SPEEDSUM
    SPEEDSUM --> SEQ1
    SPEEDSUM --> SEQ2
    SPEEDSUM --> SEQ3
    SEQ1 --> OSC1
    SEQ2 --> OSC2
    SEQ3 --> OSC3
    OSC1 --> MIX
    OSC2 --> MIX
    OSC3 --> MIX
    K7 --> MIX
    MIX --> FILT
    K5 --> CUTSUM
    CC2 --> CUTSUM
    CUTSUM --> FILT
    K6 --> FILT
    FILT --> OUT

    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style MIDI_IN fill:#90EE90
    style CC1 fill:#90EE90
    style CC2 fill:#90EE90
    style OSC1 fill:#87CEEB
    style OSC2 fill:#87CEEB
    style OSC3 fill:#87CEEB
    style SEQ1 fill:#DDA0DD
    style SEQ2 fill:#DDA0DD
    style SEQ3 fill:#DDA0DD
    style MIX fill:#DDA0DD
    style QUANT fill:#DDA0DD
    style SPEEDSUM fill:#DDA0DD
    style CUTSUM fill:#DDA0DD
    style FILT fill:#87CEEB
    style OUT fill:#90EE90
```

---

## Project 3: Harmonizer Effect (Complexity: 5/10)

### Description
Real-time pitch harmonizer creating parallel harmony voices at musical intervals (thirds, fifths). Based on DAFX Digital Audio Effects techniques.

### Controls Mapping
| Control | Function |
|---------|----------|
| Audio In | Input signal |
| Knob 1 | Voice 1 interval (semitones) |
| Knob 2 | Voice 2 interval (semitones) |
| Knob 3 | Voice 3 interval (semitones) |
| Knob 4 | Dry/Wet mix |
| Knob 5 | Voice 1 level |
| Knob 6 | Voice 2 level |
| Knob 7 | Voice 3 level |
| Knob 8 | Key/Scale selection |
| Toggle | Bypass |

### Block Diagram
```mermaid
graph TB
    subgraph Input["Audio Input"]
        AIN["🟢 Audio In"]
        K1["🟢 Knob 1<br/>Interval 1"]
        K2["🟢 Knob 2<br/>Interval 2"]
        K3["🟢 Knob 3<br/>Interval 3"]
        K4["🟢 Knob 4<br/>Dry/Wet"]
        K5["🟢 Knob 5<br/>Level 1"]
        K6["🟢 Knob 6<br/>Level 2"]
        K7["🟢 Knob 7<br/>Level 3"]
        K8["🟢 Knob 8<br/>Scale"]
        TOG["🟢 Toggle<br/>Bypass"]
    end

    subgraph Processing["Audio Effects"]
        PS1["🔵 Pitch Shifter 1<br/>Delay-based"]
        PS2["🔵 Pitch Shifter 2<br/>Delay-based"]
        PS3["🔵 Pitch Shifter 3<br/>Delay-based"]
    end

    subgraph Utils["Utilities"]
        QUANT["🟣 Scale Quantizer<br/>Interval Snap"]
        MIX["🟣 4-Channel Mixer"]
        DRYWET["🟣 Crossfade<br/>Dry/Wet"]
    end

    subgraph Output["Audio Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    AIN --> PS1
    AIN --> PS2
    AIN --> PS3
    AIN --> DRYWET
    K1 --> QUANT
    K2 --> QUANT
    K3 --> QUANT
    K8 --> QUANT
    QUANT --> PS1
    QUANT --> PS2
    QUANT --> PS3
    PS1 --> MIX
    PS2 --> MIX
    PS3 --> MIX
    K5 --> MIX
    K6 --> MIX
    K7 --> MIX
    MIX --> DRYWET
    K4 --> DRYWET
    DRYWET --> AOUT
    TOG --> DRYWET

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style TOG fill:#90EE90
    style PS1 fill:#87CEEB
    style PS2 fill:#87CEEB
    style PS3 fill:#87CEEB
    style QUANT fill:#DDA0DD
    style MIX fill:#DDA0DD
    style DRYWET fill:#DDA0DD
    style AOUT fill:#90EE90
```

---

## Project 4: MIDI-Controlled Polyphonic Synthesizer (Complexity: 7/10)

### Description
6-voice polyphonic synthesizer with MIDI control over pitch, filter, and amplitude. Implements voice allocation and envelope generation per voice.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Polyphonic note input |
| Knob 1 | Filter cutoff |
| Knob 2 | Filter resonance |
| Knob 3 | Attack time |
| Knob 4 | Decay time |
| Knob 5 | Sustain level |
| Knob 6 | Release time |
| Knob 7 | Oscillator detune |
| Knob 8 | Master volume |
| MIDI CC 1 | Filter cutoff modulation |
| MIDI CC 2 | Resonance modulation |
| MIDI CC 74 | Filter envelope amount |
| MIDI Velocity | VCA dynamics |

### Block Diagram
```mermaid
graph TB
    subgraph Controls["User Interface"]
        KB["🟢 Keyboard<br/>Polyphonic"]
        MIDI["🟢 MIDI In"]
        K1["🟢 Knob 1<br/>Cutoff"]
        K2["🟢 Knob 2<br/>Resonance"]
        K3["🟢 Knob 3<br/>Attack"]
        K4["🟢 Knob 4<br/>Decay"]
        K5["🟢 Knob 5<br/>Sustain"]
        K6["🟢 Knob 6<br/>Release"]
        K7["🟢 Knob 7<br/>Detune"]
        CC1["🟢 MIDI CC 1<br/>Cut Mod"]
        CC2["🟢 MIDI CC 2<br/>Res Mod"]
        CC74["🟢 MIDI CC 74<br/>Env Amt"]
    end

    subgraph VoiceAlloc["Utilities"]
        ALLOC["🟣 Voice Allocator<br/>6 voices"]
        CUTSUM["🟣 Cutoff Summer"]
        RESSUM["🟣 Resonance Summer"]
    end

    subgraph VoiceEngine["Audio Sources & Envelopes"]
        OSC["🔵 6x Oscillator Bank<br/>Saw/Square/Tri"]
        ENV["🟠 6x ADSR Envelopes"]
        VCA["🟠 6x VCA"]
    end

    subgraph Filter["Audio Effects"]
        FILT["🔵 6x SVF Filters<br/>Parallel"]
    end

    subgraph Mix["Utilities"]
        VMIX["🟣 Voice Mixer<br/>6-to-2"]
    end

    subgraph Outputs["Outputs"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    KB --> ALLOC
    MIDI --> ALLOC
    ALLOC --> OSC
    K7 --> OSC
    OSC --> VCA
    ALLOC --> ENV
    K3 --> ENV
    K4 --> ENV
    K5 --> ENV
    K6 --> ENV
    ENV --> VCA
    MIDI --> VCA
    VCA --> FILT
    K1 --> CUTSUM
    CC1 --> CUTSUM
    CC74 --> CUTSUM
    ENV --> CUTSUM
    CUTSUM --> FILT
    K2 --> RESSUM
    CC2 --> RESSUM
    RESSUM --> FILT
    FILT --> VMIX
    VMIX --> AOUT
    K8 --> AOUT

    style KB fill:#90EE90
    style MIDI fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style CC1 fill:#90EE90
    style CC2 fill:#90EE90
    style CC74 fill:#90EE90
    style ALLOC fill:#DDA0DD
    style CUTSUM fill:#DDA0DD
    style RESSUM fill:#DDA0DD
    style OSC fill:#87CEEB
    style ENV fill:#FFA500
    style VCA fill:#FFA500
    style FILT fill:#87CEEB
    style VMIX fill:#DDA0DD
    style AOUT fill:#90EE90
```

---

## Project 5: Markov Chain Melody Generator (Complexity: 8/10)

### Description
AI-enhanced melodic sequence generator using Markov models for probabilistic note generation. Based on "Build AI-Enhanced Audio Plugins with C++."

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Train Markov model |
| Knob 1 | Randomness/Temperature |
| Knob 2 | Note density |
| Knob 3 | Scale root |
| Knob 4 | Scale type |
| Knob 5 | Octave range |
| Knob 6 | Rhythm variation |
| Knob 7 | Sequence length |
| Knob 8 | Tempo |
| Button 1 | Record pattern |
| Button 2 | Generate/Play |
| MIDI In | Pattern training input |
| MIDI Out | Generated melody |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard<br/>Training"]
        MIDI_IN["🟢 MIDI In"]
        K1["🟢 Knob 1<br/>Randomness"]
        K2["🟢 Knob 2<br/>Density"]
        K3["🟢 Knob 3<br/>Root"]
        K4["🟢 Knob 4<br/>Scale"]
        K5["🟢 Knob 5<br/>Octaves"]
        K6["🟢 Knob 6<br/>Rhythm Var"]
        K7["🟢 Knob 7<br/>Length"]
        K8["🟢 Knob 8<br/>Tempo"]
        B1["🟢 Button 1<br/>Record"]
        B2["🟢 Button 2<br/>Generate"]
    end

    subgraph AI["Utilities - AI Engine"]
        MARKOV["🟣 Markov Model<br/>State Machine"]
        TRANS["🟣 Transition Matrix<br/>Probabilities"]
        RNG["🟣 Weighted Random<br/>Generator"]
    end

    subgraph Processing["Utilities - Processing"]
        QUANT["🟣 Scale Quantizer"]
        CLK["🟣 Clock Generator<br/>Tempo Sync"]
        RHYTHM["🟣 Rhythm Generator<br/>Euclidean"]
    end

    subgraph Modulation["Envelopes"]
        GATE["🟠 Gate Generator"]
    end

    subgraph Output["Outputs"]
        MIDI_OUT["🟢 MIDI Out"]
        LED["🟢 LED Display<br/>Status"]
    end

    KB --> MARKOV
    MIDI_IN --> MARKOV
    B1 --> MARKOV
    MARKOV --> TRANS
    K1 --> RNG
    TRANS --> RNG
    B2 --> RNG
    RNG --> QUANT
    K3 --> QUANT
    K4 --> QUANT
    K5 --> QUANT
    QUANT --> GATE
    K8 --> CLK
    K6 --> RHYTHM
    K7 --> RHYTHM
    CLK --> RHYTHM
    RHYTHM --> GATE
    K2 --> GATE
    GATE --> MIDI_OUT
    RNG --> LED

    style KB fill:#90EE90
    style MIDI_IN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style MARKOV fill:#DDA0DD
    style TRANS fill:#DDA0DD
    style RNG fill:#DDA0DD
    style QUANT fill:#DDA0DD
    style CLK fill:#DDA0DD
    style RHYTHM fill:#DDA0DD
    style GATE fill:#FFA500
    style MIDI_OUT fill:#90EE90
    style LED fill:#90EE90
```

---

## Project 6: Granular Sampler with LFO Modulation (Complexity: 8/10)

### Description
Real-time granular synthesis engine with SD card sample loading and extensive internal LFO modulation capabilities.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Pitch/playback speed |
| Knob 1 | Grain size |
| Knob 2 | Grain density |
| Knob 3 | Grain position |
| Knob 4 | Grain pitch shift |
| Knob 5 | Grain envelope shape |
| Knob 6 | Spray/randomness |
| Knob 7 | Filter cutoff |
| Knob 8 | Dry/wet mix |
| Button 1 | Load previous sample / Trigger grain |
| Button 2 | Load next sample |
| Toggle | LFO modulation routing |

### Block Diagram
```mermaid
graph TB
    subgraph Controls["User Interface"]
        KB["🟢 Keyboard"]
        K1["🟢 Knob 1<br/>Grain Size"]
        K2["🟢 Knob 2<br/>Density"]
        K3["🟢 Knob 3<br/>Position"]
        K4["🟢 Knob 4<br/>Pitch"]
        K5["🟢 Knob 5<br/>Envelope"]
        K6["🟢 Knob 6<br/>Spray"]
        K7["🟢 Knob 7<br/>Filter"]
        K8["🟢 Knob 8<br/>Mix"]
        B1["🟢 Button 1<br/>Prev/Trig"]
        B2["🟢 Button 2<br/>Next"]
        TOG["🟢 Toggle<br/>LFO Route"]
    end

    subgraph Storage["Sample Storage"]
        SD["🟣 SD Card Reader<br/>WAV Files"]
        BUF["🟣 Sample Buffer<br/>RAM"]
    end

    subgraph Modulation["Modulation Sources"]
        LFO1["🟠 LFO 1<br/>Position Mod"]
        LFO2["🟠 LFO 2<br/>Size Mod"]
        LFO3["🟠 LFO 3<br/>Density Mod"]
    end

    subgraph GranEngine["Audio Sources & Processing"]
        GRAN["🔵 Granular Engine<br/>8-voice polyphony"]
        PITCH["🔵 Pitch Shifter<br/>Per-grain"]
    end

    subgraph Envelope["Envelopes & Modulation"]
        GENV["🟠 Grain Envelopes<br/>Windowing"]
        RND["🟠 Random Modulation<br/>Spray"]
    end

    subgraph Utils["Utilities"]
        POSSUM["🟣 Position Summer"]
        SIZESUM["🟣 Size Summer"]
        DENSSUM["🟣 Density Summer"]
        MIX["🟣 Dry/Wet Mix"]
        ROUTER["🟣 LFO Router"]
    end

    subgraph Filter["Audio Effects"]
        FILT["🔵 Lowpass Filter"]
    end

    subgraph Output["Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    B1 --> SD
    B2 --> SD
    SD --> BUF
    BUF --> GRAN
    K3 --> POSSUM
    TOG --> ROUTER
    LFO1 --> ROUTER
    LFO2 --> ROUTER
    LFO3 --> ROUTER
    ROUTER --> POSSUM
    K6 --> RND
    RND --> POSSUM
    POSSUM --> GRAN
    K1 --> SIZESUM
    ROUTER --> SIZESUM
    SIZESUM --> GRAN
    K2 --> DENSSUM
    ROUTER --> DENSSUM
    DENSSUM --> GRAN
    B1 --> GRAN
    K5 --> GENV
    GENV --> GRAN
    GRAN --> PITCH
    K4 --> PITCH
    KB --> PITCH
    PITCH --> FILT
    K7 --> FILT
    FILT --> MIX
    K8 --> MIX
    MIX --> AOUT

    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style TOG fill:#90EE90
    style SD fill:#DDA0DD
    style BUF fill:#DDA0DD
    style LFO1 fill:#FFA500
    style LFO2 fill:#FFA500
    style LFO3 fill:#FFA500
    style GRAN fill:#87CEEB
    style PITCH fill:#87CEEB
    style GENV fill:#FFA500
    style RND fill:#FFA500
    style POSSUM fill:#DDA0DD
    style SIZESUM fill:#DDA0DD
    style DENSSUM fill:#DDA0DD
    style MIX fill:#DDA0DD
    style ROUTER fill:#DDA0DD
    style FILT fill:#87CEEB
    style AOUT fill:#90EE90
```

---

## Project 7: Euclidean Rhythm Synthesizer (Complexity: 6/10)

### Description
Percussion synthesizer with Euclidean rhythm generation for up to 4 voices. Each voice has its own synthesis engine and rhythm pattern. MIDI output for external sync.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Voice 1 steps |
| Knob 2 | Voice 1 hits |
| Knob 3 | Voice 2 steps |
| Knob 4 | Voice 2 hits |
| Knob 5 | Voice 3 steps |
| Knob 6 | Voice 3 hits |
| Knob 7 | Voice 4 steps |
| Knob 8 | Voice 4 hits |
| MIDI CC 1 | Master tempo modulation |
| Button 1 | Start/Stop / Pattern rotation trigger |
| Button 2 | Reset all patterns |
| MIDI Out | Clock output (24 PPQN) |

### Block Diagram
```mermaid
graph TB
    subgraph Controls["User Interface"]
        K1["🟢 Knob 1<br/>V1 Steps"]
        K2["🟢 Knob 2<br/>V1 Hits"]
        K3["🟢 Knob 3<br/>V2 Steps"]
        K4["🟢 Knob 4<br/>V2 Hits"]
        K5["🟢 Knob 5<br/>V3 Steps"]
        K6["🟢 Knob 6<br/>V3 Hits"]
        K7["🟢 Knob 7<br/>V4 Steps"]
        K8["🟢 Knob 8<br/>V4 Hits"]
        CC1["🟢 MIDI CC 1<br/>Tempo Mod"]
        B1["🟢 Button 1<br/>Start/Rotate"]
        B2["🟢 Button 2<br/>Reset"]
    end

    subgraph Sequencers["Utilities - Sequencing"]
        MCLK["🟣 Master Clock"]
        TEMPOSUM["🟣 Tempo Summer"]
        EUC1["🟣 Euclidean Gen 1"]
        EUC2["🟣 Euclidean Gen 2"]
        EUC3["🟣 Euclidean Gen 3"]
        EUC4["🟣 Euclidean Gen 4"]
        ROT["🟣 Pattern Rotator"]
    end

    subgraph Synthesis["Audio Sources"]
        KICK["🔵 Analog Kick<br/>Drum Synth"]
        SNARE["🔵 Analog Snare<br/>Drum Synth"]
        HAT["🔵 Noise + Filter<br/>Hi-hat"]
        TOM["🔵 Modal Synthesis<br/>Tom"]
    end

    subgraph Envelopes["Envelopes"]
        ENV1["🟠 AD Envelope 1"]
        ENV2["🟠 AD Envelope 2"]
        ENV3["🟠 AD Envelope 3"]
        ENV4["🟠 AD Envelope 4"]
    end

    subgraph Mix["Utilities - Mixing"]
        DMIX["🟣 Drum Mixer<br/>4-to-2"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 Audio Out<br/>Stereo"]
        MIDIOUT["🟢 MIDI Out<br/>Clock"]
    end

    CC1 --> TEMPOSUM
    TEMPOSUM --> MCLK
    B1 --> MCLK
    B2 --> MCLK
    MCLK --> EUC1
    MCLK --> EUC2
    MCLK --> EUC3
    MCLK --> EUC4
    K1 --> EUC1
    K2 --> EUC1
    K3 --> EUC2
    K4 --> EUC2
    K5 --> EUC3
    K6 --> EUC3
    K7 --> EUC4
    K8 --> EUC4
    B1 --> ROT
    ROT --> EUC1
    ROT --> EUC2
    ROT --> EUC3
    ROT --> EUC4
    EUC1 --> ENV1
    EUC2 --> ENV2
    EUC3 --> ENV3
    EUC4 --> ENV4
    ENV1 --> KICK
    ENV2 --> SNARE
    ENV3 --> HAT
    ENV4 --> TOM
    KICK --> DMIX
    SNARE --> DMIX
    HAT --> DMIX
    TOM --> DMIX
    DMIX --> AOUT
    MCLK --> MIDIOUT

    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style CC1 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style MCLK fill:#DDA0DD
    style TEMPOSUM fill:#DDA0DD
    style EUC1 fill:#DDA0DD
    style EUC2 fill:#DDA0DD
    style EUC3 fill:#DDA0DD
    style EUC4 fill:#DDA0DD
    style ROT fill:#DDA0DD
    style KICK fill:#87CEEB
    style SNARE fill:#87CEEB
    style HAT fill:#87CEEB
    style TOM fill:#87CEEB
    style ENV1 fill:#FFA500
    style ENV2 fill:#FFA500
    style ENV3 fill:#FFA500
    style ENV4 fill:#FFA500
    style DMIX fill:#DDA0DD
    style AOUT fill:#90EE90
    style MIDIOUT fill:#90EE90
```

---

## Project 8: Multi-FX Processor with Preset Management (Complexity: 7/10)

### Description
Chain-able multi-effects processor with 8 effect slots and preset save/recall via SD card.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Effect 1 parameter |
| Knob 2 | Effect 2 parameter |
| Knob 3 | Effect 3 parameter |
| Knob 4 | Effect 4 parameter |
| Knob 5 | Effect 5 parameter |
| Knob 6 | Effect 6 parameter |
| Knob 7 | Effect 7 parameter |
| Knob 8 | Effect 8 parameter |
| Keyboard | Effect selection per slot |
| Button 1 | Save preset |
| Button 2 | Load preset |
| Toggle | Bypass all |

### Block Diagram
```mermaid
graph TB
    subgraph Input["Input & Controls"]
        AIN["🟢 Audio In"]
        KB["🟢 Keyboard<br/>FX Select"]
        K1["🟢 Knob 1<br/>Param 1"]
        K2["🟢 Knob 2<br/>Param 2"]
        K3["🟢 Knob 3<br/>Param 3"]
        K4["🟢 Knob 4<br/>Param 4"]
        K5["🟢 Knob 5<br/>Param 5"]
        K6["🟢 Knob 6<br/>Param 6"]
        K7["🟢 Knob 7<br/>Param 7"]
        K8["🟢 Knob 8<br/>Param 8"]
        B1["🟢 Button 1<br/>Save"]
        B2["🟢 Button 2<br/>Load"]
        TOG["🟢 Toggle<br/>Bypass"]
    end

    subgraph Storage["Preset Management"]
        SD["🟣 SD Card<br/>Preset Storage"]
        PRESET["🟣 Preset Manager<br/>State Machine"]
    end

    subgraph FXChain["Audio Effects Chain"]
        FX1["🔵 Effect Slot 1<br/>Selectable"]
        FX2["🔵 Effect Slot 2<br/>Selectable"]
        FX3["🔵 Effect Slot 3<br/>Selectable"]
        FX4["🔵 Effect Slot 4<br/>Selectable"]
        FX5["🔵 Effect Slot 5<br/>Selectable"]
        FX6["🔵 Effect Slot 6<br/>Selectable"]
        FX7["🔵 Effect Slot 7<br/>Selectable"]
        FX8["🔵 Effect Slot 8<br/>Selectable"]
    end

    subgraph Utils["Utilities"]
        ROUTER["🟣 Signal Router<br/>Chain Manager"]
        BYP["🟣 Bypass Switch"]
    end

    subgraph Output["Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
        LED["🟢 LED Indicators"]
    end

    AIN --> ROUTER
    ROUTER --> FX1
    FX1 --> FX2
    FX2 --> FX3
    FX3 --> FX4
    FX4 --> FX5
    FX5 --> FX6
    FX6 --> FX7
    FX7 --> FX8
    KB --> ROUTER
    K1 --> FX1
    K2 --> FX2
    K3 --> FX3
    K4 --> FX4
    K5 --> FX5
    K6 --> FX6
    K7 --> FX7
    K8 --> FX8
    B1 --> PRESET
    B2 --> PRESET
    PRESET --> SD
    PRESET --> ROUTER
    FX8 --> BYP
    AIN --> BYP
    TOG --> BYP
    BYP --> AOUT
    ROUTER --> LED

    style AIN fill:#90EE90
    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style TOG fill:#90EE90
    style SD fill:#DDA0DD
    style PRESET fill:#DDA0DD
    style FX1 fill:#87CEEB
    style FX2 fill:#87CEEB
    style FX3 fill:#87CEEB
    style FX4 fill:#87CEEB
    style FX5 fill:#87CEEB
    style FX6 fill:#87CEEB
    style FX7 fill:#87CEEB
    style FX8 fill:#87CEEB
    style ROUTER fill:#DDA0DD
    style BYP fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED fill:#90EE90
```

---

## Project 9: Modal Resonator Bank (Complexity: 6/10)

### Description
Physical modeling synthesizer using modal synthesis to simulate resonant materials (metal, wood, glass). Based on modal voice techniques with internal LFO modulation.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Strike/excitation trigger + pitch |
| Knob 1 | Material selection |
| Knob 2 | Brightness/harmonic content |
| Knob 3 | Damping/decay time |
| Knob 4 | Structure/geometry |
| Knob 5 | Accent/strike hardness |
| Knob 6 | Resonator mix |
| Knob 7 | Reverb amount |
| Knob 8 | Master volume |
| Toggle | LFO modulation on/off |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard<br/>Trigger + Pitch"]
        K1["🟢 Knob 1<br/>Material"]
        K2["🟢 Knob 2<br/>Brightness"]
        K3["🟢 Knob 3<br/>Damping"]
        K4["🟢 Knob 4<br/>Structure"]
        K5["🟢 Knob 5<br/>Accent"]
        K6["🟢 Knob 6<br/>Mix"]
        K7["🟢 Knob 7<br/>Reverb"]
        K8["🟢 Knob 8<br/>Volume"]
        TOG["🟢 Toggle<br/>LFO Mod"]
    end

    subgraph Modulation["Modulation Sources"]
        LFO1["🟠 LFO 1<br/>Brightness Mod"]
        LFO2["🟠 LFO 2<br/>Damping Mod"]
    end

    subgraph Excitation["Audio Sources"]
        NOISE["🔵 Noise Burst<br/>Exciter"]
        PULSE["🔵 Impulse<br/>Generator"]
    end

    subgraph Envelope["Envelopes"]
        EXENV["🟠 Excitation Env<br/>Short Decay"]
    end

    subgraph Modal["Audio Effects - Modal"]
        RES1["🔵 Modal Resonator 1"]
        RES2["🔵 Modal Resonator 2"]
        RES3["🔵 Modal Resonator 3"]
        RES4["🔵 Modal Resonator 4"]
    end

    subgraph Utils["Utilities"]
        MATSEL["🟣 Material Bank<br/>Preset Ratios"]
        RESMIX["🟣 Resonator Mixer"]
        BRIGHTSUM["🟣 Brightness Summer"]
        DAMPSUM["🟣 Damping Summer"]
    end

    subgraph Effects["Audio Effects"]
        VERB["🔵 Reverb<br/>Shimmer"]
    end

    subgraph Output["Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    KB --> EXENV
    KB --> PULSE
    K5 --> EXENV
    EXENV --> NOISE
    EXENV --> PULSE
    NOISE --> RES1
    NOISE --> RES2
    PULSE --> RES3
    PULSE --> RES4
    K1 --> MATSEL
    MATSEL --> RES1
    MATSEL --> RES2
    MATSEL --> RES3
    MATSEL --> RES4
    K2 --> BRIGHTSUM
    TOG --> LFO1
    LFO1 --> BRIGHTSUM
    BRIGHTSUM --> RES1
    BRIGHTSUM --> RES2
    K3 --> DAMPSUM
    TOG --> LFO2
    LFO2 --> DAMPSUM
    DAMPSUM --> RES1
    DAMPSUM --> RES2
    DAMPSUM --> RES3
    DAMPSUM --> RES4
    K4 --> RES3
    K4 --> RES4
    RES1 --> RESMIX
    RES2 --> RESMIX
    RES3 --> RESMIX
    RES4 --> RESMIX
    K6 --> RESMIX
    RESMIX --> VERB
    K7 --> VERB
    VERB --> AOUT
    K8 --> AOUT

    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style TOG fill:#90EE90
    style LFO1 fill:#FFA500
    style LFO2 fill:#FFA500
    style NOISE fill:#87CEEB
    style PULSE fill:#87CEEB
    style EXENV fill:#FFA500
    style RES1 fill:#87CEEB
    style RES2 fill:#87CEEB
    style RES3 fill:#87CEEB
    style RES4 fill:#87CEEB
    style MATSEL fill:#DDA0DD
    style RESMIX fill:#DDA0DD
    style BRIGHTSUM fill:#DDA0DD
    style DAMPSUM fill:#DDA0DD
    style VERB fill:#87CEEB
    style AOUT fill:#90EE90
```

---

## Project 10: MIDI Sequence Recorder and Looper (Complexity: 7/10)

### Description
4-track MIDI sequence recorder/looper with quantization and playback speed control. Can record keyboard, knobs, and external MIDI.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | MIDI input source 1 |
| Knob 1 | Track 1 transpose |
| Knob 2 | Track 2 transpose |
| Knob 3 | Track 3 transpose |
| Knob 4 | Track 4 transpose |
| Knob 5 | Playback speed (all tracks) |
| Knob 6 | Quantization amount |
| Knob 7 | Loop length |
| Knob 8 | Track select (1-4) |
| MIDI In | External MIDI inputs |
| MIDI Out | Playback outputs |
| Button 1 | Rec/Overdub |
| Button 2 | Clear selected track |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard<br/>MIDI Source"]
        MIDI_IN["🟢 MIDI In"]
        K1["🟢 Knob 1<br/>Transp 1"]
        K2["🟢 Knob 2<br/>Transp 2"]
        K3["🟢 Knob 3<br/>Transp 3"]
        K4["🟢 Knob 4<br/>Transp 4"]
        K5["🟢 Knob 5<br/>Speed"]
        K6["🟢 Knob 6<br/>Quantize"]
        K7["🟢 Knob 7<br/>Length"]
        K8["🟢 Knob 8<br/>Track Sel"]
        B1["🟢 Button 1<br/>Rec"]
        B2["🟢 Button 2<br/>Clear"]
    end

    subgraph Recording["Utilities - Recording"]
        BUF1["🟣 Loop Buffer 1<br/>MIDI Events"]
        BUF2["🟣 Loop Buffer 2<br/>MIDI Events"]
        BUF3["🟣 Loop Buffer 3<br/>MIDI Events"]
        BUF4["🟣 Loop Buffer 4<br/>MIDI Events"]
        CLK["🟣 Master Clock<br/>Sync"]
    end

    subgraph Processing["Utilities - Processing"]
        QUANT["🟣 MIDI Quantizer<br/>Scale Snap"]
        SPEED["🟣 Playback Speed<br/>Time Stretch"]
        TRANS1["🟣 Transpose 1"]
        TRANS2["🟣 Transpose 2"]
        TRANS3["🟣 Transpose 3"]
        TRANS4["🟣 Transpose 4"]
    end

    subgraph Playback["Utilities - Playback"]
        PLAY1["🟣 Playback Head 1"]
        PLAY2["🟣 Playback Head 2"]
        PLAY3["🟣 Playback Head 3"]
        PLAY4["🟣 Playback Head 4"]
    end

    subgraph Output["Outputs"]
        MIDI_OUT["🟢 MIDI Out"]
        LED["🟢 LED Status"]
    end

    KB --> BUF1
    MIDI_IN --> BUF1
    MIDI_IN --> BUF2
    MIDI_IN --> BUF3
    MIDI_IN --> BUF4
    B1 --> BUF1
    B1 --> BUF2
    B1 --> BUF3
    B1 --> BUF4
    B2 --> BUF1
    B2 --> BUF2
    B2 --> BUF3
    B2 --> BUF4
    K8 --> BUF1
    K8 --> BUF2
    K8 --> BUF3
    K8 --> BUF4
    K7 --> CLK
    CLK --> BUF1
    CLK --> BUF2
    CLK --> BUF3
    CLK --> BUF4
    K5 --> SPEED
    BUF1 --> PLAY1
    BUF2 --> PLAY2
    BUF3 --> PLAY3
    BUF4 --> PLAY4
    SPEED --> PLAY1
    SPEED --> PLAY2
    SPEED --> PLAY3
    SPEED --> PLAY4
    PLAY1 --> TRANS1
    PLAY2 --> TRANS2
    PLAY3 --> TRANS3
    PLAY4 --> TRANS4
    K1 --> TRANS1
    K2 --> TRANS2
    K3 --> TRANS3
    K4 --> TRANS4
    TRANS1 --> QUANT
    TRANS2 --> QUANT
    TRANS3 --> QUANT
    TRANS4 --> QUANT
    K6 --> QUANT
    QUANT --> MIDI_OUT
    B1 --> LED

    style KB fill:#90EE90
    style MIDI_IN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style BUF1 fill:#DDA0DD
    style BUF2 fill:#DDA0DD
    style BUF3 fill:#DDA0DD
    style BUF4 fill:#DDA0DD
    style CLK fill:#DDA0DD
    style QUANT fill:#DDA0DD
    style SPEED fill:#DDA0DD
    style TRANS1 fill:#DDA0DD
    style TRANS2 fill:#DDA0DD
    style TRANS3 fill:#DDA0DD
    style TRANS4 fill:#DDA0DD
    style PLAY1 fill:#DDA0DD
    style PLAY2 fill:#DDA0DD
    style PLAY3 fill:#DDA0DD
    style PLAY4 fill:#DDA0DD
    style MIDI_OUT fill:#90EE90
    style LED fill:#90EE90
```

---

## Project 11: West Coast Style Synthesizer (Complexity: 8/10)

### Description
Complex modulation synthesizer with wavefolding, FM, and low-pass gate. Inspired by Buchla synthesizers and west coast synthesis philosophy. Uses internal modulation routing.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Base pitch + trigger |
| Knob 1 | Oscillator 1 waveform |
| Knob 2 | Oscillator 2 waveform |
| Knob 3 | FM amount (Osc 2 -> Osc 1) |
| Knob 4 | Wavefolder depth |
| Knob 5 | LPG resonance |
| Knob 6 | Envelope attack |
| Knob 7 | Envelope decay |
| Knob 8 | LFO rate |
| Toggle | LFO routing (FM/Fold/LPG) |
| Button 1 | Retrigger envelope |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard"]
        K1["🟢 Knob 1<br/>Wave 1"]
        K2["🟢 Knob 2<br/>Wave 2"]
        K3["🟢 Knob 3<br/>FM Amt"]
        K4["🟢 Knob 4<br/>Fold"]
        K5["🟢 Knob 5<br/>Resonance"]
        K6["🟢 Knob 6<br/>Attack"]
        K7["🟢 Knob 7<br/>Decay"]
        K8["🟢 Knob 8<br/>LFO Rate"]
        TOG["🟢 Toggle<br/>LFO Route"]
        B1["🟢 Button 1<br/>Retrig"]
    end

    subgraph Oscillators["Audio Sources"]
        OSC1["🔵 Complex Osc 1<br/>Through-Zero FM"]
        OSC2["🔵 Complex Osc 2<br/>Modulator"]
    end

    subgraph Modulation["Modulation Sources"]
        LFO["🟠 LFO<br/>Multi-shape"]
        ENV["🟠 AD Envelope"]
    end

    subgraph Utils["Utilities"]
        FMSUM["🟣 FM Summer"]
        FOLDSUM["🟣 Fold Summer"]
        LPGSUM["🟣 LPG Summer"]
        LFOROUTER["🟣 LFO Router"]
    end

    subgraph Shaping["Audio Effects"]
        FOLD["🔵 Wavefolder<br/>Nonlinear"]
        LPG["🔵 Low-Pass Gate<br/>Vactrol Model"]
    end

    subgraph Output["Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    KB --> OSC1
    KB --> OSC2
    K1 --> OSC1
    K2 --> OSC2
    K3 --> FMSUM
    K8 --> LFO
    TOG --> LFOROUTER
    LFO --> LFOROUTER
    LFOROUTER --> FMSUM
    FMSUM --> OSC1
    OSC2 --> OSC1
    OSC1 --> FOLD
    K4 --> FOLDSUM
    LFOROUTER --> FOLDSUM
    ENV --> FOLDSUM
    FOLDSUM --> FOLD
    FOLD --> LPG
    K5 --> LPG
    K6 --> ENV
    K7 --> ENV
    KB --> ENV
    B1 --> ENV
    ENV --> LPGSUM
    LFOROUTER --> LPGSUM
    LPGSUM --> LPG
    LPG --> AOUT

    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style TOG fill:#90EE90
    style B1 fill:#90EE90
    style OSC1 fill:#87CEEB
    style OSC2 fill:#87CEEB
    style LFO fill:#FFA500
    style ENV fill:#FFA500
    style FMSUM fill:#DDA0DD
    style FOLDSUM fill:#DDA0DD
    style LPGSUM fill:#DDA0DD
    style LFOROUTER fill:#DDA0DD
    style FOLD fill:#87CEEB
    style LPG fill:#87CEEB
    style AOUT fill:#90EE90
```

---

## Project 12: Probabilistic Step Sequencer (Complexity: 9/10)

### Description
Advanced 16-step sequencer with per-step probability, ratcheting, slides, and multiple simultaneous sequences. Inspired by modern Eurorack sequencers. MIDI-based control and output.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Step selection (12 keys) + note input |
| Knob 1 | Pitch (selected step) |
| Knob 2 | Probability (selected step) |
| Knob 3 | Ratchet count (selected step) |
| Knob 4 | Slide time (selected step) |
| Knob 5 | Master tempo |
| Knob 6 | Sequence length (1-16) |
| Knob 7 | Swing amount |
| Knob 8 | Gate length |
| MIDI CC 1 | Tempo modulation |
| MIDI CC 2 | Probability bias |
| Button 1 | Play/Stop |
| Button 2 | Reset sequence |
| MIDI Out | Sequence output |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard<br/>Step Sel"]
        K1["🟢 Knob 1<br/>Pitch"]
        K2["🟢 Knob 2<br/>Probability"]
        K3["🟢 Knob 3<br/>Ratchet"]
        K4["🟢 Knob 4<br/>Slide"]
        K5["🟢 Knob 5<br/>Tempo"]
        K6["🟢 Knob 6<br/>Length"]
        K7["🟢 Knob 7<br/>Swing"]
        K8["🟢 Knob 8<br/>Gate Len"]
        CC1["🟢 MIDI CC 1<br/>Tempo Mod"]
        CC2["🟢 MIDI CC 2<br/>Prob Bias"]
        B1["🟢 Button 1<br/>Play"]
        B2["🟢 Button 2<br/>Reset"]
    end

    subgraph Sequencer["Utilities - Sequencing"]
        CLK["🟣 Master Clock<br/>With Swing"]
        TEMPOSUM["🟣 Tempo Summer"]
        SEQ["🟣 Step Counter<br/>16 steps"]
        STEPMEM["🟣 Step Memory<br/>16x4 params"]
    end

    subgraph Probability["Utilities - Probability"]
        RNG["🟣 Random Generator"]
        PROB["🟣 Probability Gate<br/>Per-step"]
        PROBSUM["🟣 Probability Summer"]
        RATCH["🟣 Ratchet Engine<br/>Sub-divisions"]
    end

    subgraph Processing["Utilities - Processing"]
        SLIDE["🟣 Portamento<br/>Pitch Slide"]
        QUANT["🟣 Pitch Quantizer"]
    end

    subgraph Envelope["Envelopes"]
        GATE["🟠 Gate Generator<br/>Variable Length"]
    end

    subgraph Output["Outputs"]
        MIDI_OUT["🟢 MIDI Out"]
        LED["🟢 LED Array<br/>Step Display"]
    end

    K5 --> TEMPOSUM
    CC1 --> TEMPOSUM
    TEMPOSUM --> CLK
    K7 --> CLK
    B1 --> CLK
    B2 --> SEQ
    CLK --> SEQ
    K6 --> SEQ
    KB --> STEPMEM
    K1 --> STEPMEM
    K2 --> STEPMEM
    K3 --> STEPMEM
    K4 --> STEPMEM
    SEQ --> STEPMEM
    STEPMEM --> PROBSUM
    K2 --> PROBSUM
    CC2 --> PROBSUM
    PROBSUM --> PROB
    RNG --> PROB
    PROB --> RATCH
    STEPMEM --> RATCH
    RATCH --> GATE
    K8 --> GATE
    STEPMEM --> SLIDE
    SLIDE --> QUANT
    QUANT --> MIDI_OUT
    GATE --> MIDI_OUT
    SEQ --> LED

    style KB fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style CC1 fill:#90EE90
    style CC2 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style CLK fill:#DDA0DD
    style TEMPOSUM fill:#DDA0DD
    style SEQ fill:#DDA0DD
    style STEPMEM fill:#DDA0DD
    style RNG fill:#DDA0DD
    style PROB fill:#DDA0DD
    style PROBSUM fill:#DDA0DD
    style RATCH fill:#DDA0DD
    style SLIDE fill:#DDA0DD
    style QUANT fill:#DDA0DD
    style GATE fill:#FFA500
    style MIDI_OUT fill:#90EE90
    style LED fill:#90EE90
```

---

## Project 13: Spectral Freezer and Processor (Complexity: 9/10)

### Description
FFT-based spectral effect that can freeze, scramble, and process the frequency spectrum of incoming audio. Advanced DSP project with LFO modulation.

### Controls Mapping
| Control | Function |
|---------|----------|
| Audio In | Input signal |
| Knob 1 | Freeze amount (blend) |
| Knob 2 | Spectral shift (pitch) |
| Knob 3 | Bin scramble amount |
| Knob 4 | Formant preservation |
| Knob 5 | Spectral blur |
| Knob 6 | Freeze decay time |
| Knob 7 | Dry/wet mix |
| Knob 8 | Output gain |
| Button 1 | Capture spectrum |
| Button 2 | Clear freeze buffer |
| Toggle | Freeze on/off |
| MIDI CC 1 | Spectral shift modulation |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 Audio In"]
        K1["🟢 Knob 1<br/>Freeze Mix"]
        K2["🟢 Knob 2<br/>Shift"]
        K3["🟢 Knob 3<br/>Scramble"]
        K4["🟢 Knob 4<br/>Formant"]
        K5["🟢 Knob 5<br/>Blur"]
        K6["🟢 Knob 6<br/>Decay"]
        K7["🟢 Knob 7<br/>Mix"]
        K8["🟢 Knob 8<br/>Gain"]
        B1["🟢 Button 1<br/>Capture"]
        B2["🟢 Button 2<br/>Clear"]
        TOG["🟢 Toggle<br/>Freeze"]
        CC1["🟢 MIDI CC 1<br/>Shift Mod"]
    end

    subgraph Analysis["Audio Effects - FFT"]
        FFT["🔵 FFT Analysis<br/>2048 bins"]
    end

    subgraph Processing["Utilities - Spectral"]
        FBUF["🟣 Freeze Buffer<br/>Spectral Storage"]
        SHIFT["🟣 Spectral Shifter"]
        SHIFTSUM["🟣 Shift Summer"]
        SCRAM["🟣 Bin Scrambler<br/>Random"]
        BLUR["🟣 Spectral Blur<br/>Smoothing"]
        FORM["🟣 Formant Preserver"]
    end

    subgraph Envelope["Envelopes"]
        DECAY["🟠 Decay Envelope<br/>Per-bin"]
    end

    subgraph Synthesis["Audio Effects - Resynthesis"]
        IFFT["🔵 IFFT Synthesis<br/>Overlap-add"]
    end

    subgraph Utils["Utilities"]
        MIX["🟣 Crossfade<br/>Dry/Wet"]
    end

    subgraph Output["Output"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    AIN --> FFT
    FFT --> FBUF
    B1 --> FBUF
    B2 --> FBUF
    TOG --> FBUF
    K1 --> FBUF
    FBUF --> SHIFT
    K2 --> SHIFTSUM
    CC1 --> SHIFTSUM
    SHIFTSUM --> SHIFT
    SHIFT --> SCRAM
    K3 --> SCRAM
    SCRAM --> BLUR
    K5 --> BLUR
    BLUR --> FORM
    K4 --> FORM
    FORM --> DECAY
    K6 --> DECAY
    DECAY --> IFFT
    IFFT --> MIX
    AIN --> MIX
    K7 --> MIX
    MIX --> AOUT
    K8 --> AOUT

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style TOG fill:#90EE90
    style CC1 fill:#90EE90
    style FFT fill:#87CEEB
    style FBUF fill:#DDA0DD
    style SHIFT fill:#DDA0DD
    style SHIFTSUM fill:#DDA0DD
    style SCRAM fill:#DDA0DD
    style BLUR fill:#DDA0DD
    style FORM fill:#DDA0DD
    style DECAY fill:#FFA500
    style IFFT fill:#87CEEB
    style MIX fill:#DDA0DD
    style AOUT fill:#90EE90
```

---

## Project 14: Quad Envelope Follower and VCA (Complexity: 6/10)

### Description
Four-channel envelope follower with individual VCA controls. Perfect for sidechain compression, ducking, and dynamic processing chains. Uses audio input for external sidechain.

### Controls Mapping
| Control | Function |
|---------|----------|
| Audio In L | Channel 1 & 2 input |
| Audio In R | Channel 3 & 4 input (or sidechain) |
| Knob 1 | Ch1 attack time |
| Knob 2 | Ch1 release time |
| Knob 3 | Ch2 attack time |
| Knob 4 | Ch2 release time |
| Knob 5 | Ch3 attack time |
| Knob 6 | Ch3 release time |
| Knob 7 | Ch4 attack time |
| Knob 8 | Ch4 release time |
| Toggle | External sidechain mode |
| MIDI Out | Envelope follower outputs (MIDI CC) |

### Block Diagram
```mermaid
graph TB
    subgraph Input["Audio & Controls"]
        AINL["🟢 Audio In L"]
        AINR["🟢 Audio In R<br/>Sidechain"]
        K1["🟢 Knob 1<br/>Ch1 Attack"]
        K2["🟢 Knob 2<br/>Ch1 Release"]
        K3["🟢 Knob 3<br/>Ch2 Attack"]
        K4["🟢 Knob 4<br/>Ch2 Release"]
        K5["🟢 Knob 5<br/>Ch3 Attack"]
        K6["🟢 Knob 6<br/>Ch3 Release"]
        K7["🟢 Knob 7<br/>Ch4 Attack"]
        K8["🟢 Knob 8<br/>Ch4 Release"]
        TOG["🟢 Toggle<br/>SC Mode"]
    end

    subgraph EnvFollow["Envelopes - Envelope Followers"]
        EF1["🟠 Env Follower 1<br/>RMS Detection"]
        EF2["🟠 Env Follower 2<br/>RMS Detection"]
        EF3["🟠 Env Follower 3<br/>RMS Detection"]
        EF4["🟠 Env Follower 4<br/>RMS Detection"]
    end

    subgraph VCAs["Envelopes - VCAs"]
        VCA1["🟠 VCA 1"]
        VCA2["🟠 VCA 2"]
        VCA3["🟠 VCA 3"]
        VCA4["🟠 VCA 4"]
    end

    subgraph Utils["Utilities"]
        SCROUTER["🟣 Sidechain Router"]
        MIX["🟣 4-Channel Mixer"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 Audio Out<br/>Stereo"]
        MIDI_OUT["🟢 MIDI Out<br/>Env CC 1-4"]
    end

    AINL --> SCROUTER
    AINR --> SCROUTER
    TOG --> SCROUTER
    SCROUTER --> EF1
    SCROUTER --> EF2
    SCROUTER --> EF3
    SCROUTER --> EF4
    K1 --> EF1
    K2 --> EF1
    K3 --> EF2
    K4 --> EF2
    K5 --> EF3
    K6 --> EF3
    K7 --> EF4
    K8 --> EF4
    AINL --> VCA1
    AINL --> VCA2
    AINR --> VCA3
    AINR --> VCA4
    EF1 --> VCA1
    EF2 --> VCA2
    EF3 --> VCA3
    EF4 --> VCA4
    VCA1 --> MIX
    VCA2 --> MIX
    VCA3 --> MIX
    VCA4 --> MIX
    MIX --> AOUT
    EF1 --> MIDI_OUT
    EF2 --> MIDI_OUT
    EF3 --> MIDI_OUT
    EF4 --> MIDI_OUT

    style AINL fill:#90EE90
    style AINR fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style TOG fill:#90EE90
    style EF1 fill:#FFA500
    style EF2 fill:#FFA500
    style EF3 fill:#FFA500
    style EF4 fill:#FFA500
    style VCA1 fill:#FFA500
    style VCA2 fill:#FFA500
    style VCA3 fill:#FFA500
    style VCA4 fill:#FFA500
    style SCROUTER fill:#DDA0DD
    style MIX fill:#DDA0DD
    style AOUT fill:#90EE90
    style MIDI_OUT fill:#90EE90
```

---

## Project 15: Hybrid Analog/Digital Synthesizer Voice (Complexity: 10/10)

### Description
Full-featured synthesizer voice combining digital oscillators with analog-style filters, dual LFOs, dual envelopes, modulation matrix, and extensive MIDI integration. Professional-level complexity.

### Controls Mapping
| Control | Function |
|---------|----------|
| Keyboard | Note input (mono priority) |
| Knob 1 | Osc 1 waveform + PWM |
| Knob 2 | Osc 2 waveform + detune |
| Knob 3 | Oscillator mix |
| Knob 4 | Filter cutoff |
| Knob 5 | Filter resonance |
| Knob 6 | Envelope 1 amount (filter) |
| Knob 7 | LFO 1 rate |
| Knob 8 | Master output level |
| MIDI In | Note + velocity + CC control |
| MIDI CC 1 | Pitch modulation |
| MIDI CC 2 | Filter cutoff modulation |
| MIDI CC 3 | PWM modulation |
| MIDI CC 74 | Filter brightness |
| Toggle | Modulation routing mode |
| Button 1 | Retrigger envelopes |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        KB["🟢 Keyboard<br/>Mono"]
        MIDI["🟢 MIDI In"]
        K1["🟢 Knob 1<br/>Osc1 Wave"]
        K2["🟢 Knob 2<br/>Osc2 Det"]
        K3["🟢 Knob 3<br/>Mix"]
        K4["🟢 Knob 4<br/>Cutoff"]
        K5["🟢 Knob 5<br/>Res"]
        K6["🟢 Knob 6<br/>Env Amt"]
        K7["🟢 Knob 7<br/>LFO Rate"]
        K8["🟢 Knob 8<br/>Level"]
        CC1["🟢 MIDI CC 1<br/>Pitch Mod"]
        CC2["🟢 MIDI CC 2<br/>Cut Mod"]
        CC3["🟢 MIDI CC 3<br/>PWM Mod"]
        CC74["🟢 MIDI CC 74<br/>Brightness"]
        TOG["🟢 Toggle<br/>Mod Route"]
        B1["🟢 Button 1<br/>Retrig"]
    end

    subgraph PitchProc["Utilities - Pitch"]
        PITCHSUM["🟣 Pitch Summer"]
        GLIDE["🟣 Portamento"]
    end

    subgraph Oscillators["Audio Sources"]
        OSC1["🔵 Digital Osc 1<br/>Multi-wave + PWM"]
        OSC2["🔵 Digital Osc 2<br/>Multi-wave + Sync"]
        NOISE["🔵 Noise Generator<br/>White/Pink"]
    end

    subgraph Modulation["Modulation Sources"]
        LFO1["🟠 LFO 1<br/>Multi-shape"]
        LFO2["🟠 LFO 2<br/>Multi-shape"]
        ENV1["🟠 ADSR 1<br/>Filter Env"]
        ENV2["🟠 ADSR 2<br/>Amp Env"]
    end

    subgraph ModMatrix["Utilities - Mod Matrix"]
        MATRIX["🟣 Modulation Matrix<br/>4x8 routing"]
        PWMSUM["🟣 PWM Summer"]
        CUTSUM["🟣 Cutoff Summer"]
    end

    subgraph Mixer["Utilities - Mixing"]
        OSCMIX["🟣 Oscillator Mixer<br/>3-channel"]
    end

    subgraph Filter["Audio Effects - Filter"]
        FILT["🔵 SVF Filter<br/>4-pole ladder"]
    end

    subgraph VCA["Envelopes - VCA"]
        VCA["🟠 VCA<br/>Exponential"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 Audio Out<br/>Stereo"]
    end

    KB --> PITCHSUM
    MIDI --> PITCHSUM
    CC1 --> PITCHSUM
    PITCHSUM --> GLIDE
    GLIDE --> OSC1
    GLIDE --> OSC2
    K1 --> OSC1
    K2 --> OSC2
    CC3 --> PWMSUM
    K1 --> PWMSUM
    K7 --> LFO1
    LFO1 --> MATRIX
    LFO2 --> MATRIX
    ENV1 --> MATRIX
    TOG --> MATRIX
    MATRIX --> PWMSUM
    PWMSUM --> OSC1
    OSC1 --> OSCMIX
    OSC2 --> OSCMIX
    NOISE --> OSCMIX
    K3 --> OSCMIX
    OSCMIX --> FILT
    K4 --> CUTSUM
    CC2 --> CUTSUM
    CC74 --> CUTSUM
    K6 --> CUTSUM
    ENV1 --> CUTSUM
    MATRIX --> CUTSUM
    CUTSUM --> FILT
    K5 --> FILT
    KB --> ENV1
    MIDI --> ENV1
    B1 --> ENV1
    KB --> ENV2
    MIDI --> ENV2
    B1 --> ENV2
    FILT --> VCA
    ENV2 --> VCA
    VCA --> AOUT
    K8 --> AOUT

    style KB fill:#90EE90
    style MIDI fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style K3 fill:#90EE90
    style K4 fill:#90EE90
    style K5 fill:#90EE90
    style K6 fill:#90EE90
    style K7 fill:#90EE90
    style K8 fill:#90EE90
    style CC1 fill:#90EE90
    style CC2 fill:#90EE90
    style CC3 fill:#90EE90
    style CC74 fill:#90EE90
    style TOG fill:#90EE90
    style B1 fill:#90EE90
    style PITCHSUM fill:#DDA0DD
    style GLIDE fill:#DDA0DD
    style OSC1 fill:#87CEEB
    style OSC2 fill:#87CEEB
    style NOISE fill:#87CEEB
    style LFO1 fill:#FFA500
    style LFO2 fill:#FFA500
    style ENV1 fill:#FFA500
    style ENV2 fill:#FFA500
    style MATRIX fill:#DDA0DD
    style PWMSUM fill:#DDA0DD
    style CUTSUM fill:#DDA0DD
    style OSCMIX fill:#DDA0DD
    style FILT fill:#87CEEB
    style VCA fill:#FFA500
    style AOUT fill:#90EE90
```

---

*End of Daisy Field Project Ideas*
*All projects revised to use MIDI CC, buttons, toggle, and internal modulation instead of CV/Gate*
