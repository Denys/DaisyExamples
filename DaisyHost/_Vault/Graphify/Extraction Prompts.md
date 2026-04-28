---
title: Extraction Prompts
source: graphify refinement pass
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/graphify"]
---

# Extraction Prompts

## Community Labeling Prompt

Read `_Vault/Graphify/Community Naming Map.md` and rename graph communities using the `Domain name` column. Preserve the original community id as metadata.

## Edge Refinement Prompt

Read `_Vault/Relations/Relationship Taxonomy.md` and `_Vault/Relations/Graph Seed Triples.md`. Convert generic AST relationships into source-backed semantic edges only when the vault or source file supports the relation.

## Architecture Prompt

Read `_Vault/Architecture Map.md` and the linked concept notes. Extract product-level nodes for host runtime, live rack, render runtime, board profile, Field mapping, CLI diagnostics, build gate, and each hosted app.
