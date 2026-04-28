---
title: Community 21 - Gate diagnostics CLI and blocker classification
community_id: 21
node_count: 20
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/cli-verification"]
---

# Community 21 - Gate diagnostics CLI and blocker classification

This note gives `Community 21` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Gate diagnostics CLI and blocker classification groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\GateDiagnostics.cpp`: 17 graph nodes
- `tests\test_gate_diagnostics.cpp`: 3 graph nodes

## High-Degree Nodes

- `GateDiagnostics.cpp`: 16 graph edges, source `src\GateDiagnostics.cpp`
- `BuildGateDiagnostics()`: 8 graph edges, source `src\GateDiagnostics.cpp`
- `SerializeGateDiagnosticsPayloadJson()`: 6 graph edges, source `src\GateDiagnostics.cpp`
- `ClassifyBlockers()`: 5 graph edges, source `src\GateDiagnostics.cpp`
- `TEST()`: 4 graph edges, source `tests\test_gate_diagnostics.cpp`
- `AddBlocker()`: 3 graph edges, source `src\GateDiagnostics.cpp`
- `StringArrayVar()`: 3 graph edges, source `src\GateDiagnostics.cpp`
- `CtestVar()`: 3 graph edges, source `src\GateDiagnostics.cpp`
- `DefaultGateTargets()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `Tail()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `Lines()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `ContainsCaseSensitive()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `HintForKind()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `ParseCtest()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `BuildPhases()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `FailedPhaseKind()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `PhasesVar()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `BlockersVar()`: 2 graph edges, source `src\GateDiagnostics.cpp`
- `test_gate_diagnostics.cpp`: 2 graph edges, source `tests\test_gate_diagnostics.cpp`
- `ParseJson()`: 2 graph edges, source `tests\test_gate_diagnostics.cpp`

## Neighbor Communities

- No cross-community edge summary found in current graph.

## Extraction Guidance

When this community appears in graphify output, label it as `Gate diagnostics CLI and blocker classification` instead of `Community 21`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
