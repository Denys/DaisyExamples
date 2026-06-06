# Delay Algorithm Structure Visualization

This package compares four visualization approaches for explaining the
`Field_delay_bundle` code and algorithms:

- Markdown Mermaid as the source-controlled reference.
- Graphify-style knowledge graph extraction from curated algorithm notes.
- FigJam generated diagram for editable discussion.
- GoJS static web explorer for interactive comparison.

The model is based on the current local bundle sources:

- `MyProjects/_projects/Field_delay_bundle/README.md`
- `MyProjects/_projects/Field_delay_bundle/CONTROLS.md`
- `MyProjects/_projects/Field_delay_shared/FieldDelayFieldApp.h`
- `DaisyHost/include/daisyhost/DaisyDelayFxCore.h`
- `DaisyHost/src/DaisyDelayFxCore.cpp`
- `C:/Users/denko/Codex/_weekly/Embedded_DSP_GitHub_Digest/docs/reports/2026-06-03-daisy-delay-source-verified-research.md`

## Conclusion

For durable code and algorithm documentation, keep Mermaid as the canonical
source. For the most comprehensible reader-facing artifact, use the GoJS page in
this folder because it can filter algorithms, show source-backed details, and
grow by editing one data file. Use FigJam for editable reviews and Graphify for
relationship discovery, not as final proof of exact DSP behavior.

Personal opinion: the best long-term setup is a two-layer system: Mermaid in
Git as the exact documentation source, then the GoJS explorer generated or
maintained from the same algorithm model. FigJam and Graphify are useful, but
they should feed the canonical model rather than replace it.

Open the collector page:

`docs/visualizations/delay-algorithm-structure/index.html`

The collector page has two independent selectors:

- `Algorithms` on the left contains the four implemented bundle modes plus
  the five selectable Phantasmagoria standalone delay algorithms. The right
  details pane only uses these standalone delay algorithms.
- `Diagram Content` on the left selects Bundle overview, Selected algorithm, or
  Controls and synth.
- `Diagram Technology` above the main diagram window selects GoJS, Mermaid,
  Graphify, or FigJam for that content.
- `Extracted Algorithm Blocks` below the diagram shows source-backed internal
  DSP blocks for the selected delay type, including inputs and outputs.
- `External Source Extractions` lists named algorithms extracted from reference
  projects that are not yet part of the Field bundle firmware.
- `Literature Review Synthesis` shows the Phantasmagoria review, the five
  standalone delay algorithms, three delay modifiers, exclusions, gaps, and
  recommendation.

Standalone review:

- `docs/visualizations/delay-algorithm-structure/phantasmagoria-delay-literature-review.md`

Graphify output:

- `docs/visualizations/delay-algorithm-structure/graphify-corpus/graphify-out/graph.json`
- `docs/visualizations/delay-algorithm-structure/graphify-corpus/graphify-out/GRAPH_TREE.html`

FigJam diagram:

https://www.figma.com/board/kX5emp9EAmxTxOjb1ecj4u?utm_source=other&utm_content=edit_in_figjam&oai_id=&request_id=0ab4552c-85b5-4b64-811b-b5d9d7db05e3

## External Source Extractions

### Phantasmagoria [FuzzyLotus]

Source: `FuzzyLotus/Phantasmagoria`

Evidence checked:

- GitHub README describes spectral delay, reverse delay, room mode, halo mode,
  freeze, freeze evolution, reverb taps, and hi-fi dynamics.
- `phantasmagoria.cpp` contains the DSP source for the named blocks below.
- This dashboard separates standalone delay algorithms from delay modifiers.
  Hi-Fi dynamics and constant-power dry/wet mixing are treated as support
  blocks, not extracted delay algorithms.

Selectable standalone delay algorithms:

| Name | Type | Source-backed role |
|---|---|---|
| Main Spectral Delay Line | Delay core | SDRAM delay write/read path with smoothed forward delay and feedback. |
| Reverse Dual-Grain Reader | Reverse delay | Dual windowed grain read is crossfaded with the forward read through SW1. |
| Smear Multi-Tap Diffusion | Diffusion | SW2 adds widened +10 ms and +25 ms taps into the audible wet and feedback path. |
| Echo Chamber Reverb Taps | Reverb | Separate reverb delay with fixed 83, 151, 227, and 311 ms taps. |
| Freeze Voice Bank | Freeze | Three freeze buffers hold or accumulate frozen audio layers. |

Delay modifiers kept in the review, not in the right details algorithm panel:

| Name | Type | Source-backed role |
|---|---|---|
| Tri-LFO Tape Warble | Delay-time modulation | Three sine LFOs modulate read time for tape-like instability. |
| Erosion Repeat Aging Filter | Repeat aging | SW3 progressively darkens and attenuates the audible delay read and feedback. |
| Freeze Evolution Drift | Freeze modulation | SW4 adds ultra-slow drift to frozen voices while freeze is active. |

## Algorithm Set

| Type-first label | Source project | Current function | Algorithm basis |
|---|---|---|---|
| Tape [multifx] | `balazsbencs/daisy-multifx-pedal` | `ProcessMultiFx` | Modulated tape-style circular delay with tone, grit, flutter, smoothed time, and freeze-like hold behavior. |
| Tank [reverb] | `Farmer2K5/daisy-reverb-playground` | `ProcessReverbPlayground` | Four-line damped feedback delay network with diffusion, early reflections, width, and tank size. |
| Texture [FunBox] | `GuitarML/FunBox` | `ProcessFunBox` | Multi-tap texture delay with normal taps, grain tap, reverse accent, drift, smear, freeze, and cross-feedback. |
| Long [sdram] | `Farmer2K5/daisy-sdram-delaylines` | `ProcessSdramDelaylines` | Long fractional stereo delay with external-buffer behavior, LFO modulation, warp taps, smear, and ping-pong feedback. |

## Bundle Code Architecture

```mermaid
---
title: Field Delay Bundle Architecture
---
flowchart LR
    accTitle: Field delay bundle architecture
    accDescr: Shows how Field inputs, UI, shared delay core, four algorithms, and outputs relate.

    subgraph input["Inputs"]
        audio["Audio In L/R"]
        midi["External MIDI"]
        bkeys["B1-B8 C4-C5 test keys"]
        akeys["A1-A8 mode keys"]
        knobs["K1-K8 and SW1/SW2"]
    end

    subgraph adapter["FieldDelayFieldApp.h"]
        keymap["Physical key map"]
        layers{"Layer changed or knob moved?"}
        snapshot["Per-algorithm snapshots"]
        oled["OLED main plus zoom"]
        leds["Three-state key LEDs"]
    end

    subgraph core["DaisyDelayFxCore"]
        synth["8-voice pluck/pad synth"]
        params["24 stable parameter slots"]
        switcher{"Selected DaisyDelayFxSource"}
        storage["4 external delay lines"]
        mixer["Dry/wet, bypass, wet-only, gain"]
    end

    subgraph algorithms["Algorithms"]
        tape["Tape [multifx]"]
        tank["Tank [reverb]"]
        texture["Texture [FunBox]"]
        long["Long [sdram]"]
    end

    audio -->|"input samples"| switcher
    midi -->|"note events"| synth
    bkeys -->|"C4-C5 notes"| keymap
    keymap -->|"note actions"| synth
    akeys -->|"select or cycle modes"| snapshot
    knobs -->|"movement-gated writes"| layers
    layers -->|"active-layer parameter"| params
    snapshot -->|"save and restore"| params
    params -->|"native values"| switcher
    synth -->|"internal tone"| switcher
    storage -->|"read/write buffers"| tape
    storage -->|"read/write buffers"| tank
    storage -->|"read/write buffers"| texture
    storage -->|"read/write buffers"| long
    switcher --> tape
    switcher --> tank
    switcher --> texture
    switcher --> long
    tape --> mixer
    tank --> mixer
    texture --> mixer
    long --> mixer
    mixer --> out["Audio Out L/R"]
    params --> oled
    snapshot --> oled
    oled --> display["Display"]
    leds --> display

    classDef inputFill fill:#E7F0FF,stroke:#2D5B9A,color:#10233F
    classDef adapterFill fill:#FFF2D5,stroke:#B87716,color:#3A2600
    classDef coreFill fill:#E7F8EC,stroke:#2E7A45,color:#12351E
    classDef algoFill fill:#F1E9FF,stroke:#7353BA,color:#241843
    classDef outputFill fill:#FFEAEA,stroke:#A94747,color:#3B1717
    class audio,midi,bkeys,akeys,knobs inputFill
    class keymap,layers,snapshot,oled,leds adapterFill
    class synth,params,switcher,storage,mixer coreFill
    class tape,tank,texture,long algoFill
    class out,display outputFill
```

## Signal Path Overview

```mermaid
---
title: Audio Callback Signal Path
---
flowchart LR
    accTitle: Audio callback signal path
    accDescr: Shows one audio frame through input, internal synth injection, selected algorithm, and output mix.

    input["Audio input sample"] --> drive["Input drive and algorithm input shaping"]
    midi["MIDI or B-key note"] --> voice["SynthVoice allocation and envelope"]
    voice --> synth["ProcessInternalSynth"]
    synth --> sum["Add internal synth to audio input"]
    drive --> sum
    sum --> select{"Active source"}
    select --> tape["ProcessMultiFx"]
    select --> tank["ProcessReverbPlayground"]
    select --> texture["ProcessFunBox"]
    select --> long["ProcessSdramDelaylines"]
    tape --> wet["Wet L/R"]
    tank --> wet
    texture --> wet
    long --> wet
    wet --> mix["Mix dry and wet"]
    sum --> mix
    mix --> clamp["Clamp and output gain"]
    clamp --> output["Audio output sample"]

    classDef signal fill:#E7F0FF,stroke:#2D5B9A,color:#10233F
    classDef synthFill fill:#E7F8EC,stroke:#2E7A45,color:#12351E
    classDef algo fill:#F1E9FF,stroke:#7353BA,color:#241843
    classDef outputFill fill:#FFEAEA,stroke:#A94747,color:#3B1717
    class input,drive,sum,wet,mix,clamp signal
    class midi,voice,synth synthFill
    class tape,tank,texture,long,select algo
    class output outputFill
```

## Per-Algorithm Signal Diagrams

### Tape [multifx]

```mermaid
flowchart LR
    accTitle: Tape algorithm signal structure
    accDescr: Tape uses smoothed delay reads, flutter, feedback tone, saturation grit, and freeze-aware writes.

    input["Driven mono input"] --> smooth["Smoothed target delay"]
    lfo["Flutter LFO"] --> read["Stereo delay reads"]
    smooth --> read
    read --> tone["Feedback tone filter"]
    tone --> grit["FastTanh grit blend"]
    input --> grit
    grit --> freeze{"Freeze amount"}
    read --> freeze
    freeze --> write["Write delay lines"]
    read --> wet["Wet L/R"]
    write --> read
```

### Tank [reverb]

```mermaid
flowchart LR
    accTitle: Tank algorithm signal structure
    accDescr: Tank uses four size-scaled delay lines, damping, diffusion, and a Hadamard-style feedback network.

    input["Driven mono input"] --> diffuse["Diffusion injection"]
    size["Tank size"] --> reads["Four delay reads"]
    reads --> damp["Per-line damping"]
    damp --> matrix["Sum/difference feedback matrix"]
    matrix --> writes["Four feedback writes"]
    diffuse --> writes
    damp --> stereo["Stereo width and early output"]
    writes --> reads
```

### Texture [FunBox]

```mermaid
flowchart LR
    accTitle: Texture algorithm signal structure
    accDescr: Texture combines normal taps, grain tap, reverse accent, drift, smear, freeze, and cross-feedback writes.

    input["Driven stereo input"] --> normal["Normal L/R taps"]
    drift["Slow drift LFO"] --> normal
    drift --> grain["Grain tap"]
    density["Density and smear"] --> grain
    normal --> blend["Texture blend"]
    grain --> blend
    reverse["Reverse accent tap"] --> blend
    blend --> cross["Cross-feedback writes"]
    freeze{"Freeze or hold"} --> cross
    cross --> normal
    blend --> wet["Wet L/R"]
```

### Long [sdram]

```mermaid
flowchart LR
    accTitle: Long SDRAM algorithm signal structure
    accDescr: Long delay uses smoothed fractional reads, slow LFO modulation, secondary warp taps, smear, and ping-pong feedback.

    input["Driven stereo input"] --> cross["Ping-pong cross feedback"]
    time["Long base time"] --> primary["Primary fractional L/R reads"]
    lfo["Slow modulation"] --> primary
    time --> warp["Secondary warp taps"]
    primary --> cross
    warp --> cross
    cross --> primary
    primary --> smear["Smear blend"]
    warp --> smear
    smear --> wet["Wet L/R"]
```

## Control And UI Diagrams

```mermaid
---
title: Until-Touched Parameter Layers
---
stateDiagram-v2
    accTitle: Until-touched knob layer state
    accDescr: Changing layer does not write any parameter until the physical knob moves in that active layer.

    [*] --> BaseLayer
    BaseLayer --> Shift1Layer: hold SW1
    BaseLayer --> Shift2Layer: hold SW2
    Shift1Layer --> BaseLayer: release SW1
    Shift2Layer --> BaseLayer: release SW2
    BaseLayer --> Armed: enter current layer
    Shift1Layer --> Armed: enter shift 1
    Shift2Layer --> Armed: enter shift 2
    Armed --> Waiting: store entry knob values
    Waiting --> Waiting: knob unchanged
    Waiting --> Writing: knob movement exceeds epsilon
    Writing --> Writing: write active-layer parameter
    Writing --> Armed: layer changes
```

```mermaid
---
title: Algorithm Selection Snapshot Flow
---
sequenceDiagram
    accTitle: Algorithm switch snapshot sequence
    accDescr: Shows how A1-A4 algorithm switching preserves each algorithm's parameter values.

    participant Key as A1-A4 key
    participant Field as Field adapter
    participant Core as DaisyDelayFxCore
    participant Store as Snapshot store
    participant OLED as OLED zoom

    Key->>Field: rising edge
    Field->>Core: read current source index
    Field->>Store: save current parameter values
    Field->>Core: SetSource(new source)
    Field->>Store: restore new source values
    Field->>Field: reset layer touch gates
    Field->>OLED: show algorithm zoom
```

```mermaid
---
title: Keybed Input And LED Mapping
---
flowchart LR
    accTitle: Field keybed input and LED mapping
    accDescr: Shows why input and LED mapping are intentionally represented as separate concerns.

    physical["Physical key press"] --> scan{"Observed scan row"}
    scan -->|"B1-B8 = index 0-7"| binput["B-row input path"]
    scan -->|"A1-A8 = index 8-15"| ainput["A-row input path"]
    binput --> notes["C4-C5 note actions"]
    ainput --> modes["Algorithm, synth, hold, octave modes"]
    notes --> ledlogic["Logical LED state model"]
    modes --> ledlogic
    ledlogic --> ledorder["DaisyField LED enum order"]
    ledorder --> keyleds["Physical key LEDs"]
```

## Approach Comparison

| Approach | Strongest use | Weakest use | Fit for code structure | Fit for DSP signal flow | Best role |
|---|---|---|---:|---:|---|
| Markdown Mermaid | Versioned docs, reviews, exact control-flow invariants | Large exploratory graphs | High | High | Canonical source |
| GoJS web explorer | Interactive comparison and future expansion | Small one-off diagrams | High | High | Main reader experience |
| FigJam generated diagram | Editable workshop artifact | Long-term exact source of truth | Medium | Medium | Stakeholder discussion |
| Graphify knowledge graph | Relationship discovery across notes and code summaries | Sample-accurate signal flow | Medium | Low-Medium | Ingestion and discovery |

Recommended operating model:

1. Add or change an algorithm in `data/algorithms.js`.
2. Update the Mermaid source in this README when the structure changes.
3. Refresh the GoJS page by reloading `index.html`.
4. Regenerate the FigJam diagram only for review sessions.
5. Re-run Graphify when the corpus grows enough that new relationships matter.

## Verification Notes

Commands run from the DaisyExamples repo root:

```powershell
node --check docs\visualizations\delay-algorithm-structure\data\algorithms.js
node -e "const fs=require('fs'); const h=fs.readFileSync('docs/visualizations/delay-algorithm-structure/index.html','utf8'); const scripts=[...h.matchAll(/<script>([\s\S]*?)<\/script>/g)].map(m=>m[1]); scripts.forEach(s=>new Function(s)); console.log(scripts.length)"
```

Result: pass. The page has one inline script and it compiles.

Graphify semantic Markdown extraction was attempted first:

```powershell
& C:\Users\denko\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe -m graphify extract docs\visualizations\delay-algorithm-structure\graphify-corpus --no-cluster --out docs\visualizations\delay-algorithm-structure\graphify-generated --max-concurrency 1
```

Result: blocked because no Graphify LLM backend API key was visible in this
shell. The exact Graphify error was:

```text
error: no LLM API key found. Set GEMINI_API_KEY or GOOGLE_API_KEY (gemini), MOONSHOT_API_KEY (kimi), ANTHROPIC_API_KEY (claude), OPENAI_API_KEY (openai), DEEPSEEK_API_KEY (deepseek), or pass --backend.
```

Fallback Graphify no-LLM AST extraction was then run against
`graphify-corpus/delay_bundle_structure.cpp`, a source-shaped seed that mirrors
current bundle code names and call relationships:

```powershell
& C:\Users\denko\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe -m graphify update docs\visualizations\delay-algorithm-structure\graphify-corpus --no-cluster --force
& C:\Users\denko\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe -m graphify tree --graph docs\visualizations\delay-algorithm-structure\graphify-corpus\graphify-out\graph.json --output docs\visualizations\delay-algorithm-structure\graphify-corpus\graphify-out\GRAPH_TREE.html --label "Field Delay Bundle Structure"
& C:\Users\denko\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe -m graphify diagnose multigraph --graph docs\visualizations\delay-algorithm-structure\graphify-corpus\graphify-out\graph.json --json
```

Result: pass. Graphify generated 88 nodes and 152 links. The multigraph
diagnostic reported no missing endpoints, dangling endpoints, self loops,
duplicate edges, or same-endpoint collapse groups.

Browser check: a Playwright render using temporary `playwright-core` and the
system Chrome binary successfully loaded the page after the diagram technology
selector change. It verified four algorithm buttons, three content views, four
diagram technology buttons, selectable Mermaid, Graphify, and FigJam panes, and
no 1440px horizontal overflow.

Current extracted-block dashboard change was verified with local Node checks:
the model reports `tape:5,tank:5,texture:5,long:5`, and the HTML still has one
compiling inline script.

## Expandability Contract

To add a new delay algorithm later:

1. Add one entry to `data/algorithms.js`.
2. Add one Markdown note to `graphify-corpus/`.
3. Add a Mermaid per-algorithm signal diagram to this README.
4. Re-run Graphify if you want the knowledge-graph output refreshed.
5. Open `index.html` and confirm the selector, graph, details, and tables render.
