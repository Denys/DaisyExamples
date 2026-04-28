---
title: Community 24 - Daisy Field RSP layout helpers
community_id: 24
node_count: 13
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/board-field"]
---

# Community 24 - Daisy Field RSP layout helpers

This note gives `Community 24` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Daisy Field RSP layout helpers groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\DaisyFieldRSPLayout.cpp`: 6 graph nodes
- `tests\test_daisy_field_rsp_layout.cpp`: 5 graph nodes
- `include\daisyhost\DaisyFieldRSPLayout.h`: 2 graph nodes

## High-Degree Nodes

- `BuildDaisyFieldCvGeneratorCardLayout()`: 7 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `TEST()`: 6 graph edges, source `tests\test_daisy_field_rsp_layout.cpp`
- `DaisyFieldRSPLayout.cpp`: 5 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `test_daisy_field_rsp_layout.cpp`: 4 graph edges, source `tests\test_daisy_field_rsp_layout.cpp`
- `Overlaps()`: 4 graph edges, source `tests\test_daisy_field_rsp_layout.cpp`
- `BuildDaisyFieldKeyMappingLegendLayout()`: 3 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `RightOf()`: 3 graph edges, source `tests\test_daisy_field_rsp_layout.cpp`
- `BottomOf()`: 3 graph edges, source `tests\test_daisy_field_rsp_layout.cpp`
- `daisyhost()`: 2 graph edges, source `include\daisyhost\DaisyFieldRSPLayout.h`
- `Inset()`: 2 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `TakeTop()`: 2 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `DropTop()`: 2 graph edges, source `src\DaisyFieldRSPLayout.cpp`
- `DaisyFieldRSPLayout.h`: 1 graph edges, source `include\daisyhost\DaisyFieldRSPLayout.h`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 2 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Daisy Field RSP layout helpers` instead of `Community 24`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
