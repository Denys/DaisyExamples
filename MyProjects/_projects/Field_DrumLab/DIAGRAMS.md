# DIAGRAMS — Field_DrumLab

## 1. System Architecture

```mermaid
block-beta
    columns 3
    MIDI["MIDI In\n(Ch 10 — GM Perc)"] CPU["Daisy Field\nARM Cortex-M7\n48kHz / 48 sample blocks"] AUDIO["Stereo Out\nL / R"]
    KNOBS["8 Knobs\nK1-K7: Focus params\nK8: Master Volume"] DSP["5-Voice DSP\nAnalogBassDrum\nSyntheticSnareDrum\nHiHat\nAnalogSnareDrum\nSyntheticBassDrum"] OLED["128×64 OLED\nOverview + Zoom\n(1.4s timeout)"]
    KEYS["A1-A7: Trigger Pads\nB1-B5: Focus Select\nSW1: Page  SW2: Panic"] space LEDS["Key LEDs\nA: Trigger Flash\nB: Focus On/Dim"]
```

---

## 2. Signal Flow

```mermaid
flowchart LR
    A1["A1 Kick Pad"] --> KICK["AnalogBassDrum\n(kick)"]
    A2["A2 Snare Pad"] --> SNARE["SyntheticSnareDrum\n(snare)"]
    A3["A3 Closed Hat"] --> HIHAT["HiHat\n(closed/open)"]
    A4["A4 Open Hat"] --> HIHAT
    A5["A5 Clap Pad"] --> CLAP["AnalogSnareDrum\n(clap)"]
    A6["A6 Perc Pad"] --> PERC["SyntheticBassDrum\n(perc)"]
    MIDI["MIDI Ch10"] --> KICK & SNARE & HIHAT & CLAP & PERC

    KICK -->|"lvl × pan"| MIXL["Stereo Mixer L"]
    KICK -->|"lvl × pan"| MIXR["Stereo Mixer R"]
    SNARE -->|"lvl × pan"| MIXL & MIXR
    HIHAT -->|"lvl × pan"| MIXL & MIXR
    CLAP -->|"lvl × pan"| MIXL & MIXR
    PERC -->|"lvl × pan"| MIXL & MIXR

    MIXL -->|"× master_vol\nfclamp ±0.95"| OUTL["Out L"]
    MIXR --> OUTR["Out R"]
```

**HiHat choke**: A3 (closed) sets `hihat_open = false` before triggering — A4 (open) sets `hihat_open = true`. Decay in the audio callback uses `open_hat ? 0.7f : v_decay[HiHat]`.

---

## 3. Control Flow

```mermaid
flowchart LR
    LOOP["Main Loop\nSystem::Delay(4ms)"] --> PROC["hw.ProcessAllControls()"]
    PROC --> SNAP["SnapshotKnobs()\nMove threshold: 0.015\nFocus timeout: 1.4s"]
    PROC --> ROUTE["RouteFocusKnobs()\nSynth page: K1-K7 → focused voice\nMix page: K1-K5 levels, K6 M.Decay, K7 M.Accent\nK8 always → master_vol"]

    PROC --> AROW["A1-A7 RisingEdge\nSet trig flags + flash timers\nA3/A4: set hihat_open"]
    PROC --> BROW["B1-B5 RisingEdge\nSet focus_voice"]
    PROC --> SW["SW1 RisingEdge → page toggle\nSW2 RisingEdge → Panic()"]

    LOOP --> LEDS["UpdateLeds()\nA: flash kTriggerFlashMs=80ms\nB: LedForFocus brightness\nKnob LEDs: knob_values[i]"]
    LOOP --> OLED["UpdateOled()\nZoom if knob moved ≤1.4s\nOverview otherwise"]

    AUDIOCB["AudioCallback\n(48 sample blocks)"] --> PARAMS["Read volatile params\nonce per block"]
    PARAMS --> VOICES["Process 5 × Process(trig i==0)\nApply per-voice level × mix_level\nLinear stereo pan\nfclamp ±0.95"]
```
