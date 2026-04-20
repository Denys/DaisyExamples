# Noderr Daisy Helper Agent Design

## Task framing

- **Goal:** Add a local helper-agent surface under `DaisyTheory/noderr/` that helps agents and users install Noderr into Daisy projects and interface Noderr concepts with Daisy firmware work.
- **Primary context:** `DaisyTheory/noderr/` is a stock, generic Noderr copy. The real Daisy firmware Noderr instance already lives at `DaisyExamples/noderr/`.
- **Constraint:** The stock Noderr install and audit prompts assume web stacks, preview URLs, and `UI_`/`API_`/`SVC_` NodeIDs. Those assumptions are wrong for embedded Daisy firmware.
- **Done when:** `DaisyTheory/noderr/` contains a Daisy-aware `AGENTS.md` plus a concise operational runbook that routes work toward the correct Daisy surfaces without rewriting the stock Noderr bundle.

## Deployment fit

The smallest durable surface is a local `AGENTS.md` plus one companion runbook:

- `AGENTS.md` is the right always-on surface for a "helper agent" because it changes how an agent behaves when operating inside `DaisyTheory/noderr/`.
- A companion runbook keeps the always-on file compact while still giving a reusable install and interface checklist.
- A full rewrite of the stock Noderr docs or prompts would create a second Daisy-specific Noderr distribution and increase drift.

## Relevant repo context

- `DaisyExamples/noderr/INSTANCE_IDENTITY.md` already declares a Daisy firmware Noderr instance with `Mode: Firmware`.
- `DaisyExamples/noderr/environment_context.md` already shows the correct embedded interpretation of Noderr's environment model:
  - `local_dev_preview` is `N/A` for firmware
  - production is a flashed binary on physical hardware
  - verification centers on `make`, optional `make program`, and hardware behavior
- Repo-level `AGENTS.md` requires small scoped changes, targeted validation, and opt-in flashing.

## Proposed file set

- Create `DaisyTheory/noderr/AGENTS.md`
- Create `DaisyTheory/noderr/DAISY_HELPER_AGENT.md`
- Create `DaisyTheory/docs/plans/2026-04-20-noderr-daisy-helper-agent-design.md`
- Create `DaisyTheory/docs/plans/2026-04-20-noderr-daisy-helper-agent.md`

## Design

### 1. Local helper `AGENTS.md`

Keep the file short and operational:

- define `DaisyTheory/noderr/` as a helper adapter, not the canonical firmware Noderr instance
- route agents to `../../noderr/` when they need the real Daisy firmware Noderr state
- require an exact target Daisy project path before installation or reconciliation work
- map stock Noderr concepts onto Daisy firmware concepts
- warn against web-only assumptions unless the target is genuinely host-side or frontend work
- preserve repo safety rules: no flashing unless asked, no broad rewrites, no dirty-worktree cleanup

### 2. Companion runbook

Add one concise runbook that gives:

- routing rules between `DaisyTheory/noderr/`, repo-root `noderr/`, and project-local `noderr/` copies
- a Daisy install-and-reconcile checklist
- a concept translation table from generic Noderr language to Daisy firmware language
- minimum verification commands and decision points

### 3. Scope boundaries

This is a docs-only adapter. It does **not**:

- rewrite the stock Noderr prompts under `DaisyTheory/noderr/prompts/`
- modify the repo-root `DaisyExamples/noderr/` instance
- install Noderr into a specific project automatically

## Residual risks

- The stock generic prompt files remain present in `DaisyTheory/noderr/`, so the helper must explicitly tell users not to treat them as Daisy truth by default.
- A future full embedded rewrite could reduce ambiguity further, but it would also create a second Daisy-specific maintenance surface.

## Verification plan

- Read back the new files and confirm they point to `../../noderr/` as the canonical Daisy firmware instance.
- Run `git diff -- DaisyTheory` and confirm the change is contained to `DaisyTheory/`.
