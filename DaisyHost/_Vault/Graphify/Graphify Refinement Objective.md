---
title: Graphify Refinement Objective
source: DaisyHost graphify pass
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/graphify"]
---

# Graphify Refinement Objective

Objective: turn the first DaisyHost AST graph into a highly detailed, source-grounded project knowledge graph.

## What The Current Graph Already Gives

- Structural coverage across C++, Python, PowerShell, tests, and headers.
- God nodes that identify high-connectivity functions and test macros.
- Community detection that reveals natural work areas.
- A reproducible baseline with zero semantic extraction token cost.

## What The Current Graph Lacks

- Domain labels for communities.
- Semantic edge names beyond `contains`, `calls`, and inferred code proximity.
- Product-level relationships across host, render, CLI, Field, firmware adapter, and tests.
- Rationale attributes explaining why contracts exist.

## Vault Strategy

The vault provides semantic seed documents with named communities, explicit triples, source file references, and cross-links between concepts.
