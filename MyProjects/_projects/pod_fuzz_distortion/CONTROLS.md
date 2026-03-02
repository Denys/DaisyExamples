# Pod Fuzz Distortion — Controls Documentation

## A. System Architecture

```mermaid
block-beta
    columns 3

    block:hardware["Pod Hardware"]:3
        knob1["Knob 1"] knob2["Knob 2"]
        btn1["Button 1"] btn2["Button 2"] enc["Encoder"]
        led1["LED 1 (RGB)"] led2["LED 2 (RGB)"]
        midi["MIDI In (TRS)"]
    end

    block:dsp["DSP Chain"]:3
        midiin["MIDI In"] --> modal["Modal Voice x4"]
        audioin["Audio In"] --> mix["Mix"]
        modal --> mix
        mix --> vca["Drive VCA"]
        vca --> clip["Hard/Soft Clip"]
        clip --> svf["SVF Lowpass"]
        svf --> verb["ReverbSc (Stereo)"]
        verb --> audioout["Audio Out L+R"]
    end
```

## B. Signal Flow

```mermaid
flowchart LR
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px
    classDef synth fill:#8e44ad,stroke:#fff,stroke-width:2px

    MIDI[MIDI IN TRS]:::io --> MODAL[MODAL VOICE x4]:::synth
    IN[AUDIO IN]:::io --> MIX[MIX]:::audio
    MODAL --> MIX
    MIX --> VCA[DRIVE VCA x1-20]:::audio
    VCA --> CLIP[HARD / SOFT CLIP]:::audio
    CLIP --> SVF[SVF LOWPASS]:::audio
    SVF --> VERB[REVERBSC STEREO]:::audio
    VERB --> OUT[AUDIO OUT L+R]:::io

    K1[KNOB 1]:::control -.->|Mode 0: Drive| VCA
    K1 -.->|Mode 1: Feedback| VERB
    K1 -.->|Mode 2: Rev Mix| VERB
    K1 -.->|Mode 3: Structure| MODAL

    K2[KNOB 2]:::control -.->|Mode 0: Tone| SVF
    K2 -.->|Mode 1: LP Freq| VERB
    K2 -.->|Mode 2: Damping| MODAL
    K2 -.->|Mode 3: Brightness| MODAL

    BTN1[BUTTON 1]:::control -.->|Toggle| CLIP
    BTN2[BUTTON 2]:::control -.->|Enable/Disable| MODAL
```

## C. Control Flow

```mermaid
flowchart TD
    ENC[Encoder Rotate] --> MODE{Mode 0/1/2/3}

    MODE -->|Mode 0| M0[Fuzz Controls]
    MODE -->|Mode 1| M1[Reverb Controls]
    MODE -->|Mode 2| M2[Mix Controls]
    MODE -->|Mode 3| M3[Synth Controls]

    M0 --> K1_0[K1 -> Drive 1x-20x]
    M0 --> K2_0[K2 -> Tone 200-12kHz]
    M0 --> LED1_0[LED1 = Red brightness]
    M0 --> LED2_0[LED2 = Red/Green clip mode]

    M1 --> K1_1[K1 -> Feedback 0.6-0.999]
    M1 --> K2_1[K2 -> LP Freq 500-18kHz]
    M1 --> LED1_1[LED1 = Yellow]

    M2 --> K1_2[K1 -> Reverb Mix 0-1]
    M2 --> K2_2[K2 -> Voice Damping 0-1]
    M2 --> LED2_2[LED2 = Cyan]

    M3 --> K1_3[K1 -> Structure 0-1]
    M3 --> K2_3[K2 -> Brightness 0-1]
    M3 --> LED1_3[LED1 = Purple]
    M3 --> LED2_3[LED2 = Cyan voice activity]

    BTN1[Button 1 Press] --> TOGGLE[Toggle Hard/Soft Clip]
    BTN2[Button 2 Press] --> SYNTH[Toggle Synth On/Off]
    ENCPUSH[Encoder Push] --> SEQ[RESERVED: Future Sequencer]
```

## Parameter Mapping

### Encoder Modes

| Mode | LED Indicator | Knob 1 | Range | Knob 2 | Range |
|------|---------------|--------|-------|--------|-------|
| 0 — Fuzz | LED1 Red | Drive (Gain) | 1x - 20x (linear) | Tone (Cutoff) | 200 Hz - 12 kHz (log) |
| 1 — Reverb | LED1 Yellow | Feedback | 0.6 - 0.999 | LP Freq | 500 Hz - 18 kHz (log) |
| 2 — Mix | LED2 Cyan | Reverb Mix (Dry/Wet) | 0.0 - 1.0 | Voice Damping | 0.0 - 1.0 |
| 3 — Synth | LED1 Purple | Structure | 0.0 - 1.0 | Brightness | 0.0 - 1.0 |

### Buttons

| Button | Function | Indicator |
|--------|----------|-----------|
| Button 1 | Toggle clip mode | LED2 (mode 0): Red = Hard, Green = Soft |
| Button 2 | Toggle synth on/off | LED2 (mode 3): Cyan = On, Dim Red = Off |
| Encoder Push | RESERVED | Future sequencer implementation |

### Default Values

| Parameter | Default |
|-----------|---------|
| Drive | 1.0x (clean) |
| Tone | 2000 Hz |
| Clip Mode | Hard |
| Feedback | 0.85 |
| LP Freq | 10000 Hz |
| Reverb Mix | 0.5 |
| Structure | 0.5 |
| Brightness | 0.5 |
| Damping | 0.5 |
| Synth | Enabled |

## Voice Architecture

The synth uses a modular `Voice` interface (`voice.h`) for swappable voice implementations.

**Current voice**: `ModalSynth` — 4-voice polyphonic modal synthesis (ModalVoice resonators)

- Mallet exciter: click -> LPF -> resonator (Plaits-derived)
- Triggered (not gated) — natural decay via Damping parameter
- Round-robin voice allocation with free-voice preference
- Per-sample `fonepole()` smoothing on Structure and Brightness
- Velocity-sensitive accent

**To swap voices**: See instructions in `voice.h`
