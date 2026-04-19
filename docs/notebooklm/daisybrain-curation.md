# DaisyBrain Curation

Small operating workflow for keeping the `DaisyBrain` NotebookLM notebook useful
without turning it into a noisy archive.

## Goal

Keep DaisyBrain as a strategic decision aid by:

- preserving a small core source set
- preferring curated synthesis over overlapping raw docs
- refreshing sources only when strategic state changes
- avoiding ambiguous notebook titles such as duplicate `README.md`

## Core Source Set

Keep these sources in DaisyBrain by default:

- `AGENTS.md`
- `LATEST_PROJECTS.md`
- `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`
- active DaisyHost roadmap doc
- `docs/notebooklm/context/01-repo-map-and-workflow.md`
- `docs/notebooklm/context/02-libraries-and-platforms.md`
- `docs/notebooklm/context/03-project-status-model.md`
- `docs/notebooklm/context/04-project-portfolio.md`
- `docs/notebooklm/context/05-current-state-2026-04.md`
- `docs/notebooklm/context/06-roadmaps-and-finished-work.md`
- `docs/notebooklm/context/07-daisybrain-memory-policy.md`
- `DaisyHost/CHECKPOINT.md`

## Sources To Avoid Unless There Is A Specific Need

- raw duplicate `README.md` files
- append-only `CHANGELOG.md` files
- one-off summary dumps that overlap the context pack
- meta skill-evaluation docs unless the notebook is being used to reason about
  the memory system itself

These can stay in the repo, but they should usually not stay attached to the
notebook.

## Naming Convention For Notebook-Facing Mirrors

When a source is useful but its filename is too generic, create a notebook-facing
mirror with a descriptive filename.

Pattern:

- `README - <workspace> <context>.md`
- `CHECKPOINT - <workspace> <context>.md`
- `POLICY - <topic>.md`
- `STATE - <scope> <date>.md`

Examples:

- `README - DaisyHost Workspace.md`
- `README - Pedal Workspace.md`
- `CHECKPOINT - DaisyHost Host.md`

The notebook title should tell a future agent what the source is without
opening it.

## Refresh Triggers

Refresh DaisyBrain when:

- a foundational workspace changes strategic role
- `LATEST_PROJECTS.md` changes materially
- a roadmap phase is completed, re-ordered, or superseded
- a project moves between status buckets
- the repo memory hierarchy changes

Do not refresh for every small implementation change.

## Minimal Refresh Workflow

1. Update the relevant repo docs first.
2. Update the context files under `docs/notebooklm/context/`.
3. Compare the current notebook sources against the core source set.
4. Remove sources that are now redundant or noisy.
5. Add any newly important sources.
6. Run one or two sanity questions against DaisyBrain.
7. If the notebook answer is aligned, stop. Do not keep adding more sources.

## Sanity Questions

Use short questions that verify the notebook still knows:

- memory hierarchy
- strategic centers
- project status buckets
- DaisyHost next architectural gate

## Relationship To WrapUp

`wrapup` is useful for session summaries and long-term session memory, but it is
not the DaisyBrain curation workflow. Curation is about:

- source selection
- source pruning
- naming policy
- refresh timing
- sanity verification
