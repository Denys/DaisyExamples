# Field_MFOS_NoiseToaster Reference Diagrams Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Update the prompt and documentation for `Field_MFOS_NoiseToaster` with PDF-backed analog reference diagrams and a revised analog-faithfulness analysis.

**Architecture:** Keep the current firmware documentation honest by separating three layers: the analog reference redraw, the current firmware-accurate block diagram, and the target post-improvement architecture. Patch the reusable generation prompt so note-playing projects ask for external MIDI only when needed, and so Daisy Field projects explicitly specify the OLED idle-overview plus active-parameter zoom behavior.

**Tech Stack:** Markdown, Mermaid, Daisy Field project conventions, PDF text extraction with local Python packages

---

### Task 1: Record The Approved Design

**Files:**
- Create: `docs/plans/2026-03-07-field-mfos-noisetoaster-reference-diagrams-design.md`

**Step 1: Write the design doc**

Include:

- the approved three-diagram rule
- the conditional MIDI rule
- the Daisy Field OLED rule
- the PDF-backed analog-faithfulness interpretation

**Step 2: Verify the file exists**

Run: `rg --files docs/plans`
Expected: the new design file is listed

### Task 2: Patch The Reusable Prompt

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`

**Step 1: Make MIDI conditional**

Update both Field and Pod prompt blocks so:

- external MIDI keyboard note input is required only for pitched keyboard-style performance projects
- MIDI is optional and omitted by default for non-keyboard-style projects unless explicitly requested

**Step 2: Add the Daisy Field OLED behavior requirement**

Add wording that the OLED should:

- show a compact overview of key live parameters when controls are idle
- switch temporarily to a zoomed view of the parameter being edited when a control changes

### Task 3: Rebuild The Diagram Documentation

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`

**Step 1: Add the analog reference redraw**

Create a Mermaid redraw of Figure 4-2 using the analog block relationships from the Ray Wilson book.

**Step 2: Add the current firmware block diagram**

Create a Mermaid block diagram that matches the current C++ implementation exactly, even where the design diverges from the analog original.

**Step 3: Add the target post-improvement block diagram**

Create a Mermaid block diagram showing the proposed next-step architecture for a more analog-faithful Daisy Field adaptation.

**Step 4: Preserve supporting flow diagrams**

Keep or refresh the audio signal flow and control/event flow diagrams so the docs remain useful for the current firmware.

### Task 4: Refresh The Analog-Faithfulness Notes

**Files:**
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Modify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`

**Step 1: Update the README**

Add:

- the local analog reference source
- a concise analog-gap summary
- the revised "most analog-faithful improvement per unit of work" order

**Step 2: Update CONTROLS.md**

Add a short section listing the analog controls and routing behaviors that are not currently exposed in the Field firmware.

### Task 5: Verify The Documentation

**Files:**
- Verify: `docs/plans/2026-03-07-field-mfos-noisetoaster-reference-diagrams-design.md`
- Verify: `docs/plans/2026-03-07-field-mfos-noisetoaster-reference-diagrams.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/GENERATION_PROMPT.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/README.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`

**Step 1: Read the changed files back**

Run: `Get-Content` on the modified Markdown files.
Expected: the new prompt rules, analog-reference diagram section, and updated analog-gap notes are present.

**Step 2: Check patch integrity**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: no output

**Step 3: Review the diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`
Expected: only the intended Markdown files changed
