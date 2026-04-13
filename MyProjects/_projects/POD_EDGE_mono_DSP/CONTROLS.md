# POD_EDGE_mono_DSP — Controls

## Overview

Mono DSP effects instrument on Daisy Pod + external I2C board (OLED, encoder, BAK, CON).

Signal chain: Input Trim → DC Block → Input HP → Soft Drive → Tempo-Synced Delay
(with filtered feedback + wow/flutter) → Dark Diffusion → Wet/Dry → Output Tilt → Limiter → Dual-Mono Out.

### Hardware Summary

| Control | Type | Location | libDaisy API | Role |
|---|---|---|---|---|
| Knob 1 | Pot | Pod native | `hw.knob1.Process()` → 0–1 | Delay Time |
| Knob 2 | Pot | Pod native | `hw.knob2.Process()` → 0–1 | Feedback |
| SW1 | Pushbutton | Pod native | `hw.button1.RisingEdge()` | **Shift** (hold) / Freeze (short) |
| SW2 | Pushbutton | Pod native | `hw.button2.RisingEdge()` | Tap Tempo |
| Pod Encoder | EC11 | Pod native | `hw.encoder.Increment()` / `hw.encoder.RisingEdge()` | Wet/Dry · Freeze Latch |
| Ext Encoder | EC11 | I2C board D7/D8/D9 | `ext_enc.Increment()` | Menu Navigate |
| Ext PSH | Pushbutton | I2C board D9 | `ext_enc.EncoderPressed()` / `ext_enc.EncoderHeld()` | Edit / Commit / Quick Save |
| BAK | Pushbutton | I2C board D22 | `ext_enc.BackPressed()` — direct GPIO | Back / Cancel |
| CON | Pushbutton | I2C board D10 | `ext_enc.ConfirmPressed()` | Confirm / Apply |
| OLED | SSD1306 128×64 | I2C board | SCL=D11 SDA=D12, ≤30 fps | Display |

> **ExtEncoder.h**: copy from `Pod_OLED_EuclideanDrums/ExtEncoder.h`. Handles BAK pin conflict with `pod.Init()` / D17 LED driver.

---

## Shift Button

**SW1** acts as **Shift**.

- **Short press alone** → Freeze momentary (freeze held for duration of press)
- **Hold + twist/press another control** → activates secondary (Shifted) parameter for that control
- Shift state is always visible on OLED (top-right corner indicator `[SHF]`)
- Shift must be deterministic — no ambiguous timing windows

---

## Unshifted Control Map

| Control | Function | Range / Notes |
|---|---|---|
| Knob 1 | Delay Time | Beat divisions (sync mode) or 10–2000 ms (free mode) |
| Knob 2 | Feedback | 0 % – 98 % (hard clamped to prevent runaway) |
| SW1 short press | Freeze momentary | Hold for freeze, release to unfreeze |
| SW1 hold | Shift modifier | See Shifted Control Map |
| SW2 | Tap Tempo | Tap 2–4× to set BPM; rolling average of last 4 taps |
| Pod Encoder turn | Wet/Dry Mix | 0 % dry → 100 % wet |
| Pod Encoder push | Freeze Latch toggle | Latches / unlatches freeze — OLED shows `[FRZ]` |
| Ext Encoder turn | Menu navigation | Move cursor between items on active page |
| Ext PSH short | Edit / commit | Enter edit mode on selected item; confirm value |
| Ext PSH long (500 ms) | Quick preset save | Save to last-used slot without entering P5 |
| BAK short | Back / cancel | Exit edit mode; go up one level |
| CON short | Confirm / apply | Confirm selected preset or menu action |

---

## Shifted Control Map (SW1 held)

| Control | Shifted Function | Range / Notes |
|---|---|---|
| Shift + Knob 1 | Delay Subdivision | 1/1 · 1/2 · 1/4 · 1/8 · 1/16 · dotted · triplet |
| Shift + Knob 2 | Feedback Tone / Damping | Feedback LP cutoff macro (500–18000 Hz) |
| Shift + SW2 | Delay Mode toggle | Synced BPM ↔ Free time |
| Shift + Pod Enc turn | Drive amount | 0 % – 100 % soft saturation depth |
| Shift + Pod Enc push | Page quick jump | Cycles P1 → P2 → P3 → P4 → back to P1 |
| Shift + Ext PSH long | Alternate Freeze Mode | Latch ↔ Momentary mode toggle |

---

## Beat Divisions (Knob 1 — Delay Time, Sync Mode)

| Knob Zone | Division | Musical Name |
|---|---|---|
| 0–10 % | 1/16 | Sixteenth note |
| 10–20 % | 1/16d | Dotted sixteenth |
| 20–30 % | 1/16t | Sixteenth triplet |
| 30–40 % | 1/8 | Eighth note |
| 40–50 % | 1/8d | Dotted eighth |
| 50–60 % | 1/8t | Eighth triplet |
| 60–70 % | 1/4 | Quarter note |
| 70–80 % | 1/4d | Dotted quarter |
| 80–90 % | 1/2 | Half note |
| 90–100 % | 1/1 | Whole note |

In Free Time mode Knob 1 maps linearly: 10 ms – 2000 ms.

---

## Page / Effect / Control Summary Table

| Page | Parameter / Effect | Primary Control | Shifted / Alt Control | Display |
|---|---|---|---|---|
| **P1 Perform** | Delay Time | Knob 1 | Shift+Knob1 = Subdivision | Beat div or ms |
| **P1 Perform** | BPM | SW2 (tap) / Ext Enc (edit) | — | BPM value |
| **P1 Perform** | Feedback | Knob 2 | Shift+Knob2 = FB Tone | 0–98 % |
| **P1 Perform** | Wet/Dry Mix | Pod Enc turn | — | 0–100 % |
| **P1 Perform** | Drive | Shift + Pod Enc turn | — | 0–100 % |
| **P1 Perform** | Freeze | SW1 short (momentary) / Pod Enc push (latch) | Shift+Ext PSH long = mode toggle | FRZ ON/OFF/LATCH |
| **P1 Perform** | Delay Mode | Shift + SW2 | — | SYNC / FREE |
| **P1 Perform** | Page jump | Shift + Pod Enc push | — | Cycles P1–P4 |
| **P2 Tone** | Input HP cutoff | Ext Enc (navigate + edit) | — | Hz |
| **P2 Tone** | Feedback LP cutoff | Ext Enc · or Shift+Knob2 | — | Hz |
| **P2 Tone** | Feedback HP cutoff | Ext Enc | — | Hz |
| **P2 Tone** | Wet Tone / Damping | Ext Enc | — | Hz |
| **P2 Tone** | Output Tilt | Ext Enc | — | −6 to +6 dB |
| **P3 Motion** | Wow Depth | Ext Enc | — | 0–100 % |
| **P3 Motion** | Wow Rate | Ext Enc | — | 0.1–5 Hz |
| **P3 Motion** | Modulation Enable | Ext Enc / Ext PSH | — | ON / OFF |
| **P4 Freeze** | Freeze Mode | Ext Enc | — | Momentary / Latch |
| **P4 Freeze** | Hold Behavior | Ext Enc | — | Loop / Hold-last |
| **P4 Freeze** | Loop Size | Ext Enc | — | 50–2000 ms |
| **P5 Presets** | Load Preset | Ext Enc navigate + CON | — | Preset name |
| **P5 Presets** | Save Preset | Ext Enc navigate + CON | Ext PSH long (quick save) | Slot number |
| **P5 Presets** | Init / Default | Ext Enc + CON confirm | — | — |
| **P6 System** | OLED Brightness | Ext Enc | — | 25/50/75/100 % |
| **P6 System** | Encoder Direction | Ext Enc | — | Normal / Flipped |
| **P6 System** | Bypass Mode | Ext Enc | — | Active / Bypass |
| **P6 System** | Sample Rate | Read-only | — | 48 kHz |
| **P6 System** | Block Size | Read-only | — | 48 samples |
| **P6 System** | Firmware Version | Read-only | — | vX.Y.Z |

---

## OLED Pages

### P1 — Perform (default)

```
┌────────────────────────┐
│ PERFORM        [SHF?]  │
│ Time: 1/8d  BPM: 120  │
│ FB:  65%   Mix:  48%  │
│ Drive: 30%  [FRZ OFF] │
└────────────────────────┘
```

| Item | Primary Control | Ext Edit |
|---|---|---|
| Time | Knob 1 | Ext Enc (in edit mode) |
| BPM | SW2 tap | Ext Enc (in edit mode) |
| Feedback | Knob 2 | Ext Enc (in edit mode) |
| Mix | Pod Enc | Ext Enc (in edit mode) |
| Drive | Shift + Pod Enc | — |
| Freeze | SW1 / Pod Enc push | — |

### P2 — Tone

| Item | Range | Unit |
|---|---|---|
| Input HP | 20–500 Hz | Hz |
| Feedback LP | 500–18000 Hz | Hz |
| Feedback HP | 20–500 Hz | Hz |
| Wet Tone / Damping | 1000–20000 Hz | Hz |
| Output Tilt | −6 to +6 dB | dB |

All items: Ext Enc navigate → Ext PSH to enter edit → Ext Enc to change → Ext PSH / CON to confirm.

### P3 — Motion

| Item | Range | Unit |
|---|---|---|
| Wow Depth | 0–100 % | % |
| Wow Rate | 0.1–5 Hz | Hz |
| Modulation Enable | ON / OFF | — |

### P4 — Freeze

| Item | Range |
|---|---|
| Freeze Mode | Momentary / Latch |
| Hold Behavior | Loop / Hold-last |
| Loop Size | 50–2000 ms |

### P5 — Presets

| Item | Action |
|---|---|
| Load Preset | Ext Enc navigate → CON confirm |
| Save Preset | Ext Enc navigate → CON confirm; or Ext PSH long (quick save) |
| Init / Default | Resets all params to factory default |

Factory presets: Kick Dub · Snare Space · Metal Burst · Noise Wash · Bass Echo Drive

### P6 — System

| Item | Editable | Notes |
|---|---|---|
| OLED Brightness | Ext Enc | 25 / 50 / 75 / 100 % |
| Encoder Direction | Ext Enc | Normal / Flipped — calls `ext_enc.SetFlipped()` |
| Bypass Mode | Ext Enc | Active / Bypass |
| Sample Rate | Read-only | 48 kHz |
| Block Size | Read-only | 48 samples |
| Firmware Version | Read-only | vX.Y.Z |

---

## FxParams Struct Reference

```cpp
struct FxParams {
    float input_gain_db;      // P1 Trim
    float hp_hz;              // P2 Input HP
    float drive;              // P1/P3 Drive (0–1)
    float delay_time_beats;   // P1 Time (beat fraction, sync mode)
    float delay_time_ms;      // P1 Time (free mode, ms)
    float feedback;           // P1 Feedback (0–0.98)
    float fb_lp_hz;           // P2 Feedback LP
    float fb_hp_hz;           // P2 Feedback HP
    float wow_depth;          // P3 Wow Depth (0–1)
    float wow_rate_hz;        // P3 Wow Rate
    bool  wow_enabled;        // P3 Modulation Enable
    float diffuse_amt;        // P2 Wet Tone / Damping
    float wet;                // P1 Mix (0–1)
    float output_tilt_db;     // P2 Output Tilt
    float limiter_drive;      // internal — not user-editable
    float freeze_size_ms;     // P4 Loop Size
    bool  freeze_reverse;     // P4 — future
    bool  freeze_latched;     // P4 Freeze Mode
    float bpm;                // P1 BPM (40–240)
    bool  sync_mode;          // P1 Delay Mode (synced / free)
    bool  bypass;             // P6 Bypass
};
```

> Write from main loop with `__DSB()` barrier. Read from AudioCallback as cached snapshot — no `hw.*` calls in ISR.

---

## OLED Display Conventions

- **Top-right corner**: `[SHF]` when SW1 is held; `[FRZ]` when freeze is active
- **Selected item**: inverted highlight (white bg, black text)
- **Edit mode**: blinking cursor on selected value
- **BPM**: always visible on P1; status bar on other pages if space permits
- **Refresh**: `if(now - last_draw >= 33)` in main loop only — never from ISR

---

## Implementation Notes

- All control reads must occur in the **main loop** — never in `AudioCallback`
- `ext_enc.Debounce()` must be called every main loop iteration for correct 1 ms debounce
- `pod.ProcessAllControls()` handles Knob1, Knob2, SW1 (`button1`), SW2 (`button2`), Pod encoder
- **SW1 Shift detection**: use a held-time counter — short press (<200 ms, no other control touched) = Freeze momentary; otherwise = Shift modifier while held
- **Tap tempo**: rolling average of last 4 intervals; ignore taps > 3 s apart (resets sequence)
- **Freeze engage/disengage**: crossfade over 5–10 ms to prevent clicks (§7.6 of SOW)
- **Parameter smoothing**: `fonepole(current, target, 0.001f)` per param in AudioCallback
