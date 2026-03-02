# CONTROLS — x0x Drum Machine & 16-Step Sequencer
## Platform: Daisy Field | Project 6

---

## Knob Mapping

| Knob | Parameter | Range | Unit | DSP Call |
|------|-----------|-------|------|----------|
| K1   | Tempo (BPM) | 60–180 | BPM | `metro.SetFreq((bpm/60.f)*4.f)` |
| K2   | Kick Frequency | 50–250 | Hz | `bass_drum.SetFreq()` |
| K3   | Kick Decay | 0–1 | — | `bass_drum.SetDecay()` |
| K4   | Snare Snappy | 0–1 | — | `snare_drum.SetSnappy()` |
| K5   | Snare Decay | 0–1 | — | `snare_drum.SetDecay()` |
| K6   | Hi-Hat Decay | 0–1 | — | `hihat` decay param |
| K7   | Kick Accent | 0–1 | — | `bass_drum.SetAccent()` |
| K8   | Master Volume | 0–1 | — | Output gain scalar |

All knob values are fonepole()-smoothed before being applied to DSP params.

---

## Button Mapping

| Button | Function | Behavior |
|--------|----------|----------|
| BTN1   | Cycle active drum | RisingEdge: Kick → Snare → HiHat → Kick (wraps) |
| BTN2   | Clear pattern | RisingEdge: clear all 16 steps for current drum only |

Active drum is shown on OLED title immediately on BTN1 press.

---

## Key Mapping (Field 16-key keyboard)

| Keys | Indices | Function | Effect |
|------|---------|----------|--------|
| A1–A8 | kKeyAIndices[0–7] = {0..7} | Toggle Steps 1–8 | Flips step bit for **active drum** |
| B1–B8 | kKeyBIndices[0–7] = {8..15} | Toggle Steps 9–16 | Flips step bit for **active drum** |

Active drum is selected via BTN1. Only one drum's pattern is shown/edited at a time.

---

## LED Behavior

| Situation | LED State |
|-----------|-----------|
| Step is ON for active drum | Key LED lit |
| Step is OFF for active drum | Key LED off |
| Sequencer advances to step N | All LEDs: brief flash on key N position (both rows) |
| BTN1 pressed | All LEDs reload to show new active drum's pattern |
| BTN2 pressed | All LEDs go off (pattern cleared) |

LED index mapping uses `FieldKeyboardLEDs` from `field_defaults.h`.
Note: A-row LED indices are **reversed** (`kLedKeysA` = {15,14,13,12,11,10,9,8}).

---

## OLED Display

| Mode | Content |
|------|---------|
| Idle | Title: `KICK` / `SNARE` / `HIHAT` (active drum) + `BPM: 120` |
| Param zoom | 1.2s popup on any knob move → e.g. `K2: 85Hz`, `K1: 120BPM` |
| Step indicator | Bottom pixel row = 16-dot progress bar, current_step highlighted |

Display is managed by `FieldOLEDDisplay` from `field_defaults.h`.
`display.Update()` called in main loop only — never in audio callback.

---

## Default Preset (power-on)

### Step Patterns
| Drum | Active Steps | Pattern |
|------|-------------|---------|
| Kick | 0, 4, 8, 12 | 4-on-the-floor |
| Snare | 4, 12 | Backbeat (2 & 4) |
| Hi-Hat | 0,2,4,6,8,10,12,14 | 8th notes |

### Parameters
| Parameter | Value |
|-----------|-------|
| BPM | 120 |
| Kick Freq | 60 Hz |
| Kick Decay | 0.5 |
| Kick Accent | 0.5 |
| Snare Snappy | 0.5 |
| Snare Decay | 0.5 |
| HiHat Decay | 0.4 |
| Master Vol | 0.8 |

---

## Signal Chain Summary

```
Metro (K1: BPM) → Step Counter (0-15) → step_arrays[3][16]
                                          ├── kick_steps  → AnalogBassDrum  (K2,K3,K7)
                                          ├── snare_steps → AnalogSnareDrum (K4,K5)
                                          └── hihat_steps → HiHat           (K6)
                                                            ↓
                                              Weighted Mix × K8 → out[0][i] / out[1][i]
```
