---
title: Effective Host State Snapshot
aliases: ["EffectiveHostStateSnapshot", "live state readback"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/runtime"]
---

# Effective Host State Snapshot

EffectiveHostStateSnapshot exposes live canonical parameters, mapped automation slots, selected-node identity, board and topology fields, active-node MetaControllers, node summaries, routes, CV state, gate state, and current audio-input state.

## Source Evidence

- `src/EffectiveHostStateSnapshot.cpp`
- `include/daisyhost/EffectiveHostStateSnapshot.h`
- `README.md`

## Explicit Relationships

- [[Effective Host State Snapshot]] -- reads from -> [[DaisyHost processor state]]
- [[Effective Host State Snapshot]] -- feeds -> [[debugState JSON]]
- [[Effective Host State Snapshot]] -- captures -> [[selected-node context]]

## Related Concepts

- [[CLI Payload JSON Contracts]]
- [[Field Surface Mapping]]
- [[Live Rack Runtime]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
