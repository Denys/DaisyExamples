# Field_MI_Plaits Dependencies

This document tracks the code dependencies for the current `Field_MI_Plaits` Daisy Field build.

Scope:
- This is the dependency view for the current compiled project in `DaisyExamples/MyProjects/_projects/Field_MI_Plaits`.
- It focuses on build inputs, runtime flow, active Plaits engine dependencies, the banked-parameter helper layer, and the resource tables that are still relevant to the reduced-engine build.
- It does not attempt to fully diagram every unused file still present in `PlaitsPatchInit`; disabled engines and disabled resource families are shown separately.

## 1. Build And Link Dependency Graph

```mermaid
flowchart TD
    MK[Field_MI_Plaits/Makefile]

    APP[Field_MI_Plaits.cpp]
    FD[field_defaults.h]
    PFB[field_parameter_banks.h]
    FI[field_instrument_ui.h]
    DF[daisy_field.h / libDaisy]
    DSP[daisysp.h / DaisySP]

    VH[plaits/dsp/voice.h]
    VC[plaits/dsp/voice.cc]
    RH[plaits/resources.h]
    RC[plaits/resources.cc]

    STU[stmlib/dsp/units.cc]
    STR[stmlib/utils/random.cc]

    E1[virtual_analog_engine.cc]
    E2[fm_engine.cc]
    E3[grain_engine.cc]
    E4[additive_engine.cc]
    E5[swarm_engine.cc]
    E6[particle_engine.cc]
    E7[bass_drum_engine.cc]
    E8[snare_drum_engine.cc]
    E9[hi_hat_engine.cc]

    LIB[libDaisy static library]
    DSPLIB[DaisySP static library]

    MK --> APP
    MK --> VC
    MK --> RC
    MK --> STU
    MK --> STR
    MK --> E1
    MK --> E2
    MK --> E3
    MK --> E4
    MK --> E5
    MK --> E6
    MK --> E7
    MK --> E8
    MK --> E9
    MK --> LIB
    MK --> DSPLIB

    APP --> FD
    APP --> PFB
    APP --> FI
    APP --> DF
    APP --> DSP
    APP --> VH

    FI --> FD
    FI --> DF

    VC --> VH
    RC --> RH

    VH --> RH
    VH --> E1
    VH --> E2
    VH --> E3
    VH --> E4
    VH --> E5
    VH --> E6
    VH --> E7
    VH --> E8
    VH --> E9

    APP --> LIB
    APP --> DSPLIB
```

## 2. Runtime Control And Audio Flow

```mermaid
flowchart TD
    MIDI[External TRS MIDI / sequencer]
    CTRLS[Daisy Field hardware controls]
    MAIN[main loop]
    AUDIO[AudioCallback]

    HM[HandleMidiMessage]
    UHS[UpdateHoldState]
    RK[ReadKnobs]
    BANK[SetActiveKnobBank]
    PNK[ProcessBankKnobs]
    PK[ProcessKeys]
    DISP[UpdateDisplay]
    LEDS[RefreshLeds / key_leds.Update]

    MS[MidiState]
    PS[Params]
    ZS[ParamZoomState]

    UPS[UpdatePatchState]
    PATCH[plaits::Patch]
    MODS[plaits::Modulations]
    VOICE[plaits::Voice::Render]
    ENG[EngineRegistry active engine]
    POST[ChannelPostProcessor + LPG/decay envelopes]
    OUT[Field stereo outputs]

    MIDI --> MAIN
    CTRLS --> MAIN

    MAIN --> HM
    MAIN --> UHS
    MAIN --> RK
    MAIN --> BANK
    MAIN --> PNK
    MAIN --> PK
    MAIN --> DISP
    MAIN --> LEDS

    HM --> MS
    BANK --> ZS
    BANK --> LEDS
    PNK --> PS
    UHS --> PS
    PK --> PS
    PNK --> ZS
    ZS --> DISP
    PS --> DISP
    MS --> DISP
    PS --> LEDS

    AUDIO --> UPS
    PS --> UPS
    MS --> UPS
    UPS --> PATCH
    UPS --> MODS
    PATCH --> VOICE
    MODS --> VOICE
    VOICE --> ENG
    ENG --> POST
    POST --> OUT
```

## 3. Field Application File-Level Dependency Graph

```mermaid
flowchart LR
    subgraph FieldApp[Field_MI_Plaits.cpp]
        MAIN2[main()]
        LOOP[main loop helpers]
        CB[AudioCallback]
        DRAW[OLED draw helpers]
    end

    subgraph Helpers[Field helper headers]
        FD2[field_defaults.h]
        PFB2[field_parameter_banks.h]
        FI2[field_instrument_ui.h]
        LEDBANK[OneHotKeyLedBank]
        ZOOM[ParamZoomState]
        FORMAT[FormatPercent / note-name helpers]
    end

    subgraph PlaitsCore[Plaits wrapper objects]
        VOBJ[plaits::Voice]
        POBJ[plaits::Patch]
        MOBJ[plaits::Modulations]
        AFR[plaits::Voice::Frame buffer]
    end

    MAIN2 --> LOOP
    MAIN2 --> CB
    LOOP --> DRAW

    MAIN2 --> FD2
    MAIN2 --> PFB2
    MAIN2 --> FI2
    FI2 --> LEDBANK
    FI2 --> ZOOM
    FD2 --> FORMAT

    MAIN2 --> VOBJ
    MAIN2 --> POBJ
    MAIN2 --> MOBJ
    CB --> AFR
    CB --> VOBJ
    LOOP --> POBJ
    LOOP --> MOBJ
```

## 4. Plaits Voice Dependency Graph

```mermaid
flowchart TD
    VH[voice.h]
    VC[voice.cc]

    PATCH2[Patch]
    MODS2[Modulations]
    REG[EngineRegistry]
    DENV[DecayEnvelope]
    LPG[LPGEnvelope]
    TDL[trigger DelayLine]
    OPP[ChannelPostProcessor out]
    APP2[ChannelPostProcessor aux]

    ENVH[envelope.h]
    LPGH[fx/low_pass_gate.h]
    DLH[physical_modelling/delay_line.h]
    STML[stmlib filter/limiter/buffer allocator]

    VC --> VH
    VH --> PATCH2
    VH --> MODS2
    VH --> REG
    VH --> DENV
    VH --> LPG
    VH --> TDL
    VH --> OPP
    VH --> APP2

    VH --> ENVH
    VH --> LPGH
    VH --> DLH
    VH --> STML
```

## 5. Active Engine Dependency Graph

```mermaid
flowchart TD
    REG2[Voice EngineRegistry]

    VA[VirtualAnalogEngine]
    FM[FMEngine]
    GRAIN[GrainEngine]
    ADD[AdditiveEngine]
    SWARM[SwarmEngine]
    PART[ParticleEngine]
    BD[BassDrumEngine]
    SD[SnareDrumEngine]
    HH[HiHatEngine]

    REG2 --> VA
    REG2 --> FM
    REG2 --> GRAIN
    REG2 --> ADD
    REG2 --> SWARM
    REG2 --> PART
    REG2 --> BD
    REG2 --> SD
    REG2 --> HH

    VA --> VSO[VariableShapeOscillator]
    VA --> VSAW[VariableSawOscillator]

    FM --> FMSTATE[phase accumulators / feedback state]
    FM --> RSRC1[resources.h]

    GRAIN --> GLET[GrainletOscillator]
    GRAIN --> ZOSC[ZOscillator]
    GRAIN --> OPF[stmlib::OnePole]

    ADD --> HOSC[HarmonicOscillator]

    SWARM --> OSC[Oscillator]
    SWARM --> SSO[StringSynthOscillator]
    SWARM --> SINEOSC[SineOscillator]
    SWARM --> RSRC2[resources.h]
    SWARM --> RAND[stmlib::Random]

    PART --> NOISE[Particle noise objects]
    PART --> DIFF[Diffuser]
    PART --> SVF[stmlib::Svf]

    BD --> ABD[AnalogBassDrum]
    BD --> SBD[SyntheticBassDrum]
    BD --> ODRV[Overdrive]
    BD --> SRR[SampleRateReducer]

    SD --> ASD[AnalogSnareDrum]
    SD --> SSD[SyntheticSnareDrum]

    HH --> HH1[HiHat SquareNoise branch]
    HH --> HH2[HiHat RingModNoise branch]
```

## 6. Resource Dependency Graph

```mermaid
flowchart TD
    RH2[resources.h]
    RC2[resources.cc]

    SINE[lut_sine]
    FMQ[lut_fm_frequency_quantizer]
    LTT[lookup_table_table]

    SO[SineOscillator]
    ZO[ZOscillator]
    GLO[GrainletOscillator]
    FMO[FMEngine]
    SWE[SwarmEngine]

    RH2 --> RC2
    RC2 --> SINE
    RC2 --> FMQ
    RC2 --> LTT

    SINE --> SO
    SINE --> ZO
    SINE --> GLO
    SINE --> FMO
    SINE --> SWE
    FMQ --> FMO
```

## 7. Current Disabled Dependency Families

These files still exist in the source tree, but they are not part of the current reduced `Field_MI_Plaits` build path.

```mermaid
flowchart TD
    DIS[Disabled in current build]

    WAVESHAPE[WaveshapingEngine]
    STRING[StringEngine]
    MODAL[ModalEngine]
    WTBL[WavetableEngine]
    CHORD[ChordEngine]
    SPEECH[SpeechEngine]
    NOISE[NoiseEngine]

    WSRES[fold/ws lookup tables]
    PMRES[physical modelling string/modal chain]
    WRES[wavetable resources]
    LPC[lpc speech tables]

    DIS --> WAVESHAPE
    DIS --> STRING
    DIS --> MODAL
    DIS --> WTBL
    DIS --> CHORD
    DIS --> SPEECH
    DIS --> NOISE

    WAVESHAPE --> WSRES
    STRING --> PMRES
    MODAL --> PMRES
    WTBL --> WRES
    CHORD --> WRES
    SPEECH --> LPC
```

## 8. Original Slot Layout To Active Build Mapping

This is not a compile dependency, but it is an important selection dependency in `Field_MI_Plaits.cpp` because the UI keeps the original Plaits 16-slot layout while only some slots map to live engines.

```mermaid
flowchart LR
    A1[A1 PAIR] --> E0[Engine 0 VirtualAnalogEngine]
    A2[A2 SHAPE] --> X1[disabled]
    A3[A3 FM] --> E1[Engine 1 FMEngine]
    A4[A4 FORMANT] --> E2[Engine 2 GrainEngine]
    A5[A5 HARM] --> E3[Engine 3 AdditiveEngine]
    A6[A6 WTBL] --> X2[disabled]
    A7[A7 CHORD] --> X3[disabled]
    A8[A8 SPEECH] --> X4[disabled]

    B1[B1 CLOUD] --> E4[Engine 4 SwarmEngine]
    B2[B2 FILT] --> X5[disabled]
    B3[B3 PART] --> E5[Engine 5 ParticleEngine]
    B4[B4 STRING] --> X6[disabled]
    B5[B5 MODAL] --> X7[disabled]
    B6[B6 BD] --> E6[Engine 6 BassDrumEngine]
    B7[B7 SNARE] --> E7[Engine 7 SnareDrumEngine]
    B8[B8 HHAT] --> E8[Engine 8 HiHatEngine]
```

## 9. Dependency Notes

- `Field_MI_Plaits.cpp` is the orchestration layer. It owns hardware init, MIDI polling, control scanning, OLED drawing, LED refresh, and translation into `plaits::Patch` / `plaits::Modulations`.
- `field_defaults.h` is the hardware mapping layer for key indices, LEDs, and shared Daisy Field constants.
- `field_parameter_banks.h` stores the main and alt knob banks, active bank selection, and pickup/catch state.
- `field_instrument_ui.h` provides the narrow UI helpers used by this firmware: zoom-state tracking, one-hot key LED handling, baseline management, and formatting helpers.
- `voice.h` / `voice.cc` are the integration boundary between the Field wrapper and the Mutable Plaits DSP engines.
- `resources.h` / `resources.cc` provide lookup tables shared by multiple oscillators and engines.
- `units.cc` and `random.cc` come from `stmlib` and are linked explicitly by the project `Makefile`.
- `libDaisy` provides the board, audio, MIDI, ADC, display, and timing APIs used by the app.
- `DaisySP` is included at the application layer even though the current reduced Plaits wrapper is primarily driven by Plaits DSP and `stmlib`.

## 11. Banked Control Notes

- Main and alt knob values are stored separately in `ParamBankSet`.
- `SW1` switches the active bank only; it does not copy values between banks.
- Knob LEDs are rendered from stored logical values, not from raw knob positions.
- `ParamZoomState` follows the active stored values so the zoom UI stays consistent with pickup/catch behavior.
- The audio callback stays DSP-only; all scanning, pickup logic, and OLED work remain in the main loop.

## 10. Maintenance Checklist

When the project changes, update this file if any of the following happen:

- a new engine source is added to or removed from `Field_MI_Plaits/Makefile`
- `voice.h` or `voice.cc` changes the registered engine set
- `Field_MI_Plaits.cpp` changes the 16-slot map in `kEngineSlotToEngine`
- a previously disabled resource family is re-enabled in `resources.cc`
- new shared helper headers are added between the app and the Plaits core
