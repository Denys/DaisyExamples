createvisual  HTML based equivalent to 4-2_Noise_Toaster_Block_Diagram block diagram based on FW Field_MFOS_NoiseToaster.cpp
and FW controls maping to 4-8_NT_controls_front_panel -> visually indicate on the front panel which control of field are maped to control of noise toaster, ie K1 -> VCO-Frequency

---

Using brainstorming for the design phase. After approval, I’ll implement it as a single self-contained HTML with frontend-design.

Three viable approaches:

1. Recommended: technical reference sheet
	* One HTML page with two linked panels: a firmware block diagram and a front-panel mapping overlay.
	* Pros: closest to your request, easy to open locally, easy to keep in sync with Field_MFOS_NoiseToaster.cpp, good for documentation.
	* Cons: less “poster-like” than a pure visual showcase.
2. Poster-style infographic
	* One HTML page that looks like a synth manual poster, with the diagram and mapping merged into one dramatic composition.
	* Pros: strongest visual impact.
	* Cons: harder to read precisely, and harder to maintain when firmware mappings change.
3. Interactive mini-app
	* One HTML page with tabs, filters, hover states, and drill-down sections for each subsystem/control.
	* Pros: richest interaction.
	* Cons: more JS and more complexity than the problem needs.

My recommendation is 1: a technical reference sheet with strong visual styling.

Design
I’d structure the page as a single landscape-style document with three sections:

	* Firmware Block Diagram
	Recreate the spirit of 4-2_Noise_Toaster_Block_Diagram, but based strictly on the current firmware. Use custom HTML/CSS blocks and SVG connectors for Performance Logic, VCO, LFO, White Noise, VCF, AREG, VCA, and Output. Solid lines for audio path, dashed lines for modulation/control.

	* Front Panel Mapping
	Use 4-8_NT_controls_front_panel.png as the visual base, then overlay labeled callouts showing which Daisy Field controls map to which Noise Toaster function, for example K1 -> VCO Frequency, K7 -> AREG Attack, B3 -> VCF Mod Source, SW1 -> Manual Gate. Unmapped analog-original controls would be visually muted and labeled as fixed internal or not implemented.

	* Mapping Legend
	A compact table/list underneath that groups mappings by subsystem and explicitly calls out fixed internal values like LFO Rate 2.2 Hz, Noise Blend 18%, Output Level 72%.

Interaction would be lightweight:

	* Hover a block in the diagram highlights the related panel callouts.
	* Hover a panel callout highlights the related block(s).
	* Click locks a highlight so the page can be used as a static explainer while reading.

---

create 1 and 3.
1 is what I wanted. 3 can be useful in future for interactive user controls interface brainstorm

---

Updated recommendation: one self-contained HTML with two modes on the same page.

Structure

	* Reference Mode
	This is the main deliverable. It shows the firmware block diagram plus the overlaid 4-8 front-panel mapping exactly as documentation.

	* Interactive Brainstorm Mode
	This is a second section or toggle in the same HTML. It reuses the same blocks and mappings, but makes them clickable as “candidate controls” for future UI ideation. Example: clicking LFO or B3 can expose notes like current firmware, fixed internal, missing analog-original control, possible future remap.

	* Shared Data Model
	Both modes use the same mapping definitions in JS so the block diagram, panel callouts, legend, and future brainstorming notes stay consistent.

This avoids making two separate files while still giving you both the exact reference sheet and a foundation for future control-interface exploration.

---

Created Field_MFOS_NoiseToaster_reference.html as a single self-contained HTML artifact. It includes the firmware-derived block diagram, the 4-8 front-panel overlay with Daisy Field mappings like K1 -> VCO Frequency, B3 -> VCF Mod Source, SW1 -> Manual Gate, and a second Interactive Brainstorm Mode for future control-surface ideas. I also added the regression check at check_reference_html.ps1.

I documented the approved design and plan in 2026-03-08-field-mfos-noisetoaster-html-reference-design.md and 2026-03-08-field-mfos-noisetoaster-html-reference.md. Verification run: check_reference_html.ps1 passed, and git diff --check was clean. I did not run a browser-based visual inspection in this environment, so the artifact is verified structurally and by content checks, not by screenshot review.
