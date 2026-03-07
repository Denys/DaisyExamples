# Noderr Architecture: Field_AmbientGarden Firmware

**Last Updated:** 2026-03-06
**NodeID Count:** 22

---

## System Topology

```mermaid
flowchart LR
    subgraph HW ["Hardware Layer (Daisy Field)"]
        HW_Platform["HW_Platform\nDaisyField hw\nInit / StartAdc / StartAudio"]
        HW_Knobs["HW_Knobs\n8× knob[0-7]"]
        HW_KeysA["HW_KeysA\nA1–A8\nScale Select"]
        HW_KeysB["HW_KeysB\nB1–B8\nPreset Select"]
        HW_Switches["HW_Switches\nSW1 Freeze / SW2 Wide"]
        HW_OLED["HW_OLED\n128×64 display"]
        HW_LEDs["HW_LEDs\nled_driver + keyLeds"]
    end

    subgraph GEN ["Generative Engine"]
        GEN_RandomClock["GEN_RandomClock\nProbabilistic trigger"]
        GEN_TuringMachine["GEN_TuringMachine\n8-bit shift register"]
        GEN_ScaleQuantizer["GEN_ScaleQuantizer\nScale + root mapping"]
    end

    subgraph CFG ["Configuration"]
        CFG_ScaleSystem["CFG_ScaleSystem\n8 scales + root offset"]
        CFG_VoicePresets["CFG_VoicePresets\n8 character presets"]
    end

    subgraph FW ["Firmware Bridge"]
        FW_ParamBridge["FW_ParamBridge\nParams / SmoothedParams\nmain→callback contract"]
        FW_KnobProcessor["FW_KnobProcessor\nProcessKnobs()\nmapping + targets"]
        FW_ClockTrigger["FW_ClockTrigger\nOnClockTrigger()\nvoice distribution"]
        FW_AudioCallback["FW_AudioCallback\nAudioCallback()\nper-sample loop"]
    end

    subgraph DSP ["DSP Layer"]
        DSP_ModalVoices["DSP_ModalVoices\n4× ModalVoice (LGPL)\nphysical synthesis"]
        DSP_PreLPF["DSP_PreLPF\nSvf L + R\npre-reverb LPF"]
        DSP_Reverb["DSP_Reverb\nReverbSc (LGPL)\nstereo"]
        DSP_SoftClip["DSP_SoftClip\ntanhf() × 0.8\noutput saturation"]
    end

    subgraph UI ["UI Layer (main loop)"]
        UI_OLEDRenderer["UI_OLEDRenderer\nTitle / Voice / Param\n/ Status display"]
        UI_LEDAnimator["UI_LEDAnimator\nKnob + A/B rows\n+ switch LEDs"]
    end

    %% Hardware init
    HW_Platform -->|"sample rate"| FW_AudioCallback
    HW_Platform -->|"ProcessAllControls"| FW_KnobProcessor

    %% Knob/key/switch → param bridge
    HW_Knobs --> FW_KnobProcessor
    HW_KeysA --> CFG_ScaleSystem
    HW_KeysB --> CFG_VoicePresets
    HW_Switches -->|"freeze / wide flags"| FW_AudioCallback

    %% Config → quantizer / clock
    CFG_ScaleSystem --> GEN_ScaleQuantizer
    CFG_VoicePresets -->|"brightness/damping/structure/accent"| FW_ClockTrigger

    %% Param bridge: main loop writes, callback reads
    FW_KnobProcessor -->|"writes Params struct"| FW_ParamBridge
    FW_ParamBridge -->|"reads Params struct"| FW_AudioCallback

    %% Clock → trigger
    FW_AudioCallback -->|"per-sample clock tick"| GEN_RandomClock
    GEN_RandomClock -->|"trigger pulse"| FW_ClockTrigger
    FW_ClockTrigger -->|"Process()"| GEN_TuringMachine
    GEN_TuringMachine -->|"8-bit value"| GEN_ScaleQuantizer
    GEN_ScaleQuantizer -->|"Hz"| FW_ClockTrigger

    %% Voice triggering
    FW_ClockTrigger -->|"SetFreq / SetAccent / Trig"| DSP_ModalVoices

    %% DSP chain
    DSP_ModalVoices -->|"4× mono signal"| DSP_PreLPF
    FW_AudioCallback -->|"smoothed params"| DSP_PreLPF
    DSP_PreLPF -->|"filtered L+R"| DSP_Reverb
    FW_AudioCallback -->|"smoothed feedback / lpfreq"| DSP_Reverb
    DSP_Reverb -->|"wet L+R"| DSP_SoftClip
    DSP_SoftClip -->|"out[0][i] / out[1][i]"| HW_Platform

    %% UI feeds
    FW_ParamBridge -->|"knob_values[]"| UI_OLEDRenderer
    FW_ClockTrigger -->|"voice_pulse[] / last_freq[]"| UI_OLEDRenderer
    HW_Switches -->|"freeze / wide state"| UI_OLEDRenderer
    CFG_VoicePresets -->|"preset name"| UI_OLEDRenderer
    UI_OLEDRenderer --> HW_OLED

    FW_ParamBridge -->|"knob_values[]"| UI_LEDAnimator
    FW_ClockTrigger -->|"voice_pulse[]"| UI_LEDAnimator
    HW_Switches -->|"freeze / wide state"| UI_LEDAnimator
    CFG_ScaleSystem -->|"current_scale"| UI_LEDAnimator
    CFG_VoicePresets -->|"current_preset"| UI_LEDAnimator
    UI_LEDAnimator --> HW_LEDs
```

---

## NodeID Quick Reference

| NodeID | File / Location | Category |
|--------|----------------|----------|
| `HW_Platform` | `Field_AmbientGarden.cpp:39, 373-431` | Hardware |
| `HW_Knobs` | `Field_AmbientGarden.cpp:233-242` | Hardware |
| `HW_KeysA` | `Field_AmbientGarden.cpp:445-456` | Hardware |
| `HW_KeysB` | `Field_AmbientGarden.cpp:460-478` | Hardware |
| `HW_Switches` | `Field_AmbientGarden.cpp:483-494` | Hardware |
| `HW_OLED` | `Field_AmbientGarden.cpp:513-584` | Hardware |
| `HW_LEDs` | `Field_AmbientGarden.cpp:589-607` | Hardware |
| `GEN_TuringMachine` | `turing_machine.h` | Generative |
| `GEN_ScaleQuantizer` | `scale_quantizer.h` | Generative |
| `GEN_RandomClock` | `random_clock.h` | Generative |
| `DSP_ModalVoices` | `Field_AmbientGarden.cpp:49, 399-409` | DSP (LGPL) |
| `DSP_PreLPF` | `Field_AmbientGarden.cpp:51-52, 416-425` | DSP |
| `DSP_Reverb` | `Field_AmbientGarden.cpp:50, 412-414` | DSP (LGPL) |
| `DSP_SoftClip` | `Field_AmbientGarden.cpp:361-362` | DSP |
| `FW_AudioCallback` | `Field_AmbientGarden.cpp:278-364` | Firmware |
| `FW_ParamBridge` | `Field_AmbientGarden.cpp:80-104` | Firmware |
| `FW_ClockTrigger` | `Field_AmbientGarden.cpp:163-223` | Firmware |
| `FW_KnobProcessor` | `Field_AmbientGarden.cpp:229-272` | Firmware |
| `UI_OLEDRenderer` | `Field_AmbientGarden.cpp:509-584` | UI |
| `UI_LEDAnimator` | `Field_AmbientGarden.cpp:589-607` | UI |
| `CFG_VoicePresets` | `Field_AmbientGarden.cpp:110-128` | Config |
| `CFG_ScaleSystem` | `scale_quantizer.h + cpp:61, 257` | Config |
