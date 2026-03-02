# Daisy Pod Project Ideas
*Generated from literature research - Complexity 5-10/10*

---

## Project 1: Dual LFO Modulated Delay (Complexity: 5/10)

### Description
Stereo delay effect with two independent LFOs modulating delay time and feedback, creating lush modulated delay textures.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Delay time |
| Knob 2 | Feedback amount |
| Encoder Rotate | LFO 1 rate |
| Encoder Press | Toggle between LFO shapes |
| Button 1 | Tap tempo |
| Button 2 | Freeze delay buffer |
| LED 1 | Tempo indicator (blinks) |
| LED 2 | Freeze status |
| Audio In | Stereo input |
| Audio Out | Stereo delayed output |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT<br/>Stereo"]
        K1["🟢 KNOB 1<br/>Delay Time"]
        K2["🟢 KNOB 2<br/>Feedback"]
        ENC["🟢 ENCODER<br/>LFO Rate"]
        ENCBTN["🟢 ENCODER PRESS<br/>LFO Shape"]
        B1["🟢 BUTTON 1<br/>Tap Tempo"]
        B2["🟢 BUTTON 2<br/>Freeze"]
    end

    subgraph Modulation["Modulation - Orange"]
        LFO1["🟠 LFO<br/>Time Mod"]
        LFO2["🟠 LFO<br/>Feedback Mod"]
    end

    subgraph Utils["Utilities - Violet"]
        SUM1["🟣 ADD<br/>Time CV Sum"]
        SUM2["🟣 ADD<br/>FB CV Sum"]
        TAP["🟣 Tap Tempo<br/>Calculator"]
    end

    subgraph Effects["Audio Effects - Blue"]
        DELL["🔵 DELAY<br/>Left Channel"]
        DELR["🔵 DELAY<br/>Right Channel"]
    end

    subgraph Output["Outputs - Green"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Tempo Blink"]
        LED2["🟢 LED 2<br/>Freeze"]
    end

    AIN --> DELL
    AIN --> DELR
    K1 --> SUM1
    B1 --> TAP
    TAP --> SUM1
    ENC --> LFO1
    ENCBTN --> LFO1
    LFO1 --> SUM1
    LFO2 --> SUM2
    SUM1 --> DELL
    SUM1 --> DELR
    K2 --> SUM2
    SUM2 --> DELL
    SUM2 --> DELR
    B2 --> DELL
    B2 --> DELR
    DELL --> AOUT
    DELR --> AOUT
    TAP --> LED1
    B2 --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style LFO1 fill:#FFA500
    style LFO2 fill:#FFA500
    style SUM1 fill:#DDA0DD
    style SUM2 fill:#DDA0DD
    style TAP fill:#DDA0DD
    style DELL fill:#87CEEB
    style DELR fill:#87CEEB
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 2: Generative Euclidean Melody Box (Complexity: 6/10)

### Description
Generative music box using Euclidean rhythm patterns to trigger melodic sequences. Inspired by "Introduction to Digital Music with Python Programming."

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Tempo (BPM) |
| Knob 2 | Sequence density |
| Encoder Rotate | Select scale (Major, Minor, Pentatonic, etc.) |
| Encoder Press | Regenerate melody |
| Button 1 | Start/Stop |
| Button 2 | Change Euclidean pattern |
| LED 1 | Beat indicator |
| LED 2 | Pattern indicator (color = pattern) |
| MIDI Out | Generated melody output |

### Block Diagram
```mermaid
graph TB
    subgraph Controls["User Interface - Green"]
        K1["🟢 KNOB 1<br/>Tempo"]
        K2["🟢 KNOB 2<br/>Density"]
        ENC["🟢 ENCODER<br/>Scale Select"]
        ENCBTN["🟢 ENCODER PRESS<br/>Regenerate"]
        B1["🟢 BUTTON 1<br/>Start/Stop"]
        B2["🟢 BUTTON 2<br/>Pattern"]
    end

    subgraph Sequencing["Utilities - Sequencing"]
        CLK["🟣 Master Clock"]
        EUC["🟣 Euclidean<br/>Pattern Gen"]
        SCALE["🟣 Scale Quantizer<br/>Mode Select"]
        RNG["🟣 Random<br/>Pitch Gen"]
        SEQ["🟣 Note Buffer<br/>8 steps"]
    end

    subgraph Modulation["Envelopes - Orange"]
        GATE["🟠 Gate Generator"]
    end

    subgraph Synthesis["Audio Sources - Blue"]
        OSC["🔵 OSCILLATOR<br/>Sine/Tri"]
    end

    subgraph Output["Outputs - Green"]
        MIDI["🟢 MIDI NOTE"]
        AOUT["🟢 AUDIO OUTPUT<br/>Mono"]
        LED1["🟢 LED 1<br/>Beat"]
        LED2["🟢 LED 2<br/>Pattern"]
    end

    K1 --> CLK
    B1 --> CLK
    CLK --> EUC
    K2 --> EUC
    B2 --> EUC
    ENCBTN --> RNG
    RNG --> SEQ
    EUC --> SEQ
    SEQ --> SCALE
    ENC --> SCALE
    SCALE --> OSC
    SCALE --> MIDI
    EUC --> GATE
    GATE --> OSC
    GATE --> MIDI
    OSC --> AOUT
    CLK --> LED1
    EUC --> LED2

    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style CLK fill:#DDA0DD
    style EUC fill:#DDA0DD
    style SCALE fill:#DDA0DD
    style RNG fill:#DDA0DD
    style SEQ fill:#DDA0DD
    style GATE fill:#FFA500
    style OSC fill:#87CEEB
    style MIDI fill:#90EE90
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 3: Stereo Wavefolder and Filter (Complexity: 5/10)

### Description
Parallel wavefolder effect with state-variable filtering. Creates aggressive harmonic distortion with tonal control.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Fold amount/drive |
| Knob 2 | Filter cutoff |
| Encoder Rotate | Filter resonance |
| Encoder Press | Cycle filter mode (LP/BP/HP/Notch) |
| Button 1 | Bypass wavefolder |
| Button 2 | Bypass filter |
| LED 1 | Wavefolder active |
| LED 2 | Filter mode (color coded) |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT"]
        K1["🟢 KNOB 1<br/>Fold Drive"]
        K2["🟢 KNOB 2<br/>Cutoff"]
        ENC["🟢 ENCODER<br/>Resonance"]
        ENCBTN["🟢 ENCODER PRESS<br/>Filter Mode"]
        B1["🟢 BUTTON 1<br/>Bypass Fold"]
        B2["🟢 BUTTON 2<br/>Bypass Filt"]
    end

    subgraph Effects["Audio Effects"]
        FOLD["🔵 WAVEFOLDER"]
        SVF["🔵 SVF<br/>Multi-output"]
    end

    subgraph Utils["Utilities"]
        BYP1["🟣 BYPASS<br/>Fold"]
        BYP2["🟣 BYPASS<br/>Filter"]
        MODESEL["🟣 Mode Selector<br/>4-to-1 Mux"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Fold Status"]
        LED2["🟢 LED 2<br/>Filter Mode"]
    end

    AIN --> FOLD
    K1 --> FOLD
    FOLD --> BYP1
    AIN --> BYP1
    B1 --> BYP1
    BYP1 --> SVF
    K2 --> SVF
    ENC --> SVF
    SVF --> MODESEL
    ENCBTN --> MODESEL
    MODESEL --> BYP2
    BYP1 --> BYP2
    B2 --> BYP2
    BYP2 --> AOUT
    B1 --> LED1
    ENCBTN --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style FOLD fill:#87CEEB
    style SVF fill:#87CEEB
    style BYP1 fill:#DDA0DD
    style BYP2 fill:#DDA0DD
    style MODESEL fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 4: Sample and Hold Noise Synthesizer (Complexity: 5/10)

### Description
Random stepped voltage synthesizer using sample and hold on white noise, creating retro computer game sounds and random melodies.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Clock rate (sample rate) |
| Knob 2 | Filter cutoff |
| Encoder Rotate | Quantization (steps) |
| Encoder Press | Toggle quantization on/off |
| Button 1 | External clock mode |
| Button 2 | Hold current value |
| LED 1 | Clock indicator |
| LED 2 | Quantization on/off |
| MIDI In | External clock input |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        K1["🟢 KNOB 1<br/>Clock Rate"]
        K2["🟢 KNOB 2<br/>Filter"]
        ENC["🟢 ENCODER<br/>Quantize Steps"]
        ENCBTN["🟢 ENCODER PRESS<br/>Quant On/Off"]
        B1["🟢 BUTTON 1<br/>Ext Clock"]
        B2["🟢 BUTTON 2<br/>Hold"]
        MIDI["🟢 MIDI IN<br/>Ext Clock"]
    end

    subgraph Sources["Audio Sources"]
        NOISE["🔵 WHITE NOISE"]
    end

    subgraph Clocking["Utilities"]
        CLK["🟣 Clock Generator"]
        CLKMUX["🟣 Clock Mux<br/>Int/Ext"]
        SH["🟣 Sample & Hold"]
        QUANT["🟣 Quantizer<br/>Voltage Steps"]
    end

    subgraph Synthesis["Audio Sources & Effects"]
        OSC["🔵 OSCILLATOR<br/>Pitch Tracking"]
        FILT["🔵 LPF (1-POLE)"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Clock"]
        LED2["🟢 LED 2<br/>Quant"]
    end

    K1 --> CLK
    CLK --> CLKMUX
    MIDI --> CLKMUX
    B1 --> CLKMUX
    CLKMUX --> SH
    NOISE --> SH
    B2 --> SH
    SH --> QUANT
    ENC --> QUANT
    ENCBTN --> QUANT
    QUANT --> OSC
    OSC --> FILT
    K2 --> FILT
    FILT --> AOUT
    CLKMUX --> LED1
    ENCBTN --> LED2

    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style MIDI fill:#90EE90
    style NOISE fill:#87CEEB
    style CLK fill:#DDA0DD
    style CLKMUX fill:#DDA0DD
    style SH fill:#DDA0DD
    style QUANT fill:#DDA0DD
    style OSC fill:#87CEEB
    style FILT fill:#87CEEB
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 5: Dual Oscillator FM Synth Voice (Complexity: 6/10)

### Description
Classic 2-operator FM synthesis voice with ADSR envelope. Simple but powerful synthesis engine.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | FM index (modulation amount) |
| Knob 2 | FM ratio (frequency ratio) |
| Encoder Rotate | Filter cutoff |
| Encoder Press | Cycle waveforms (sine/tri/saw/square) |
| Button 1 | Retrigger envelope |
| Button 2 | Hold note |
| LED 1 | Envelope status |
| LED 2 | Waveform indicator |
| MIDI In | Note + velocity input |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        MIDI["🟢 MIDI NOTE"]
        K1["🟢 KNOB 1<br/>FM Index"]
        K2["🟢 KNOB 2<br/>Ratio"]
        ENC["🟢 ENCODER<br/>Filter"]
        ENCBTN["🟢 ENCODER PRESS<br/>Waveform"]
        B1["🟢 BUTTON 1<br/>Retrigger"]
        B2["🟢 BUTTON 2<br/>Hold"]
    end

    subgraph Synthesis["Audio Sources"]
        FM["🔵 FM2<br/>2-Op FM"]
    end

    subgraph Modulation["Envelopes & Modulation"]
        ENV["🟠 ADSR ENVELOPE"]
        VCA["🟠 LINEAR VCA"]
    end

    subgraph Filter["Audio Effects"]
        FILT["🔵 MOOG LADDER"]
    end

    subgraph Utils["Utilities"]
        HOLD["🟣 Sample & Hold<br/>Note Hold"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Env Active"]
        LED2["🟢 LED 2<br/>Waveform"]
    end

    MIDI --> HOLD
    B2 --> HOLD
    HOLD --> FM
    K2 --> FM
    K1 --> FM
    MIDI --> ENV
    B1 --> ENV
    FM --> VCA
    ENV --> VCA
    VCA --> FILT
    ENC --> FILT
    FILT --> AOUT
    ENV --> LED1
    ENCBTN --> LED2

    style MIDI fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style FM fill:#87CEEB
    style ENV fill:#FFA500
    style VCA fill:#FFA500
    style FILT fill:#87CEEB
    style HOLD fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 6: Granular Looper (Complexity: 7/10)

### Description
Real-time granular synthesis looper that captures and processes audio into grains with variable size, density, and pitch. Inspired by granular synthesis techniques from the literature.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Grain size |
| Knob 2 | Grain density/playback rate |
| Encoder Rotate | Loop position/scrubbing |
| Encoder Press | Record/Overdub toggle |
| Button 1 | Clear loop buffer |
| Button 2 | Freeze grains |
| LED 1 | Recording status |
| LED 2 | Loop position indicator |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT"]
        K1["🟢 KNOB 1<br/>Grain Size"]
        K2["🟢 KNOB 2<br/>Density"]
        ENC["🟢 ENCODER<br/>Position"]
        ENCBTN["🟢 ENCODER PRESS<br/>Rec/OD"]
        B1["🟢 BUTTON 1<br/>Clear"]
        B2["🟢 BUTTON 2<br/>Freeze"]
    end

    subgraph Storage["Utilities - Buffer"]
        LOOPBUF["🟣 Circular Buffer<br/>Loop Storage"]
        GRAINBUF["🟣 Grain Buffer<br/>Temp Storage"]
    end

    subgraph Granular["Audio Effects - Granular"]
        GRAN["🔵 Granular Engine<br/>Multi-grain"]
        PITCH["🔵 PV PITCH<br/>Per-grain"]
    end

    subgraph Envelope["Envelopes"]
        GENV["🟠 Grain Envelope<br/>Windowing"]
    end

    subgraph Utils["Utilities"]
        POSCTL["🟣 Position Control<br/>Scrubbing"]
        DENSCTL["🟣 Density Control<br/>Grain Trig"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Rec Status"]
        LED2["🟢 LED 2<br/>Position"]
    end

    AIN --> LOOPBUF
    ENCBTN --> LOOPBUF
    B1 --> LOOPBUF
    LOOPBUF --> GRAINBUF
    ENC --> POSCTL
    POSCTL --> GRAINBUF
    K2 --> DENSCTL
    DENSCTL --> GRAINBUF
    K1 --> GENV
    GRAINBUF --> GRAN
    GENV --> GRAN
    GRAN --> PITCH
    B2 --> PITCH
    PITCH --> AOUT
    ENCBTN --> LED1
    POSCTL --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style LOOPBUF fill:#DDA0DD
    style GRAINBUF fill:#DDA0DD
    style GRAN fill:#87CEEB
    style PITCH fill:#87CEEB
    style GENV fill:#FFA500
    style POSCTL fill:#DDA0DD
    style DENSCTL fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 7: Chaotic Modulation Source (Complexity: 6/10)

### Description
Dual chaotic LFO system with cross-modulation creating unpredictable but musical modulation patterns. Great for experimental sound design.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Chaos amount/feedback |
| Knob 2 | Cross-modulation amount |
| Encoder Rotate | Master rate |
| Encoder Press | Reset chaos state |
| Button 1 | Sample chaos to CV out |
| Button 2 | Sync both LFOs |
| LED 1 | LFO 1 activity (brightness) |
| LED 2 | LFO 2 activity (brightness) |
| Audio Out | Chaotic audio rate oscillation |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        K1["🟢 KNOB 1<br/>Chaos Amt"]
        K2["🟢 KNOB 2<br/>Cross Mod"]
        ENC["🟢 ENCODER<br/>Rate"]
        ENCBTN["🟢 ENCODER PRESS<br/>Reset"]
        B1["🟢 BUTTON 1<br/>Sample"]
        B2["🟢 BUTTON 2<br/>Sync"]
    end

    subgraph Modulation["Modulation Sources"]
        LFO1["🟠 LFO<br/>Chaotic 1"]
        LFO2["🟠 LFO<br/>Chaotic 2"]
    end

    subgraph Utils["Utilities - Processing"]
        CHAOS1["🟣 Chaos Function<br/>Feedback Loop"]
        CHAOS2["🟣 Chaos Function<br/>Feedback Loop"]
        XMOD["🟣 Cross Modulator<br/>Multiplier"]
        SH["🟣 Sample & Hold"]
    end

    subgraph Synthesis["Audio Sources"]
        OSC["🔵 OSCILLATOR<br/>Audio Rate"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Chaotic Wave"]
        LED1["🟢 LED 1<br/>LFO1 Level"]
        LED2["🟢 LED 2<br/>LFO2 Level"]
    end

    ENC --> LFO1
    ENC --> LFO2
    B2 --> LFO1
    B2 --> LFO2
    LFO1 --> CHAOS1
    K1 --> CHAOS1
    CHAOS1 --> LFO1
    LFO2 --> CHAOS2
    K1 --> CHAOS2
    CHAOS2 --> LFO2
    LFO1 --> XMOD
    LFO2 --> XMOD
    K2 --> XMOD
    XMOD --> LFO1
    XMOD --> LFO2
    ENCBTN --> LFO1
    ENCBTN --> LFO2
    LFO1 --> SH
    B1 --> SH
    LFO1 --> OSC
    LFO2 --> OSC
    OSC --> AOUT
    LFO1 --> LED1
    LFO2 --> LED2

    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style LFO1 fill:#FFA500
    style LFO2 fill:#FFA500
    style CHAOS1 fill:#DDA0DD
    style CHAOS2 fill:#DDA0DD
    style XMOD fill:#DDA0DD
    style SH fill:#DDA0DD
    style OSC fill:#87CEEB
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 8: Chord Memory and Strum (Complexity: 7/10)

### Description
Chord memory bank with strumming patterns. Records MIDI chords and plays them back with adjustable strum timing. Based on chord machine concepts.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Strum speed |
| Knob 2 | Chord inversion |
| Encoder Rotate | Select chord bank (1-8) |
| Encoder Press | Record current chord |
| Button 1 | Trigger strum up |
| Button 2 | Trigger strum down |
| LED 1 | Chord bank indicator (color) |
| LED 2 | Strum direction |
| MIDI In | Chord input |
| MIDI Out | Strummed notes |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        MIDI_IN["🟢 MIDI IN"]
        K1["🟢 KNOB 1<br/>Strum Speed"]
        K2["🟢 KNOB 2<br/>Inversion"]
        ENC["🟢 ENCODER<br/>Bank Select"]
        ENCBTN["🟢 ENCODER PRESS<br/>Record"]
        B1["🟢 BUTTON 1<br/>Strum Up"]
        B2["🟢 BUTTON 2<br/>Strum Down"]
    end

    subgraph Memory["Utilities - Storage"]
        BANK["🟣 Chord Bank<br/>8 slots"]
        INVERT["🟣 Chord Inverter<br/>Voicing"]
    end

    subgraph Sequencing["Utilities - Sequencing"]
        STRUM["🟣 Strum Engine<br/>Note Delay"]
        SORT["🟣 Note Sorter<br/>Pitch Order"]
    end

    subgraph Modulation["Envelopes"]
        GATE["🟠 Gate Generator<br/>Per Note"]
    end

    subgraph Synthesis["Audio Sources"]
        OSC1["🔵 OSCILLATOR 1"]
        OSC2["🔵 OSCILLATOR 2"]
        OSC3["🔵 OSCILLATOR 3"]
        OSC4["🔵 OSCILLATOR 4"]
    end

    subgraph Utils["Utilities"]
        MIX["🟣 4-Channel Mixer"]
    end

    subgraph Output["Outputs"]
        MIDI_OUT["🟢 MIDI OUT"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Bank"]
        LED2["🟢 LED 2<br/>Direction"]
    end

    MIDI_IN --> BANK
    ENCBTN --> BANK
    ENC --> BANK
    BANK --> INVERT
    K2 --> INVERT
    INVERT --> SORT
    B1 --> SORT
    B2 --> SORT
    SORT --> STRUM
    K1 --> STRUM
    B1 --> STRUM
    B2 --> STRUM
    STRUM --> GATE
    GATE --> OSC1
    GATE --> OSC2
    GATE --> OSC3
    GATE --> OSC4
    STRUM --> OSC1
    STRUM --> OSC2
    STRUM --> OSC3
    STRUM --> OSC4
    OSC1 --> MIX
    OSC2 --> MIX
    OSC3 --> MIX
    OSC4 --> MIX
    MIX --> AOUT
    STRUM --> MIDI_OUT
    GATE --> MIDI_OUT
    ENC --> LED1
    B1 --> LED2
    B2 --> LED2

    style MIDI_IN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style BANK fill:#DDA0DD
    style INVERT fill:#DDA0DD
    style STRUM fill:#DDA0DD
    style SORT fill:#DDA0DD
    style GATE fill:#FFA500
    style OSC1 fill:#87CEEB
    style OSC2 fill:#87CEEB
    style OSC3 fill:#87CEEB
    style OSC4 fill:#87CEEB
    style MIX fill:#DDA0DD
    style MIDI_OUT fill:#90EE90
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 9: Probability Gate Sequencer (Complexity: 8/10)

### Description
8-step gate sequencer with per-step probability and pattern chaining. Creates evolving rhythmic patterns. Inspired by modern Eurorack sequencers.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Master tempo |
| Knob 2 | Global probability bias |
| Encoder Rotate | Select step (1-8) |
| Encoder Press | Toggle step on/off |
| Button 1 | Set step probability |
| Button 2 | Reset sequence |
| LED 1 | Current step (blinks) |
| LED 2 | Probability indicator (brightness) |
| MIDI In | External clock |
| MIDI Out | Gate output |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        K1["🟢 KNOB 1<br/>Tempo"]
        K2["🟢 KNOB 2<br/>Prob Bias"]
        ENC["🟢 ENCODER<br/>Step Select"]
        ENCBTN["🟢 ENCODER PRESS<br/>Toggle Step"]
        B1["🟢 BUTTON 1<br/>Set Prob"]
        B2["🟢 BUTTON 2<br/>Reset"]
        MIDI_IN["🟢 MIDI IN<br/>Ext Clock"]
    end

    subgraph Sequencing["Utilities - Sequencing"]
        CLK["🟣 Clock Gen<br/>Internal"]
        CLKMUX["🟣 Clock Mux<br/>Int/Ext"]
        SEQ["🟣 Step Counter<br/>8 steps"]
        STEPMEM["🟣 Step Memory<br/>8x2 params"]
    end

    subgraph Probability["Utilities - Probability"]
        RNG["🟣 Random Gen<br/>Per-step"]
        PROB["🟣 Probability Gate<br/>Comparator"]
    end

    subgraph Modulation["Envelopes"]
        GATE["🟠 Gate Generator"]
    end

    subgraph Synthesis["Audio Sources"]
        KICK["🔵 ANALOG KICK"]
    end

    subgraph Output["Outputs"]
        MIDI_OUT["🟢 MIDI OUT<br/>Gate"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Step"]
        LED2["🟢 LED 2<br/>Probability"]
    end

    K1 --> CLK
    CLK --> CLKMUX
    MIDI_IN --> CLKMUX
    CLKMUX --> SEQ
    B2 --> SEQ
    SEQ --> STEPMEM
    ENC --> STEPMEM
    ENCBTN --> STEPMEM
    B1 --> STEPMEM
    STEPMEM --> PROB
    K2 --> PROB
    SEQ --> RNG
    RNG --> PROB
    PROB --> GATE
    GATE --> KICK
    GATE --> MIDI_OUT
    KICK --> AOUT
    SEQ --> LED1
    PROB --> LED2

    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style MIDI_IN fill:#90EE90
    style CLK fill:#DDA0DD
    style CLKMUX fill:#DDA0DD
    style SEQ fill:#DDA0DD
    style STEPMEM fill:#DDA0DD
    style RNG fill:#DDA0DD
    style PROB fill:#DDA0DD
    style GATE fill:#FFA500
    style KICK fill:#87CEEB
    style MIDI_OUT fill:#90EE90
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 10: Spectral Delay (Complexity: 8/10)

### Description
FFT-based spectral delay that can delay different frequency bands independently, creating unique rhythmic textures.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Spectral shift (rotate bands) |
| Knob 2 | Feedback amount |
| Encoder Rotate | Band delay spread |
| Encoder Press | Freeze spectrum |
| Button 1 | Clear delay buffer |
| Button 2 | Toggle stereo/mono |
| LED 1 | Freeze status |
| LED 2 | Processing indicator |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT"]
        K1["🟢 KNOB 1<br/>Spectral Shift"]
        K2["🟢 KNOB 2<br/>Feedback"]
        ENC["🟢 ENCODER<br/>Band Spread"]
        ENCBTN["🟢 ENCODER PRESS<br/>Freeze"]
        B1["🟢 BUTTON 1<br/>Clear"]
        B2["🟢 BUTTON 2<br/>St/Mono"]
    end

    subgraph Analysis["Audio Effects - FFT"]
        FFT["🔵 FFT Analysis<br/>1024 bins"]
    end

    subgraph Processing["Utilities - Spectral"]
        SHIFT["🟣 Spectral Shifter<br/>Band Rotation"]
        BANDCTL["🟣 Band Controller<br/>8 bands"]
        DELBUF["🟣 Delay Buffers<br/>Per-band"]
        FBCTL["🟣 Feedback Loop"]
    end

    subgraph Synthesis["Audio Effects - Resynthesis"]
        IFFT["🔵 IFFT Synthesis<br/>Overlap-add"]
    end

    subgraph Utils["Utilities"]
        STMONO["🟣 Stereo/Mono<br/>Switch"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Freeze"]
        LED2["🟢 LED 2<br/>Active"]
    end

    AIN --> FFT
    FFT --> SHIFT
    K1 --> SHIFT
    SHIFT --> BANDCTL
    ENC --> BANDCTL
    BANDCTL --> DELBUF
    ENCBTN --> DELBUF
    B1 --> DELBUF
    DELBUF --> FBCTL
    K2 --> FBCTL
    FBCTL --> DELBUF
    FBCTL --> IFFT
    IFFT --> STMONO
    B2 --> STMONO
    STMONO --> AOUT
    ENCBTN --> LED1
    FFT --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style FFT fill:#87CEEB
    style SHIFT fill:#DDA0DD
    style BANDCTL fill:#DDA0DD
    style DELBUF fill:#DDA0DD
    style FBCTL fill:#DDA0DD
    style IFFT fill:#87CEEB
    style STMONO fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 11: Resonant Filter Bank (Complexity: 7/10)

### Description
8-band resonant filter bank for spectral processing, creating formant-like sounds and vowel filtering effects.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Filter spread (spacing) |
| Knob 2 | Global resonance |
| Encoder Rotate | Center frequency |
| Encoder Press | Cycle filter modes |
| Button 1 | Odd harmonics only |
| Button 2 | Even harmonics only |
| LED 1 | Odd/even indicator |
| LED 2 | Mode indicator |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT"]
        K1["🟢 KNOB 1<br/>Spread"]
        K2["🟢 KNOB 2<br/>Resonance"]
        ENC["🟢 ENCODER<br/>Center Freq"]
        ENCBTN["🟢 ENCODER PRESS<br/>Mode"]
        B1["🟢 BUTTON 1<br/>Odd Harm"]
        B2["🟢 BUTTON 2<br/>Even Harm"]
    end

    subgraph Filters["Audio Effects - Filter Bank"]
        FILT1["🔵 SVF 1"]
        FILT2["🔵 SVF 2"]
        FILT3["🔵 SVF 3"]
        FILT4["🔵 SVF 4"]
        FILT5["🔵 SVF 5"]
        FILT6["🔵 SVF 6"]
        FILT7["🔵 SVF 7"]
        FILT8["🔵 SVF 8"]
    end

    subgraph Utils["Utilities"]
        FREQGEN["🟣 Frequency Generator<br/>Harmonic Series"]
        MIXER["🟣 8-Channel Mixer"]
        HARMSEL["🟣 Harmonic Selector"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Harm Mode"]
        LED2["🟢 LED 2<br/>Filter Mode"]
    end

    AIN --> FILT1
    AIN --> FILT2
    AIN --> FILT3
    AIN --> FILT4
    AIN --> FILT5
    AIN --> FILT6
    AIN --> FILT7
    AIN --> FILT8
    ENC --> FREQGEN
    K1 --> FREQGEN
    B1 --> HARMSEL
    B2 --> HARMSEL
    HARMSEL --> FREQGEN
    FREQGEN --> FILT1
    FREQGEN --> FILT2
    FREQGEN --> FILT3
    FREQGEN --> FILT4
    FREQGEN --> FILT5
    FREQGEN --> FILT6
    FREQGEN --> FILT7
    FREQGEN --> FILT8
    K2 --> FILT1
    K2 --> FILT2
    K2 --> FILT3
    K2 --> FILT4
    K2 --> FILT5
    K2 --> FILT6
    K2 --> FILT7
    K2 --> FILT8
    ENCBTN --> FILT1
    ENCBTN --> FILT2
    ENCBTN --> FILT3
    ENCBTN --> FILT4
    FILT1 --> MIXER
    FILT2 --> MIXER
    FILT3 --> MIXER
    FILT4 --> MIXER
    FILT5 --> MIXER
    FILT6 --> MIXER
    FILT7 --> MIXER
    FILT8 --> MIXER
    MIXER --> AOUT
    HARMSEL --> LED1
    ENCBTN --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style FILT1 fill:#87CEEB
    style FILT2 fill:#87CEEB
    style FILT3 fill:#87CEEB
    style FILT4 fill:#87CEEB
    style FILT5 fill:#87CEEB
    style FILT6 fill:#87CEEB
    style FILT7 fill:#87CEEB
    style FILT8 fill:#87CEEB
    style FREQGEN fill:#DDA0DD
    style MIXER fill:#DDA0DD
    style HARMSEL fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 12: Rhythmic Gate Effect (Complexity: 6/10)

### Description
Tempo-synced rhythmic gate/tremolo effect with pattern memory. Creates rhythmic choppy effects.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Tempo (BPM) |
| Knob 2 | Gate width |
| Encoder Rotate | Pattern select (16 patterns) |
| Encoder Press | Record custom pattern |
| Button 1 | Tap tempo |
| Button 2 | Gate/Tremolo mode |
| LED 1 | Beat indicator |
| LED 2 | Mode (gate=red, trem=blue) |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT"]
        K1["🟢 KNOB 1<br/>Tempo"]
        K2["🟢 KNOB 2<br/>Gate Width"]
        ENC["🟢 ENCODER<br/>Pattern"]
        ENCBTN["🟢 ENCODER PRESS<br/>Record"]
        B1["🟢 BUTTON 1<br/>Tap Tempo"]
        B2["🟢 BUTTON 2<br/>Mode"]
    end

    subgraph Sequencing["Utilities - Sequencing"]
        CLK["🟣 Clock Generator"]
        TAP["🟣 Tap Tempo Calc"]
        PATMEM["🟣 Pattern Memory<br/>16 patterns x 16 steps"]
        SEQ["🟣 Step Sequencer"]
    end

    subgraph Modulation["Envelopes & Modulation"]
        GATE["🟠 Gate Generator"]
        TREM["🟠 Tremolo LFO"]
    end

    subgraph Processing["Utilities"]
        MODE["🟣 Mode Switch<br/>Gate/Trem"]
        VCA["🟣 LINEAR VCA"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Beat"]
        LED2["🟢 LED 2<br/>Mode"]
    end

    K1 --> CLK
    B1 --> TAP
    TAP --> CLK
    CLK --> SEQ
    ENC --> PATMEM
    ENCBTN --> PATMEM
    PATMEM --> SEQ
    SEQ --> GATE
    K2 --> GATE
    CLK --> TREM
    GATE --> MODE
    TREM --> MODE
    B2 --> MODE
    AIN --> VCA
    MODE --> VCA
    VCA --> AOUT
    CLK --> LED1
    B2 --> LED2

    style AIN fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style CLK fill:#DDA0DD
    style TAP fill:#DDA0DD
    style PATMEM fill:#DDA0DD
    style SEQ fill:#DDA0DD
    style GATE fill:#FFA500
    style TREM fill:#FFA500
    style MODE fill:#DDA0DD
    style VCA fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 13: Physical Modeling Percussion Synth (Complexity: 8/10)

### Description
Dual physical modeling percussion synthesizer using modal synthesis and resonators. Create bell, wood, and metal percussion sounds.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Material/brightness |
| Knob 2 | Damping/decay |
| Encoder Rotate | Structure/geometry |
| Encoder Press | Trigger both voices |
| Button 1 | Trigger voice 1 |
| Button 2 | Trigger voice 2 |
| LED 1 | Voice 1 envelope |
| LED 2 | Voice 2 envelope |
| MIDI In | Trigger notes + velocity |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        MIDI["🟢 MIDI NOTE"]
        K1["🟢 KNOB 1<br/>Brightness"]
        K2["🟢 KNOB 2<br/>Damping"]
        ENC["🟢 ENCODER<br/>Structure"]
        ENCBTN["🟢 ENCODER PRESS<br/>Trig Both"]
        B1["🟢 BUTTON 1<br/>Trig V1"]
        B2["🟢 BUTTON 2<br/>Trig V2"]
    end

    subgraph Excitation["Audio Sources"]
        NOISE1["🔵 WHITE NOISE<br/>Voice 1"]
        NOISE2["🔵 WHITE NOISE<br/>Voice 2"]
    end

    subgraph Modulation["Envelopes"]
        ENV1["🟠 AD ENVELOPE<br/>Voice 1"]
        ENV2["🟠 AD ENVELOPE<br/>Voice 2"]
    end

    subgraph Physical["Physical Modeling"]
        MODAL1["🔵 MODAL VOICE<br/>Voice 1"]
        MODAL2["🔵 MODAL VOICE<br/>Voice 2"]
    end

    subgraph Utils["Utilities"]
        MIX["🟣 Stereo Mixer<br/>Pan voices"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Env1"]
        LED2["🟢 LED 2<br/>Env2"]
    end

    MIDI --> ENV1
    B1 --> ENV1
    ENCBTN --> ENV1
    MIDI --> ENV2
    B2 --> ENV2
    ENCBTN --> ENV2
    ENV1 --> NOISE1
    ENV2 --> NOISE2
    NOISE1 --> MODAL1
    NOISE2 --> MODAL2
    MIDI --> MODAL1
    MIDI --> MODAL2
    K1 --> MODAL1
    K1 --> MODAL2
    K2 --> MODAL1
    K2 --> MODAL2
    ENC --> MODAL1
    ENC --> MODAL2
    MODAL1 --> MIX
    MODAL2 --> MIX
    MIX --> AOUT
    ENV1 --> LED1
    ENV2 --> LED2

    style MIDI fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style NOISE1 fill:#87CEEB
    style NOISE2 fill:#87CEEB
    style ENV1 fill:#FFA500
    style ENV2 fill:#FFA500
    style MODAL1 fill:#87CEEB
    style MODAL2 fill:#87CEEB
    style MIX fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 14: Adaptive Envelope Follower Effect (Complexity: 7/10)

### Description
Dynamics-responsive multi-effect that uses envelope following to control multiple parameters simultaneously. Great for reactive sound design.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | Envelope attack time |
| Knob 2 | Envelope release time |
| Encoder Rotate | Modulation depth |
| Encoder Press | Select target (filter/delay/reverb) |
| Button 1 | Invert envelope |
| Button 2 | Sidechain input mode |
| LED 1 | Envelope level (brightness) |
| LED 2 | Target indicator |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        AIN["🟢 AUDIO INPUT<br/>Main"]
        SC["🟢 AUDIO INPUT<br/>Sidechain"]
        K1["🟢 KNOB 1<br/>Attack"]
        K2["🟢 KNOB 2<br/>Release"]
        ENC["🟢 ENCODER<br/>Depth"]
        ENCBTN["🟢 ENCODER PRESS<br/>Target"]
        B1["🟢 BUTTON 1<br/>Invert"]
        B2["🟢 BUTTON 2<br/>SC Mode"]
    end

    subgraph EnvFollow["Envelope Following"]
        ENVF["🟠 ENV FOL<br/>Detector"]
    end

    subgraph Utils["Utilities - Modulation"]
        SCSEL["🟣 SC Selector<br/>Main/SC"]
        INVERT["🟣 Inverter<br/>-1 * x"]
        SCALE["🟣 Scaler<br/>Depth"]
        ROUTER["🟣 Mod Router<br/>To Targets"]
    end

    subgraph Effects["Audio Effects"]
        FILT["🔵 SVF<br/>Env Ctrl Cutoff"]
        DEL["🔵 DELAY<br/>Env Ctrl Time"]
        VERB["🔵 FDN REVERB<br/>Env Ctrl Mix"]
    end

    subgraph Mix["Utilities"]
        MIXER["🟣 FX Mixer"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT"]
        LED1["🟢 LED 1<br/>Env Level"]
        LED2["🟢 LED 2<br/>Target"]
    end

    AIN --> SCSEL
    SC --> SCSEL
    B2 --> SCSEL
    SCSEL --> ENVF
    K1 --> ENVF
    K2 --> ENVF
    ENVF --> INVERT
    B1 --> INVERT
    INVERT --> SCALE
    ENC --> SCALE
    SCALE --> ROUTER
    ENCBTN --> ROUTER
    AIN --> FILT
    ROUTER --> FILT
    AIN --> DEL
    ROUTER --> DEL
    AIN --> VERB
    ROUTER --> VERB
    FILT --> MIXER
    DEL --> MIXER
    VERB --> MIXER
    MIXER --> AOUT
    ENVF --> LED1
    ENCBTN --> LED2

    style AIN fill:#90EE90
    style SC fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style ENVF fill:#FFA500
    style SCSEL fill:#DDA0DD
    style INVERT fill:#DDA0DD
    style SCALE fill:#DDA0DD
    style ROUTER fill:#DDA0DD
    style FILT fill:#87CEEB
    style DEL fill:#87CEEB
    style VERB fill:#87CEEB
    style MIXER fill:#DDA0DD
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

## Project 15: Karplus-Strong String Synthesizer (Complexity: 9/10)

### Description
Advanced physical modeling string synthesizer using Karplus-Strong algorithm with damping, pickup position simulation, and sympathetic resonance. Professional-level string synthesis.

### Controls Mapping
| Control | Function |
|---------|----------|
| Knob 1 | String damping |
| Knob 2 | Pickup position |
| Encoder Rotate | String brightness/harmonics |
| Encoder Press | Trigger pluck |
| Button 1 | Sustain/damper pedal |
| Button 2 | Sympathetic resonance on/off |
| LED 1 | Note activity |
| LED 2 | Resonance status |
| MIDI In | Note input + velocity |

### Block Diagram
```mermaid
graph TB
    subgraph Input["User Interface"]
        MIDI["🟢 MIDI NOTE"]
        K1["🟢 KNOB 1<br/>Damping"]
        K2["🟢 KNOB 2<br/>Pickup Pos"]
        ENC["🟢 ENCODER<br/>Brightness"]
        ENCBTN["🟢 ENCODER PRESS<br/>Pluck"]
        B1["🟢 BUTTON 1<br/>Sustain"]
        B2["🟢 BUTTON 2<br/>Resonance"]
    end

    subgraph Excitation["Audio Sources"]
        NOISE["🔵 WHITE NOISE<br/>Pluck Exciter"]
        BURST["🔵 Impulse<br/>Sharp Attack"]
    end

    subgraph Envelope["Envelopes"]
        PLUCKENV["🟠 AD ENVELOPE<br/>Pluck Shape"]
    end

    subgraph Physical["Physical Modeling"]
        STRING1["🔵 STRING VOICE<br/>Main String"]
        STRING2["🔵 STRING VOICE<br/>Sympathetic"]
        DLINE["🔵 LP COMB<br/>Delay Line"]
    end

    subgraph Utils["Utilities - String Processing"]
        DAMPER["🟣 Damping Control<br/>Sustain Pedal"]
        PICKUP["🟣 Pickup Sim<br/>Position Filter"]
        COUPLE["🟣 String Coupler<br/>Resonance"]
    end

    subgraph Filter["Audio Effects"]
        BRIGHT["🔵 HIGH SHELF<br/>Brightness"]
        BODY["🔵 RESONATOR<br/>Body Resonance"]
    end

    subgraph Output["Outputs"]
        AOUT["🟢 AUDIO OUTPUT<br/>Stereo"]
        LED1["🟢 LED 1<br/>Active"]
        LED2["🟢 LED 2<br/>Resonance"]
    end

    MIDI --> PLUCKENV
    ENCBTN --> PLUCKENV
    PLUCKENV --> NOISE
    PLUCKENV --> BURST
    NOISE --> STRING1
    BURST --> STRING1
    MIDI --> STRING1
    K1 --> DAMPER
    B1 --> DAMPER
    DAMPER --> STRING1
    STRING1 --> DLINE
    DLINE --> STRING1
    STRING1 --> PICKUP
    K2 --> PICKUP
    PICKUP --> BRIGHT
    ENC --> BRIGHT
    STRING1 --> COUPLE
    B2 --> COUPLE
    COUPLE --> STRING2
    MIDI --> STRING2
    STRING2 --> COUPLE
    BRIGHT --> BODY
    COUPLE --> BODY
    BODY --> AOUT
    PLUCKENV --> LED1
    B2 --> LED2

    style MIDI fill:#90EE90
    style K1 fill:#90EE90
    style K2 fill:#90EE90
    style ENC fill:#90EE90
    style ENCBTN fill:#90EE90
    style B1 fill:#90EE90
    style B2 fill:#90EE90
    style NOISE fill:#87CEEB
    style BURST fill:#87CEEB
    style PLUCKENV fill:#FFA500
    style STRING1 fill:#87CEEB
    style STRING2 fill:#87CEEB
    style DLINE fill:#87CEEB
    style DAMPER fill:#DDA0DD
    style PICKUP fill:#DDA0DD
    style COUPLE fill:#DDA0DD
    style BRIGHT fill:#87CEEB
    style BODY fill:#87CEEB
    style AOUT fill:#90EE90
    style LED1 fill:#90EE90
    style LED2 fill:#90EE90
```

---

*End of Daisy Pod Project Ideas*
