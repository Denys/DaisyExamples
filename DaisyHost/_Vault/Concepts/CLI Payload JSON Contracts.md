---
title: CLI Payload JSON Contracts
aliases: ["CliPayloads", "debugState JSON"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/cli-verification"]
---

# CLI Payload JSON Contracts

CliPayloads serializes apps, boards, snapshots, render results, routes, selected-node target cues, debugState, and gate payload structures into JSON for external agent, QA, render, and CI workflows.

## Source Evidence

- `src/CliPayloads.cpp`
- `include/daisyhost/CliPayloads.h`
- `README.md`

## Explicit Relationships

- [[CLI Payload JSON Contracts]] -- serializes -> [[effective state snapshots]]
- [[CLI Payload JSON Contracts]] -- serializes -> [[render results]]
- [[CLI Payload JSON Contracts]] -- supports -> [[agent diagnostics]]

## Related Concepts

- [[Render Runtime]]
- [[Effective Host State Snapshot]]
- [[DaisyHostCLI Diagnostics]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
