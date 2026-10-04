# DVPE integration: deliberately no invented project

## Current result

No `.dvpe` file or DVPE block implementation is included. The two requested
repositories are useful investigation targets:

- https://github.com/Denys/DVPE_Daisy-Visual-Programming-Environment
- https://github.com/Denys/DVPE

A public README for the first project describes `.dvpe` save/load, C++/Makefile
export and custom/nested blocks. That is not enough to establish the exact
current serialization schema, valid block identifiers, Field control bindings,
or generated-code behavior. A complete registry/schema/export round trip was
not executed, and the other repository could not be qualified through the
available retrieval route.

The handwritten C++/Makefile implementation is the working development
candidate. Omitting an unvalidated visual project is intentional.

## Reusable primitives already implemented and host-tested

| Candidate block | C++ source | Contract to preserve |
|---|---|---|
| Parameter ramp | `src/dsp/Primitives.h::Ramp` | Finite bounded target, fixed-duration linear transition, unchanged target does not restart |
| Retrigger-safe one-shot | `src/dsp/Primitives.h::Envelope` | Attack from current amplitude, finite release, repeated release does not postpone silence |
| Sample-domain groove clock | `src/core/GrooveClock.h` | Rational remainder, 16th-note tempo, pair-preserving swing, no tempo-reset drift |
| Soft pickup | `src/core/ControlPickup.h` | Target crossing capture, finite input, explicit rearm |
| Bounded event queue | `src/core/SpscQueue.h` | Single producer/consumer, N-1 capacity, release/acquire ownership, visible overflow |
| Output-test arm | `src/core/OutputTestArm.h` | Release-to-arm, momentary hold, inhibit, not hardware fail-safe |
| Four-voice percussion unit | `src/dsp/Voices.*` | Explicit init/trigger/render, four semantic controls, deterministic noise and finite tails |

These are reusable C++ units, not registered DVPE blocks. A queue or output-arm
primitive also needs a domain-appropriate editor interface; exposing internal
concurrency semantics as arbitrary audio-rate graph wiring would be misleading.

## Codex integration procedure after the standalone candidate is qualified

1. Inspect the actual DVPE README, package manifests, schema/migration code,
   block registry, board definitions, compiler/code generator, exporter,
   known fixtures and tests at a recorded commit. Do not classify the project
   from folder shape.
2. Identify supported Field hardware and event/control/audio-rate scheduling.
   Verify how initialization, per-block processing, per-sample processing,
   persistent state, bounded memory and mutable parameters are represented.
3. Map a single simple primitive, preferably Ramp, to a genuine existing block
   definition pattern. Add complete parameter limits, rate semantics, help
   text and error paths in the actual registry's format.
4. Use the editor/serializer to create a small valid graph, save `.dvpe`,
   reopen it, export code and compile against the pinned real library.
   Compare its host numerical output with the C++ reference.
5. Add tests for schema round trips, generated code, parameter validation and
   mismatched execution rates. Ensure graph edits cannot introduce allocation,
   logging or blocking in the callback.
6. Only then compose the complete instrument graph if its scheduler/state
   model can express transport, A/B/Fill, foreground/UI ownership and bounded
   events without hidden unsafe glue. Otherwise keep the handwritten engine
   and make only clearly supported blocks reusable.
7. Record exact DVPE commit, package lock, graph schema version, generator
   version, output diff and build results. Preserve existing projects and
   migration compatibility; propose public registry changes separately.

Do not create a fictional `.dvpe` with guessed JSON keys, omit unsupported nodes
silently, or call a graph complete because it contains labels matching the
instrument's modules.
