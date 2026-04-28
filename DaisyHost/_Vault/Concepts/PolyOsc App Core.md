---
title: PolyOsc App Core
aliases: ["polyosc", "polyosc hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# PolyOsc App Core

PolyOsc imports the original Patch PolyOsc behavior into a portable host-side core with oscillator frequency controls, waveform selection, Patch outputs, and Field K5 waveform mapping.

## Source Evidence

- `src/apps/PolyOscCore.cpp`
- `src/DaisyPolyOscCore.cpp`

## Explicit Relationships

- [[PolyOsc App Core]] -- implements -> [[Hosted App Core Contract]]
- [[PolyOsc App Core]] -- is registered as app id -> [[polyosc]]
- [[PolyOsc App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[Field Surface Mapping]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
