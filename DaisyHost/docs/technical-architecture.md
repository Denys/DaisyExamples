# DaisyHost Technical Architecture

This document is the source-oriented companion to the main DaisyHost README. It explains
how the current desktop host is divided, where board semantics live, how the two-node rack
is bounded, and which evidence can and cannot be carried from the desktop host to Daisy
firmware or hardware.

The goal is not to restate every tracker entry. It is to give a stable map that lets an
engineer find the right contract before changing code.

## 1. System map

```mermaid
flowchart TD
    accTitle: DaisyHost Technical System Map
    accDescr: DaisyHost separates board-surface metadata, JUCE live-host state, portable hosted-app contracts, offline render and CLI tooling, and separate Daisy firmware adapters. The desktop host does not claim full libDaisy or STM32 emulation.

    %% Styling definitions
    classDef surfaceStyle fill:#334155,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef juceStyle fill:#2563EB,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef coreStyle fill:#0F766E,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef stateStyle fill:#7C3AED,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef offlineStyle fill:#D97706,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef firmwareStyle fill:#166534,stroke:#ffffff,stroke-width:2px,color:#ffffff;
    classDef guardStyle fill:#B91C1C,stroke:#ffffff,stroke-width:2px,color:#ffffff;

    subgraph layer_surface["1. Board and UI Surfaces"]
        profile["BoardProfile<br/>Patch / Field geometry, ports, display, surface metadata"]:::surfaceStyle
        mapping["BoardControlMapping<br/>Field K/CV/SW/key/LED bindings"]:::surfaceStyle
        editor["DaisyHostPluginEditor<br/>JUCE controls, rack header, OLED, Host Tools"]:::juceStyle
        hub["DaisyHost Hub<br/>board / app / activity launcher"]:::juceStyle
    end

    subgraph layer_live["2. Live JUCE Host"]
        processor["DaisyHostPluginProcessor<br/>2 rack nodes, audio/MIDI, selected-node state"]:::juceStyle
        topology["LiveRackTopology<br/>node0 only / node1 only / serial 0→1 / 1→0"]:::stateStyle
        session["HostSessionState v5<br/>board, selected node, node app state, routes"]:::stateStyle
        automation["HostAutomationBridge<br/>5 stable DAW slots"]:::stateStyle
        modulation["HostModulation<br/>4 lanes from CV1-4 or LFO1-4"]:::stateStyle
        snapshot["EffectiveHostStateSnapshot<br/>operator / agent readback"]:::stateStyle
    end

    subgraph layer_apps["3. Hosted App Contract"]
        registry["AppRegistry<br/>app id → factory"]:::coreStyle
        interface["HostedAppCore<br/>Prepare / Process / controls / ports / menu / state"]:::coreStyle
        apps["App cores<br/>MultiDelay / Torus / CloudSeed / Braids / Harmoniqs / VA / PolyOsc / Subharmoniq / pedal_multidelay / Field delay adaptations"]:::coreStyle
        pedal["PedalDelayEngine<br/>host-side compact delay DSP engine"]:::coreStyle
    end

    subgraph layer_offline["4. Offline and Automation Surfaces"]
        render["RenderRuntime + DaisyHostRender<br/>scenario → WAV + manifest"]:::offlineStyle
        cli["DaisyHostCLI<br/>discovery / describe / validate / render / snapshot / doctor / gate"]:::offlineStyle
        training["training/<br/>dataset sweeps and checked-in scenarios"]:::offlineStyle
    end

    subgraph layer_firmware["5. Separate Firmware Boundary"]
        patch_fw["patch/MultiDelay<br/>Patch firmware adapter"]:::firmwareStyle
        field_fw["field/MultiDelay<br/>Field firmware adapter"]:::firmwareStyle
        generated_fw["field/MultiDelayGenerated<br/>generated adapter proof"]:::firmwareStyle
        guard["Evidence guard<br/>host pass ≠ ARM timing / analog / hardware pass"]:::guardStyle
    end

    profile --> editor
    mapping --> editor
    hub --> processor
    editor <--> processor
    processor <--> topology
    processor <--> session
    processor <--> automation
    processor <--> modulation
    processor --> snapshot

    processor --> registry
    registry --> interface
    interface --> apps
    apps --> pedal

    registry --> render
    topology --> render
    session -. schema concepts .-> render
    render --> cli
    render --> training
    snapshot --> cli

    interface -. shared behavioral contracts only .-> patch_fw
    interface -. shared behavioral contracts only .-> field_fw
    interface -. generated adapter contract .-> generated_fw
    patch_fw --> guard
    field_fw --> guard
    generated_fw --> guard
    processor --> guard
```

## 2. Architectural boundaries

### Board metadata is data, not app logic

[`BoardProfile`](../include/daisyhost/BoardProfile.h) owns board identity and desktop
surface description:

- board ID and display name;
- controls and their panel bounds;
- typed virtual ports;
- 128 × 64 display description;
- editor-surface policy;
- decorations, labels, and indicators.

The factory currently accepts exactly `daisy_patch` and `daisy_field`. Unsupported IDs
return no profile through `TryCreateBoardProfile` and throw through `CreateBoardProfile`.
This is preferable to silently creating the wrong board.

Field-specific binding policy lives in
[`BoardControlMapping`](../include/daisyhost/BoardControlMapping.h), where K1-K8,
CV1-CV4, CV OUT monitors, SW1/SW2, A/B keys, and LEDs are represented as explicit
bindings with target kinds and availability state.

### The JUCE editor does not own DSP truth

[`DaisyHostPluginEditor`](../src/juce/DaisyHostPluginEditor.cpp) renders controls and
forwards user interaction. It gets labels, values, display/menu snapshots, app identities,
and rack state from the processor and board/app contracts.

This keeps three things from collapsing into one giant GUI object:

1. panel geometry;
2. live host state;
3. hosted-app DSP/control semantics.

A UI label is therefore presentation, not an independent parameter definition.

### The processor is the live integration point

[`DaisyHostPluginProcessor`](../src/juce/DaisyHostPluginProcessor.cpp) is the JUCE audio
processor and the current live-host integration layer. Its public API exposes:

- selected board profile;
- two rack nodes and selected-node control;
- per-node app selection;
- topology selection;
- top controls and Field controls;
- CV/gate state and CV generators;
- MIDI keyboard/tracking/learn support;
- test-input generators;
- host modulation lanes;
- OLED/menu/parameter snapshots;
- version/build identity;
- effective-state readback.

The header contains mutex/atomic state because the desktop host must bridge JUCE/UI/audio
concerns. Do **not** use this class as evidence that the same synchronization strategy is
suitable inside an embedded Daisy audio callback.

## 3. Hosted app contract

The stable app-side abstraction is
[`HostedAppCore`](../include/daisyhost/HostedAppCore.h).

An app must provide:

- stable app ID and display name;
- declared capabilities;
- Patch/Field binding metadata;
- `Prepare(sampleRate, maxBlockSize)`;
- block `Process(...)`;
- control and virtual-port setters;
- parameter descriptors and values;
- menu model and navigation;
- display model;
- state capture/restore/reset.

### Parameter metadata

`ParameterDescriptor` carries both generic host fields and the newer pedal-specific
contract. Pedal-style parameters can declare:

- `slotId`;
- musician-facing `label`;
- `engineeringLabel`;
- native unit/range/default;
- curve;
- exact `dspTargets`;
- smoothing time;
- macro flag;
- provisional flag.

That metadata is why README text should not become the canonical control database.
`DaisyHostCLI describe-app <app> --json` is the better inspection surface when exact
current ranges or DSP targets matter.

## 4. App registry and factories

[`AppRegistry`](../src/AppRegistry.cpp) maps supported app IDs to factories. Live JUCE,
CLI description, and offline render paths all use the same hosted-app contract rather than
maintaining separate lists of DSP implementations.

Representative registered app families in the current tree include:

- MultiDelay regression/reference core;
- Torus;
- CloudSeed;
- Braids;
- Harmoniqs;
- VA Synth;
- PolyOsc;
- Subharmoniq;
- `pedal_multidelay`;
- Field delay/Fx adaptations and bundle.

Use `DaisyHostCLI list-apps --json` for current machine-readable inventory instead of
copying this list into automation.

## 5. Two-node rack contract

The live runtime is deliberately bounded to two nodes. The visible IDs are `node0` and
`node1`.

[`LiveRackTopology`](../include/daisyhost/LiveRackTopology.h) owns legal topology presets
and role labels. The current operator-facing audio presets are:

- `node0_only`;
- `node1_only`;
- `node0_to_node1`;
- `node1_to_node0`.

This is a fixed topology family, not a graph editor. That constraint matters because it
keeps:

- session representation bounded;
- route validation explicit;
- UI targeting understandable;
- render scenarios deterministic;
- regression coverage finite.

### Selection and topology are orthogonal

The selected node determines where live control, Field/Patch surface input, keyboard MIDI,
CV/gate/test input, menu edits, and automation binding operate. Topology determines audio
ordering.

A serial topology can therefore have `node0` first in the audio chain while `node1` is the
currently selected edit target. This is intentional.

## 6. Session state

[`HostSessionState`](../include/daisyhost/HostSessionState.h) is the durable desktop-host
session contract. The current README/tracker identifies the live schema as v5, carrying
rack-global state plus node and route records while retaining backward compatibility with
legacy single-node sessions.

Treat session serialization as a host contract. It is not a firmware preset format unless a
firmware path explicitly adopts the same schema.

## 7. Automation and modulation

### Stable DAW automation slots

[`HostAutomationBridge`](../src/HostAutomationBridge.cpp) exposes five stable host slots:

- `daisyhost.slot1`
- `daisyhost.slot2`
- `daisyhost.slot3`
- `daisyhost.slot4`
- `daisyhost.slot5`

The slot IDs remain stable while their selected-app parameter targets can be rebound. App
state remains canonical by app parameter ID rather than by whichever DAW slot happened to
control it.

### Host modulation

[`HostModulation`](../include/daisyhost/HostModulation.h) defines four modulation lanes.
Each lane can select CV1-CV4 or LFO1-LFO4 and applies its contribution in the target
parameter's native range before normalization/clamping.

The effective result can be inspected through the live snapshot. This is valuable for
external tooling because it distinguishes base state from effective modulated state.

## 8. Patch and Field UI composition

```mermaid
flowchart LR
    accTitle: Board surface composition
    accDescr: BoardProfile supplies common panel metadata. Patch uses the top-control and typed-port view. Field adds BoardControlMapping to populate its extended K, key, switch, CV and indicator surface. Both edit the same selected hosted rack node.

    app["Hosted app<br/>bindings + parameter/menu metadata"]
    profile["BoardProfile"]
    mapping["BoardControlMapping"]
    patch["Patch editor surface<br/>CTRL1-4 + ENC + typed ports + OLED"]
    field["Field editor surface<br/>K1-8 + A/B + SW1/2 + CV/Gate + indicators"]
    processor["Selected rack node"]

    app --> profile
    app --> mapping
    profile --> patch
    profile --> field
    mapping --> field
    patch --> processor
    field --> processor
```

Patch and Field should therefore remain visually distinct while sharing the same live app
and state machinery.

## 9. Test input and MIDI paths

The processor exposes these test-input modes in source:

- Host Input;
- Sine;
- Saw;
- Noise;
- Impulse;
- Triangle;
- Square.

The standalone default remains Host Input. Generated sources exist to separate app/DSP
problems from OS/device/input problems.

MIDI can arrive from:

- the DAW/plugin MIDI bus;
- the on-screen keyboard;
- the computer keyboard;
- an external standalone MIDI device enabled through JUCE settings.

MIDI learn is CC-based. Recent note/CC/program activity is also exposed through the host
tracker UI.

## 10. Offline render and CLI

[`RenderRuntime`](../src/RenderRuntime.cpp) is a separate headless path built on the same
hosted-app contracts. Scenario JSON can drive parameters, CV, gate, MIDI, test input, and
node-targeted events, then emit:

- `audio.wav`;
- `manifest.json`;
- render/debug JSON through CLI surfaces.

`DaisyHostCLI` currently provides the practical automation surface for:

- app/board discovery;
- app/board description;
- scenario validation;
- render;
- snapshot;
- smoke;
- doctor/preflight;
- gate orchestration;
- render assertions.

`doctor` is a readiness report, not a substitute for running the gate. Render assertions are
host evidence, not physical-audio evidence.

## 11. `pedal_multidelay` architecture

The compact delay app has two distinct layers:

1. [`PedalDelayEngine`](../include/daisyhost/PedalDelayEngine.h) and
   [`src/PedalDelayEngine.cpp`](../src/PedalDelayEngine.cpp): the host-side five-mode delay
   DSP engine;
2. [`PedalDelayCore`](../include/daisyhost/apps/PedalDelayCore.h) and
   [`src/apps/PedalDelayCore.cpp`](../src/apps/PedalDelayCore.cpp): hosted-app adapter,
   parameter/menu metadata, board bindings, state, and port semantics.

The current technical note documents host-measured policies and known gaps:
[`pedal-multidelay.md`](pedal-multidelay.md).

Important boundary: the host implementation can be useful for interaction and DSP-policy
experiments without becoming the accepted Daisy target-performance proof. ARM build,
Cortex-M7 cycle cost, SDRAM/cache behavior, physical audio, and pedal hardware remain
separate gates.

## 12. Firmware adapters are separate products of evidence

Current repository firmware references include:

- [`patch/MultiDelay`](../../patch/MultiDelay/README.md)
- [`field/MultiDelay`](../../field/MultiDelay/README.md)
- [`field/MultiDelayGenerated`](../../field/MultiDelayGenerated/README.md)

A shared core or matching control contract can reduce duplication, but each firmware target
still needs its own build and hardware evidence. Desktop JUCE controls, Windows scheduling,
and host memory behavior cannot be promoted into embedded facts by diagram.

## 13. Verification model

DaisyHost has several evidence levels. Keep them separate:

| Evidence | What it can prove | What it cannot prove |
|---|---|---|
| Unit/CTest | Host contracts and deterministic regressions covered by tests. | Physical GUI feel, DAW behavior, hardware. |
| Standalone smoke | Process can launch and survive the smoke harness. | Complete visual correctness or audio quality by ear. |
| CLI/render | Headless state/routing/scenario behavior and generated audio evidence. | Live DAW scheduling or physical Daisy timing. |
| VST3 build | Plugin artifact is generated. | Successful loading in every DAW. |
| Firmware `make` | A firmware target compiles/links. | Flash success, real-time margin, analog behavior. |
| Flash | Image reaches hardware. | Audio quality, control ergonomics, EMC, or timing margin. |
| Bench/listening | The measured/listened condition. | Conditions that were not exercised. |

The most recent repository tracker records a green Release host gate of `337/337` for the
2026-08-13 `pedal_multidelay` publication pass. That result is historical evidence from the
recorded checkout; documentation-only edits do not automatically make it a fresh test run.

## 14. Source map

| Concern | Primary source |
|---|---|
| App abstraction | `include/daisyhost/HostedAppCore.h` |
| App inventory/factory | `src/AppRegistry.cpp` |
| Board profiles | `include/daisyhost/BoardProfile.h`, `src/BoardProfile.cpp` |
| Field bindings | `include/daisyhost/BoardControlMapping.h`, `src/BoardControlMapping.cpp` |
| Rack topology | `include/daisyhost/LiveRackTopology.h`, `src/LiveRackTopology.cpp` |
| Session state | `include/daisyhost/HostSessionState.h`, `src/HostSessionState.cpp` |
| Automation | `src/HostAutomationBridge.cpp` |
| Modulation | `include/daisyhost/HostModulation.h`, `src/HostModulation.cpp` |
| Live processor | `src/juce/DaisyHostPluginProcessor.*` |
| Live editor | `src/juce/DaisyHostPluginEditor.*` |
| Readback | `include/daisyhost/EffectiveHostStateSnapshot.h`, `src/EffectiveHostStateSnapshot.cpp` |
| Offline render | `src/RenderRuntime.cpp`, `tools/render_app.cpp` |
| CLI | `src/CliPayloads.cpp`, `tools/cli_app.cpp` |
| Hub | `src/hub/` |
| Compact delay DSP | `include/daisyhost/PedalDelayEngine.h`, `src/PedalDelayEngine.cpp` |
| Compact delay app | `include/daisyhost/apps/PedalDelayCore.h`, `src/apps/PedalDelayCore.cpp` |
| Tests | `tests/` |
| Scenario/dataset tooling | `training/` |

## 15. Non-goals of this architecture

The current architecture does not claim:

- full STM32/libDaisy emulation;
- arbitrary firmware-source translation;
- arbitrary multi-node graph editing;
- mixed-board racks;
- Patch SM / Pod / custom-board runtime parity;
- Daisy target timing from desktop host timing;
- analog I/O, CV voltage, noise, headroom, EMI/EMC, or mechanical behavior from the UI model.

Those are intentionally separate engineering problems. Keeping the boundary explicit makes
DaisyHost useful instead of letting it become a desktop-shaped source of accidental hardware
claims.
