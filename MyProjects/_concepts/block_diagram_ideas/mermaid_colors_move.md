# Mermaid Tutorial: Colors & Block Positioning for Synth Diagrams

> A practical guide for creating color-coded block diagrams in Obsidian, optimized for synthesizer architecture documentation.

---

## 1. Color Selection and Setting

### 1.1 The Synth Color System

This tutorial uses a consistent 4-color coding system for synthesizer block diagrams:

```mermaid
flowchart LR
  subgraph LEGEND["Legend"]
    A["Audio\n(OSC/VCF/VCA/FX)"]:::audio
    C["Control\n(ADSR/LFO/CV/Gate)"]:::ctrl
    U["UI / I/O\n(Knobs/MIDI/IN/OUT)"]:::ui
    M["Math / Routing\n(Mix/Sum/Mux/Const)"]:::math
  end

  classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
  classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
  classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
  classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

### 1.2 Color Definitions

**Audio (Blue)**
- Use for: Oscillators, Filters (VCF), Amplifiers (VCA), Effects, Engines
- DaisySP examples: `Oscillator`, `Svf`, `Fm2`, `ReverbSc`, `DelayLine`
- Fill: `#1E88E5` (Blue 600)
- Stroke: `#0D47A1` (Blue 900)

**Control (Orange)**
- Use for: Envelopes (ADSR), LFOs, Gates, Triggers, Clocks, CV signals
- DaisySP examples: `AdEnv`, `Metro`, `AudioDetector`
- Fill: `#FB8C00` (Orange 600)
- Stroke: `#E65100` (Orange 900)
- Text color: `#111111` (Black - for contrast on orange)

**UI / I/O (Green)**
- Use for: Physical inputs/outputs, Knobs, Buttons, Keys, MIDI ports, Audio jacks
- Fill: `#43A047` (Green 600)
- Stroke: `#1B5E20` (Green 900)

**Math / Routing (Violet)**
- Use for: Mixers, Summing buses, Mux/Demux, Constants, Voice managers, Sequencers
- Fill: `#8E24AA` (Purple 600)
- Stroke: `#4A148C` (Purple 900)

### 1.3 Applying Colors to Nodes

**Method 1: Inline Class Tag (Recommended)**

```mermaid
flowchart TD
    OSC["Oscillator"]:::audio
    VCF["SVF Filter"]:::audio
    VCA["VCA"]:::audio
    OUT["Output"]:::ui

    OSC --> VCF --> VCA --> OUT

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
```

**Method 2: Separate classDef Block**

```mermaid
flowchart TD
    OSC["Oscillator"]
    VCF["SVF Filter"]
    VCA["VCA"]
    ADSR["ADSR"]:::ctrl
    OUT["Output"]

    OSC --> VCF --> VCA --> OUT
    ADSR --> VCA

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
```

### 1.4 Color Assignment Rules

| Signal Type | Color | Example |
|-------------|-------|---------|
| Audio-rate oscillators | Blue | `Oscillator`, `Fm2`, `VosimOscillator` |
| Audio processing | Blue | `Svf`, `Overdrive`, `DelayLine`, `ReverbSc` |
| Control voltage (CV) | Orange | ADSR output, LFO output, Gate, Trigger |
| Physical controls | Green | Knobs, Buttons, Keys, Audio In/Out |
| MIDI input | Green | `MIDI In`, `MIDI Note` |
| Routing/Mixing | Violet | `Mixer`, `Sum`, `VoiceManager`, `Sequencer` |

### 1.5 Low-Contrast Alternative (Optional)

For users who prefer lighter colors:

```mermaid
flowchart LR
    A["Audio"]:::audio
    C["Control"]:::ctrl
    U["UI / I/O"]:::ui
    M["Math / Routing"]:::math

    classDef audio fill:#42A5F5,stroke:#1565C0,stroke-width:2px,color:#0b0b0b;
    classDef ctrl  fill:#FFB74D,stroke:#EF6C00,stroke-width:2px,color:#0b0b0b;
    classDef ui    fill:#66BB6A,stroke:#2E7D32,stroke-width:2px,color:#0b0b0b;
    classDef math  fill:#BA68C8,stroke:#6A1B9A,stroke-width:2px,color:#0b0b0b;
```

---

## 2. Block Position Setting

### 2.1 Flow Direction

**Top-to-Bottom (TD)** - Recommended for signal flow diagrams:

```mermaid
flowchart TD
    IN["Input"] --> PROC["Processing"] --> OUT["Output"]
```

**Left-to-Right (LR)** - Alternative for horizontal layouts:

```mermaid
flowchart LR
    IN["Input"] --> PROC["Processing"] --> OUT["Output"]
```

### 2.2 Single Block Positioning

Mermaid auto-layout positions nodes based on declaration order and connections.

**Basic Chain (Spine Pattern)**

```mermaid
flowchart TD
    OSC["Oscillator"]:::audio --> VCF["VCF"]:::audio --> VCA["VCA"]:::audio --> OUT["Output"]:::ui
```

**Branching Paths**

```mermaid
flowchart TD
    IN["Input"] --> SPLIT["Split"]:::math
    SPLIT --> PATH1["Path A"]:::audio
    SPLIT --> PATH2["Path B"]:::audio
    PATH1 --> MIX["Mixer"]:::math
    PATH2 --> MIX
    MIX --> OUT["Output"]:::ui
```

### 2.3 Multiple Block Grouping with Subgraphs

**Subgraph for Logical Grouping**

```mermaid
flowchart TD
    subgraph AUDIO["Audio Path"]
        OSC["Oscillator"]:::audio --> VCF["VCF"]:::audio --> VCA["VCA"]:::audio
    end

    subgraph CONTROL["Control Path"]
        ADSR["ADSR"]:::ctrl
        LFO["LFO"]:::ctrl
    end

    ADSR --> VCA
    LFO --> VCF

    VCA --> OUT["Output"]:::ui
```

**Subgraph for Parallel Processing**

```mermaid
flowchart TD
    IN["Input"] --> SPLIT["Input Splitter"]:::math

    subgraph FX1["FX Chain A"]
        DIST["Distortion"]:::audio --> FILT["Filter"]:::audio
    end

    subgraph FX2["FX Chain B"]
        CHORUS["Chorus"]:::audio --> DLY["Delay"]:::audio
    end

    SPLIT --> FX1
    SPLIT --> FX2

    FX1 --> MIX["Final Mix"]:::math
    FX2 --> MIX
    MIX --> OUT["Output"]:::ui
```

### 2.4 Complex Routing Patterns

**Modulation Injection Pattern**

```mermaid
flowchart TD
    subgraph AUDIO["Audio Path"]
        OSC["Oscillator"]:::audio --> VCF["VCF"]:::audio --> VCA["VCA"]:::audio --> OUT["Output"]:::ui
    end

    subgraph MOD["Modulation Sources"]
        LFO["LFO"]:::ctrl
        ADSR["ADSR"]:::ctrl
    end

    LFO -->|Cutoff CV| VCF
    ADSR -->|Amp CV| VCA
```

**Feedback Loop Pattern**

```mermaid
flowchart TD
    IN["Input"]:::ui --> DLY["Delay Line"]:::audio
    DLY --> OUT["Output"]:::ui
    DLY -->|Feedback| DLY
```

**Mixer Summing Pattern**

```mermaid
flowchart TD
    OSC1["Osc 1"]:::audio --> MIX["Mixer"]:::math
    OSC2["Osc 2"]:::audio --> MIX
    OSC3["Osc 3"]:::audio --> MIX
    OSC4["Osc 4"]:::audio --> MIX
    MIX --> VCF["VCF"]:::audio --> OUT["Output"]:::ui
```

### 2.5 Layout Control Techniques

**Anchor Nodes for Alignment**

```mermaid
flowchart TD
    X1[" "] --> OSC["Oscillator"]:::audio --> X2[" "]
    X1 --> LFO["LFO"]:::ctrl
    X2 --> VCF["VCF"]:::audio
```

**Note:** Label invisible anchor nodes with a space or dot (`[" "]` or `["·"]`) if they appear.

**Bus Architecture Pattern**

```mermaid
flowchart TD
    subgraph SOURCES["Source Modules"]
        OSC["Oscillator"]:::audio
        NOISE["Noise"]:::audio
    end

    OSC --> SUM["Mix Bus"]:::math
    NOISE --> SUM

    SUM --> FX["FX Processor"]:::audio --> OUT["Output"]:::ui
```

### 2.6 Complete Synth Voice Template

This template demonstrates a complete subtractive synth voice:

```mermaid
flowchart TD
    %% Audio Path (Blue)
    MIDI["MIDI In"]:::ui --> CV["CV→Freq"]:::math
    CV --> OSC1["Osc 1"]:::audio
    CV --> OSC2["Osc 2"]:::audio
    OSC1 --> MIX["Mix"]:::math
    OSC2 --> MIX
    MIX --> VCF["SVF Filter"]:::audio --> VCA["VCA"]:::audio --> OUT["Audio Out"]:::ui

    %% Control Path (Orange)
    MIDI -->|Gate| ADSR["ADSR"]:::ctrl
    LFO["LFO"]:::ctrl --> VCF
    ADSR --> VCA

    %% UI Controls (Green)
    K1["Knob 1: Cutoff"]:::ui --> VCF
    K2["Knob 2: Res"]:::ui --> VCF
    K3["Knob 3: Attack"]:::ui --> ADSR
    K4["Knob 4: Release"]:::ui --> ADSR
    K5["Knob 5: LFO Rate"]:::ui --> LFO

    %% Color Definitions
    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

### 2.7 Common Layout Mistakes & Fixes

| Mistake | Fix |
|---------|-----|
| Diagram too wide | Use `flowchart TD` instead of `LR`, or split into multiple diagrams |
| Nodes overlapping | Declare main chain first, modulation later |
| Messy modulation lines | Group modulation sources in a subgraph |
| Too many nodes at once | Split into Audio diagram + Control diagram |

---

## 3. Quick Reference

### 3.1 Copy-Paste Color Block

```mermaid
flowchart TD
    %% Nodes here
    AUD["Audio Module"]:::audio
    CTRL["Control Module"]:::ctrl
    UI["UI Element"]:::ui
    MATH["Routing/Math"]:::math

    classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
    classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
    classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
    classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

### 3.2 Legend Macro

```mermaid
flowchart LR
  subgraph LEGEND["Legend"]
    A["Audio\n(OSC/VCF/VCA/FX)"]:::audio
    C["Control\n(ADSR/LFO/CV/Gate)"]:::ctrl
    U["UI / I/O\n(Knobs/MIDI/IN/OUT)"]:::ui
    M["Math / Routing\n(Mix/Sum/Mux/Const)"]:::math
  end

  classDef audio fill:#1E88E5,stroke:#0D47A1,stroke-width:2px,color:#ffffff;
  classDef ctrl  fill:#FB8C00,stroke:#E65100,stroke-width:2px,color:#111111;
  classDef ui    fill:#43A047,stroke:#1B5E20,stroke-width:2px,color:#ffffff;
  classDef math  fill:#8E24AA,stroke:#4A148C,stroke-width:2px,color:#ffffff;
```

---

## 4. Related Notes

- [[Mermaid Workbook (Part 2): ASCII → Color-Coded Synth Diagrams]]
- [[Mermaid Style Guide (Synth Diagrams): Obsidian-First]]
- [[Daisy Field Architecture Ideas]]
- [[Daisy Pod Architecture Ideas]]

---

*Generated per the Luminous Modular Grammar color system for synth diagram documentation.*
