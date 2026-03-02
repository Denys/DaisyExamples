# Tube Effect Synth (Daisy Field)

A 24-voice polyphonic synthesizer playing through a vacuum tube simulation effect. This project demonstrates the integration of the DAFX Tube Distortion model into the DaisySP library.

## Controls

| Control | Parameter | Description |
|---------|-----------|-------------|
| **Knob 1** | Filter Cutoff | Controls the low-pass filter frequency for all synth voices (250Hz - 8250Hz) |
| **Knob 5** | Tube Drive | Input gain/drive into the tube capability (0.1 - 50.0) |
| **Knob 6** | Work Point | Biasing of the tube. Center = Pentode, Left = Triode, Right = Asymmetric |
| **Knob 7** | Hardness | Soft-clipping hardness/character |
| **Knob 8** | Mix | Dry/Wet blend between clean synth and tube effect |
| **Switch 1** | Panic | Cut all voices immediately |
| **Keyboard** | Notes | Play synthesizer voices |

### Potential Mappings (Unused)
| Control | Suggested Parameter | Implementation Hint |
|---------|---------------------|---------------------|
| **Knob 2** | Filter Resonance | Add `SetRes` to Voice class |
| **Knob 3** | Amp Envelope Attack/Release | Add `SetRelease` to Voice class |
| **Knob 4** | Oscillator Waveform | Add `SetWaveform` to Voice class (Saw/Sqr variable) |

### Keyboard Suggestions (Currently Unused)
The onboard 16 keys (A1-A8, B1-B8) are currently unmapped. Here are 3 recommended ways to use them:

#### 1. Chromatic Keyboard (Piano Style)
- **B1-B8 (Bottom)**: White Keys (C, D, E, F, G, A, B, C)
- **A1, A2, A4, A5, A6**: Black Keys (C#, D#, F#, G#, A#)
- **Implementation**: Map indices to MIDI note numbers in `main()` loop.

#### 2. Preset Selector
- **Row A**: Select Waveforms (A1=Saw, A2=Sqr, A3=Tri) or Filter Types.
- **Row B**: Instant Chords (Major/Minor/7th) or Drone ON/OFF.

#### 3. Scale Mode
- **Row B**: 8 notes of a selected scale (e.g., C Major Pentatonic).
- **Row A**: Octave Down (A1), Octave Up (A8), Scale Select (A4-A5).

#### 4. "Shift" Mode (Alternative Knob Functions)
Use keys as momentary modifiers to change what the knobs do while held:

- **Hold A1 (Envelope Mode)**:
  - Knob 1 → Attack
  - Knob 2 → Decay
  - Knob 3 → Sustain
  - Knob 4 → Release
- **Hold A2 (LFO Mode)**:
  - Knob 1 → LFO Rate
  - Knob 2 → LFO Depth -> Cutoff
  - Knob 3 → LFO Waveform
- **Hold B1 (Momentary Tube Boost)**:
  - Maximize Tube Drive regardless of Knob 5 position while held.

## Block Diagrams

### Enhanced Signal Flow (with LFO/Envelope)
```
                                     ┌─────────────┐
                                     │   VOICE 1   │
                                  ┌──►[OSC]─►[LFP]─►Amp─┐
                                  │           ▲      ▲  │
                  ┌─────┐         │           │      │  │
             ┌───►│ LFO ├─────────┼───────────┘      │  │
             │    └─────┘         │                  │  │
┌─────────┐  │   ┌────────────┐   │  ┌─────────────┐ │  │   ┌────────────┐
│  MIDI   │  │   │   VOICE    │   │  │   VOICE 24  │ │  │   │    TUBE    │
│  INPUT  ├──┼──►│  MANAGER   ├───┼──►[OSC]─►[LFP]─►Amp─┼───►│   EFFECT   ├─────► OUT
└────┬────┘  │   └─────┬──────┘      └─────────────┘        └─────┬──────┘
     │Key    │Param    │Param                                     │Param
     │       │Rate/Dep │Att/Dec/Sus/Rel                           │Drive/Bias
     │       │         │                                          │
  ┌──┴──┐    │      ┌──▼──┐                                  ┌────▼─────┐
  │ KEY │    └──────┤ MUX │◄────── [HOLD A1/A2] ────────────┤ KNOBS    │
  │ BOARD           └──▲──┘                                  │ 1..8     │
  └─────┘              │                                     └──────────┘
                       │
             ┌─────────┴─────────┐
             │ PHYSICAL CONTROLS │
             └───────────────────┘
```

### Layered Control Flow
This diagram shows how the physical knobs map to different destinations based on the "Shift Key" held.

```
                  ┌──────────────┐
                  │ PHYSICAL KNOBS│
                  └──────┬───────┘
                         │
           ┌─────────────▼─────────────┐
           │  MULTIPLEXER (SHIFT LOGIC)│
           └──────┬─────────────┬──────┘
                  │             │
        [NO KEY HELD]      [HOLD A1 (ENV)]       [HOLD A2 (LFO)]
          (Normal)            (Shift)                (Shift)
            │                    │                      │
    ┌───────┼───────┐    ┌───────┼───────┐      ┌───────┼───────┐
    │       │       │    │       │       │      │       │       │
 K1:Cutoff  │       │  K1:Attack │       │    K1:Rate   │       │
    │    K5:Drive   │    │    K2:Decay   │      │    K2:Depth   │
    │       │    K8:Mix  │       │    K3:Sust   │       │    K3:Wave
    ▼       ▼       ▼    ▼       ▼       ▼      ▼       ▼       ▼
 [FILTER] [TUBE]  [MIX] [VCA ENV] [VCA ENV]  [LFO]──►[FILTER] [LFO]
```

## OLED Visualization
The OLED display provides real-time feedback for all parameters:
- **Idle**: Shows knob assignment helper text.
- **Active**: When a knob is turned, zooms in to show:
  - Parameter Name
  - Value (with Units like Hz, %, or Mode)
  - Progress Bar

## Build Instructions
1. Ensure the global `DaisySP` library is built and includes the `Tube` module.
2. Run `make clean && make`
3. Enter bootloader (Hold BOOT, Press RESET)
4. Run `make program-dfu`
