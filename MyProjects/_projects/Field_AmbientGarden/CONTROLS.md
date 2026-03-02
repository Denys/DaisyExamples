# Generative Ambient Garden — Controls Documentation

## 1. Control Mapping

### Hardware Overview

| Control Type | Quantity | Purpose |
|--------------|----------|---------|
| **Knobs** | 8 | Continuous parameter control |
| **Keys (A-row)** | 8 | Scale selection |
| **Keys (B-row)** | 8 | Voice character preset selection |
| **Switches** | 2 | Freeze / Stereo mode |

---

### Knob Assignments

| Knob | Parameter | Range | Default | Unit | Mapping |
|------|-----------|-------|---------|------|---------|
| **K1** | Density / Activity | 0.0 – 1.0 | 0.4 | — | Clock interval: `50 + (1-k)^3 * 1950` ms (exponential) |
| **K2** | Scale Root | C – B (12 steps) | 0 (C) | semitone | Quantized to 12 steps, display shows note name |
| **K3** | Reverb Size | 0.0 – 1.0 | 0.6 | — | Decay 0.90–0.998, damping 500–18000 Hz |
| **K4** | Harmony Spread | 0.0 – 1.0 | 0.3 | octaves | Voice interval spread (0=unison, 1=3 oct) + TM probability |
| **K5** | Brightness | 0.0 – 1.0 | 0.5 | — | ModalVoice brightness + pre-reverb LPF cutoff 800–12000 Hz |
| **K6** | Damping | 0.0 – 1.0 | 0.4 | — | ModalVoice damping (low=long ring, high=short pluck) |
| **K7** | Structure | 0.0 – 1.0 | 0.5 | — | ModalVoice structure (metallic to woody) |
| **K8** | Wet/Dry Mix | 0.0 – 1.0 | 0.6 | — | Reverb mix (0=dry, 1=fully wet) |

---

### Key Assignments

#### A-Row: Scale Select (A1–A8)

| Key | Scale | Intervals (semitones) | Character |
|-----|-------|-----------------------|-----------|
| A1 | Pentatonic Major | 0, 2, 4, 7, 9 | Safe, warm, world music |
| A2 | Major (Ionian) | 0, 2, 4, 5, 7, 9, 11 | Bright, happy |
| A3 | Minor (Aeolian) | 0, 2, 3, 5, 7, 8, 10 | Melancholy |
| A4 | Dorian | 0, 2, 3, 5, 7, 9, 10 | Jazz, ambient |
| A5 | Mixolydian | 0, 2, 4, 5, 7, 9, 10 | Bluesy, modal |
| A6 | Pentatonic Minor | 0, 3, 5, 7, 10 | Dark, bluesy |
| A7 | Whole Tone | 0, 2, 4, 6, 8, 10 | Dreamy, Debussy |
| A8 | Chromatic | 0–11 (all) | Atonal, experimental |

#### B-Row: Voice Character Preset (B1–B8)

| Key | Name | Brightness | Structure | Damping | Accent | Character |
|-----|------|------------|-----------|---------|--------|-----------|
| B1 | Glass Bells | 0.9 | 0.3 | 0.3 | 0.8 | Crystalline, bright |
| B2 | Wooden Marimba | 0.4 | 0.7 | 0.5 | 0.6 | Warm, woody |
| B3 | Metal Chime | 0.7 | 0.2 | 0.2 | 0.9 | Metallic, long decay |
| B4 | Soft Pads | 0.3 | 0.5 | 0.7 | 0.3 | Muted, padlike |
| B5 | Gamelan | 0.6 | 0.4 | 0.4 | 0.7 | Metallic, detuned |
| B6 | Wind Chimes | 0.8 | 0.1 | 0.15 | 0.5 | Airy, long tails |
| B7 | Steel Drum | 0.5 | 0.6 | 0.6 | 0.85 | Caribbean, pitched |
| B8 | Temple Bowl | 0.2 | 0.8 | 0.1 | 0.4 | Deep, sustained ring |

---

### Switch Assignments

| Switch | Function | States | Behavior |
|--------|----------|--------|----------|
| **SW1** | Freeze / Run | OFF = Running, ON = Frozen | Pauses clock; voices ring out naturally |
| **SW2** | Stereo Width | OFF = Mono center, ON = Wide | Wide: V1,V3 left / V2,V4 right |

---

## 2. LED Behavior

| LED Group | Behavior |
|-----------|----------|
| Knob LEDs (8) | Brightness proportional to current knob value |
| Key A LEDs (8) | One LED lit solid = current scale (exclusive select) |
| Key B LEDs (8) | One LED lit solid = current preset; brief pulse on voice trigger |
| SW1 LED | ON when frozen |
| SW2 LED | ON when stereo wide |

---

## 3. OLED Display States

### Normal Running
```
+------------------------------+
| Garden: Pentatonic C         |
| V1:C4  V2:G3  V3:E5  V4:--  |
|                              |
| Density: 40%   Spread: 30%  |
| RUN  Wide  Glass Bells       |
+------------------------------+
```

### Parameter Adjustment (e.g., K3)
```
+------------------------------+
| Garden: Dorian D             |
| > Reverb Size                |
|                              |
|  0.72                        |
|                              |
| RUN  Mono  Gamelan           |
+------------------------------+
```

### Frozen
```
+------------------------------+
| Garden: Minor A              |
| *** FROZEN ***               |
| V1:A4  V2:E3  V3:--  V4:--  |
|                              |
| Voices ringing...            |
| FRZ  Wide  Temple Bowl       |
+------------------------------+
```

---

## 4. Presets

### Preset 1: "Zen Garden"

| Control | Setting | Notes |
|---------|---------|-------|
| Scale | A1 (Pentatonic Major) | Safe, consonant |
| Root | K2 = C (0) | |
| Voice | B1 (Glass Bells) | Crystalline |
| K1 Density | 0.25 | Sparse, meditative |
| K4 Spread | 0.2 | Close harmony |
| K3 Reverb | 0.8 | Large space |
| K5 Brightness | 0.6 | Bright |
| K6 Damping | 0.3 | Long decay |
| K7 Structure | 0.4 | Slightly metallic |
| K8 Wet/Dry | 0.7 | Mostly wet |

### Preset 2: "Dark Forest"

| Control | Setting | Notes |
|---------|---------|-------|
| Scale | A3 (Minor) | Melancholy |
| Root | K2 = A (9) | A minor |
| Voice | B8 (Temple Bowl) | Deep, sustained |
| K1 Density | 0.35 | Moderate |
| K4 Spread | 0.6 | Wide intervals |
| K3 Reverb | 0.9 | Huge space |
| K5 Brightness | 0.2 | Dark |
| K6 Damping | 0.5 | Medium decay |
| K7 Structure | 0.7 | Woody |
| K8 Wet/Dry | 0.8 | Very wet |

### Preset 3: "Gamelan Rain"

| Control | Setting | Notes |
|---------|---------|-------|
| Scale | A7 (Whole Tone) | Dreamy |
| Root | K2 = D (2) | |
| Voice | B5 (Gamelan) | Metallic, detuned |
| K1 Density | 0.7 | Dense, rapid |
| K4 Spread | 0.45 | Medium spread |
| K3 Reverb | 0.5 | Medium room |
| K5 Brightness | 0.5 | Balanced |
| K6 Damping | 0.4 | Balanced |
| K7 Structure | 0.5 | Balanced |
| K8 Wet/Dry | 0.5 | Balanced |

### Preset 4: "Frozen Cathedral"

| Control | Setting | Notes |
|---------|---------|-------|
| Scale | A4 (Dorian) | Jazz/ambient |
| Root | K2 = E (4) | |
| Voice | B6 (Wind Chimes) | Airy, long tails |
| K1 Density | 0.15 | Very sparse |
| K4 Spread | 0.8 | Very wide harmony |
| K3 Reverb | 1.0 | Maximum reverb |
| K5 Brightness | 0.7 | Bright |
| K6 Damping | 0.15 | Very long decay |
| K7 Structure | 0.3 | Metallic |
| K8 Wet/Dry | 0.9 | Almost fully wet |

---

## Quick Reference Card

```
+-----------------------------------------------------------+
|    GENERATIVE AMBIENT GARDEN - CONTROL REFERENCE          |
+-----------------------------------------------------------+
|  KNOBS                                                    |
|  K1: Density  K2: Root     K3: Reverb   K4: Spread       |
|  K5: Bright   K6: Damp     K7: Struct   K8: Wet/Dry      |
+-----------------------------------------------------------+
|  KEYS (A-ROW): Scale Select                               |
|  A1=PentMaj A2=Major A3=Minor A4=Dorian                   |
|  A5=Mixo A6=PentMin A7=WholeTone A8=Chromatic             |
+-----------------------------------------------------------+
|  KEYS (B-ROW): Voice Preset                               |
|  B1=Glass B2=Marimba B3=Metal B4=Pads                     |
|  B5=Gamelan B6=WindChime B7=SteelDrum B8=Temple            |
+-----------------------------------------------------------+
|  SWITCHES                                                 |
|  SW1: Freeze On/Off    SW2: Stereo Width (Mono/Wide)      |
+-----------------------------------------------------------+
```
