# POD MultiDelay v0.1 — User manual

**Five-mode mono delay demonstrator · Manual revision 3 · 9 September 2026**

This guide describes the current v0.1 normal firmware: DIGI, TAPE, MOD, REV and
FREEZE, operated by two pots, two buttons and the push encoder. It covers one
selected effect at a time; only DIGI offers two delay lines in Series or Parallel.
The firmware processes mono audio at 48 kHz, in blocks of 48 samples.

The 9 September POD run passed the digital functional screen, verified the normal
firmware write/readback and observed its audio callback. Physical listening and
control/LED checks remain open. See [Hardware test and recovery](HARDWARE_TEST.md)
and the validation summary in section 12. This manual describes the matching image;
it cannot establish what is running after any later reprogramming.

## 1. First sound

1. Connect your audio source to **LINE IN**. Listen through **PHONES**, or connect
   **LINE OUT** to your audio interface, mixer or powered monitoring system.
   Begin with a low listening level. **MIDI IN** receives MIDI data, not audio;
   this demonstrator does not use it to control the delay.
2. After firmware startup, wait at least 100 ms before operating the controls.
3. Leave the pots still initially. The stored startup sound is **DIGI / SINGLE**:
   400 ms delay, 0.35 feedback, 35% mix, 6000 Hz feedback high-cut and zero drift.
4. Play a short note, then stop. The intended result is a dry note followed by repeats.
5. **LED1 green** identifies DIGI. **LED2 green**, when B2 is released, identifies
   page 1: K1 controls TIME and K2 controls FEEDBACK.
6. Turn K2 counterclockwise to reduce repeats. Click the encoder once for page 2;
   turn K1 to change MIX. The knob movement rule below applies each time you switch pages.

### Audio and MIDI connectors

| POD label | Use in this demonstrator |
|---|---|
| **LINE IN** | Audio source input |
| **LINE OUT** | Audio output to external line-level equipment |
| **PHONES** | Headphone output |
| **MIDI IN** | MIDI data input; unused by this firmware |

**L and R refer to channels within the audio connection, not separate POD sockets.**
The v0.1 firmware processes only the left channel of LINE IN; it does not sum the
left and right channels. Ensure your source/cable sends audio to that channel.
Material present only on the right channel will not be heard. The processed mono
signal is duplicated to the left and right output channels, so this version does
not produce a stereo or Ping-Pong delay.

Use the hardware labels **K1**, **K2**, **B1**, **B2**, **LED1**, **LED2** and the
push encoder. RESET and BOOT are programming controls, not musical functions in this guide.

## 2. The controls at a glance

| Gesture | Result |
|---|---|
| Turn encoder, B2 released | Select mode: DIGI → TAPE → MOD → REV → FREEZE; rotation wraps |
| Click encoder, B2 released | Select parameter page: 1 → 2 → 3 → 1 |
| Press B1, B2 released | Toggle effect bypass |
| Short B2 press and release | Tap tempo outside FREEZE; capture/release in FREEZE |
| Hold B2 and turn encoder in DIGI | Select SINGLE, SERIES or PARALLEL |
| Hold B2 and turn encoder in FREEZE | Step through Capture, Hold, Accumulate and Replace |
| Hold B2 and click encoder | Toggle trails in every mode |
| Hold B2 and press B1 in FREEZE | Clear the captured loop; bypass remains unchanged |

A short B2 press lasts **less than 500 ms**. The short action happens on release.
B2 acts as SHIFT as soon as it is held; there is no mandatory long-press delay
before turning/clicking the encoder. After a shifted gesture, releasing B2 does
not also send a tap or Freeze command. Holding B2 alone for at least 500 ms and
releasing it sends no short action; this is useful when inspecting the status LED.

Shift-turn has no assigned action in TAPE, MOD or REV. Shift-B1 has no assigned
action outside FREEZE. Those shifted gestures still consume the B2 release, so
they do not produce a tap when B2 is released. Ordinary mode selection always
returns to page 1. Bypass and trails remain in their current states when changing mode.

### The three parameter pages

| Page | LED2 with B2 released | K1 | K2 |
|---|---|---|---|
| 1 | Green | TIME | FEEDBACK |
| 2 | Red | MIX | COLOR |
| 3 | Blue | MOTION | D2 time ratio, DIGI only |

**Move a knob to take control.** After startup, any encoder turn/click, or an accepted
tap interval, both pots must be moved again before they can change a value. This
also applies to shifted encoder gestures, including toggling trails without changing
the page. Values stay stored until the corresponding pot moves by about 1.2% of its normalized
range. It then takes the pot's absolute position. This avoids immediate changes
from a stationary pot, but it is not soft pickup: you do not need to cross the
stored value, and the first movement can cause a noticeable parameter jump.
If a knob is already at an endpoint, move it away slightly and return to reach
that endpoint deliberately.

**B2 does not select an alternate pot bank.** While holding B2, K1 and K2 still
control the current parameter page. LED2 is showing status at that moment; release
B2 to see which page is selected. Holding B2 alone does not disarm an already active pot.

Each mode remembers its five parameter values during the current session.
Returning to a mode recalls those values. Power/restart loses edits and captured
loops. There is no preset-save command, numeric display or persistent preset bank.
D2 configuration and ratio persist while changing modes within the same session.
Bypass and trails are also shared across modes. They are not recalled independently
with each mode's five parameter values.

## 3. LED reference

| LED1 color | Mode |
|---|---|
| Green | DIGI |
| Amber | TAPE |
| Blue | MOD |
| Purple | REV |
| Cyan | FREEZE |

LED1 becomes dimmer when bypassed. Its color still identifies the selected mode.

When B2 is held, LED2 changes from the page display to a status display:

| Context | LED2 color | Meaning |
|---|---|---|
| DIGI | Green | SINGLE |
| DIGI | Amber | SERIES |
| DIGI | Blue | PARALLEL |
| FREEZE | Green | Idle/released |
| FREEZE | Amber | Capturing |
| FREEZE | Blue | Holding |
| FREEZE | Purple | Accumulating |
| FREEZE | Cyan | Replacing |

Capture and Replace automatically become Hold when capture finishes, so their
colors can be brief. A brief red indication takes priority during DSP reconfiguration;
red with B2 released also normally means page 2. Trails has **no dedicated on/off
indicator** in v0.1. In TAPE/MOD/REV, shifted LED2 is not a useful mode-status indicator;
use LED1 for mode and release B2 for the page display.

These are software color assignments; actual brightness and color appearance
remain dependent on the physical POD and its validation.

## 4. DIGI — clear repeats and dual delay

| Parameter | Range | Initial value |
|---|---|---|
| TIME | 20–2000 ms | 400 ms |
| FEEDBACK | 0–0.90 | 0.35 |
| MIX | 0–100% | 35% |
| COLOR | 500–12000 Hz feedback high-cut | 6000 Hz |
| MOTION | 0–5 ms slow delay modulation | 0 ms |

TIME has a logarithmic control law: halfway along the normalized control range
is about 200 ms, not 1000 ms. Increasing COLOR keeps subsequent repeats brighter.
MOTION introduces slow pitch drift; leave it at minimum for the neutral baseline.
FEEDBACK reaches its 0.90 cap before the last small portion of pot travel.

Hold B2 and turn the encoder to select:

- **SINGLE:** one mono delay. D2 ratio has no audible role.
- **SERIES:** D1 feeds D2. Each has its own feedback loop, controlled by the shared
  FEEDBACK setting. The processed output comes from D2.
- **PARALLEL:** both delays receive the input independently. Their processed outputs
  are averaged equally into one mono output.

Page 3, K2 selects seven D2/D1 time ratios, in clockwise order:
**1/2, 2/3, 3/4, 1, 4/3, 3/2, 2**. D2 time is limited to 20–2000 ms.
Startup ratio is 3/4. Both the configuration and ratio are retained during the session.

For D1 = 400 ms and ratio = 3/4, D2 = 300 ms. With feedback and motion at zero,
Single's first processed echo is around 400 ms; Series' is around 700 ms;
Parallel produces processed echoes around 300 and 400 ms. These are timing examples,
not a measured physical latency specification. Dry sound depends on MIX.
Changing configuration discards the previous tails after a brief fade to dry.
Changing TIME or the D2 ratio slews the read position and can bend pitch.

## 5. TAPE — darkening and transport movement

| Parameter | Range | Initial value |
|---|---|---|
| TIME | 40–1200 ms | 380 ms |
| FEEDBACK | 0–0.95 | 0.45 |
| MIX | 0–100% | 40% |
| COLOR / AGE | 0–100% | 35% |
| MOTION / WARBLE | 0–100% | 25% |

Start with the initial sound. Increase AGE to explore the model's aged conditioning;
change WARBLE to vary transport instability. Change TIME slowly to hear the intentional
pitch movement. Higher feedback makes conditioning on successive repeats more apparent.
This is the existing reference tape algorithm, not a validated emulation of a named machine.

## 6. MOD — moving short delay

| Parameter | Range | Initial value |
|---|---|---|
| TIME / BASE TIME | 0.5–50 ms | 8 ms |
| FEEDBACK / RESONANCE | −0.95 to +0.95 | 0 |
| MIX | 0–100% | 50% |
| COLOR / DEPTH | 0–10 ms | 2 ms |
| MOTION / RATE | 0.02–8 Hz | 0.6 Hz |

FEEDBACK is bipolar: the middle is approximately zero, clockwise is positive and
counterclockwise is negative. Start near the middle, then explore resonance gradually.
COLOR controls sweep depth; MOTION controls speed. Increasing MIX emphasizes the moving
delayed signal. At full wet it can expose a vibrato-like result; this is a listening
starting point, not a hardware-validated preset.
The POD adapter uses external dry/wet blending, so its blend is not identical to
all settings of the reference engine's internal comb blend. MOD tap tempo clips
to 50 ms and is therefore unsuitable for setting an ordinary musical beat interval.

## 7. REV — reversed slices

| Parameter | Range | Initial value |
|---|---|---|
| TIME / SLICE | 50–1200 ms | 300 ms |
| FEEDBACK | 0–0.85 | 0.25 |
| MIX | 0–100% | 50% |
| COLOR / REVERSE | 0–100% | 100% |
| MOTION / GRAIN | 0–100% | 50% |

Play a phrase and leave a gap. SLICE sets the scale of the reversed segments;
it does not reverse an entire recorded performance. COLOR moves from the forward
branch toward the reverse branch. MOTION changes grain-window taper, not an LFO rate.
Expect the processed response to require recorded history; an immediate dry response
can remain audible through MIX. Grain length is not a complete measured analog-latency value.

## 8. FREEZE — capture and replay

| Parameter | Range | Initial value |
|---|---|---|
| TIME / LOOP | 50–2000 ms | 600 ms |
| FEEDBACK / DECAY | 0.50–0.999 | 0.97 |
| MIX / LEVEL | 0–100% | 60% |
| COLOR / DAMPING | 0–100% | 30% |
| MOTION / EVOLVE | 0–100% | 10% |

### Capture your first loop

1. Select FREEZE with B2 released. LED1 becomes cyan. The loop starts idle.
2. Set LOOP before capturing if a different duration is needed.
3. Briefly press and release B2, then play the material to capture. Recording starts
   after the release and brief transition; it does not retrieve audio from before the gesture.
4. Keep feeding sound for the selected loop duration. Capture automatically enters Hold.
   At 100% MIX the capture interval can be silent because replay has not started yet.
5. Stop playing and listen to the loop. Hold rejects new input to the loop, while
   a dry component may still pass according to MIX.
6. A further short B2 press/release returns to Idle. This stops replay; it is not bypass.

### Hold, add, replace or clear

After capture, hold B2 and turn the encoder, checking LED2 between actions.
Clockwise progression is Capture → Hold → Accumulate → Replace → Capture; reverse
rotation steps backward. Because Capture/Replace advance automatically to Hold, the
current observed state determines the next step. **Selecting Hold while idle does
not create a loop**; use short B2 to capture first.

For a **single encoder detent while B2 is held**, the current state selects the
command below. Turn one detent at a time and allow the status to settle; Capture
and Replace can finish automatically between gestures.

| Current FREEZE state | Clockwise detent | Counterclockwise detent |
|---|---|---|
| Idle | Hold | Replace |
| Capture | Hold | Replace |
| Hold | Accumulate | Capture |
| Accumulate | Replace | Hold |
| Replace | Capture | Accumulate |

Idle is not one of the four shifted selections. Use short B2 from an active state
to return to Idle. The simplest sequence is **short B2 from Idle → wait for Hold →
SHIFT + one clockwise detent for Accumulate → SHIFT + one counterclockwise detent
to return to Hold**. Allow a capture to complete before changing its state.

- **Hold:** replay the captured loop without adding new input. Filtering, decay and
  evolution still operate; Hold is not an indefinitely lossless freeze.
- **Accumulate:** add new input to the running loop. Repeated input can build level.
- **Replace:** discard the old capture and record a new loop; it then enters Hold.
- **Clear:** hold B2 and press B1. The capture is erased and the state becomes Idle.
  Releasing B2 after this gesture does not start a new capture.
- **Release to Idle:** short B2 stops playback without invoking Clear. Old material
  may remain available if Hold is selected again; Clear is the explicit erase action.

Increasing DECAY retains more loop energy, but its maximum is below unity. DAMPING
and EVOLVE continue to alter the sound. Shortening LOOP after capture can truncate
playback; lengthening it does not recover the discarded portion. Recapture to obtain
a longer loop. Leaving FREEZE clears its audio state on the mode change.

## 9. Tap, bypass and trails

**Tap:** outside FREEZE, use two short B2 presses with releases 100–2000 ms apart.
The measured interval is between releases and is limited to the mode's TIME range.
Examples: a 500 ms tap sets DIGI/TAPE/REV to 500 ms, but MOD to 50 ms. Tap changes
TIME, not feedback, D2 ratio or an independent rhythm subdivision. Changing mode
starts a new tap measurement pair. A shifted gesture is not a tap.

An interval outside 100–2000 ms leaves TIME unchanged and makes the latest release
the starting point for the next interval. The first valid tap pair is enough;
there is no multi-tap averaging, MIDI clock, tap-rate LED or tap subdivision control.

**Bypass:** B1 toggles the effect. New input to the delayed path fades out; LED1 dims.
With trails on, existing delayed material remains audible over the dry signal.
With trails off, the wet contribution fades out. Bypass does not clear delay memory,
and re-enabling the effect can reveal remaining material. Mode/config changes clear tails.

Trails starts **on** after startup. Hold B2 and click the encoder to toggle it.
There is no persistent trails LED. If you need a known dry signal without relying
on the remembered toggle state, select page 2 and deliberately move K1 to minimum
MIX. This is a software dry path, not a relay/true-bypass claim.
A frozen loop can remain audible in bypass with trails on. Idle FREEZE with MIX
above zero can attenuate the dry signal because the wet branch is silent; use
minimum MIX or dry bypass when comparing levels.

**MIX at zero hides the effect; it does not stop recording.** With the effect
enabled, new input still enters delay history or a Capture/Accumulate operation.
Increasing MIX later can reveal that material. Bypass instead fades out new input
to the effect, while stored material continues to evolve. Neither action erases a
FREEZE loop: use SHIFT + B1 to clear it. Clearing returns FREEZE to Idle and does
not change MIX or bypass, so it need not restore full dry level by itself.

With trails on during bypass, the full dry signal and the remaining wet tail are
added. The transition can therefore change the combined level; use minimum MIX
for a dry-only comparison. The software output limit is not a guarantee of clean
or level-matched bypass.

## 10. Short listening exercises

These are proposed user checks, not a record of tests already performed on hardware.

| Exercise | Action | What to listen or look for |
|---|---|---|
| Basic delay | Start in DIGI with the initial sound | Repeats after a short input note |
| Page ownership | Click through all three pages and move K1 | TIME, MIX, then MOTION change; LED2 changes page color |
| Dual comparison | DIGI, feedback/motion minimum; compare configs at one tapped time | Single echo, cascaded timing, then two parallel arrivals |
| Tape movement | TAPE; vary WARBLE and slowly change TIME | Movement of repeat pitch/texture |
| Modulation | MOD; resonance near zero; vary depth and rate separately | Depth and speed have different effects |
| Reverse | REV; play a phrase followed by silence | Reversed fragments in the processed output |
| Freeze lifecycle | Capture, wait, Hold, Accumulate, Replace, Clear | A replaying loop; added/replaced content; no captured playback after Clear |
| Bypass/trails | Let a tail ring, then bypass; compare both trails states | Tail preserved or wet contribution fading out |

Record unexpected results with mode, page, gesture, input connection and approximate
knob positions. Listening alone does not quantify timing margin, distortion or noise.

## 11. Troubleshooting

| Symptom | Check |
|---|---|
| No sound | Connect the source to LINE IN and listen through PHONES or LINE OUT. Confirm matching firmware and check that the source/cable feeds the left channel of LINE IN. MIDI IN does not carry audio. In FREEZE, capture first or lower MIX. |
| Sound changes when a pot first moves | Expected until-moved behavior: the pot takes its absolute position. Approach changes gradually. |
| Knob appears inactive | Release B2 to see the LED2 page. Encoder gestures and accepted taps rearm both pots; move by more than the entry threshold. If at an endpoint, move away and back. Page 3 K2 is DIGI-only. |
| Short B2 seems to do nothing | Keep it under 500 ms and release; shifted gestures consume release. A tap needs two valid release intervals. |
| Selecting Hold produces silence | No completed capture may exist. Use short B2 from Idle to record new material. |
| Delay bends pitch when changing TIME | Expected read-time slew. Configuration changes instead use a brief dry transition and discard tails. |
| Loop remains after bypass | Trails may be on. Use minimum MIX for a known dry path, or toggle trails off from a known state. |
| Old sound returns when MIX is raised | MIX at zero hid the effect while its memory kept running. Bypass stops new input; SHIFT + B1 clears FREEZE. |
| FREEZE is quiet after Clear | Clear leaves the mode idle and keeps the current MIX. Lower MIX for full dry level, or capture a new loop. |
| Repeats/distortion build up | Reduce feedback/resonance, input level or Accumulate input. Output limiting is containment, not clean headroom. |
| Left and right listening channels sound alike | Intended mono duplication within the audio output; this version has no stereo/Ping-Pong output. |
| Settings disappeared | Edits and captures are session-only; there is no save function. |

## 12. Version and source scope

This manual describes the v0.1 files `Controls.h`, `Demo.h`, `DigiMono.h` and
`Pod_MultiDelay.cpp`, together with the referenced `PedalDelayEngine.cpp` descriptors
and Freeze implementation. It documents software behavior, not a later instrumented
build or the eventual custom pedal panel. MIDI, expression, USB-audio operation,
preset storage, screen readout and guitar-front-end electrical performance are not
implemented or established by this adapter.

### Recorded validation — 9 September 2026

| Area | Recorded result | Meaning for use |
|---|---|---|
| Host software tests and ARM build | PASS | Covered DSP/control behavior compiled and passed the recorded checks |
| POD digital diagnostic | PASS: 7 configurations, 12,288 impulse comparisons, 14,336 measured blocks; no observed output/guard errors or bracket overruns | Algorithms ran on the POD with internal stimulus; diagnostic outputs to the audio sockets were muted |
| Normal firmware load | Write/readback verified; audio callback entry observed | The normal demonstrator was left running at the end of that hardware run |
| Listening, analog audio and physical gestures/LEDs | NOT_RUN | Use section 10 for the remaining manual checks; software tests do not replace them |
| Cold boot, extended target stress and calibrated audio measurements | NOT_RUN | No production timing, noise, distortion or power-cycle acceptance is implied |

The diagnostic firmware intentionally outputs silence and runs its own sequence.
It is not the interactive image described here. The normal build is
`build/Pod_MultiDelay.bin`; source for the separate muted screen is under `diagnostic/`.

The [project README](README.md) contains build/dependency details.
[HARDWARE_TEST.md](HARDWARE_TEST.md) contains measurement boundaries, logs and the
verified backup/restore procedure. This documentation update does not reload firmware
or add new physical-test results.

Source fingerprints and the revision 3 documentation verification record are in
`work_products/delay/runs/2026-09-09_pod_user_manual_update.md` in the Custom Pedals hub.
The earlier record `2026-09-09_pod_user_manual.md` retains revisions 1 and 2 history.
The manual remains a separate companion file; the firmware ZIP and manifest are
not regenerated by this documentation-only update.

Revision 3 preserves the user-confirmed LINE IN, LINE OUT, PHONES and MIDI IN labels,
clarifies shifted pot behavior and FREEZE state selection, distinguishes MIX/bypass/
clear, and incorporates the dated hardware evidence without marking listening or
physical controls as tested.
