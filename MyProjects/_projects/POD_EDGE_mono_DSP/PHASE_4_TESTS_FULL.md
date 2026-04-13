# EDGE Performance FX - Master Test Checklist (Phases 1-4)

Purpose: A comprehensive hardware and listening checklist combining all development phases up to Phase 4.

Current firmware scope:
- **Phase 1:** Hardware, OLED, navigation, presets, control system map.
- **Phase 2:** Active audio front end (Input Gain, DcBlock, Input Svf HP, Overdrive).
- **Phase 3:** Delay core (SDRAM buffers, tap tempo sync, FB high-pass & low-pass filtering).
- **Phase 4:** Polish processing (FDNReverb Diffusion, Wow/Flutter tape modulation, ToneStack Tilt EQ, Master Limiter).
- *(Freeze DSP logic is pending Phase 5).*

Recommended baseline test setup:
- Daisy Pod + custom external control board fully connected.
- Mono Behringer EDGE source into Pod LEFT input.
- Headphones or dual monitors on outputs.

---

## Part 1: Phase 1 & 2 - Hardware, UI, and Input DSP

### 1. Boot, OLED, and Routing
- [ ] Power-on shows boot screen correctly, followed by the normal Page 1 UI.
- [ ] OLED has no corruption, random pixels, or lockups.
- [ ] Mono signal on Pod left input is heard on both left and right outputs evenly.
- [ ] Silence at input produces silence at output.

### 2. Physical Navigation & Shift Behavior
- [ ] External continuous encoder scrolls fluidly across all 6 pages (wraps correctly).
- [ ] `BAK` tact switch always returns to Page 1 `PERFORM`.
- [ ] `Shift + Pod Enc Push` successfully cycles all pages.
- [ ] Short SW1 tap triggers freeze hook without showing `SHF`; holding SW1 > 200ms locks the `SHF` badge correctly.

### 3. Edit Mode & Preset Actions
- [ ] External encoder push toggles Edit mode on non-preset pages; parameters visibly edit.
- [ ] Presets (P5) scroll accurately and loading (SW2 / Pod push) successfully pulls expected variables. After loading, the UI jumps back to P1.

### 4. Input Base DSP (Phase 2)
- [ ] **Input Gain (P2 Shift-Pod):** Audibly raises/lowers level smoothly.
- [ ] **Input High-Pass (P2 K1):** Sweeping correctly strips bass from the dry signal.
- [ ] **Overdrive (P1 Shift-Pod):** Enhances saturation. Verify no zipper noise on sweeps.
- [ ] **Bypass (P2 SW2):** Correctly toggles processed signal against pure clean passthrough without popping/hanging.

---

## Part 2: Phase 3 - Delay Core & Feedback Validation

### 1. Page 1 (Perform Page) Core
- [ ] **Delay Time (K1):** Sweep full range. Delay should transition from tight metallic flanging to long spaced echoes smoothly.
- [ ] **Feedback (K2):** Sweep from 0% (one repeat) to 98% (infinite repeats).
- [ ] **Saturation Test:** Push K2 to maximum while playing aggressive bursts. The repeats should compress and clip gracefully due to the `tanh()` function, maintaining dense stability.
- [ ] **Wet Level (Pod Enc):** Mix transitions cleanly from 100% dry to heavily dominant wet mix at 1.00.

### 2. Page 2 (Filters Page) Core
- [ ] **Feedback LP (K2):** Sweeping down should make repeats progressively darker/muddier (BBD bucket-brigade style).
- [ ] **Feedback HP (K1 Shift):** Sweeping up should carve out sub-frequencies in the feedback loop, yielding thin/clicky echoes.

### 3. Sync & Clock Interactions
- [ ] **Tap Tempo (SW2):** Tap quarter notes. BPM should accurately follow your foot/hand.
- [ ] **Sync Toggle (Shift + SW2):** Turn Sync ON. Delay time immediately locks to tapped tempo grid.
- [ ] **Subdiv Change (K1 with Sync ON):** Adjust down from `1/4` to `1/8` or `1/16` and ensure immediate double/quadruple time jumping relative to the tapped clock.

---

## Part 3: Phase 4 - Polish & Post-Processing Validations

### 1. Output Tilt EQ (ToneStack)
- [ ] **Tilt (P2 Pod Encoder):** Sweep from -6 dB to +6 dB. The master output should globally shift from heavy, muffled sub-bass (negative values) to sparkling, thin treble (positive values) bridging exactly across the 0-point null.

### 2. FDNReverb Diffusion
- [ ] **Diffusion Damping (P2 Shift-K2):** Wait for a distinct delay repeater to ring out, then turn up the diffusion constraint. The discrete machine-gun echoes should smear out into a dense, reverberant room wash.

### 3. Wow/Flutter Modulation (Phasor)
- [ ] **Wow Dpth (P3 K1):** Turn this up to introduce tape wobble to the delay line. The echoes should start diving gracefully in pitch.
- [ ] **Wow Rate (P3 K2):** With Depth active, sweep the Wow Rate to hear the pitch wobble accelerate from a slow, seasick drift up to a fast, chaotic tape flutter.

### 4. Master Limiter Protection
- [ ] **Limiter Stress Test:** Crank `Drive` (Shift P1 Pod), input a massively loud signal, and crank `Feedback` (P1 K2) to 90+%. Output absolute maximal power. Verify the unit hits a dense brick-wall plateau preventing digital harsh clipping out of the physical monitor jacks.

---

## 🎧 Suggested Behringer EDGE Source Configurations

To properly isolate and evaluate all sections of this firmware, distinct styles of audio inputs from your Behringer EDGE are recommended depending on what you are testing. 

> **Knob Values:** 0 to 9, where 4.5~5 is Top-Center.  
> **Switch Values:** UP / MIDDLE / DOWN.

### A. Testing Phase 1/2 Input stages (Saturation & Filters)
* **Goal:** Needs deep sustained bass and heavy mid-range harmonics to grab onto the overdrive.
* **Oscillators:** Low pitched Triangle + slightly higher pitched Square wave.
* **Filter:** Wide open low-pass.
* **VCA:** Sustained drone or long decay. (Transient clicks hide saturation artifacts).

**Edge Panel Settings:**
* **VCO 1 Freq:** `2` | **Shape:** `UP` (Triangle) | **Level:** `8`
* **VCO 2 Freq:** `4` | **Shape:** `DOWN` (Square) | **Level:** `7`
* **1-2 FM AMT:** `0` | **Hard Sync:** `DOWN` (Off) | **EG Decay:** `5`
* **Noise/Ext Level:** `0`
* **VCF Cutoff:** `9` | **VCF Select:** `DOWN` (LP) | **VCF Res:** `1`
* **VCF Decay:** `5` | **VCF EG INT:** `5` (Center/Off)
* **VCA Decay:** `9` (Long) | **VCA Mode:** `DOWN` (Slow)
* **Sequencer:** All 8 Velocity knobs set to `9`.

### B. Testing Phase 3 Delay Echoes & Feedback
* **Goal:** Needs sharp isolated transients to clearly expose exact echo timing and subdiv changes. 
* **Sequencer:** Sparse pattern (e.g. 1 & 5 active).
* **Oscillators:** Simple Square or FM triangle.
* **Filter:** Snappy low-pass sweep.
* **VCA:** Very tight decay. Prevent dry signal masking the delayed repeats. 
* **Noise:** Mix some noise in so that sweeping the Feedback HP/LP filters triggers obvious "whooshing" on the echo tails.

**Edge Panel Settings:**
* **VCO 1 Freq:** `6` | **Shape:** `DOWN` (Square) | **Level:** `7`
* **VCO 2 Freq:** `3` | **Shape:** `UP` (Triangle) | **Level:** `0`
* **1-2 FM AMT:** `4` | **Hard Sync:** `DOWN` (Off) | **EG Decay:** `2`
* **Noise/Ext Level:** `5` | **Noise Color:** `DOWN` (Pink)
* **VCF Cutoff:** `3` | **VCF Select:** `DOWN` (LP) | **VCF Res:** `2`
* **VCF Decay:** `2` | **VCF EG INT:** `7` (Positive sweep)
* **VCA Decay:** `1` (Tight) | **VCA Mode:** `UP` (Fast)
* **Sequencer:** Velocity knobs `1` & `5` set to `9`; all others `0`. Pitch knobs `1` & `5` to different values for distinction.

### C. Testing Phase 4 Wow & Flutter
* **Goal:** Clear tonal sustaining pitches to emphasize the detuning.
* **Oscillators:** Pure Triangle or light FM. ZERO noise.
* **VCA:** Lengthen decay to let the note ring.
* **Filter:** Open.
* *Action:* Tonal lines instantly reveal the tape warble as pitch deviates from center.

**Edge Panel Settings:**
* **VCO 1 Freq:** `5` | **Shape:** `UP` (Triangle) | **Level:** `9`
* **VCO 2 Level:** `0`
* **1-2 FM AMT:** `1`
* **Noise/Ext Level:** `0`
* **VCF Cutoff:** `8` | **VCF Select:** `DOWN` (LP) | **VCF Res:** `0`
* **VCF Decay:** `5` | **VCF EG INT:** `5` (Center/Off)
* **VCA Decay:** `8` (Ringing) | **VCA Mode:** `DOWN` (Slow)
* **Sequencer:** Velocity knob `1` set to `9`; all others `0`. Tempo set relatively slow so the note sustains visibly.

### D. Testing Phase 4 FDNReverb Diffusion
* **Goal:** High frequency static bursts to simulate impulse responses.
* **Sequencer:** Fast staggered beat.
* **Oscillators:** Max noise!
* **VCA:** Very short decay (clicks/snaps).
* *Action:* Turn up the wet mix and diffusion to hear the harsh machine-gun noise clicks melt together into an ethereal hiss tail.

**Edge Panel Settings:**
* **VCO 1 & 2 Levels:** `0`
* **Noise/Ext Level:** `9` | **Noise Color:** `UP` (White)
* **VCF Cutoff:** `9` | **VCF Select:** `UP` (HP) | **VCF Res:** `1`
* **VCF Decay:** `1` | **VCF EG INT:** `5` (Center/Off)
* **VCA Decay:** `1` (Clicks) | **VCA Mode:** `UP` (Fast)
* **Sequencer:** Velocity knobs `1`, `4`, `7` set to `9`; all others `0`. Tempo set fast.

### E. Testing Phase 4 Output Tilt 
* **Goal:** Full-spectrum broadband signals covering both sub ranges and air ranges simultaneously.
* **Oscillators:** Massive heavy low sub triangle mixed with a high ringing noise hat.
* *Action:* Ensures neither frequency band feels artificially capped before you twist the Tilt parameter.

**Edge Panel Settings:**
* **VCO 1 Freq:** `1` (Low sub) | **Shape:** `UP` (Triangle) | **Level:** `9`
* **VCO 2 Level:** `0`
* **Noise/Ext Level:** `7` | **Noise Color:** `UP` (White)
* **VCF Cutoff:** `9` | **VCF Select:** `DOWN` (LP) | **VCF Res:** `0`
* **VCF Decay:** `5` | **VCF EG INT:** `5` (Center/Off)
* **VCA Decay:** `7` (Sustained) | **VCA Mode:** `DOWN` (Slow)
* **Sequencer:** All Velocity knobs to `7` or `8` to provide a consistent rolling tone.
