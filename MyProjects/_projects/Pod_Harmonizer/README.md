# Pod_Harmonizer

**Platform**: Daisy Pod  
**Category**: Audio Effect (Pitch Processing)  
**Complexity**: Intermediate

---

## Project Definition

A **stereo harmonizer** effect that takes audio input and creates two pitch-shifted harmony voices. The user can control the harmony intervals and mix between dry and wet signals.

### Features
- Dual pitch shifting (Voice A and Voice B)
- MIDI-controlled transposition (optional)
- Dry/Wet mix
- Button-selectable harmony presets (Thirds, Fifths, Octave)

### Control Mapping

| Control | Function | Range |
|---------|----------|-------|
| **Knob 1** | Dry/Wet Mix | 0-100% |
| **Knob 2** | Detune Amount | 0-50 cents |
| **Button 1** | Cycle Harmony Preset | 3rds / 5ths / Oct |
| **Button 2** | Bypass Toggle | On / Off |
| **LED** | Indicates Mode | Green=3rd, Blue=5th, Purple=Oct |

### Hardware Constraints
- Sample Rate: 48kHz
- Block Size: 48 samples
- Audio: Stereo In / Stereo Out
- MIDI: Optional (for real-time interval control)

---

## Block Diagram (Mermaid)

This is the **source of truth** for the signal flow. C++ implementation MUST match this diagram.

```mermaid
flowchart TB
    subgraph INPUTS["📥 INPUTS"]
        AUDIO_IN["audio_input\n(Stereo)"]
        K1["knob 1\nDry/Wet"]
        K2["knob 2\nDetune"]
        B1["button 1\nPreset Cycle"]
        B2["button 2\nBypass"]
    end

    subgraph PITCH_SECTION["🎵 PITCH SHIFTERS"]
        direction LR
        PS_A["pitch_shifter\nVoice A (+Interval)"]
        PS_B["pitch_shifter\nVoice B (-Interval)"]
    end

    subgraph MIX_SECTION["🎚️ MIXER"]
        direction LR
        DRY["dry_path"]
        WET_MIX["add\n(Voice A + Voice B)"]
        XFADE["crossfade"]
    end

    subgraph OUTPUT["🔊 OUTPUT"]
        AUDIO_OUT["audio_output\n(Stereo)"]
    end

    %% Signal Flow
    AUDIO_IN -->|"L"| PS_A
    AUDIO_IN -->|"R"| PS_B
    AUDIO_IN -->|"dry"| DRY

    PS_A --> WET_MIX
    PS_B --> WET_MIX

    DRY -->|"in1"| XFADE
    WET_MIX -->|"in2"| XFADE
    K1 -->|"mix"| XFADE

    XFADE --> AUDIO_OUT

    %% Control Connections (dashed)
    B1 -.->|"interval"| PS_A
    B1 -.->|"interval"| PS_B
    K2 -.->|"detune"| PS_A
    K2 -.->|"detune"| PS_B
    B2 -.->|"bypass"| XFADE

    %% Styling
    style AUDIO_IN fill:#51cf66,stroke:#2f9e44,color:#fff
    style AUDIO_OUT fill:#ffd43b,stroke:#fab005,color:#000
    style PS_A fill:#74c0fc,stroke:#339af0
    style PS_B fill:#74c0fc,stroke:#339af0
    style K1 fill:#e599f7,stroke:#be4bdb
    style K2 fill:#e599f7,stroke:#be4bdb
    style B1 fill:#ff8787,stroke:#fa5252
    style B2 fill:#ff8787,stroke:#fa5252
```

### Block Legend
| Color | Meaning |
|-------|---------|
| 🟢 **Green** | Audio Input |
| 🔵 **Blue** | DSP Blocks (Pitch Shifters) |
| 🟣 **Purple** | Knob Controls |
| 🔴 **Red** | Button Controls |
| 🟡 **Yellow** | Audio Output |

---

## DVPE Gap Analysis (Pre-Implementation)

**Expected Rating**: 9/10

All blocks in the diagram exist in DVPE:
- `audio_input` ✅
- `pitch_shifter` ✅
- `add` ✅
- `crossfade` ✅
- `audio_output` ✅
- `knob` ✅
- `button` ✅

**Minor Gap**: The "Preset Cycling" logic (Button 1 → State Machine → Interval Value) requires a small state machine or lookup table. This can be handled with a `counter` or `selector` block if available, or embedded in the `pitch_shifter` parameters.

---

## C++ Implementation

The C++ module uses the DaisySP pitch shifters with dynamic preset shifting configurations and a strict split-polling latency architecture separating analog signal polling (`ProcessAnalogControls()`) and UI button actions in the primary loop.

For a detailed visual map of how parameters and user controls connect directly to DSP operations, please refer to [CONTROLS.md](CONTROLS.md).
