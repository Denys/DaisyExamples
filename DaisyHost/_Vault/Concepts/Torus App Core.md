---
title: Torus App Core
aliases: ["torus", "torus hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# Torus App Core

Torus is the first nontrivial second hosted app, implemented as a DaisyHost-native Patch wrapper with Torus-style semantics and menu/control assignment behavior.

## Source Evidence

- `src/apps/TorusCore.cpp`

## Explicit Relationships

- [[Torus App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Torus App Core]] -- is registered as app id -> [[torus]]
- [[Torus App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
