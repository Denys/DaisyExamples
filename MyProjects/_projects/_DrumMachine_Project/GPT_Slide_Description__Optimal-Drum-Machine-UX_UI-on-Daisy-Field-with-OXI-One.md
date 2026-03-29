## Presentation draft

Below is a **complete slide-by-slide deck** you can paste into PowerPoint/Google Slides directly.

---

# 1. Title

**Optimal Drum Machine UX/UI on Daisy Field with OXI One**
**Designing a focused drum-synth/performance surface instead of a cramped standalone groovebox**

**Subtitle:**
Using Daisy Field for local sound, focused editing, and performance; using OXI One for structural sequencing

---

# 2. Problem statement

## Goal

Build a **good exploration prototype** of a drum machine on **Daisy Field + DaisySP** without fighting the hardware UI constraints.

## Daisy Field constraints

* 8 encoders with LEDs
* 16 LED keys
* 2 switches
* small OLED
* fixed front-panel layout
* no dedicated per-track knobs/faders
* no large grid

## Key design challenge

A good drum machine needs:

* immediate track focus
* fast step entry/editing
* performance functions
* low cognitive overhead
* clear LED semantics

But Daisy Field does **not** have enough dedicated controls to copy a TR-style front panel directly.

## Thesis

Do **not** make Daisy Field a full sequencer UI if OXI One is already present.
Make it a **focused local drum instrument**:

* **OXI One** = transport, structure, deep sequencing
* **Daisy Field** = sound, track focus, local performance, quick step overlay

---

# 3. What strong drum-machine UX usually gets right

## Core UX concepts

1. **Immediate temporal model**

   * user always knows what track and what step is active

2. **Per-track focus**

   * deep editing usually happens on one voice at a time

3. **Low mode anxiety**

   * temporary modifier layers are fine
   * hidden persistent modes are expensive

4. **Performance safety**

   * mutes, fills, rolls must be fast
   * destructive actions must be harder to trigger

5. **Stable visual language**

   * LED states must be globally consistent

## Design consequence

Daisy Field should optimize for:

* **track-centric interaction**
* **temporary step view**
* **temporary performance layer**
* **utility behind deeper gesture**

---

# 4. Why OXI One should carry the structural sequencing load

OXI One is already designed as a performance sequencer with **4 independent sequencers**, shift-driven secondary access, multitrack behavior, page/follow logic, step parameters, copy/paste workflows, randomization, and performance-oriented sequencing. Its multitrack mode provides **8 mono tracks** and explicit per-track editing/shortcuts.  

## Best functions to assign to OXI

* clock / transport
* long pattern editing
* probability / conditions
* ratchets / retrigs
* microtiming
* pattern changes
* arranger / song structure
* cross-track structural operations

## Why

Those are exactly the functions that would otherwise consume most of Daisy Field’s keys and modes.

---

# 5. Recommended division of responsibilities

| Function                     | OXI One |            Daisy Field | Reason                                |
| ---------------------------- | ------- | ---------------------: | ------------------------------------- |
| Clock / start / stop         | Primary |            status only | Keep one transport authority          |
| Long pattern editing         | Primary |                     no | OXI already solves it better          |
| Per-step conditions          | Primary | optional local overlay | Avoid duplicating dense sequencing UI |
| Track selection              | shared  |       Primary local UI | Must stay tactile on Daisy            |
| Voice synthesis              | no      |                Primary | Core value of DaisySP drum machine    |
| Local fills / rolls / macros | shared  |                Primary | Best use of Daisy keys                |
| Kit / page / sound browse    | no      |                Primary | Local instrument behavior             |
| Arranger / song mode         | Primary |                     no | Prevent UI bloat                      |

---

# 6. Main architecture for Daisy Field

## 4-layer model

### Layer 1 — Base

**Track Focus / Status**

### Layer 2 — SW1 held

**Step Overlay**

### Layer 3 — SW2 held

**Performance Layer**

### Layer 4 — SW1 + SW2 held

**Utility / Service Layer**

## Why this is optimal

* only **4 semantic layers**
* both switches act as true modifiers
* no deep menu tree
* default mode stays musical
* OLED only has to describe **current focus**, not full arrangement state

---

# 7. Base layer: default interaction model

## Principle

Default state must answer:

* **which voice is selected?**
* **is external clock running?**
* **which page am I editing?**
* **what is currently sounding?**

## Suggested 16-key map

### Row A — voices

* A1 = Kick
* A2 = Snare
* A3 = Low Tom
* A4 = Mid Tom
* A5 = High Tom / Clap
* A6 = Closed Hat
* A7 = Open Hat
* A8 = Cymbal / Perc

### Row B — local functions

* B1 = Mute context
* B2 = Solo context
* B3 = Local record / arm
* B4 = Performance entry anchor
* B5 = Kit / preset browser
* B6 = Page select
* B7 = FX / bus page
* B8 = Sync / run status

## LED semantics in base layer

* **Off** = idle / not selected
* **On** = selected / latched
* **Blink** = current-time activity (incoming hit / run status)

---

# 8. Step overlay layer (SW1)

## Purpose

Temporary **16-step view** for the **currently selected track only**

## Why temporary

OXI already owns the full pattern/timeline.
Daisy only needs:

* quick step inspection
* punch-in edits
* immediate current-track correction

## Step LED meanings

* **Off** = no trig
* **On** = trig stored
* **Blink** = playhead

## Interaction

* tap = toggle trig
* hold step + encoder = edit local step parameters
* release SW1 = return to track view immediately

## Optional local step parameters

Use only if useful locally:

* accent / velocity
* decay override
* tune offset
* ratchet
* flam spread
* local probability
* micro offset

If OXI is already doing advanced step conditions, keep Daisy’s local step edits minimal.

---

# 9. Performance layer (SW2)

## Main idea

This is where OXI frees the interface.

Without OXI, these keys would be spent on sequencing administration.
With OXI, they become **musical controls**.

## Suggested 16-key map

### Fills / transitions

* FILL 1
* FILL 2
* FILL 3
* FILL 4

### Mute groups

* MUTE A
* MUTE B
* MUTE C
* MUTE D

### Sound macros

* DECAY
* DRIVE
* CRUSH
* TONE

### Time / FX actions

* ROLL
* PROB macro
* FX 1
* FX 2

## LED behavior

* **Off** = inactive
* **On** = latched
* **Blink** = momentary active

## Design rule

Prefer **momentary** behavior for performance actions unless latch is musically obvious.

---

# 10. Utility layer (SW1 + SW2)

## Purpose

Low-frequency actions only

## Suggested assignments

* COPY
* PASTE
* CLEAR
* DUPL
* LEN-
* LEN+
* NUDGE-
* NUDGE+
* CHOKE
* ROUTE
* MIDI
* CC MAP
* SAVE
* LOAD
* BACK
* OK

## Safety rules

* enter only while both switches held
* long-press confirm for destructive actions
* OLED confirmation for overwrite/clear
* auto-return to base when action completes

## Why this matters

It prevents utility functions from polluting the performance surface.

---

# 11. Encoder architecture

## Best page design

Each page should be coherent. Do **not** mix unrelated parameters.

### Page A — Transient / Body

* Tune
* Decay
* Attack
* Pitch Env
* Click / Snap
* Body
* Level
* Pan

### Page B — Tone / Filter

* Filter
* Resonance
* Drive
* Fold / Clip
* Noise
* Damping
* Brightness
* Character

### Page C — Mod

* LFO amount
* LFO rate
* Env mod
* Velocity depth
* Accent depth
* Macro destination
* Macro amount
* Alt modulation

### Page D — FX / Bus

* Send A
* Send B
* Compressor amount
* Bus drive
* Choke group
* Output route
* Reverb send
* Delay send

## Rule

When the selected track changes, encoders remain mapped to the **same page schema**.
Only the **voice target** changes.

That preserves muscle memory.

---

# 12. OLED policy

## OLED should show only focused information

### Recommended layout

* **Line 1:** mode | sync | tempo / ext clock
* **Line 2:** selected track | current page
* **Line 3:** 8 short parameter labels
* **Line 4:** values / mini bars / status icons

## OLED should not become

* arranger browser
* deep file manager
* global song editor
* long text menu stack

## Reason

The OLED is too small for hierarchy-heavy workflows.
It should support the tactile surface, not replace it.

---

# 13. LED language with only 3 states

## Recommended universal semantic model

### Off

Absent / idle / empty / unavailable

### On

Selected / stored / latched / focused

### Blink

Current-time event:

* playhead
* incoming hit
* active fill
* armed recording
* pending save/confirm

## Strong rule

**Blink must always mean temporal state.**

Do **not** reuse blink for:

* “selected”
* “menu available”
* “warning”
* “currently playing”
  all at once

That destroys readability.

---

# 14. Firmware/UI implementation model on Daisy

## Recommended state model

```text
AppState
- currentLayer
- focusedTrack
- currentPage
- extClockPresent
- transportRunning
- recArmed
- heldStep
- heldMacro
- pendingUtilityAction
- uiDirtyFlags
```

## Recommended processing order

1. scan keys / switches / encoders
2. resolve modifier state first
3. interpret controls according to current layer
4. update central UI state
5. emit engine commands / MIDI reactions
6. render LEDs + OLED from state

## Important rule

Render UI from the **state model**, not from scattered ad-hoc flags.

That avoids:

* inconsistent LEDs
* stuck temporary modes
* hard-to-debug modifier interactions

---

# 15. Real-time architecture

## Suggested split

### Audio callback

* voice DSP only
* no UI logic
* no OLED rendering

### MIDI event queue

* note triggers
* CCs
* transport
* external sync messages

### UI tick

* key scan
* switch scan
* encoder deltas
* gesture detection

### Display / LED tick

* OLED redraw at slower rate
* LED blink phase generation
* activity decay indicators

## Practical recommendation

Keep UI and audio cleanly separated.
A drum-machine prototype becomes unpleasant quickly if UI logic leaks into timing-critical DSP.

---

# 16. Suggested MVP scope

## Minimum viable prototype

* 8 drum voices
* 4 UI layers
* 4 encoder pages
* external clock + note input from OXI
* base track-focus layer
* step overlay for selected track
* performance layer with fills / rolls / macros
* utility layer with copy/clear/save
* kit / preset browsing
* consistent 3-state LED language

## Explicitly defer

* full song mode
* complex sample management
* standalone deep sequencer UI
* large pattern storage browser
* Elektron-level per-step condition menus on Daisy

---

# 17. Validation plan

## UX tests

1. **Track focus speed**

   * can user switch voice and edit sound immediately?

2. **Step correction speed**

   * can user quickly fix one track while OXI runs?

3. **Performance confidence**

   * are fills / rolls / macros usable without accidental utility entry?

4. **LED clarity**

   * can user interpret off/on/blink correctly under playback?

5. **OLED readability**

   * are page names and values readable at performance distance?

## Engineering tests

* OXI transport sync stability
* note trigger latency
* no stuck modifier states
* no accidental destructive actions
* deterministic redraw timing
* safe UI behavior during heavy clock traffic

---

# 18. Final conclusion

## Verdict

The optimal UX/UI for this project is **not** a tiny standalone groovebox.

It is a **focused drum-synth/performance instrument** with:

* **OXI One** handling structure, timeline, and deep sequencing
* **Daisy Field** handling sound, track focus, performance, and quick local edits

## Best design principles

* track focus by default
* step view only as temporary overlay
* performance layer as first-class citizen
* utility behind deeper modifier gesture
* one stable LED language
* OLED only for focused context
* central UI state model in firmware

## Final statement

If implemented this way, Daisy Field becomes a **strong instrument**.
If it tries to duplicate OXI, it becomes a **compromised mini workstation**.

---

## Source basis

Key OXI workflow claims above are grounded in the uploaded **OXI One User Manual v5.6** and **OXI One Quick Guide 4.2**, including the points that OXI has **4 fully independent sequencers**, a workflow intended to avoid deep nested menus, and a **Multitrack mode with 8 tracks** plus step/performance workflows.  

If you want, the next step can be one of these:

1. convert this into a **real slide deck structure** with slide titles + exact bullet limits
2. turn it into a **firmware state chart + control map spec**
3. produce a **complete per-key / per-encoder mapping table** for implementation
