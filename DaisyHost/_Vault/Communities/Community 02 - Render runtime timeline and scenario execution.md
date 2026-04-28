---
title: Community 02 - Render runtime timeline and scenario execution
community_id: 2
node_count: 100
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 02 - Render runtime timeline and scenario execution

This note gives `Community 2` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Render runtime timeline and scenario execution groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\RenderRuntime.cpp`: 66 graph nodes
- `src\TestInputSignal.cpp`: 8 graph nodes
- `src\MidiLearnMap.cpp`: 6 graph nodes
- `tests\test_render_runtime.cpp`: 6 graph nodes
- `tools\render_app.cpp`: 3 graph nodes
- `include\daisyhost\AppRegistry.h`: 2 graph nodes
- `src\AppRegistry.cpp`: 2 graph nodes
- `src\juce\DaisyHostPluginProcessor.cpp`: 2 graph nodes
- `tests\test_host_session_state.cpp`: 2 graph nodes
- `tests\test_test_input_signal.cpp`: 2 graph nodes

## High-Degree Nodes

- `RenderRuntime.cpp`: 65 graph edges, source `src\RenderRuntime.cpp`
- `RunMultiNodeRenderScenario()`: 37 graph edges, source `src\RenderRuntime.cpp`
- `RunRenderScenario()`: 35 graph edges, source `src\RenderRuntime.cpp`
- `ValidateScenario()`: 14 graph edges, source `src\RenderRuntime.cpp`
- `ValidateMultiNodeScenario()`: 14 graph edges, source `src\RenderRuntime.cpp`
- `Assign()`: 13 graph edges, source `src\MidiLearnMap.cpp`
- `ResolveMultiNodeScenario()`: 13 graph edges, source `src\RenderRuntime.cpp`
- `CreateHostedAppCore()`: 12 graph edges, source `src\AppRegistry.cpp`
- `ParseRenderScenarioJson()`: 12 graph edges, source `src\RenderRuntime.cpp`
- `ClampTestInputSignalMode()`: 11 graph edges, source `src\TestInputSignal.cpp`
- `GenerateSyntheticTestInput()`: 10 graph edges, source `src\TestInputSignal.cpp`
- `ParseTimelineEvent()`: 9 graph edges, source `src\RenderRuntime.cpp`
- `SerializeRenderManifestJson()`: 9 graph edges, source `src\RenderRuntime.cpp`
- `BuildFieldSurfaceSnapshotForRender()`: 8 graph edges, source `src\RenderRuntime.cpp`
- `TEST()`: 8 graph edges, source `tests\test_render_runtime.cpp`
- `ToLower()`: 7 graph edges, source `src\RenderRuntime.cpp`
- `IsFinite()`: 7 graph edges, source `src\RenderRuntime.cpp`
- `ResolveEventNodeIndex()`: 7 graph edges, source `src\RenderRuntime.cpp`
- `ValidateCommonRenderScenario()`: 7 graph edges, source `src\RenderRuntime.cpp`
- `ResolveFieldSurfaceControlBinding()`: 7 graph edges, source `src\RenderRuntime.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 43 cross-community edges
- [[Community 00 - JUCE processor editor and live host surface]]: 29 cross-community edges
- [[Community 13 - Board control mapping Field controls and keyboard MIDI]]: 7 cross-community edges
- [[Community 08 - CLI payload serialization and JSON contracts]]: 4 cross-community edges
- [[Community 15 - Host modulation lanes and external control safety]]: 3 cross-community edges
- [[Community 23 - DaisyHostCLI command shell and gate command wiring]]: 3 cross-community edges
- [[Community 22 - Live rack topology route plan contract]]: 2 cross-community edges
- [[Community 14 - Hub support launch plans and activity dispatch]]: 1 cross-community edges
- [[Community 16 - Board profiles Patch Field layout and editor policy]]: 1 cross-community edges
- [[Community 06 - PolyOsc portable core and hosted app wrapper]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Render runtime timeline and scenario execution` instead of `Community 2`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
