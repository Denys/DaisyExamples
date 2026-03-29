# Field_AdditiveSynth Architecture & Patch Examples

## 1. Block Diagram (System Architecture)

```mermaid
block-beta
  columns 3
  MIDI["MIDI In\n(TRS Jack)"]:1 CPU["Daisy Field\nCortex-M7"]:1 AUDIO["Audio Out\nL+R"]:1
  KNOBS["K1–K8\n(Page A: Rolloff/Even/Odd/ADSR/Rev)\n(Page B: LFO/Chorus/Volume)"]:1 DSP["DSP Chain\nHarmonicOsc×8 + ADSR×8\nLFO + Chorus + ReverbSc"]:1 OLED["128×64 OLED\nPartial bars + Zoom"]:1
  KEYS["A1–A8: Spectral Presets\nB1–B4: LFO Wave\nB5–B8: Sync/Bypass/Sustain\nSW1/SW2: Page Select"]:1 space:1 space:1
```

## 2. Signal Flow (Audio Path per Voice)

```mermaid
flowchart LR
  classDef audioFill fill:#e1f5fe,stroke:#01579b,stroke-width:2px;
  classDef modFill fill:#f3e5f5,stroke:#4a148c,stroke-width:2px;
  classDef ctrlFill fill:#fff3e0,stroke:#e65100,stroke-width:2px;

  MIDI[MIDI Note In]:::ctrlFill -->|pitch| M2F[MIDI to Freq]:::ctrlFill
  M2F -->|freq| HOSC[HarmonicOscillator<16>]:::audioFill
  MIDI -->|gate| ENV[ADSR]:::modFill
  ENV -->|amp| VCA{VCA}:::audioFill
  HOSC --> VCA
  VCA --> GAIN_M[Master Volume]:::audioFill

  LFO[LFO]:::modFill -->|mod| LFO_TGT{Target: Pitch/Amp}:::modFill
  LFO_TGT -.->|Pitch Vibrato| HOSC
  LFO_TGT -.->|Tremolo| VCA
  
  GAIN_M --> CHO[Chorus]:::audioFill
  CHO --> REV[ReverbSc]:::audioFill
  REV --> OUT[Audio Out L/R]:::audioFill

  %% Parameter Mappings
  K1A[Knob 1A: Rolloff]:::ctrlFill -->|amp_1..16| HOSC
  K2A[Knob 2A: Even]:::ctrlFill -->|amp_even| HOSC
  K3A[Knob 3A: Odd]:::ctrlFill -->|amp_odd| HOSC
  
  K4A[Knobs 4-7A]:::ctrlFill -->|A/D/S/R| ENV
  K8A[Knob 8A]:::ctrlFill -->|Mix| REV
  
  K1B[Knob 1-3B]:::ctrlFill -->|Rate/Dep/Tgt| LFO
  K4B[Knob 4-7B]:::ctrlFill -->|Rate/Dep/Del/Mix| CHO
```

## 3. Interaction Flow (Control Processing)

```mermaid
flowchart TD
  LOOP["Main Loop (System::Delay 1ms)"] --> MIDI_RX["midi.Listen + PopEvents"]
  MIDI_RX --> NOTE{"NoteOn?"}
  NOTE -->|yes| ALLOC["VoiceMgr.NoteOn\nHarmonicOsc.SetFreq\nAdsr gate=true"]
  NOTE -->|NoteOff| RELEASE["VoiceMgr.NoteOff\nAdsr gate=false"]

  LOOP --> PROC["hw.ProcessAllControls"]
  PROC --> SW["ProcessSwitches\nSW1 → Page A\nSW2 → Page B"]
  PROC --> KNOBS["ProcessKnobs\nPickup/catch logic per page"]
  KNOBS --> PAGE_A["Page A: Spectrum / ADSR / Reverb"]
  KNOBS --> PAGE_B["Page B: LFO / Chorus / Master"]
  PAGE_A --> RECALC["RecalcPartials\nNormalize harmonic amplitudes"]
  PAGE_A --> ENVSYNC["SetEnvAll (ADSR)"]

  PROC --> KEYS["ProcessKeys"]
  KEYS --> PRESET["A1-A8: Load Spectral Preset\nLock K1-K3"]
  KEYS --> LFO_WAVE["B1-B4: LFO Waveform (Sin/Tri/Saw/S&H)"]
  KEYS --> TOGGLES["B5: LFO Sync\nB6: Rev bypass\nB7: Cho bypass\nB8: Sustain"]

  LOOP --> DISPLAY["UpdateDisplay + UpdateLEDs"]
```

---

## 4. Patch Examples

### Patch 1: "Glass Pad"
*A slow, ethereal pad with a bell-like harmonic structure and heavy reverberation.*

| Control | Parameter | Value / Position | Comments |
|---------|-----------|------------------|----------|
| **K1 (A)** | Rolloff | `0.4` | Slow rolloff to preserve higher harmonics |
| **K2 (A)** | Even Harms | `0.1` | Very few even harmonics |
| **K3 (A)** | Odd Harms | `0.9` | Rich odd harmonics (bell-like) |
| **K4 (A)** | Attack | `0.7` (~500ms) | Slow, swelling attack |
| **K6 (A)** | Sustain | `1.0` (100%) | Sustains at full volume |
| **K7 (A)** | Release | `0.8` (~2s) | Long, graceful release |
| **K8 (A)** | Reverb Mix | `0.7` (70%) | Heavy, washing reverberation |
| **K4 (B)** | Chorus Rate | `0.2` (~0.5 Hz) | Very slow, subtle chorus movement |

### Patch 2: "Tremolo Organ"
*A classic tonewheel organ sound with a fast amplitude tremolo.*

| Control | Parameter | Value / Position | Comments |
|---------|-----------|------------------|----------|
| **Key A7** | Preset: Organ | `Active` | Sets flat/even drawbar spectrum |
| **K4 (A)** | Attack | `0.0` (1ms) | Instant attack |
| **K6 (A)** | Sustain | `1.0` (100%) | Full sustain while key held |
| **K7 (A)** | Release | `0.1` (~10ms) | Fast release, similar to real organ |
| **K1 (B)** | LFO Rate | `0.6` (~5 Hz) | Fast rotating speaker speed |
| **K2 (B)** | LFO Depth | `0.8` (80%) | Deep modulation |
| **K3 (B)** | LFO Target | `1.0` (Amp) | Targets amplitude (Tremolo) |
| **K7 (B)** | Chorus Mix | `0.5` (50%) | Adds Leslie-like spatial widening |

### Patch 3: "Wobble Bass"
*An aggressive, low-end focused patch with a synchronized LFO modulating pitch.*

| Control | Parameter | Value / Position | Comments |
|---------|-----------|------------------|----------|
| **Key A3** | Preset: Saw | `Active` | Full 1/n harmonic spectrum |
| **K4 (A)** | Attack | `0.0` (1ms) | Snappy attack |
| **K5 (A)** | Decay | `0.4` (~150ms) | Short, punchy decay |
| **K6 (A)** | Sustain | `0.0` (0%) | Plucky envelope, no sustain |
| **K1 (B)** | LFO Rate | `0.7` (~8 Hz) | Fast wobble |
| **K2 (B)** | LFO Depth | `0.5` (±1 st) | 1 semitone pitch modulation |
| **K3 (B)** | LFO Target | `0.0` (Pitch) | Targets pitch (Vibrato) |
| **Key B5** | LFO Sync | `Active` | LFO resets phase on NoteOn |

### Patch 4: "Spacy Chimes"
*Randomized, aggressive pitch modulation on a pure sine wave, washed in delay and reverb.*

| Control | Parameter | Value / Position | Comments |
|---------|-----------|------------------|----------|
| **Key A1** | Preset: Sine | `Active` | Pure fundamental only |
| **Key B4** | LFO Wave | `Active` | Sample & Hold (Random) waveform |
| **K1 (B)** | LFO Rate | `0.8` (~12 Hz) | Fast, chaotic modulation |
| **K2 (B)** | LFO Depth | `1.0` (±2 st) | Maximum pitch jump depth |
| **K3 (B)** | LFO Target | `0.0` (Pitch) | Targets pitch |
| **K8 (A)** | Reverb Mix | `0.85` (85%) | Drenches the chimes in reverb |
| **K6 (B)** | Cho Delay | `0.9` (90%) | Pushes chorus into delay territory |
