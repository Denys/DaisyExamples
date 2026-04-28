---
title: Board Profile System
aliases: ["BoardProfile", "Patch and Field board profiles"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/board-field"]
---

# Board Profile System

BoardProfile centralizes board-aware surface metadata for daisy_patch and daisy_field: controls, ports, text, decorations, indicators, editor policy, and selected-node hints.

## Source Evidence

- `src/BoardProfile.cpp`
- `include/daisyhost/BoardProfile.h`
- `README.md`

## Explicit Relationships

- [[Board Profile System]] -- creates -> [[daisy_patch profile]]
- [[Board Profile System]] -- creates -> [[daisy_field profile]]
- [[Board Profile System]] -- drives -> [[JUCE Editor Surface]]

## Related Concepts

- [[Field Surface Mapping]]
- [[JUCE Editor Surface]]
- [[Live Rack Runtime]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
