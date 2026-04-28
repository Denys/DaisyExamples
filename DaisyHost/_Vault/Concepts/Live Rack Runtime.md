---
title: Live Rack Runtime
aliases: ["two-node rack", "live rack topology"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/runtime"]
---

# Live Rack Runtime

The live rack runs exactly node0 and node1 and supports four audio-only topology presets: node0_only, node1_only, node0_to_node1, and node1_to_node0. Selected-node targeting directs Patch controls, Field controls, CV/gate/test inputs, keyboard MIDI, and automation to the active node.

## Source Evidence

- `src/LiveRackTopology.cpp`
- `include/daisyhost/LiveRackTopology.h`
- `README.md`

## Explicit Relationships

- [[Live Rack Runtime]] -- serializes through -> [[Host Session State]]
- [[Live Rack Runtime]] -- is validated by -> [[LiveRackTopology tests]]
- [[Live Rack Runtime]] -- routes audio through -> [[route plan contract]]

## Related Concepts

- [[Render Runtime]]
- [[Host Session State]]
- [[Effective Host State Snapshot]]
- [[Board Profile System]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
