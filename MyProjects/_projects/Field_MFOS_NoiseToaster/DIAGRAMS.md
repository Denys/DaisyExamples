# Field_MFOS_NoiseToaster Diagrams

## 1. Analog Reference Redraw

This Mermaid redraw is based on Figure 4-2 from the MFOS Noise Toaster reference. It is the analog guiding-star architecture, not the current Daisy Field firmware.

```mermaid
flowchart TB
    classDef block fill:#f3f3f3,stroke:#222,stroke-width:1px,color:#000;

    WN["White Noise<br/>Input Select"]:::block
    VCO["VCO<br/>Ramp / Square<br/>AR Mod | LFO Mod | Freq | Sync"]:::block
    VCF["VCF<br/>Inputs | Cutoff | Res | Mod"]:::block

    LFO["LFO<br/>Freq | Wave Select<br/>Square | Diff Square | Int Square"]:::block
    AREG["AREG<br/>Attack | Release<br/>Repeat / Manual | Manual Gate"]:::block
    VCA["VCA<br/>In | CV | Out<br/>Bypass"]:::block
    OUT["Output Stage<br/>Out Level<br/>Speaker / Line Out"]:::block

    WN --> VCF
    VCO --> VCF

    LFO -. LFO mod .-> VCO
    LFO -. sync pulse .-> VCO
    LFO -. mod source .-> VCF

    AREG -. AR mod .-> VCO
    AREG -. mod source .-> VCF
    AREG -. CV .-> VCA

    VCF --> VCA
    VCF -. bypass path .-> OUT
    VCA --> OUT
```

## 2. Analog Reference Audio Signal Flow

```mermaid
flowchart LR
    VCOSEL["VCO Ramp / Square Select"] --> VCFIN["VCF Input"]
    NOISE["White Noise Switch"] --> VCFIN
    VCFIN --> VCF["VCF"]
    VCF --> VCA["VCA"]
    VCF -. bypass .-> OUT["Output Stage"]
    VCA --> OUT

    LFO["LFO"] -. VCO mod .-> VCOSEL
    LFO -. sync .-> VCOSEL
    LFO -. VCF mod source .-> VCF

    AREG["AREG"] -. VCO mod .-> VCOSEL
    AREG -. VCF mod source .-> VCF
    AREG -. VCA CV .-> VCA
```

## 3. Analog Reference Control And Event Flow

```mermaid
flowchart TD
    PANEL["Analog Front Panel"] --> VCOCTL["VCO Freq / LFO Mod / AR Mod / Sync"]
    PANEL --> VCFCTL["VCF Cutoff / Res / Mod Depth / Mod Source"]
    PANEL --> LFOCTL["LFO Rate / Wave Select"]
    PANEL --> AREGCTL["AREG Attack / Release / Repeat / Manual Gate"]
    PANEL --> VCACTL["VCA Bypass"]
    PANEL --> OUTCTL["Output Level"]

    AREGCTL --> AREG["AREG"]
    LFOCTL --> LFO["LFO"]
    VCOCTL --> VCO["VCO"]
    VCFCTL --> VCF["VCF"]
    VCACTL --> VCA["VCA"]
    OUTCTL --> OUT["Output"]

    LFO --> VCO
    LFO --> VCF
    AREG --> VCO
    AREG --> VCF
    AREG --> VCA
    VCO --> VCF
    VCF --> VCA
    VCA --> OUT
```

## 4. Current Firmware Block Diagram

This block diagram is a 1:1 representation of the current `Field_MFOS_NoiseToaster.cpp` firmware.

```mermaid
flowchart TB
    classDef block fill:#f3f3f3,stroke:#222,stroke-width:1px,color:#000;

    PERF["Field Performance Logic<br/>A1-A8 Note Select + Trigger<br/>SW1 Manual Gate<br/>SW2 Panic"]:::block
    VCO["VCO<br/>Saw / Square / Triangle<br/>K1 Freq | K2 LFO Mod | K3 AREG Mod<br/>B1 Cycle"]:::block
    LFO["LFO<br/>Fixed 2.2 Hz<br/>Sine / Square / Triangle<br/>B2 Cycle"]:::block
    WN["White Noise<br/>Fixed 18% Blend"]:::block
    VCF["SVF Low-Pass<br/>K4 Cutoff | K5 Res | K6 Mod Depth<br/>B3 Source: LFO / AREG / Off"]:::block
    AREG["AREG<br/>K7 Attack | K8 Release<br/>B4 Repeat / Manual"]:::block
    VCA["VCA<br/>AREG Driven or Bypassed<br/>B5 Bypass"]:::block
    OUT["Output Stage<br/>Fixed 72% Level<br/>Stereo Mirrored Out"]:::block

    PERF --> VCO
    PERF --> AREG
    VCO --> VCF
    WN --> VCF
    VCF --> VCA
    VCA --> OUT

    LFO -. pitch mod .-> VCO
    LFO -. if B3=LFO .-> VCF
    AREG -. pitch mod .-> VCO
    AREG -. if B3=AREG .-> VCF
    AREG -. if B5 off .-> VCA
```

## 5. Current Firmware Audio Signal Flow

```mermaid
flowchart LR
    NOTE["Armed Note"] --> TUNE["K1 Coarse Tune"]
    TUNE --> PITCH["Final Oscillator Pitch"]
    LFOP["K2 LFO -> VCO"] -. always active .-> PITCH
    ARP["K3 AREG -> VCO"] -. always active .-> PITCH

    PITCH --> VCO["VCO (B1 Wave)"]
    NOISE["Fixed 18% White Noise"] --> MIX["Pre-Filter Blend"]
    VCO --> MIX

    MIX --> LPF["SVF Low-Pass"]
    CUTOFF["K4 Cutoff"] --> LPF
    RES["K5 Resonance"] --> LPF
    VCFMOD["K6 Filter Mod Depth"] --> LPF
    LFOSRC["B3 = LFO"] -. selected source .-> LPF
    ARSRC["B3 = AREG"] -. selected source .-> LPF

    LPF --> VCA["VCA"]
    AREG["AREG"] -. if B5 off .-> VCA
    VCA --> LEVEL["Fixed 72% Output Level"]
    LEVEL --> OUT["Left + Right Outputs"]
```

## 6. Current Firmware Control And Event Flow

```mermaid
flowchart TD
    LOOP["Main Loop"] --> PROC["hw.ProcessAllControls()"]
    PROC --> KNOBS["Snapshot K1-K8 and detect focus"]
    PROC --> AROW["Check A1-A8 rising edges"]
    PROC --> BROW["Check B1-B5 rising edges"]
    PROC --> SWITCH["Check SW1 / SW2"]
    PROC --> UI["Update LEDs and OLED"]

    AROW --> NOTEON["Arm note_hz and queue AREG trigger"]
    BROW --> B1["B1 -> cycle VCO wave"]
    BROW --> B2["B2 -> cycle LFO wave"]
    BROW --> B3["B3 -> cycle VCF mod source"]
    BROW --> B4["B4 -> toggle Repeat / Manual"]
    BROW --> B5["B5 -> toggle VCA bypass"]
    SWITCH --> SW1["SW1 -> retrigger armed note"]
    SWITCH --> SW2["SW2 -> panic and clear armed note"]

    AUDIO["Audio Callback"] --> PARAMS["Read cached knobs, set AREG times, keep LFO at 2.2 Hz"]
    PARAMS --> REP["If Repeat and AREG idle, queue retrigger"]
    REP --> DSP["Process LFO, AREG, VCO, noise, filter, VCA"]
    DSP --> OUT["Write stereo outputs"]
```

## 7. Next Faithfulness Target

This target diagram shows the next logical step after the current firmware if the project keeps moving toward the analog panel.

```mermaid
flowchart TB
    classDef block fill:#f3f3f3,stroke:#222,stroke-width:1px,color:#000;

    PERF["Field Performance Logic<br/>Optional No-Key Manual-Gate Mode"]:::block
    WN["White Noise<br/>Discrete On / Off"]:::block
    VCO["VCO<br/>Ramp / Square / Triangle<br/>Freq | LFO Mod | AREG Mod | Sync"]:::block
    VCF["VCF<br/>Input Select | Cutoff | Res<br/>Mod Source | Mod Depth"]:::block
    LFO["LFO<br/>Live Rate | Wave Select"]:::block
    AREG["AREG<br/>Attack | Release<br/>Repeat / Manual | Manual Gate"]:::block
    VCA["VCA<br/>AREG CV | Bypass"]:::block
    OUT["Output Stage<br/>Live Out Level"]:::block

    PERF --> VCO
    PERF --> AREG
    WN --> VCF
    VCO --> VCF
    VCF --> VCA
    VCA --> OUT

    LFO -. VCO mod / sync .-> VCO
    LFO -. VCF mod source .-> VCF
    AREG -. VCO mod .-> VCO
    AREG -. VCF mod source .-> VCF
    AREG -. VCA control .-> VCA
```

## 8. Documentation Notes

- The current firmware now implements the analog-style `Repeat / Manual`, `Manual Gate`, `VCF mod source`, `VCA bypass`, and live `AREG` timing controls.
- The current firmware still uses a fixed white-noise blend, a fixed LFO rate, and a fixed output level.
- The current block and flow diagrams are intended to be literal reflections of the code, not aspirational sketches.
