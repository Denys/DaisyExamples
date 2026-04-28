---
title: Community 14 - Hub support launch plans and activity dispatch
community_id: 14
node_count: 33
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/cli-verification"]
---

# Community 14 - Hub support launch plans and activity dispatch

This note gives `Community 14` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Hub support launch plans and activity dispatch groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\HubSupport.cpp`: 29 graph nodes
- `tests\test_hub_support.cpp`: 3 graph nodes
- `src\AppRegistry.cpp`: 1 graph nodes

## High-Degree Nodes

- `HubSupport.cpp`: 28 graph edges, source `src\HubSupport.cpp`
- `BuildHubLaunchPlan()`: 12 graph edges, source `src\HubSupport.cpp`
- `GetDefaultHostedAppId()`: 11 graph edges, source `src\AppRegistry.cpp`
- `TEST()`: 9 graph edges, source `tests\test_hub_support.cpp`
- `NormalizeHubProfile()`: 8 graph edges, source `src\HubSupport.cpp`
- `LoadHubProfile()`: 6 graph edges, source `src\HubSupport.cpp`
- `ToVar()`: 5 graph edges, source `src\HubSupport.cpp`
- `SerializeHubStartupRequest()`: 5 graph edges, source `src\HubSupport.cpp`
- `LoadAndConsumeHubStartupRequest()`: 5 graph edges, source `src\HubSupport.cpp`
- `ToCompactJsonString()`: 4 graph edges, source `src\HubSupport.cpp`
- `BuildRenderScenarioJson()`: 4 graph edges, source `src\HubSupport.cpp`
- `BuildDatasetJobJson()`: 4 graph edges, source `src\HubSupport.cpp`
- `GetDefaultBoardId()`: 4 graph edges, source `src\HubSupport.cpp`
- `GetDefaultActivityId()`: 4 graph edges, source `src\HubSupport.cpp`
- `GetDefaultHubSupportDirectory()`: 4 graph edges, source `src\HubSupport.cpp`
- `FindBoardRegistration()`: 3 graph edges, source `src\HubSupport.cpp`
- `FindActivityRegistration()`: 3 graph edges, source `src\HubSupport.cpp`
- `IsKnownAppId()`: 3 graph edges, source `src\HubSupport.cpp`
- `HubProfileToVar()`: 3 graph edges, source `src\HubSupport.cpp`
- `SaveHubProfile()`: 3 graph edges, source `src\HubSupport.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 6 cross-community edges
- [[Community 15 - Host modulation lanes and external control safety]]: 2 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 1 cross-community edges
- [[Community 08 - CLI payload serialization and JSON contracts]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Hub support launch plans and activity dispatch` instead of `Community 14`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
