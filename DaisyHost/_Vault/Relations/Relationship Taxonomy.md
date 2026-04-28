---
title: Relationship Taxonomy
source: graphify annotation refinement
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/relations"]
---

# Relationship Taxonomy

The AST graph mostly emits generic structural relations. Use these domain relations when creating semantic graph fragments from DaisyHost notes.

| Generic relation in AST graph | Higher-value semantic relation | DaisyHost example |
|---|---|---|
| `contains` | `defines contract for` | `HostedAppCore.h` defines the hosted app contract. |
| `contains` | `serializes payload for` | `CliPayloads.cpp` serializes app, board, snapshot, render, and gate payloads. |
| `calls` | `runs validation through` | `build_host.cmd` runs the full host verification gate. |
| `calls` | `routes event to` | Render timeline events route to the selected target node. |
| `calls` | `builds route plan for` | `LiveRackTopology.cpp` builds route plans for live and render consumers. |
| `imports/includes` | `depends on contract` | `DaisyHostPluginProcessor.cpp` depends on host session, app core, board profile, and snapshot contracts. |
| inferred proximity | `shares canonical parameter model with` | Hosted app wrappers share canonical parameter ids with automation, menu, drawer, and render. |
| inferred proximity | `records evidence for` | Render manifests record evidence for Field controls, routes, nodes, and timeline targeting. |

## Confidence Rules

- Use `EXTRACTED` when a source file or local doc explicitly states the relationship.
- Use `INFERRED` when the relationship is supported by multiple source references but not written as a direct statement.
- Use `AMBIGUOUS` when naming or direction is uncertain.
