# DaisyTheory Local Agent Design Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Create a local-only instruction layer for `DaisyTheory/` that is theory-focused, implementation-capable, scoped-write safe, and aware of `DaisyExamples/` and DaisyBrain context.

**Architecture:** Add a local `README.md` and `AGENTS.md` inside `DaisyTheory/`, then add thin wrapper files that defer to the local `AGENTS.md`. Keep the write surface local while explicitly routing theory questions toward nearby repo docs, examples, DaisyDAFX, DaisySP, and DaisyBrain context when needed.

**Tech Stack:** Markdown instruction files, local repo docs, DaisyExamples workflow, DaisyBrain notebook context

---

### Task 1: Add local theory workspace entry points

**Files:**
- Create: `DaisyTheory/README.md`
- Create: `DaisyTheory/AGENTS.md`

**Step 1: Draft the local README**

Write a short `README.md` that explains:
- `DaisyTheory/` is a theory-first workspace for audio microcontrollers, DSP, DAFX, and language/runtime questions
- it is a sharable conceptual surface for neighboring DaisyExamples work
- it may consult `../AGENTS.md`, `../README.md`, `../LATEST_PROJECTS.md`, and DaisyBrain docs for context

**Step 2: Draft the local AGENTS file**

Write `AGENTS.md` with:
- theory-first default behavior
- scoped-write rules for implementation tasks
- cross-awareness rules for DaisyExamples, DaisyDAFX, DaisySP, and DaisyBrain
- non-destructive editing constraints

**Step 3: Review readback**

Run:

```powershell
Get-Content .\README.md
Get-Content .\AGENTS.md
```

Expected: both files exist and describe local-only theory behavior plus shared-context routing.

### Task 2: Add thin wrapper files

**Files:**
- Create: `DaisyTheory/CODEX.md`
- Create: `DaisyTheory/CLAUDE.md`
- Create: `DaisyTheory/CHATGPT.md`
- Create: `DaisyTheory/GEMINI.md`
- Create: `DaisyTheory/OPENCODE.md`
- Create: `DaisyTheory/KILO.md`

**Step 1: Write thin wrappers**

Each wrapper should:
- say to read local `AGENTS.md` first
- stay short
- remind the agent that theory prompts default to explanation, not edits

**Step 2: Read back one wrapper and compare**

Run:

```powershell
Get-Content .\CODEX.md
```

Expected: the wrapper is short and defers behavior to local `AGENTS.md`.

### Task 3: Verify containment and safety

**Files:**
- Test: `DaisyTheory/README.md`
- Test: `DaisyTheory/AGENTS.md`
- Test: `DaisyTheory/CODEX.md`
- Test: `DaisyTheory/docs/plans/2026-04-19-daisytheory-local-agent-design.md`

**Step 1: Check git diff scope**

Run:

```powershell
git diff -- DaisyTheory
```

Expected: only local `DaisyTheory/` files are added or modified.

**Step 2: Optional commit if requested**

Run:

```powershell
git add DaisyTheory
git commit -m "docs: add local DaisyTheory agent instructions"
```

Expected: only local DaisyTheory instruction files are included.
