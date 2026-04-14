# Multi-FX Synth Pod — Controls Documentation

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
        midiin["MIDI In"] --> osc["Dual Osc Synth"]
        audioin["Audio In (Stereo)"] --> mix["Mixer"]
        osc --> mix
        mix --> drive["Overdrive"]
        drive --> comp["Compressor"]
        comp --> del["Delay"]
        del --> cho["Chorus"]
        cho --> dcblock["DC Block"]
        dcblock --> lim["Limiter"]
        lim --> audioout["Audio Out L+R"]
    end
```

## B. Signal Flow

```mermaid
flowchart LR
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px
    classDef synth fill:#8e44ad,stroke:#fff,stroke-width:2px

    MIDI[MIDI IN TRS]:::io --> OSC1[OSC 1]:::synth
    MIDI --> OSC2[OSC 2 +5c]:::synth
    OSC1 --> MIX1[OSC MIX]:::synth
    OSC2 --> MIX1
    
    IN[AUDIO IN]:::io --> MIX2[MAIN MIX]:::audio
    MIX1 --> MIX2
    
    MIX2 --> DRIVE[OVERDRIVE]:::audio
    DRIVE --> COMP[COMPRESSOR]:::audio
    COMP --> DELAY[DELAY]:::audio
    DELAY --> CHORUS[CHORUS]:::audio
    CHORUS --> DC[DC BLOCK]:::audio
    DC --> LIM[LIMITER]:::audio
    LIM --> OUT[AUDIO OUT L+R]:::io

    K1[KNOB 1]:::control -.->|Base Mode: Osc Mix| MIX1
    K1 -.->|Shift Mode: Del Time| DELAY

    K2[KNOB 2]:::control -.->|Base Mode: Drive| DRIVE
    K2 -.->|Shift Mode: Cho Depth| CHORUS

    BTN2[BUTTON 2]:::control -.->|Bypass FX Rack| DELAY
```

## C. Parameter Mapping

### Hardware Controls

| Control | Mode | Function | Range / Notes |
|---------|------|----------|---------------|
| **Button 1** | Global | Toggle Shift Mode | Switches between Base (Blue) and Shift (Green) mode |
| **Button 2** | Global | Toggle FX Bypass | Bypasses Drive, Comp, Delay, and Chorus. |
| **Knob 1** | Base | Osc Blend | Crossfades between Osc 1 and Osc 2 (linear) |
| **Knob 2** | Base | Drive | Saturation amount (0 to Max Drive) |
| **Knob 1** | Shift | Delay Time | 50ms to 800ms |
| **Knob 2** | Shift | Chorus Depth | LFO Modulation Depth (0.0 to 1.0) |

### LED Indicators

| Component | State | Meaning |
|-----------|-------|---------|
| **LED 1** | Blue | Base Mode Active (Knobs edit Synth and Drive) |
| **LED 1** | Green | Shift Mode Active (Knobs edit Delay and Chorus) |
| **LED 2** | OFF | FX Rack Bypassed |
| **LED 2** | Blue / Green | FX Rack Active (color matches LED 1 to indicate current mode) |

## D. Voice Architecture

The synthesizer consists of a dual-oscillator voice triggered via MIDI.

- **Oscillators**: Two High-quality Saw waveforms.
- **Detune**: Osc 2 is offset internally by +5 cents for a thicker sound.
- **Envelope**: Plucked ADSR envelope (A:10ms, D:100ms, S:80%, R:200ms) mapped to output amplitude.

## E. FX Rack Architecture

The FX rack processes a mono sum of the synth voices and external hardware inputs, returning a stereo-spatialised output post-Chorus.

`Audio In L+R + Synth Mix -> Overdrive -> Compressor -> Delay -> Chorus (Stereo) -> Limiter -> Output`
