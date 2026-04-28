---
title: Hosted App Core Contract
aliases: ["HostedAppCore", "shared app core contract"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# Hosted App Core Contract

HostedAppCore is the shared abstraction that lets supported apps expose capabilities, parameters, menu models, display models, patch bindings, MIDI/gate handling, and audio processing through one host-facing shape.

## Source Evidence

- `include/daisyhost/HostedAppCore.h`
- `README.md`

## Explicit Relationships

- [[Hosted App Core Contract]] -- is implemented by -> [[supported hosted app cores]]
- [[Hosted App Core Contract]] -- feeds -> [[DaisyHost processor/editor binding]]
- [[Hosted App Core Contract]] -- defines -> [[canonical parameters and menu model]]

## Related Concepts

- [[MultiDelay App Core]]
- [[CloudSeed App Core]]
- [[Braids App Core]]
- [[Harmoniqs App Core]]
- [[VA Synth App Core]]
- [[PolyOsc App Core]]
- [[Subharmoniq App Core]]
- [[Torus App Core]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
