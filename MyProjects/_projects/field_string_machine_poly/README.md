# Field String Machine Poly

## Overview
Field String Machine Poly is an 8-voice polyphonic string machine for the Daisy Field. It uses a 7-part oscillator bank per voice, an ADSR envelope, an optional SVF low‑pass filter, and a stereo chorus for the classic ensemble sound. Notes are triggered via MIDI and the front‑panel controls (knobs, switches, CVs, and keyboard buttons) shape envelope timing, detune spread, chorus, and output level.

## Key Features
- 8‑voice polyphony with per‑voice oscillator bank
- 7‑harmonic string registration (fixed mix)
- ADSR envelope (attack/decay/sustain/release)
- Optional SVF low‑pass filter (on/off)
- Stereo chorus with rate/depth control (on/off)
- CV modulation for Attack, Decay, Detune, and Master Level
- OLED status readout + LED feedback

## Installation & Usage
1. Open a terminal in the project directory:
   ```bash
   cd DaisyExamples/MyProjects/_projects/field_string_machine_poly
   ```
2. Build:
   ```bash
   make
   ```
3. Flash (Daisy Field in DFU mode):
   ```bash
   make program-dfu
   ```
4. Connect a MIDI keyboard to the Field’s MIDI In and play.

## MIDI
- **Note On/Off**: triggers voices and envelope; velocity scales amplitude.
- **MIDI CC**: not assigned in this firmware.

## Control Mapping
**Legend**: Knob values are 0.0–1.0 unless otherwise noted. CV inputs add up to **+0.5** to their target parameter (clamped to 0.0–1.0).

| Control | Mapping | Range | Description |
|---|---|---|---|
| Knob 1 | Attack | 0.001–2.001 s | Envelope attack time. |
| Knob 2 | Decay | 0.001–2.001 s | Envelope decay time. |
| Knob 3 | Sustain | 0.0–1.0 | Envelope sustain level. |
| Knob 4 | Release | 0.001–3.001 s | Envelope release time. |
| Knob 5 | Detune | 0.0–1.0 (spread) | Detune spread across voices (up to ~±7 cents per voice). |
| Knob 6 | Chorus Rate | 0.1–5.1 Hz | Chorus LFO rate (keyboard can extend to 6.0 Hz). |
| Knob 7 | Chorus Depth | 0.0–1.0 | Chorus LFO depth. |
| Knob 8 | Master Level | 0.0–1.0 | Output level scaling. |
| CV 1 | Attack Mod | +0.0–0.5 | Adds to Knob 1 before scaling. |
| CV 2 | Decay Mod | +0.0–0.5 | Adds to Knob 2 before scaling. |
| CV 3 | Detune Mod | +0.0–0.5 | Adds to Knob 5 before scaling. |
| CV 4 | Master Mod | +0.0–0.5 | Adds to Knob 8 before scaling. |
| SW 1 | Filter Enable | Toggle | Toggles SVF low‑pass filter on/off. |
| SW 2 | Chorus Enable | Toggle | Toggles stereo chorus on/off. |
| A1 (Keyboard) | Detune + | +0.02 | Increment detune spread. |
| A2 (Keyboard) | Detune − | −0.02 | Decrement detune spread. |
| A3 (Keyboard) | Chorus Rate + | +0.1 Hz | Increase chorus rate. |
| A4 (Keyboard) | Chorus Rate − | −0.1 Hz | Decrease chorus rate. |
| A5 (Keyboard) | Chorus Depth + | +0.05 | Increase chorus depth. |
| A6 (Keyboard) | Chorus Depth − | −0.05 | Decrease chorus depth. |
| A7/A8 (Keyboard) | — | — | Unused. |
| A9 (Keyboard) | Master + | +0.05 | Increase master level. |
| A10 (Keyboard) | Master − | −0.05 | Decrease master level. |

**Fixed Parameters**
- Filter cutoff: 2000 Hz
- Filter resonance: 0.3
- Chorus wet mix: 35%
- Oscillator registration: 7 fixed harmonic amplitudes

## Signal Flow
```mermaid
flowchart LR
    MIDI[MIDI Note On/Off] --> VM[Voice Manager (8 voices)]
    VM --> OSC[Oscillator Bank (7 harmonics)]
    OSC --> ENV[ADSR Envelope / VCA]
    ENV -->|Filter On| SVF[SVF Low‑Pass Filter]
    ENV -->|Filter Off| MIX[Voice Mix]
    SVF --> MIX
    MIX -->|Chorus On| CH[Chorus]
    MIX -->|Chorus Off| OUT[Audio Out L/R]
    CH --> OUT
```

## Presets
Each preset lists concrete parameter values as set in the control map.

### 1) Vintage Ensemble
- Attack: 0.06 s
- Decay: 0.35 s
- Sustain: 0.80
- Release: 0.70 s
- Detune: 0.35
- Chorus Rate: 0.80 Hz
- Chorus Depth: 0.55
- Master Level: 0.70
- Filter Enable: On
- Chorus Enable: On
- Cutoff: 2000 Hz (fixed)
- Resonance: 0.30 (fixed)

### 2) Slow Warm Pad
- Attack: 1.50 s
- Decay: 1.20 s
- Sustain: 0.90
- Release: 2.50 s
- Detune: 0.25
- Chorus Rate: 0.40 Hz
- Chorus Depth: 0.75
- Master Level: 0.60
- Filter Enable: On
- Chorus Enable: On
- Cutoff: 2000 Hz (fixed)
- Resonance: 0.30 (fixed)

### 3) Bright Pluck
- Attack: 0.01 s
- Decay: 0.20 s
- Sustain: 0.30
- Release: 0.15 s
- Detune: 0.15
- Chorus Rate: 1.60 Hz
- Chorus Depth: 0.35
- Master Level: 0.80
- Filter Enable: Off
- Chorus Enable: On
- Cutoff: 2000 Hz (fixed)
- Resonance: 0.30 (fixed)

## Notes
- Filter cutoff and resonance are currently fixed in firmware; only the filter enable toggle is exposed.
- No MIDI CC mapping is implemented by default.
