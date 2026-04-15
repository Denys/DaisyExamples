# Daisy QAE Reasonable Controls Policy Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a general Daisy QAE policy that guides control-curve and control-range recommendations across Pod, Seed, and Field projects.

**Architecture:** Update `DAISY_DEVELOPMENT_STANDARDS.md` only. Add a short reminder in the common-pitfalls table, then add a new global policy section near the platform adaptation material so QAE can reason about curve choice, useful ranges, and instability-prone parameter interactions.

**Tech Stack:** Markdown documentation

---

### Task 1: Add the new policy text

**Files:**
- Modify: `DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

**Step 1: Add a common-pitfalls reminder**

- Add one row telling QAE not to expose raw algorithm extremes by default
- Point readers to the new controls policy section

**Step 2: Add a new general controls policy section**

- Apply it to all Daisy targets
- Cover:
  - curve selection (`linear`, `exponential`, `log`)
  - meaningful range selection
  - instability and interaction awareness
  - justification when full extremes are intentionally exposed

**Step 3: Update version metadata**

- bump document version
- update last-updated date
- add changelog entry

### Task 2: Verify the document update

**Files:**
- Test: `DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md`

**Step 1: Check inserted sections**

Run:

```powershell
rg -n "Reasonable Controls Policy|curve|exponential|log|instability|changelog" DaisyExamples/DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md
```

Expected:

- the new policy section is present
- the pitfalls reminder is present
- the changelog entry is present

**Step 2: Review for consistency**

- verify the wording is advisory rather than absolute
- verify it applies to all Daisy targets rather than only Seed/Field
