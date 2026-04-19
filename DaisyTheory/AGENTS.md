# AGENTS.md

Local instructions for coding agents working in `DaisyTheory/`.

This file specializes the parent `../AGENTS.md` for theory-heavy work. Keep the
parent repo workflow, safety rules, and validation expectations in force unless
this local file adds a narrower rule for `DaisyTheory/`.

## Scope

- `DaisyTheory/` is a local theory workspace, not a primary firmware build
  root.
- Its default job is to answer conceptual questions about audio
  microcontrollers, DSP, DAFX, programming languages, runtime constraints, and
  related embedded-audio design tradeoffs.
- It may produce code, docs, plans, or examples when explicitly asked, but
  implementation must be scoped before any write.

## Primary Behavior

- Treat theory questions as explanation, analysis, comparison, derivation, or
  design tasks by default.
- Do not infer edit permission from a conceptual prompt, even when the answer
  includes code snippets or implementation advice.
- Map abstract questions onto the local Daisy ecosystem when useful:
  `libDaisy/`, `DaisySP/`, `DaisyDAFX/`, `DaisyHost/`, board examples,
  `DAISY_QAE/`, and relevant docs/plans.
- Distinguish clearly between:
  - repo evidence from local files
  - general DSP or embedded-systems knowledge
  - inference or design recommendation

## Shared Context And Awareness

`DaisyTheory/` should be aware of nearby DaisyExamples work, and nearby
DaisyExamples work may use `DaisyTheory/` as a conceptual reference surface.

When context is needed, prefer this route:

1. Read this directory's `README.md` and `AGENTS.md`.
2. Read parent repo orientation docs:
   `../AGENTS.md`, `../README.md`, and `../LATEST_PROJECTS.md`.
3. Read the nearest target-specific docs or plans that match the question.
4. If repo-wide strategy or older context is needed, consult DaisyBrain via:
   `../docs/notebooklm/README.md` and `../docs/notebooklm/context/`.
5. Treat DaisyTheory as a sharable theory node, not a replacement for source of
   truth in code or target-local docs.

## Scoped Write Model

Implementation capability is preserved, but writes are gated by explicit scope.

Before writing any file, state:

- the exact target directory
- the intended file set
- whether the task is `docs-only`, `code-only`, or `mixed`
- why the task is moving from theory into implementation

Default local write surface:

- `DaisyTheory/*.md`
- `DaisyTheory/docs/plans/*`

Touch files outside `DaisyTheory/` only when one of these is true:

- the user explicitly asks for implementation in a named target path
- the user asks for a concrete artifact whose natural home is outside
  `DaisyTheory/`
- the answer cannot be completed honestly without a narrowly scoped edit in the
  named target workspace

If the write scope expands beyond what was declared, stop and restate the new
scope before continuing.

## Safety Rules

- Never delete, move, or rename files unless the user explicitly asks for that
  action.
- Prefer additive edits and narrow in-place patches over broad rewrites.
- Do not run mass formatters, broad search/replace operations, or repo-wide
  cleanup from `DaisyTheory/` unless explicitly requested.
- Preserve unrelated changes in the parent repo. Expect a dirty worktree.
- Check `git status` before writing and review the relevant diff before
  claiming completion.
- For risky multi-file implementation tasks outside `DaisyTheory/`, recommend a
  feature branch or git worktree first.

## Theory Quality Bar

For theoretical answers, be concrete about the engineering constraints that
matter:

- sample rate, block size, latency, and callback timing
- CPU budget, RAM budget, and memory placement
- fixed-point vs floating-point tradeoffs
- aliasing, interpolation, numerical stability, and modulation artifacts
- ISR/main-loop/audio-callback separation
- control-rate vs audio-rate processing
- host-side vs embedded-side validation boundaries

Prefer tying explanations to local examples when possible. Useful reference
surfaces include:

- `../DaisySP/`
- `../DaisyDAFX/`
- `../libDaisy/`
- `../DaisyHost/`
- `../advanced_daisy_functionalities.txt`
- `../docs/plans/`
- `../DAISY_QAE/`

If a claim is based on general knowledge rather than local repo evidence, say
so.

## Verification

For local instruction changes in `DaisyTheory/`, verify with targeted readback
and diff review.

For code or firmware changes outside `DaisyTheory/`, fall back to the smallest
relevant validation path from the parent `../AGENTS.md`.
