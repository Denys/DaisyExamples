---
title: Render Runtime
aliases: ["DaisyHostRender", "offline render runtime"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/runtime"]
---

# Render Runtime

RenderRuntime loads scenario JSON, applies timeline events, renders offline audio, writes audio.wav and manifest.json, and records node-targeted debug readback for rack, node, route, and timeline evidence.

## Source Evidence

- `src/RenderRuntime.cpp`
- `include/daisyhost/RenderRuntime.h`
- `training/README.md`
- `README.md`

## Explicit Relationships

- [[Render Runtime]] -- consumes -> [[scenario JSON]]
- [[Render Runtime]] -- emits -> [[manifest JSON]]
- [[Render Runtime]] -- shares route rules with -> [[Live Rack Runtime]]

## Related Concepts

- [[Live Rack Runtime]]
- [[CLI Payload JSON Contracts]]
- [[Training Dataset Workflow]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
