# DaisyTheory

`DaisyTheory/` is a local, theory-first workspace for questions about audio
microcontrollers, embedded DSP, DAFX, signal flow, programming languages,
algorithm study, algorithm design, and how those topics map onto the
surrounding `DaisyExamples/` repo.

This directory is intentionally local. It does not redefine the broader
firmware workflow for the rest of `DaisyExamples/`. Instead, it provides a
safer conceptual workspace that can still turn theory into implementation when
the target path and write scope are made explicit.

## Purpose

- explain and compare DSP, DAFX, embedded-audio, and language/runtime ideas
- study textbook, paper, MATLAB, C/C++, and prototype algorithms before they are
  ported or productized
- design algorithm transfer packages that capture equations, parameters, state,
  realtime constraints, and validation needs
- map theoretical questions onto real local surfaces such as `DaisySP/`,
  `DaisyDAFX/`, `libDaisy/`, `DaisyHost/`, and board examples
- recommend the right integration target for an algorithm: reusable DSP module,
  DAFX library component, hardware abstraction, host-side validator, or
  board-specific example
- serve as a sharable theory surface that nearby DaisyExamples work can consult
  when conceptual context is needed
- preserve implementation capability without treating theory questions as edit
  permission

## Context Sources

When local theory work needs repo context, start with:

- local algorithm references such as `Hack_Audio_textbookcode-master/`,
  `Synthesis_ToolKit/`, generated LLM reference files, and companion notes
- `../AGENTS.md`
- `../README.md`
- `../LATEST_PROJECTS.md`
- nearby project docs under `../docs/plans/`, `../DAISY_QAE/`, `../DaisyHost/`,
  and other target-specific paths

When algorithm work is intended for integration, confirm the target ownership
before editing:

- `../DaisySP/` for small reusable DSP modules
- `../DaisyDAFX/` for textbook/research-derived algorithm libraries with
  host-side tests and reference docs
- `../libDaisy/` for hardware support rather than pure DSP
- board examples or `../MyProjects/_projects/` for productized patches and
  control mappings
- `../DaisyHost/` for host-side validation and plugin-facing adapters

When theory questions need strategic memory across multiple workspaces, consult
the DaisyBrain entry points:

- `../docs/notebooklm/README.md`
- `../docs/notebooklm/context/`

## Write Safety

Default mode here is analysis, explanation, comparison, derivation, and design.
Do not edit files just because the subject is technical.

Implementation is still allowed, but only after the exact target directory and
intended file set are named.
