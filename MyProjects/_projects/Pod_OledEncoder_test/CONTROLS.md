# CONTROLS — Pod Tonestack + Nonlinear Filter

## Project: Pod_OledEncoder_test
**Location:** `DaisyExamples/MyProjects/_projects/Pod_OledEncoder_test`
**Platform:** Daisy Pod + OLED + 2PB + Encoder

---

## Control Mapping Table

### Mode A: TONE Mode (Default — no SW held)

| Control | Parameter | Min | Max | Default | Unit/Notes |
|---------|-----------|-----|-----|---------|------------|
| **Knob1** | Bass (ToneStack) | -1.0 | +1.0 | 0.0 | ToneStack bass |
| **Knob2** | Treble (ToneStack) | -1.0 | +1.0 | 0.0 | ToneStack treble |
| **Encoder** | Middle (ToneStack) | -1.0 | +1.0 | 0.0 | ToneStack mid |
| **Button1** | ToneStack Bypass | Off | On | Off | Toggle |
| **Button2** | NlFilt Bypass | Off | On | Off | Toggle |
| **Encoder Click** | Reset Middle | — | — | — | Sets to 0.0 |

### Mode B: LFO Mode (SW1 held)

| Control | Parameter | Min | Max | Default | Unit/Notes |
|---------|-----------|-----|-----|---------|------------|
| **Knob1** | LFO Rate | 0.1 | 10.0 | 1.0 | Hz, logarithmic |
| **Knob2** | LFO Depth | 0.0 | 1.0 | 0.5 | Modulation amount |
| **Encoder** | LFO Wave | 0 | 3 | 0 | 0=Sin, 1=Tri, 2=Saw, 3=Sqr |
| **Button1** | ToneStack Bypass | Off | On | Off | Same as default |
| **Button2** | NlFilt Bypass | Off | On | Off | Same as default |
| **SW1 Hold** | Enter LFO Mode | — | — | — | While held |

### Mode C: NL Mode (SW2 held)

| Control | Parameter | Min | Max | Default | Unit/Notes |
|---------|-----------|-----|-----|---------|------------|
| **Knob1** | Attack (Envelope) | 0.01 | 2.0 | 0.1 | Seconds, logarithmic |
| **Knob2** | Release (Envelope) | 0.1 | 5.0 | 0.5 | Seconds, logarithmic |
| **Encoder** | NlFilt Coef "d" | 0.0 | 3.0 | 0.5 | Nonlinear drive |
| **Button1** | ToneStack Bypass | Off | On | Off | Same as default |
| **Button2** | NlFilt Bypass | Off | On | Off | Same as default |
| **SW2 Hold** | Enter NL Mode | — | — | — | While held |

---

## LED Behavior

| LED | Function | Behavior |
|-----|----------|----------|
| **LED1** | ToneStack Status | ON = active, OFF = bypassed |
| **LED2** | NlFilt Status | ON = active, OFF = bypassed |

---

## OLED Display Layout

### Default (Tone Mode)
```
┌────────────────────────────────┐
│  POD TONESTACK v1.0           │
│                                │
│  B: ████████░░  +0.3          │
│  M: ██████████   0.0          │
│  T: ██████░░░░  -0.2          │
│                                │
│  [NL] ON   [TS] ON            │
└────────────────────────────────┘
```

- **B/M/T**: Bass/Mid/Treble levels (8-segment bar)
- **Values**: Current parameter value
- **Status**: Bypass state for each stage

### Focus View (1.2s after any knob move)
```
┌────────────────────────────────┐
│         FOCUS: MID            │
│                                │
│         ████████░░            │
│           +0.5                │
│                                │
└────────────────────────────────┘
```

### LFO Mode (SW1 held)
```
┌────────────────────────────────┐
│        LFO MODE               │
│   ╱╲ (Sin) ╱╲ ╱╲              │
│                                │
│  Rate: 2.5Hz  Depth: ████░░   │
│                                │
└────────────────────────────────┘
```

### NL Mode (SW2 held)
```
┌────────────────────────────────┐
│        NL MODE                │
│  Att: ████░░  Rel: ████░░     │
│  Coef d: ██████░░ +0.5        │
│                                │
└────────────────────────────────┘
```

---

## Presets

### Init (Default)
| Param | Value |
|-------|-------|
| Bass | 0.0 |
| Middle | 0.0 |
| Treble | 0.0 |
| NlFilt Coefs | a=0, b=0, d=0, C=0, L=0 |
| LFO Rate | 1.0 Hz |
| LFO Depth | 0.5 |
| LFO Wave | 0 (Sin) |
| Attack | 0.1s |
| Release | 0.5s |

### Preset 1: Clean Guitar
| Param | Value |
|-------|-------|
| Bass | 0.0 |
| Middle | 0.0 |
| Treble | +0.2 |
| NlFilt Coefs | a=0, b=0, d=0, C=0, L=0 |

### Preset 2: Overdrive
| Param | Value |
|-------|-------|
| Bass | +0.3 |
| Middle | +0.5 |
| Treble | -0.2 |
| NlFilt Coefs | a=1.5, b=-0.9, d=0.5, C=0, L=10 |

### Preset 3: Fuzz Chaos
| Param | Value |
|-------|-------|
| Bass | +0.5 |
| Middle | +0.8 |
| Treble | 0.0 |
| NlFilt Coefs | a=2.0, b=-1.5, d=1.0, C=0.1, L=20 |

---

## Bypass Logic

| Scenario | ToneStack | NlFilt | LED1 | LED2 |
|----------|-----------|--------|------|------|
| Both ON | Process | Process | ON | ON |
| TS Bypassed | Bypass | Process | OFF | ON |
| NlFilt Bypassed | Process | Bypass | ON | OFF |
| Both Bypassed | Bypass | Bypass | OFF | OFF |

---

## Mode Transition Notes

- **Switching modes**: Parameters retain current value (no reset)
- **LFO Mode**: Applies modulation to NlFilt coef "d" only
- **NL Mode**: Envelope applies to coef "d" with attack/release
- Both SW held: NL Mode takes precedence (SW2 > SW1)