---
title: Braids App Core
aliases: ["braids", "braids hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# Braids App Core

Braids is a percussion-first hosted app with a portable DaisyBraidsCore wrapper, six model subset, Drum and Finish pages, MIDI/gate triggering, and utility actions for audition/randomize/panic.

## Source Evidence

- `src/apps/BraidsCore.cpp`
- `src/DaisyBraidsCore.cpp`

## Explicit Relationships

- [[Braids App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Braids App Core]] -- is registered as app id -> [[braids]]
- [[Braids App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
