# Pollen8 VA+FM 2.0 — Field Controls & Setup Guide

**Firmware:** Pollen8 VA+FM 2.0 | **Hardware:** Daisy Field | **Author:** Hammond Eggs Music © 2021–2025
**Full documentation:** https://hammondeggsmusic.ca/daisy/pollen8_vafm_2.html
**Noderr spec:** `DaisyExamples/noderr/specs/SYNTH_Pollen8.md`

---

## 1. Requirements

| Item | Requirement |
|------|------------|
| Browser | Chrome 61 or newer (for Web Programmer) |
| MIDI adapter | TRS **Type A** — standard 5-pin DIN to 3.5mm |
| USB cable | Micro-USB (for programming) |
| Web programmer | https://electro-smith.github.io/Programmer/ |

---

## 2. Loading Procedure

### Phase 1 — Factory Presets (do once, or when resetting to factory)

> ⚠️ **This erases patches 0–31.** Only run if you want the factory preset pack.

1. **Enter DFU mode:** Hold `BOOT` button on Daisy Field while connecting USB to PC (or hold BOOT + tap RESET while powered)
2. **Open** https://electro-smith.github.io/Programmer/ in Chrome
3. **Click Connect** → select the DFU device from the dialog
4. **Upload** `pollen8_VA_FM_field_presetLoader1.bin`
5. After flashing completes: **unplug and replug** USB (or press RESET)
6. **Hold SW1 + SW2 simultaneously** when the OLED prompts you — this loads the 32 factory presets into flash slots 0–31

### Phase 2 — Main Firmware

1. Return to DFU mode (hold BOOT, reconnect USB)
2. **Upload** `pollen8.bin` ← use this file (newest build, Feb 2025)
   - Alternative: `pollen8_VA_FM_field_v2_0.bin` (original 2021 release)
3. Press **RESET** — OLED shows current patch name; synth is ready
4. Connect MIDI keyboard via TRS-A adapter to the Field's MIDI input jack

---

## 3. MIDI Setup

- **Connector type:** 3.5mm TRS **Type A** (not Type B — these are wired differently)
- **Channel:** Set in System Menu B3, Knob 1 (OMNI or channels 1–16)
- **Velocity:** Fully supported — affects amplitude and FM operator sensitivity
- **Pitch bend:** Range configured in Menu B8
- **CC control:** Fully mappable — configure in System Menu B3, Knobs 6–8

---

## 4. Button Layout

The 16 Field keys (A1–A8 top row, B1–B8 bottom row) select menu pages. Active menu determines what K1–K8 control. Pressing a button cycles OLED through knob pairs for that menu.

### VA Mode Buttons

| Button | Menu Page | K1–K2 | K3–K4 | K5–K6 | K7–K8 |
|--------|-----------|--------|--------|--------|--------|
| **A1** | Oscillator 1 | Coarse Tune / Fine Tune | Shape / Pulse Width | LFO PWM / LFO Freq Amt | Drift / Pitch EG Amt |
| **B1** | Oscillator 2 | Coarse Tune / Fine Tune | Shape / Pulse Width | LFO PWM / LFO Freq Amt | Sync / Pitch EG Amt |
| **A2** | Mixer | OSC1 Level / OSC2 Level | Noise Level / Overdrive | — | — |
| **A3** | Filter | Cutoff / Resonance | Mode (LP→BP→HP→Notch) / — | Filter EG Amt / — | Overdrive / — |
| **A4** | Envelope 1 (Amp + Filter) | Amp A / Amp D | Amp S / Amp R | Filter A / Filter D | Filter S / Filter R |
| **B4** | Envelope 2 (Pitch) | Pitch A / Pitch D | Pitch S / Pitch R | — | — |
| **A5** | Modulation 1 (LFOs) | LFO1 Shape / LFO1 Rate | LFO1 Dest / LFO1 Depth | LFO2 Shape / LFO2 Rate | LFO2 Dest / LFO2 Depth |
| **B5** | Modulation 2 (LFO3/ModWheel) | LFO3 Shape / LFO3 Rate | LFO3 Dest / LFO3 Depth | ModWheel Dest / ModWheel Depth | — |
| **A6** | Arpeggiator | Rate / Mode (Up/Down) | Gate / Octaves | — | — |
| **A7** | Performance | Polyphony Mode / Unison Spread | Portamento Mode / Time | — | — |
| **A8** | Effects | Chorus Type / Depth | Delay Time / Feedback | Delay Mix / Reverb Mix | EQ Low / EQ High |
| **B2** | EQ + Stereo | Low Freq / Low Gain | Mid Freq / Mid Gain | High Freq / High Gain | Stereo Spread / — |
| **B3** | System Menu | MIDI Channel / VA-FM Mode | Write Mode / Factory Reset | — / — | CC Param Type / CC Select / CC Number |
| **B7** | Patch Name | Character select (scroll) | — | — | — |
| **B8** | Pitch Bend | Bend Range Up / Bend Range Down | — | — | — |

### FM Mode Buttons

| Button | Menu Page |
|--------|-----------|
| **A1** | Operator Frequency Multiplier (0.5×–32×) |
| **B1** | Velocity Sensitivity per operator |
| **A2** | Operator Fine Tune (±100 cents) |
| **A3** | Operator Amplitude |
| **A4** | Operator Envelopes (ADSR per op) |
| **B4** | Envelope Amount (modulation depth) |
| **A5** | Feedback + Algorithm (1–23) |
| **B5** | Modulation EG + LFO assignment |
| **B6** | LFO per operator |
| **A6** | Arpeggiator (same as VA) |
| **A7** | Performance (same as VA) |
| **A8** | Effects (same as VA) |
| **B2** | EQ + Stereo (same as VA) |
| **B3** | System Menu (same as VA) |
| **B7** | Patch Name |
| **B8** | Pitch Bend |

### FM Algorithm Display

The OLED shows the algorithm diagram:
- **White tab, black number** = Carrier (produces audio output)
- **Black tab, white number** = Modulator (shapes the carrier's timbre)

---

## 5. Knob Behavior

- Knobs are **always active** regardless of which menu is displayed
- Turning any knob **auto-updates the OLED** to show that parameter
- OLED shows: parameter name, value with units (Hz, ms, %, dB), and a bar graph
- **8 LED indicators above knobs** show current voice amplitude levels in real time

---

## 6. LFO Configuration (VA Mode, Menu A5/B5)

| Shape | Description |
|-------|-------------|
| Triangle | Smooth up/down |
| Sine | Smooth sinusoidal |
| Sawtooth Up | Rising ramp |
| Sawtooth Down | Falling ramp |
| Random | Sample & hold (stepped random) |

| Mode | Behavior |
|------|----------|
| **Poly Free-Run** | Each voice has its own LFO, running continuously |
| **Single Free-Run** | One shared LFO controls all voices |
| **Poly Sync** | Per-voice LFO, resets on each Note On |
| **Single Sync** | Shared LFO, resets on any Note On |

**Destinations:** Pitch, Pulse Width, Filter Cutoff

---

## 7. Performance Modes (Menu A7)

| Mode | Description |
|------|-------------|
| **Polyphonic** | 8 independent voices (default) |
| **Monophonic** | 1 voice — last note priority |
| **Unison** | All 8 voices on one note; K2 controls spread (0–1 octave) |

**Portamento:** Two modes (exponential / linear) — set time with K4 in A7 menu.

---

## 8. System Menu (Button B3)

| Knob | Function | Range |
|------|----------|-------|
| K1 | MIDI Channel | OMNI, 1–16 |
| K2 | Synthesis Mode | VA / FM |
| K3 | Write Mode | New / Overwrite / Replace |
| K4 | Factory Reset | Rotate 6 full turns to confirm |
| K6 | CC Parameter Type | VA / FM / Common |
| K7 | CC Parameter Select | Scroll through parameters |
| K8 | CC Number | 0–127, or learn from controller |
| **SW1** | Assign CC | Press to map current K8 CC to K7 parameter |
| **SW2** | Save CC config | Stores mapping |

**MIDI Learn:** Turn K8 to select CC number, or move a controller knob/pedal while in this menu to auto-detect the CC number.

---

## 9. Patch Management

| Action | Gesture |
|--------|---------|
| Previous patch | Press **SW1** (wraps 127→0) |
| Next patch | Press **SW2** (wraps 0→127) |
| **Save** | Hold **SW1** → press **SW2** (while still holding SW1) |
| Factory reset | System Menu B3, K4 — rotate 6 full turns |

> ⚠️ **No save warning:** Switching patches discards unsaved edits silently. Save before changing patches.

---

## 10. Effects Chain (Menu A8)

### Chorus / Flanger
| Type | Character |
|------|-----------|
| **Hera** | 80s-style lush chorus |
| **Flanger** | Classic metallic sweep |
| **Buckets** | 70s string machine triple-bucket-brigade chorus |

### Delay
- Up to **1.36 seconds** delay time
- Feedback and dry/wet mix

### Reverb
- **Cathedral xd** algorithmic reverb
- Dry/wet mix controlled in FX menu

### 3-Band EQ (Menu B2)
- Low shelf: frequency + gain
- Mid peak: frequency + gain
- High shelf: frequency + gain
- Stereo spread for voice widening

---

## 11. External Audio Input

The Field's audio input can be routed through the FX chain:

| Setting | Description |
|---------|-------------|
| Bypass (off) | Input ignored |
| Dry | Pass-through, no processing |
| Pre-delay | Routed to delay (bypasses chorus) |
| Full chain | Through chorus → delay → reverb → EQ |

Pollen8 can function as an **external effects processor** with this routing.

---

## 12. Known Issues & Warnings

| Issue | Notes |
|-------|-------|
| **Startup pop** | Audio codec limitation — unavoidable hardware behavior |
| **No patch save warning** | Switching patches silently discards changes — save first |
| **Preset loader erases slots 0–31** | Only run Phase 1 (preset loader) when you want factory presets |
| **v1.0 patch incompatibility** | Old Pollen8 v1.0 patches cannot be imported into v2.0 |
| **Flash nearly full** | Very limited space for future features |
| **SD card removed** | v2.0 removed SD card storage — all 128 patches are in internal flash only |
| **Closed source** | No source code available; cannot be recompiled or modified |
