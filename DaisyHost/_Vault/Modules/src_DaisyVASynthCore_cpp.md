---
title: Module - src\DaisyVASynthCore.cpp
source_file: src\DaisyVASynthCore.cpp
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/module", "daisyhost/app"]
---

# Module - `src\DaisyVASynthCore.cpp`

This module note was generated from the graphify AST graph. It gives a source file its own graph-friendly document so future semantic extraction can connect file-level roles to symbols and communities.

## Community Membership

- [[../Communities/Community 18 - VA Synth portable polyphonic core]]: 27 nodes
- [[../Communities/Community 01 - Effective state snapshot and hosted synth lifecycle]]: 5 nodes

## Prominent Symbols

- `DaisyVASynthCore.cpp`: 31 graph edges
- `Process()`: 9 graph edges
- `TriggerMidiNote()`: 9 graph edges
- `Clamp01()`: 8 graph edges
- `ReleaseMidiNote()`: 6 graph edges
- `GetWaveLabel()`: 5 graph edges
- `WaveIndex()`: 4 graph edges
- `AttackSeconds()`: 3 graph edges
- `ReleaseSeconds()`: 3 graph edges
- `DetuneSemitones()`: 3 graph edges
- `SetParameterValue()`: 3 graph edges
- `TriggerMomentaryAction()`: 3 graph edges
- `TriggerGate()`: 3 graph edges
- `Panic()`: 3 graph edges
- `GetCurrentVoiceCount()`: 3 graph edges
- `GetLastMidiNote()`: 3 graph edges
- `MidiNoteToFrequency()`: 2 graph edges
- `WaveLabel()`: 2 graph edges
- `RenderWave()`: 2 graph edges
- `SetEffectiveParameterValue()`: 2 graph edges
- `RestoreStatefulParameterValues()`: 2 graph edges
- `MakeDefaultParameters()`: 1 graph edges
- `DaisyVASynthCore()`: 1 graph edges
- `Prepare()`: 1 graph edges
- `ResetToDefaultState()`: 1 graph edges

## Relationship Types Observed

- `calls`: 37 edges
- `contains`: 31 edges

## Extraction Guidance

Add a concise human-written role for this module before future full semantic extraction if this file becomes a focus area. The current AST graph can say what symbols exist; this vault should say what job the module performs in DaisyHost.
