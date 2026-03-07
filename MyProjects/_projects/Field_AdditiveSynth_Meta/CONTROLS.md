# Field_AdditiveSynth — Controls

## Overview

8-voice polyphonic additive synthesizer. Each voice uses `HarmonicOscillator<16>` (16 Chebyshev
partials) + `Adsr` envelope. Global: LFO (tutti vibrato/tremolo), Chorus, ReverbSc.
MIDI: TRS hardware, 8-voice last-note stealing.

SW1 = Page A (tone + ADSR + reverb)
SW2 = Page B (LFO + chorus + volume)

**Knob Pickup/Catch**: On page switch, all knobs lock. A locked knob's stored value persists
until the physical knob crosses that value (within 0.02). Locked knob LED dims to 5%.

---

## Page A (SW1, default)

| Knob | Parameter      | Range             | OLED Zoom            |
|------|----------------|-------------------|----------------------|
| K1   | Rolloff        | 0.0–2.0           | "Rolloff: x.xx"      |
| K2   | Even Harmonics | 0.0–1.0           | "Even: xx%"          |
| K3   | Odd Harmonics  | 0.0–1.0           | "Odd: xx%"           |
| K4   | Attack         | 1ms–2000ms (log)  | "Atk: xxxx ms"       |
| K5   | Decay          | 1ms–2000ms (log)  | "Dec: xxxx ms"       |
| K6   | Sustain        | 0.0–1.0           | "Sus: xx%"           |
| K7   | Release        | 1ms–4000ms (log)  | "Rel: xxxx ms"       |
| K8   | Reverb Mix     | 0.0–1.0           | "Rev: xx%"           |

---

## Page B (SW2)

| Knob | Parameter      | Range             | OLED Zoom              |
|------|----------------|-------------------|------------------------|
| K1   | LFO Rate       | 0.1–20 Hz (log)   | "LFO: x.xx Hz"         |
| K2   | LFO Depth      | 0.0–1.0           | "Dep: xx% (±x.x st)"   |
| K3   | LFO Target     | 0=pitch, 1=amp    | "Tgt: Pitch / Amp"     |
| K4   | Chorus Rate    | 0.1–5 Hz          | "Ch.Rate: x.xx Hz"     |
| K5   | Chorus Depth   | 0.0–1.0           | "Ch.Dep: xx%"          |
| K6   | Chorus Delay   | 0.0–1.0           | "Ch.Dly: xx%"          |
| K7   | Chorus Mix     | 0.0–1.0           | "Ch.Mix: xx%"          |
| K8   | Master Volume  | 0.0–1.0           | "Vol: xx%"             |

---

## Switches

| SW  | Function       | SW LED           |
|-----|----------------|------------------|
| SW1 | Select Page A  | Bright = Page A  |
| SW2 | Select Page B  | Bright = Page B  |

**Meta-mode entry**: Hold SW1+SW2 simultaneously for 200–800 ms, then release both.
Both LEDs dim to 25% while meta-mode is active.
**Meta-mode exit**: Press SW1 alone (→ Page A) or SW2 alone (→ Page B).

---

## Meta-Controller Mode

A Superknob-style macro controller (Chapter 13, Yee-King 2024): one meta-position knob
morphs **all** Page A and Page B parameters simultaneously across up to 4 stored anchor
scenes. Interpolation is linear (y = mx + b) between adjacent stored scenes.

### Activation
Hold **SW1 + SW2** together for 200–800 ms → both LEDs dim → OLED shows META header.
Press **SW1** or **SW2** alone → exit to Page A or B.

### Controls in Meta-Mode

| Control | Function                                                         |
|---------|------------------------------------------------------------------|
| **K1**  | **Morph Position** — 0.0 = Scene 1, 1.0 = last stored scene     |
| K2      | Edit active scene: Rolloff (0–2)                                 |
| K3      | Edit active scene: Even harmonics (0–1)                          |
| K4      | Edit active scene: Odd harmonics (0–1)                           |
| K5      | Edit active scene: Attack (1ms–2s log)                           |
| K6      | Edit active scene: Decay (1ms–2s log)                            |
| K7      | Edit active scene: Sustain (0–1)                                 |
| K8      | Edit active scene: Release (1ms–4s log)                          |
| **B1**  | Short press = select Scene 1 for editing                         |
| **B2**  | Short press = select Scene 2 for editing                         |
| **B3**  | Short press = select Scene 3 for editing                         |
| **B4**  | Short press = select Scene 4 for editing                         |
| B1–B4   | **Long press (≥500 ms)** = store ALL current params as that scene|
| A1–A8   | Load spectral preset tonal params into current scene             |
| B5      | LFO Sync toggle (unchanged)                                      |
| B6      | Reverb bypass toggle (unchanged)                                 |
| B7      | Chorus bypass toggle (unchanged)                                 |
| B8      | Sustain pedal toggle (unchanged)                                 |

### Scene LEDs (B1-B4 in meta-mode)
- **Full bright**: currently selected for editing
- **Dim (35%)**: stored but not selected
- **Off**: empty (not yet stored)

### Default Scenes (pre-loaded at boot)
| Scene | Character              | rolloff | even | odd  |
|-------|------------------------|---------|------|------|
| 1     | Sine-like (pure pad)   | 2.0     | 0.0  | 0.0  |
| 2     | Sawtooth (bright lead) | 1.0     | 0.8  | 0.8  |
| 3     | Empty — store via B3   | —       | —    | —    |
| 4     | Empty — store via B4   | —       | —    | —    |

### OLED in Meta-Mode
```
META [1](2). .            ← [N]=selected, (N)=stored, .=empty
[════════|═══════════════] ← morph position bar
S1->S2 t=0.45             ← segment being interpolated
R:1.4 E:40% O:40%         ← live tonal param preview
S1 K2-8=edit LP=store     ← editing guide
```

---

## Keyboard A Row — Spectral Presets

Selecting a preset loads rolloff/even/odd AND locks K1-K3 on Page A.
Locked K1-K3 LEDs dim. Moving any locked knob releases it (custom mode, preset LED off).

| Key | Preset    | rolloff | even | odd  | Character               |
|-----|-----------|---------|------|------|-------------------------|
| A1  | Sine      | 5.0     | 0.0  | 0.0  | Pure fundamental        |
| A2  | Triangle  | 2.0     | 0.0  | 0.7  | Soft, odd harmonics     |
| A3  | Sawtooth  | 1.0     | 0.8  | 0.8  | Full spectrum 1/n       |
| A4  | Square    | 1.0     | 0.0  | 0.8  | Hollow, odd-only        |
| A5  | Hollow    | 1.5     | 0.9  | 0.1  | Even-dominant, airy     |
| A6  | Bell      | 0.5     | 0.2  | 0.9  | Bright, metallic        |
| A7  | Organ     | 0.0     | 0.8  | 0.6  | Flat drawbar style      |
| A8  | Buzz      | 0.1     | 0.8  | 0.8  | Rich, bright buzz       |

---

## Keyboard B Row — LFO + Performance

| Key | Function              | Mode   | LED              |
|-----|-----------------------|--------|------------------|
| B1  | LFO Waveform: Sine   | radio  | Lit = active     |
| B2  | LFO Waveform: Tri    | radio  | Lit = active     |
| B3  | LFO Waveform: Saw    | radio  | Lit = active     |
| B4  | LFO Waveform: S&H    | radio  | Lit = active     |
| B5  | LFO Sync on NoteOn   | toggle | Lit = on         |
| B6  | Reverb bypass        | toggle | Lit = bypassed   |
| B7  | Chorus bypass        | toggle | Lit = bypassed   |
| B8  | Sustain pedal        | toggle | Lit = held       |

---

## MIDI (TRS hardware)

8-voice polyphonic, last-note stealing.
Velocity per voice (scales output amplitude).
Octave shifting handled on MIDI keyboard hardware.

---

## OLED Display

**Normal view** (128×64, Font_7x10):
- Row 0 (y=0):  Active notes + page indicator — e.g., "C4 G4 [A]"
- Row 1 (y=16): 16 partial spectrum bars (8 px × 16 = 128 px), relative to max
- Row 2 (y=54): Preset name (if active) + bypass flags — e.g., "[Saw] [R] [C]"

**Zoom view** (1.5s after any knob change):
- Row 0 (y=0):  Parameter name (locked knobs prefixed "(L) ")
- Row 1 (y=16): Value + unit in Font_11x18
- Row 2 (y=52): Progress bar (full width = 100%)
