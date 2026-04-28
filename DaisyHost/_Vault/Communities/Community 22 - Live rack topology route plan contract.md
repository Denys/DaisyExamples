---
title: Community 22 - Live rack topology route plan contract
community_id: 22
node_count: 19
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 22 - Live rack topology route plan contract

This note gives `Community 22` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Live rack topology route plan contract groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\LiveRackTopology.cpp`: 13 graph nodes
- `tests\test_live_rack_topology.cpp`: 4 graph nodes
- `src\juce\DaisyHostPluginEditor.cpp`: 1 graph nodes
- `src\juce\DaisyHostPluginProcessor.cpp`: 1 graph nodes

## High-Degree Nodes

- `LiveRackTopology.cpp`: 12 graph edges, source `src\LiveRackTopology.cpp`
- `TryBuildLiveRackRoutePlan()`: 11 graph edges, source `src\LiveRackTopology.cpp`
- `TEST()`: 10 graph edges, source `tests\test_live_rack_topology.cpp`
- `BuildLiveRackTopologyConfig()`: 7 graph edges, source `src\LiveRackTopology.cpp`
- `TryMatchSerialPreset()`: 5 graph edges, source `src\LiveRackTopology.cpp`
- `GetLiveRackNodeRoleDisplayLabel()`: 5 graph edges, source `src\LiveRackTopology.cpp`
- `TryInferLiveRackTopologyPreset()`: 5 graph edges, source `src\LiveRackTopology.cpp`
- `ValidateLiveRackTopologyConfig()`: 4 graph edges, source `src\LiveRackTopology.cpp`
- `GetRackNodeRoleLabelCompat()`: 4 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `MakeCanonicalRoutes()`: 3 graph edges, source `src\LiveRackTopology.cpp`
- `GetLiveRackTopologyDisplayLabel()`: 3 graph edges, source `src\LiveRackTopology.cpp`
- `GetRackNodeRoleLabel()`: 3 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `test_live_rack_topology.cpp`: 3 graph edges, source `tests\test_live_rack_topology.cpp`
- `MakeConfig()`: 3 graph edges, source `tests\test_live_rack_topology.cpp`
- `IsSupportedNodeId()`: 2 graph edges, source `src\LiveRackTopology.cpp`
- `TryParseAudioSourcePort()`: 2 graph edges, source `src\LiveRackTopology.cpp`
- `TryParseAudioDestPort()`: 2 graph edges, source `src\LiveRackTopology.cpp`
- `MakeEndpoint()`: 2 graph edges, source `src\LiveRackTopology.cpp`
- `MakeRoute()`: 2 graph edges, source `tests\test_live_rack_topology.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 9 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 2 cross-community edges
- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Live rack topology route plan contract` instead of `Community 22`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
