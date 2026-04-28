---
title: Field Surface Mapping
aliases: ["Daisy Field controls", "Field native controls"]
source: DaisyHost vault semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/concept", "daisyhost/board-field"]
---

# Field Surface Mapping

Field host support maps K1-K4 to selected-node Patch page bindings, K5-K8 to ranked extra automatable parameters, CV1-CV4 and Gate In to host CV/gate paths, A1-B8 to MIDI notes, SW1/SW2 to selected-app utility actions, and CV OUT/LED indicators to derived monitor state.

## Source Evidence

- `src/BoardControlMapping.cpp`
- `src/DaisyFieldRSPLayout.cpp`
- `README.md`

## Explicit Relationships

- [[Field Surface Mapping]] -- depends on -> [[Board Profile System]]
- [[Field Surface Mapping]] -- targets -> [[selected live rack node]]
- [[Field Surface Mapping]] -- records evidence in -> [[snapshots and render manifests]]

## Related Concepts

- [[Board Profile System]]
- [[Effective Host State Snapshot]]
- [[Host Modulation Lanes]]

## Graphify Extraction Notes

Create this as a named concept node. Prefer semantic edges from the explicit relationships section over generic containment edges.
