---
title: Community 15 - Host modulation lanes and external control safety
community_id: 15
node_count: 28
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 15 - Host modulation lanes and external control safety

This note gives `Community 15` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Host modulation lanes and external control safety groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\HostModulation.cpp`: 13 graph nodes
- `src\HostSessionState.cpp`: 6 graph nodes
- `src\HostModulationUiText.cpp`: 5 graph nodes
- `src\juce\DaisyHostPluginProcessor.cpp`: 4 graph nodes

## High-Degree Nodes

- `HostModulation.cpp`: 12 graph edges, source `src\HostModulation.cpp`
- `ApplyRackNodeModulation()`: 11 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `EvaluateHostModulation()`: 10 graph edges, source `src\HostModulation.cpp`
- `BuildModulationSnapshots()`: 9 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `ApplyDaisyFieldExternalControlSafetyFloor()`: 8 graph edges, source `src\HostModulation.cpp`
- `BuildHostModulationLaneDisplayText()`: 6 graph edges, source `src\HostModulationUiText.cpp`
- `GetModulationLaneDisplayText()`: 6 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `Clamp01()`: 5 graph edges, source `src\HostModulation.cpp`
- `IsHostModulationTargetEligible()`: 5 graph edges, source `src\HostModulation.cpp`
- `HostSessionState.cpp`: 5 graph edges, source `src\HostSessionState.cpp`
- `Deserialize()`: 5 graph edges, source `src\HostSessionState.cpp`
- `SourceIndex()`: 4 graph edges, source `src\HostModulation.cpp`
- `ParameterNormalizedToNative()`: 4 graph edges, source `src\HostModulation.cpp`
- `HostModulationUiText.cpp`: 4 graph edges, source `src\HostModulationUiText.cpp`
- `Serialize()`: 4 graph edges, source `src\HostSessionState.cpp`
- `IsCvSource()`: 3 graph edges, source `src\HostModulation.cpp`
- `IsLfoSource()`: 3 graph edges, source `src\HostModulation.cpp`
- `IsCutoffLikeParameterId()`: 3 graph edges, source `src\HostModulation.cpp`
- `HostModulationSourceToString()`: 3 graph edges, source `src\HostModulation.cpp`
- `ParameterNativeToNormalized()`: 3 graph edges, source `src\HostModulation.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 13 cross-community edges
- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 7 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 3 cross-community edges
- [[Community 14 - Hub support launch plans and activity dispatch]]: 2 cross-community edges
- [[Community 08 - CLI payload serialization and JSON contracts]]: 1 cross-community edges
- [[Community 04 - MultiDelay app core controls menu and meta controllers]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Host modulation lanes and external control safety` instead of `Community 15`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
