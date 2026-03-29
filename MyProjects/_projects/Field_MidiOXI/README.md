# Field MidiOXI

Minimal diagnostic polysynth for Daisy Field driven by OXI ONE over hardware MIDI.

## Purpose

This project is intentionally small. It exists to verify:

- Daisy Field hardware MIDI reception from OXI ONE
- polyphonic note handling for `MONO`, `POLY`, and `CHORD` OXI modes
- basic channel filtering
- voice allocation, steals, and dropped notes
- receipt of CC, pitch bend, program change, and system realtime messages

It is not a finished instrument and does not attempt transport sync or multitimbral routing.

## Controls

- `K1`: filter cutoff
- `K2`: filter resonance
- `K3`: attack
- `K4`: decay
- `K5`: sustain
- `K6`: release
- `K7`: pitch bend range
- `K8`: master volume
- `A1-A4`: waveform select (`sin`, `tri`, `saw`, `square`)
- `A5`: parameter page
- `A6`: diagnostics page
- `A7`: reset counters
- `A8`: panic / all notes off
- `SW1`: panic / all notes off
- `SW2`: cycle OLED page

## OXI setup

- Connect OXI MIDI out to Daisy Field MIDI in
- Use a single MIDI channel first
- Start with `MONO`, then `POLY`, then `CHORD`
- Transport/clock may be enabled for observation, but the synth does not sync to them yet
