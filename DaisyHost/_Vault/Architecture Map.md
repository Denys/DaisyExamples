---
title: DaisyHost Architecture Map
source: README.md plus graphify communities
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/platform"]
---

# DaisyHost Architecture Map

DaisyHost is organized around several cooperating surfaces:

- [[Concepts/DaisyHost Platform]] is the product/runtime envelope.
- [[Concepts/Hosted App Core Contract]] is the common app-core interface.
- [[Concepts/Live Rack Runtime]] manages two hosted nodes and route presets.
- [[Concepts/Render Runtime]] runs offline scenario rendering and emits manifests.
- [[Concepts/Board Profile System]] models Patch and Field surfaces.
- [[Concepts/Field Surface Mapping]] maps Field hardware-shaped controls into host controls.
- [[Concepts/CLI Payload JSON Contracts]] and [[Concepts/DaisyHostCLI Diagnostics]] expose agent-readable state.
- [[Concepts/Build Verification Gate]] is the current source-backed verification authority.

## Hosted Apps

- [[Concepts/MultiDelay App Core]]
- [[Concepts/Torus App Core]]
- [[Concepts/CloudSeed App Core]]
- [[Concepts/Braids App Core]]
- [[Concepts/Harmoniqs App Core]]
- [[Concepts/VA Synth App Core]]
- [[Concepts/PolyOsc App Core]]
- [[Concepts/Subharmoniq App Core]]

## Key Graph Communities

- [[Communities/Community 00 - JUCE processor editor and live host surface]]
- [[Communities/Community 02 - Render runtime timeline and scenario execution]]
- [[Communities/Community 08 - CLI payload serialization and JSON contracts]]
- [[Communities/Community 13 - Board control mapping Field controls and keyboard MIDI]]
- [[Communities/Community 16 - Board profiles Patch Field layout and editor policy]]
- [[Communities/Community 22 - Live rack topology route plan contract]]
