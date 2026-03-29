## Slide deck outline — text only

Below is a **detailed slide-by-slide deck description** for a presentation about the **“Daisy Field Drum Machine UI Replica” HTML concept**, with explicit comparison to **DaisySP** capabilities.

---

## Slide 1 — Title / positioning

**Title:**
**Daisy Field Drum Machine UI Replica**
**A UI-first drum machine concept for Daisy Field, designed around OXI One and evaluated against DaisySP capabilities**

**Main content:**

* Introduce the HTML replica as a **concept model**, not a finished product UI.
* State the central design thesis:

  * **OXI One** handles structural sequencing.
  * **Daisy Field** becomes the local **drum voice / edit / performance surface**.
  * The HTML replica visualizes this split as an interaction architecture.
* State the comparison axis:

  * **Replica = UX/UI model**
  * **DaisySP = sound-generation / DSP building blocks**
* Add one sentence that frames the core question:

  * “How much of this concept is already supported directly by DaisySP, and what parts must be implemented at the application/UI layer?”

**Speaker note:**
The point of the deck is not “can DaisySP make sound?” It clearly can. The point is whether the **specific UI architecture shown in the HTML mockup** is well matched to what DaisySP provides natively versus what must be built in firmware.

---

## Slide 2 — Why this concept exists

**Title:**
**Why a replica was needed before firmware**

**Main content:**

* Daisy Field has a fixed control surface:

  * 8 encoders
  * 16 LED keys
  * 2 switches
  * OLED
* A drum machine needs:

  * track focus
  * step editing
  * performance actions
  * utility/service actions
  * clear state feedback
* The HTML replica exists to answer these questions before implementation:

  * What should the default layer be?
  * Which controls should be temporary overlays?
  * How should the 3 LED states be interpreted?
  * What belongs on OXI vs. Daisy?
* Explain that the concept is **interaction-first**, not audio-first.

**Suggested visual on slide:**

* Screenshot or cropped render of the HTML replica
* Caption: “UI architecture mockup, not final industrial design”

---

## Slide 3 — Core thesis of the HTML concept

**Title:**
**The core UX decision: Daisy should not duplicate OXI**

**Main content:**

* The replica is based on a deliberate split:

  * **OXI One = timeline / transport / deep sequencing / pattern structure**
  * **Daisy Field = focused sound control / local status / immediate performance**
* Why this split is justified:

  * OXI One has **4 fully independent sequencers** and is explicitly designed around a layout relationship that avoids heavy nested menus. It also has **Multitrack mode with 8 mono tracks**, making it well suited to structural drum sequencing.  
* Therefore the HTML concept intentionally avoids using Daisy Field as a “second OXI”.

**Speaker note:**
This is the most important architectural decision in the whole deck. Once accepted, the whole UI starts to make sense.

---

## Slide 4 — What the HTML replica actually implements

**Title:**
**What is encoded in the replica**

**Main content:**
Describe the four semantic layers in the HTML model:

1. **Base layer**

   * track focus
   * status
   * sound page entry points

2. **SW1 layer**

   * step overlay for the currently selected track

3. **SW2 layer**

   * performance macros / fills / rolls / mute groups

4. **SW1 + SW2 layer**

   * utility / service / destructive actions

Also explain the LED language encoded in the mockup:

* **Off** = absent / idle / empty
* **On** = selected / stored / latched
* **Blink** = temporal event / playhead / active fill / incoming hit

**Key point:**
The replica is not just visual. It already defines:

* control priority
* mode hierarchy
* LED semantics
* separation of musical vs. administrative operations

---

## Slide 5 — Why this is good drum-machine UX

**Title:**
**Why this layer model fits drum-machine workflow**

**Main content:**
Explain the drum-machine UX principles embedded in the concept:

* **Track-first default**

  * default state answers: “what am I editing now?”
* **Step view as temporary overlay**

  * step editing is available, but does not consume the entire default UI
* **Performance actions are first-class**

  * fills, rolls, mute groups, macros have their own privileged layer
* **Utility is hidden**

  * copy/clear/save/routing do not clutter the main interaction path
* **LED states are globally consistent**

  * no semantic drift between layers

**Suggested comparison statement:**
This is closer to a performance-oriented hardware instrument than to a mini workstation.

---

## Slide 6 — OXI responsibilities vs Daisy responsibilities

**Title:**
**Explicit task split between OXI One and Daisy Field**

**Main content:**

| Function                                    | OXI One | Daisy Field           |
| ------------------------------------------- | ------- | --------------------- |
| transport / clock                           | primary | status only           |
| pattern launch / chaining                   | primary | no                    |
| per-step conditions / probability / ratchet | primary | optional overlay only |
| track selection                             | shared  | local primary         |
| voice synthesis                             | no      | primary               |
| performance macros / rolls / fills          | shared  | primary local         |
| kit / voice pages                           | no      | primary               |
| arrangement / song structure                | primary | no                    |

**Why OXI gets the sequencing-heavy responsibilities:**

* OXI supports independent sequencers, multitrack operation, step parameters, randomization/generator tools, page handling, copy/paste, and performance sequencing functions.  

**Speaker note:**
This slide is the bridge from interaction design to implementation design.

---

## Slide 7 — DaisySP capability overview

**Title:**
**What DaisySP already gives the concept**

**Main content:**
DaisySP is an open-source DSP library aimed at embedded Daisy hardware and other audio-software contexts. Its documented feature set includes:

* control generators: AD, ADSR, phasor
* drum synthesis: analog/synthetic bass drum, snare drum, hi-hat
* dynamics
* effects: phaser, wavefolder, decimate, overdrive
* filters
* noise generators
* physical modeling
* granular player
* utility DSP blocks such as looper and signal conditioning ([electro-smith.github.io][1])

**Key framing statement:**
DaisySP clearly provides **sound-generation primitives and processing blocks**, but it does **not** present itself as a finished drum-machine application framework. That is an inference from the documented feature set and API style. ([electro-smith.github.io][1])

---

## Slide 8 — The central comparison

**Title:**
**Replica concept vs DaisySP: what is native, what is not**

**Main content:**

| Replica element                      | DaisySP support    | Comment                                                 |
| ------------------------------------ | ------------------ | ------------------------------------------------------- |
| kick / snare / hat engines           | strong             | directly supported by drum modules                      |
| envelopes / modulation               | strong             | control generators and modulation-friendly blocks exist |
| distortion / crush / tone shaping    | strong             | overdrive, decimate, filters, wavefolder available      |
| voice-level parameter pages          | medium-strong      | easy to build from module APIs                          |
| step sequencing UI                   | weak / none native | must be app logic                                       |
| kit browser / save/load / routing UI | weak / none native | must be application layer                               |
| track focus and layer logic          | none native        | pure UI state machine                                   |
| LED semantics and OLED rendering     | none native        | firmware/UI responsibility                              |
| macro layer                          | medium             | DSP targets exist, but mapping logic must be built      |

**Conclusion of slide:**
The replica is **well aligned with DaisySP for sound and parameter targets**, but **not** for sequencing and UI state management.

---

## Slide 9 — Base layer and DaisySP mapping

**Title:**
**Base layer: best matched part of the concept**

**Main content:**
Describe the base layer from the HTML concept:

* A1–A8 = track/voice select
* B row = mute/solo/rec/perf/kit/page/fx/sync anchors

Explain why this layer fits DaisySP well:

* DaisySP gives a strong set of per-voice control targets:

  * drum voice frequency
  * decay
  * tone
  * accent
  * noise/timbre mix
  * filter / distortion / auxiliary processing blocks

Examples:

* `AnalogBassDrum` exposes `SetFreq`, `SetTone`, `SetDecay`, `SetAccent`, plus FM-related shaping controls. ([electro-smith.github.io][2])
* `SyntheticSnareDrum` exposes `SetFreq`, `SetDecay`, `SetAccent`, `SetFmAmount`, `SetSnappy`. ([electro-smith.github.io][3])
* `HiHat` exposes `SetFreq`, `SetTone`, `SetDecay`, `SetAccent`, `SetNoisiness`. ([electro-smith.github.io][4])

**Key point:**
The base layer is the strongest match because it is mainly a **voice-selection + voice-parameter** surface.

---

## Slide 10 — Step overlay and DaisySP mapping

**Title:**
**SW1 step overlay: useful UX, but not a DaisySP feature**

**Main content:**
Describe the HTML concept:

* holding SW1 turns the 16 keys into a step view for the selected track
* LED meanings:

  * off = empty
  * on = trig present
  * blink = playhead

Explain the comparison to DaisySP:

* DaisySP can render the **result** of per-step changes:

  * different decay
  * pitch offsets
  * accent
  * ratchets if implemented
* But DaisySP does **not** provide the sequencing UI model itself

  * step storage
  * playhead model
  * grid interaction
  * step edit state
  * page follow logic
    all have to be built above the DSP layer

**Recommended wording on slide:**
“DaisySP can be the engine behind the steps; it is not the step sequencer UI.”

---

## Slide 11 — Performance layer and DaisySP mapping

**Title:**
**SW2 performance layer: strong concept, strong DSP fit**

**Main content:**
Describe the HTML concept:

* fills
* mute groups
* decay macro
* drive
* crush
* tone
* roll
* FX sends

Why this layer maps well to DaisySP:

* effects processors include overdrive and decimation, which directly support drive/crush style macros. ([electro-smith.github.io][1])
* filters and tone-shaping blocks support brightness/darkness macros. ([electro-smith.github.io][1])
* drum engines already expose accent, tone, decay and related parameters. ([electro-smith.github.io][2])
* envelopes and phasor/control generators support performance modulation infrastructure. ([electro-smith.github.io][1])

**Important nuance:**

* the DSP targets exist
* macro definition, latch behavior, momentary behavior, safe range limits, and inter-parameter coupling must still be designed in firmware

---

## Slide 12 — Utility layer: almost entirely outside DaisySP

**Title:**
**SW1 + SW2 utility layer: application logic, not DSP**

**Main content:**
Describe the utility layer from the HTML concept:

* copy
* paste
* clear
* duplicate
* length
* nudge
* choke group
* route
* MIDI mapping
* save/load
* back/ok

Comparison to DaisySP:

* DaisySP has utility DSP components, but not a project-management model, pattern clipboard, routing UI, or confirmation workflows. ([electro-smith.github.io][1])
* Therefore this layer must be implemented almost entirely in:

  * application state
  * storage schema
  * UI gesture logic
  * MIDI integration code
  * render layer

**Key statement:**
This is the clearest example of where the HTML replica is a **product UX spec**, not a DSP design.

---

## Slide 13 — Voice architecture options enabled by DaisySP

**Title:**
**What voice models the concept can realistically support**

**Main content:**
Propose concrete voice strategies for the replica:

### Option A — Pure synthesized kit

* Kick = `AnalogBassDrum` or `SyntheticBassDrum`
* Snare = `SyntheticSnareDrum`
* Hats = `HiHat`
* Toms / perc = oscillator + envelopes + filters + noise blocks
* Master FX = filter + overdrive + decimator

### Option B — Hybrid synthesized kit

* Use DaisySP drums for core kit
* Add physical-model or noise-based percussion layers
* Use filters + decimate + wavefolder for aggressive techno coloration

### Option C — Experimental / modular drum engine

* Per-track voice model selectable
* Some tracks use drum models
* Some use resonator/modal/karplus-like structures
* One track reserved for granular or texture percussion

Why this is justified:

* DaisySP documents not only drum synthesis but also filters, physical modeling, noise generators, and a granular player. ([electro-smith.github.io][1])

---

## Slide 14 — Where the replica currently overshoots DaisySP

**Title:**
**Conceptual gaps: what DaisySP does not solve**

**Main content:**
List the major gaps between the HTML concept and DaisySP’s native scope:

1. **No built-in drum-machine app model**

   * no kit abstraction
   * no voice slot manager
   * no performance scene model

2. **No native step-sequencer UX**

   * no grid editing model
   * no page logic
   * no LED policy

3. **No native storage/browser layer**

   * save/load semantics are your responsibility

4. **No high-level control mapping policy**

   * “which knob controls what on which page” is not a DaisySP concern

5. **No explicit OXI integration layer**

   * MIDI parsing, transport handling, track mapping, CC mapping are external responsibilities

**Conclusion of slide:**
The HTML concept is feasible, but it is feasible because DaisySP covers the **DSP substrate**, not because it covers the whole instrument.

---

## Slide 15 — Recommended MVP interpretation of the HTML concept

**Title:**
**Recommended first firmware implementation**

**Main content:**

### Keep in MVP

* Base layer
* SW1 step overlay
* SW2 performance layer
* 4 coherent parameter pages
* 8 drum voices
* OXI external clock and note trigger
* consistent 3-state LED policy

### Defer

* deep utility workflows
* pattern browser complexity
* elaborate copy/paste model
* per-step condition depth already available on OXI
* too many voice types in one build

### MVP voice mapping suggestion

* BD = AnalogBassDrum
* SD = SyntheticSnareDrum
* CH/OH = HiHat variants
* Toms / perc = oscillator/noise/filter derived voices
* Macro effects = filter, overdrive, decimate

### Why

This gives the strongest ratio of:

* immediate usability
* low firmware complexity
* high leverage from DaisySP’s existing modules

---

## Slide 16 — Final verdict

**Title:**
**Conclusion: the HTML concept is stronger as a UI architecture than as a promise of a full standalone drum machine**

**Main content:**

* The **“Daisy Field Drum Machine UI Replica”** is a strong concept because it solves the main UX problem correctly:

  * keep Daisy focused
  * delegate sequencing depth to OXI
  * use temporary layers instead of permanent clutter
* The concept aligns **well** with DaisySP where it matters most:

  * drum voices
  * modulation targets
  * effects
  * tone shaping
* The concept is **not** “already implemented by DaisySP”

  * sequencing behavior
  * state model
  * visual policy
  * kit management
  * performance logic
    all remain product-level firmware work

**Closing line:**
The replica should be treated as a **front-end architecture spec for a DaisySP-based drum engine**, not as a thin wrapper over an existing library.

---

## Optional appendix slides

### Appendix A — Per-layer engineering ownership

* **DSP layer**: DaisySP
* **application layer**: voice manager, step state, macros
* **UI layer**: keys, OLED, LEDs, modifier logic
* **integration layer**: OXI MIDI/transport mapping

### Appendix B — Best one-to-one parameter mappings

* Kick page ↔ `SetFreq`, `SetDecay`, `SetTone`, `SetAccent`
* Snare page ↔ `SetFreq`, `SetDecay`, `SetFmAmount`, `SetSnappy`
* Hat page ↔ `SetFreq`, `SetDecay`, `SetTone`, `SetNoisiness`, `SetAccent`

### Appendix C — Risks

* too many layers
* overbuilding local sequencing already solved by OXI
* inconsistent LED semantics
* encoder page incoherence
* audio callback contaminated by UI logic

---

## Source notes for the presenter

For the OXI-related claims used in this deck:

* OXI One is documented as a battery-powered performative sequencer/composition tool with **4 fully independent sequencers**; the interface is described as using a **linear relationship between layouts** to avoid nested menus; and **Multitrack mode** provides **8 mono tracks**.  

For the DaisySP-related claims:

* DaisySP documents drum synthesis, filters, effects, control generators, physical modeling, granular player, and utility DSP blocks. ([electro-smith.github.io][1])
* Specific drum APIs expose relevant parameters:

  * `AnalogBassDrum`: freq, tone, decay, accent, attack FM, self FM. ([electro-smith.github.io][2])
  * `SyntheticSnareDrum`: freq, decay, accent, FM amount, snappy. ([electro-smith.github.io][3])
  * `HiHat`: freq, tone, decay, accent, noisiness. ([electro-smith.github.io][4])

I can convert this into a tighter **10-slide version** or a **speaker-notes version** with estimated time per slide.

[1]: https://electro-smith.github.io/DaisySP/index.html "DaisySP: DaisySP"
[2]: https://electro-smith.github.io/DaisySP/classdaisysp_1_1_analog_bass_drum.html "DaisySP: daisysp::AnalogBassDrum Class Reference"
[3]: https://electro-smith.github.io/DaisySP/classdaisysp_1_1_synthetic_snare_drum.html "DaisySP: daisysp::SyntheticSnareDrum Class Reference"
[4]: https://electro-smith.github.io/DaisySP/classdaisysp_1_1_hi_hat.html "DaisySP: daisysp::HiHat< MetallicNoiseSource, VCA, resonance > Class Template Reference"
