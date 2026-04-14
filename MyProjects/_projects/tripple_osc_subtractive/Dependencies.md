# tripple_osc_subtractive Dependencies

This document tracks the dependency structure for the current `tripple_osc_subtractive` Daisy Field project.

## Project status (Codex Cloud -> Local)

- **Current working status:** Local project folder is present and self-contained in:
  - `DaisyExamples/MyProjects/_projects/tripple_osc_subtractive`
- **Code source status:** Main runtime source is local in `tripple_osc_subtractive.cpp`.
- **Docs status:** `README.md`, `CONTROLS.md`, and this `Dependencies.md` are local and synchronized for this project.
- **Build environment status in this container:** compile is blocked unless repository-relative `libDaisy` and `DaisySP` dependencies are present.

## 1) Build and link dependency graph

```mermaid
flowchart LR
    MK[tripple_osc_subtractive/Makefile]

    APP[tripple_osc_subtractive.cpp]
    DF[daisy_field.h / libDaisy]
    DSP[daisysp.h / DaisySP]

    LIB[libDaisy static library]
    DSPLIB[DaisySP static library]

    MK --> APP
    MK --> LIB
    MK --> DSPLIB

    APP --> DF
    APP --> DSP
    APP --> LIB
    APP --> DSPLIB
```

## 2) Runtime control and audio flow

```mermaid
flowchart TD
    MIDIIN[External MIDI keyboard / sequencer]
    CTRLS[Daisy Field hardware controls]

    LOOP[main loop]
    MIDILISTEN[hw.midi.Listen + PopEvent]
    HM[HandleMidiMessage]

    UILED[UpdateLeds]
    UIOLED[UpdateDisplay]

    AUDIO[AudioCallback]
    MODE[UpdateModeFromSwitches]
    KEYS[HandleLedKeyFunctions]
    BANK[UpdateControlBanks]

    SYNTH[3 Osc + Noise + Sub]
    MOD[LFO + AMP/FILT ADSR]
    FILT[SVF LP/BP/HP]
    OUT[Stereo Output]

    MIDIIN --> MIDILISTEN
    CTRLS --> LOOP
    LOOP --> MIDILISTEN
    MIDILISTEN --> HM

    LOOP --> UILED
    LOOP --> UIOLED

    AUDIO --> MODE
    AUDIO --> KEYS
    AUDIO --> BANK
    AUDIO --> SYNTH
    AUDIO --> MOD
    AUDIO --> FILT
    FILT --> OUT
```

## 3) File-level application dependency graph

```mermaid
flowchart LR
    subgraph AppFile[tripple_osc_subtractive.cpp]
        MAIN[main]
        CB[AudioCallback]
        MIDIH[HandleMidiMessage]
        DISP[UpdateDisplay]
        LEDS[UpdateLeds]
        MAP[mode_param_map + params]
    end

    subgraph Hardware[Daisy Field Runtime]
        MIDIDEV[hw.midi]
        OLED[hw.display]
        LEDDRV[hw.led_driver]
        KNOBS[hw.knob]
        SWITCHES[hw.sw]
        KEYS2[hw.KeyboardRisingEdge]
    end

    subgraph DSP[Voice DSP]
        OSCS[osc1 osc2 osc3]
        NOISE[noiseosc]
        LFO[lfo]
        ENV[amp_env + filt_env]
        SVF[filter]
    end

    MAIN --> MIDIDEV
    MAIN --> DISP
    MAIN --> LEDS
    MAIN --> CB

    CB --> MAP
    CB --> OSCS
    CB --> NOISE
    CB --> LFO
    CB --> ENV
    CB --> SVF

    DISP --> OLED
    LEDS --> LEDDRV
    CB --> KNOBS
    CB --> SWITCHES
    CB --> KEYS2
    MIDIH --> MIDIDEV
```

## 4) Control-bank and parameter-flow diagrams

### 4.1 Can this be one Mermaid graph?

Short answer: **partially**.

- A single graph can show the overall relationship between the 3 pages, shared DSP destinations, and UI feedback loops.
- But a single graph that also lists all 24 parameters and all destinations becomes visually dense and hard to maintain.

For this project, the best documentation compromise is:

1. **One compact global graph** for architecture-level understanding.
2. **Three per-page detail graphs** (DEFAULT/SW1/SW2) for complete parameter coverage.

### 4.2 Single global controls-flow graph (overview)

```mermaid
flowchart LR
    SW[SW1/SW2 page select] --> MODE[current_mode]
    K[K1-K8 knobs] --> MAP[mode_param_map 3x8]
    MODE --> MAP
    MAP --> P[params[24] store]

    subgraph PAGES[Control Pages]
        D[DEFAULT page
P0..P7]
        S1[SW1 page
P8..P15]
        S2[SW2 page
P16..P23]
    end

    P --> D
    P --> S1
    P --> S2

    D --> OSC[Oscillator Pitch/Mix Bus]
    D --> FILT[Filter Base Bus]
    D --> AMPENV[Amp Env Time Bus]

    S1 --> FILTENV[Filter Env Bus]
    S1 --> LFOBUS[LFO Routing Bus]
    S1 --> PERF[Performance Bus]

    S2 --> AMPBUS[Amp Shape Bus]
    S2 --> NOISEBUS[Noise/Sub/Stereo Bus]
    S2 --> GAIN[Output Gain Bus]

    OSC --> AUDIO[AudioCallback DSP]
    FILT --> AUDIO
    AMPENV --> AUDIO
    FILTENV --> AUDIO
    LFOBUS --> AUDIO
    PERF --> AUDIO
    AMPBUS --> AUDIO
    NOISEBUS --> AUDIO
    GAIN --> AUDIO

    AUDIO --> OUT[Stereo Out]

    P --> OLED[UpdateDisplay + zoom]
    P --> LEDS[UpdateLeds rings]
    MODE --> OLED
    MODE --> LEDS
```

### 4.3 Split detailed graphs (complete parameter coverage)

#### 4.3.1 DEFAULT page (`P0..P7`) detailed flow

```mermaid
flowchart TD
    subgraph DEFAULT[DEFAULT page knobs K1..K8]
        P0[P0 OSC1<->OSC2 MIX]
        P1[P1 OSC3 LEVEL]
        P2[P2 OSC2 DETUNE]
        P3[P3 OSC3 DETUNE]
        P4[P4 FILTER CUTOFF]
        P5[P5 FILTER RES]
        P6[P6 AMP ATTACK]
        P7[P7 AMP RELEASE]
    end

    P0 --> MIXBUS[Osc mix]
    P1 --> MIXBUS
    P2 --> PITCHBUS[Osc pitch set]
    P3 --> PITCHBUS

    P4 --> FILTBASE[Base cutoff]
    P5 --> FILTBASE

    P6 --> AMPADSR[amp_env attack]
    P7 --> AMPADSR[amp_env release]

    MIXBUS --> DSP[AudioCallback voice path]
    PITCHBUS --> DSP
    FILTBASE --> DSP
    AMPADSR --> DSP
```

#### 4.3.2 SW1 page (`P8..P15`) detailed flow

```mermaid
flowchart TD
    subgraph SW1[SW1 page knobs K1..K8]
        P8[P8 FILT ENV AMT]
        P9[P9 FILT DECAY]
        P10[P10 FILT SUSTAIN]
        P11[P11 DRIVE]
        P12[P12 LFO RATE]
        P13[P13 LFO->PITCH]
        P14[P14 LFO->FILTER]
        P15[P15 GLIDE]
    end

    P8 --> FILTENVAMT[Env to cutoff depth]
    P9 --> FILTADSR[filt_env decay]
    P10 --> FILTADSR[filt_env sustain]

    P11 --> DRIVEBUS[Pre-filter saturation]

    P12 --> LFOSET[lfo freq/wave process]
    P13 --> LFOPITCH[LFO pitch modulation]
    P14 --> LFOFILT[LFO cutoff modulation]

    P15 --> GLIDEBUS[Portamento smoothing]

    FILTENVAMT --> DSP[AudioCallback modulation]
    FILTADSR --> DSP
    DRIVEBUS --> DSP
    LFOSET --> DSP
    LFOPITCH --> DSP
    LFOFILT --> DSP
    GLIDEBUS --> DSP
```

#### 4.3.3 SW2 page (`P16..P23`) detailed flow

```mermaid
flowchart TD
    subgraph SW2[SW2 page knobs K1..K8]
        P16[P16 AMP DECAY]
        P17[P17 AMP SUSTAIN]
        P18[P18 NOISE LEVEL]
        P19[P19 SUB LEVEL]
        P20[P20 PAN SPREAD]
        P21[P21 VELOCITY AMT]
        P22[P22 LFO->AMP]
        P23[P23 MASTER VOL]
    end

    P16 --> AMPADSR2[amp_env decay]
    P17 --> AMPADSR2[amp_env sustain]

    P18 --> NOISEBUS2[Noise mix contribution]
    P19 --> SUBBUS2[Sub oscillator contribution]

    P20 --> STEREOBUS[Stereo spread scaler]
    P21 --> VELBUS[Velocity response blend]
    P22 --> AMP_LFOBUS[Amp tremolo depth]
    P23 --> MASTERBUS[Final output gain]

    AMPADSR2 --> DSP[AudioCallback output stage]
    NOISEBUS2 --> DSP
    SUBBUS2 --> DSP
    STEREOBUS --> DSP
    VELBUS --> DSP
    AMP_LFOBUS --> DSP
    MASTERBUS --> DSP
```

### 4.4 Interconnection notes across pages

- The pages are **stored independently** in one `params[24]` array but consumed together every audio block.
- SW1 modulation parameters (LFO/filter env/drive/glide) alter how DEFAULT oscillator+filter settings behave.
- SW2 amplitude/output parameters shape final loudness, stereo image, and dynamics after DEFAULT+SW1 synthesis stages.

## 5) MIDI state and note-priority graph

```mermaid
flowchart TD
    EVT[MidiEvent]
    NOTEON[NoteOn]
    NOTEOFF[NoteOff]

    HELD[note_held[128]]
    RECOMP[RecomputeCurrentNote]

    NOTE[current_note]
    VEL[current_velocity]
    GATE[gate]

    EVT --> NOTEON
    EVT --> NOTEOFF

    NOTEON --> HELD
    NOTEON --> VEL
    NOTEON --> NOTE
    NOTEON --> GATE

    NOTEOFF --> HELD
    HELD --> RECOMP
    RECOMP --> NOTE
    RECOMP --> GATE
```

## 6) Makefile dependency notes

- `TARGET = tripple_osc_subtractive`
- `CPP_SOURCES = tripple_osc_subtractive.cpp`
- External dirs are expected at:
  - `../../../libDaisy`
  - `../../../DaisySP`

If these paths are missing in the local checkout, `make` will fail before compilation.

## 7) Documentation synchronization policy

When control routing, mode mapping, or DSP architecture changes, update all of:

1. `README.md`
2. `CONTROLS.md`
3. `Dependencies.md`

in the same commit to keep local docs consistent.
