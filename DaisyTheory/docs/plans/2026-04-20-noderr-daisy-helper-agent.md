# Noderr Daisy Helper Agent Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Create a small Daisy-aware helper-agent layer inside `DaisyTheory/noderr/` that teaches agents how to install and interface Noderr with Daisy firmware projects.

**Architecture:** Add one always-on `AGENTS.md` and one companion runbook under `DaisyTheory/noderr/`. Keep the helper local and lightweight while routing any real firmware Noderr work toward the canonical instance at `DaisyExamples/noderr/`.

**Tech Stack:** Markdown instruction files, DaisyExamples repo docs, existing repo-root Noderr firmware instance

---

### Task 1: Add the local helper-agent surface

**Files:**
- Create: `DaisyTheory/noderr/AGENTS.md`
- Create: `DaisyTheory/noderr/DAISY_HELPER_AGENT.md`

**Step 1: Draft `AGENTS.md`**

Write a short local `AGENTS.md` that:
- defines `DaisyTheory/noderr/` as a helper adapter, not the canonical firmware Noderr instance
- routes agents to `../../noderr/INSTANCE_IDENTITY.md`, `../../noderr/noderr_project.md`, and `../../noderr/environment_context.md`
- requires an exact target project path before install/reconcile work
- warns against web assumptions like `npm`, preview URLs, and `UI_`/`API_`/`SVC_` NodeIDs when the target is embedded firmware

**Step 2: Draft the runbook**

Write `DAISY_HELPER_AGENT.md` with:
- routing rules for repo-root vs project-local Noderr use
- a Daisy install checklist
- a Noderr-to-Daisy concept mapping table
- minimum validation commands and safety rules

**Step 3: Read back both files**

Run:

```powershell
Get-Content .\noderr\AGENTS.md
Get-Content .\noderr\DAISY_HELPER_AGENT.md
```

Expected: both files exist, stay Daisy-specific, and route to the canonical firmware Noderr instance instead of duplicating it.

### Task 2: Verify containment

**Files:**
- Test: `DaisyTheory/noderr/AGENTS.md`
- Test: `DaisyTheory/noderr/DAISY_HELPER_AGENT.md`
- Test: `DaisyTheory/docs/plans/2026-04-20-noderr-daisy-helper-agent-design.md`
- Test: `DaisyTheory/docs/plans/2026-04-20-noderr-daisy-helper-agent.md`

**Step 1: Check diff scope**

Run:

```powershell
git diff -- DaisyTheory
```

Expected: only the new `DaisyTheory/` docs and helper-agent files appear.

**Step 2: Optional commit if requested**

Run:

```powershell
git add DaisyTheory
git commit -m "docs: add Daisy Noderr helper agent"
```

Expected: only the local helper-agent docs are staged.
