# Controls Report - Field_delay_daisy-reverb-playground

## Behavior

FDN-style reverb/delay tank with diffusion and damping. It is an algorithmic
playground adaptation, not a literal port of one source example.

```mermaid
flowchart LR
    IN["Audio In + MIDI test tone"] --> PRE["Pre delay"]
    PRE --> DIFF["Diffusion"]
    DIFF --> FDN["4-line damped FDN tank"]
    FDN --> MATRIX["Hadamard feedback mix"]
    MATRIX --> FDN
    FDN --> MIX["Dry/Wet mix"]
    IN --> MIX
    MIX --> OUT["Audio Out L/R"]
```

## Knob Layers

Knobs use movement-gated "until touched" layers. Holding `SW1` or `SW2` changes
the active target, and only moved knobs write values.

```mermaid
flowchart TD
    START["Layer entry"] --> SNAP["Snapshot physical K1-K8"]
    SNAP --> WAIT["Keep stored parameters"]
    WAIT --> MOVE["Knob moves"]
    MOVE --> WRITE["Write active-layer parameter only"]
```

| Knob | Base | Hold SW1 | Hold SW2 |
|---|---|---|---|
| K1 | Mix | Pre Delay | Tank Size |
| K2 | Delay Time ms | Width | Density |
| K3 | Decay % | Diffusion | Low Cut Hz |
| K4 | HF Damp % | Damping | High Cut Hz |
| K5 | Tank Color % | Rhythm | Smear |
| K6 | Mod % | Freeze Amt | Warp |
| K7 | Input Drive dB | MIDI Level | MIDI Attack ms |
| K8 | Output dB | Tempo BPM | MIDI Release ms |

## Keys And Switches

| Control | Function |
|---|---|
| SW1 | Hold for shift layer 1 |
| SW2 | Hold for shift layer 2 |
| A1 | Bypass state: off active, blink wet-only, on bypass |
| A2 | Freeze state for reverb tail write behavior |
| A3 | Reverse/grain accent |
| A4 | Rhythm ratio |
| A5 | Diffusion density boost |
| A6 | MIDI test synth waveform |
| A7 | Octave down |
| A8 | Octave up |
| B1-B8 | White keys C4 D4 E4 F4 G4 A4 B4 C5 |

```mermaid
flowchart LR
    A["A-row mode states"] --> FDN["FDN/diffusion behavior"]
    B["B-row white-key test notes"] --> TONE["MIDI test tone"]
    MIDI["External MIDI keyboard"] --> TONE
    TONE --> FDN
    SW["SW1/SW2"] --> L["Knob layer"]
```

## OLED

Main screen shows active layer and key parameters with units. Touch zoom appears
when a knob changes and times out automatically.

```mermaid
flowchart TD
    MAIN["Main value page"] --> TOUCH["Parameter touched"]
    TOUCH --> ZOOM["Zoom: name, value, unit, bar"]
    ZOOM --> MAIN
```
