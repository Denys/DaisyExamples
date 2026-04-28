---
title: Community 06 - PolyOsc portable core and hosted app wrapper
community_id: 6
node_count: 65
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/app"]
---

# Community 06 - PolyOsc portable core and hosted app wrapper

This note gives `Community 6` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

PolyOsc portable core and hosted app wrapper groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\apps\PolyOscCore.cpp`: 43 graph nodes
- `src\DaisyPolyOscCore.cpp`: 22 graph nodes

## High-Degree Nodes

- `PolyOscCore.cpp`: 42 graph edges, source `src\apps\PolyOscCore.cpp`
- `DaisyPolyOscCore.cpp`: 21 graph edges, source `src\DaisyPolyOscCore.cpp`
- `RefreshSnapshots()`: 13 graph edges, source `src\apps\PolyOscCore.cpp`
- `BuildMenuModel()`: 9 graph edges, source `src\apps\PolyOscCore.cpp`
- `GetWaveformLabel()`: 6 graph edges, source `src\DaisyPolyOscCore.cpp`
- `GetPatchBindings()`: 6 graph edges, source `src\apps\PolyOscCore.cpp`
- `SetControl()`: 6 graph edges, source `src\apps\PolyOscCore.cpp`
- `SetParameterValue()`: 6 graph edges, source `src\apps\PolyOscCore.cpp`
- `StripParameterId()`: 6 graph edges, source `src\apps\PolyOscCore.cpp`
- `Clamp01()`: 5 graph edges, source `src\DaisyPolyOscCore.cpp`
- `SetParameterValue()`: 5 graph edges, source `src\DaisyPolyOscCore.cpp`
- `GetWaveformIndex()`: 5 graph edges, source `src\DaisyPolyOscCore.cpp`
- `PolyOscCore()`: 5 graph edges, source `src\apps\PolyOscCore.cpp`
- `SetMenuItemValue()`: 5 graph edges, source `src\apps\PolyOscCore.cpp`
- `BuildDisplay()`: 5 graph edges, source `src\apps\PolyOscCore.cpp`
- `WaveformIndexFromNormalized()`: 4 graph edges, source `src\DaisyPolyOscCore.cpp`
- `Process()`: 4 graph edges, source `src\DaisyPolyOscCore.cpp`
- `GetOscillatorFrequencyHz()`: 4 graph edges, source `src\DaisyPolyOscCore.cpp`
- `Process()`: 4 graph edges, source `src\apps\PolyOscCore.cpp`
- `GetParameterValue()`: 4 graph edges, source `src\apps\PolyOscCore.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 3 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 1 cross-community edges
- [[Community 00 - JUCE processor editor and live host surface]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `PolyOsc portable core and hosted app wrapper` instead of `Community 6`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
