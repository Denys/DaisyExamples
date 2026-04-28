---
title: Obsidian Graph Color Groups
source: graph visual refinement pass
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/graphify"]
---

# Obsidian Graph Color Groups

The vault graph uses tag-backed color groups so the visual model is based on DaisyHost domains rather than graphify's generic `Community X` labels.

| Tag group | Color role | What it means |
|---|---|---|
| `#daisyhost/platform` | yellow | platform overview, index, architecture, and core host concepts |
| `#daisyhost/runtime` | blue | live rack, render runtime, processor/editor, state snapshots, session, MIDI, modulation, and routing |
| `#daisyhost/app` | purple | hosted apps, portable app cores, app-specific communities, and app modules |
| `#daisyhost/board-field` | green | board profiles, Field controls, Field layout, and board-control mapping |
| `#daisyhost/cli-verification` | orange | CLI payloads, gate diagnostics, smoke tests, build wrappers, hub tooling, adapter tooling, and recommender flow |
| `#daisyhost/relations` | red | semantic relationship taxonomy and seed triples |
| `#daisyhost/graphify` | cyan | graphify findings, runbooks, extraction prompts, and source-document indexes |
| `#daisyhost/module` | gray | generated source-module notes |
| `#daisyhost/community` | violet | generated graphify community notes |
| `#daisyhost/concept` | teal | curated concept notes |

## Compare Against graphify HTML

The graphify HTML view colors by detected graph community and labels many edges as structural facts such as `calls [EXTRACTED]`.

The Obsidian graph colors by vault tags and therefore answers a different visual question: which notes are about runtime, apps, board/Field support, CLI/verification, graphify refinement, or semantic relationships.

Use Obsidian when you want domain orientation. Use graphify HTML when you want raw AST graph topology.
