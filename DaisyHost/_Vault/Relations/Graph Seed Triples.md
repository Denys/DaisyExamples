---
title: Graph Seed Triples
source: curated DaisyHost semantic seed
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/relations"]
---

# Graph Seed Triples

These triples are intentionally explicit so semantic extraction can produce meaningful edges instead of generic `contains [EXTRACTED]` labels.

## DaisyHost Platform

- [[DaisyHost Platform]] -- uses -> [[JUCE plugin and standalone shell]]
- [[DaisyHost Platform]] -- runs -> [[two hosted nodes at a time]]
- [[DaisyHost Platform]] -- builds with -> [[CMake and build_host.cmd]]

## Hosted App Core Contract

- [[Hosted App Core Contract]] -- is implemented by -> [[supported hosted app cores]]
- [[Hosted App Core Contract]] -- feeds -> [[DaisyHost processor/editor binding]]
- [[Hosted App Core Contract]] -- defines -> [[canonical parameters and menu model]]

## Live Rack Runtime

- [[Live Rack Runtime]] -- serializes through -> [[Host Session State]]
- [[Live Rack Runtime]] -- is validated by -> [[LiveRackTopology tests]]
- [[Live Rack Runtime]] -- routes audio through -> [[route plan contract]]

## Render Runtime

- [[Render Runtime]] -- consumes -> [[scenario JSON]]
- [[Render Runtime]] -- emits -> [[manifest JSON]]
- [[Render Runtime]] -- shares route rules with -> [[Live Rack Runtime]]

## Board Profile System

- [[Board Profile System]] -- creates -> [[daisy_patch profile]]
- [[Board Profile System]] -- creates -> [[daisy_field profile]]
- [[Board Profile System]] -- drives -> [[JUCE Editor Surface]]

## Field Surface Mapping

- [[Field Surface Mapping]] -- depends on -> [[Board Profile System]]
- [[Field Surface Mapping]] -- targets -> [[selected live rack node]]
- [[Field Surface Mapping]] -- records evidence in -> [[snapshots and render manifests]]

## DaisyHostCLI Diagnostics

- [[DaisyHostCLI Diagnostics]] -- wraps -> [[build_host.cmd]]
- [[DaisyHostCLI Diagnostics]] -- reports -> [[CTest totals]]
- [[DaisyHostCLI Diagnostics]] -- classifies -> [[known blockers]]

## CLI Payload JSON Contracts

- [[CLI Payload JSON Contracts]] -- serializes -> [[effective state snapshots]]
- [[CLI Payload JSON Contracts]] -- serializes -> [[render results]]
- [[CLI Payload JSON Contracts]] -- supports -> [[agent diagnostics]]

## Build Verification Gate

- [[Build Verification Gate]] -- runs -> [[cmake configure]]
- [[Build Verification Gate]] -- builds -> [[unit_tests and DaisyHost binaries]]
- [[Build Verification Gate]] -- verifies -> [[CTest suite]]

## Effective Host State Snapshot

- [[Effective Host State Snapshot]] -- reads from -> [[DaisyHost processor state]]
- [[Effective Host State Snapshot]] -- feeds -> [[debugState JSON]]
- [[Effective Host State Snapshot]] -- captures -> [[selected-node context]]

## MultiDelay App Core

- [[MultiDelay App Core]] -- implements -> [[Hosted App Core Contract]]
- [[MultiDelay App Core]] -- is registered as app id -> [[multidelay]]
- [[MultiDelay App Core]] -- is hosted by -> [[DaisyHost Platform]]

## CloudSeed App Core

- [[CloudSeed App Core]] -- implements -> [[Hosted App Core Contract]]
- [[CloudSeed App Core]] -- is registered as app id -> [[cloudseed]]
- [[CloudSeed App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Braids App Core

- [[Braids App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Braids App Core]] -- is registered as app id -> [[braids]]
- [[Braids App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Harmoniqs App Core

- [[Harmoniqs App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Harmoniqs App Core]] -- is registered as app id -> [[harmoniqs]]
- [[Harmoniqs App Core]] -- is hosted by -> [[DaisyHost Platform]]

## VA Synth App Core

- [[VA Synth App Core]] -- implements -> [[Hosted App Core Contract]]
- [[VA Synth App Core]] -- is registered as app id -> [[vasynth]]
- [[VA Synth App Core]] -- is hosted by -> [[DaisyHost Platform]]

## PolyOsc App Core

- [[PolyOsc App Core]] -- implements -> [[Hosted App Core Contract]]
- [[PolyOsc App Core]] -- is registered as app id -> [[polyosc]]
- [[PolyOsc App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Subharmoniq App Core

- [[Subharmoniq App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Subharmoniq App Core]] -- is registered as app id -> [[subharmoniq]]
- [[Subharmoniq App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Torus App Core

- [[Torus App Core]] -- implements -> [[Hosted App Core Contract]]
- [[Torus App Core]] -- is registered as app id -> [[torus]]
- [[Torus App Core]] -- is hosted by -> [[DaisyHost Platform]]

## Cross-System Triples

- [[Board Profile System]] -- drives -> [[JUCE Editor Surface]]
- [[Field Surface Mapping]] -- uses -> [[Board Profile System]]
- [[Render Runtime]] -- uses -> [[Live Rack Runtime route plan contract]]
- [[CLI Payload JSON Contracts]] -- serializes -> [[Effective Host State Snapshot]]
- [[DaisyHostCLI Diagnostics]] -- wraps -> [[Build Verification Gate]]
- [[Build Verification Gate]] -- includes -> [[Smoke Test Harness]]
- [[Host Session State]] -- persists -> [[selected node and rack topology]]
- [[Host Automation Bridge]] -- maps -> [[five stable DAW automation slots]]
- [[MetaControllers]] -- write back to -> [[canonical hosted app parameters]]
- [[Training Dataset Workflow]] -- uses -> [[Render Runtime]]

