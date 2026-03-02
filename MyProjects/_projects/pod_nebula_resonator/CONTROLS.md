# Nebula-Resonator — Controls Documentation

## A. System Architecture

```mermaid
block-beta
    columns 3

    block:hardware["Pod Hardware"]:3
        knob1["Knob 1"] knob2["Knob 2"]
        btn1["Button 1 (Freeze)"] btn2["Button 2 (Seq)"] enc["Encoder (Page)"]
        led1["LED 1 (RGB)"] led2["LED 2 (RGB)"]
        midi["MIDI In (TRS)"]
    end

    block:source["Internal Source Engine"]:1
        blosc["BlOsc (Saw/Sq)"]
        noise["WhiteNoise"]
        srcmix["Source Mix"]
    end

    block:buffer["Freeze System"]:1
        fbuf["FreezeBuffer (SDRAM 96k)"]
        fgate["Freeze Gate"]
    end

    block:granular["Granular Engine"]:1
        tv["TextureVoice (4-Tap)"]
        jit["Jitter x4"]
        seq["Step Sequencer (8-step)"]
    end

    block:output["Output Stage"]:3
        verb["ReverbSc (Stereo)"]
        audioout["Audio Out L+R"]
    end
```
```mermaid
block-beta
    columns 3

    block:hardware["Pod Hardware"]:3
        knob1["Knob 1"] knob2["Knob 2"]
        btn1["Button 1 (Freeze)"] btn2["Button 2 (Seq)"] enc["Encoder (Page)"]
        led1["LED 1 (RGB)"] led2["LED 2 (RGB)"]
        midi["MIDI In (TRS)"]
    end

    block:source["Internal Source Engine"]:1
        blosc["BlOsc (Saw/Sq)"]
        noise["WhiteNoise"]
        srcmix["Source Mix"]
    end

    block:buffer["Freeze System"]:1
        fbuf["FreezeBuffer (SDRAM 96k)"]
        fgate["Freeze Gate"]
    end

    block:granular["Granular Engine"]:1
        tv["TextureVoice (4-Tap)"]
        jit["Jitter x4"]
        seq["Step Sequencer (8-step)"]
    end

    block:output["Output Stage"]:3
        verb["ReverbSc (Stereo)"]
        audioout["Audio Out L+R"]
    end
```

## B. Signal Flow

```mermaid
flowchart LR
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px
    classDef synth fill:#8e44ad,stroke:#fff,stroke-width:2px
    classDef buffer fill:#c0392b,stroke:#fff,stroke-width:2px

    BLOSC[BlOsc Saw/Sq]:::synth --> SRCMIX[Source Mix]:::synth
    NOISE[WhiteNoise]:::synth --> SRCMIX
    SRCMIX --> FBUF[FreezeBuffer 96k SDRAM]:::buffer
    FBUF --> TV[TextureVoice 4-Tap]:::audio
    TV --> VERB[ReverbSc Stereo]:::audio
    VERB --> OUT[Audio Out L+R]:::io

    SEQ[Step Sequencer 8-step]:::control -.->|Scan Mod| TV
    METRO[Metro Clock]:::control -.->|Tick| SEQ
    MIDI[MIDI In TRS]:::io -.->|Clock 0xF8| METRO
    MIDI -.->|NoteOn| BLOSC

    K1[KNOB 1]:::control -.->|Mode 0: Timbre| SRCMIX
    K1 -.->|Mode 1: Texture| TV
    K1 -.->|Mode 2: Seq Depth| SEQ

    K2[KNOB 2]:::control -.->|Mode 0: Decay| SRCMIX
    K2 -.->|Mode 1: Scan Pos| TV
    K2 -.->|Mode 2: Rev Mix| VERB

    BTN1[BUTTON 1]:::control -.->|Toggle Freeze| FBUF
    BTN2[BUTTON 2]:::control -.->|Start/Stop| SEQ
```

## C. Control Flow

```mermaid
flowchart TD
    ENC[Encoder Click] --> MODE{Page 0/1/2}

    MODE -->|Page 0| P0[SOURCE Controls]
    MODE -->|Page 1| P1[GRANULAR Controls]
    MODE -->|Page 2| P2[MOTION Controls]

    P0 --> K1_0["K1 → Timbre Morph<br/>0-50%: Osc Shape<br/>50-100%: Noise Blend"]
    P0 --> K2_0["K2 → Source Decay<br/>10ms - 2s"]
    P0 --> LED0["LED1 = Cyan"]

    P1 --> K1_1["K1 → Texture / Jitter<br/>0 - 1000 samples spread"]
    P1 --> K2_1["K2 → Scan Position<br/>0.0 - 1.0 (manual scrub)"]
    P1 --> LED1M["LED1 = Magenta"]

    P2 --> K1_2["K1 → Seq Depth<br/>0.0 - 1.0 modulation"]
    P2 --> K2_2["K2 → Reverb Mix<br/>0.0 - 1.0"]
    P2 --> LED2Y["LED1 = Yellow"]

    BTN1[Button 1 Press] --> FREEZE["Toggle FREEZE<br/>LED1: Red=Frozen, Green=Recording"]
    BTN2[Button 2 Press] --> SEQCTL["Toggle Sequencer<br/>LED2: Pulse on beat"]
    ENCPUSH[Encoder Push] --> RES["RESERVED: Future Seq Edit"]
```

## Parameter Mapping

### Encoder Modes (3-Page FSM)

| Mode | LED1 Color | Knob 1 | Range | Knob 2 | Range |
|------|------------|--------|-------|--------|-------|
| 0 — Source | Cyan | Timbre Morph | 0-50% Osc shape (Saw→Sq), 50-100% Noise blend | Source Decay | 10ms - 2s (envelope) |
| 1 — Granular | Magenta | Texture (Jitter) | 0 - 1000 samples spread | Scan Position | 0.0 - 1.0 (manual scrub) |
| 2 — Motion | Yellow | Seq Depth | 0.0 - 1.0 (seq → scan mod) | Reverb Mix | 0.0 - 1.0 (dry/wet) |

### Buttons

| Button | Function | LED Indicator |
|--------|----------|---------------|
| Button 1 | FREEZE toggle (lock/unlock buffer) | LED1: Red = Frozen, Green = Recording |
| Button 2 | Sequencer Start/Stop | LED2: Pulse on beat when running |
| Encoder Push | RESERVED | Future: step sequencer edit mode |

### Default Values

| Parameter | Default |
|-----------|---------|
| Osc Frequency | 110 Hz (A2 drone) |
| Osc Waveform | Sawtooth |
| Noise Mix | 0.0 (pure osc) |
| Source Decay | 500ms |
| Freeze | Off (recording) |
| Scan Position | 0.5 (buffer center) |
| Texture (Jitter) | 0.3 |
| Seq Rate | 4 Hz |
| Seq Depth | 0.3 |
| Reverb Mix | 0.4 |
| Reverb Feedback | 0.85 |
| Reverb LP Freq | 10000 Hz |

## DSP Architecture

### Internal Source Engine
- `daisysp::BlOsc` — Band-limited oscillator (Saw/Square morphing)
- `daisysp::WhiteNoise` — Noise texture layer
- Timbre Morph macro: 0-50% sweeps waveform, 50-100% crossfades noise in
- MIDI NoteOn triggers re-inject pitched tones into buffer

### FreezeBuffer (Custom Class)
- 96000 samples (~2s at 48kHz) in `DSY_SDRAM_BSS`
- Conditional `Write()` — bypassed when frozen
- Fractional `Read()` with linear interpolation
- Pre-filled with sawtooth on Init (immediate sound)

### TextureVoice (Custom Class)
- 4 asynchronous read heads with jittered offsets
- `daisysp::Jitter` x4 for Brownian-motion position randomization
- Stereo panning: taps 0,2 → Left, taps 1,3 → Right
- Hanning window per grain to prevent clicks

### Step Sequencer
- 8-step array of scan positions (0.0 - 1.0)
- `daisysp::Metro` for internal clock (syncable to MIDI)
- `daisysp::Port` for smooth transitions between steps
- Seq Depth knob scales modulation amount

### Soft Takeover Protocol
- Stored ghost values per page per knob
- Parameter only updates when physical knob crosses stored value
- Prevents audible jumps on page change
