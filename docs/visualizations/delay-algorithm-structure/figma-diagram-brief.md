# FigJam Diagram Brief

Generated FigJam diagram:

https://www.figma.com/board/kX5emp9EAmxTxOjb1ecj4u?utm_source=other&utm_content=edit_in_figjam&oai_id=&request_id=0ab4552c-85b5-4b64-811b-b5d9d7db05e3

## Purpose

Explain the `Field_delay_bundle` firmware and shared DaisyHost delay core to a
reader who needs to understand:

- How Field controls reach shared DSP parameters.
- How audio, MIDI, and B-row test notes enter the selected algorithm.
- Why Tape, Tank, Texture, and Long are structurally different.
- Where the code keeps state that must remain stable during the audio callback.

## Current FigJam Diagram Scope

The generated diagram is a single source-backed flowchart with these lanes:

1. Inputs: audio, MIDI, B keys, A keys, knobs, switches.
2. Field UI adapter: physical key mapping, knob layer gate, snapshots, OLED, LEDs.
3. DaisyDelayFxCore: synth, parameters, source switch, delay storage, final mixer.
4. Algorithms: Tape, Tank, Texture, Long.
5. Outputs: audio output and display output.

## Mermaid Source Used For FigJam

```mermaid
flowchart LR
    subgraph inputs ["Inputs"]
        audioIn["Audio In L/R"]
        midiIn["External MIDI"]
        bKeys["B1-B8 white keys"]
        aKeys["A1-A8 mode keys"]
        knobs["K1-K8 plus SW1/SW2"]
    end

    subgraph fieldUi ["Daisy Field UI Adapter"]
        keyMap["Physical key map: B input and separate LED order"]
        layerGate{"Layer context moved?"}
        snapshots["Per-algorithm parameter snapshots"]
        oled["OLED main view and short zoom"]
        ledState["Three-state key LED model"]
    end

    subgraph core ["DaisyDelayFxCore"]
        synth["8-voice pluck/pad synth"]
        params["24 stable parameter slots"]
        sourceSwitch{"Selected source"}
        storage["4 external delay lines"]
        mixer["Dry/wet, bypass, wet-only, output gain"]
    end

    subgraph algorithms ["Delay Algorithms"]
        tape["Tape [multifx]: flutter, tone, grit, freeze"]
        tank["Tank [reverb]: damped FDN, diffusion, early reflections"]
        texture["Texture [FunBox]: grain, reverse, smear, hold"]
        longAlg["Long [sdram]: fractional stereo, ping-pong taps"]
    end

    subgraph outputs ["Outputs"]
        audioOut["Audio Out L/R"]
        displayOut["OLED and LEDs"]
    end

    midiIn -->|"Note on/off"| synth
    bKeys -->|"C4-C5 triggers"| keyMap
    keyMap -->|"Field key action"| synth
    aKeys -->|"Select source or synth mode"| snapshots
    knobs -->|"Movement-gated layer write"| layerGate
    layerGate -->|"Set active parameter only"| params
    snapshots -->|"Save and restore"| params
    params -->|"Native values"| sourceSwitch
    audioIn -->|"Input signal"| synth
    synth -->|"Internal test/performance tone"| sourceSwitch
    sourceSwitch --> tape
    sourceSwitch --> tank
    sourceSwitch --> texture
    sourceSwitch --> longAlg
    storage -->|"Read/write buffers"| tape
    storage -->|"Read/write buffers"| tank
    storage -->|"Read/write buffers"| texture
    storage -->|"Read/write buffers"| longAlg
    tape --> mixer
    tank --> mixer
    texture --> mixer
    longAlg --> mixer
    mixer --> audioOut
    params --> oled
    snapshots --> oled
    oled --> displayOut
    ledState --> displayOut
    keyMap --> ledState
```

## Recommended Manual FigJam Refinements

- Split the single generated flowchart into four side-by-side algorithm insets
  if it becomes crowded during review.
- Add sticky notes with code references:
  - `DaisyDelayFxCore.cpp:323` for `Process`.
  - `DaisyDelayFxCore.cpp:776` for stable parameter slot rebuild.
  - `FieldDelayFieldApp.h:332` for movement-gated knob writes.
  - `FieldDelayFieldApp.h:135` for keybed scan mapping.
- Keep the FigJam file as a review artifact. Update the Mermaid and GoJS data
  files first when code behavior changes.
