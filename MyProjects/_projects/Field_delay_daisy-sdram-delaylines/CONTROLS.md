# Controls Report - Field_delay_daisy-sdram-delaylines

## Behavior

Long stereo fractional delay using Field SDRAM-backed buffers. This is the most
primitive-focused adaptation: it highlights external memory, fractional reads,
smear/warp, and ping-pong feedback rather than a complex mode system.

```mermaid
flowchart LR
    INL["Audio In L"] --> DL["Long SDRAM delay L"]
    INR["Audio In R"] --> DR["Long SDRAM delay R"]
    DL --> PP["Ping-pong crossfeed"]
    DR --> PP
    PP --> DL
    PP --> DR
    DL --> TAP["Secondary fractional taps"]
    DR --> TAP
    TAP --> MIX["Dry/Wet mix"]
    INL --> MIX
    INR --> MIX
    MIX --> OUT["Audio Out L/R"]
```

## Knob Layers

Knobs use movement-gated "until touched" layers. Shifted layers never write base
parameters unless the base layer is active and the knob moves.

```mermaid
flowchart TD
    ENTER["Enter layer"] --> SNAP["Snapshot physical knob"]
    SNAP --> IDLE["Stored parameter stays active"]
    IDLE --> MOVE["Knob moved"]
    MOVE --> SET["Set active-layer target"]
```

| Knob | Base | Hold SW1 | Hold SW2 |
|---|---|---|---|
| K1 | Mix | Pre Delay | Range |
| K2 | Long Time ms | Width | Density |
| K3 | Feedback % | Spread | Low Cut Hz |
| K4 | Tone % | Damping | High Cut Hz |
| K5 | Ping-pong % | Rhythm | Smear |
| K6 | Mod % | Freeze Amt | Interp Warp |
| K7 | Input Drive dB | MIDI Level | MIDI Attack ms |
| K8 | Output dB | Tempo BPM | MIDI Release ms |

## Keys And Switches

| Control | Function |
|---|---|
| SW1 | Hold for shift layer 1 |
| SW2 | Hold for shift layer 2 |
| A1 | Bypass state |
| A2 | Freeze long buffer write state |
| A3 | Reverse/grain accent |
| A4 | Rhythm ratio |
| A5 | Diffuse/wide state |
| A6 | MIDI test synth waveform |
| A7 | Octave down |
| A8 | Octave up |
| B1-B8 | White keys C4 D4 E4 F4 G4 A4 B4 C5 |

```mermaid
flowchart LR
    B["B-row white-key notes"] --> SYN["Internal MIDI test tone"]
    MIDI["External MIDI keyboard"] --> SYN
    SYN --> DELAY["Long SDRAM delay"]
    A["A-row states"] --> DELAY
    SW["SW1/SW2"] --> LAYERS["Layered K1-K8"]
```

## OLED

The OLED reports the active layer and values with units. Moving a parameter
opens a short zoom view so the user sees the touched target immediately.

```mermaid
flowchart TD
    MAIN["Active layer values"] --> TOUCH["Touch parameter"]
    TOUCH --> ZOOM["Zoom with value and units"]
    ZOOM --> MAIN
```
