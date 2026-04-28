---
title: Harmoniqs App Core
aliases: ["harmoniqs", "harmoniqs hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# Harmoniqs App Core

Harmoniqs is an additive hosted app with Spectrum and Envelope pages, harmonic lanes, MIDI/gate triggering, and utility actions for audition/init/randomize/panic.

## Source Evidence

- `src/apps/HarmoniqsCore.cpp`
- `src/DaisyHarmoniqsCore.cpp`

## Explicit Relationships

- [[Harmoniqs App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Harmoniqs App Core]] -- is registered as app id -> [[harmoniqs]]
- [[Harmoniqs App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
