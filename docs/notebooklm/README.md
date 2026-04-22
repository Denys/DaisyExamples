# DaisyBrain Notebook Guide

`DaisyBrain` is the strategic NotebookLM notebook for `DaisyExamples/`.

Use it when an agent needs:

- repo-wide context that spans multiple workspaces
- current strategic direction across DaisyHost, DaisyDAFX, Pedal, and custom projects
- high-signal recall of roadmaps, constraints, and architectural tradeoffs
- help synthesizing local docs before making design decisions

Do **not** use it as a replacement for local source-of-truth docs. The order is:

1. nearest `README.md`, `CHECKPOINT.md`, `CONTROLS.md`, and local plans
2. repo-level `AGENTS.md` and `LATEST_PROJECTS.md`
3. DaisyBrain for cross-project synthesis or strategic recall

## Notebook Identity

- Notebook title: `DaisyBrain`
- Notebook id: `8084395c-2d50-464c-967b-7569926fe771`
- Preferred local CLI home: `C:\Users\denko\.notebooklm`
- Repo-local bridge:
  - [daisybrain.config.json](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/docs/notebooklm/daisybrain.config.json)
  - [daisybrain.ps1](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/docs/notebooklm/daisybrain.ps1)

## Direct Repo Bridge

Prefer the repo-local wrapper when talking to DaisyBrain directly:

```powershell
powershell -ExecutionPolicy Bypass -File .\docs\notebooklm\daisybrain.ps1 info
powershell -ExecutionPolicy Bypass -File .\docs\notebooklm\daisybrain.ps1 ask --json "What changed in DaisyHost?"
powershell -ExecutionPolicy Bypass -File .\docs\notebooklm\daisybrain.ps1 source-list
powershell -ExecutionPolicy Bypass -File .\docs\notebooklm\daisybrain.ps1 source-add .\docs\notebooklm\context\05-current-state-2026-04.md
```

The wrapper:

- pins the checked-in DaisyBrain notebook id
- sets `NOTEBOOKLM_HOME` to the standard local NotebookLM home
- resolves the local `notebooklm.exe`
- removes the need to repeat `-n <notebook-id>` manually

## Raw CLI Usage

```sh
$env:NOTEBOOKLM_HOME = "$HOME\.notebooklm"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" ask -n 8084395c-2d50-464c-967b-7569926fe771 "question"
& "$HOME\AppData\Local\Programs\Python\Python314\Scripts\notebooklm.exe" source list -n 8084395c-2d50-464c-967b-7569926fe771 --json
```

Prefer the CLI over MCP for this notebook when local automation is needed.

## Target Source Set

The preferred steady-state DaisyBrain source set is intentionally small:

- `AGENTS.md`
- `LATEST_PROJECTS.md`
- `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`
- `docs/plans/2026-04-18-daisyhost-book-roadmap.md`
- `docs/notebooklm/context/01-repo-map-and-workflow.md`
- `docs/notebooklm/context/02-libraries-and-platforms.md`
- `docs/notebooklm/context/03-project-status-model.md`
- `docs/notebooklm/context/04-project-portfolio.md`
- `docs/notebooklm/context/05-current-state-2026-04.md`
- `docs/notebooklm/context/06-roadmaps-and-finished-work.md`
- `docs/notebooklm/context/07-daisybrain-memory-policy.md`
- `DaisyHost/CHECKPOINT.md`

Prefer this core over a larger set of overlapping raw docs.

## Companion Context Pack

The files under `docs/notebooklm/context/` are the curated persistent-memory
pack for DaisyBrain. They are intentionally split by function:

- `01-repo-map-and-workflow.md`
- `02-libraries-and-platforms.md`
- `03-project-status-model.md`
- `04-project-portfolio.md`
- `05-current-state-2026-04.md`
- `06-roadmaps-and-finished-work.md`
- `07-daisybrain-memory-policy.md`

Use this pack to keep DaisyBrain high-signal and referential rather than turning
it into a raw archive.

## Notebook-Facing Naming Rule

Avoid attaching generic filenames like plain `README.md` when a notebook-facing
mirror is needed. Use a descriptive title in the filename itself so NotebookLM
shows unambiguous source names.

Preferred pattern:

- `README - DaisyHost Workspace.md`
- `README - Pedal Workspace.md`
- `README - Field CloudSeed.md`

Use two to four context words after `README` so an agent can tell what it is at
a glance during notebook queries.

See [daisybrain-curation.md](/C:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/docs/notebooklm/daisybrain-curation.md) for the small curation workflow.

## Refresh Triggers

Refresh the context pack and DaisyBrain sources when:

- a first-party workspace changes strategic role or verification status
- a new family of active projects becomes important
- a roadmap phase is completed or materially re-ordered
- a new durable library/workspace is added to the repo
- the repo-level memory hierarchy changes
