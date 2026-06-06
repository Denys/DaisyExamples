# Controls Report - Field_delay_daisy-multifx-pedal

## Behavior

Tape-style SDRAM delay with flutter, tone, grit, feedback limiting, freeze, and
MIDI/B-row test tone input.

```mermaid
flowchart LR
    IN["Audio In + MIDI test tone"] --> DRIVE["Input drive"]
    DRIVE --> DELAY["SDRAM tape delay"]
    DELAY --> TONE["Feedback tone + grit"]
    TONE --> DELAY
    DELAY --> MIX["Dry/Wet mix"]
    IN --> MIX
    MIX --> OUT["Audio Out L/R"]
```

## Knob Layers

Knobs use movement-gated "until touched" layers. Holding `SW1` or `SW2` changes
which parameter the knob targets. A parameter is not written merely because the
layer changed; it is written only after that physical knob moves in the active
layer.

```mermaid
flowchart TD
    BASE["No switch held: Base layer"] --> K["Move knob"]
    SW1["Hold SW1: Shift 1 layer"] --> K
    SW2["Hold SW2: Shift 2 layer"] --> K
    K --> TOUCH["Movement exceeds touch threshold"]
    TOUCH --> WRITE["Write only active-layer parameter"]
```

| Knob | Base | Hold SW1 | Hold SW2 |
|---|---|---|---|
| K1 | Mix | Pre Delay | Range |
| K2 | Delay Time ms | Width | Density |
| K3 | Feedback % | Spread | Low Cut Hz |
| K4 | Tone % | Damping | High Cut Hz |
| K5 | Grit % | Rhythm | Smear |
| K6 | Mod % | Freeze Amt | Warp |
| K7 | Input Drive dB | MIDI Level | MIDI Attack ms |
| K8 | Output dB | Tempo BPM | MIDI Release ms |

## Keys And Switches

| Control | Function |
|---|---|
| SW1 | Hold for shift layer 1 |
| SW2 | Hold for shift layer 2 |
| A1 | Bypass state: off active, blink wet-only, on bypass |
| A2 | Freeze state: off live write, blink soft freeze, on hard freeze |
| A3 | Reverse/grain accent |
| A4 | Sync ratio: free, dotted, triplet |
| A5 | Diffuse/spread boost |
| A6 | MIDI test synth waveform |
| A7 | Octave down |
| A8 | Octave up |
| B1-B8 | White keys C4 D4 E4 F4 G4 A4 B4 C5, shifted by A7/A8 |

```mermaid
flowchart LR
    MIDI["External MIDI keyboard"] --> TONE["Internal test tone"]
    BROW["B1-B8 white keys"] --> TONE
    AROW["A1-A6 three-state modes"] --> DSP["Delay behavior"]
    SW["SW1/SW2 held"] --> LAYERS["Knob layer selection"]
    TONE --> DSP
```

## OLED

The OLED shows the active layer, octave offset, and main parameter values with
units. Moving a knob opens a short zoom view with the changed parameter, value,
unit, and bar.

```mermaid
flowchart TD
    IDLE["Main values"] --> MOVE["Knob moved"]
    MOVE --> ZOOM["1.2 s parameter zoom"]
    ZOOM --> IDLE
```
