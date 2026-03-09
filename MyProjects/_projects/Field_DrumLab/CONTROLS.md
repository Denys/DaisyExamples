# CONTROLS — Field_DrumLab

## Overview

`Field_DrumLab` is a 5-voice drum synthesizer on the Daisy Field. All five drum engines trigger independently. The B-row selects one "Focus Voice" at a time; K1-K7 then edit that voice's 7 parameters live. SW1 toggles between **Synth page** (voice parameters) and **Mix page** (per-voice levels + global controls). K8 is always Master Volume.

---

## Knobs

### Synth Page (SW1 LED = dim white)

| Knob | Parameter | Formula | Range |
|------|-----------|---------|-------|
| `K1` | Focus: Pitch | Voice-specific frequency map | Kick 30-200 Hz · Snare 100-400 Hz · HiHat 1-12 kHz · Clap 150-500 Hz · Perc 100-800 Hz |
| `K2` | Focus: Decay | `SetDecay(knob × master_decay)` | 0 = instant snap · 1 = long ring |
| `K3` | Focus: Tone | Kick/Clap/Perc: `SetTone()` · Snare: `SetFmAmount()` · HiHat: `SetTone()` | Dark tonal body to bright/noisy |
| `K4` | Focus: Timbre | Kick: (fixed) · Snare: `SetSnappy()` · HiHat: `SetNoisiness()` · Clap: `SetSnappy()` · Perc: `SetDirtiness()` | Engine-specific character knob |
| `K5` | Focus: Accent | `SetAccent(knob × master_accent)` | Soft hit to hard accent |
| `K6` | Focus: Pan | Linear pan 0=full L, 0.5=centre, 1=full R | Hard L to Hard R |
| `K7` | Focus: Level | Per-voice output gain | Silence to full |
| `K8` | Master Volume | Final output scalar | Silence to full |

### Mix Page (SW1 LED = bright white)

| Knob | Parameter | Formula | Range |
|------|-----------|---------|-------|
| `K1` | Kick Level | `mix_level[0]` | 0-100% |
| `K2` | Snare Level | `mix_level[1]` | 0-100% |
| `K3` | HiHat Level | `mix_level[2]` | 0-100% |
| `K4` | Clap Level | `mix_level[3]` | 0-100% |
| `K5` | Perc Level | `mix_level[4]` | 0-100% |
| `K6` | Master Decay | `master_decay = 0.2 + knob × 1.6` | Tighter (×0.2) to looser (×1.8) all voices |
| `K7` | Master Accent | `master_accent = knob` | Soft to hard for all voices |
| `K8` | Master Volume | (same as Synth page) | |

---

## A-Row Trigger Pads

| Key | Voice | Behavior |
|-----|-------|----------|
| `A1` | Kick (AnalogBassDrum) | Rising edge triggers; accent from K5 |
| `A2` | Snare (SyntheticSnareDrum) | Rising edge triggers |
| `A3` | Closed HiHat | Sets `hihat_open = false` then triggers short decay |
| `A4` | Open HiHat | Sets `hihat_open = true` then triggers long decay (0.7) |
| `A5` | Clap (AnalogSnareDrum) | Rising edge triggers; high snappy |
| `A6` | Perc (SyntheticBassDrum) | Rising edge triggers; click + attack noise |
| `A7` | **Crash — all voices** | Triggers all 5 voices simultaneously with open hat |
| `A8` | Reserved | No action |

**HiHat choke**: Pressing A3 (closed) while A4 (open) is still ringing immediately sets decay to closed-hat mode for the next sample block.

### A-Row LED Behavior

Each A-row LED (A1-A6) **flashes on for 80 ms** when its voice is triggered, then returns to off.

---

## B-Row Focus Selection

| Control | Function | LED Behavior |
|---------|----------|-------------|
| `B1` | Focus: Kick | Full brightness when focused, dim (8%) otherwise |
| `B2` | Focus: Snare | Full brightness when focused, dim otherwise |
| `B3` | Focus: HiHat | Full brightness when focused, dim otherwise |
| `B4` | Focus: Clap | Full brightness when focused, dim otherwise |
| `B5` | Focus: Perc | Full brightness when focused, dim otherwise |
| `B6-B8` | Unused | Off |

Pressing a B-row key immediately re-routes K1-K7 to that voice.

---

## Switches

| Switch | Function | LED Behavior |
|--------|----------|-------------|
| `SW1` | Page toggle: Synth ↔ Mix | Dim = Synth page · Bright = Mix page |
| `SW2` | Panic — clears all trigger pending flags | Always off |

---

## MIDI Behavior (Channel 10 — GM Percussion)

| MIDI Note | Voice Triggered |
|-----------|-----------------|
| 35, 36 (B1, C2) | Kick |
| 38, 40 (D2, E2) | Snare |
| 42, 44 (F#2, G#2) | Closed HiHat |
| 46 (A#2) | Open HiHat |
| 39 (D#2) | Clap |
| 37, 56 (C#2, G#3) | Perc |

- **Velocity** maps to `SetAccent()` on the triggered voice (`accent = velocity / 127.0`).
- MIDI triggers cannot be combined with A-row focus on the same sample block edge.

---

## OLED Display

### Overview (idle — no knob moved in last 1.4 s)

```
DRUM LAB  [KICK]
SYN  Vol:80%
Pit:55Hz Dec:40%
Ton:50%  Tmb:50%
Acc:60%  Lv:80%
K8:Vol B1-5:Focus
A1-6:Pad A7:ALL
SW1:Page SW2:Panic
```

Row 0: Project name + `[focused voice short name]`
Row 1: Current page (SYN/MIX) + Master Volume %
Rows 2-4: Four parameter pairs of the focused voice
Rows 5-7: Control hints

### Focused Parameter Zoom (1.4 s after any knob movement)

```
KICK > Pitch
     55 Hz
KICK | SYNTH
[████████░░░░]
```

Line 0: `<Voice full name> > <Parameter name>`
Line 1: Large value in physical units (Hz / % / L/R)
Line 3: Voice and page context
Line 4: Progress bar (128 px wide)

---

## Stereo Panning

| Voice | Default Pan | Physical Position |
|-------|------------|-------------------|
| Kick | 0.50 | Centre |
| Snare | 0.60 | Slightly right |
| HiHat | 0.70 | Right |
| Clap | 0.40 | Slightly left |
| Perc | 0.35 | Left |

---

## Startup Defaults

| Voice | Pitch | Decay | Tone | Timbre | Accent | Pan | Level |
|-------|-------|-------|------|--------|--------|-----|-------|
| Kick | 55 Hz | 40% | 50% | 50% | 60% | 50% | 80% |
| Snare | 200 Hz | 40% | 50% | 50% | 60% | 60% | 80% |
| HiHat | 3000 Hz | 20% | 50% | 80% | 80% | 70% | 80% |
| Clap | 300 Hz | 30% | 60% | 85% | 60% | 40% | 80% |
| Perc | 400 Hz | 20% | 50% | 30% | 60% | 35% | 80% |

- Focus voice on boot: **Kick** (B1)
- Page on boot: **Synth**
- Master Volume: 80%
- Mix page levels: all 80%

---

## Fixed Internal Settings

| Block | Setting | Value | Description |
|-------|---------|-------|-------------|
| AnalogBassDrum | Self FM | 0.20 | Pitch feedback for punch |
| AnalogBassDrum | Attack FM | 0.30 | Transient click |
| SyntheticSnareDrum | FM Amount | 0.40 | Body modulation depth |
| HiHat | Open hat decay | 0.70 | Forced when A4 or MIDI note 46 |
| SyntheticBassDrum | FM Env Amount | 0.50 | Pitch sweep attack |
| SyntheticBassDrum | FM Env Decay | 0.30 | Pitch sweep speed |
| Output | Buffer ceiling | ±0.95 | Hard clip via `fclamp` |
| Output | Routing | Stereo per-voice pan | Linear pan law |

---

## Preset — Init (Boot Default)

```
Kick:  Pitch=55Hz, Decay=40, Tone=50, Timbre=50, Accent=60, Pan=CTR, Lvl=80
Snare: Pitch=200Hz, Decay=40, Tone=50, Timbre=50, Accent=60, Pan=R12, Lvl=80
HiHat: Pitch=3kHz,  Decay=20, Tone=50, Timbre=80, Accent=80, Pan=R40, Lvl=80
Clap:  Pitch=300Hz, Decay=30, Tone=60, Timbre=85, Accent=60, Pan=L20, Lvl=80
Perc:  Pitch=400Hz, Decay=20, Tone=50, Timbre=30, Accent=60, Pan=L30, Lvl=80
Page=Synth  Focus=Kick  Vol=80%
```

---

## Serial Boot Log

At boot, the following lines are printed:

```
Field_DrumLab ready
A1=Kick A2=Snare A3=CHat A4=OHat A5=Clap A6=Perc A7=ALL
B1-B5=Focus  SW1=Page(Syn/Mix)  SW2=Panic
MIDI ch10: 36=Kick 38=Snr 42=CHat 46=OHat 39=Clap 37=Perc
```

---

## Known Limitations

- No internal sequencer; requires external MIDI sequencer or manual pad triggering
- Only one HiHat instance; simultaneous closed + open is not possible
- No individual voice audio outputs — all voices sum to stereo pair
- No patch memory or preset save system (RAM-only; resets to defaults on boot)
- A7 crash-all may clip at high accent; hardware limiter on Daisy Field is not present — fclamp is the only protection
- `SyntheticSnareDrum` has no `SetTone()` — K3 maps to `SetFmAmount()` instead (FM sweep depth)
