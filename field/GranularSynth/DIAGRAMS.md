# GranularSynth Mermaid Diagram

This diagram reflects the current `GranularSynth.cpp` firmware structure. The
firmware uses 13 Field keyboard notes with an 8-voice allocator; the companion
`GranularSynth.dvpe` starter patch now represents that polyphonic section as
one compact `poly_grainlet_voice` block with shared filter, reverb, output, and
K1-K8 macro controls.

```mermaid
flowchart LR
    %% Hardware and runtime
    FIELD["Daisy Field hardware"]
    MAIN["main loop<br/>UpdateControls<br/>UpdateDisplay<br/>UpdateLeds"]
    AUDIO["audio callback<br/>48 kHz block processing"]
    OUT["Field stereo output<br/>OUT L / OUT R"]

    FIELD --> MAIN
    FIELD --> AUDIO

    %% Controls
    subgraph UI["Field controls and feedback"]
        KEYS["13 playable keys<br/>A1-A8, B2, B3, B5-B7"]
        SW["SW1 / SW2<br/>octave down / up"]
        KNOBS["K1-K8 macro knobs"]
        OLED["OLED status<br/>octave, voice count, params"]
        LEDS["knob, key, switch LEDs"]
    end

    FIELD --> KEYS
    FIELD --> SW
    FIELD --> KNOBS
    MAIN --> OLED
    MAIN --> LEDS

    %% Control state
    subgraph STATE["ControlState"]
        OCT["octave<br/>0..4"]
        SHAPE["shape"]
        FORMANT["formant Hz<br/>120..6000"]
        BLEED["bleed"]
        ATTACK["attack<br/>0.001..1.251 s"]
        RELEASE["release<br/>0.02..2.52 s"]
        CUTOFF["low-pass cutoff<br/>180..12000 Hz"]
        SPREAD["stereo spread"]
        REVAMT["reverb amount"]
    end

    SW --> OCT
    KNOBS --> SHAPE
    KNOBS --> FORMANT
    KNOBS --> BLEED
    KNOBS --> ATTACK
    KNOBS --> RELEASE
    KNOBS --> CUTOFF
    KNOBS --> SPREAD
    KNOBS --> REVAMT

    %% Note handling
    KEYS --> NOTEEDGE{"keyboard<br/>rising/falling edge"}
    OCT --> MIDI["KeyToMidi<br/>scale + octave"]
    NOTEEDGE --> MIDI
    MIDI --> MTOF["mtof"]
    MTOF --> VOICEMGR["VoiceManager<br/>8 voices<br/>reuse key, free voice, oldest steal"]
    NOTEEDGE --> VOICEMGR
    AUDIO --> VOICEMGR

    %% Per-voice synthesis
    subgraph VOICE["GrainVoice x8"]
        GRAIN["GrainletOscillator<br/>freq, shape, formant, bleed"]
        ENV["ADSR<br/>attack, decay 0.08, sustain 0.85, release"]
        PAN["equal-power pan<br/>key position x spread"]
        VOUT["voice stereo pair"]
        GRAIN --> VOUT
        ENV --> VOUT
        PAN --> VOUT
    end

    VOICEMGR --> GRAIN
    VOICEMGR --> ENV
    VOICEMGR --> PAN

    SHAPE --> GRAIN
    FORMANT --> GRAIN
    BLEED --> GRAIN
    ATTACK --> ENV
    RELEASE --> ENV
    SPREAD --> PAN

    %% Audio path
    VOUT --> MIX["dry stereo sum"]
    MIX --> LPF["SVF low-pass<br/>left + right<br/>res 0.35, drive 0.6"]
    CUTOFF --> LPF
    LPF --> DRY["filtered dry signal"]
    DRY --> REV["ReverbSc<br/>feedback 0.78 + 0.18 * amount<br/>LP 8000 Hz"]
    REVAMT --> REV
    DRY --> FINAL["dry + wet mix<br/>gain 0.18<br/>clamp -1..1"]
    REV --> FINAL
    REVAMT --> FINAL
    FINAL --> OUT

    %% DVPE companion
    subgraph DVPE["GranularSynth.dvpe companion patch"]
        DVPEVOICE["poly_grainlet_voice<br/>13 keys, 8 voices"]
        DVPEFX["left/right SVF + ReverbSc + output"]
        DVPEKNOBS["K1-K8 macro controls"]
        DVPEKNOBS --> DVPEVOICE
        DVPEKNOBS --> DVPEFX
        DVPEVOICE --> DVPEFX
    end

    VOICEMGR -. firmware source model .-> DVPE
```
