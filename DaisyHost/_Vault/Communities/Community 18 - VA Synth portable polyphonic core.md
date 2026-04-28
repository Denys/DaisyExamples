---
title: Community 18 - VA Synth portable polyphonic core
community_id: 18
node_count: 27
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/app"]
---

# Community 18 - VA Synth portable polyphonic core

This note gives `Community 18` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

VA Synth portable polyphonic core groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\DaisyVASynthCore.cpp`: 27 graph nodes

## High-Degree Nodes

- `DaisyVASynthCore.cpp`: 31 graph edges, source `src\DaisyVASynthCore.cpp`
- `Process()`: 9 graph edges, source `src\DaisyVASynthCore.cpp`
- `Clamp01()`: 8 graph edges, source `src\DaisyVASynthCore.cpp`
- `GetWaveLabel()`: 5 graph edges, source `src\DaisyVASynthCore.cpp`
- `WaveIndex()`: 4 graph edges, source `src\DaisyVASynthCore.cpp`
- `AttackSeconds()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `ReleaseSeconds()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `DetuneSemitones()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `SetParameterValue()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `TriggerMomentaryAction()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `Panic()`: 3 graph edges, source `src\DaisyVASynthCore.cpp`
- `MidiNoteToFrequency()`: 2 graph edges, source `src\DaisyVASynthCore.cpp`
- `WaveLabel()`: 2 graph edges, source `src\DaisyVASynthCore.cpp`
- `RenderWave()`: 2 graph edges, source `src\DaisyVASynthCore.cpp`
- `SetEffectiveParameterValue()`: 2 graph edges, source `src\DaisyVASynthCore.cpp`
- `RestoreStatefulParameterValues()`: 2 graph edges, source `src\DaisyVASynthCore.cpp`
- `MakeDefaultParameters()`: 1 graph edges, source `src\DaisyVASynthCore.cpp`
- `DaisyVASynthCore()`: 1 graph edges, source `src\DaisyVASynthCore.cpp`
- `Prepare()`: 1 graph edges, source `src\DaisyVASynthCore.cpp`
- `ResetToDefaultState()`: 1 graph edges, source `src\DaisyVASynthCore.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 10 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `VA Synth portable polyphonic core` instead of `Community 18`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
