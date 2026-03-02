# Field + Pod Integration Projects: Extended Scenarios

This document details 15 project concepts where the **Daisy Field** and **Daisy Pod** function as a unified system.
**Core Concept**: An **External MIDI Keyboard** handles note input (pitch/gate), freeing up the Daisy Field's tactile interface (Knobs, Switches, Matrix Keys) for performance macros, modulation, and system control.

---

## 1. The "Texture Weaver" Ambient System
**Concept**: A cinematic soundscape generator. The Field acts as a 4-voice Polyphonic Wavetable Synthesizer, while the Pod functions as a granulator/freezer for the audio output.

**Scenario**: You are scoring a film scene. You hold a chord on the external MIDI keyboard. As the chord sustains, you use the Field's touch keys to slowly morph the timbre (wavetable position) and the Pod's encoder to "freeze" the texture, creating a bed of sound over which you can play new notes.

### Control Mapping
**Daisy Field (Synth Engine)**
*   **Knobs A-D (ADSR)**: Amp Envelope Attack, Decay, Sustain, Release.
*   **Knobs E-H (Timbre)**: Wavetable Position, Cutoff, Resonance, LFO Rate.
*   **Switch 1**: Oscillator Waveform / Bank Select.
*   **Switch 2**: Filter Mode (LP/BP/HP).
*   **Keys A1-A8 (Chord Memory)**: 
    *   Pressing A1-A8 saves the currently held MIDI chord to that slot.
    *   Tapping notes later triggers these stored chords.
*   **Keys B1-B8 (Modulation)**:
    *   `B1-B4`: Toggle "Slow Motion" LFOs to different destinations (Pitch, Filter, Pan).
    *   `B5-B8`: Engage "Infinite Sustain" for Voices 1-4 respectively.

**Daisy Pod (Texture Processor)**
*   **Audio**: Processes Field Stereo Out.
*   **Knob 1**: Grain Size / Density.
*   **Knob 2**: Pitch Shift of Grains (+/- 1 Octave).
*   **Encoder**: Click to **FREEZE** buffer. Turn to scan through frozen buffer.
*   **Leds**: Pulse to show grain density.

---

## 2. "Rhythm & Bass" Split Setup
**Concept**: A complete techno groovebox. The Field is an 8-track Sample-based Drum Machine. The Pod is a dedicated Monophonic Bass Synth (Saw/Square + Ladder Filter).

**Scenario**: You are jamming live. Your left hand plays a bassline on the external MIDI keyboard (routed to Pod). Your right hand operates the Field, muting and unmuting drum parts, adding fills, and tweaking the drum mix.

### Control Mapping
**Daisy Field (Drum Brain)**
*   **Knobs A-H (Mixer)**: Volume levels/Pitch for the 8 active drum tracks.
*   **Switch 1**: Bank Select (808 / 909 / Acoustic).
*   **Switch 2**: Master Compressor On/Off.
*   **Keys A1-A8 (Mutes)**: 
    *   Toggle Mute for Kick, Snare, Closed Hat, Open Hat, Tom L, Tom H, Clap, Cymbal.
*   **Keys B1-B8 (Performance FX)**:
    *   Holding `B1-B8` triggers a "Beat Repeat" or "Stutter" effect for that specific track (e.g., rapid fire hi-hats).

**Daisy Pod (Bass Voice)**
*   **Audio**: Generates Bass Audio (mixed externally or passed through Field aux if custom hardware permits).
*   **Knob 1**: Filter Cutoff.
*   **Knob 2**: Filter Resonance / Envelope Amount.
*   **Encoder**: Selects Bass Waveform (Saw, Square, Triangle).
*   **MIDI In**: Receives notes from External Keyboard (Channel 1).

---

## 3. The "Spectral Resonator" Lead
**Concept**: An expressive lead synthesizer where the Field generates the dry signal (Virtual Analog) and the Pod creates a harmonic, metallic "shadow" using a bank of tuned bandpass filters (Resonator).

**Scenario**: You interpret a melody. The Field provides a classic saw lead. The Pod adds a "ghostly" ringing tone that follows the pitch but can be detuned or frozen to create a drone accompaniment to your solo.

### Control Mapping
**Daisy Field (Lead Synth)**
*   **Knobs A-D**: Oscillator Detune, Sub-Osc Vol, Noise Vol, Glide Time.
*   **Knobs E-H**: Filter settings (Cutoff, Res, Drive, Env Amount).
*   **Keys A1-A8 (Scale Quantizer)**: 
    *   Selects the scale for the MIDI input (Chromatic, Major, Minor, Pentatonic, Blues, Dorian, Phrygian, Lydian). This prevents "wrong" notes during fast runs.
*   **Keys B1-B8 (Harmonics)**:
    *   Adds specific harmonic intervals to the oscillators (Octave up, 5th, Major 3rd, etc.).

**Daisy Pod (Resonator)**
*   **Audio**: Processes Field Audio.
*   **Knob 1**: Decay Time (Damping of resonators).
*   **Knob 2**: "Brightness" / Spectral Tilt.
*   **Encoder**: Tuning Offset (Dissonance vs Consonance).

---

## 4. "Loop Station Pro"
**Concept**: A 4-Track Live Looper. The Pod acts as an input effects processor (getting audio from a Mic or Guitar), feeding into the Field which records and arranges the loops. Use the MIDI keyboard to play a soft synth layer on top.

**Scenario**: You plug a guitar into the Pod. You add some distortion (Pod). You record a 4-bar chord progression into Track 1 (Field). Then you plug a mic, record vocals to Track 2. Finally, you play a synth solo over the top using the MIDI keyboard and Field's internal synth engine.

### Control Mapping
**Daisy Field (Looper & Synth)**
*   **Knobs A-D**: Volume Levels for Loop Tracks 1-4.
*   **Knobs E-H**: Synth Layer Params (Cutoff, Attack, Release, Reverb Send).
*   **Keys A1-A4 (Transport)**: Record/Play/Overdub specific tracks.
*   **Keys A5-A8 (Stop/Clear)**: Stop or Clear specific tracks (Hold to clear).
*   **Keys B1-B4 (Reverse)**: Toggle Reverse playback for tracks.
*   **Keys B5-B8 (Speed)**: Toggle Half-Speed / Double-Speed.

**Daisy Pod (Input FX)**
*   **Knob 1**: Input Gain / Drive.
*   **Knob 2**: Tone / EQ.
*   **Encoder**: FX Select (Clean, Fuzz, Chorus, Delay).

---

## 5. "Glitch Commander"
**Concept**: An IDM / Glitch workstation. Field is an FM Percussion synth. Pod represents a "Master Destruction" bus effect (Bitcrusher, Sample Rate Reducer, Glitch delays).

**Scenario**: You program a clean beat on the Field. Then, using the Field's tactile keys, you "perform" the glitch effects hosted on the Pod (sending MIDI CCs) or internal buffer effects, mangling the beat in real-time.

### Control Mapping
**Daisy Field (FM Drum Brain)**
*   **Knobs A-H**: FM Ratios, Modulation Depths, Decay times for different drum voices.
*   **Keys A1-A8 (Algorithm)**: Selects different FM Algorithms (routing of Operators) for the active voice.
*   **Keys B1-B8 (Glitch Triggers)**: 
    *   `B1`: Stutter 1/4 note.
    *   `B2`: Stutter 1/16 note.
    *   `B3`: Tape Stop.
    *   `B4`: Reverse active buffer.
    *   `B5`: Bitcrush Momentary.
    *   `B6`: Random Pitch warp.
    *   `B7`: Gater / Mute.
    *   `B8`: Reload original patterns (Instant undo).

**Daisy Pod (Glitch Processor)**
*   **Knob 1**: Dry/Wet Mix.
*   **Knob 2**: "Chaos" Amount (Random modulation of effect parameters).
*   **Encoder**: Selects the primary "Destruction Flavor".

---

## 6. The "Dual-Layer" Performance Synth
**Concept**: A Bi-timbral synth. Layer A (Field) is a lush Pad. Layer B (Pod) is a crystalline Arpeggio or Pluck.

**Scenario**: You play a chord progression. The Field plays the full chords (Pad). The MIDI note data is also sent to the Pod, which runs an arpeggiator on those notes, creating a rhythmic top layer. You mix them live.

### Control Mapping
**Daisy Field**
*   **Knobs A-D**: Layer A (Field) Params (Filter, Shape, Chorus).
*   **Knobs E-H**: Layer B (Pod) Macro Controls (sends MIDI CC to Pod).
*   **Keys A1-A4 (Mixer)**: Mute A, Mute B, Solo A, Solo B.
*   **Keys A5-A8 (Pod Arp)**: Select Arp pattern/direction for the Pod (Up, Down, Random, Converge).
*   **Keys B1-B8 (Scenes)**: 
    *   Store "Snapshots" of all Knob positions. A single press transitions the entire sound engine to a new state (e.g., "Dark Intro" -> "Bright Chorus").

**Daisy Pod (Arp Voice)**
*   **Knob 1**: Arp Speed / Division.
*   **Knob 2**: Pluck Decay.
*   **Encoder**: Octave Range (1, 2, 3 octaves).

---

## 7. "Generative Soundscape"
**Concept**: A semi-autonomous music generator. You provide the "seed" (Root note/Scale) via keyboard. The Field generates melodies and textures based on probability. Pod provides a massive reverb.

**Scenario**: You press "C3" on the keyboard. The system begins generating a C Major ambient piece. You use the Field Keys to adjust the "Mood" (probability of high notes, density of events, timbre changes).

### Control Mapping
**Daisy Field (The Brain)**
*   **Knobs A-H**: Setting Probability thresholds (Note Density, Octave Spread, Rhythm Regularity).
*   **Keys A1-A8 (Scale/Mood)**: 
    *   `A1-A8`: Select Scales (Major, Minor, Pentatonic, Hirajoshi, etc.).
*   **Keys B1-B8 (Constraints)**:
    *   `B1`: Freeze Pitch (Hold current note).
    *   `B2`: Freeze Rhythm.
    *   `B3`: Force Octave Up.
    *   `B4`: Force Octave Down.
    *   `B5-B8`: Inject random "Events" (Swell, Rattle, Chirp).

**Daisy Pod (The Space)**
*   **Knob 1**: Reverb Decay (up to infinite).
*   **Knob 2**: Reverb Mix / Modulation.

---

## 8. "Vocal Formant" Synthesizer
**Concept**: A synthesizer mimicking the human vocal tract.
**Scenario**: You play a melody. With your left hand on the Field keys, you "type" the vowels (A-E-I-O-U) that the synth sings, creating a choir or talk-box effect.

### Control Mapping
**Daisy Field (Formant Engine)**
*   **Knobs A-D**: Pitch LFO (Vibrato), Throat Size (Format Shift), Breathiness (Noise), Glide.
*   **Keys A1-A5 (Vowels)**: 
    *   `A1`: Trigger "Ah". `A2`: "Eh". `A3`: "Ee". `A4`: "Oh". `A5`: "Oo".
*   **Keys A6-A8**: Diphthongs / Blends.
*   **Keys B1-B8 (LFO Shapes)**:
    *   Selects modulation shapes for the formant filter (Rhythmic chanting).

**Daisy Pod (Distortion/Cabinet)**
*   **Knob 1**: Drive / Saturation.
*   **Knob 2**: Tone / Presence.
*   **Encoder**: Select Cabinet Model (Small Speaker, Megaphone, Amp).

---

## 9. "Physical Modeling" Duo
**Concept**: Modeling a string instrument. Field models the "Exciter" (Pick, Hammer, Bow). Pod models the "Resonator" (Body, Soundboard).

**Scenario**: You play chords. Tapping Field keys changes *how* the string is struck (e.g., switching from a soft finger-pick to a hard plectrum slap), drastically changing the dynamic response.

### Control Mapping
**Daisy Field (Exciter)**
*   **Knobs A-D**: Pluck Strength, Pick Position (Bridge vs Neck), Wire Stiffness.
*   **Keys A1-A8 (Excitation Types)**: 
    *   `A1`: Finger. `A2`: Pick (Down). `A3`: Pick (Up). `A4`: Hammer. `A5`: Bow (Continuous). `A6`: Scrape.
*   **Keys B1-B8 (Damping)**:
    *   Simulate left-hand techniques (Palm Mute, Fret Noise, Harmonics).

**Daisy Pod (Body)**
*   **Knob 1**: Body Size (Ukulele -> Cello -> Dreadnought).
*   **Knob 2**: Decay / Resonance.

---

## 10. "Dub Techno" Engine
**Concept**: A machine designed for one thing: Basic Channel chords with endless, rhythmic echoes.
**Scenario**: You play a simple minor triad. It sounds dry. You hit a Field Key, and suddenly that specific chord is thrown into the Pod's Tape Delay, creating a swirling rhythm. You release the key, and the next chords are dry again.

### Control Mapping
**Daisy Field (Chord Synth)**
*   **Knobs A-D**: Filter Cutoff, Envelope Mod, Filter Decay, AMP Release.
*   **Keys A1-A8 (Chord Macros)**: 
    *   Force input notes to specific chord inversions (m7, m9, m11, sus4).
*   **Keys B1-B8 (Dub Throws)**:
    *   `B1`: Send to Delay 1/4 (Momentary).
    *   `B2`: Send to Delay 1/8 dotted.
    *   `B3`: Send to Reverb (Explosion).
    *   `B4`: Feedback Loop Overload (Caution!).

**Daisy Pod (Tape Delay)**
*   **Knob 1**: Delay Time.
*   **Knob 2**: Feedback (High variance).
*   **Encoder**: Tape Age / Warble / Hiss.

---

## 11. "Chiptune / NES" Tracker
**Concept**: Emulating the RP2A03 chip. Field handles the 2 Pulse + 1 Triangle channels. Pod handles the DPCM (Sample) channel and Noise.

**Scenario**: You play a melody on the keyboard using the Pulse channels. The Field keys enact classic tracker commands (Arpeggios, Portamento slides, Vibrato) live.

### Control Mapping
**Daisy Field (Pulse/Tri)**
*   **Knobs A-D**: Pulse 1 Width, Pulse 2 Width, Detune, Envelope Decay.
*   **Keys A1-A8 (Duty Cycle)**: 
    *   Quick sets for Pulse Width (12.5%, 25%, 50%, 75%).
*   **Keys B1-B8 (Tracker FX)**:
    *   `B1`: Fast Arp (007). `B2`: Slow Arp. `B3`: Vibrato (451). `B4`: Note Slide (1xx).

**Daisy Pod (DPCM/Noise)**
*   **Knob 1**: Sample Select / Pitch.
*   **Knob 2**: Noise Loop / Color.

---

## 12. "Polyrhythmic" Sequencer Host
**Concept**: The Field is strictly a MIDI Sequencer (generating notes). The Pod makes the sound (Simple Sine), or it drives external gear.
**Scenario**: You input a simple 4/4 clock. You use the Field keys to set different track lengths for Pitch, Velocity, and Gate, causing the sequence to phase and evolve over time (Steve Reich style).

### Control Mapping
**Daisy Field (Sequencer)**
*   **Knobs A-D**: Loop Start/End points for Pitch, Vel, Gate, Ratchet.
*   **Keys A1-A8 (Track Select)**: 
    *   Select active "Lane" (Pitch, Velocity, Duration, Ratchet).
*   **Keys B1-B8 (Length)**:
    *   Set the loop length of the selected lane (1-8 steps, or shift for 9-16). This creates the polyrhythms.

**Daisy Pod (Voice)**
*   **Knob 1**: Decay.
*   **Knob 2**: FM Amount (Simple Ding).

---

## 13. "Karplus-Strong" Drum Kit
**Concept**: Acoustic percussion modeling.
**Scenario**: Keyboard keys C1-C2 trigger different physical models. Field keys change the physics of the "room" or the "materials" instantly.

### Control Mapping
**Daisy Field**
*   **Knobs A-H**: Material parameters (Stiffness, Mass, Tension).
*   **Keys A1-A8 (Material)**: 
    *   `A1`: Wood. `A2`: Glass. `A3`: Metal. `A4`: Skin.
*   **Keys B1-B8 (Exciter)**:
    *   `B1`: Hard Stick. `B2`: Soft Mallet. `B3`: Brush.

**Daisy Pod (Dynamics)**
*   **Knob 1**: Compression Threshold.
*   **Knob 2**: Transient Shaping (Attack boost).

---

## 14. "Super-Saw" Trance Machine
**Concept**: Massive 7-sawtooth unison leads (Field) gated by a rhythmic VCA (Pod).
**Scenario**: You hold a big chord. It's a solid wall of sound. You engage the Pod's Trance Gate, and use the Field keys to swap the rhythmic pattern of the gate on the fly.

### Control Mapping
**Daisy Field (Super Saw)**
*   **Knobs A-D**: Detune, Mix, Filter Cutoff, Filter Envelope.
*   **Keys A1-A8 (Detune Snapshots)**: 
    *   `A1`: Pure/Thin. -> `A8`: Maximum Width/Detune.
*   **Keys B1-B8 (Gate Pattern Select)**:
    *   Sends Program Change to Pod to select Gate Rhythm 1-8.

**Daisy Pod (Trance Gate)**
*   **Knob 1**: Gate Mix (0% = Sustain, 100% = Chopped).
*   **Knob 2**: Gate Smoothness (Hard chop vs soft pulse).

---

## 15. "Morphing Wavetable" Pad
**Concept**: Vector Synthesis. Field plays two wavetables simultaneously. Pod manages stereo imaging.
**Scenario**: You play a pad. You use a joystick (or Knob A/B) to blend between Wavetable A and B. Field keys act as "Targets" to instantly jump to specific blend ratios.

### Control Mapping
**Daisy Field**
*   **Knobs A-B**: X/Y Position (Morphing inputs).
*   **Knobs C-D**: LFO Speed for X and Y.
*   **Keys A1-A8 (Wavetable A)**: Select base waveform for Osc 1.
*   **Keys B1-B8 (Wavetable B)**: Select base waveform for Osc 2.

**Daisy Pod (Chorus/Wide)**
*   **Knob 1**: Chorus Rate.
*   **Knob 2**: Chorus Depth / Width.
