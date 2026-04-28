---
title: DaisyHost Platform
aliases: ["DaisyHost", "host-side Daisy platform"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/platform"]
---

# DaisyHost Platform

DaisyHost is a Windows-first JUCE and CMake host for Daisy app cores. Its current product shape is a visible two-node virtual Daisy Patch rack with standalone, VST3, hub, CLI, render, and dataset workflows.

## Source Evidence

- `README.md`
- `CHECKPOINT.md`

## Explicit Relationships

- [[DaisyHost Platform]] -- uses -> [[JUCE plugin and standalone shell]]
- [[DaisyHost Platform]] -- runs -> [[two hosted nodes at a time]]
- [[DaisyHost Platform]] -- builds with -> [[CMake and build_host.cmd]]

## Related Concepts

- [[Hosted App Core Contract]]
- [[Live Rack Runtime]]
- [[Render Runtime]]
- [[Board Profile System]]
- [[DaisyHostCLI Diagnostics]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
