---
title: Community 10 - Subharmoniq portable DSP core
community_id: 10
node_count: 49
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/app"]
---

# Community 10 - Subharmoniq portable DSP core

This note gives `Community 10` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Subharmoniq portable DSP core groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\DaisySubharmoniqCore.cpp`: 49 graph nodes

## High-Degree Nodes

- `DaisySubharmoniqCore.cpp`: 48 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `Clamp01()`: 9 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `Process()`: 8 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `SetParameterValue()`: 6 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `GetSequencerStepSemitones()`: 6 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `RestoreStatefulParameterValues()`: 5 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `AdvanceClockPulse()`: 5 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `GetSequencerStepRatio()`: 4 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `ClampInt()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `NormalizedToLog()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `QuantizedSemitone()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `SetSequencerStepValue()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `SetRhythmDivisor()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `GetSequencerCv()`: 3 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `MidiNoteToFrequency()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `NormalizedToTempoBpm()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `DecodeSequencerStepId()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `SoftSaw()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `Square()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`
- `MakeDefaultParameters()`: 2 graph edges, source `src\DaisySubharmoniqCore.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 4 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Subharmoniq portable DSP core` instead of `Community 10`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
