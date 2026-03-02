# Daisy Field Project Ideas (Complexity 5-10)

Generated based on DSP literature and the DVPE Module Catalog.
**Date**: 2026-02-08

## Color Coding Reference
- <span style="color:#2980b9">**Blue**</span>: Audio Signal (Sources, Effects, Filters)
- <span style="color:#e67e22">**Orange**</span>: Control Signals (Envelopes, LFOs, Modulation)
- <span style="color:#8e44ad">**Violet**</span>: Math & Utility (Mixers, Logic)
- <span style="color:#27ae60">**Green**</span>: User I/O (Hardware Controls)

---

## 1. Spectral Freeze & Glitch Texture
**Complexity**: 8/10  
**Description**: Real-time granular freezing and spectral glitching of incoming audio. Based on granular synthesis concepts from *Microsound (Roads)*.  
**Controls**:
- **Knob 1**: Freeze Threshold
- **Knob 2**: Grain Size
- **Knob 3**: Pitch Shift
- **Knob 4**: Texture/Scatter
- **Knob 5**: Reverb Mix
- **Button 1**: Freeze Momentary

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Particle[PARTICLE]:::audio
    Knob1[KNOB 1: Freeze]:::io --> Gate[GATE]:::math
    Gate --> Particle
    Knob2[KNOB 2: Grain]:::io --> Particle
    Knob3[KNOB 3: Pitch]:::io --> Particle
    Particle --> Reverb[REVERB]:::audio
    Knob5[KNOB 5: Mix]:::io --> Reverb
    Reverb --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 2. Euclidean Polyrhythmic Drum Machine
**Complexity**: 9/10
**Description**: 3-track generative drum sequencer using Euclidean algorithms. Concepts from *Godfried Toussaint's Geometry of Musical Rhythm*.
**Controls**:
- **Knob 1-3**: Density (Kick, Snare, Hat)
- **Knob 4-6**: Decay (Kick, Snare, Hat)
- **Knob 7**: Tempo
- **Knob 8**: Master Distortion

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Clock[LFO: Clock]:::control --> Seq1[EUCLIDEAN SEQ 1]:::math
    Clock --> Seq2[EUCLIDEAN SEQ 2]:::math
    Clock --> Seq3[EUCLIDEAN SEQ 3]:::math
    
    Knob1[KNOB 1: Dens K]:::io --> Seq1
    Knob2[KNOB 2: Dens S]:::io --> Seq2
    Knob3[KNOB 3: Dens H]:::io --> Seq3

    Seq1 --> Kick[ANALOG KICK]:::audio
    Seq2 --> Snare[ANALOG SNARE]:::audio
    Seq3 --> Hat[HI-HAT]:::audio

    Knob4[KNOB 4: Dec K]:::io --> Kick
    Knob5[KNOB 5: Dec S]:::io --> Snare
    Knob6[KNOB 6: Dec H]:::io --> Hat

    Kick --> Mixer[MIXER 3ch]:::math
    Snare --> Mixer
    Hat --> Mixer

    Mixer --> Dist[DISTORTION]:::audio
    Knob8[KNOB 8: Drive]:::io --> Dist
    Dist --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 3. Wavetable Morphing Synthesizer
**Complexity**: 7/10
**Description**: Classic wavetable synthesis with LFO-driven table morphing. Inspired by *Pirkle's Designing Software Synthesizers*.
**Controls**:
- **Knob 1**: Morph Position
- **Knob 2**: Filter Cutoff
- **Knob 3**: Resonance
- **Knob 4**: LFO Body
- **Keys**: Pitch/Gate

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Midi[MIDI IN]:::io --> MtoF[MIDI -> Hz]:::math
    Midi --> Adsr[ADSR ENVELOPE]:::control

    MtoF --> Osc[VAR SHAPE OSC]:::audio
    
    LFO[LFO]:::control --> Osc
    Knob1[KNOB 1: Morph]:::io --> Osc
    
    Osc --> Filter[MOOG LADDER]:::audio
    Knob2[KNOB 2: Cutoff]:::io --> Filter
    Knob3[KNOB 3: Res]:::io --> Filter

    Filter --> VCA[LINEAR VCA]:::audio
    Adsr --> VCA
    VCA --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 4. Physical Modeling Flute
**Complexity**: 8/10
**Description**: Waveguide synthesis simulation of a wind instrument. Based on *Perry Cook's Real Sound Synthesis*.
**Controls**:
- **Knob 1**: Breath Pressure (Noise gain)
- **Knob 2**: Tube Length (Pitch)
- **Knob 3**: Jet Delay
- **Knob 4**: Output Level

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Noise[WHITE NOISE]:::audio --> VCA[LINEAR VCA]:::audio
    Knob1[KNOB 1: Breath]:::io --> VCA
    
    VCA --> Filter[LowPass]:::audio
    Filter --> Delay[DELAY LINE]:::audio
    Knob2[KNOB 2: Pitch]:::io --> Delay

    Delay --> Saturation[SOFTCLIP]:::audio
    Saturation --> Filter
    
    Filter --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 5. Dual-Band Compressor
**Complexity**: 6/10
**Description**: Mastering-style dynamics processor splitting audio into Low/High bands.
**Controls**:
- **Knob 1**: Crossover Freq
- **Knob 2**: Low Threshold
- **Knob 3**: High Threshold
- **Knob 4**: Makeup Gain

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Split[SPLITTER]:::math
    Knob1[KNOB 1: X-Over]:::io --> LPF[LPF (1-POLE)]:::audio
    Knob1 --> HPF[HPF (1-POLE)]:::audio
    
    Split --> LPF
    Split --> HPF

    LPF --> CompL[COMPRESSOR]:::audio
    HPF --> CompH[COMPRESSOR]:::audio

    Knob2[KNOB 2: Thresh L]:::io --> CompL
    Knob3[KNOB 3: Thresh H]:::io --> CompH

    CompL --> Mix[ADD]:::math
    CompH --> Mix
    Mix --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 6. Stereo Image Widener & Mid-Side Proc
**Complexity**: 6/10
**Description**: Manipulates the stereo field using Mid-Side encoding/decoding.
**Controls**:
- **Knob 1**: Width (Side Gain)
- **Knob 2**: Center (Mid Gain)
- **Knob 3**: HPF Side (remove mud)
- **Knob 4**: Saturation Side

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Enc[M/S Encoder]:::math
    Enc --> Mid[Mid Ch]:::audio
    Enc --> Side[Side Ch]:::audio

    Knob2[KNOB 2: Mid Vol]:::io --> Mid
    
    Side --> HPF[HPF]:::audio
    Knob3[KNOB 3: HPF]:::io --> HPF
    HPF --> Sat[SATURATION]:::audio
    Knob1[KNOB 1: Width]:::io --> Sat

    Mid --> Dec[M/S Decoder]:::math
    Sat --> Dec
    Dec --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 7. Formant Vocal Synthesizer
**Complexity**: 7/10
**Description**: Generates vowel sounds using Formant Synthesis (FOF/VOSIM).
**Controls**:
- **Knob 1**: Vowel A-E-I-O-U (Formant Freqs)
- **Knob 2**: Pitch
- **Knob 3**: Vibrato Depth
- **Knob 4**: Throat Size (Shift)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Knob2[KNOB 2: Pitch]:::io --> Osc[VOSIM OSC]:::audio
    Knob1[KNOB 1: Vowel]:::io --> FormantMap[MAP VOWEL]:::math
    
    FormantMap --> Osc
    
    LFO[LFO: Vibrato]:::control --> Osc
    Knob3[KNOB 3: Vib Depth]:::io --> LFO

    Osc --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 8. Resonant Filter Bank
**Complexity**: 8/10
**Description**: 8 parallel bandpass filters for vocoder-like or spectral shaping effects.
**Controls**:
- **Knob 1**: Base Freq
- **Knob 2**: Spacing
- **Knob 3**: Resonance
- **Knob 4-8**: Band Gains

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Split[Splitter]:::math
    
    Split --> BP1[SVF Band 1]:::audio
    Split --> BP2[SVF Band 2]:::audio
    Split --> BP3[SVF Band 3]:::audio
    Split --> BP4[SVF Band 4]:::audio

    Knob1[KNOB 1: Base]:::io --> FreqMath[Math: Spacing]:::math
    Knob2[KNOB 2: Spread]:::io --> FreqMath
    
    FreqMath --> BP1
    FreqMath --> BP2
    FreqMath --> BP3
    FreqMath --> BP4

    BP1 --> Mixer[MIXER 4ch]:::math
    BP2 --> Mixer
    BP3 --> Mixer
    BP4 --> Mixer

    Mixer --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 9. Lo-Fi Tape Deck
**Complexity**: 6/10
**Description**: Simulates tape degradation with wow, flutter, noise, and saturation.
**Controls**:
- **Knob 1**: Wow/Flutter (LFO Depth)
- **Knob 2**: Hiss Level
- **Knob 3**: Saturation (Drive)
- **Knob 4**: Tone (Filter)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Delay[DELAY (Modulated)]:::audio
    LFO[LFO: Flutter]:::control --> Delay
    Knob1[KNOB 1: Wow]:::io --> LFO

    Noise[WHITE NOISE]:::audio --> Mix1[ADD]:::math
    Knob2[KNOB 2: Hiss]:::io --> Noise
    Delay --> Mix1
    Noise --> Mix1
    
    Mix1 --> Sat[OVERDRIVE]:::audio
    Knob3[KNOB 3: Drive]:::io --> Sat

    Sat --> Tone[TONE STACK]:::audio
    Knob4[KNOB 4: Tone]:::io --> Tone
    
    Tone --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 10. Generative Ambient Garden
**Complexity**: 9/10
**Description**: Self-playing patch using probabilistic logic and Modal synthesis.
**Controls**:
- **Knob 1**: Density / Activity
- **Knob 2**: Scale Root
- **Knob 3**: Reverb Size
- **Knob 4**: Harmony Spread

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Clock[Random Clock]:::control --> Logic[Turing Machine Logic]:::math
    Logic --> Quantizer[Scale Quantizer]:::math
    Knob2[KNOB 2: Root]:::io --> Quantizer
    
    Quantizer --> Voice1[MODAL VOICE 1]:::audio
    Quantizer --> Voice2[MODAL VOICE 2]:::audio

    Voice1 --> Reverb[FDN REVERB]:::audio
    Voice2 --> Reverb
    
    Knob3[KNOB 3: Size]:::io --> Reverb
    Reverb --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 11. Phase Vocoder Time-Stretcher
**Complexity**: 10/10
**Description**: Frequency-domain time stretching of real-time audio. From *DAFX (Zolzer)*.
**Controls**:
- **Knob 1**: Stretch Factor
- **Knob 2**: Pitch Shift
- **Knob 3**: Window Size
- **Knob 4**: Dry/Wet

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> FFT[FFT Analysis]:::math
    
    FFT --> PhaseProc[Phase Unwrapping]:::math
    FFT --> MagProc[Magnitude Interp]:::math
    
    Knob1[KNOB 1: Stretch]:::io --> PhaseProc
    Knob2[KNOB 2: Pitch]:::io --> PhaseProc

    PhaseProc --> IFFT[Inverse FFT]:::math
    MagProc --> IFFT

    IFFT --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 12. FM Percussion Synth
**Complexity**: 7/10
**Description**: 2-Operator FM synthesis tailored for metallic percussion.
**Controls**:
- **Knob 1**: Carrier Freq
- **Knob 2**: Modulator Ratio
- **Knob 3**: FM Index (Amount)
- **Knob 4**: Decay Time

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Trig[GATE IN]:::io --> Env[AD ENVELOPE]:::control
    Knob4[KNOB 4: Decay]:::io --> Env

    Env --> FM[FM2 OPERATOR]:::audio
    Knob1[KNOB 1: Freq]:::io --> FM
    Knob2[KNOB 2: Ratio]:::io --> FM
    Knob3[KNOB 3: Index]:::io --> FM

    FM --> Dist[SOFTCLIP]:::audio
    Dist --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 13. Super Saw Pluck
**Complexity**: 6/10
**Description**: Massive detuned saw stack for trance/dance leads.
**Controls**:
- **Knob 1**: Detune Amount
- **Knob 2**: Cutoff Frequency
- **Knob 3**: Pluck Decay
- **Knob 4**: Reverb

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Midi[MIDI NOTE]:::io --> OscBank[OSC BANK (7x)]:::audio
    Knob1[KNOB 1: Detune]:::io --> OscBank
    
    Midi --> Env[AD ENVELOPE]:::control
    Knob3[KNOB 3: Decay]:::io --> Env

    OscBank --> Filter[MOOG LADDER]:::audio
    Env --> Filter
    Knob2[KNOB 2: Cutoff]:::io --> Filter

    Filter --> Reverb[REVERB]:::audio
    Reverb --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 14. Karplus-Strong String Ensemble
**Complexity**: 8/10
**Description**: Physical modeling of plucked strings using filtered delay loops.
**Controls**:
- **Knob 1**: String Tension (Pitch)
- **Knob 2**: Damping (Filter)
- **Knob 3**: Pluck Pos (Comb Filter)
- **Knob 4**: Body Resonance

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Burst[Noise Burst]:::control --> Pluck[PLUCK PH_MODEL]:::audio
    Knob1[KNOB 1: Freq]:::io --> Pluck
    Knob2[KNOB 2: Damp]:::io --> Pluck

    Pluck --> Chorus[CHORUS]:::audio
    Knob3[KNOB 3: Body]:::io --> Chorus
    
    Chorus --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 15. Chaos-Modulated Drone
**Complexity**: 9/10
**Description**: Drone synthesizer where parameters are controlled by chaotic attractors (Lorenz/Rossler).
**Controls**:
- **Knob 1**: Chaos Rate
- **Knob 2**: Pitch Spread
- **Knob 3**: Timbre (FM)
- **Knob 4**: Space (Reverb)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Chaos[Chaos Generator]:::control --> Osc1[OSCILLATOR 1]:::audio
    Chaos --> Osc2[OSCILLATOR 2]:::audio
    Chaos --> Filter[SVF FILTER]:::audio

    Knob1[KNOB 1: Rate]:::io --> Chaos
    
    Osc1 --> Ring[RING MOD]:::math
    Osc2 --> Ring
    Ring --> Filter
    
    Filter --> Reverb[FDN REVERB]:::audio
    Knob4[KNOB 4: Space]:::io --> Reverb

    Reverb --> AudioOut[AUDIO OUTPUT]:::io
```
