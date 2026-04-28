---
title: Community 23 - DaisyHostCLI command shell and gate command wiring
community_id: 23
node_count: 15
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/cli-verification"]
---

# Community 23 - DaisyHostCLI command shell and gate command wiring

This note gives `Community 23` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

DaisyHostCLI command shell and gate command wiring groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `tools\cli_app.cpp`: 15 graph nodes

## High-Degree Nodes

- `cli_app.cpp`: 14 graph edges, source `tools\cli_app.cpp`
- `main()`: 11 graph edges, source `tools\cli_app.cpp`
- `RunDoctor()`: 5 graph edges, source `tools\cli_app.cpp`
- `HasFlag()`: 3 graph edges, source `tools\cli_app.cpp`
- `ReadOption()`: 3 graph edges, source `tools\cli_app.cpp`
- `BuildCommandLine()`: 3 graph edges, source `tools\cli_app.cpp`
- `RunExternalCommand()`: 3 graph edges, source `tools\cli_app.cpp`
- `RunSmokeCommand()`: 3 graph edges, source `tools\cli_app.cpp`
- `PrintUsage()`: 2 graph edges, source `tools\cli_app.cpp`
- `QuoteArgument()`: 2 graph edges, source `tools\cli_app.cpp`
- `StatusJson()`: 2 graph edges, source `tools\cli_app.cpp`
- `PrintPayload()`: 2 graph edges, source `tools\cli_app.cpp`
- `DoctorItem()`: 2 graph edges, source `tools\cli_app.cpp`
- `IsSupportedConfig()`: 1 graph edges, source `tools\cli_app.cpp`
- `PrintGateTextSummary()`: 1 graph edges, source `tools\cli_app.cpp`

## Neighbor Communities

- [[Community 02 - Render runtime timeline and scenario execution]]: 3 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `DaisyHostCLI command shell and gate command wiring` instead of `Community 23`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
