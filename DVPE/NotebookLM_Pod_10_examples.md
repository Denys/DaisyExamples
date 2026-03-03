<!-- markdownlint-disable MD024 -->
# NotebookLM Pod — 10 DVPE Examples

Each project has **3 mandatory Mermaid diagrams**:

- **A. Block Diagram** (`block-beta`) — System architecture
- **B. Audio Flow** (`flowchart LR`) — Signal path source → output
- **C. Control Flow** (`flowchart TD`) — How Pod encoder/knobs update DSP

DVPE block IDs match `BlockRegistry.ts`. `*` = LGPL (`USE_DAISYSP_LGPL = 1`).

---

## Project 1: Auto-Pan & Tremolo Modulator

- **Difficulty:** 2/10
- **Concept:** A foundational amplitude and spatial modulation effect. A low-frequency oscillator modulates the overall volume (tremolo) and the left/right balance (auto-pan).
- **DVPE Architecture:** `audio_input` → `tremolo` block. An `lfo` modulates the amplitude, and a second `lfo` (offset by 90 degrees) controls a `pan` block.
- **Pod FSM Mapping:**
  - _Page 1 (Cyan — Tremolo):_ Knob 1 = LFO Rate, Knob 2 = Tremolo Depth.
  - _Page 2 (Magenta — Panning):_ Knob 1 = Pan Rate, Knob 2 = Stereo Width.

### A. System Architecture

```mermaid
block-beta
  columns 4
  IN["Audio In (Stereo)"]
  TREM["daisysp::Tremolo"]
  PAN["Violet Pan Block"]
  OUT["Audio Out (Stereo)"]

  IN --> TREM
  TREM --> PAN
  PAN --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Tremolo[Blue Tremolo]
  Tremolo --> Pan[Violet Pan]
  Pan --> Out([Green Audio Out])

  LFO1((Orange LFO 1)) -.->|Amplitude Mod| Tremolo
  LFO2((Orange LFO 2)) -.->|Spatial Mod| Pan
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Cyan| P1[Tremolo Mode]
  FSM -->|Page 2: Magenta| P2[Panning Mode]

  P1 --> K1_P1[Knob 1] -->|SetFreq| LFO1(Tremolo LFO Rate)
  P1 --> K2_P1[Knob 2] -->|SetDepth| Tremolo(Tremolo Depth)

  P2 --> K1_P2[Knob 1] -->|SetFreq| LFO2(Pan LFO Rate)
  P2 --> K2_P2[Knob 2] -->|SetDepth| Pan(Stereo Width)
```

---

## Project 2: Asymmetrical Fuzz & Tube Overdrive

- **Difficulty:** 3/10
- **Concept:** A nonlinear distortion effect simulating a Class-A vacuum tube preamp and an analog "Fuzz Face" pedal via asymmetrical soft/hard clipping.
- **DVPE Architecture:** `audio_input` → `overdrive` (soft clipping) → `fold` (wavefolding) → `tone` (lowpass filter to remove harsh upper harmonics).
- **Pod FSM Mapping:**
  - _Page 1 (Red — Drive):_ Knob 1 = Input Gain (Drive), Knob 2 = Tone (Cutoff).
  - _Page 2 (Orange — Shape):_ Knob 1 = Asymmetry, Knob 2 = Output Trim.

### A. System Architecture

```mermaid
block-beta
  columns 5
  IN["Audio In"]
  DRV["daisysp::Overdrive"]
  FLD["daisysp::Fold"]
  FLT["daisysp::Tone"]
  OUT["Audio Out"]

  IN --> DRV
  DRV --> FLD
  FLD --> FLT
  FLT --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Drive[Blue Overdrive]
  Drive --> Fold[Blue Wavefolder]
  Fold --> Tone[Blue Tone LPF]
  Tone --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Red| P1[Drive & Tone]
  FSM -->|Page 2: Orange| P2[Shape & Trim]

  P1 --> K1_P1[Knob 1] -->|SetDrive| Drive(Input Gain/Drive)
  P1 --> K2_P1[Knob 2] -->|SetFreq| Tone(Tone Cutoff)

  P2 --> K1_P2[Knob 1] -->|SetOffset| Fold(Asymmetry/Offset)
  P2 --> K2_P2[Knob 2] -->|Gain multiplier| VCA(Output Trim)
```

---

## Project 3: The Lo-Fi Digital Degrader

- **Difficulty:** 4/10
- **Concept:** Intentional destruction of the audio signal using quantization noise (bit-crushing) and sample-rate reduction (aliasing), popular in modern electronic production.
- **DVPE Architecture:** `audio_input` → `decimator` → `svf` (State Variable Filter to sweep through the resulting digital artifacts).
- **Pod FSM Mapping:**
  - _Page 1 (Blue — Degrade):_ Knob 1 = Sample Rate (Downsample), Knob 2 = Bit Depth.
  - _Page 2 (Yellow — Filter):_ Knob 1 = Filter Cutoff, Knob 2 = Resonance.

### A. System Architecture

```mermaid
block-beta
  columns 4
  IN["Audio In"]
  DEC["daisysp::Decimator"]
  SVF["daisysp::Svf"]
  OUT["Audio Out"]

  IN --> DEC
  DEC --> SVF
  SVF --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Crush[Blue Decimator]
  Crush --> Filter[Blue Svf]
  Filter --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Blue| P1[Degrade Mode]
  FSM -->|Page 2: Yellow| P2[Filter Mode]

  P1 --> K1_P1[Knob 1] -->|SetDownsampleFactor| Crush(Sample Rate)
  P1 --> K2_P1[Knob 2] -->|SetBitsToCrush| Crush(Bit Depth)

  P2 --> K1_P2[Knob 1] -->|SetFreq| Filter(SVF Cutoff)
  P2 --> K2_P2[Knob 2] -->|SetRes| Filter(SVF Resonance)
```

---

## Project 4: Studio Dynamics Compressor / Expander

- **Difficulty:** 5/10
- **Concept:** Algorithmic dynamic range control. This uses envelope followers to automatically attenuate peaks (compression) or silence noise floors (expanding/gating).
- **DVPE Architecture:** `audio_input` splits. Path A goes to `envelope_follower` → Violet Logic (calculating gain reduction). Path B goes to `vca`. The Logic block modulates the VCA.
- **Pod FSM Mapping:**
  - _Page 1 (White — Dynamics):_ Knob 1 = Threshold, Knob 2 = Ratio.
  - _Page 2 (Green — Time):_ Knob 1 = Attack Time, Knob 2 = Release Time.

### A. System Architecture

```mermaid
block-beta
  columns 5
  IN["Audio In"]
  ENV["Envelope Follower"]
  LOG["Logic/Gain Calc"]
  VCA["daisysp::VCA"]
  OUT["Audio Out"]

  IN --> ENV
  ENV --> LOG
  LOG --> VCA
  VCA --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Split{Splitter}
  Split --> VCA[Blue VCA]
  Split --> Follower[Orange Env Follower]

  Follower --> Logic[Violet Gain Computer]
  Logic -.->|Gain Reduction CV| VCA

  VCA --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: White| P1[Dynamics]
  FSM -->|Page 2: Green| P2[Time Constants]

  P1 --> K1_P1[Knob 1] -->|Threshold logic| Logic(Threshold)
  P1 --> K2_P1[Knob 2] -->|Ratio math| Logic(Ratio)

  P2 --> K1_P2[Knob 1] -->|SetAttack| Follower(Attack Time)
  P2 --> K2_P2[Knob 2] -->|SetRelease| Follower(Release Time)
```

---

## Project 5: VOSIM Formant "Speaking" Voice

- **Difficulty:** 5/10
- **Concept:** Voice Simulation (VOSIM) synthesis that models the human vocal tract using squared sine wave pulses to create distinct vowel formants.
- **DVPE Architecture:** `midi_note` → `vosim_oscillator`. An `lfo` is lightly applied to the fundamental pitch to create natural human vibrato.
- **Pod FSM Mapping:**
  - _Page 1 (Cyan — Tract):_ Knob 1 = Formant 1 Freq (Vowel A), Knob 2 = Formant 2 Freq (Vowel B).
  - _Page 2 (Magenta — Breath):_ Knob 1 = Vibrato Depth, Knob 2 = Noise Blend (Aspiration).

### A. System Architecture

```mermaid
block-beta
  columns 3
  MIDI["MIDI Input"]
  VOSIM["daisysp::VosimOscillator"]
  OUT["Audio Out"]

  MIDI --> VOSIM
  VOSIM --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  MIDI([Green MIDI Note]) --> Pitch(Pitch CV)
  Pitch --> Vosim[Blue VosimOscillator]
  LFO((Orange LFO)) -.->|Pitch Mod| Vosim
  Vosim --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Cyan| P1[Vocal Tract]
  FSM -->|Page 2: Magenta| P2[Breath & Vibrato]

  P1 --> K1_P1[Knob 1] -->|SetForm1Freq| Vosim(Formant 1 Freq)
  P1 --> K2_P1[Knob 2] -->|SetForm2Freq| Vosim(Formant 2 Freq)

  P2 --> K1_P2[Knob 1] -->|SetAmp| LFO(Vibrato Depth)
  P2 --> K2_P2[Knob 2] -->|Blend| Noise(Aspiration/Noise Mix)
```

---

## Project 6: Spectral Pitch-Shifter / Harmonizer

- **Difficulty:** 6/10
- **Concept:** Shifting the pitch of an incoming audio signal without changing its duration, used to create artificial harmonies or octaves.
- **DVPE Architecture:** `audio_input` → `pitch_shifter` (using granular or phase-vocoder techniques) → `delay_line`.
- **Pod FSM Mapping:**
  - _Page 1 (Yellow — Harmony):_ Knob 1 = Pitch Shift Interval (-12 to +12 semitones), Knob 2 = Dry/Wet Mix.
  - _Page 2 (Purple — Space):_ Knob 1 = Delay Time, Knob 2 = Delay Feedback.

### A. System Architecture

```mermaid
block-beta
  columns 4
  IN["Audio In"]
  PITCH["daisysp::PitchShifter"]
  DLY["daisysp::DelayLine"]
  OUT["Audio Out"]

  IN --> PITCH
  PITCH --> DLY
  DLY --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Split{Dry/Wet Split}
  Split --> Dry[Dry Path]
  Split --> Pitch[Blue PitchShifter]
  Pitch --> Delay[Blue DelayLine]

  Dry --> Mix((Violet Mixer))
  Delay --> Mix
  Mix --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Yellow| P1[Harmony]
  FSM -->|Page 2: Purple| P2[Space / Echo]

  P1 --> K1_P1[Knob 1] -->|SetTransposition| Pitch(Interval -12 to +12)
  P1 --> K2_P1[Knob 2] -->|Crossfade| Mix(Dry/Wet Mix)

  P2 --> K1_P2[Knob 1] -->|SetDelay| Delay(Delay Time)
  P2 --> K2_P2[Knob 2] -->|Audio feedback| Delay(Delay Feedback)
```

---

## Project 7: Modal Resonator (Acoustic Material Modeler)

- **Difficulty:** 7/10
- **Concept:** Simulating the physics of struck objects (wood, glass, metal membranes) by running an excitation signal through a finely tuned bank of bandpass filters.
- **DVPE Architecture:** `audio_input` (acting as the exciter) → `modal_voice`* (Resonator Filter Bank).
- **Pod FSM Mapping:**
  - _Page 1 (Orange — Material):_ Knob 1 = Structure (Inharmonicity), Knob 2 = Brightness.
  - _Page 2 (Cyan — Physics):_ Knob 1 = Damping (Decay), Knob 2 = Fundamental Pitch.

### A. System Architecture

```mermaid
block-beta
  columns 3
  IN["Audio In / Exciter"]
  MODAL["daisysp::ModalVoice*"]
  OUT["Audio Out"]

  IN --> MODAL
  MODAL --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Burst[Audio Transient / Exciter]
  Burst --> Modal[Blue ModalVoice Filterbank]
  Modal --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Orange| P1[Material Properties]
  FSM -->|Page 2: Cyan| P2[Physics]

  P1 --> K1_P1[Knob 1] -->|SetStructure| Modal(Structure / Inharmonicity)
  P1 --> K2_P1[Knob 2] -->|SetBrightness| Modal(Brightness)

  P2 --> K1_P2[Knob 1] -->|SetDamping| Modal(Damping / Decay)
  P2 --> K2_P2[Knob 2] -->|SetFreq| Modal(Fundamental Pitch)
```

---

## Project 8: Stochastic Particle Texturizer

- **Difficulty:** 8/10
- **Concept:** Generating chaotic, bubbling, or crackling textures using random impulse trains. Creates "clouds" of sound rather than traditional notes.
- **DVPE Architecture:** `metro` (clock) triggers a `particle` module. The output feeds into `reverb_sc`* to smear the clicks into an ambient bed.
- **Pod FSM Mapping:**
  - _Page 1 (Red — Cloud):_ Knob 1 = Particle Density (Rate of clicks), Knob 2 = Pitch Randomization.
  - _Page 2 (Blue — Filter):_ Knob 1 = Resonant Filter Cutoff, Knob 2 = Q (Resonance).
  - _Page 3 (White — Space):_ Knob 1 = Reverb Size, Knob 2 = Reverb Mix.

### A. System Architecture

```mermaid
block-beta
  columns 4
  CLK["daisysp::Metro"]
  PART["daisysp::Particle"]
  VERB["daisysp::ReverbSc*"]
  OUT["Audio Out"]

  CLK --> PART
  PART --> VERB
  VERB --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  Clock((Violet Clock)) -->|Trigger| Particle[Blue Particle Engine]
  Particle --> Reverb[Blue ReverbSc]
  Reverb --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Red| P1[Cloud Density]
  FSM -->|Page 2: Blue| P2[Particle Filter]
  FSM -->|Page 3: White| P3[Ambient Space]

  P1 --> K1_P1[Knob 1] -->|SetDensity| Particle(Rate of Clicks)
  P1 --> K2_P1[Knob 2] -->|SetSpread| Particle(Pitch Randomization)

  P2 --> K1_P2[Knob 1] -->|SetFreq| Particle(Resonant Cutoff)
  P2 --> K2_P2[Knob 2] -->|SetRes| Particle(Resonance Q)

  P3 --> K1_P3[Knob 1] -->|SetFeedback| Reverb(Reverb Size)
  P3 --> K2_P3[Knob 2] -->|Wet/Dry| Mixer(Reverb Mix)
```

---

## Project 9: Feedback Delay Network (FDN) Reverb

- **Difficulty:** 8/10
- **Concept:** Instead of a pre-packaged reverb block, this project builds a custom algorithmic room simulator from scratch using a matrix of delay lines that feed into each other.
- **DVPE Architecture:** `audio_input` splits into 4 `delay_line` blocks. The outputs are routed through Violet Multipliers (attenuators) and crossed back into the inputs of the other delay lines.
- **Pod FSM Mapping:**
  - _Page 1 (Cyan — Size):_ Knob 1 = Master Delay Time, Knob 2 = Matrix Feedback (Decay).
  - _Page 2 (Yellow — Tone):_ Knob 1 = Lowpass Damping, Knob 2 = Highpass Filter.

### A. System Architecture

```mermaid
block-beta
  columns 5
  IN["Audio In"]
  DLYS["4x DelayLines"]
  MTX["Feedback Matrix"]
  FLT["Filters"]
  OUT["Audio Out"]

  IN --> DLYS
  DLYS --> MTX
  MTX --> FLT
  FLT --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  In([Green Audio In]) --> Split{Signal Splitter}
  Split --> DL1[Blue DelayLine 1]
  Split --> DL2[Blue DelayLine 2]
  Split --> DL3[Blue DelayLine 3]
  Split --> DL4[Blue DelayLine 4]

  DL1 & DL2 & DL3 & DL4 --> Matrix[Violet Feedback Matrix / Multipliers]
  Matrix --> LPF[Blue SVF Filters]
  Matrix -.->|Recursive Routing| DL1 & DL2 & DL3 & DL4
  LPF --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Cyan| P1[Room Size]
  FSM -->|Page 2: Yellow| P2[Room Tone]

  P1 --> K1_P1[Knob 1] -->|SetDelay| Delay(Master Delay Time)
  P1 --> K2_P1[Knob 2] -->|Multiply| Matrix(Matrix Feedback Decay)

  P2 --> K1_P2[Knob 1] -->|SetFreq LP| LPF(Lowpass Damping)
  P2 --> K2_P2[Knob 2] -->|SetFreq HP| HPF(Highpass Damping)
```

---

## Project 10: Markov-Chain AI Drummer

- **Difficulty:** 9/10
- **Concept:** Instead of a rigid step sequencer, this uses an artificial intelligence "Agent" (a variable-order Markov Model) to generate ever-evolving, non-looping rhythms based on probability matrices.
- **DVPE Architecture:** `metro` clocks a Violet Markov Chain logic block. The Markov block calculates state transitions and outputs conditional triggers to `analog_bass_drum` and `hihat` modules.
- **Pod FSM Mapping:**
  - _Page 1 (Green — Time):_ Knob 1 = Tempo, Knob 2 = Swing Amount.
  - _Page 2 (Magenta — AI Brain):_ Knob 1 = Kick Probability/Density, Knob 2 = Hat Complexity.
  - _Page 3 (Red — Tone):_ Knob 1 = Kick Decay, Knob 2 = Hat Pitch.

### A. System Architecture

```mermaid
block-beta
  columns 5
  CLK["daisysp::Metro"]
  AI["Markov Chain"]
  SYN["Drum Synths"]
  MIX["daisysp::Mixer"]
  OUT["Audio Out"]

  CLK --> AI
  AI --> SYN
  SYN --> MIX
  MIX --> OUT
```

### B. Audio Flow

```mermaid
flowchart LR
  Clock((Orange Metro)) --> Brain{Violet Markov Logic}

  Brain -.->|Kick Trigger| Kick[Blue AnalogBassDrum]
  Brain -.->|Hat Trigger| Hat[Blue HiHat]

  Kick --> Mix((Violet Mixer))
  Hat --> Mix
  Mix --> Out([Green Audio Out])
```

### C. Control Flow

```mermaid
flowchart TD
  Encoder((Encoder Click)) -->|Cycles| FSM{FSM Page State}

  FSM -->|Page 1: Green| P1[Time & Groove]
  FSM -->|Page 2: Magenta| P2[AI Brain]
  FSM -->|Page 3: Red| P3[Drum Tone]

  P1 --> K1_P1[Knob 1] -->|SetFreq| Clock(Master Tempo)
  P1 --> K2_P1[Knob 2] -->|Time Offset| Clock(Swing Amount)

  P2 --> K1_P2[Knob 1] -->|Matrix Weights| Logic(Kick Probability/Density)
  P2 --> K2_P2[Knob 2] -->|Matrix Weights| Logic(Hat Complexity)

  P3 --> K1_P3[Knob 1] -->|SetDecay| Kick(Kick Decay)
  P3 --> K2_P3[Knob 2] -->|SetFreq| Hat(Hat Pitch)
```

---

_LGPL modules (`*`): `modal_voice`, `reverb_sc`, `string_voice`, `moog_ladder` — require `USE_DAISYSP_LGPL = 1` in Makefile._
