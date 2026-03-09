# Field_MFOS_NoiseToaster HTML Reference Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a single self-contained HTML reference artifact that documents the current firmware block diagram and visually maps Daisy Field controls onto the Noise Toaster front panel, with an additional interactive brainstorm mode.

**Architecture:** The implementation uses one static HTML file with embedded CSS and JavaScript. A shared in-file data model defines firmware blocks, control mappings, panel hotspots, and brainstorm notes so the reference and interactive modes stay aligned.

**Tech Stack:** HTML, CSS, vanilla JavaScript, PowerShell regression check

---

### Task 1: Add the regression check

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`
- Target: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`

**Step 1: Write the failing test**

Create a PowerShell script that:
- fails if the HTML file does not exist
- fails if required section markers are missing
- fails if representative mappings like `K1 -> VCO Frequency`, `B3 -> VCF Mod Source`, and `SW1 -> Manual Gate` are missing

**Step 2: Run test to verify it fails**

Run: `powershell -ExecutionPolicy Bypass -File MyProjects\_projects\Field_MFOS_NoiseToaster\tests\check_reference_html.ps1`

Expected: FAIL because the HTML artifact does not exist yet.

**Step 3: Commit**

Skip commit unless explicitly requested.

### Task 2: Implement the single-file HTML artifact

**Files:**
- Create: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster.cpp`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/CONTROLS.md`
- Reference: `MyProjects/_projects/Field_MFOS_NoiseToaster/DIAGRAMS.md`
- Asset: `MyProjects/_projects/Field_MFOS_NoiseToaster/4-8_NT_controls_front_panel.png`

**Step 1: Build the page shell**

Add:
- page header
- mode toggle (`Reference`, `Brainstorm`)
- main diagram section
- panel overlay section
- legend / inspector section

**Step 2: Add the shared data model**

Define JavaScript objects/arrays for:
- firmware blocks
- connection metadata
- panel hotspots
- Daisy mappings
- status labels
- brainstorm notes

**Step 3: Build the firmware diagram**

Render blocks for:
- Performance Logic
- VCO
- LFO
- White Noise
- VCF
- AREG
- VCA
- Output

Show solid audio connections and dashed modulation/control connections.

**Step 4: Build the front-panel mapping overlay**

Use the existing panel PNG as the visual base.

Add positioned callouts for:
- `K1..K8`
- `B1..B5`
- `SW1`, `SW2`
- `A1..A8`
- fixed internal items

**Step 5: Build the interactive brainstorm inspector**

Clicking a block or hotspot should show:
- firmware role
- mapped Daisy control
- analog reference counterpart
- status
- brainstorm note

**Step 6: Add visual polish**

Apply the approved service-manual / synth-lab visual direction with:
- strong typography
- technical borders and labels
- subtle hover/selection animation
- color-coded path and status language

### Task 3: Verify

**Files:**
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/Field_MFOS_NoiseToaster_reference.html`
- Verify: `MyProjects/_projects/Field_MFOS_NoiseToaster/tests/check_reference_html.ps1`

**Step 1: Run regression check**

Run: `powershell -ExecutionPolicy Bypass -File MyProjects\_projects\Field_MFOS_NoiseToaster\tests\check_reference_html.ps1`

Expected: PASS

**Step 2: Run diff hygiene**

Run: `git diff --check -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`

Expected: no whitespace errors

**Step 3: Review diff**

Run: `git diff -- docs/plans MyProjects/_projects/Field_MFOS_NoiseToaster`

Expected: only the planned design/plan/test/html additions

**Step 4: Commit**

Skip commit unless explicitly requested.
