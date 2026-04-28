---
title: Community 00 - JUCE processor editor and live host surface
community_id: 0
node_count: 297
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/runtime"]
---

# Community 00 - JUCE processor editor and live host surface

This note gives `Community 0` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

JUCE processor editor and live host surface groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `src\juce\DaisyHostPluginProcessor.cpp`: 172 graph nodes
- `src\juce\DaisyHostPluginEditor.cpp`: 83 graph nodes
- `src\SignalGenerator.cpp`: 10 graph nodes
- `src\MidiEventTracker.cpp`: 5 graph nodes
- `src\BoardControlMapping.cpp`: 4 graph nodes
- `src\HostAutomationBridge.cpp`: 4 graph nodes
- `tests\test_cli_payloads.cpp`: 4 graph nodes
- `tests\test_host_automation_bridge.cpp`: 4 graph nodes
- `src\juce\DaisyHostPluginProcessor.h`: 3 graph nodes
- `include\daisyhost\BoardControlMapping.h`: 2 graph nodes

## High-Degree Nodes

- `DaisyHostPluginProcessor.cpp`: 186 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `DaisyHostPluginEditor.cpp`: 83 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `Clear()`: 46 graph edges, source `src\MidiLearnMap.cpp`
- `DaisyHostPatchAudioProcessorEditor()`: 37 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `processBlock()`: 32 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `RefreshCoreStateFromIdleHostChange()`: 32 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `LoadSession()`: 26 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `GetSelectedRackNode()`: 26 graph edges, source `src\juce\DaisyHostPluginProcessor.h`
- `paint()`: 20 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `prepareToPlay()`: 20 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `UpdateFieldControlUi()`: 19 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `buttonClicked()`: 18 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `timerCallback()`: 18 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `BuildActiveFieldControlMapping()`: 18 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `Clamp01()`: 17 graph edges, source `src\juce\DaisyHostPluginProcessor.cpp`
- `resized()`: 15 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `DrawDisplay()`: 15 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `DrawPanelTexts()`: 14 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `UpdateRackUi()`: 14 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`
- `RectFromPanel()`: 13 graph edges, source `src\juce\DaisyHostPluginEditor.cpp`

## Neighbor Communities

- [[Community 01 - Effective state snapshot and hosted synth lifecycle]]: 47 cross-community edges
- [[Community 02 - Render runtime timeline and scenario execution]]: 29 cross-community edges
- [[Community 13 - Board control mapping Field controls and keyboard MIDI]]: 26 cross-community edges
- [[Community 15 - Host modulation lanes and external control safety]]: 13 cross-community edges
- [[Community 22 - Live rack topology route plan contract]]: 9 cross-community edges
- [[Community 14 - Hub support launch plans and activity dispatch]]: 6 cross-community edges
- [[Community 05 - Subharmoniq hosted app and MIDI preview support]]: 6 cross-community edges
- [[Community 07 - CloudSeed hosted app performance pages and arp]]: 5 cross-community edges
- [[Community 03 - Braids portable core and hosted app wrapper]]: 4 cross-community edges
- [[Community 04 - MultiDelay app core controls menu and meta controllers]]: 4 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `JUCE processor editor and live host surface` instead of `Community 0`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
