# Daisy Project Generation Prompts

Use one of the prompt blocks below as a direct copy-paste starting point.

Replace the bracketed placeholders before sending the prompt:

- `<PROJECT_NAME>`
- `<PATCH_CONCEPT>`
- `<VOICE_MODEL>`
- `<DSP_BLOCKS>`
- `<STARTUP_DEFAULTS>`
- `<CONTROL_MAPPING_DETAILS>`
- `<OPTIONAL_INTERFACES>`
- `<KNOWN_LIMITATIONS>`

Delete lines that do not apply. Keep the board vocabulary intact.

## Daisy Field Prompt

```text
Create a complete Daisy Field project named <PROJECT_NAME>.

Patch concept:
<PATCH_CONCEPT>

Important: generate the documentation together with the code. Do not return only a .cpp file.

Deliver these files as a complete project:
- <PROJECT_NAME>.cpp
- Makefile
- README.md
- CONTROLS.md
- DIAGRAMS.md

Implementation requirements:
- Target hardware: Daisy Field
- Language: C++ with libDaisy and DaisySP
- Keep the audio callback real-time safe
- Process controls, OLED, serial logging, and MIDI in the main loop, not in the audio callback
- Clamp unstable parameter ranges where appropriate
- Use standard DaisyExamples project layout
- Voice model: <VOICE_MODEL>
- Core DSP blocks: <DSP_BLOCKS>
- External MIDI keyboard note input: required only if the project is intended for pitched keyboard-style playing. If the project is not meant to be played from piano-style keys, MIDI is optional and omitted by default unless explicitly requested
- If the project uses the Daisy Field OLED, show a compact live overview of the key parameters when controls are idle and temporarily zoom in on the parameter currently being changed
- Optional interfaces used in this project: <OPTIONAL_INTERFACES>
- Startup defaults: <STARTUP_DEFAULTS>

Required Daisy Field control mapping:
- K1: <function>
- K2: <function>
- K3: <function>
- K4: <function>
- K5: <function>
- K6: <function>
- K7: <function>
- K8: <function>
- A1..A8: <note actions, triggers, toggles, pads, or other performance actions depending on the project>
- B1..B8: <mode buttons, toggles, waveform selects, preset selects, or other actions>
- SW1: <function>
- SW2: <function>

Control mapping details:
<CONTROL_MAPPING_DETAILS>

Behavior requirements:
- The project behavior must match the hardware mapping exactly
- If the patch is intended for pitched keyboard-style playing, note input must come from an external MIDI keyboard via MIDI In and this must be documented clearly
- If the patch is not intended for piano-style note playing, MIDI may be omitted unless explicitly requested
- If the patch is monophonic, document the note priority and hold behavior explicitly
- If the patch is polyphonic, document voice allocation and steal behavior explicitly
- If a modulation path is always active, document it as always active instead of calling it routable
- Do not invent extra pages, presets, or routings unless they are described in the prompt
- Do not invent controls that are not present on Daisy Field

Makefile requirements:
- TARGET must match <PROJECT_NAME>
- CPP_SOURCES must match <PROJECT_NAME>.cpp
- Use standard DaisyExamples relative paths for libDaisy and DaisySP

Documentation requirements:

1. README.md
- High-level overview
- Feature list
- Architecture summary
- Quick control summary
- Startup/default behavior
- Build instructions
- Known limitations: <KNOWN_LIMITATIONS>

2. CONTROLS.md
- Full table for K1-K8
- Full table for A-row behavior
- Full table for B-row behavior
- SW1/SW2 behavior
- OLED behavior if used, including idle overview content and the focused parameter view shown while a control is being edited
- Serial boot behavior if used
- MIDI behavior: if MIDI is used, document the external keyboard note input, velocity mapping, channel, and any CC assignments
- Runtime behavior notes such as hold, note priority, toggles, pages, or presets
- Fixed internal settings that are not exposed on the panel

3. DIAGRAMS.md
- Mermaid block diagram
- Mermaid audio signal flow diagram
- Mermaid control/event flow diagram

Output format:
- First show the project file tree
- Then provide the full content of each file in separate fenced code blocks
- End with a short summary explaining how the documentation matches the code

Quality requirements:
- No placeholders left unresolved
- No TODO sections
- No invented controls
- Use ASCII only
- Keep the documentation concrete and operator-facing rather than generic
```

## Daisy Pod Prompt

```text
Create a complete Daisy Pod project named <PROJECT_NAME>.

Patch concept:
<PATCH_CONCEPT>

Important: generate the documentation together with the code. Do not return only a .cpp file.

Deliver these files as a complete project:
- <PROJECT_NAME>.cpp
- Makefile
- README.md
- CONTROLS.md
- DIAGRAMS.md

Implementation requirements:
- Target hardware: Daisy Pod
- Language: C++ with libDaisy and DaisySP
- Keep the audio callback real-time safe
- Process buttons, encoder, LEDs, serial logging, and MIDI in the main loop, not in the audio callback
- Clamp unstable parameter ranges where appropriate
- Use standard DaisyExamples project layout
- Voice model: <VOICE_MODEL>
- Core DSP blocks: <DSP_BLOCKS>
- External MIDI keyboard note input: required only if the project is intended for pitched keyboard-style playing. If the project is not meant to be played from piano-style keys, MIDI is optional and omitted by default unless explicitly requested
- Optional interfaces used in this project: <OPTIONAL_INTERFACES>
- Startup defaults: <STARTUP_DEFAULTS>

Required Daisy Pod control mapping:
- Knob 1: <function>
- Knob 2: <function>
- Encoder turn: <function>
- Encoder press: <function>
- Button 1: <function>
- Button 2: <function>
- Button combos or long-press actions: <if any>
- LED1: <meaning>
- LED2: <meaning>

Control mapping details:
<CONTROL_MAPPING_DETAILS>

Behavior requirements:
- The project behavior must match the hardware mapping exactly
- If the patch is intended for pitched keyboard-style playing, note input must come from an external MIDI keyboard via MIDI In
- If the patch is not intended for piano-style note playing, MIDI may be omitted unless explicitly requested
- If the design uses pages, document how page switching works
- If the design uses combo actions or long-press actions, document them explicitly
- Do not invent an OLED or keyboard controls, because Daisy Pod does not have them
- Do not invent extra controls that are not present on Daisy Pod
- If MIDI is used, document MIDI port and behavior explicitly, including note range, velocity response, and channel

Makefile requirements:
- TARGET must match <PROJECT_NAME>
- CPP_SOURCES must match <PROJECT_NAME>.cpp
- Use standard DaisyExamples relative paths for libDaisy and DaisySP

Documentation requirements:

1. README.md
- High-level overview
- Feature list
- Control summary
- Startup/default behavior
- LED feedback summary
- Build instructions
- Known limitations: <KNOWN_LIMITATIONS>

2. CONTROLS.md
- Full table for knobs
- Encoder turn/press behavior
- Button behavior
- Combo or long-press behavior if used
- LED meanings
- Serial boot behavior if used
- MIDI behavior: if MIDI is used, document the external keyboard note input, velocity mapping, channel, and any CC assignments
- Runtime behavior notes such as page model, toggles, or pattern editing rules
- Fixed internal settings that are not exposed on the panel

3. DIAGRAMS.md
- Mermaid block diagram
- Mermaid audio signal flow diagram
- Mermaid control/event flow diagram

Output format:
- First show the project file tree
- Then provide the full content of each file in separate fenced code blocks
- End with a short summary explaining how the documentation matches the code

Quality requirements:
- No placeholders left unresolved
- No TODO sections
- No invented controls
- Use ASCII only
- Keep the documentation concrete and operator-facing rather than generic
```
