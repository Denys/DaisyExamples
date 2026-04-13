# EDGE Performance FX — Pages, Controls & Effects

Three visual maps:
- **Map 1** — Page hierarchy and parameters per page
- **Map 2** — Physical controls → direct and shifted effect targets
- **Map 3** — DSP signal chain with control assignments per stage

---

## Map 1 — Pages & Parameters

Each page node branches to its editable parameters.
Live controls (Knob 1, Knob 2, SW1, SW2, Pod Enc) are accessible from any page — shown in Map 2.

```mermaid
flowchart LR
    classDef root fill:#0a0a0f,stroke:#e94560,color:#e94560,font-weight:bold
    classDef p1   fill:#3d0812,stroke:#e94560,color:#fff,font-weight:bold
    classDef p2   fill:#0a1e30,stroke:#4fc3f7,color:#fff,font-weight:bold
    classDef p3   fill:#1a0d35,stroke:#b39ddb,color:#fff,font-weight:bold
    classDef p4   fill:#001c28,stroke:#4dd0e1,color:#fff,font-weight:bold
    classDef p5   fill:#071a07,stroke:#81c784,color:#fff,font-weight:bold
    classDef p6   fill:#1c1c1c,stroke:#9e9e9e,color:#fff,font-weight:bold
    classDef d1   fill:#250610,stroke:#e94560,color:#fcc
    classDef d2   fill:#060e18,stroke:#4fc3f7,color:#cde
    classDef d3   fill:#0e0820,stroke:#b39ddb,color:#dce
    classDef d4   fill:#001018,stroke:#4dd0e1,color:#cef
    classDef d5   fill:#040e04,stroke:#81c784,color:#cec
    classDef d6   fill:#111111,stroke:#9e9e9e,color:#ccc

    ROOT(["🎛 EDGE FX"]):::root

    P1["P1  PERFORM"]:::p1
    P2["P2  TONE"]:::p2
    P3["P3  MOTION"]:::p3
    P4["P4  FREEZE"]:::p4
    P5["P5  PRESETS"]:::p5
    P6["P6  SYSTEM"]:::p6

    A1["Delay Time"]:::d1
    A2["Feedback"]:::d1
    A3["Wet / Dry"]:::d1
    A4["Drive"]:::d1
    A5["BPM"]:::d1
    A6["Freeze"]:::d1

    B1["Input HP"]:::d2
    B2["Feedback LP"]:::d2
    B3["Feedback HP"]:::d2
    B4["Wet Tone"]:::d2
    B5["Output Tilt"]:::d2

    C1["Wow Depth"]:::d3
    C2["Wow Rate"]:::d3
    C3["Mod On / Off"]:::d3

    D1["Mode\nMom · Latch"]:::d4
    D2["Hold Behavior"]:::d4
    D3["Loop Size"]:::d4

    E1["Load Preset"]:::d5
    E2["Save Preset"]:::d5
    E3["Init Default"]:::d5

    F1["Brightness"]:::d6
    F2["Enc Direction"]:::d6
    F3["Bypass"]:::d6
    F4["Version  Info"]:::d6

    ROOT --> P1 & P2 & P3 & P4 & P5 & P6

    P1 --> A1 & A2 & A3 & A4 & A5 & A6
    P2 --> B1 & B2 & B3 & B4 & B5
    P3 --> C1 & C2 & C3
    P4 --> D1 & D2 & D3
    P5 --> E1 & E2 & E3
    P6 --> F1 & F2 & F3 & F4
```

---

## Map 2 — Control Routing

Solid arrows = direct action. Dashed arrows = Shift + control.
SW1 is the Shift modifier: short tap alone = Freeze momentary.

```mermaid
flowchart LR
    classDef pod  fill:#0a1e30,stroke:#4fc3f7,color:#fff
    classDef ext  fill:#160a28,stroke:#ce93d8,color:#fff
    classDef shft fill:#3d0812,stroke:#e94560,color:#fff,font-weight:bold
    classDef dir  fill:#071a07,stroke:#81c784,color:#eee
    classDef alt  fill:#1f0f00,stroke:#ffb74d,color:#eee

    subgraph POD["Pod Controls"]
        K1["Knob 1"]:::pod
        K2["Knob 2"]:::pod
        SW1["SW1\nSHIFT · FREEZE"]:::shft
        SW2["SW2\nTAP TEMPO"]:::pod
        PE["Pod Enc"]:::pod
    end

    subgraph EXT["External Board"]
        EE["Ext Enc"]:::ext
        EP["Ext PSH"]:::ext
        BK["BAK"]:::ext
        CN["CON"]:::ext
    end

    subgraph DIR["Direct"]
        N1["Delay Time"]:::dir
        N2["Feedback"]:::dir
        N3["Freeze\nMomentary"]:::dir
        N4["BPM  Tap"]:::dir
        N5["Wet / Dry"]:::dir
        N6["Freeze Latch\nenc push"]:::dir
        N7["Menu Cursor"]:::dir
        N8["Edit  Commit"]:::dir
        N9["Back  Cancel"]:::dir
        N10["Confirm"]:::dir
    end

    subgraph SHIFT["Shift +  SW1 held"]
        S1["Delay\nSubdivision"]:::alt
        S2["FB Tone\nDamping"]:::alt
        S3["Delay Mode\nSync ↔ Free"]:::alt
        S4["Drive\nAmount"]:::alt
        S5["Page Jump\nP1→P2→P3→P4"]:::alt
    end

    K1  --> N1
    K2  --> N2
    SW1 -->|short tap| N3
    SW2 --> N4
    PE  -->|turn| N5
    PE  -->|push| N6
    EE  --> N7
    EP  --> N8
    BK  --> N9
    CN  --> N10

    K1  -.->|Shift +| S1
    K2  -.->|Shift +| S2
    SW2 -.->|Shift +| S3
    PE  -.->|Shift + turn| S4
    PE  -.->|Shift + push| S5
```

---

## Map 3 — DSP Chain + Control Assignments

Main signal path (left → right). Feedback loop shown. Dashed lines = which controls
or pages govern each DSP stage.

```mermaid
flowchart LR
    classDef io   fill:#003300,stroke:#66bb6a,color:#fff,font-weight:bold
    classDef dsp  fill:#0d1b2a,stroke:#1e88e5,color:#fff
    classDef fb   fill:#0d1825,stroke:#546e7a,color:#ccc
    classDef frz  fill:#001c28,stroke:#4dd0e1,color:#fff
    classDef ctrl fill:#1a0800,stroke:#ff8a65,color:#ddd

    IN["IN\nMono"]:::io

    TR["Trim\nGain"]:::dsp
    DC["DC Block\nDcBlock"]:::dsp
    HP["Input HP\nSvf"]:::dsp
    DV["Drive\nOverdrive"]:::dsp

    DL(["DELAY\nDelayLine\nSDRAM"]):::dsp

    FH["FB HP\nSvf"]:::fb
    FL["FB LP\nSvf"]:::fb
    WF["Wow\nPhasor"]:::fb
    FS["FB Sat\ntanh"]:::fb

    RV["Diffusion\nReverbSc"]:::dsp
    MX["Wet/Dry\nCrossFade"]:::dsp
    TL["Tilt\nBiquad"]:::dsp
    LM["Limiter"]:::dsp
    OUT["OUT\nL + R"]:::io

    FZ(["FREEZE\nEngine"]):::frz

    CA["P2: Input HP\ncutoff Hz"]:::ctrl
    CB["Shift+Enc\nDrive 0–100%"]:::ctrl
    CC["K1: Time · Subdiv\nK2: Feedback\nSW2: BPM tap"]:::ctrl
    CD["P2: FB LP · HP"]:::ctrl
    CE["P3: Wow Depth\nP3: Wow Rate"]:::ctrl
    CF["P2: Wet Tone\nDamping Hz"]:::ctrl
    CG["Pod Enc: Mix\nSW1/Enc: Freeze"]:::ctrl
    CH["P2: Output Tilt"]:::ctrl

    IN --> TR --> DC --> HP --> DV --> DL

    DL -->|feedback| FH --> FL --> WF --> FS --> DL
    DL -->|wet| RV --> MX
    DL -->|dry| MX
    MX --> TL --> LM --> OUT

    FZ -.->|freeze flag| DL

    CA -.-> HP
    CB -.-> DV
    CC -.-> DL
    CD -.-> FH & FL
    CE -.-> WF
    CF -.-> RV
    CG -.-> MX & FZ
    CH -.-> TL
```
