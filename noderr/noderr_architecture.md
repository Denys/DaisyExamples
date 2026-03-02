# Noderr - Architectural Flowchart

**Purpose:** Mermaid architecture with canonical NodeIDs tracked in `noderr_tracker.md` and specified in `noderr/specs/`.

---

```mermaid
graph TD
    subgraph Legend
        direction LR
        L_IDConv(NodeID Convention TypeName)
        L_Proc([Process Backend Logic])
        L_UI[/UI Component/]
    end

    subgraph Hardware_and_LibDaisy
        direction TB
        LIBDASY_HWInit[LIBDASY_HWInit Hardware Initialization]
        LIBDASY_AudioDriver[LIBDASY_AudioDriver Audio Callback Driver]
        LIBDASY_HWInit --> LIBDASY_AudioDriver
    end

    subgraph DaisySP_Core
        direction TB
        DSP_Oscillators[DSP_Oscillators Sine Saw Square Triangle]
        DSP_Filters[DSP_Filters LP HP BP Notch]
        DSP_Delay[DSP_Delay Echo Ping Pong]
        DSP_Reverb[DSP_Reverb Algorithmic Reverb]
        DSP_Envelope[DSP_Envelope ADSR Generators]
        DSP_Modulation[DSP_Modulation Chorus Flanger Phaser]

        DSP_Oscillators --> DSP_Filters
        DSP_Filters --> DSP_Delay
        DSP_Delay --> DSP_Reverb
        DSP_Modulation --> DSP_Filters
    end

    subgraph Field_Effects
        direction TB
        FX_Chorus[FX_Chorus Chorus Effect]
        FX_Flanger[FX_Flanger Flanger Effect]
        FX_Phaser[FX_Phaser Phaser Effect]
        FX_Sampler[FX_Sampler Sample Playback]
        FX_StringVoice[FX_StringVoice String Synth]
        FX_ModalVoice[FX_ModalVoice Modal Synth]
    end

    subgraph Nimbus
        direction TB
        NIM_Granular[NIM_Granular Granular Processor]
        NIM_SamplePlayer[NIM_SamplePlayer Sample Player]
        NIM_WSOLA[NIM_WSOLA WSOLA Time Stretch]
    end

    subgraph Examples
        direction TB
        EX_Keyboard[EX_Keyboard Keyboard Test]
        EX_Midi[EX_Midi Midi Interface]
    end

    subgraph DVPE_MVP_Missing
        direction TB
        DVPE_UI[/DVPE_UI Visual Editor Interface/]
        DVPE_Compiler[DVPE_Compiler Block To Cpp Compiler]
        DVPE_ParamControl[DVPE_ParamControl Runtime Parameter Control]
        DVPE_PresetManager[DVPE_PresetManager Preset Storage And Recall]
    end

    LIBDASY_AudioDriver --> DSP_Oscillators
    LIBDASY_AudioDriver --> DSP_Filters
    LIBDASY_AudioDriver --> DSP_Delay
    LIBDASY_AudioDriver --> DSP_Reverb
    LIBDASY_AudioDriver --> DSP_Envelope
    LIBDASY_AudioDriver --> DSP_Modulation

    DSP_Modulation --> FX_Chorus
    DSP_Modulation --> FX_Flanger
    DSP_Modulation --> FX_Phaser
    DSP_Filters --> FX_Sampler
    DSP_Delay --> FX_StringVoice
    DSP_Envelope --> FX_ModalVoice

    DSP_Oscillators --> NIM_Granular
    DSP_Filters --> NIM_Granular
    NIM_Granular --> NIM_SamplePlayer
    NIM_SamplePlayer --> NIM_WSOLA

    LIBDASY_HWInit --> EX_Keyboard
    LIBDASY_HWInit --> EX_Midi

    DVPE_UI --> DVPE_Compiler
    DVPE_UI --> DVPE_ParamControl
    DVPE_UI --> DVPE_PresetManager
    DVPE_Compiler -.-> FX_Chorus
    DVPE_Compiler -.-> DSP_Filters
```

---

## Component Summary

### Existing Components Implemented
- LIBDASY_HWInit
- LIBDASY_AudioDriver
- DSP_Oscillators
- DSP_Filters
- DSP_Delay
- DSP_Reverb
- DSP_Envelope
- DSP_Modulation
- FX_Chorus
- FX_Flanger
- FX_Phaser
- FX_Sampler
- FX_StringVoice
- FX_ModalVoice
- NIM_Granular
- NIM_SamplePlayer
- NIM_WSOLA
- EX_Keyboard
- EX_Midi

### Missing Components Required For MVP
- DVPE_UI
- DVPE_Compiler
- DVPE_ParamControl
- DVPE_PresetManager
