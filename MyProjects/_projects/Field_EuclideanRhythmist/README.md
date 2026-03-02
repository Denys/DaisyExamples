# Field_EuclideanRhythmist

**Platform**: Daisy Field  
**Category**: Algorithmic Sequencer / Drum Machine  
**Complexity**: High (Algorithmic Logic)

---

## Project Definition

An **8-channel Euclidean rhythm sequencer** that generates drum patterns using the Euclidean algorithm. Each channel controls a different drum voice, and users can adjust the number of pulses, sequence length, and rotation for each.

### What is a Euclidean Rhythm?
The Euclidean algorithm distributes `K` pulses as evenly as possible across `N` steps. For example:
- `E(3, 8)` = `[X . . X . . X .]` (Cuban Tresillo)
- `E(4, 16)` = `[X . . . X . . . X . . . X . . .]` (4-on-the-floor)
- `E(5, 8)` = `[X . X X . X X .]` (Bossa Nova)

### Features
- 8 drum voices (Kick, Snare, Clap, HiHat Closed, HiHat Open, Tom, Rim, Cowbell)
- Per-voice Euclidean parameters: Pulses (K), Length (N), Rotation (offset)
- Global tempo with tap tempo
- Internal clock or external MIDI sync
- OLED: Circular Euclidean ring visualization

### Control Mapping

| Control | Function | Range |
|---------|----------|-------|
| **Knob 1** | Pulses (K) for selected voice | 0-16 |
| **Knob 2** | Length (N) for selected voice | 1-16 |
| **Knob 3** | Rotation for selected voice | 0 to N-1 |
| **Knob 4** | Voice Decay | 0-100% |
| **Knob 5-6** | Voice Tone/Tune | Varies |
| **Knob 7** | Tempo | 40-240 BPM |
| **Knob 8** | Swing | 0-50% |
| **Keys A1-A8** | Select voice 1-8 | Toggle |
| **Keys B1-B8** | Mute voice 1-8 | Toggle |
| **SW1** | Play/Stop | Toggle |
| **SW2** | Tap Tempo | Tap |

### Hardware Constraints
- Sample Rate: 48kHz
- Block Size: 48 samples
- Audio: Stereo Out (L = Kick/Snare/Tom, R = HiHat/Clap/Rim)
- MIDI: Clock Sync In (optional)

---

## Block Diagram (Mermaid)

This is the **source of truth** for the signal flow. C++ implementation MUST match this diagram.

```mermaid
flowchart TB
    subgraph CLOCK["⏱️ TIMING ENGINE"]
        direction LR
        TEMPO["knob K7\nTempo"]
        SWING["knob K8\nSwing"]
        TAP["button SW2\nTap Tempo"]
        METRO["metro\n(Internal Clock)"]
        MIDI_CLK["midi_clock\n(External Sync)"]
        CLK_MUX["mux\n(Clock Source)"]
        
        TEMPO --> METRO
        TAP -.-> METRO
        METRO --> CLK_MUX
        MIDI_CLK --> CLK_MUX
    end

    subgraph VOICE_SELECT["🎛️ VOICE SELECTION"]
        direction TB
        KEYS_A["keys A1-A8\n(Voice Select)"]
        KEYS_B["keys B1-B8\n(Voice Mute)"]
        K1["knob K1\nPulses (K)"]
        K2["knob K2\nLength (N)"]
        K3["knob K3\nRotation"]
        K4["knob K4\nDecay"]
    end

    subgraph EUCLIDEAN_ENGINE["🧮 EUCLIDEAN ALGORITHM (x8)"]
        direction TB
        subgraph VOICE_1["Voice 1: Kick"]
            EU1["euclidean_gen\nK=4, N=16"]
        end
        subgraph VOICE_2["Voice 2: Snare"]
            EU2["euclidean_gen\nK=2, N=8"]
        end
        subgraph VOICE_3["Voice 3: Clap"]
            EU3["euclidean_gen"]
        end
        subgraph VOICE_4["Voice 4: CHat"]
            EU4["euclidean_gen"]
        end
        subgraph VOICE_5["Voice 5: OHat"]
            EU5["euclidean_gen"]
        end
        subgraph VOICE_6["Voice 6: Tom"]
            EU6["euclidean_gen"]
        end
        subgraph VOICE_7["Voice 7: Rim"]
            EU7["euclidean_gen"]
        end
        subgraph VOICE_8["Voice 8: Cowbell"]
            EU8["euclidean_gen"]
        end
    end

    subgraph DRUM_VOICES["🥁 DRUM SYNTHESIS"]
        direction LR
        KICK["analog_bass_drum"]
        SNARE["synth_snare_drum"]
        CLAP["🆕 clap_synth"]
        CHAT["hihat\n(Closed)"]
        OHAT["hihat\n(Open)"]
        TOM["🆕 tom_synth"]
        RIM["🆕 rim_synth"]
        COWBELL["🆕 cowbell_synth"]
    end

    subgraph MIXER["🎚️ MIXER"]
        direction LR
        MIX_L["add\n(L Channel)"]
        MIX_R["add\n(R Channel)"]
    end

    subgraph OUTPUT["🔊 OUTPUT"]
        AUDIO_OUT["audio_output\n(Stereo)"]
        OLED["oled_display\n(Euclidean Rings)"]
    end

    %% Clock Distribution
    CLK_MUX -->|"step"| EU1
    CLK_MUX -->|"step"| EU2
    CLK_MUX -->|"step"| EU3
    CLK_MUX -->|"step"| EU4
    CLK_MUX -->|"step"| EU5
    CLK_MUX -->|"step"| EU6
    CLK_MUX -->|"step"| EU7
    CLK_MUX -->|"step"| EU8
    SWING -.->|"offset"| CLK_MUX

    %% Euclidean Triggers -> Drums
    EU1 -->|"trig"| KICK
    EU2 -->|"trig"| SNARE
    EU3 -->|"trig"| CLAP
    EU4 -->|"trig"| CHAT
    EU5 -->|"trig"| OHAT
    EU6 -->|"trig"| TOM
    EU7 -->|"trig"| RIM
    EU8 -->|"trig"| COWBELL

    %% Voice Parameter Control
    KEYS_A -.->|"select"| EUCLIDEAN_ENGINE
    K1 -.->|"K"| EUCLIDEAN_ENGINE
    K2 -.->|"N"| EUCLIDEAN_ENGINE
    K3 -.->|"rot"| EUCLIDEAN_ENGINE
    K4 -.->|"decay"| DRUM_VOICES
    KEYS_B -.->|"mute"| DRUM_VOICES

    %% Audio Mixing (Stereo Pan)
    KICK --> MIX_L
    SNARE --> MIX_L
    TOM --> MIX_L
    CLAP --> MIX_R
    CHAT --> MIX_R
    OHAT --> MIX_R
    RIM --> MIX_R
    COWBELL --> MIX_R

    MIX_L --> AUDIO_OUT
    MIX_R --> AUDIO_OUT

    %% OLED Visualization
    EUCLIDEAN_ENGINE -.->|"pattern"| OLED

    %% Color coding
    style CLOCK fill:#ffd43b,stroke:#fab005
    style EUCLIDEAN_ENGINE fill:#ffec99,stroke:#f08c00,stroke-width:2px,stroke-dasharray: 5 5
    style DRUM_VOICES fill:#74c0fc,stroke:#339af0
    style MIXER fill:#b2f2bb,stroke:#51cf66
    style AUDIO_OUT fill:#51cf66,stroke:#2f9e44,color:#fff
```

### Block Legend
| Color | Meaning |
|-------|---------|
| 🟡 **Yellow** | Timing/Clock Engine |
| 🟠 **Orange Dashed** | Euclidean Algorithm (Custom Logic) |
| 🔵 **Blue** | Drum Synthesis (DaisySP) |
| 🟢 **Green** | Mixer / Output |

---

## DVPE Gap Analysis (Pre-Implementation)

**Expected Rating**: 4/10

### Identified Gaps

| Block | Status | Notes |
|-------|--------|-------|
| `analog_bass_drum` | ✅ Exists | DaisySP |
| `synth_snare_drum` | ✅ Exists | DaisySP |
| `hihat` | ✅ Exists | DaisySP |
| `metro` | ✅ Exists | Clock source |
| `midi_clock` | ⚠️ Partial | MIDI blocks exist, clock sync may need work |
| **`euclidean_gen`** | ❌ **MISSING** | Core algorithm block |
| `clap_synth` | ❌ Missing | No dedicated clap in DaisySP |
| `tom_synth` | ❌ Missing | No dedicated tom in DaisySP |
| `rim_synth` | ❌ Missing | No dedicated rim in DaisySP |
| `cowbell_synth` | ❌ Missing | No dedicated cowbell in DaisySP |
| `swing` | ⚠️ Partial | Needs custom timing offset logic |

**Critical Gap**: The `euclidean_gen` block (the core Euclidean rhythm generator) does not exist in DVPE. This is pure algorithmic logic, not signal flow.

---

## C++ Implementation

### Implementation Status
- ✅ Internal clock with tap tempo (SW2)
- ✅ Swing applied to even/odd 16th steps
- ✅ Bjorklund-based Euclidean pattern generation per voice
- ✅ OLED ring visualization for the selected voice
- ⚠️ External MIDI clock sync not implemented yet
