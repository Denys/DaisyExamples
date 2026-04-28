---
title: Community 29 - Host startup policy
community_id: 29
node_count: 4
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 29 - Host startup policy

This note gives `Community 29` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Host startup policy groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\HostStartupPolicy.cpp`: 2 graph nodes
- `tests\test_host_startup_policy.cpp`: 2 graph nodes

## High-Degree Nodes

- `ResolveStartupTestInputMode()`: 3 graph edges, source `src\HostStartupPolicy.cpp`
- `TEST()`: 2 graph edges, source `tests\test_host_startup_policy.cpp`
- `HostStartupPolicy.cpp`: 1 graph edges, source `src\HostStartupPolicy.cpp`
- `test_host_startup_policy.cpp`: 1 graph edges, source `tests\test_host_startup_policy.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Host startup policy` instead of `Community 29`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
