---
title: Community 25 - Standalone UI mute banner policy
community_id: 25
node_count: 7
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 25 - Standalone UI mute banner policy

This note gives `Community 25` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Standalone UI mute banner policy groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\StandaloneUiPolicy.cpp`: 5 graph nodes
- `tests\test_standalone_ui_policy.cpp`: 2 graph nodes

## High-Degree Nodes

- `IsStandaloneMuteBannerCandidate()`: 7 graph edges, source `src\StandaloneUiPolicy.cpp`
- `StandaloneUiPolicy.cpp`: 5 graph edges, source `src\StandaloneUiPolicy.cpp`
- `AdjustEditorBoundsForHiddenMuteBanner()`: 4 graph edges, source `src\StandaloneUiPolicy.cpp`
- `TEST()`: 3 graph edges, source `tests\test_standalone_ui_policy.cpp`
- `RightOf()`: 2 graph edges, source `src\StandaloneUiPolicy.cpp`
- `BottomOf()`: 2 graph edges, source `src\StandaloneUiPolicy.cpp`
- `test_standalone_ui_policy.cpp`: 1 graph edges, source `tests\test_standalone_ui_policy.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 4 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Standalone UI mute banner policy` instead of `Community 25`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
