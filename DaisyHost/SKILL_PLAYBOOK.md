# DaisyHost Skill Playbook

Last updated: 2026-04-22

Use this file for DaisyHost skill-related activities:

- skill selection for a task or task class
- `Expected UF` vs `Observed UF`
- evidence and validation notes
- skill activity logs
- refinements or corrections to reusable skill workflows

`PROJECT_TRACKER.md` records that a skill was used in an iteration.
`SKILL_PLAYBOOK.md` records how well the skill actually performed.

## UF Definitions

- `Expected UF`:
  - expected usefulness for DaisyHost tasks of the relevant class
  - a prior estimate based on task fit, prerequisites, and expected quality gain
  - not validated by the skill description alone
- `Observed UF`:
  - evidence-based usefulness from actual DaisyHost task use
  - update only after a materially used skill produced an observable outcome
  - keep as `Pending` when there is no meaningful DaisyHost evidence yet

## Validation Method

Evaluate `Observed UF` per task class, not globally.

For each materially used skill, capture:

- task class
- date
- expected UF before use
- actual outcome
- concrete evidence
- blockers or friction
- whether the skill should be reused for the same DaisyHost task class

Recommended scoring dimensions, each on `0..5`:

| Dimension | Meaning |
|---|---|
| Task fit | How directly the skill matched the DaisyHost task |
| Quality gain | How much it improved the output or reduced drift/regressions |
| Time gain | How much it reduced rework or iteration count |
| Reliability | How usable it was without auth/tooling/process blockers |
| Reuse value | How likely it is to help again on similar DaisyHost work |

Recommended weighted formula:

```text
Observed UF =
0.30 * Quality gain +
0.25 * Task fit +
0.20 * Time gain +
0.15 * Reliability +
0.10 * Reuse value
```

Round to the nearest tenth or nearest integer. If the evidence is too weak,
leave `Observed UF` as `Pending`.

## Current Skill Register

| Skill | Primary DaisyHost use | Expected UF | Observed UF | Sample size | Evidence status | Caveat | Improvement note | Last validated |
|---|---|---|---|---|---|---|---|---|
| `superpowers:test-driven-development` | Code iterations touching production behavior, tests, or bug fixes. | `5/5` | `5/5` | `2` | Observed in the 2026-04-22 W4/W5 package and the later same-day CloudSeed Workstream-6 pilot: helper-layer red/green flow forced a clean failing build first, then landed the portable wrapper, app adapter, render coverage, and a green full host gate. | C++ helper work still needed interface/header scaffolding before the first red build could link. | Add DaisyHost-native red-step templates for helper-layer and processor-adjacent JUCE work. | `2026-04-22` |
| `verification-before-completion` | Fresh build/test gate verification before any DaisyHost success claim or closeout. | `5/5` | `5/5` | `3` | Observed in the 2026-04-22 W4/W5 package, the same-day verification-hardening pass, and the later CloudSeed Workstream-6 pilot: it forced the full host gate to be rerun again after the wrapper/app work and kept the processor-binding change from being treated as “unit-tested only”. | Full verification is slower and still depends on surfacing exact commands/results in the tracker. | Keep the wrapper command plus any required firmware commands copied verbatim into tracker/checkpoint closeouts. | `2026-04-22` |
| `systematic-debugging` | Unexpected DaisyHost build/test/runtime failures where the first obvious fix may be wrong. | `4/5` | `5/5` | `1` | Observed in the 2026-04-22 verification-hardening pass: it prevented blaming the new wrapper for a VST3 build failure and instead isolated the real break to the MSBuild/`cmd` launch path for `juce_vst3_helper.exe`. | The workflow is slower up front because it insists on reproducing failures outside the main build command before fixing. | Add a DaisyHost-specific JUCE/MSBuild triage checklist for helper-path and post-build command failures. | `2026-04-22` |
| `coderabbit:code-review` | Coherent diff review or milestone handoff after local testing has passed. | `4/5` | `Pending` | `0` | No DaisyHost-local observed sample set yet in this file. | Requires a real git worktree plus CodeRabbit auth; this checkout does not currently satisfy that prerequisite. | Define a DaisyHost review-scope policy for `uncommitted` vs `committed` review and note the standard config-file set to pass with `-c`. | `Pending` |
| `notebooklm` | DaisyBrain architectural recall when local docs are insufficient. | `3/5` | `Pending` | `0` | No DaisyHost-local observed sample set yet in this file. | Strategic memory only; must not override local source-of-truth docs. | Add a small DaisyHost prompt cookbook and keep the “local docs first, NotebookLM second” rule explicit. | `Pending` |
| `doc-coauthoring` | Multi-doc DaisyHost maintenance where tracker/checkpoint/readme/agents/changelog need to stay aligned. | `3/5` | `3/5` | `2` | Observed in the 2026-04-22 skill-playbook extraction/docs-sync pass and the same-day verification-hardening closeout; helped keep the wrapper, firmware proof, and post-WS7 DAW-validation deferral aligned across the tracking docs. | The default workflow is heavier than most DaisyHost maintenance passes need. | Define a lightweight repo-maintenance variant focused on synchronized tracking-doc updates. | `2026-04-22` |

## Planned Evaluation Queue

Predeclare likely skill use here before code starts when an iteration is
expected to produce fresh DaisyHost evidence.

| Date | Planned skill | Task class | Expected UF | Why it is being queued now | Evidence target |
|---|---|---|---|---|---|
| 2026-04-22 | `superpowers:test-driven-development` | Host automation bridge helper and slot-mapping tests | `5/5` | W4 is a new behavior surface with a clear red/green helper-test path. | Red helper tests fail for the expected bridge contract, then pass after implementation and host-gate rerun. |
| 2026-04-22 | `superpowers:test-driven-development` | Effective-state snapshot/readback helper and contract tests | `5/5` | W5 is also a new behavior surface and should be specified through helper-layer tests before processor glue lands. | Red helper tests fail for snapshot contract gaps, then pass after implementation and host-gate rerun. |
| 2026-04-22 | `doc-coauthoring` | Cross-doc closeout for tracker, checkpoint, readme, changelog, and skill playbook | `3/5` | This package changes both runtime capability and local workflow truth, so cross-doc sync should be evaluated explicitly. | Post-verification doc pass keeps workstream status, unlock language, and test evidence aligned without stale claims. |

## Skill Activity Log

Log only materially used skills here. Referencing a skill as a source of
phrasing or policy without actually exercising its workflow does not count as a
material use.

| Date | Skill | Task class | Expected UF | Outcome | Observed UF contribution | Evidence | Blockers / friction | Reuse verdict |
|---|---|---|---|---|---|---|---|---|
| 2026-04-22 | `superpowers:test-driven-development` | Workstream-6 CloudSeed hosted-app pilot (`DaisyCloudSeedCore`, app adapter, render smoke) | `5/5` | Forced the CloudSeed package to start with a real red step: the first `unit_tests` build failed for the missing wrapper/app headers, then the implementation was driven until the new core/app/render slice and the full host gate both went green. | `5/5` | Red build failed on missing `daisyhost/DaisyCloudSeedCore.h` and `daisyhost/apps/CloudSeedCore.h`; targeted CloudSeed slice passed `24/24`; full host gate passed `79/79`, including standalone/render smoke. | The red phase also surfaced an upstream MSVC portability defect in `RandomBuffer.cpp`, so the skill still needed repo-aware judgment about whether the fix belonged in host code or the imported source. | Strong reuse for DaisyHost app pilots where the cleanest contract can be specified in helper-layer and render tests before the UI/processor glue is trusted. |
| 2026-04-22 | `verification-before-completion` | Workstream-6 CloudSeed closeout with processor-binding refresh | `5/5` | Prevented stopping at “the new tests pass” and forced the exact JUCE target set plus full `ctest` rerun after the processor binding refresh and smoke-harness changes. | `5/5` | After targeted CloudSeed tests passed, the full sanitized build command rebuilt `unit_tests`, `DaisyHostHub`, `DaisyHostRender`, `DaisyHostPatch_VST3`, and `DaisyHostPatch_Standalone`, then `ctest --test-dir build -C Release --output-on-failure` passed `79/79`. | The full proof loop still needs the shell-local `Path` / `PATH` sanitization step before MSBuild-backed commands. | Reuse on every DaisyHost app addition or processor/UI integration change, not just on infrastructure work. |
| 2026-04-22 | `systematic-debugging` | Verification-hardening pass after the new host-build wrapper exposed a VST3 post-build crash | `4/5` | Prevented a wrapper-level band-aid: the failure was reproduced outside the wrapper, narrowed to `juce_vst3_helper.exe` succeeding in PowerShell but failing through the MSBuild/`cmd` launch path, and then fixed with a PowerShell-backed manifest helper script. | `5/5` | `powershell -ExecutionPolicy Bypass -File .\build_host.ps1` exposed the failing VST3 target; direct helper invocation succeeded from PowerShell; `cmd`-style invocation failed with `LoadLibraryW failed for path ""`; the CMake fix plus `tools/write_vst3_manifest.ps1` restored `.\build_host.cmd` to green. | Needed extra repro steps outside the main build because the first failure looked like a generic VST3 target break. | Strong reuse whenever DaisyHost hits opaque JUCE/MSBuild post-build failures. |
| 2026-04-22 | `verification-before-completion` | Wrapper/factory verification hardening plus firmware proof closeout | `5/5` | Forced the session to keep going after the first wrapper run exposed a real VST3 build failure, and required fresh wrapper-driven host-gate evidence plus fresh firmware `make` evidence before closeout. | `5/5` | `.\build_host.cmd` passed `71/71`; `make` passed in `patch/MultiDelay/`; `patch/Torus` was forced from an up-to-date response into a fresh rebuild after PowerShell removal of `build/`. | The proof loop is slower when a verification command uncovers an unrelated build defect that then has to be debugged first. | Reuse on every DaisyHost verification, remediation, or milestone-closeout pass. |
| 2026-04-22 | `superpowers:test-driven-development` | W4/W5 helper-layer implementation for the DAW automation bridge and effective-state snapshot contract | `5/5` | Forced a clean red step first: the first `unit_tests` build failed on unresolved helper symbols, then one real contract assertion failed before green implementation and the final `71/71` host gate. | `5/5` | Red build failed for the expected missing helper entrypoints; targeted helper tests passed `6/6`; full sanitized host gate passed `71/71`. | C++/JUCE work still needed header/interface scaffolding before the first red build could link, and one `effectiveNormalizedValue` assumption had to be corrected. | Strong reuse for future DaisyHost feature work that can be isolated into helper-layer tests before processor glue. |
| 2026-04-22 | `verification-before-completion` | Final W4/W5 verification and closeout discipline | `5/5` | Prevented treating a timed-out partial target build as success and forced a fresh rerun of the exact full host-gate command before closeout. | `5/5` | After a partial build timed out, the full sanitized command was rerun successfully: `cmake -S . -B build`, the full Release target set, then `ctest --test-dir build -C Release --output-on-failure` with `71/71` passing. | The full pass is slower and requires the shell-local `Path` / `PATH` sanitization step. | Reuse on every DaisyHost milestone or user-facing completion claim. |
| 2026-04-22 | `doc-coauthoring` | Multi-doc verification-hardening closeout after wrapper/firmware remediation | `3/5` | Helped keep the wrapper entrypoint, VST3 manifest workaround, firmware rerun evidence, and post-WS7 DAW-validation deferral aligned across `README.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, and `CHANGELOG.md`. | `3/5` | The same closeout pass updated all four tracking docs plus the skill playbook without reintroducing stale “manual gap” wording for the shell issue. | Still heavier than necessary for small one-file documentation edits. | Reuse for multi-doc closeouts where verification truth changes in more than one place. |
| 2026-04-22 | `doc-coauthoring` | Multi-doc DaisyHost maintenance and workflow synchronization | `3/5` | Helped split skill-specific content out of the tracker and keep `README.md`, `AGENTS.md`, `PROJECT_TRACKER.md`, `CHECKPOINT.md`, `CHANGELOG.md`, and `LATEST_PROJECTS.md` aligned around the new file. | `3/5` | Cross-doc terminology stayed consistent; the dedicated skill file did not become orphaned; tracker and local-doc entrypoints now point to the same skill-governance model. | Workflow is broader and heavier than needed for small maintenance passes. | Reuse for larger DaisyHost doc refactors; avoid for trivial single-file edits. |

## Update Rules

- If a skill was materially used in an iteration, update both:
  - `PROJECT_TRACKER.md` to record that it was used in the iteration
  - `SKILL_PLAYBOOK.md` to record the evidence and any `Observed UF` change
- Do not increase `Observed UF` without citing concrete DaisyHost evidence.
- Do not copy a skill description into `Observed UF`; validation must come from
  actual task outcomes.
- Revisit `Expected UF` if repeated DaisyHost evidence shows the prior estimate
  was consistently wrong.
