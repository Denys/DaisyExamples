# Drum Machine Context Reference
> Comprehensive session summary for cross-chat context continuity.
> Covers: Daisy Field hardware projects + UX/UI design principles.

---

## 1. HARDWARE PROJECTS — Daisy Field Drum Machines

### Platform
- **Hardware**: Electro-Smith Daisy Field
- **Audio**: Non-interleaved, `out[0][i]` / `out[1][i]`
- **HAL**: `daisy_field.h`, `DaisyField hw`
- **MIDI**: Hardware TRS via `hw.midi`, not USB MIDI
- **Display**: 128×32 OLED via `hw.display`
- **Controls**: 8 knobs (`hw.knob[0..7]`), 16 keyboard keys (A-row + B-row), SW1/SW2 buttons

### Keyboard Index Mapping (Critical — HAL reverses A-row)
```
KeyboardRisingEdge(0)  = A8 (rightmost physical key)
KeyboardRisingEdge(7)  = A1 (leftmost physical key)
KeyboardRisingEdge(8)  = B1 (leftmost B-row key)
KeyboardRisingEdge(15) = B8 (rightmost B-row key)
```

### LED Index Constants (daisy_field.h enum)
```
LED_KEY_B1=0  .. LED_KEY_B8=7    (B-row, sequential)
LED_KEY_A8=8  .. LED_KEY_A1=15   (A-row, REVERSED — A1 is highest index)
LED_KNOB_1=16 .. LED_KNOB_8=23   (Knob ring LEDs)
```
**Critical bug pattern**: `LED_KEY_A1 + i` walks INTO knob LED range. Must use `LED_KEY_A1 - i`.

---

### Projects Worked On

#### Field_DrumLab
- **Status**: Fixed + rebuilt
- **Root cause of no-sound**: `hw.seed.StartLog(true)` blocks boot waiting for USB serial terminal
- **Fix**: `hw.seed.StartLog(false)` — allows standalone boot
- **OLED**: Reference implementation — used as gold standard for zoom pattern
- **Features**: Multi-voice drum machine, 16-step sequencer, ADSR per voice

#### Field_EuclideanRhythmist
- **Status**: Fixed + rebuilt
- **Root cause of no-sound**: `bool seq_playing = false` — sequencer started paused
- **Fix 1**: `bool seq_playing = true` — autostart
- **Root cause of wrong keys**: `kKeyAIndices` had B-row indices (8-15), `kKeyBIndices` had A-row (0-7) — completely swapped
- **Fix 2**:
  ```cpp
  const int kKeyAIndices[8] = {0, 1, 2, 3, 4, 5, 6, 7};      // A-row = indices 0-7
  const int kKeyBIndices[8] = {8, 9, 10, 11, 12, 13, 14, 15}; // B-row = indices 8-15
  ```
- **Fix 3**: Added OLED zoom pattern
- **Features**: Euclidean rhythm generator, 8 voices, adjustable pulses/length per voice

#### Field_AnalogDrumCore
- **Status**: Fixed + rebuilt
- **LED bug**: A-row LEDs iterating wrong direction
- **Fix**: `hw.led_driver.SetLed(DaisyField::LED_KEY_A1 - i, aBrightness)`
- **OLED improvement**: Added missing params (KickTone, AttackFM, SnareTone, Snappy) to overview
- **Overview grid**: 16 steps × 8px each = 128px wide
- **Features**: Analog-modeled kick + snare drum voices with CV-style controls

#### DrumMachine
- **Status**: Fixed + rebuilt
- **LED bug**: `keyboard_leds[]` array mapped A1 first but `KeyboardRisingEdge(0)` = A8
- **Fix**: Reordered to `{LED_KEY_A8, LED_KEY_A7, ..., LED_KEY_A1, LED_KEY_B1, ..., LED_KEY_B8}`
- **OLED improvement**: Bottom line shows sound params: `"K:%d S:%d H:%d Sw:%d%%"`
- **Features**: Classic TR-style kick/snare/hihat drum machine

#### FieldOpus_DrumMachinePro
- **Status**: No bugs, OLED zoom added
- **OLED knob names**: Kick Decay (ms), Snare Decay (ms), HiHat Decay (ms), Tom Tune (Hz), Clap Decay (ms), Master Vol (%), Tempo (BPM), Swing (%)
- **Features**: 5-voice drum machine (kick, snare, hihat, tom, clap) with master volume and swing

#### Field_RetroStepSequencer
- **Status**: No bugs, OLED zoom added
- **Unusual pattern**: Controls processed INSIDE AudioCallback (knobVals[] global populated there)
- **OLED improvement**: Added 3rd info line `"LFO:%.1fHz Atk:%dms"` (previously hidden values)
- **Features**: Step sequencer with oscillator, SVF filter, LFO, ADSR envelope

#### Field_GlitchDrumPerformer
- **Status**: Needs full rewrite — Pod → Field port
- **Issue**: Uses `daisy_pod.h` / `DaisyPod hw` (wrong platform), produces partial audio on Field but controls don't work

---

### Universal OLED Zoom Pattern (Gold Standard)

All projects now implement this consistent pattern (reference: `Field_DrumLab.cpp`):

```cpp
// === State globals ===
float    kz_prev[8]  = {};
int      kz_idx      = -1;
uint32_t kz_time     = 0;
constexpr uint32_t kZoomMs    = 1400;   // zoom display duration ms
constexpr float    kZoomDelta = 0.015f; // minimum change to trigger zoom

// === Knob names array (project-specific) ===
const char* kKnobNames[8] = {
    "KickTone", "Kick Decay", "Attack FM", "Snare Tone",
    "Snr Decay", "Snappy", "Tempo", "Drive"
};

// === CheckKnobs() — detects parameter movement ===
void CheckKnobs() {
    for(int i = 0; i < 8; i++) {
        float v = hw.knob[i].Process();
        kvals[i] = v;
        if(fabsf(v - kz_prev[i]) > kZoomDelta) {
            kz_idx = i;
            kz_time = System::GetNow();
            kz_prev[i] = v;
        }
    }
}

// === DrawZoom() — zoomed parameter display ===
void DrawZoom() {
    hw.display.Fill(false);
    char buf[32];
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(kKnobNames[kz_idx], Font_7x10, true);  // name row
    float v = kz_prev[kz_idx];
    snprintf(buf, sizeof(buf), "%.0f Hz", 20.f + v * 2000.f);     // value + units
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(buf, Font_11x18, true);                  // large font
    int barW = (int)(v * 126.f);
    hw.display.DrawRect(0, 28, barW, 31, true, true);               // progress bar
    hw.display.Update();
}

// === UpdateDisplay() — dispatcher ===
void UpdateDisplay() {
    CheckKnobs();
    bool zoomed = (kz_idx >= 0) && (System::GetNow() - kz_time < kZoomMs);
    if(zoomed) {
        DrawZoom();
    } else {
        kz_idx = -1;
        DrawOverview();  // project-specific
    }
}
```

**Zoom display layout (128×32 OLED)**:
- Row y=0: Param name in `Font_7x10`
- Row y=12: Value + units in `Font_11x18` (large)
- Row y=28–31: Full-width progress bar

---

### Build Results (All Successful)
```
Field_DrumLab:            FLASH 99.73% ✅  (StartLog fix)
Field_EuclideanRhythmist: FLASH 91.91% ✅  (seq_playing + key indices + zoom)
Field_AnalogDrumCore:     FLASH 89.30% ✅  (LED fix + OLED improvement)
DrumMachine:              FLASH 92.50% ✅  (LED fix + OLED improvement)
FieldOpus_DrumMachinePro: FLASH 91.96% ✅  (OLED zoom added)
Field_RetroStepSequencer: FLASH 92.41% ✅  (OLED zoom + extra info line)
```

**Pending**: Flash all 6 via ST-Link (`make program` in each project dir). Field_GlitchDrumPerformer needs Pod→Field port.

---

## 2. UX/UI PRESENTATION — Optimal Drum Machine Design

### Format
- **Tool**: Pencil (.pen file), 10 slides at 1920×1080 (16:9)
- **Style**: Dark Bold / Neon Lime — void black `#0A0A0A` + electric lime `#C4F82A`
- **Fonts**: Space Grotesk (headlines), Manrope (body), Space Mono (metadata/labels)

### The 10 Slides

| # | Title | Core Message | Layout |
|---|-------|-------------|--------|
| 01 | DRUM MACHINE UX/UI | "Designing for Flow, Feel & Performance" | Cover + AI bg |
| 02 | The Problem | Engineers design for engineers, not musicians | Split text/image |
| 03 | Tactile Immediacy | `< 20ms` response — below perception threshold | Image + stat |
| 04 | The Grid as Language | 16 steps is the universal rhythm grammar | Center + live grid |
| 05 | Group by Function | RHYTHM / PITCH / DYNAMICS / EFFECTS zones | 2×2 matrix |
| 06 | Mode Clarity | Program Mode vs Performance Mode | Side-by-side compare |
| 07 | Close the Feedback Loop | Visual → Audio → Tactile → Cognitive | 4-step process |
| 08 | The Numbers That Matter | `< 20ms` / `16` steps / `7±2` Miller's Law | 3 KPIs |
| 09 | The Golden Rules | 5 design commandments | Numbered list |
| 10 | Make It Disappear | "The best UI is the one you forget is there" | Closing + AI bg |

### The 5 Golden Rules
1. **Design for the Hands** — If it requires looking, it's not fast enough
2. **Never Interrupt the Beat** — Mode changes must be seamless and musical
3. **Show State, Not Settings** — Display shows what IS happening, not what COULD
4. **One Action = One Result** — No button should ever do two different things
5. **Make Mistakes Recoverable** — Undo/redo history; fear kills creativity

---

## 3. Core Design Philosophy (Synthesized)

### The Fundamental Tension
Hardware drum machine design sits at the intersection of:
- **Engineering constraints** (latency, memory, CPU budget)
- **Musician UX** (flow state, muscle memory, immediacy)
- **Physical ergonomics** (knob reach, LED visibility on stage)

### Design Principle Map

| Principle | Hardware Reality | UX Goal |
|-----------|-----------------|---------|
| Latency | ARM Cortex-M7, DMA audio, block size 4–48 | `< 20ms` end-to-end |
| Parameter display | 128×32 OLED, 4 font sizes | Zoom on touch, overview at rest |
| Control mapping | 8 knobs × 16 keys = 128 inputs | Group by function zone, not by voice |
| Mode switching | Single button, no menus | Program ↔ Performance = 1 tap |
| Feedback | LEDs (per step) + OLED (param) + audio (sound) | All three < 20ms |
| Cognitive load | 7±2 items in working memory (Miller's Law) | Max 8 visible parameters at once |

### The OLED as UX Instrument
The 128×32 OLED is not a debug screen — it is the primary parameter interface.

**The zoom pattern solves the core tension**:
- **Overview mode**: Peripheral awareness — "What is the patch doing right now?"
- **Zoom mode**: Focused attention — "What am I editing right now?"

Trigger: knob movement > 0.015 threshold. Duration: 1400ms. Auto-revert to overview.

This mirrors how musicians actually think: broad peripheral awareness that snaps to sharp
focus on demand, then releases back — without interrupting the musical performance.

### Performance vs Program Mode Duality
All mature drum machines solve this identically:
- **Elektron**: Pattern mode vs Sound mode
- **Roland TR-8S**: Pattern vs Sound layer
- **Daisy Field projects**: SW1 toggles `seq_playing`, keys switch step-edit vs live mute

**UX rule**: Mode changes must be a single, unambiguous gesture with immediate visual confirmation. The transition must never disrupt audio playback.

---

## 4. DaisySP Drum Module Reference

| Module | Init | Trigger | Process | LGPL |
|--------|------|---------|---------|------|
| `AnalogBassDrum` | `.Init(sr)` | `.Trig()` | `.Process(false)` | No |
| `SyntheticSnareDrum` | `.Init(sr)` | `.Trig()` | `.Process(false)` | No |
| `HiHat` | `.Init(sr)` | `.Trig()` | `.Process(false)` | No |
| `AnalogSnareDrum` | `.Init(sr)` | `.Trig()` | `.Process(false)` | No |
| `ReverbSc` | `.Init(sr)` | — | `.Process(inL, inR, &outL, &outR)` | **Yes** |
| `MoogLadder` | `.Init(sr)` | — | `.Process(in)` | **Yes** |

**Key pattern**: Trigger from main loop / MIDI handler → process from audio callback.

**LGPL modules** require `USE_DAISYSP_LGPL = 1` in Makefile.

### Parameter Smoothing (Anti-Zipper Noise)
```cpp
// Per-sample smoothing in AudioCallback — prevents clicks when knob changes
float current_freq = 440.f, target_freq = 440.f;
fonepole(current_freq, target_freq, 0.001f);
osc.SetFreq(current_freq);
```

### Makefile Path Reference
```makefile
# For projects at: DaisyExamples/MyProjects/_projects/ProjectName/
LIBDAISY_DIR = ../../../../libDaisy
DAISYSP_DIR  = ../../../../DaisySP
```

---

## 5. Key Bugs Catalog

| Project | Bug | Symptom | Fix |
|---------|-----|---------|-----|
| Field_DrumLab | `StartLog(true)` | No sound, waits for USB serial | `StartLog(false)` |
| Field_EuclideanRhythmist | `seq_playing = false` | No sound, sequencer paused | `seq_playing = true` |
| Field_EuclideanRhythmist | Key arrays swapped | Wrong keys trigger wrong voices | Swap `kKeyAIndices` ↔ `kKeyBIndices` |
| Field_AnalogDrumCore | `LED_KEY_A1 + i` | LEDs light under wrong keys, knob LEDs corrupted | `LED_KEY_A1 - i` |
| DrumMachine | LED array direction | Step LEDs light wrong physical key | Reorder array: A8 first |
| All Field projects | No OLED zoom | Only raw values shown, no zoom on knob touch | Add `CheckKnobs()` + `DrawZoom()` |

---

*Session date: March 2026*
*Projects: `DaisyExamples/MyProjects/_projects/`*
*OLED zoom reference: `Field_DrumLab/Field_DrumLab.cpp`*
*UX/UI slides: Pencil .pen file (Dark Bold / Neon Lime style)*
