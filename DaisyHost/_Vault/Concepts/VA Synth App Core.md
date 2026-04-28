---
title: VA Synth App Core
aliases: ["vasynth", "vasynth hosted app"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/app"]
---

# VA Synth App Core

VA Synth is a seven-voice polyphonic hosted app with Osc, Filter, and Motion pages, MIDI-first triggering, gate aliasing, stereo simulation, and utility actions.

## Source Evidence

- `src/apps/VASynthCore.cpp`
- `src/DaisyVASynthCore.cpp`

## Explicit Relationships

- [[VA Synth App Core]] -- implements -> [[Hosted App Core Contract]]
- [[VA Synth App Core]] -- is registered as app id -> [[vasynth]]
- [[VA Synth App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[DaisyHost Platform]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
