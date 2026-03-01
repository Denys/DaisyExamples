# DVPE Module Catalog

**Total Modules:** 162

This catalog lists all available modules in the Daisy Visual Programming Environment (DVPE), categorized by function. Use the **Typically Used With** column to find compatible modules for building patches.

## 1. DYNAMICS

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **COMPRESSOR** | Dynamic range compression | IN, SIDECHAIN | OUT | Thresh, Ratio, Attack, Release, Makeup | Kick, Snare, Master Bus |
| **COMPRESSOR (EXPANDER)** | Compressor with expansion capability | IN, SIDECHAIN | OUT | Thresh, Ratio, Attack, Release, Makeup | Drums, Vocals |
| **LIMITER** | Peak limiting to prevent clipping | IN | OUT | Pre-Gain, Ceiling | Master Out, High-Resonance Filters |
| **NOISE GATE** | Mutes audio below a threshold | IN, KEY | OUT | Threshold, Attack, Release | Guitar Input, Noisy Synth |
| **ENV FOLLOWER** | Generates CV envelope from audio amplitude | IN | ENV | Attack, Release | Filter Cutoff, Compressor Sidechain, LED |
| **LINEAR VCA** | Voltage Controlled Amplifier (Linear) | IN, CV | OUT | Level | ADSR, LFO |
| **VCA** | Voltage Controlled Amplifier (Exp/Lin) | IN, CV | OUT | Level, Curve | ADSR, LFO, Velocity |

## 2. EFFECTS

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **AUTOWAH** | Automatic wah filter effect | IN, CV | OUT | Sens, Res, Dry/Wet | Guitar, Funky Lead |
| **BITCRUSH** | Reduces sample rate and bit depth | IN, CV | OUT | Depth, Rate | Drums, Glitch Synth |
| **CHORUS** | Modulated delay for thickening sound | IN | OUT (Stereo) | Rate, Depth, Delay, Feedback, Mix | Pads, Strings, Guitar |
| **DECIMATOR** | Sample rate reduction effect | IN | OUT | Downsample Factor | Lo-Fi textures, Industrial Synth |
| **DELAY** | Digital delay line | IN, TIME CV | OUT | Delay Time, Feedback, Mix | Plucks, Vocals, Lead Synth |
| **DISTORTION** | Hard clipping distortion | IN | OUT | Drive, Mix | Bass, Lead, Drums |
| **FLANGER** | Modulated comb filter effect | IN | OUT (Stereo) | Rate, Depth, Feedback, Delay, Mix | Pads, Jet Plane FX |
| **FOLD** | Wavefolding distortion | IN | OUT | Increment, Threshold | Sine Oscillator, West Coast Synth |
| **OVERDRIVE** | Soft saturation/drive | IN | OUT | Drive | Bass, Warmth |
| **PHASER** | Phase shifting effect | IN | OUT (Stereo) | Rate, Depth, Feedback, Freq | Pads, Electric Piano |
| **PITCH SHIFTER** | Real-time pitch shifting | IN | OUT | Shift (Semitones) | Vocals, Harmony |
| **REVERB** | Stereo Reverb (Sc) | IN L, IN R | OUT L, OUT R | Feedback, LpFreq, Mix | Master Out, Ambient Pads |
| **ROBOTIZE** | Robot voice effect (Granular) | IN | OUT | - | Vocals, Drums |
| **TIME STRETCH** | Time stretching effect | IN | OUT | Stretch Factor | Samples, Textures |
| **TREMOLO** | Amplitude modulation effect | IN | OUT | Rate, Depth, Shape | Electric Piano, Organ |
| **VIBRATO** | Pitch modulation effect | IN | OUT | Rate, Depth | Lead Synth, Violin |
| **WAHWAH** | Spectral glide effect | IN, PEDAL | OUT | Freq, Drive, Res | Expression Pedal, Envelope Follower |
| **WHISPERIZE** | Noise-like texture effect | IN | OUT | - | Vocals, textural FX |

## 3. FILTERS

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ALLPASS** | All-pass filter | IN | OUT | Freq | Phasers, Reverbs |
| **ATONE** | High-pass filter (Tone Stack) | IN | OUT | Freq | Equalization |
| **BIQUAD** | General purpose biquad filter | IN | OUT | Freq, Res | EQ, General Filtering |
| **COMB** | Comb filter (IIR) | IN | OUT | Freq, Rev Time | Physical Modeling, Flanger |
| **HIGH SHELF** | High shelving EQ | IN | OUT | Freq, Gain | Mastering, EQ |
| **LOW SHELF** | Low shelving EQ | IN | OUT | Freq, Gain | Bass Boost, EQ |
| **MOOG LADDER** | 24dB/oct Low-pass filter emulation | IN, CUTOFF | OUT | Cutoff, Res, Drive | Sawtooth Wave, Bass Lines |
| **ONE POLE** | 6dB/oct Low-pass/High-pass | IN | OUT | Freq | Smoothing Control Signals |
| **PEAK EQ** | Peaking/Bell EQ filter | IN | OUT | Freq, Gain, Q | Parametric EQ |
| **SVF** | State Variable Filter (Multi-mode) | IN, CUTOFF | HP, LP, BP, NOTCH | Cutoff, Res, Drive | Polyphonic Synths, General Use |
| **TONE** | Low-pass filter (Tone Stack) | IN | OUT | Freq | Simple Tone Control |

## 4. IO (INPUT/OUTPUT)

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **AUDIO IN** | Hardware Audio Input | - | OUT | Channel Select | External Audio Processing |
| **AUDIO OUT** | Hardware Audio Output | IN | - | Channel Select | Master Output |
| **CV IN** | Hardware CV/Knob Input | - | CV | Channel Select | Parameter Modulation |
| **CV OUT** | Hardware CV/DAC Output | CV | - | Channel Select | External Gear Control |
| **GATE IN** | Hardware Gate/Trigger Input | - | GATE | Channel Select | Clock In, Trigger In |
| **GATE OUT** | Hardware Gate/Trigger Output | GATE | - | Channel Select | Clock Out, Envelope Trigger |
| **LEDS** | Hardware LED Output | BRIGHTNESS | - | LED Select | Visual Feedback |
| **MIDI NOTE** | MIDI Note Input Handler | - | PITCH, VEL, GATE | Channel | Oscillator, ADSR |
| **MIDI CC** | MIDI Control Change Input | - | CV | CC Number, Channel | Filter Cutoff, Effects Mix |

## 5. LOGIC

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **AND** | Logical AND | A, B | OUT | - | Gate Logic, Seq Reset |
| **OR** | Logical OR | A, B | OUT | - | Combining Triggers |
| **NOT** | Logical NOT (Inverter) | IN | OUT | - | Inverting Gates |
| **XOR** | Logical XOR | A, B | OUT | - | Complex Rhythms |
| **EQUALS** | Outputs 1 if A == B | A, B | OUT | Tolerance | Thresholds, Comparators |
| **NOT EQUALS** | Outputs 1 if A != B | A, B | OUT | - | Change Detection |
| **GREATER** | Outputs 1 if A > B | A, B | OUT | - | Thresholds, Sidechain |
| **LESS** | Outputs 1 if A < B | A, B | OUT | - | Thresholds |
| **EDGE FALL** | Trigger on falling edge | IN | TRIG | - | Clock Division, Release Trig |
| **EDGE RISE** | Trigger on rising edge | IN | TRIG | - | Clock Multiplier, Attack Trig |
| **SCHMITT** | Schmitt Trigger with hysteresis | IN | OUT | High, Low | Cleaning Noisy Gates |
| **TOGGLE** | Toggles state on trigger | TRIG | STATE | - | Start/Stop, Mode Switch |

## 6. MATH

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ABS** | Absolute value | IN | OUT | - | Rectification, Distortion |
| **ADD** | Addition (Mix) | A, B | OUT | - | Mixing CVs, Transposition |
| **ATAN2** | Arc Tangent 2 | Y, X | OUT | - | Phase Calculation |
| **CEIL** | Ceiling function | IN | OUT | - | Quantization |
| **CLAMP** | Constrains signal to range | IN | OUT | Min, Max | Limiting CV ranges |
| **COS** | Cosine function | PHASE | OUT | - | LFOs, Stereo Panning |
| **DIVIDE** | Division | A, B | OUT | - | Scaling, Ratio |
| **EXP** | Exponential function | IN | OUT | - | Lin->Exp Conversion |
| **FLOOR** | Floor function | IN | OUT | - | Quantization, Sample & Hold |
| **LOG** | Natural Logarithm | IN | OUT | - | Exp->Lin Conversion |
| **MAX** | Maximum valid signal | A, B | OUT | - | Rectification, Logic |
| **MIN** | Minimum valid signal | A, B | OUT | - | Sidechaining, Logic |
| **MODULO** | Modulo operation (Rem) | A, B | OUT | - | Looping Ramps, Phase Wrap |
| **MULTIPLY** | Multiplication (VCA/Ring Mod) | A, B | OUT | - | VCA, Ring Mod, Scaling |
| **NEGATE** | Inverts signal sign | IN | OUT | - | Inverting CV, Phase Invert |
| **POW** | Power function (x^y) | BASE, EXP | OUT | - | Waveshaping, Scaling |
| **RECIPROCAL** | 1 / x | IN | OUT | - | Frequency <-> Period |
| **SCALE** | Scale and Offset | IN | OUT | Scale, Offset | Mapping 0-1 to Freq |
| **SIN** | Sine function | PHASE | OUT | - | Oscillators, LFOs |
| **SQRT** | Square Root | IN | OUT | - | RMS Calculation |
| **SUBTRACT** | Subtraction | A, B | OUT | - | Inverting mix, Calculating Diff |
| **TAN** | Tangent function | PHASE | OUT | - | Distortion, Waveshaping |
| **HZ -> MIDI** | Frequency to MIDI Note | FREQ | NOTE | - | Analysis, Tuning |
| **MIDI -> HZ** | MIDI Note to Frequency | NOTE | FREQ | - | Oscillator Pitch |

## 7. OSCILLATORS

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **BL OSC** | Band-Limited Oscillator | FREQ, FM | OUT | Wave, Amp, Freq | Filter, VCA |
| **CLOCK NOISE** | Clocked Noise Generator | FREQ | OUT | - | Snare, Hi-Hats, S&H |
| **DRIP** | Water drip physical model | TRIG | OUT | - | Percussion |
| **FORMANT** | Formant Oscillator | FREQ | OUT | Vowel, Formant Freq | Speech Synthesis |
| **GRAINLET** | Grainlet Oscillator | FREQ | OUT | - | Glitch, Texture |
| **HARMONIC** | Additive Harmonic Oscillator | FREQ | OUT | Harmonics | Organs, Bell Tones |
| **HIHAT** | Analog Hi-Hat Model | TRIG, DECAY | OUT | Tone | Drum Machine |
| **KICK (ANALOG)**| Analog Kick Drum Model | TRIG | OUT | Decay, Tone | Drum Machine |
| **KICK (SYNTH)** | Synth Kick Drum Model | TRIG | OUT | Freq, Decay | Drum Machine |
| **LFO** | Low Frequency Oscillator | RESET | TRI, SAW, SQR | Freq, Amp | Filter Cutoff, PWM, VCA |
| **MODAL** | Modal Resonator | TRIG, EXCITE | OUT | Structure, Brightness | Physical Modeling Percussion |
| **OSCILLATOR** | Standard Oscillator (analog emu) | V/OCT, FM | OUT | Wave, Amp, Freq | Filter, VCA, ADSR |
| **OSC BANK** | Bank of 7 Oscillators | V/OCT | OUT | Spread, Wave | Supersaw, Drones |
| **PARTICLE** | Particle Noise Generator | - | OUT | - | Textures, FX |
| **PHASOR** | Ramp generator (0 to 1) | FREQ | OUT | - | Wavetable Read, Sequencers |
| **PLUCK** | Karplus-Strong String | TRIG | OUT | Freq, Decay, Color | Plucked Strings |
| **RESONATOR** | Resonant structure model | IN, TRIG | OUT | Freq, Structure | Physical Modeling |
| **SNARE (ANALOG)**| Analog Snare Drum | TRIG | OUT | Tone, Snappy | Drum Machine |
| **SNARE (SYNTH)**| Synth Snare Drum | TRIG | OUT | Freq, Decay, Noise | Drum Machine |
| **STRING** | String physical model | TRIG | OUT | Freq, Damping | Guitar, Violin |
| **VARISAW** | Variable Slope Sawtooth | FREQ, WIDTH | OUT | - | PWM-like Sawtooth |
| **VARISHAPE** | Variable Shape Oscillator | FREQ, SHAPE | OUT | - | Complex Timbres |
| **VOSIM** | Vocal Simulation Oscillator | FREQ | OUT | Formant 1/2 | Speech, Robotic Sounds |
| **WAVETABLE** | Wavetable Oscillator | FREQ, POS | OUT | Table Index | Morphing Leads, Pads |
| **WHITE NOISE** | White Noise Generator | - | OUT | - | Snare, Wind, S&H Source |
| **Z OSC** | Through-Zero FM Oscillator | FREQ, FM | OUT | Formant, Shape | FM Synthesis |

## 8. SEQUENCING & CONTROL

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ADSR** | Attack-Decay-Sustain-Release Env | GATE | ENV | A, D, S, R | VCA, Filter Cutoff |
| **ARPEGGIATOR** | MIDI Arpeggiator | CLOCK, NOTE | PITCH, GATE | Mode, Div | Oscillator, Synth Voice |
| **CLOCK** | BPM Clock Generator | BPM CV | TRIG | BPM | Sequencer, Arpeggiator |
| **METRO** | Metronome Trigger | FREQ CV | TRIG | Freq (Hz) | Sequencer, Envelope |
| **RAMP** | Linear Ramp Generator | TRIG | OUT | Time | Modulation, Envelopes |
| **SEQUENCER** | Step Sequencer (8/16 step) | CLOCK, RESET | CV, GATE | Steps | Oscillator, Drum Voice |

## 9. UTILITY

| Module Name | Description | Inputs | Outputs | Controls | Typically Used With |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **CROSSFADE** | Crossfade between two signals | A, B, MIX | OUT | Curve | Dry/Wet Mix, Morphing |
| **DEMUX** | Demultiplexer (1 to N) | IN, SEL | OUT 1-4 | - | Signal Routing |
| **GAIN** | Signal Gain / Attenuation | IN, CV | OUT | Level | Pre-Amp, Matching Levels |
| **GATE LENGTH** | Sets fixed gate duration | TRIG | GATE | Length | Drum Triggers |
| **MERGER** | Merges Mono to Stereo/Multi | IN 1, IN 2 | OUT | - | Stereo FX, Polyphony |
| **MIXER** | Multi-channel Mixer | IN 1-4 | OUT | Levels | Oscillators, FX Return |
| **MUX** | Multiplexer (N to 1) | IN 1-4, SEL | OUT | - | Signal Switching |
| **OFFSET** | Adds DC Offset | IN | OUT | Amount | Unipolar <-> Bipolar |
| **PAN** | Stereo Panning | IN, PAN | L, R | Pan Pos | Stereo Mix |
| **QUANTIZER** | Pitch Quantizer | IN | OUT | Scale | Random Melodies, Sequencer |
| **SAMPLE & HOLD**| Samples input on trigger | IN, TRIG | OUT | - | Random Steps, Stepped LFO |
| **SAMPLE DELAY** | Delay by N samples | IN | OUT | Delay | Phase Alignment, Feedback |
| **SELECT** | Selects between A/B | A, B, SEL | OUT | - | A/B Testing, Logic Switch |
| **SLEW** | Slew Limiter (Glide/Portamento) | IN | OUT | Time | Pitch Glide, Smoothing |
| **SMOOTH** | Signal Smoother | IN | OUT | Time | Control smoothing |
| **SPLITTER** | Splits Multi/Stereo to Mono | IN | OUT 1, OUT 2 | - | Stereo Processing |
| **SWITCH** | Signal On/Off Switch | IN, STATE | OUT | - | Muting |
| **YIN PITCH** | Pitch Detection | IN | PITCH, GATE | Range | Audio-to-MIDI, Guitar Synth |
