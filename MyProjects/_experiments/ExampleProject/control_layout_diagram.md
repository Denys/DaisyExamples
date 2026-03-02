# Daisy Field FM-Granular Step Sequencer - Complete Control Layout

## System Overview
This is a dual-voice 8-step sequencer combining FM synthesis, granular oscillators, and real-time parameter control through the Daisy Field hardware interface.

## Hardware Layout

### Front Panel Controls

```
┌─────────────────────────────────────────────────────────────┐
│                    DAISY FIELD INTERFACE                    │
├─────────────────────────────────────────────────────────────┤
│  [KNOB1] [KNOB2] [KNOB3] [KNOB4] [KNOB5] [KNOB6] [KNOB7] [KNOB8] │
│    │      │      │      │      │      │      │      │      │
│    │      │      │      │      │      │      │      │      │
│  [A1]   [A2]   [A3]   [A4]   [A5]   [A6]   [A7]   [A8]    │ TOP ROW
│  [B1]   [B2]   [B3]   [B4]   [B5]   [B6]   [B7]   [B8]    │ BOTTOM ROW
│    │      │      │      │      │      │      │      │      │
│           [SW1] [SW2]                                     │ SWITCHES
└─────────────────────────────────────────────────────────────┘
```

## Control Modes

### MODE_PLAY (Default)
- **Knob 1**: Tempo (60-240 BPM)
- **Knob 2**: Swing (0.0-0.5)
- **Knob 3**: FM Index (0-10)
- **Knob 4**: Filter Cutoff (100-10,000 Hz)
- **Knob 5**: Filter Resonance (0-0.95)
- **Knob 6**: Formant Frequency (200-5000 Hz)
- **Knob 7**: Overdrive Drive (0-1.0)
- **Knob 8**: Reverb Feedback (0-0.8)

### MODE_RECORD
- **Knob 1-8**: Same as PLAY mode
- **Top Row Keys (A1-A8)**: Set step frequencies (C3-C4 scale)
- **Bottom Row Keys**: Control functions

### MODE_EDIT
- **Knob 0**: Step frequency transpose (±2 octaves)
- **Knob 1**: Step velocity (0-1.0)
- **Knob 2**: Gate length (0.1-1.0)
- **Knob 3**: Step active toggle (>0.5 = active)
- **Top Row Keys**: Select step for editing (A1-A8)

## Key Functions

### Top Row Keys (A1-A8)
- **PLAY Mode**: No function (or step selection display)
- **RECORD Mode**: Program step frequencies (A1=C3, A2=D3, etc.)
- **EDIT Mode**: Select step for parameter editing

### Bottom Row Keys (B1-B8)
- **B1**: START/STOP sequencer
- **B2**: RESET to step 1
- **B3**: Toggle RECORD mode
- **B4**: Clear selected step
- **B5-B8**: Unused (reserved)

## Audio Signal Flow

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Voice 1   │    │   Voice 2   │    │    Mix      │
│             │    │             │    │             │
│ FM2 Osc ────┼────┼─ FM2 Osc ───┼────┤  (50/50)   │
│      │      │    │      │      │    │      │      │
│ Grainlet ───┼────┼─ Grainlet ──┤    │      │      │
│      │      │    │      │      │    │      │      │
│  Mix 50/50  │    │  Mix 50/50  │    │      │      │
│      │      │    │      │      │    │      │      │
│  MoogLadder │    │  MoogLadder │    │      │      │
│      │      │    │      │      │    │      │      │
│    AdEnv    │    │    AdEnv    │    │      │      │
└──────┼──────┘    └──────┼──────┘    │      │      │
       │                   │           │      │      │
       └───────────────────┼───────────┤      │      │
                           │           │      │      │
                    ┌──────▼──────┐    │      │      │
                    │  Overdrive  │◄───┘      │      │
                    └──────┬──────┘           │      │
                           │                  │      │
                    ┌──────▼──────┐           │      │
                    │   Reverb    │◄──────────┘      │
                    │   (Stereo)  │                  │
                    └──────┬──────┘                  │
                           │                         │
                    ┌──────▼──────┐                  │
                    │   OUTPUT    │◄─────────────────┘
                    │ L/R Stereo  │
                    └─────────────┘
```

## DSP Module Details

### Voice Architecture
Each voice contains:
- **Fm2**: 2-operator FM synthesis with ratio and index control
- **GrainletOscillator**: Granular synthesis with formant and shape control
- **MoogLadder**: 4-pole ladder filter with cutoff and resonance
- **AdEnv**: Attack-Decay envelope (trigger-based)

### Effects Chain
1. **Overdrive**: Soft clipping distortion (0-1 drive)
2. **ReverbSc**: Stereo reverb with feedback control

## Sequencer Operation

### Timing System
- **Metro Clock**: Handles step timing based on BPM
- **Gate Timer**: Controls note length within each step
- **Step Counter**: 8-step循环 (0-7)

### Step Data Structure
```c
struct Step {
    float frequency;      // Note frequency
    float velocity;       // 0.0-1.0
    bool active;          // Step enabled/disabled
    float gate_length;    // 0.1-1.0 (gate duration)
};
```

### Trigger Logic
1. **Clock Tick**: Advance to next step, trigger active steps
2. **FM/Granular**: Set frequencies based on step data
3. **Envelope**: Trigger AD envelopes with velocity scaling
4. **Gate Control**: Turn off gate after gate_length fraction

## LED Feedback System

### Top Row LEDs (A1-A8)
- **PLAY Mode**: Current step (bright), active steps (dim)
- **RECORD Mode**: Active steps (dim), pressed keys (bright)
- **EDIT Mode**: Selected step (bright), active steps (dim)

### Bottom Row LEDs (B1-B8)
- **B1**: Sequencer running indicator
- **B2**: Reset indicator
- **B3**: Record mode indicator
- **B4**: Clear function indicator

## Control Parameter Ranges

| Parameter | Range | Function |
|-----------|-------|----------|
| FM Ratio | 0.5-8.0 | FM operator frequency ratio |
| FM Index | 0-10 | FM modulation depth |
| Formant | 200-5000 Hz | Granular formant frequency |
| Grain Shape | 0-1.0 | Granular oscillator shape |
| Cutoff | 100-10,000 Hz | Filter cutoff frequency |
| Resonance | 0-0.95 | Filter resonance |
| Drive | 0-1.0 | Overdrive amount |
| Reverb Feedback | 0-0.8 | Reverb feedback level |
| Tempo | 60-240 BPM | Sequencer tempo |
| Swing | 0-0.5 | Swing amount |

## Audio Specifications
- **Sample Rate**: 48 kHz
- **Block Size**: 48 samples
- **Bit Depth**: 32-bit float
- **Output**: Stereo (Left/Right)
- **Latency**: ~1ms (48-sample blocks)

## Mode Switching
1. **POWER ON**: Default to PLAY mode
2. **B3 Key**: Toggle PLAY ↔ RECORD modes
3. **Mode Selection**: Affects knob and key functionality
4. **State Persistence**: Maintains step data across mode changes

This layout provides complete control over a sophisticated FM-granular synthesis system with real-time parameter modulation and step sequencing capabilities.