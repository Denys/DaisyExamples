---
title: Subharmoniq App Core
aliases: ["subharmoniq", "subharmoniq hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# Subharmoniq App Core

Subharmoniq is a hosted and Field-oriented app with six oscillator sources, two four-step sequencers, rhythm dividers, quantize modes, Field controls, and internal tempo-clock behavior.

## Source Evidence

- `src/apps/SubharmoniqCore.cpp`
- `src/DaisySubharmoniqCore.cpp`

## Explicit Relationships

- [[Subharmoniq App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Subharmoniq App Core]] -- is registered as app id -> [[subharmoniq]]
- [[Subharmoniq App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[Field Surface Mapping]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
