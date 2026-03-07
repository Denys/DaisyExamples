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

## 2. Current Firmware Block Diagram

This block diagram is a 1:1 representation of the current `Field_MFOS_NoiseToaster.cpp` firmware, drawn in the same style as the analog reference rather than the same architecture.

```mermaid
flowchart TB
    classDef block fill:#f3f3f3,stroke:#222,stroke-width:1px,color:#000;

    KEYS["Field Note / Gate Logic<br/>A1-A8 Notes<br/>B4 or SW1 Hold<br/>SW2 Panic"]:::block
    VCO["VCO<br/>Saw / Square / Triangle<br/>K1 Tune | B1-B3 Wave"]:::block
    WN["White Noise<br/>Always Available"]:::block
    MIX["Mix<br/>K2 Osc / Noise Mix"]:::block
    VCF["SVF Low-Pass<br/>K3 Cutoff | K4 Res"]:::block
    LFO["LFO<br/>Fixed Sine<br/>K6 Rate | K7 Shared Depth"]:::block
    AR["AR Contour<br/>Fixed Attack / Decay<br/>K5 Depth | B5 Pitch/Filter"]:::block
    VCA["VCA<br/>Fixed ADSR<br/>No Bypass"]:::block
    OUT["Output Stage<br/>K8 Level<br/>Stereo Mirrored Out"]:::block

    KEYS --> VCO
    KEYS --> AR
    KEYS --> VCA

    VCO --> MIX
    WN --> MIX
    MIX --> VCF
    VCF --> VCA
    VCA --> OUT

    LFO -. pitch depth always active .-> VCO
    LFO -. filter depth always active .-> VCF
    AR -. B5 selects pitch or filter .-> VCO
    AR -. B5 selects pitch or filter .-> VCF
```

## 3. Target Post-Improvement Block Diagram

This target diagram shows the closest practical Daisy Field adaptation to the analog Figure 4-2 architecture after the highest-value faithfulness improvements are applied. It is not the current firmware.

```mermaid
flowchart TB
    classDef block fill:#f3f3f3,stroke:#222,stroke-width:1px,color:#000;

    PERF["Field Performance Controls<br/>Manual Gate | Repeat / Manual<br/>Optional note actions"]:::block
    WN["White Noise<br/>On / Off"]:::block
    VCO["VCO<br/>Ramp / Square<br/>Freq | LFO Mod | AR Mod | Sync"]:::block
    VCF["VCF<br/>Input Select | Cutoff | Res<br/>Mod Source | Mod Depth"]:::block
    LFO["LFO<br/>Rate | Wave Select<br/>Square | Diff Square | Int Square"]:::block
    AREG["AREG<br/>Attack | Release<br/>Manual Gate | Repeat"]:::block
    VCA["VCA<br/>AREG CV | Bypass"]:::block
    OUT["Output Stage<br/>Out Level<br/>Stereo-Mirrored Line Out"]:::block

    PERF --> AREG
    PERF --> VCO

    WN --> VCF
    VCO --> VCF
    VCF --> VCA
    VCF -. bypass path .-> OUT
    VCA --> OUT

    LFO -. VCO mod .-> VCO
    LFO -. sync source .-> VCO
    LFO -. VCF mod source .-> VCF

    AREG -. VCO mod .-> VCO
    AREG -. VCF mod source .-> VCF
    AREG -. VCA control .-> VCA
```

## 4. Audio Signal Flow

```mermaid
flowchart LR
    NOTE["Selected Note"] --> TUNE["K1 Coarse Tune"]
    TUNE --> PITCH["Final Oscillator Pitch"]
    ARP["AR To Pitch"] -. if selected .-> PITCH
    LFOP["LFO To Pitch"] -. always active .-> PITCH

    PITCH --> VCO["VCO"]
    NOISE["White Noise"] --> MIX["K2 Mix"]
    VCO --> MIX

    MIX --> LPF["SVF Low-Pass"]
    ARC["AR To Filter"] -. if selected .-> LPF
    LFOF["LFO To Filter"] -. always active .-> LPF
    CUTOFF["K3 Cutoff"] --> LPF
    RES["K4 Resonance"] --> LPF

    LPF --> VCA["VCA"]
    ADSR["Fixed ADSR"] --> VCA
    VCA --> LEVEL["K8 Output Level"]
    LEVEL --> OUT["Left + Right Outputs"]
```

## 5. Control And Event Flow

```mermaid
flowchart TD
    LOOP["Main Loop"] --> PROC["hw.ProcessAllControls()"]
    PROC --> AROW["Check A1-A8 edges"]
    PROC --> BROW["Check B1-B5 edges"]
    PROC --> SWITCH["Check SW1 / SW2"]
    PROC --> OLED["Refresh OLED"]

    AROW --> NOTEON["TriggerKey(index)"]
    NOTEON --> GATE["Set note_hz and gate = true"]
    AROW --> NOTEOFF["If active key released and hold is off -> gate = false"]

    BROW --> WAVE["B1/B2/B3 -> SetWaveform()"]
    BROW --> HOLD["B4 -> toggle hold"]
    BROW --> ROUTE["B5 -> toggle AR destination"]

    SWITCH --> SWHOLD["SW1 -> toggle hold"]
    SWITCH --> PANIC["SW2 -> gate off, hold off"]

    AUDIO["Audio Callback"] --> KNOBS["Read K1-K8"]
    KNOBS --> MOD["Update AR depth, LFO rate, LFO depth"]
    MOD --> DSP["Process LFO, AR, ADSR, VCO, noise, filter, VCA"]
    DSP --> OUT["Write stereo outputs"]
```

## 6. Documentation Notes

- The current firmware has one shared LFO depth control, not separate routing buttons.
- The AR contour destination is switchable, but the LFO destination is not.
- `SW1` and `B4` both control the same hold state.
- The analog reference redraw is based on the Ray Wilson PDF and the included `4-2_Noise_Toaster_Block_Diagram.png`.
- The current-firmware block diagram is intentionally less faithful to the analog architecture than the reference redraw, because it reflects the code as it exists today.
- The target post-improvement diagram is a planning aid only and must not be read as a statement about current firmware behavior.
