# Seed MIDI Grainlet

Minimal Daisy Seed proof of concept for controlling a `GrainletOscillator` from an external Teensy over UART MIDI.

## What Grainlet Synthesis Is

`GrainletOscillator` is not sample-based granular playback. In DaisySP it is a compact oscillator model built from two interacting sine-based elements:

- a main carrier oscillator that sets the played note
- a second sine component that acts like a moving formant or resonance

Internally, the carrier is wave-shaped, then multiplied by the formant component, and the whole structure is kept synchronized to the main pitch. The result sits between a simple oscillator and a vocal, grainy, resonant tone generator. It can move from relatively pure and rounded to buzzy, nasal, hollow, or bright depending on the parameter settings.

## What The Parameters Do

- `frequency`: This is the fundamental pitch of the note. Raising it makes the whole sound play higher, like any normal oscillator. It does not mainly change the character of the waveform; it changes where the note sits musically.
- `formant frequency`: This controls the frequency of the internal resonant sine component. Perceptually, this acts like moving a spectral emphasis or vowel-like peak up and down. Lower settings feel darker, throatier, and more hollow. Higher settings feel brighter, tighter, and more nasal or metallic.
- `shape`: This changes the carrier wave-shaping before it is combined with the formant. In practice this is the main waveform-morph control. Lower values sound smoother and more sine-like. Mid values introduce more asymmetry, edge, and grain. Higher values produce sharper, more aggressive contour changes that increase buzz, bite, and a more complex harmonic profile.
- `bleed`: This controls how much of the formant component leaks directly into the final output. Lower values keep the sound more dominated by the shaped carrier, which feels cleaner and less colored. Higher values make the formant more obvious, which usually increases body, resonance, and perceived brightness or vocal character.

## How The Controls Affect Shape And Perception

- If you change `frequency` only, the same timbre moves up and down in pitch.
- If you change `formant frequency` only, the pitch can stay the same while the tone color shifts, similar to changing the mouth shape of a voiced sound.
- If you change `shape`, the waveform contour itself changes more strongly, which is usually heard as a shift in harmonic density, roughness, and attack character.
- If you change `bleed`, you hear more or less of the resonant layer sitting on top of the carrier, which changes how forward, hollow, airy, or vowel-like the sound feels.

## Listening Tips

- Start with low `shape` and low `bleed` to hear the simplest version of the oscillator.
- Sweep `formant frequency` while holding one note to hear the internal resonance move independently of pitch.
- Increase `shape` gradually to hear the transition from smooth to grainy and more harmonically rich.
- Raise `bleed` after that to emphasize the resonant character and make the tone feel more pronounced in the upper partials.

## Scope

- Monophonic Grainlet voice
- MIDI note on/off for pitch and gate
- Three MIDI CC controls for timbre
- Stereo output with the same signal on both channels

## Wiring

- Teensy `TX1` -> Daisy Seed `D14` (`USART1_RX`)
- Teensy `GND` -> Daisy `GND`

This project assumes a direct 3.3V UART MIDI link between the two boards.

## MIDI Map

- `Note On`: set Grainlet pitch and trigger envelope
- `Note Off`: release envelope
- `CC 14`: `shape`
- `CC 15`: `formant frequency`
- `CC 16`: `bleed`

## Defaults

- Shape: `0.35`
- Formant frequency: `1200 Hz`
- Bleed: `0.25`

## Build

From this directory:

```sh
make
```

## Teensy Companion

A matching `Teensy 4.0` controller sketch lives at [Seed_MIDI_Grainlet_Teensy40.ino](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\teensy_controller\Seed_MIDI_Grainlet_Teensy40\Seed_MIDI_Grainlet_Teensy40.ino).

Detailed setup and programming instructions live in [TEENSY_4_SETUP_AND_PROGRAM.md](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\TEENSY_4_SETUP_AND_PROGRAM.md).

Suggested Teensy wiring:

- `A0` pot -> `CC 14` -> shape
- `A1` pot -> `CC 15` -> formant frequency
- `A2` pot -> `CC 16` -> bleed
- `Pin 2` pushbutton to ground -> fixed `Note On/Off`
- `Serial1 TX` -> Daisy `D14`
- `GND` -> Daisy `GND`

The sketch targets `Teensy 4.0`, sends raw UART MIDI at `31250` baud with no extra Arduino libraries, and now exposes USB serial debug output at `115200` baud.
