# Retro Step Sequencer for Daisy Field

**Ported from**: Arduino for Musicians by Brent Edstrom (Chapter 15)  
**Platform**: Electrosmith Daisy Field  
**Version**: 1.0.0  
**Date**: 2026-01-19

A classic 8-step analog-style sequencer with per-step pitch, velocity, and duration controls. Features pentatonic scale quantization, multiple waveforms, resonant Moog filter with envelope modulation, and USB MIDI output.

---

## Quick Start Guide

### First Boot

1. **Power On**: Connect Daisy Field via USB or Eurorack power
2. **Default State**: Sequencer starts stopped with an ascending 8-note pattern
3. **Start Playback**: Press **SW1** (left button)

### 30-Second Demo

1. Press **SW1** → Playback starts, ascending C pentatonic arpeggio plays
2. Tap **B-row keys (B1-B8)** → Toggle steps on/off
3. Hold **A-row key + turn Knob 1** → Adjust that step's pitch
4. Turn **Knob 1** → Adjust tempo (when not holding A-key)
5. Press **SW2** → Cycle through waveforms (SIN→TRI→SAW→SQR...)
6. Turn **Knob 3** → Adjust filter cutoff

---

## Control Mapping

### Hardware Layout

```
┌─────────────────────────────────────────────────────────────┐
│  DAISY FIELD - RETRO STEP SEQUENCER                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐
│  │ K1  │ │ K2  │ │ K3  │ │ K4  │ │ K5  │ │ K6  │ │ K7  │ │ K8  │
│  │Tempo│ │F.Env│ │ Flt │ │ Res │ │ LFO │ │L.Amt│ │ Atk │ │Decay│
│  └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘ └─────┘
│                                                             │
│  [SW1: PLAY/STOP]              [OLED]              [SW2: WAVE]
│                                                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ A1 │ A2 │ A3 │ A4 │ A5 │ A6 │ A7 │ A8 │  ← PITCH/SELECT │
│  └────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────┐ │
│  │ B1 │ B2 │ B3 │ B4 │ B5 │ B6 │ B7 │ B8 │  ← TOGGLE ON/OFF│
│  └────────────────────────────────────────────────────────┘ │
│                                                             │
│  Gate In: External trigger input                            │
│  CV 1-4: Reserved for future use                            │
└─────────────────────────────────────────────────────────────┘
```

### Play Mode Controls

| Control | Function | Range |
|---------|----------|-------|
| **SW1** | Play/Stop | Toggle |
| **SW2** | Cycle waveform | SIN→TRI→SAW→SQR→PBTRI→PBSAW |
| **Knob 1** | Tempo | 40-240 BPM |
| **Knob 2** | Filter Env Amount | 0-100% |
| **Knob 3** | Filter Cutoff | 100 Hz - 10 kHz |
| **Knob 4** | Filter Resonance | 0-100% |
| **Knob 5** | LFO Rate | 0.1-20 Hz |
| **Knob 6** | LFO Amount | 0-100% |
| **Knob 7** | Amp Attack | 1-500 ms |
| **Knob 8** | Amp Decay | 50 ms - 2 s |
| **Keys B1-B8** | Toggle step on/off | - |

### Edit Mode Controls (Hold A-row key)

| Control | Function | Range |
|---------|----------|-------|
| **A-row key held** | Select step to edit | Step 1-8 |
| **Knob 1** | Step pitch | Low → High (pentatonic) |
| **Knob 2** | Step velocity | 0-100% |
| **Knob 3** | Step gate length | 10-100% of step |

### LED Indicators

| LED | Meaning |
|-----|---------|
| **A-row LEDs** | Step position (bright = current, dim = active) |
| **B-row LEDs** | Step active status (on = active) |
| **Knob LEDs** | Current knob values |
| **SW1 LED** | Playing indicator |
| **SW2 LED** | Edit mode active |

---

## Sound Design

### Signal Flow

```
┌──────────┐    ┌──────────┐
│   LFO    │───▶│  (Future │
│ (Sine)   │    │   Mod)   │
└──────────┘    └──────────┘

┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│  Metro   │───▶│ Trigger  │───▶│  AD Env  │───▶│   VCA    │
│ (Tempo)  │    │          │    │ (Amp)    │    │          │
└──────────┘    └──────────┘    └──────────┘    └────┬─────┘
                                                     │
┌──────────┐    ┌──────────┐    ┌──────────┐         │
│  Step    │───▶│Oscillator│───▶│  Moog    │─────────┘
│  Data    │    │  (SAW)   │    │  Filter  │◀────┐
└──────────┘    └──────────┘    └──────────┘     │
                                             ┌───┴────┐
                                             │ AD Env │
                                             │(Filter)│
                                             └────────┘
```

### Waveforms

| # | Name | Character |
|---|------|-----------|
| 0 | SIN | Pure, soft, organ-like |
| 1 | TRI | Bright but mellow |
| 2 | SAW | Rich harmonics, classic synth |
| 3 | SQR | Hollow, woodwind-like |
| 4 | PBTRI | Band-limited triangle |
| 5 | PBSAW | Band-limited saw |

### Pentatonic Scale

Notes are quantized to C major pentatonic:
- **Scale**: C, D, E, G, A
- **Root**: C3 (MIDI 48)
- **Range**: 2+ octaves

---

## Presets

### Preset 1: Classic Arpeggio
- **Tempo**: 120 BPM
- **Waveform**: SAW (2)
- **All steps active**, ascending pitch
- **Filter**: 5000 Hz, 40% resonance
- **Filter Env**: 50%
- **Attack**: 10 ms, Decay: 300 ms

### Preset 2: Acid Bass
- **Tempo**: 130 BPM
- **Waveform**: SAW (2)
- **Steps 4 muted**
- **Filter**: 800 Hz, 85% resonance
- **Filter Env**: 90%
- **Attack**: 1 ms, Decay: 150 ms

### Preset 3: Ambient Pad
- **Tempo**: 40 BPM
- **Waveform**: SIN (0)
- **All steps active**, varied pitch
- **Filter**: 2000 Hz, 20% resonance
- **Filter Env**: 20%
- **LFO Rate**: 0.3 Hz, Amount: 30%
- **Attack**: 200 ms, Decay: 2000 ms

---

## MIDI Output

USB MIDI output on Channel 1:
- Note On/Off messages for each step
- Velocity from step settings
- Can drive external synths

---

## Technical Specifications

| Parameter | Value |
|-----------|-------|
| Steps | 8 |
| Pitch Range | 2+ octaves (C3-C5) |
| Scale | C major pentatonic |
| Tempo Range | 40-240 BPM |
| Sample Rate | 48 kHz |
| Block Size | 48 samples |
| Waveforms | 6 |
| Filter | Moog Ladder (24dB/oct) |
| Envelopes | 2× AD |
| MIDI | USB (Ch.1) |

---

## Building

```bash
cd DaisyExamples/MyProjects/_projects/RetroStepSequencer_Field
make clean && make
make program-dfu  # With Daisy in DFU mode
```

---

## Credits

- **Original**: Brent Edstrom, "Arduino for Musicians" (2016)
- **Daisy Port**: DVPE Project, 2026
