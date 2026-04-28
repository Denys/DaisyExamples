---
title: MultiDelay App Core
aliases: ["multidelay", "multidelay hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# MultiDelay App Core

MultiDelay is the default regression fixture. It exposes the final Patch control hierarchy, OLED parameter menu access for all DSP parameters, last-touch-wins arbitration, and named Blend, Space, and Regen MetaControllers.

## Source Evidence

- `src/apps/MultiDelayCore.cpp`
- `include/daisyhost/apps/MultiDelayCore.h`

## Explicit Relationships

- [[MultiDelay App Core]] -- implements -> [[Hosted App Core Contract]]
- [[MultiDelay App Core]] -- is registered as app id -> [[multidelay]]
- [[MultiDelay App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
