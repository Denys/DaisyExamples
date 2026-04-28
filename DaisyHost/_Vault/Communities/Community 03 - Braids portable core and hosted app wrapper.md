---
title: Community 03 - Braids portable core and hosted app wrapper
community_id: 3
node_count: 90
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/app"]
---

# Community 03 - Braids portable core and hosted app wrapper

This note gives `Community 3` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Braids portable core and hosted app wrapper groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\apps\BraidsCore.cpp`: 50 graph nodes
- `src\DaisyBraidsCore.cpp`: 40 graph nodes

## High-Degree Nodes

- `BraidsCore.cpp`: 49 graph edges, source `src\apps\BraidsCore.cpp`
- `DaisyBraidsCore.cpp`: 39 graph edges, source `src\DaisyBraidsCore.cpp`
- `BuildDisplay()`: 18 graph edges, source `src\apps\BraidsCore.cpp`
- `BuildMenuModel()`: 15 graph edges, source `src\apps\BraidsCore.cpp`
- `RefreshSnapshots()`: 12 graph edges, source `src\apps\BraidsCore.cpp`
- `SetMenuItemValue()`: 11 graph edges, source `src\apps\BraidsCore.cpp`
- `GetPatchBindings()`: 9 graph edges, source `src\apps\BraidsCore.cpp`
- `MenuRotate()`: 8 graph edges, source `src\apps\BraidsCore.cpp`
- `SetParameterValue()`: 7 graph edges, source `src\DaisyBraidsCore.cpp`
- `TriggerMomentaryAction()`: 7 graph edges, source `src\DaisyBraidsCore.cpp`
- `FindParameter()`: 7 graph edges, source `src\DaisyBraidsCore.cpp`
- `BraidsCore()`: 7 graph edges, source `src\apps\BraidsCore.cpp`
- `SetPortInput()`: 7 graph edges, source `src\apps\BraidsCore.cpp`
- `Clamp01()`: 6 graph edges, source `src\DaisyBraidsCore.cpp`
- `QuantizedChoiceNormalized()`: 6 graph edges, source `src\DaisyBraidsCore.cpp`
- `SetControl()`: 6 graph edges, source `src\apps\BraidsCore.cpp`
- `ResetMenuState()`: 6 graph edges, source `src\apps\BraidsCore.cpp`
- `StripParameterId()`: 6 graph edges, source `src\apps\BraidsCore.cpp`
- `SetParameterValue()`: 5 graph edges, source `src\apps\BraidsCore.cpp`
- `GetParameterValue()`: 5 graph edges, source `src\apps\BraidsCore.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 6 cross-community edges
- [[Community 00 - JUCE processor editor and live host surface]]: 4 cross-community edges
- [[Community 05 - Subharmoniq hosted app and MIDI preview support]]: 2 cross-community edges
- [[Community 11 - Harmoniqs hosted app performance model]]: 2 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Braids portable core and hosted app wrapper` instead of `Community 3`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
