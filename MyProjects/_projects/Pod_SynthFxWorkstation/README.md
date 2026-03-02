# Pod_SynthFxWorkstation

MIDI-driven synth workstation for Daisy Pod:

`MIDI -> Grainlet + Particle + Dust -> FX Bus -> Distortion -> Stereo Out`

No audio-input path is used.

## Controls

- `Encoder Click`: cycle parameter page (`VOICE -> FX -> DRIVE`)
- `Button 1 (B1)`: toggle `Drone` mode on/off
- `Button 2 (B2)`: cycle internal FX preset (`0..3`)

### Page: VOICE

- `K1`: `voice_blend` (Grainlet vs Particle/Dust)
- `K2`: `timbre` (formant/filter/noise behavior)

### Page: FX

- `K1`: `fx_amount` (global send to FX bus)
- `K2`: `delay_time_ms` (35 to 455 ms)

### Page: DRIVE

- `K1`: `dist_drive` (post-FX overdrive)
- `K2`: `reverb_amount` (also influences reverb feedback)

## Startup Defaults

Code defaults at boot:

- `page = VOICE`
- `voice_blend = 0.65`
- `timbre = 0.55`
- `fx_amount = 0.45`
- `delay_time_ms = 260.0`
- `delay_feedback = 0.42`
- `dist_drive = 0.55`
- `reverb_amount = 0.35`
- `reverb_feedback = 0.62`
- `fx_preset = 0`
- `drone_mode = false`

Envelope defaults:

- `attack = 0.003 s`
- `decay = 0.12 s`
- `sustain = 0.72`
- `release = 0.30 s`

Important:

- Knobs are live (no soft takeover). On boot, current physical knob positions can immediately overwrite the active page parameters (`VOICE` page first).

## Built-in FX Presets (B2)

These scale Chorus/Delay/Reverb sends inside the FX bus:

- `Preset 0`: Dry-ish (`chorus=0.00, delay=0.00, reverb=0.00`)
- `Preset 1`: Wide chorus texture (`chorus=0.75, delay=0.20, reverb=0.15`)
- `Preset 2`: Delay-forward (`chorus=0.30, delay=0.85, reverb=0.35`)
- `Preset 3`: Ambient wash (`chorus=0.55, delay=0.45, reverb=0.95`)

## Suggested Sound Presets

Use these as quick starting points:

### 1) Grain Cloud Pad

- FX preset: `3`
- VOICE: `K1=0.70`, `K2=0.75`
- FX: `K1=0.65`, `K2=0.55`
- DRIVE: `K1=0.22`, `K2=0.70`

### 2) Broken Digital Lead

- FX preset: `2`
- VOICE: `K1=0.35`, `K2=0.85`
- FX: `K1=0.45`, `K2=0.30`
- DRIVE: `K1=0.78`, `K2=0.25`

### 3) Tight Hybrid Pluck

- FX preset: `1`
- VOICE: `K1=0.82`, `K2=0.40`
- FX: `K1=0.28`, `K2=0.22`
- DRIVE: `K1=0.42`, `K2=0.18`

## Build / Flash

```bash
make
make program-dfu-safe
```

## Serial Monitor

USB serial prints:

- boot help
- periodic `[STATUS]` telemetry
- `[MIDI]` NoteOn/NoteOff events

The firmware uses `StartLog(true)`, so connect a serial monitor after boot/flash.
