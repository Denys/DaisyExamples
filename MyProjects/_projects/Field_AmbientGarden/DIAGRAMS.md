# Field_AmbientGarden — Diagrams & Patch Examples

## 1. Block Diagram (Hardware & Architecture)

```mermaid
block-beta
  columns 3
  CLOCK["RandomClock\nTuringMachine\nScaleQuantizer"]:1 CPU["Daisy Field\nCortex-M7"]:1 AUDIO["Audio Out\nL+R"]:1
  KNOBS["K1: Density\nK2: Root\nK3: Reverb\nK4: Spread\nK5: Bright\nK6: Damp\nK7: Struct\nK8: Wet/Dry"]:1 DSP["DSP Chain\nModalVoice ×4\nMixer + SVF LP\nReverbSc (stereo)\nSoft Clip"]:1 OLED["128×64 OLED\nVoice Activity\nParam values"]:1
  KEYS["A1-A8: Scales\nB1-B8: Voice Presets\nSW1: Freeze\nSW2: Stereo Wide"]:1 space:1 space:1
```

---

## 2. Signal Flow (Audio Path)

```mermaid
flowchart LR
    RCLK[RandomClock] -->|trig| TM[TuringMachine]
    TM -->|cv| QUANT[ScaleQuantizer]
    
    QUANT -->|freq| V1[ModalVoice 1]
    QUANT -->|freq| V2[ModalVoice 2]
    QUANT -->|freq| V3[ModalVoice 3]
    QUANT -->|freq| V4[ModalVoice 4]
    
    V1 -->|audio| MIXL[Mixer L]
    V1 -->|audio| MIXR[Mixer R]
    V2 -->|audio| MIXL
    V2 -->|audio| MIXR
    V3 -->|audio| MIXL
    V3 -->|audio| MIXR
    V4 -->|audio| MIXL
    V4 -->|audio| MIXR
    
    MIXL -->|audio| SVFL[SVF LP L]
    MIXR -->|audio| SVFR[SVF LP R]
    
    SVFL -->|audio| REV[ReverbSc]
    SVFR -->|audio| REV
    
    REV -->|out_l| CLIP[Soft Clip]
    REV -->|out_r| CLIP
    
    CLIP -->|left| OUT[Audio Output]
    CLIP -->|right| OUT
```

---

## 3. Interaction Flow (Control Visualization)

```mermaid
flowchart TD
    LOOP[Main Loop] --> PROC[ProcessAllControls]
    PROC --> KNOBS[ProcessKnobs]
    
    KNOBS --> K1[K1: Density → RandomClock]
    KNOBS --> K2[K2: Root → ScaleQuantizer]
    KNOBS --> K4[K4: Spread → Voice Intervals + TM Probability]
    KNOBS --> TONE[K5/K6/K7: Bright/Damp/Struct → ModalVoices]
    KNOBS --> REV[K3/K8: Reverb Size/Mix → ReverbSc]
    
    PROC --> KEYS[ProcessKeys]
    KEYS --> A_ROW[A1-A8: Set Scale]
    KEYS --> B_ROW[B1-B8: Set Voice Preset]
    KEYS --> SW1[SW1: Freeze → RandomClock]
    KEYS --> SW2[SW2: Stereo Width → Mixer]
    
    LOOP --> AUDIO[AudioCallback]
    AUDIO --> TRIG{Clock Trigger?}
    TRIG -->|Yes| DIST[Voice Distribution]
    DIST --> V1[V1: Melody 100%]
    DIST --> V2[V2: Low Harmony 60%]
    DIST --> V3[V3: High Harmony 40%]
    DIST --> V4[V4: Accent Chime 20%]
```

---

## 4. Patch Examples

### Patch 1: "Zen Garden"
*A sparse, meditative, and consonant soundscape.*

| Control | Parameter | Value | Comments |
|---------|-----------|-------|----------|
| **Key A1** | Scale | Pentatonic Maj | Safe, consonant, world music feel |
| **Key B1** | Voice Preset | Glass Bells | Crystalline, bright tone |
| **K1** | Density | 0.25 | Sparse, slow generative rhythm |
| **K2** | Root | C (0) | Root note C |
| **K3** | Reverb Size | 0.8 | Large, expansive space |
| **K4** | Harmony Spread | 0.2 | Close harmony, tight intervals |
| **K5** | Brightness | 0.6 | Moderately bright |
| **K6** | Damping | 0.3 | Long decay, notes ring out |
| **K7** | Structure | 0.4 | Slightly metallic timbre |
| **K8** | Wet/Dry | 0.7 | Mostly wet, atmospheric |
| **SW2** | Stereo Width| ON (Wide) | Immersive stereo field |

### Patch 2: "Dark Forest"
*A dark, huge, and slightly melancholic atmosphere.*

| Control | Parameter | Value | Comments |
|---------|-----------|-------|----------|
| **Key A3** | Scale | Minor (Aeolian) | Melancholy, dark feel |
| **Key B8** | Voice Preset | Temple Bowl | Deep, sustained ring |
| **K1** | Density | 0.35 | Moderate activity |
| **K2** | Root | A (9) | Root note A (A minor) |
| **K3** | Reverb Size | 0.9 | Huge space, cathedral-like |
| **K4** | Harmony Spread | 0.6 | Wide intervals, more unpredictable |
| **K5** | Brightness | 0.2 | Dark, muted highs |
| **K6** | Damping | 0.5 | Medium decay |
| **K7** | Structure | 0.7 | Woody, hollow body resonance |
| **K8** | Wet/Dry | 0.8 | Very wet, washed out |
| **SW2** | Stereo Width| ON (Wide) | Immersive stereo field |

### Patch 3: "Gamelan Rain"
*A fast, plucky, and metallic polyrhythmic texture.*

| Control | Parameter | Value | Comments |
|---------|-----------|-------|----------|
| **Key A7** | Scale | Whole Tone | Dreamy, floating |
| **Key B5** | Voice Preset | Gamelan | Metallic, detuned |
| **K1** | Density | 0.7 | Dense, rapid triggers (like rain) |
| **K2** | Root | D (2) | Root note D |
| **K3** | Reverb Size | 0.5 | Medium room, keeps attacks clear |
| **K4** | Harmony Spread | 0.45 | Medium spread |
| **K5** | Brightness | 0.5 | Balanced tone |
| **K6** | Damping | 0.4 | Balanced decay |
| **K7** | Structure | 0.5 | Balanced |
| **K8** | Wet/Dry | 0.5 | 50/50 mix |
| **SW2** | Stereo Width| OFF (Mono) | Focused center image |

### Patch 4: "Frozen Cathedral"
*An almost static, extremely ambient pad texture.*

| Control | Parameter | Value | Comments |
|---------|-----------|-------|----------|
| **Key A4** | Scale | Dorian | Jazz/ambient color |
| **Key B6** | Voice Preset | Wind Chimes | Airy, long tails |
| **K1** | Density | 0.15 | Very sparse, notes play rarely |
| **K2** | Root | E (4) | Root note E |
| **K3** | Reverb Size | 1.0 | Maximum reverb, infinite tail |
| **K4** | Harmony Spread | 0.8 | Very wide harmony, high chaos |
| **K5** | Brightness | 0.7 | Bright, emphasizes upper harmonics |
| **K6** | Damping | 0.15 | Very long decay on the physical model |
| **K7** | Structure | 0.3 | highly metallic |
| **K8** | Wet/Dry | 0.9 | Almost fully wet |
| **SW1** | Freeze | ON | Pauses new notes, lets current ones ring forever |
