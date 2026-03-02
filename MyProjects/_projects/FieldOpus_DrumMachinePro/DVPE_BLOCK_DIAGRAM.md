# DVPE Block Diagram: FieldOpus_DrumMachinePro

## Project Overview
- **Platform**: Daisy Field
- **Type**: 6-voice, 16-step drum machine with 8 pattern slots
- **Audio**: Non-interleaved stereo output
- **Sample Rate**: 48kHz (default)
- **Block Size**: 48 samples

---

## Top-Level Block Diagram

```
┌─────────────────────────────────────────────────────────────────────────────────────────┐
│                           FIELDOPUS_DRUM_MACHINE_PRO                                    │
│                              (Daisy Field Hardware)                                      │
├─────────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                          │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐    │
│  │                           HARDWARE INPUTS/OUTPUTS                                 │    │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐           │    │
│  │  │ TOUCH    │  │   KNOB   │  │ SWITCH   │  │  AUDIO  │  │   MIDI  │           │    │
│  │  │ KEYBOARD │  │   1-8    │  │  SW1/2   │  │   OUT   │  │   IN    │           │    │
│  │  │  (16 ch) │  │  (8 ch)  │  │  (2 ch)  │  │ L / R   │  │ (opt.)  │           │    │
│  │  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬────┘  └────┬────┘           │    │
│  │       │             │             │             │            │                  │    │
│  │       ▼             ▼             ▼             ▼            ▼                  │    │
│  │  ┌─────────────────────────────────────────────────────────────────────────┐   │    │
│  │  │                         INPUT PROCESSING LAYER                          │   │    │
│  │  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐  │   │    │
│  │  │  │  Keyboard   │  │    Knob     │  │   Switch   │  │     MIDI        │  │   │    │
│  │  │  │  Handler    │  │  Processor  │  │   Handler  │  │    Handler      │  │   │    │
│  │  │  │  (16→step) │  │  (8→param)  │  │ (SW1:voice)│  │    (optional)   │  │   │    │
│  │  │  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  └────────┬────────┘  │   │    │
│  │  └─────────┼────────────────┼────────────────┼──────────────────┼──────────┘   │    │
│  │            │                │                │                  │              │    │
│  │            ▼                ▼                ▼                  ▼              │    │
│  │  ┌─────────────────────────────────────────────────────────────────────────┐   │    │
│  │  │                        CONTROL BUS (16-bit float)                       │   │    │
│  │  │  [STEP_TRIG] [PARAM_CV] [VOICE_SEL] [PLAY_STATE] [TEMPO] [SWING]        │   │    │
│  │  └────────────────────────────────┬────────────────────────────────────────┘   │    │
│  │                                   │                                            │    │
│  └───────────────────────────────────┼────────────────────────────────────────────┘    │
│                                      │                                                  │
│                                      ▼                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐    │
│  │                          SEQUENCER ENGINE                                       │    │
│  │  ┌────────────────┐  ┌────────────────┐  ┌────────────────────────────────┐  │    │
│  │  │  PATTERN       │  │   STEP         │  │      TIMING                    │  │    │
│  │  │  MEMORY        │  │   COUNTER      │  │      GENERATOR                 │  │    │
│  │  │  [8][6][16]    │  │  (0-15)        │  │      (BPM + Swing)             │  │    │
│  │  │  = 768 bits    │  │                │  │      GetStepIntervalMs()      │  │    │
│  │  └───────┬────────┘  └────────┬───────┘  └──────────────┬─────────────────┘  │    │
│  │          │                    │                         │                    │    │
│  │          └────────────────────┴─────────────────────────┘                    │    │
│  │                                       │                                         │    │
│  │                                       ▼                                         │    │
│  │  ┌────────────────────────────────────────────────────────────────────────┐  │    │
│  │  │                    TRIGGER DISTRIBUTION                                 │  │    │
│  │  │   [KICK_TRIG] [SNARE_TRIG] [CHAT_TRIG] [OHAT_TRIG] [TOM_TRIG] [CLAP_TRIG]│  │    │
│  │  └────────────────────────────────────────────────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────────────────────────────┘    │
│                                      │                                                  │
│                                      ▼                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐    │
│  │                          VOICE LAYER (x6)                                       │    │
│  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐          │    │
│  │  │    KICK      │ │    SNARE     │ │   CHAT       │ │   OHAT       │          │    │
│  │  │ AnalogBass   │ │ AnalogSnare  │ │    HiHat     │ │    HiHat     │          │    │
│  │  │   Drum       │ │    Drum      │ │   (closed)   │ │   (open)     │          │    │
│  │  │              │ │              │ │              │ │              │          │    │
│  │  │ Inputs:      │ │ Inputs:      │ │ Inputs:      │ │ Inputs:      │          │    │
│  │  │  - TRIG      │ │  - TRIG      │ │  - TRIG      │ │  - TRIG      │          │    │
│  │  │  - Decay     │ │  - Decay     │ │  - Decay     │ │  - Decay     │          │    │
│  │  │  - Tone      │ │  - Tone      │ │  - Tone      │ │  - Tone      │          │    │
│  │  │  - Accent    │ │  - Snappy    │ │  - Accent    │ │  - Accent    │          │    │
│  │  │              │ │  - Accent    │ │              │ │              │          │    │
│  │  │ Outputs:     │ │ Outputs:     │ │ Outputs:     │ │ Outputs:     │          │    │
│  │  │  - AUDIO     │ │  - AUDIO     │ │  - AUDIO     │ │  - AUDIO     │          │    │
│  │  └──────┬───────┘ └──────┬───────┘ └──────┬───────┘ └──────┬───────┘          │    │
│  │         │                 │                 │                 │                  │    │
│  │  ┌──────┴─────────────────┴─────────────────┴─────────────────┴───────┐         │    │
│  │  │                     PARAMETER SMOOTHING LAYER                        │         │    │
│  │  │         fonepole() - per-sample smoothing (coeff: 0.002)              │         │    │
│  │  │   [kickDecay] [snareDecay] [chatDecay] [ohatDecay] [tomTune] [clapDecay]│         │    │
│  │  └────────────────────────────────┬────────────────────────────────────┘         │    │
│  └───────────────────────────────────┼────────────────────────────────────────────┘    │
│                                      │                                                  │
│                                      ▼                                                  │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐    │
│  │                          MIXER & OUTPUT LAYER                                  │    │
│  │  ┌─────────────────────────────────────────────────────────────────────────┐   │    │
│  │  │                         STEREO MIXER                                    │   │    │
│  │  │                                                                         │   │    │
│  │  │   L Channel:     R Channel:                                             │   │    │
│  │  │   kick*0.45  +   kick*0.45                                             │   │    │
│  │  │   snare*0.38  +   snare*0.25                                            │   │    │
│  │  │   chat*0.18   +   chat*0.35                                             │   │    │
│  │  │   ohat*0.18   +   ohat*0.35                                             │   │    │
│  │  │   tom*0.35    +   tom*0.25                                              │   │    │
│  │  │   clap*0.25   +   clap*0.30                                             │   │    │
│  │  │                                                                         │   │    │
│  │  └────────────────────────────────┬────────────────────────────────────────┘   │    │
│  │                                   │                                            │    │
│  │                                   ▼                                            │    │
│  │  ┌─────────────────────────────────────────────────────────────────────────┐   │    │
│  │  │                    MASTER PROCESSING                                    │   │    │
│  │  │  ┌─────────────────┐   ┌─────────────────┐                            │   │    │
│  │  │  │  Master Volume  │   │  Soft Saturation │                            │   │    │
│  │  │  │  (0.0 - 1.0)   │   │  SoftSat(x/(1+|x|))                            │   │    │
│  │  │  │  fonepole      │   │  (tanh approx)   │                            │   │    │
│  │  │  └────────┬────────┘   └────────┬────────┘                            │   │    │
│  │  └───────────┼─────────────────────┼──────────────────────────────────────┘   │    │
│  │              │                     │                                            │    │
│  │              ▼                     ▼                                            │    │
│  │  ┌─────────────────────┐  ┌─────────────────────┐                              │    │
│  │  │    AUDIO OUT L     │  │    AUDIO OUT R      │                              │    │
│  │  │   (DaisyField)     │  │   (DaisyField)      │                              │    │
│  │  └─────────────────────┘  └─────────────────────┘                              │    │
│  └─────────────────────────────────────────────────────────────────────────────────┘    │
│                                                                                          │
│  ┌─────────────────────────────────────────────────────────────────────────────────┐    │
│  │                          FEEDBACK LAYER                                           │    │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────────────┐  │    │
│  │  │     OLED        │  │   LED DRIVER    │  │      SWITCH LEDs               │  │    │
│  │  │   DISPLAY       │  │  (16 keys + 8   │  │                                 │  │    │
│  │  │ 128x64 pixels  │  │   knobs)        │  │  SW1: Voice indicator          │  │    │
│  │  │                │  │                 │  │  SW2: Play/Stop state          │  │    │
│  │  │ - Pattern grid │  │ Key LEDs:       │  │                                 │  │    │
│  │  │ - BPM/Swing    │  │  - step active  │  │                                 │  │    │
│  │  │ - Voice name   │  │  - playhead     │  │                                 │  │    │
│  │  │ - Step counter │  │ Knob LEDs:      │  │                                 │  │    │
│  │  │                │  │  - position     │  │                                 │  │    │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────────────────────┘  │    │
│  └─────────────────────────────────────────────────────────────────────────────────┘    │
│                                                                                          │
└─────────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Detailed Module Specifications

### 1. INPUT PROCESSING LAYER

#### 1.1 Keyboard Handler
| Property | Value |
| :--- | :--- |
| **Inputs** | 16 touch keys (A1-A8, B1-B8) |
| **Output** | Step toggle (0-15) |
| **Behavior** | Rising edge detection for step toggle |
| **Mapping** | A1-A8 → Steps 0-7, B1-B8 → Steps 8-15 |

#### 1.2 Knob Processor
| Knob | Parameter | Range | Default |
| :--- | :--- | :--- | :--- |
| K1 | Kick Decay | 0.1 - 1.0s | 0.55s |
| K2 | Snare Decay | 0.1 - 1.0s | 0.45s |
| K3 | HiHat Decay | 0.05 - 0.55s (closed)<br>0.10 - 1.0s (open) | 0.20s (closed)<br>0.50s (open) |
| K4 | Tom Tune | 80 - 300 Hz | 200 Hz |
| K5 | Clap Decay | 0.1 - 1.0s | 0.30s |
| K6 | Master Volume | 0.0 - 1.0 | 0.80 |
| K7 | Tempo | 40 - 240 BPM | 120 BPM |
| K8 | Swing | 0 - 50% | 0% |

#### 1.3 Switch Handler
| Switch | Function | Behavior |
| :--- | :--- | :--- |
| SW1 | Voice Selection | Cycles through: Kick → Snare → CHat → OHat → Tom → Clap |
| SW2 | Play/Stop | Toggles sequencer playback |

---

### 2. SEQUENCER ENGINE

#### 2.1 Pattern Memory
| Property | Value |
| :--- | :--- |
| **Patterns** | 8 slots (0-7) |
| **Voices** | 6 drums per pattern |
| **Steps** | 16 steps per voice |
| **Total Storage** | 8 × 6 × 16 = 768 bits |

#### 2.2 Default Patterns
| Pattern | Style | Description |
| :--- | :--- | :--- |
| P0 | Basic 4/4 Rock | Kick on 1,5,9,13; Snare on 5,13; CHat 8th notes |
| P1 | House | Kick 4-on-floor; CHat offbeats; OHat on 8,16; Clap on 5,13 |
| P2 | Breakbeat | Kick syncopation; Snare triplet feel; CHat 16ths; Tom fills |
| P3 | Techno | Dense kick pattern; CHat offbeats; OHat accents; Tom + Clap fills |
| P4-P7 | (Empty) | User-defined patterns |

#### 2.3 Timing Generator
| Property | Value |
| :--- | :--- |
| **Base Timing** | 16th notes |
| **BPM Range** | 40 - 240 BPM |
| **Swing Range** | 0% - 50% |
| **Swing Algorithm** | Odd steps delayed by `baseMs * swing`<br>Even steps shortened by `baseMs * swing * 0.5` |

---

### 3. VOICE LAYER

#### 3.1 Kick Drum (AnalogBassDrum)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 52 Hz | Base pitch |
| **Tone** | 0.5 | Tone control |
| **Decay** | 0.1 - 1.0s | Envelope decay time |
| **Accent** | 0.80 | Accent level |

#### 3.2 Snare Drum (AnalogSnareDrum)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 185 Hz | Base pitch |
| **Tone** | 0.55 | Tone control |
| **Decay** | 0.1 - 1.0s | Envelope decay time |
| **Snappy** | 0.60 | Noise/snare blend |
| **Accent** | 0.75 | Accent level |

#### 3.3 Closed HiHat (HiHat\<\>)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 4000 Hz | High frequency content |
| **Tone** | 0.6 | Tone control |
| **Decay** | 0.05 - 0.55s | Short decay |
| **Accent** | 0.70 | Accent level |

#### 3.4 Open HiHat (HiHat\<\>)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 4000 Hz | High frequency content |
| **Tone** | 0.6 | Tone control |
| **Decay** | 0.10 - 1.0s | Long decay |
| **Accent** | 0.70 | Accent level |

#### 3.5 Tom (AnalogBassDrum - pitched)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 80 - 300 Hz | Adjustable pitch |
| **Tone** | 0.65 | Tone control |
| **Decay** | 0.40s | Fixed decay |
| **Accent** | 0.80 | Accent level |

#### 3.6 Clap (SyntheticSnareDrum)
| Parameter | Value | Description |
| :--- | :--- | :--- |
| **Frequency** | 800 Hz | Base pitch |
| **Decay** | 0.1 - 1.0s | Envelope decay time |
| **Snappy** | 0.85 | High noise content |
| **Accent** | 0.70 | Accent level |

---

### 4. PARAMETER SMOOTHING

| Function | Purpose | Coefficient |
| :--- | :--- | :--- |
| `fonepole()` | Per-sample parameter smoothing | 0.002 |
| **Purpose** | Prevents zipper noise on parameter changes | |

---

### 5. MIXER & OUTPUT

#### 5.1 Channel Gains
| Voice | L Channel Gain | R Channel Gain |
| :--- | :---: | :---: |
| Kick | 0.45 | 0.45 |
| Snare | 0.38 | 0.25 |
| Closed HiHat | 0.18 | 0.35 |
| Open HiHat | 0.18 | 0.35 |
| Tom | 0.35 | 0.25 |
| Clap | 0.25 | 0.30 |

#### 5.2 Master Processing
| Stage | Function | Formula |
| :--- | :--- | :--- |
| Volume | Master gain with smoothing | `fonepole()` with coeff 0.002 |
| Soft Saturation | Tanh-like soft clipping | `x / (1.0 + |x|)` |

---

### 6. FEEDBACK LAYER

#### 6.1 OLED Display (128×64)
| Line | Content |
| :--- | :--- |
| Line 1 | "DRUM PRO P[1-8]" - Pattern number |
| Line 2 | Voice name + BPM + Swing % |
| Line 3 | Steps 1-8 grid (visual boxes) |
| Line 4 | Steps 9-16 grid (visual boxes) |
| Line 5 | Play/Stop state + Current step (1-16) |

#### 6.2 LED Driver
| LED Group | Count | Function |
| :--- | :--- | :--- |
| Key LEDs | 16 | Show active steps (50% brightness)<br>Show playhead (100% brightness) |
| Knob LEDs | 8 | Mirror knob positions (0-100%) |
| SW1 LED | 1 | Voice indicator (15% + 14% per voice) |
| SW2 LED | 1 | Play (100%) / Stop (15%) |

---

## Audio Signal Flow

```
┌────────────────────────────────────────────────────────────────────────────────────┐
│                              AUDIO CALLBACK (per-sample)                           │
├────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                    │
│  for each sample i:                                                               │
│                                                                                    │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 1. PARAMETER SMOOTHING (per-sample)                                        │    │
│  │    fonepole(kickDecayCur,  kickDecay,  0.002f)                             │    │
│  │    fonepole(snareDecayCur, snareDecay, 0.002f)                             │    │
│  │    fonepole(chatDecayCur,  chatDecay,  0.002f)                             │    │
│  │    fonepole(ohatDecayCur,  ohatDecay,  0.002f)                             │    │
│  │    fonepole(tomTuneCur,    tomTune,    0.002f)                             │    │
│  │    fonepole(clapDecayCur,  clapDecay,  0.002f)                             │    │
│  │    fonepole(masterVolCur,  masterVol,  0.002f)                             │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 2. VOICE PROCESSING                                                        │    │
│  │    kick.SetDecay(kickDecayCur)                                            │    │
│  │    snare.SetDecay(snareDecayCur)                                         │    │
│  │    hihatClosed.SetDecay(chatDecayCur)                                     │    │
│  │    hihatOpen.SetDecay(ohatDecayCur)                                       │    │
│  │    tom.SetFreq(tomTuneCur)                                                │    │
│  │    tom.SetDecay(0.40f)                                                    │    │
│  │    clap.SetDecay(clapDecayCur)                                            │    │
│  │                                                                            │    │
│  │    kickSig  = kick.Process(false)                                        │    │
│  │    snareSig = snare.Process(false)                                       │    │
│  │    chatSig  = hihatClosed.Process(false)                                  │    │
│  │    ohatSig  = hihatOpen.Process(false)                                    │    │
│  │    tomSig   = tom.Process(false)                                          │    │
│  │    clapSig  = clap.Process(false)                                         │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 3. STEREO MIXING                                                           │    │
│  │    mixL = kick*0.45 + snare*0.38 + chat*0.18 + ohat*0.18 + tom*0.35      │    │
│  │          + clap*0.25                                                      │    │
│  │    mixR = kick*0.45 + snare*0.25 + chat*0.35 + ohat*0.35 + tom*0.25      │    │
│  │          + clap*0.30                                                      │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 4. MASTER PROCESSING                                                       │    │
│  │    mixL = SoftSat(mixL * (0.5 + masterVolCur * 1.5f))                    │    │
│  │    mixR = SoftSat(mixR * (0.5 + masterVolCur * 1.5f))                    │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 5. OUTPUT                                                                  │    │
│  │    out[0][i] = mixL                                                       │    │
│  │    out[1][i] = mixR                                                       │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                                                                    │
└────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Control Flow (Main Loop)

```
┌────────────────────────────────────────────────────────────────────────────────────┐
│                              MAIN LOOP (while(1))                                  │
├────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                    │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 1. hw.ProcessAllControls()                                               │    │
│  │    - Sample ADC (knobs)                                                  │    │
│  │    - Sample digital inputs (switches, keys)                             │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 2. HandleUiEvents()                                                      │    │
│  │    - SW1 Rising Edge → Cycle selectedDrum (0-5)                         │    │
│  │    - SW2 Rising Edge → Toggle seqPlaying                                 │    │
│  │    - Keyboard Rising Edge → Toggle pattern[current][drum][step]         │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 3. ProcessKnobs()                                                        │    │
│  │    - Read all 8 knobs                                                    │    │
│  │    - Map to parameter targets (kickDecay, snareDecay, etc.)            │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 4. Sequencer Timing Check                                                │    │
│  │    if (seqPlaying && (now - lastStepMs >= interval)):                  │    │
│  │        currentStep = (currentStep + 1) % 16                             │    │
│  │        swingStep++                                                       │    │
│  │        lastStepMs = now                                                  │    │
│  │        for each drum d:                                                  │    │
│  │            if (patterns[current][d][currentStep]):                      │    │
│  │                TriggerDrum(d)                                            │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 5. UpdateLeds()                                                          │    │
│  │    - Key LEDs: Show pattern + playhead                                   │    │
│  │    - Knob LEDs: Mirror knob positions                                    │    │
│  │    - Switch LEDs: Voice indicator + play state                          │    │
│  │    - SwapBuffersAndTransmit()                                           │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 6. UpdateDisplay()                                                       │    │
│  │    - Fill screen                                                         │    │
│  │    - Draw pattern grid                                                   │    │
│  │    - Draw text (voice, BPM, swing, step)                               │    │
│  │    - display.Update()                                                   │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                     │                                              │
│                                     ▼                                              │
│  ┌────────────────────────────────────────────────────────────────────────────┐    │
│  │ 7. System::Delay(2)                                                      │    │
│  └────────────────────────────────────────────────────────────────────────────┘    │
│                                                                                    │
└────────────────────────────────────────────────────────────────────────────────────┘
```

---

## Memory Usage Estimate

| Component | Memory Type | Estimated Usage |
| :--- | :--- | :--- |
| Pattern Memory | SRAM | 768 bits = 96 bytes |
| Drum Voices (6×) | SRAM | ~2 KB |
| State Variables | SRAM | ~256 bytes |
| Stack | SRAM | ~4 KB |
| **Total SRAM** | | ~6.5 KB |
| Code + Constants | Flash | ~64 KB |

---

## File: FieldOpus_DrumMachinePro.cpp

- **Lines**: 542
- **Main Functions**: `main()`, `AudioCallback()`, `HandleUiEvents()`, `ProcessKnobs()`, `UpdateLeds()`, `UpdateDisplay()`
- **DSP Modules Used**: `AnalogBassDrum`, `AnalogSnareDrum`, `HiHat<>`, `SyntheticSnareDrum`

---

## Related Documentation

- [Daisy Field Hardware Reference](../libDaisy/src/daisy_field.h)
- [DaisySP Drum Modules](../DaisySP/Source/Drums/)
- [DVPE Module Catalog](../DAISY_QAE/DVPE_MODULE_CATALOG.md)
