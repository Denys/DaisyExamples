# Daisy Field Button Scenarios (A1-A8 / B1-B8)
## When Using External MIDI Keyboard

This guide focuses on repurposing the **Daisy Field's Tactile Keyboard** (Keys A1-A8 and B1-B8) for system control, performance effects, and musical constraints, assuming an external MIDI keyboard is handling standard note input.

---

## 1. State Management & Snapshots
**Context**: Instantly recalling complex parameter settings or changing the entire instrument's character.

*   **A1-A8: Preset Banks / Sound Models**
    *   **Scenario**: Switching between different physical models or synthesis types.
    *   **Mapping**:
        *   `A1`: Plucked String Model
        *   `A2`: Bowed String Model
        *   `A3`: Blown Pipe Model
        *   `A4`: Struck Bell Model
        *   `A5-A8`: User Presets 1-4 (Snapshots of all knob positions)

*   **B1-B8: Morph Targets / Macro Scenes**
    *   **Scenario**: Morphing between "Calm" and "Chaotic" states.
    *   **Mapping**:
        *   `B1`: "Init" State (Clean, Dry)
        *   `B2`: "Dark" State (Low Cutoff, High Reverb)
        *   `B3`: "Bright" State (High Cutoff, Distortion)
        *   `B4`: "Broken" State (Bitcrushed, Detuned)
        *   **Workflow**: Pressing a key interpolates (slews) all parameters to that state over 1 second.

---

## 2. Performance Effects (The "Playable" FX)
**Context**: Momentary effects to add rhythmic interest or tension during a performance.

*   **A1-A8: Loop & Stutter (Time Manipulation)**
    *   **Scenario**: Mangling a drum beat or sequence.
    *   **Mapping**:
        *   `A1`: Loop 1 bar
        *   `A2`: Loop 1/2 bar
        *   `A3`: Loop 1/4 bar
        *   `A4`: Loop 1/8 bar
        *   `A5`: Loop 1/16 bar (Buzz)
        *   `A6`: Loop Triplet
        *   `A7`: Reverse Playback (Momentary)
        *   `A8`: Tape Stop (Slow down to 0)

*   **B1-B8: Spectral & Dynamics (Color)**
    *   **Scenario**: Adding texture to a lead line or pad.
    *   **Mapping**:
        *   `B1`: Momentary High-Pass Filter (Drop preparation)
        *   `B2`: Momentary Bitcrush
        *   `B3`: Infinite Reverb Freeze
        *   `B4`: Delay Feedback Overload
        *   `B5`: Ring Modulator (Metallic punch)
        *   `B6`: Comb Filter tuned to Note
        *   `B7`: Sidechain Ducking (Fake kick pump)
        *   `B8`: Noise Burst Injection

---

## 3. Musical Constraints (Scales & Chords)
**Context**: Helping the player stay in key or generate complex harmonies from single notes.

*   **A1-A8: Scale Quantizer Selection**
    *   **Scenario**: Changing the emotional feel of the improvisation.
    *   **Mapping**:
        *   `A1`: Chromatic (No quantize)
        *   `A2`: Major Scale
        *   `A3`: Minor Scale
        *   `A4`: Dorian Mode
        *   `A5`: Phrygian Mode
        *   `A6`: Lydian Mode
        *   `A7`: Mixolydian Mode
        *   `A8`: Whole Tone Scale

*   **B1-B8: Chord Injection / Harmonizer**
    *   **Scenario**: Playing single notes on the MIDI keyboard, but hearing full chords.
    *   **Mapping**:
        *   `B1`: Octave Up (Add +12st)
        *   `B2`: Octave Down (Add -12st)
        *   `B3`: Fifth (Add +7st)
        *   `B4`: Major Triad (Root + 4 + 7)
        *   `B5`: Minor Triad (Root + 3 + 7)
        *   `B6`: Suspended 4th
        *   `B7`: Minor 9th
        *   `B8`: Random Harmony probability

---

## 4. Modulation & Physics
**Context**: Controlling LFOs, Envelopes, or Physical behaviors in real-time.

*   **A1-A8: LFO Shapes & Destinations**
    *   **Scenario**: Applying rhythmic modulation to a filter or pitch.
    *   **Mapping**:
        *   `A1`: Sine Wave -> Filter (Wobble)
        *   `A2`: Square Wave -> Amp (Tremolo/Gater)
        *   `A3`: Ramp Down -> Pitch (Laser Zaps)
        *   `A4`: Sample & Hold -> Filter (Random Steps)
        *   `A5-A8`: LFO Speed Multipliers (x1, x2, x4, x8 relative to BPM)

*   **B1-B8: Physical Excitation (For Modeling)**
    *   **Scenario**: Changing *how* a virtual instrument is played.
    *   **Mapping**:
        *   `B1`: Pluck (Impulse)
        *   `B2`: Strike (Hard mallet)
        *   `B3`: Bow (Continuous noise)
        *   `B4`: Blow (Breath pressure)
        *   `B5`: Scrape (Friction texture)
        *   `B6`: Damp (Palm mute simulation)
        *   `B7`: Harmonics (force node points)
        *   `B8`: Body Resonance (Toggle large/small body)

---

## 5. Navigation & Transport (Sequencer Mode)
**Context**: Controlling a multi-track sequencer or looper.

*   **A1-A8: Track Muting & Selection**
    *   **Scenario**: Building a track live by bringing elements in and out.
    *   **Mapping**:
        *   `A1-A4 (Short Press)`: Mute/Unmute Tracks 1-4.
        *   `A1-A4 (Long Press)`: Select Track 1-4 for editing.
        *   `A5`: Solo Selected Track.
        *   `A6`: Clear Selected Track.
        *   `A7`: Copy Track.
        *   `A8`: Paste Track.

*   **B1-B8: Pattern & Transport**
    *   **Scenario**: Launching clips or changing sequencer direction.
    *   **Mapping**:
        *   `B1`: Play / Stop.
        *   `B2`: Record / Overdub.
        *   `B3`: Tap Tempo.
        *   `B4`: Metronome On/Off.
        *   `B5`: Pattern Forward (->).
        *   `B6`: Pattern Backward (<-).
        *   `B7`: Pattern Ping-Pong (<->).
        *   `B8`: Pattern Random (?).

---

## 6. Vocal / Formant Synth Specific
**Context**: "Typing" the voice of the synthesizer.

*   **A1-A5 (Vowels)**: A, E, I, O, U.
*   **B1-B8 (Consonants/Articulation)**:
    *   `B1`: Hard Attack (K/T sound).
    *   `B2`: Soft Attack (H sound).
    *   `B3`: Sibilance (S/Sh noise).
    *   `B4`: Nasal boost.
    *   `B5`: Formant Shift Up (+2 semitone formant offset).
    *   `B6`: Formant Shift Down (-2 semitone formant offset).
    *   `B7`: Whisper Mode (unvoiced breath emphasis, reduced fundamental).
    *   `B8`: Growl Mode (subharmonic + saturation emphasis).

---

## 7. Field + Pod Integration (Standalone Key Actions)
**Context**: External MIDI keyboard plays notes; Field keys act as immediate command surface for cross-device control.

*   **Scenario: Pod FX Send Matrix on A Bank**
    *   **Trigger**: `A1-A8` (Short Press, toggle).
    *   **Behavior**: Toggles a dedicated send from the current Field voice/bus to a Pod effect slot.
    *   **Purpose**: Fast live routing to change texture without disrupting note performance.
    *   **Mapping**:
        *   `A1`: Send -> Pod Chorus.
        *   `A2`: Send -> Pod Flanger.
        *   `A3`: Send -> Pod Delay.
        *   `A4`: Send -> Pod Reverb.
        *   `A5`: Send -> Pod Bitcrush.
        *   `A6`: Send -> Pod Resonator.
        *   `A7`: Send -> Pod Granular Freeze Input.
        *   `A8`: Send -> Pod Bypass Return (dry safety route).

*   **Scenario: Performance Gesture Latch on B Bank**
    *   **Trigger**: `B1-B8` (Short Press = latch, Long Press = momentary hold).
    *   **Behavior**: Applies high-level performance gestures to the currently active scene.
    *   **Purpose**: One-hand expressive transformations while the external keyboard handles melody/harmony.
    *   **Mapping**:
        *   `B1`: Density Up (event rate +20%).
        *   `B2`: Density Down (event rate -20%).
        *   `B3`: Brightness Tilt Up.
        *   `B4`: Brightness Tilt Down.
        *   `B5`: Spread Wide (stereo widening + voice detune spread).
        *   `B6`: Spread Narrow (mono-focus performance mode).
        *   `B7`: Motion Boost (LFO depth/rate macro).
        *   `B8`: Motion Kill (freeze modulation phase).

---

## 8. SW1 Combos (Quantized Scene Performance)
**Context**: SW1 acts as a performance modifier key for musical changes that should stay in time.

*   **Scenario: Quantized Scene Launch**
    *   **Trigger**: `SW1 (Hold) + A1-A8 (Short Press)`.
    *   **Behavior**: Queues scene change and commits it on next bar boundary.
    *   **Purpose**: Prevent abrupt, off-grid scene jumps during live arrangement changes.
    *   **Mapping**:
        *   `A1-A8`: Launch Scene 1-8 (bar-quantized).

*   **Scenario: Transition Macros**
    *   **Trigger**: `SW1 (Hold) + B1-B8 (Short Press)`.
    *   **Behavior**: Fires a predefined transition macro over a fixed ramp time (250ms to 1500ms).
    *   **Purpose**: Create DJ-style transitions and section boundaries with deterministic behavior.
    *   **Mapping**:
        *   `B1`: Filter Sweep Down.
        *   `B2`: Filter Sweep Up.
        *   `B3`: Delay Throw (1/4 sync).
        *   `B4`: Delay Throw (1/8 dotted).
        *   `B5`: Reverb Bloom.
        *   `B6`: Tape Brake.
        *   `B7`: Reverse Window (1 beat).
        *   `B8`: Noise Lift + Hard Cut.

---

## 9. SW2 Combos (System / Routing Utilities)
**Context**: SW2 acts as a utility modifier for setup-level changes during rehearsal or live performance.

*   **Scenario: External MIDI Routing and Split Management**
    *   **Trigger**: `SW2 (Hold) + A1-A8 (Short Press)`.
    *   **Behavior**: Changes how external MIDI notes are distributed between Field and Pod engines.
    *   **Purpose**: Fast reconfiguration of keyboard behavior without entering menu pages.
    *   **Mapping**:
        *   `A1`: MIDI -> Field only.
        *   `A2`: MIDI -> Pod only.
        *   `A3`: MIDI -> Field + Pod layered.
        *   `A4`: MIDI split at C3 (low to Pod, high to Field).
        *   `A5`: MIDI split at C4.
        *   `A6`: MIDI split at C5.
        *   `A7`: Alternate-note dispatch (odd notes Field, even notes Pod).
        *   `A8`: Round-robin dispatch (voice alternation Field/Pod).

*   **Scenario: Clock, Sync, and Capture Utilities**
    *   **Trigger**: `SW2 (Hold) + B1-B8 (Short Press)`.
    *   **Behavior**: Executes non-destructive transport/sync/system utility actions.
    *   **Purpose**: Keep the integrated rig locked and recover quickly from drift or timing issues.
    *   **Mapping**:
        *   `B1`: Clock Source -> Internal.
        *   `B2`: Clock Source -> MIDI External.
        *   `B3`: Re-sync sequencers at next 1-bar boundary.
        *   `B4`: Re-sync immediately (hard reset step counters).
        *   `B5`: Snapshot Save (current system state).
        *   `B6`: Snapshot Recall (last saved state).
        *   `B7`: Arm Performance Capture (records macro events only).
        *   `B8`: Stop Capture and commit macro lane.

---

## 10. Safety Combos (Dual-Switch)
**Context**: Explicit recovery actions to avoid accidental destructive edits during live use.

*   **Scenario: Panic / Audio Safety Reset**
    *   **Trigger**: `SW1 + SW2 (Hold 1s) + A8 (Short Press)`.
    *   **Behavior**: Sends MIDI all-notes-off, clears stuck gates, ramps master down/up in 150ms, bypasses destructive FX.
    *   **Purpose**: Immediate recovery from stuck notes, runaway feedback, or unstable modulation.

*   **Scenario: Full Sync Recovery**
    *   **Trigger**: `SW1 + SW2 (Hold 1s) + B8 (Short Press)`.
    *   **Behavior**: Reinitializes shared tempo phase, transport counters, and inter-device scene index while preserving current audio buffers.
    *   **Purpose**: Recover timing coherence between Field, Pod, and external MIDI clock without hard reboot.
