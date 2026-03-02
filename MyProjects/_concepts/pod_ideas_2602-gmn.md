# Daisy Pod Project Ideas (Complexity 5-10)

Generated based on DSP literature and the DVPE Module Catalog.
**Date**: 2026-02-08

## Color Coding Reference
- <span style="color:#2980b9">**Blue**</span>: Audio Signal (Sources, Effects, Filters)
- <span style="color:#e67e22">**Orange**</span>: Control Signals (Envelopes, LFOs, Modulation)
- <span style="color:#8e44ad">**Violet**</span>: Math & Utility (Mixers, Logic)
- <span style="color:#27ae60">**Green**</span>: User I/O (Hardware Controls)

---

## 1. Shimmer Reverb
**Complexity**: 8/10
**Description**: Ethereal reverb with pitch-shifted feedback loop. Inspired by *Valhalla DSP* algorithms.
**Controls**:
- **Knob 1**: Decay Time
- **Knob 2**: Shimmer Amount (Pitch Mix)
- **Button 1**: Octave Up/Down

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Reverb[FDN REVERB]:::audio
    Reverb --> Split[Splitter]:::math
    
    Split --> Pitch[PITCHSHIFT +12]:::audio
    Button1[BTN 1: Octave]:::io --> Pitch
    
    Pitch --> Feedback[Feedback Loop]:::math
    Knob2[KNOB 2: Shimmer]:::io --> Feedback
    Feedback --> Reverb

    Reverb --> AudioOut[AUDIO OUTPUT]:::io
    Knob1[KNOB 1: Decay]:::io --> Reverb
```

---

## 2. Auto-Wah Funky Filter
**Complexity**: 6/10
**Description**: Envelope-controlled bandpass filter for funk guitar/bass.
**Controls**:
- **Knob 1**: Sensitivity (Threshold)
- **Knob 2**: Resonance (Q)
- **Button 1**: Filter Type (BP/LP)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Split[Splitter]:::math
    Split --> Env[ENV FOLLOWER]:::control
    Knob1[KNOB 1: Sense]:::io --> Env

    Split --> Filter[SVF FILTER]:::audio
    Env --> Filter
    Knob2[KNOB 2: Res]:::io --> Filter

    Filter --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 3. Fuzz Distortion
**Complexity**: 5/10
**Description**: Aggressive hard-clipping distortion with tone control.
**Controls**:
- **Knob 1**: Drive (Gain)
- **Knob 2**: Tone (Filter)
- **Button 1**: Clip Mode (Hard/Soft)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Gain[LINEAR VCA]:::audio
    Knob1[KNOB 1: Drive]:::io --> Gain
    
    Gain --> Clip[HARDCLIP]:::audio
    Button1[BTN 1: Mode]:::io --> Clip
    
    Clip --> Tone[TONE STACK]:::audio
    Knob2[KNOB 2: Tone]:::io --> Tone

    Tone --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 4. Bitcrusher & Downsampler
**Complexity**: 5/10
**Description**: Digital degradation effect for lo-fi textures.
**Controls**:
- **Knob 1**: Sample Rate (Decimate)
- **Knob 2**: Bit Depth
- **Button 1**: Bypass

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Crusher[DECIMATOR]:::audio
    Knob1[KNOB 1: Rate]:::io --> Crusher
    Knob2[KNOB 2: Bits]:::io --> Crusher

    Crusher --> Bypass[BYPASS SW]:::math
    Button1[BTN 1: Bypass]:::io --> Bypass
    
    Bypass --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 5. Optical Tremolo
**Complexity**: 6/10
**Description**: Amplitude modulation simulating vintage optical tremolo circuits.
**Controls**:
- **Knob 1**: Rate (Speed)
- **Knob 2**: Depth (Intensity)
- **Button 1**: Waveform (Sine/Square)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    LFO[LFO]:::control --> VCA[LINEAR VCA]:::audio
    Knob1[KNOB 1: Rate]:::io --> LFO
    Knob2[KNOB 2: Depth]:::io --> LFO
    Button1[BTN 1: Wave]:::io --> LFO

    AudioIn[AUDIO INPUT]:::io --> VCA
    VCA --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 6. Sub-Octave Generator
**Complexity**: 7/10
**Description**: Adds a synthesized bass note one octave below the input signal.
**Controls**:
- **Knob 1**: Sub Octave Level
- **Knob 2**: Dry Signal Level
- **Button 1**: Octave Select (-1/-2)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Pitch[PITCHSHIFT]:::audio
    Button1[BTN 1: Oct]:::io --> Pitch
    
    AudioIn --> Mix[MIXER]:::math
    Pitch --> Mix

    Knob1[KNOB 1: Sub Vol]:::io --> Pitch
    Knob2[KNOB 2: Dry Vol]:::io --> AudioIn

    Mix --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 7. Triple-Oscillator Drone
**Complexity**: 7/10
**Description**: Three detuned oscillators for thick drone textures.
**Controls**:
- **Knob 1**: Pitch
- **Knob 2**: Detune Spread
- **Button 1**: Waveform Select

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Knob1[KNOB 1: Pitch]:::io --> Osc1[OSC 1]:::audio
    Knob1 --> Osc2[OSC 2]:::audio
    Knob1 --> Osc3[OSC 3]:::audio

    Knob2[KNOB 2: Detune]:::io --> DetuneLogic[Math: Offset]:::math
    DetuneLogic --> Osc2
    DetuneLogic --> Osc3

    Osc1 --> Mix[MIXER]:::math
    Osc2 --> Mix
    Osc3 --> Mix

    Mix --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 8. Filtered Noise Generator
**Complexity**: 5/10
**Description**: White noise source with a sweeping bandpass filter, useful for risers/fx.
**Controls**:
- **Knob 1**: Filter Cutoff
- **Knob 2**: Resonance
- **Button 1**: Noise Color (White/Pink/Red)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Noise[NOISE SOURCE]:::audio --> Filter[SVF FILTER]:::audio
    Button1[BTN 1: Color]:::io --> Noise
    
    Knob1[KNOB 1: Cutoff]:::io --> Filter
    Knob2[KNOB 2: Res]:::io --> Filter

    Filter --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 9. LFO Modulator Utility
**Complexity**: 6/10
**Description**: Outputs CV signals (0-3.3V) to control other voltage-controlled gear.
**Controls**:
- **Knob 1**: Rate
- **Knob 2**: Shape Skew
- **Buttons**: Waveform Select

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Knob1[KNOB 1: Rate]:::io --> LFO[LFO]:::control
    Button1[BTN 1: Wave]:::io --> LFO
    
    LFO --> Scale[Math: Unipolar]:::math
    Scale --> LED[RGB LED]:::io
    Scale --> CVOut[CV OUTPUT]:::io
```

---

## 10. Ping Pong Delay
**Complexity**: 7/10
**Description**: Stereo delay bouncing repeats between left and right channels.
**Controls**:
- **Knob 1**: Delay Time
- **Knob 2**: Feedback
- **Button 1**: Tap Tempo

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Split[Splitter]:::math
    
    Split --> DelayL[DELAY Left]:::audio
    Split --> DelayR[DELAY Right]:::audio

    DelayL --> DelayR
    DelayR --> DelayL

    Knob1[KNOB 1: Time]:::io --> DelayL
    Knob1 --> DelayR
    Knob2[KNOB 2: Fdbk]:::io --> DelayL
    Knob2 --> DelayR

    DelayL --> AudioOutL[OUT LEFT]:::io
    DelayR --> AudioOutR[OUT RIGHT]:::io
```

---

## 11. Kick Drum Synth (Trigger)
**Complexity**: 5/10
**Description**: Dedicated analog-style kick drum synthesizer triggered by button or gate.
**Controls**:
- **Knob 1**: Tuning
- **Knob 2**: Decay
- **Button 1**: Manual Trigger

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Button1[BTN 1: Trig]:::io --> Kick[ANALOG KICK]:::audio
    GateIn[GATE IN]:::io --> Kick
    
    Knob1[KNOB 1: Tune]:::io --> Kick
    Knob2[KNOB 2: Decay]:::io --> Kick

    Kick --> Dist[SOFTCLIP]:::audio
    Dist --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 12. Stereo Chorus
**Complexity**: 7/10
**Description**: Widens the stereo image using modulated delay lines.
**Controls**:
- **Knob 1**: Rate
- **Knob 2**: Depth
- **Button 1**: Voice Count (2/4)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Chorus[CHORUS]:::audio
    Knob1[KNOB 1: Rate]:::io --> Chorus
    Knob2[KNOB 2: Depth]:::io --> Chorus

    Chorus --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 13. Vocal Robot Voice
**Complexity**: 8/10
**Description**: Robotic voice effect using FFT-based robotization.
**Controls**:
- **Knob 1**: Pitch/Frequency
- **Knob 2**: Dry/Wet Mix
- **Button 1**: Freeze Buffer

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO INPUT]:::io --> Robot[ROBOTIZATION]:::audio
    Knob1[KNOB 1: Pitch]:::io --> Robot
    
    Robot --> Mixer[CROSSFADE]:::math
    AudioIn --> Mixer
    Knob2[KNOB 2: Mix]:::io --> Mixer

    Mixer --> AudioOut[AUDIO OUTPUT]:::io
```

---

## 14. Slew Limiter (Portamento)
**Complexity**: 5/10
**Description**: Utility to smooth out CV steps or audio transitions.
**Controls**:
- **Knob 1**: Rise Time
- **Knob 2**: Fall Time
- **Button 1**: Shape (Linear/Exp)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    AudioIn[AUDIO/CV IN]:::io --> Slew[SLEW LIMITER]:::math
    Knob1[KNOB 1: Rise]:::io --> Slew
    Knob2[KNOB 2: Fall]:::io --> Slew
    
    Slew --> AudioOut[AUDIO/CV OUT]:::io
```

---

## 15. Ring Modulator
**Complexity**: 6/10
**Description**: Sci-fi metallic sounds by multiplying input with an internal oscillator.
**Controls**:
- **Knob 1**: Carrier Frequency
- **Knob 2**: Mix
- **Button 1**: Waveform (Sine/Square)

```mermaid
graph TD
    classDef audio fill:#2980b9,stroke:#fff,stroke-width:2px;
    classDef control fill:#e67e22,stroke:#fff,stroke-width:2px;
    classDef math fill:#8e44ad,stroke:#fff,stroke-width:2px;
    classDef io fill:#27ae60,stroke:#fff,stroke-width:2px;

    Osc[OSCILLATOR]:::audio --> Ring[RING MOD]:::math
    AudioIn[AUDIO INPUT]:::io --> Ring
    
    Knob1[KNOB 1: Freq]:::io --> Osc
    Button1[BTN 1: Wave]:::io --> Osc

    Ring --> Cross[CROSSFADE]:::math
    AudioIn --> Cross
    Knob2[KNOB 2: Mix]:::io --> Cross

    Cross --> AudioOut[AUDIO OUTPUT]:::io
```
