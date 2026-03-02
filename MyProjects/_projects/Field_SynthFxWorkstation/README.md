# Field_SynthFxWorkstation

Daisy Field synth+FX workstation with a patch-focused workflow:

`External MIDI -> Grainlet + Particle + Dust -> FX bus -> Distortion -> Stereo out`

## Interface

- External MIDI input via `hw.midi` (`NoteOn`/`NoteOff`)
- OLED status page (active patch, gate, note/freq, key knob values)
- USB serial monitor logs (`[BOOT]`, `[PATCH]`, `[MIDI]`, `[STATUS]`)

## Controls

- `A1`: load `Bass` patch
- `A2`: load `Kick` patch
- `A3`: load `Snare` patch
- `A4`: toggle drone mode
- `B1/B2/B3`: trigger test notes `36 / 38 / 43` (quick audition)
- `SW1`: toggle drone mode
- `SW2`: panic (force gate off)

Knobs:

- `K1`: blend (Grainlet vs Particle/Dust)
- `K2`: timbre
- `K3`: filter cutoff
- `K4`: decay
- `K5`: FX amount
- `K6`: delay time
- `K7`: distortion drive
- `K8`: reverb amount

## Startup Default

On boot, `Bass` patch loads automatically.

Knob takeover is enabled:

- Patch defaults stay active after patch load.
- A knob starts controlling its parameter only after the physical knob reaches the stored value.

## Patch Defaults (recommended starts)

Values are shown as percentages for quick dialing.

### Patch A1: Bass

- `K1 Blend`: `84%`
- `K2 Timbre`: `32%`
- `K3 Cutoff`: `28%`
- `K4 Decay`: `20%` (about `0.26s`)
- `K5 FX`: `32%`
- `K6 Delay`: `25%` (about `170ms`)
- `K7 Drive`: `39%`
- `K8 Reverb`: `20%`

Use for rounded sub-bass and dark plucks.

### Patch A2: Kick Synth

- `K1 Blend`: `94%`
- `K2 Timbre`: `42%`
- `K3 Cutoff`: `22%`
- `K4 Decay`: `6%` (about `0.09s`)
- `K5 FX`: `15%`
- `K6 Delay`: `10%` (about `85ms`)
- `K7 Drive`: `73%`
- `K8 Reverb`: `8%`

Use with MIDI note `36` for punchy electronic kick tones.

### Patch A3: Snare Noise

- `K1 Blend`: `40%`
- `K2 Timbre`: `78%`
- `K3 Cutoff`: `66%`
- `K4 Decay`: `8%` (about `0.11s`)
- `K5 FX`: `28%`
- `K6 Delay`: `16%` (about `120ms`)
- `K7 Drive`: `56%`
- `K8 Reverb`: `18%`

Use around MIDI notes `38-40` for snare/noise hits.

## Build and Flash

```bash
make
make program-dfu
```
