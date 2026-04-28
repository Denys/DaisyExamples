---
title: Annotation Findings
source: browser annotations on graphify-out/graph.html
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/graphify"]
---

# Annotation Findings

## Comment 1: Generic Extracted Edge Labels

The selected graph label `contains [EXTRACTED]` is correct but low-information structural content.

A refined graph needs named semantic relationships such as:

- `processor owns selected-node runtime state`
- `render runtime reuses live rack route plan`
- `board profile drives editor surface policy`
- `hosted app exposes canonical parameters`
- `CLI payload serializes debugState`

## Comment 2: Generic Community Names

The selected legend item `Community 1 124` shows that graphify grouped related nodes but did not know their domain label.

The vault maps community ids to DaisyHost concepts in [[Community Naming Map]] and creates one note per community under [[../Communities]].

## Refinement Decision

Generated `graphify-out/graph.json` should stay reproducible. The semantic refinement lives in `_Vault` as markdown source material with explicit concept names, source references, and relation triples.
