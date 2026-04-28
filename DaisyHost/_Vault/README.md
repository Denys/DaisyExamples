---
title: DaisyHost Semantic Vault
source: graphify refinement pass
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/platform"]
---

# DaisyHost Semantic Vault

This vault is a source-grounded companion corpus for graphify.

It exists because the first AST graph was structurally useful but too generic in two visible ways:

- Edge labels such as `contains [EXTRACTED]` and `calls [EXTRACTED]` do not explain product meaning.
- Communities appeared as recurrent `Community X` labels instead of named DaisyHost domains.

The vault adds explicit domain names, concepts, relationships, source references, and graph extraction hints so a later semantic graphify pass can produce a more detailed graph.

## Start Here

- [[Graphify/Annotation Findings]]
- [[Graphify/Graphify Refinement Objective]]
- [[Graphify/Community Naming Map]]
- [[Relations/Relationship Taxonomy]]
- [[Relations/Graph Seed Triples]]
- [[Concepts/DaisyHost Platform]]
- [[Architecture Map]]

## Source Grounding

- `README.md`: workspace overview, scope, supported hosted apps, build workflow, architecture map
- `CHECKPOINT.md`: current verified state, dated caveats, validation history, behavior contracts
- `PROJECT_TRACKER.md`: active work order, implementation ledger, test evidence, next work decisions
- `AGENTS.md`: agent workflow, ownership slices, mandatory per-iteration testing, completion gate
- `CHANGELOG.md`: durable release and workflow history
- `WORKSTREAM_TRACKER.md`: post-WS7 portfolio tracker and recommender input
- `graphify-out/GRAPH_REPORT.md`: AST graph summary, god nodes, community hubs, suggested graph questions

## Current Graph Baseline

- Graph output: `graphify-out/graph.json`
- Interactive view: `graphify-out/graph.html`
- Report: `graphify-out/GRAPH_REPORT.md`
- AST graph baseline: 1559 nodes, 3873 edges, 76 communities
- Semantic extraction cost for this vault pass: 0 graphify model tokens
