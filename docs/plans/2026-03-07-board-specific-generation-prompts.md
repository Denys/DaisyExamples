# Board-Specific Generation Prompts Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the single project-specific generation prompt with two reusable copy-paste prompts, one for Daisy Field and one for Daisy Pod.

**Architecture:** Keep one Markdown file as the entry point, but split its core content into two standalone prompt blocks. Each block uses the hardware vocabulary and documentation expectations of its board so the user can copy one section directly into an AI chat with minimal edits.

**Tech Stack:** Markdown, Daisy project conventions, libDaisy/DaisySP project structure

---

### Task 1: Record The Approved Design

**Files:**
- Create: `docs/plans/2026-03-07-board-specific-generation-prompts-design.md`

**Step 1: Write the design doc**

Include:

- why the prompt should stay in one file
- why there should be two standalone board sections
- the expected Daisy Field and Daisy Pod hardware vocabulary

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: the new design file is listed

### Task 2: Rewrite The Prompt File

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`

**Step 1: Replace the project-specific prompt with a board-specific prompt kit**

The rewritten file should include:

- short intro
- short placeholder replacement notes
- Daisy Field prompt block
- Daisy Pod prompt block

**Step 2: Keep the prompts self-contained**

Each prompt must include:

- required files
- implementation rules
- board-specific control mapping placeholders
- documentation requirements
- output format
- quality guardrails

### Task 3: Verify The Prompt Content

**Files:**
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`

**Step 1: Read the file back**

Run: `Get-Content MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`
Expected: both `Daisy Field Prompt` and `Daisy Pod Prompt` sections are present

**Step 2: Check for patch issues**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`
Expected: no output

**Step 3: Review the diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`
Expected: only the prompt file and plan docs changed
