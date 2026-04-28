---
title: Community 16 - Board profiles Patch Field layout and editor policy
community_id: 16
node_count: 28
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/board-field"]
---

# Community 16 - Board profiles Patch Field layout and editor policy

This note gives `Community 16` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Board profiles Patch Field layout and editor policy groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `tests\test_board_profile.cpp`: 14 graph nodes
- `src\BoardProfile.cpp`: 12 graph nodes
- `include\daisyhost\BoardProfile.h`: 2 graph nodes

## High-Degree Nodes

- `TEST()`: 15 graph edges, source `tests\test_board_profile.cpp`
- `test_board_profile.cpp`: 13 graph edges, source `tests\test_board_profile.cpp`
- `BoardProfile.cpp`: 11 graph edges, source `src\BoardProfile.cpp`
- `TryCreateBoardProfile()`: 11 graph edges, source `src\BoardProfile.cpp`
- `MakeDaisyPatchProfile()`: 8 graph edges, source `src\BoardProfile.cpp`
- `MakeDaisyFieldProfile()`: 8 graph edges, source `src\BoardProfile.cpp`
- `GetSupportedBoardIds()`: 4 graph edges, source `src\BoardProfile.cpp`
- `Overlaps()`: 4 graph edges, source `tests\test_board_profile.cpp`
- `BoardEditorTraceMode()`: 3 graph edges, source `include\daisyhost\BoardProfile.h`
- `CreateBoardProfile()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakePort()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakeControl()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakeSurfaceControl()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakeDecoration()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakeText()`: 3 graph edges, source `src\BoardProfile.cpp`
- `MakeIndicator()`: 2 graph edges, source `src\BoardProfile.cpp`
- `FindSurfaceControl()`: 2 graph edges, source `tests\test_board_profile.cpp`
- `FindControl()`: 2 graph edges, source `tests\test_board_profile.cpp`
- `FindDecoration()`: 2 graph edges, source `tests\test_board_profile.cpp`
- `FindIndicator()`: 2 graph edges, source `tests\test_board_profile.cpp`

## Neighbor Communities

- [[Community 08 - CLI payload serialization and JSON contracts]]: 3 cross-community edges
- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 1 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 1 cross-community edges
- [[Community 00 - JUCE processor editor and live host surface]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Board profiles Patch Field layout and editor policy` instead of `Community 16`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
