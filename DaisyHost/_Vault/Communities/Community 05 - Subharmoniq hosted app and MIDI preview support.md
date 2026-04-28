---
title: Community 05 - Subharmoniq hosted app and MIDI preview support
community_id: 5
node_count: 78
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/app"]
---

# Community 05 - Subharmoniq hosted app and MIDI preview support

This note gives `Community 5` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Subharmoniq hosted app and MIDI preview support groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\apps\SubharmoniqCore.cpp`: 70 graph nodes
- `src\MidiNotePreview.cpp`: 6 graph nodes
- `tests\test_midi_note_preview.cpp`: 2 graph nodes

## High-Degree Nodes

- `SubharmoniqCore.cpp`: 74 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `RefreshSnapshots()`: 21 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `GetPatchBindings()`: 14 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `TriggerFieldKeyAction()`: 11 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `GetActivePage()`: 11 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `BuildMenuModel()`: 11 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `BuildDisplay()`: 11 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `GetFieldKeyLedValues()`: 9 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SetMenuItemValue()`: 9 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SetPortInput()`: 8 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SubharmoniqCore()`: 7 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SetParameterValue()`: 7 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `Process()`: 6 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `SetControl()`: 6 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `StripParameterId()`: 6 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `MidiNotePreview.cpp`: 5 graph edges, source `src\MidiNotePreview.cpp`
- `MenuRotate()`: 5 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `HandleMidiEvent()`: 5 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `GetQuantizeMode()`: 5 graph edges, source `src\apps\SubharmoniqCore.cpp`
- `IsPlaying()`: 5 graph edges, source `src\apps\SubharmoniqCore.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 34 cross-community edges
- [[Community 00 - JUCE processor editor and live host surface]]: 6 cross-community edges
- [[Community 03 - Braids portable core and hosted app wrapper]]: 2 cross-community edges
- [[Community 11 - Harmoniqs hosted app performance model]]: 2 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 1 cross-community edges
- [[Community 07 - CloudSeed hosted app performance pages and arp]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Subharmoniq hosted app and MIDI preview support` instead of `Community 5`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
