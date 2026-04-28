---
title: Community 01 - Effective state snapshot and hosted synth lifecycle
community_id: 1
node_count: 124
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 01 - Effective state snapshot and hosted synth lifecycle

This note gives `Community 1` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Effective state snapshot and hosted synth lifecycle groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\apps\VASynthCore.cpp`: 45 graph nodes
- `tests\test_multidelay_core.cpp`: 6 graph nodes
- `src\DaisyVASynthCore.cpp`: 5 graph nodes
- `src\juce\DaisyHostPluginProcessor.cpp`: 5 graph nodes
- `tests\test_daisy_braids_core.cpp`: 5 graph nodes
- `tests\test_daisy_cloudseed_core.cpp`: 5 graph nodes
- `tests\test_daisy_harmoniqs_core.cpp`: 5 graph nodes
- `tests\test_daisy_vasynth_core.cpp`: 5 graph nodes
- `src\apps\SubharmoniqCore.cpp`: 4 graph nodes
- `tests\test_torus_core.cpp`: 4 graph nodes

## High-Degree Nodes

- `VASynthCore.cpp`: 45 graph edges, source `src\apps\VASynthCore.cpp`
- `TEST()`: 38 graph edges, source `tests\test_subharmoniq_core.cpp`
- `GetPatchBindings()`: 29 graph edges, source `src\apps\VASynthCore.cpp`
- `ResetToDefaultState()`: 29 graph edges, source `src\apps\VASynthCore.cpp`
- `TEST()`: 28 graph edges, source `tests\test_multidelay_core.cpp`
- `Process()`: 26 graph edges, source `src\apps\VASynthCore.cpp`
- `Prepare()`: 25 graph edges, source `src\apps\VASynthCore.cpp`
- `SetParameterValue()`: 24 graph edges, source `src\apps\VASynthCore.cpp`
- `GetParameterValue()`: 18 graph edges, source `src\apps\VASynthCore.cpp`
- `SetMenuItemValue()`: 18 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `TEST()`: 17 graph edges, source `tests\test_cloudseed_core.cpp`
- `GetAppDisplayName()`: 16 graph edges, source `src\apps\VASynthCore.cpp`
- `SetPortInput()`: 16 graph edges, source `src\apps\VASynthCore.cpp`
- `RestoreStatefulParameterValues()`: 16 graph edges, source `src\apps\VASynthCore.cpp`
- `TEST()`: 16 graph edges, source `tests\test_daisy_cloudseed_core.cpp`
- `GetActivePageBinding()`: 15 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SetControl()`: 15 graph edges, source `src\apps\VASynthCore.cpp`
- `TEST()`: 15 graph edges, source `tests\test_daisy_braids_core.cpp`
- `TEST()`: 15 graph edges, source `tests\test_polyosc_core.cpp`
- `CaptureStatefulParameterValues()`: 14 graph edges, source `src\apps\VASynthCore.cpp`

## Neighbor Communities

- [[Community 00 - JUCE processor editor and live host surface]]: 47 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 43 cross-community edges
- [[Community 05 - Subharmoniq hosted app and MIDI preview support]]: 34 cross-community edges
- [[Community 18 - VA Synth portable polyphonic core]]: 10 cross-community edges
- [[Community 08 - CLI payload serialization and JSON contracts]]: 9 cross-community edges
- [[Community 04 - MultiDelay app core controls menu and meta controllers]]: 9 cross-community edges
- [[Community 11 - Harmoniqs hosted app performance model]]: 8 cross-community edges
- [[Community 15 - Host modulation lanes and external control safety]]: 7 cross-community edges
- [[Community 03 - Braids portable core and hosted app wrapper]]: 6 cross-community edges
- [[Community 07 - CloudSeed hosted app performance pages and arp]]: 5 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Effective state snapshot and hosted synth lifecycle` instead of `Community 1`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
