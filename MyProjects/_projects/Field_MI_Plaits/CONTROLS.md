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

This build now uses a banked control layer with pickup/catch semantics. The intent is to keep the physical knob positions independent from the stored parameter values so that switching pages does not jump the sound.

## Current Daisy Field control map

### Main bank

| Daisy control | Current behavior |
|------|-------------------|
| `K1` | Frequency / pitch center |
| `K2` | Harmonics |
| `K3` | Timbre |
| `K4` | Morph |
| `K5` | FM amount |
| `K6` | Timbre modulation amount |
| `K7` | Morph modulation amount |
| `K8` | Level / LPG-style voice level behavior |

### Alt bank

Hold `SW1` to edit the alt bank.

| Daisy control | Current behavior |
|------|-------------------|
| `SW1 + K1` | LPG colour |
| `SW1 + K2` | Decay |
| `SW1 + K3-K8` | Reserved and inactive on this build |

The main and alt banks are stored independently. Releasing `SW1` does not copy the alt values back into main.
`SW1 + K3-K8` do not capture, edit, light, or enter zoom.

### Keybed and switch shortcuts

| Control | Current behavior |
|------|-------------------|
| `A1-A8` | Select mapped engine slots when `SW2` is not held |
| `B1-B8` | Select mapped engine slots when `SW2` is not held |
| `SW1` tap | Panic / all-notes-off, clears sustain state |
| `SW1` hold | Activates the alt bank |
| `SW2` tap | Clears the temporary OLED zoom state |
| `SW2` hold | Opens the range page and repurposes the keybed for range selection |

Only the mapped engine slots are active. Unmapped slots intentionally do nothing.

## Current MIDI behavior

`Field_MI_Plaits` is designed around external TRS MIDI note input:

- `NoteOn` starts the mono voice.
- `NoteOff` releases the currently held note.
- `CC64` acts as sustain pedal.
- MIDI activity is shown on the OLED as `Midi:RX` for a short time after incoming data.

Pitch comes from incoming MIDI notes, not from a local `FREQUENCY` knob plus `V/OCT` input.

## Current OLED behavior

The OLED now reflects the banked parameter workflow:

- Overview mode shows the current engine slot, note state, range, parameter summary, MIDI/gate status, and active bank.
- Zoom mode appears after a knob move and enlarges the stored value for the active parameter.
- Alt bank mode appears while `SW1` is held and shows the stored alt values.
- Range mode appears while `SW2` is held and shows the selected pitch range.

The zoom display uses the stored logical value from the active bank. It does not mirror the raw physical knob position. Reserved alt slots do not enter zoom.

## Current audio behavior

The present firmware still uses a straightforward voice path:

1. External MIDI sets oscillator pitch and gate.
2. The selected engine slot is rendered.
3. The stored main-bank parameters are translated into the active Plaits patch/modulation set.
4. Alt-bank values feed the LPG colour and decay controls.
5. Output level is smoothed separately so voice level changes stay stable.

## Practical translation: original Plaits vs current Field build

| Original Plaits concept | Current Field_MI_Plaits equivalent |
|------|-------------------------------------|
| Model selection | Keybed engine-slot selection |
| `FREQUENCY` knob | Main-bank `K1` frequency/pitch center |
| `HARMONICS`, `TIMBRE`, `MORPH` | Main-bank `K2-K4` |
| `FM`, `TIMBRE`, `MORPH` attenuverters | Main-bank `K5-K7` |
| `LEVEL` | Main-bank `K8` as voice-level / LPG behavior |
| Hidden LPG controls | Alt-bank `K1-K2` |
| Hidden range control | `SW2` hold keybed range page |
| `TRIG` input behavior | External MIDI note-on |
| `AUX` output | Not exposed as a separate user control in this build |

## Startup defaults

| Item | Default |
|---|---|
| Main bank | `K1=50%`, `K2=40%`, `K3=50%`, `K4=50%`, `K5=50%`, `K6=50%`, `K7=50%`, `K8=85%` |
| Alt bank | `K1=50%`, `K2=50%`, `K3-K8=50% reserved placeholders` |
| Engine slot | First mapped slot |
| Range | Full range |
| Zoom | Off |
| Sustain | Off |
| Active bank | Main |

## Suggested next-port alignment targets

If this project is pushed closer to the original Plaits device, the next control milestones would be:

1. Restore the original 16-model architecture.
2. Add a more literal Plaits-style model selection workflow.
3. Expand the alt-bank concept into the full hidden-settings surface.
4. Decide how much of the original CV/trigger/LPG behavior should remain exposed on Daisy Field.

## Sources

- Local quickstart PDF: `DaisyExamples/MyProjects/_projects/PlaitsPatchInit/plaits_quickstart.pdf`
- Mutable Instruments Plaits documentation index and key data: <https://pichenettes.github.io/mutable-instruments-documentation/modules/plaits/>
- Mutable Instruments Plaits manual: <https://pichenettes.github.io/mutable-instruments-documentation/modules/plaits/manual/>
