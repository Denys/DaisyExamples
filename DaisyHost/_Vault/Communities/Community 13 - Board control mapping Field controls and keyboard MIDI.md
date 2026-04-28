---
title: Community 13 - Board control mapping Field controls and keyboard MIDI
community_id: 13
node_count: 40
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/board-field"]
---

# Community 13 - Board control mapping Field controls and keyboard MIDI

This note gives `Community 13` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Board control mapping Field controls and keyboard MIDI groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\BoardControlMapping.cpp`: 24 graph nodes
- `tests\test_board_control_mapping.cpp`: 7 graph nodes
- `src\ComputerKeyboardMidi.cpp`: 6 graph nodes
- `src\juce\DaisyHostPluginProcessor.cpp`: 3 graph nodes

## High-Degree Nodes

- `BoardControlMapping.cpp`: 27 graph edges, source `src\BoardControlMapping.cpp`
- `BuildDaisyFieldControlMapping()`: 26 graph edges, source `src\BoardControlMapping.cpp`
- `TEST()`: 16 graph edges, source `tests\test_board_control_mapping.cpp`
- `test_board_control_mapping.cpp`: 6 graph edges, source `tests\test_board_control_mapping.cpp`
- `BuildDaisyFieldPublicParameterList()`: 5 graph edges, source `src\BoardControlMapping.cpp`
- `IsDaisyFieldCvTargetSafe()`: 5 graph edges, source `src\BoardControlMapping.cpp`
- `IsDaisyFieldCvTargetIdSafe()`: 5 graph edges, source `src\BoardControlMapping.cpp`
- `ComputerKeyboardMidi.cpp`: 5 graph edges, source `src\ComputerKeyboardMidi.cpp`
- `NormalizeKey()`: 5 graph edges, source `src\ComputerKeyboardMidi.cpp`
- `LowerCopy()`: 4 graph edges, source `src\BoardControlMapping.cpp`
- `BuildRankedPublicParameters()`: 4 graph edges, source `src\BoardControlMapping.cpp`
- `MakeDaisyFieldKeyControlId()`: 4 graph edges, source `src\BoardControlMapping.cpp`
- `MakeDaisyFieldSwitchControlId()`: 4 graph edges, source `src\BoardControlMapping.cpp`
- `SetFieldDrawerPage()`: 4 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `ContainsText()`: 3 graph edges, source `src\BoardControlMapping.cpp`
- `MakeUnavailableBinding()`: 3 graph edges, source `src\BoardControlMapping.cpp`
- `BuildMirroredParameterIds()`: 3 graph edges, source `src\BoardControlMapping.cpp`
- `DaisyFieldKeyToMidiNote()`: 3 graph edges, source `src\BoardControlMapping.cpp`
- `StepDaisyFieldDrawerPage()`: 3 graph edges, source `src\BoardControlMapping.cpp`
- `KeyToMidiNote()`: 3 graph edges, source `src\ComputerKeyboardMidi.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 26 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 7 cross-community edges
- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 4 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Board control mapping Field controls and keyboard MIDI` instead of `Community 13`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
