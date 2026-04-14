# Pod Harmonizer — Controls Documentation

## A. System Architecture

```mermaid
block-beta
    columns 3

    block:hardware["Pod Hardware"]:3
        knob1["Knob 1"] knob2["Knob 2"]
        btn1["Button 1"] btn2["Button 2"]
        led1["LED 1 (RGB)"]
        audioin["Stereo Audio In"]
    end

    block:dsp["DSP Chain"]:3
        audioin --> dualps["Dual Pitch Shift"]
        dualps --> drywet["Crossfader"]
        drywet --> audioout["Stereo Audio Out"]
    end
```

## B. Signal Flow

```mermaid
flowchart LR
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px
    classDef synth fill:#8e44ad,stroke:#fff,stroke-width:2px

    IN[AUDIO IN]:::io --> PSA[PITCH SHIFT A]:::audio
    IN --> PSB[PITCH SHIFT B]:::audio
    
    IN --> XFADE[CROSSFADER]:::audio
    
    PSA --> WETMIX[WET MIX]:::audio
    PSB --> WETMIX
    
    WETMIX --> XFADE
    XFADE --> OUT[AUDIO OUT L+R]:::io

    K1[KNOB 1]:::control -.->|Dry / Wet Mix| XFADE
    K2[KNOB 2]:::control -.->|Detune Cents| PSA
    K2 -.->|Detune Cents| PSB

    BTN1[BUTTON 1]:::control -.->|Cycles Interval Presets| PSA
    BTN1 -.->|Cycles Interval Presets| PSB
    BTN2[BUTTON 2]:::control -.->|Global System Bypass| XFADE
```

## C. Parameter Mapping

### Hardware Controls

| Control | Mode | Function | Range / Notes |
|---------|------|----------|---------------|
| **Button 1** | Global | Preset Cycler | Cycles between Thirds, Fifths, and Octave presets |
| **Button 2** | Global | Bypass Toggle| Bypasses effect, sending only dry input to the output. |
| **Knob 1** | Global | Mix | Crossfades between 0% (Dry) and 100% (Wet) |
| **Knob 2** | Global | Detune | Pitch offsets both shifters (0 - 50 Cents) for chorus-like thickness |

### LED Indicators

| Component | Color | Presets |
|-----------|-------|---------|
| **LED 1** | Green | Thirds (+Major 3rd, -Minor 3rd) |
| **LED 1** | Blue | Fifths (+Perfect 5th, -Perfect 4th) |
| **LED 1** | Purple| Octave (+1 Octave, -1 Octave) |
| **LED 1** | Dim | Global Bypass Active |

## D. Presets

1. **Thirds (Green)**: Creates a major 3rd above the input and a minor 3rd below the input, forming rich chordal sounds with single melody lines.
2. **Fifths (Blue)**: Creates a perfect 5th up and a perfect 4th down. An iconic harmony setting for strong root reinforcements.
3. **Octaves (Purple)**: Standard dual octaver. One octave up, one octave down. Excellent for expanding sonic width. 
