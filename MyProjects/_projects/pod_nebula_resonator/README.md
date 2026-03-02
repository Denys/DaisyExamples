# Nebula-Resonator

A standalone MIDI-driven granular texture synthesizer for Daisy Pod. An internal synth drone fills a 2-second freeze buffer in SDRAM. When frozen, a 4-tap granular engine with jittered read heads creates evolving stereo cloud textures. An 8-step parameter sequencer animates the scan position rhythmically.

## Features

- Internal synthesis engine (BlOsc + WhiteNoise) — no external audio input needed
- Freeze Buffer (96k samples SDRAM) with conditional write/continuous read
- 4-tap granular TextureVoice with stereo panning and Jitter modulation
- 8-step scan position sequencer with MIDI clock sync
- 3-page FSM control (Source / Granular / Motion) via encoder
- Soft takeover on page change (no parameter jumps)
- ReverbSc stereo output stage
- MIDI note triggers inject new pitched tones into buffer

## Controls

### Encoder Modes

| Mode | LED Color | Knob 1 | Knob 2 |
|------|-----------|--------|--------|
| Source | Cyan | Timbre Morph | Source Decay |
| Granular | Magenta | Texture (Jitter) | Scan Position |
| Motion | Yellow | Seq Depth | Reverb Mix |

### Buttons

| Button | Function |
|--------|----------|
| Button 1 | FREEZE toggle (Red=frozen, Green=recording) |
| Button 2 | Sequencer start/stop (LED2 pulses on beat) |

### Signal Chain

```
BlOsc + Noise --> [FreezeBuffer] --> TextureVoice (4-tap) --> ReverbSc --> Out L/R
                       ^                    ^
                    Freeze Btn        Seq + Scan Knob
```

## Build

```bash
make clean && make
make program        # ST-Link (default)
make program-dfu    # DFU (alternative)
```

## Dependencies

- libDaisy
- DaisySP (LGPL modules enabled for ReverbSc)
