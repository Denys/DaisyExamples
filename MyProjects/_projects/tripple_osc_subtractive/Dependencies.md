# tripple_osc_subtractive Dependencies

This document tracks the dependency structure for the current `tripple_osc_subtractive` Daisy Field project.

## Project status (Codex Cloud -> Local)

- **Current working status:** Local project folder is present and self-contained in:
  - `DaisyExamples/MyProjects/_projects/tripple_osc_subtractive`
- **Code source status:** Main runtime source is local in `tripple_osc_subtractive.cpp`.
- **Docs status:** `README.md`, `CONTROLS.md`, and this `Dependencies.md` are local and synchronized for this project.
- **Build environment status:** compile depends on repository-relative `libDaisy` and `DaisySP` directories being present.

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
    CTRLMGMT[UpdateControls]
    MIDILISTEN[hw.midi.Listen + PopEvent]
    HM[HandleMidiMessage]

    UILED[UpdateLeds]
    UIOLED[UpdateDisplay]

    AUDIO[AudioCallback]

    SYNTH[3 Osc + Noise + Sub]
    MOD[LFO + AMP/FILT ADSR]
    FILT[SVF LP/BP/HP]
    OUT[Stereo Output]

    MIDIIN --> MIDILISTEN
    CTRLS --> LOOP
    LOOP --> CTRLMGMT
    LOOP --> MIDILISTEN
    MIDILISTEN --> HM

    LOOP --> UILED
    LOOP --> UIOLED

    CTRLMGMT --> AUDIO
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
        CTRLS2[knobs switches key matrix]
    end

    subgraph DSP[Voice DSP]
        OSCS[osc1 osc2 osc3 subosc]
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
    MAIN --> CTRLS2
    MIDIH --> MIDIDEV
```

## 4) Control-bank and UI dependency graph

```mermaid
flowchart TD
    SW[SW1/SW2]
    K[K1-K8]
    AK[A1-A8 B1-B8]

    MODESEL[current_mode]
    BANKMAP[mode_param_map 3x8]
    PARAMS[params 24]

    ZOOM[active_param + active_param_time]
    OLED2[UpdateDisplay]
    LEDS2[UpdateLeds]

    SW --> MODESEL
    K --> BANKMAP
    MODESEL --> BANKMAP
    BANKMAP --> PARAMS

    K --> ZOOM
    PARAMS --> OLED2
    MODESEL --> OLED2
    ZOOM --> OLED2

    AK --> MODESEL
    AK --> LEDS2
    MODESEL --> LEDS2
    PARAMS --> LEDS2
```

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
