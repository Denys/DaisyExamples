---
title: DaisyHostCLI Diagnostics
aliases: ["gate diagnostics", "CLI gate command"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/cli-verification"]
---

# DaisyHostCLI Diagnostics

DaisyHostCLI exposes machine-readable commands for agents and CI. The gate command wraps build_host.cmd and reports configure, build, ctest phases, targets, CTest totals, blocker classifications, and output tails without changing build semantics.

## Source Evidence

- `src/GateDiagnostics.cpp`
- `tools/cli_app.cpp`
- `PROJECT_TRACKER.md`

## Explicit Relationships

- [[DaisyHostCLI Diagnostics]] -- wraps -> [[build_host.cmd]]
- [[DaisyHostCLI Diagnostics]] -- reports -> [[CTest totals]]
- [[DaisyHostCLI Diagnostics]] -- classifies -> [[known blockers]]

## Related Concepts

- [[CLI Payload JSON Contracts]]
- [[Build Verification Gate]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
