---
title: Module - src\juce\DaisyHostPluginProcessor.cpp
source_file: src\juce\DaisyHostPluginProcessor.cpp
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/module", "daisyhost/runtime"]
---

# Module - `src\juce\DaisyHostPluginProcessor.cpp`

This module note was generated from the graphify AST graph. It gives a source file its own graph-friendly document so future semantic extraction can connect file-level roles to symbols and communities.

## Community Membership

- [[../Communities/Community 00 - JUCE processor editor and live host surface]]: 172 nodes
- [[../Communities/Community 01 - Effective state snapshot and hosted synth lifecycle]]: 5 nodes
- [[../Communities/Community 15 - Host modulation lanes and external control safety]]: 4 nodes
- [[../Communities/Community 13 - Board control mapping Field controls and keyboard MIDI]]: 3 nodes
- [[../Communities/Community 02 - Render runtime timeline and scenario execution]]: 2 nodes
- [[../Communities/Community 22 - Live rack topology route plan contract]]: 1 nodes

## Prominent Symbols

- `DaisyHostPluginProcessor.cpp`: 186 graph edges
- `processBlock()`: 32 graph edges
- `RefreshCoreStateFromIdleHostChange()`: 32 graph edges
- `LoadSession()`: 26 graph edges
- `prepareToPlay()`: 20 graph edges
- `BuildActiveFieldControlMapping()`: 18 graph edges
- `SetMenuItemValue()`: 18 graph edges
- `Clamp01()`: 17 graph edges
- `BuildFieldSurfaceSnapshot()`: 13 graph edges
- `getStateInformation()`: 12 graph edges
- `SetFieldSwitchPressed()`: 12 graph edges
- `FlushSelectedNodeStateToRack()`: 11 graph edges
- `SyncSelectedNodeStateFromRack()`: 11 graph edges
- `ApplyRackNodeModulation()`: 11 graph edges
- `RecreateRackNode()`: 11 graph edges
- `UpdateRackNodeSnapshots()`: 10 graph edges
- `IsDaisyFieldBoard()`: 10 graph edges
- `TryApplyBoardId()`: 9 graph edges
- `BuildModulationSnapshots()`: 9 graph edges
- `SetRackNodeAppId()`: 9 graph edges
- `SetModulationLane()`: 9 graph edges
- `ApplyCanonicalSessionStateToCore()`: 9 graph edges
- `UpdateCoreSnapshots()`: 9 graph edges
- `GetEligibleModulationDestinations()`: 8 graph edges
- `SetFieldKeyPressed()`: 8 graph edges

## Relationship Types Observed

- `calls`: 477 edges
- `contains`: 186 edges
- `method`: 1 edges

## Extraction Guidance

Add a concise human-written role for this module before future full semantic extraction if this file becomes a focus area. The current AST graph can say what symbols exist; this vault should say what job the module performs in DaisyHost.
