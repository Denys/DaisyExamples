# Controls - Field_MI_Plaits

## What this document is

This file has two jobs:

1. Preserve a practical control reference for the original Mutable Instruments Plaits module, based on the bundled quickstart PDF and the official Mutable Instruments documentation.
2. Explain how the current `Field_MI_Plaits` Daisy Field firmware behaves today.

These are not the same thing yet.

`Field_MI_Plaits` is currently a Daisy Field mono synth project with Plaits-inspired naming and documentation. It is not yet a feature-equivalent Plaits port with 16 synthesis models, CV routing, AUX behavior, or the original hidden settings.

## Original Plaits at a glance

Plaits is a digital macro-oscillator with 16 synthesis models, split into 2 banks of 8:

- Bank 1 focuses on pitched and tonal synthesis models.
- Bank 2 focuses on noisy, physical, and percussion-oriented models.

The original module centers around five main performance ideas:

- Choose a synthesis model.
- Set base pitch with the `FREQUENCY` knob plus `V/OCT`.
- Shape the sound with `HARMONICS`, `TIMBRE`, and `MORPH`.
- Modulate those parameters with CV and attenuverters.
- Use the internal low-pass gate and trigger input to make the module behave like a self-contained voice.

## Original Plaits key data

| Item | Value |
|------|-------|
| Module type | Macro-oscillator |
| Width | 12HP |
| Depth | 25mm |
| +12V current | 50mA |
| -12V current | 5mA |
| Lifetime | March 2018 to November 2022 |
| Processor | STM32F373CCT6 @ 72 MHz |
| DAC | PCM5100A |

## Original front-panel controls

The quickstart and online manual describe the control surface like this:

| Label | Original Plaits control | What it does |
|------|--------------------------|--------------|
| A | Model buttons + model LEDs | Select one of the 16 synthesis models. Each button cycles through a bank of 8 models. |
| B | `FREQUENCY` | Sets the coarse pitch. Default range is wide, but it can be narrowed in settings. |
| C | `HARMONICS` | Usually sets overtone balance, detuning, spread, chord type, or another model-specific structural control. |
| D | `TIMBRE` | Usually moves the sound from darker/sparser to brighter/denser. |
| E | `MORPH` | Usually explores an alternate axis of tone, shape, response, or decay. |
| F | CV attenuverters for `TIMBRE`, `FM`, and `MORPH` | Scale incoming CV, or, when the related CV jack is unpatched and `TRIG` is patched, set the amount of internal envelope modulation. |

## Original inputs and outputs

| Jack | Original Plaits behavior |
|------|---------------------------|
| `MODEL` CV | Modulates model selection. A steady LED shows the active model; a blinking LED shows the center model value for `0V`. |
| `TIMBRE` / `FM` / `MORPH` / `HARMONICS` CV | Modulate the four main synthesis controls. |
| `TRIG` | Triggers the internal envelope, excites physical/percussive models, strikes the internal LPG, and samples the `MODEL` CV value. |
| `LEVEL` | Opens the internal LPG and acts like amplitude plus brightness control; also works as an accent input for struck models. |
| `V/OCT` | Tracks pitch over a wide range relative to the `FREQUENCY` knob. |
| `OUT` | Main output. |
| `AUX` | Companion output carrying a variant, by-product, or alternate rendering of the active model. |

## Original hidden settings

The quickstart calls out two important hold-button functions:

### Hold the first model button

- Turn `TIMBRE` to move the internal LPG response from more filter-like toward more VCA-like.
- Turn `MORPH` to set LPG ringing time and the internal envelope decay time.
- The yellow LEDs show the stored setting values.

### Hold the second model button

- Turn `HARMONICS` to set the `FREQUENCY` knob range.
- The first 8 settings give fixed root ranges such as `C0 +/- 7 semitones`, `C1 +/- 7 semitones`, and so on.
- The last setting restores the full `C0` to `C8` style wide range.

One subtle but important behavior from the manual: after changing a hidden setting, the physical knob position may no longer match the stored parameter value. The firmware compensates until the knob and parameter line up again.

## Original synthesis model reference

The exact meaning of `HARMONICS`, `TIMBRE`, `MORPH`, and `AUX` changes by model. Use this table as the quick operator reference.

| Model | `HARMONICS` | `TIMBRE` | `MORPH` | `AUX` |
|------|--------------|----------|---------|-------|
| Pair of classic waveforms | Detuning between the two oscillators | Variable square shape, from pulse to square to sync formants | Variable saw shape, from triangle to saw to wide-notch saw | Sum of two hard-synced waves |
| Waveshaping oscillator | Waveshaper waveform | Wavefolder amount | Waveform asymmetry | Alternate wavefolder flavor |
| Two-operator FM | Frequency ratio | FM index | Feedback and cross-feedback character | Sub-oscillator |
| Granular formant oscillator | Ratio between formants | Formant frequency | Formant width and shape | Filtered-waveform style variant |
| Harmonic oscillator | Number of spectral bumps | Index of dominant harmonic | Bump width and sharpness | Organ-like harmonic subset |
| Wavetable oscillator | Wavetable bank selection | Row index | Column index | Low-fi variant |
| Chords | Chord type | Inversion and transposition | Waveform family and wavetable scan | Root note output |
| Vowel and speech synthesis | Blend between speech engines and word banks | Voice/species shift | Phoneme or segment selection | Raw vocal-cord source |
| Granular cloud | Pitch randomization | Grain density | Grain duration and overlap | Sine-grain variant |
| Filtered noise | Filter mode from LP to BP to HP | Clock frequency | Resonance | Dual-band variant |
| Particle noise | Frequency randomization | Particle density | Filter-network character | Raw dust noise |
| Inharmonic string modeling | Inharmonicity / material | Excitation brightness and dust density | Decay time | Raw exciter |
| Modal resonator bank | Material / resonator spread | Excitation brightness and dust density | Decay time | Raw exciter |
| Analog bass drum model | Attack sharpness | Brightness | Decay time | Alternate bass-drum circuit |
| Analog snare drum model | Harmonic/noise balance | Mode balance | Decay time | Alternate snare circuit |
| Analog high-hat model | Metallic/noise balance | High-pass cutoff | Decay time | Alternate tuned-noise flavor |

For the physical and percussion-style models, the manual notes that these models use their own decay envelope and filter behavior, and the standard internal LPG is disabled there.

## Current Field_MI_Plaits firmware

## Important reality check

The current Daisy Field project does not yet implement the original Plaits synthesis engine, model buttons, CV architecture, or AUX output logic.

Today, `Field_MI_Plaits` is:

- A mono synth voice built from `Oscillator`, `Adsr`, `Svf`, and soft drive.
- Primarily played from external TRS MIDI.
- Controlled from the Daisy Field knobs, keybed shortcuts, switches, and OLED.

That means the original Plaits reference above is best read as the target device behavior and naming context, not as a literal description of the current firmware.

## Current Daisy Field control map

| Daisy control | Current behavior |
|------|-------------------|
| `K1` | Filter cutoff |
| `K2` | Filter resonance |
| `K3` | Envelope attack |
| `K4` | Envelope decay |
| `K5` | Envelope sustain |
| `K6` | Envelope release |
| `K7` | Drive amount |
| `K8` | Output level |

### Current keybed and switch shortcuts

| Control | Current behavior |
|------|-------------------|
| `A1` | Select `Sine` waveform |
| `A2` | Select `Triangle` waveform |
| `A3` | Select `Saw` waveform |
| `A4` | Select `Square` waveform |
| `A5-A8` | Reserved, no action in v1 |
| `B1-B8` | Reserved, no action in v1 |
| `SW1` | Panic / all-notes-off, and clears sustain state |
| `SW2` | Clears the temporary OLED zoom/focus state |

Only one waveform shortcut is active at a time. The corresponding A-row LED is lit one-hot to show the selected waveform.

## Current MIDI behavior

`Field_MI_Plaits` is designed around external TRS MIDI note input:

- `NoteOn` starts the mono voice.
- `NoteOff` releases the currently held note.
- `CC64` acts as sustain pedal.
- MIDI activity is shown on the OLED as `Midi:RX` for a short time after incoming data.

Unlike original Plaits, pitch is not currently driven by a coarse `FREQUENCY` knob plus `V/OCT` input. In this build, pitch comes from incoming MIDI notes.

## Current OLED behavior

The OLED has two display states:

- Overview mode shows waveform, note, filter summary, envelope summary, sustain-pedal status, MIDI status, and shortcut hints.
- Zoom mode appears after a knob move and temporarily enlarges the active parameter value before returning to the overview.

The zoom display currently formats values like this:

- `K1`: Hertz
- `K2`: percent
- `K3`: milliseconds
- `K4`: milliseconds
- `K5`: percent
- `K6`: milliseconds
- `K7`: percent
- `K8`: percent

## Current audio behavior

The present firmware uses a straightforward mono-synth signal path:

1. External MIDI sets oscillator pitch and gate.
2. The selected oscillator waveform is generated.
3. An ADSR envelope shapes the voice.
4. A state-variable low-pass filter applies cutoff and resonance.
5. A soft `tanh` drive stage adds saturation.
6. Output level sets final loudness.

Cutoff, resonance, drive, and output level are smoothed in the audio callback to reduce zipper noise during knob moves.

## How to play the current firmware

1. Connect a TRS MIDI source to the Daisy Field MIDI input.
2. Play notes from the external controller.
3. Use `A1-A4` to choose the core oscillator waveform.
4. Use `K1-K8` to shape the subtractive synth voice.
5. Use `SW1` if a stuck note or sustain condition needs to be cleared immediately.
6. Use `SW2` if you want to dismiss the knob zoom state and return to the overview immediately.

## Practical translation: original Plaits vs current Field build

| Original Plaits concept | Current Field_MI_Plaits equivalent |
|------|-------------------------------------|
| Model selection | `A1-A4` waveform selection only |
| `FREQUENCY` knob | External MIDI note input |
| `HARMONICS`, `TIMBRE`, `MORPH` | Replaced by filter + ADSR + drive controls |
| Internal LPG behavior | Replaced by ADSR + low-pass filter path |
| `TRIG` input behavior | Not implemented in the current Field build |
| `LEVEL` CV | Not implemented in the current Field build |
| `AUX` output | Not implemented in the current Field build |
| CV attenuverters | Not implemented in the current Field build |

## Suggested next-port alignment targets

If this project is pushed closer to the original Plaits device, the most important future control milestones would be:

1. Replace waveform-only selection with the original 16-model architecture.
2. Restore true `HARMONICS`, `TIMBRE`, and `MORPH` behavior per model.
3. Add a Plaits-style trigger and internal envelope/LPG workflow.
4. Add a meaningful `AUX` rendering path.
5. Decide how Daisy Field controls should expose original Plaits hidden settings without losing live-play usability.

## Sources

- Local quickstart PDF: `DaisyExamples/MyProjects/_projects/PlaitsPatchInit/plaits_quickstart.pdf`
- Mutable Instruments Plaits documentation index and key data: <https://pichenettes.github.io/mutable-instruments-documentation/modules/plaits/>
- Mutable Instruments Plaits manual: <https://pichenettes.github.io/mutable-instruments-documentation/modules/plaits/manual/>
