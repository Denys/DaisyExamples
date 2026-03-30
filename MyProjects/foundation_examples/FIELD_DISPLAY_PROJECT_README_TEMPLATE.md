# Field Display Project README Template

Use this template for any Daisy Field project that has an OLED or other display.
Copy the sections into the project README and fill in the project-specific values.

## Purpose

Describe the instrument or utility in one short paragraph:

- What the project does
- What kind of synth or processor it is
- Whether it plays notes, processes audio, or both
- What the intended live workflow is

## Controls

Document every live control in a table.

| Control | Role | Notes |
|---|---|---|
| `K1` - `K8` | Primary parameters | List the exact parameter names |
| `SW1`, `SW2` | Page, bank, or mode switching | Document hold, toggle, and long-press behavior |
| `A1` - `A8`, `B1` - `B8` | Keybed controls or note triggers | If used as controls, explain the LED meaning |
| MIDI input | External notes or controllers | List the CCs and note behavior |

If the keybed is used for control instead of notes, document the LED vocabulary clearly:

- `Off`
- `Blink`
- `On`

Use those three states whenever they help communicate a 3-state control, such as a 3-position mode selector.

## LED States

Document what each LED group means.

- Knob LEDs
- Key LEDs
- Switch LEDs
- Any special mode indicators

If LEDs show stored parameter values, say so explicitly.
If LEDs show page state, note whether they reflect the current bank, the stored value, or the physical control position.

## OLED Pages

Describe every OLED page or screen.

| Page | What it shows | When it appears |
|---|---|---|
| Overview | Main operating state | Default screen |
| Edit | Active parameter detail | When a parameter is selected |
| Hidden page | Secondary or advanced controls | When the hidden bank or page is active |

If the display has zoomed or compact variants, document both.
If pages are reached through holds or modifier keys, document the exact gesture.

## Hidden Banks / Pages

Document any secondary control layers.

- Hidden knob bank
- Alt page
- Shift or hold page
- Reserved keys or inactive parameters

Explain whether switching pages:

- preserves stored values
- uses pickup or catch
- changes LED behavior
- changes the OLED page

## Startup / Default Values

Document startup defaults explicitly.

List the starting value for every important parameter, mode, and state:

- Voice mode
- Polyphony
- Oscillator or synthesis model
- Filter or resonator settings
- Envelope or exciter behavior
- Stereo or spread settings
- Bank selection
- OLED page
- LED state defaults

Do not say "uses sensible defaults" without enumerating them.
If a value is hidden behind a modifier or bank, still document its startup value.

## Panic / Reset

Document how to recover from stuck notes, bad states, or silent output.

- Panic action
- MIDI all-notes-off behavior
- Reset to defaults
- Re-sync of display and LED state

If the project has a safe startup reset or a maintenance shortcut, document it here.

## MIDI / External Control

Document external controllers and exact mappings.

- Note input
- Velocity
- Sustain
- CC mappings
- Clock or transport if used

If an external controller is expected, say which one fits best and why.

## Maintenance Notes

Keep this section for implementation reminders that future edits should preserve.

- Stored parameter values should survive bank or page switches
- Knob LEDs should represent the stored logical value, not the raw knob position
- Do not waste a knob on generic output level unless the synth architecture needs it
- Update the README whenever the OLED pages, LED semantics, or control banks change

