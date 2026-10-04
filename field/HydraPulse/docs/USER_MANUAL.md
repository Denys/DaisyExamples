# HydraPulse Field
## Player's manual | 0.2.0 candidate | 7 September 2026

### Read this first

HydraPulse is a standalone four-voice drum instrument for Daisy Field. Its normal job is
simple: choose a drum, enter a one-bar rhythm on sixteen keys, shape its sound, and play.
The first screen is the playing surface. There is no project browser, machine browser,
patch hierarchy or required desktop editor.

This manual describes the implemented candidate, not a measured production instrument.
Two separate programs are supplied as source: **Field Truth** for bench checks and
**Beat Core** for music. Run and record Field Truth before promoting Beat to normal use.
No physical control mapping, electrical range, audio measurement or target callback
deadline has been verified in this delivery. Read the build report for the particular
binary you are about to use.

Do not connect CV/Gate equipment during the first boot. Lower the external mixer,
speaker or headphone amplifier before connecting audio or replacing firmware. A software
limiter is not hearing protection and cannot prevent power-on codec transients.
Use the Field board's own documented supply and connection requirements. The supply,
signal ranges and tolerance statements in the Hydrasynth and DFAM manuals do **not**
describe Daisy Field.

Firmware replacement can overwrite another application or a resident bootloader.
Follow the boot-layout checks in the Codex guide; do not select a flash address by guesswork.

## 1. First sound and first beat

### Starting condition

Beat starts stopped on **Blank Canvas**, bank A, with Hammer selected. All pattern banks
are empty. No notes are generated until you audition a voice, receive a supported MIDI
trigger or start a pattern containing steps. Parameters have stored initial values.
Master starts at 25% of its software range, regardless of the physical knob position.

The eight knobs use **pickup**. A knob must pass through its stored value before it
takes control. This prevents selecting a new drum or loading a tutorial from yanking
that sound to the old knob position. Move slowly through the stored percentage shown
on the screen, then continue editing. A pickup can occur within about 1.5% of the target;
it is not an encoder.

### Make a kick pattern

1. Hold SW2, tap key 1, then release SW2. This selects Hammer; while stopped, selection
   also auditions it.
2. Tap keys **1, 5, 9 and 13**. Release each key promptly. Their LEDs show four active steps.
3. Move knob 1 through the displayed Tune value, then lower it for a deeper body.
4. Move knob 2 through Decay, then shorten the tail until the hits leave useful space.
5. Tap and release SW1 to start. Step 1 occurs at the next audio sample after the
   transport action is accepted. The step LEDs show the moving play position.
6. Adjust the external listening level conservatively. Use knob 8 for the instrument's
   software Master once its pickup target has been crossed.

### Add snare and hat

Hold SW2, tap key 2, release SW2. You are editing Crack without changing the kick pattern.
Tap keys 5 and 13 for a backbeat. Then select Steel with SW2 + key 3 and enter a few
off-beat or alternating steps. Press SW1 to stop when you need time to listen to a single
voice; selecting a voice while stopped auditions it without requiring a sequencer step.

A normal step action happens **on key release**, not on initial press. Holding the key
for 400 ms has a different purpose: accent. This distinction lets the same sixteen keys
edit a complete one-bar rhythm without sacrificing step visibility.

## 2. The panel

Key numbers in this manual are **logical steps 1–16**. The provisional adapter uses
libDaisy keyboard indices 0–15 in order, with LEDs A1–A8 then B1–B8. Physical row/position
assignment must be checked on the actual Field. Field Truth logs raw indices so that
this mapping can be corrected in one place: `firmware/FieldAdapter.h`.

### Eight knobs, one stable meaning

| Control | Meaning | Behavior |
|---|---|---|
| Knob 1 | Tune | Selected drum's base frequency; logarithmic mapping |
| Knob 2 | Decay | Selected drum's amplitude-decay time; logarithmic mapping |
| Knob 3 | Character | Selected drum's tonal/noise/modulation balance |
| Knob 4 | Level | Selected drum's linear level |
| Knob 5 | Tempo | Internal tempo, 40.0–240.0 BPM in 0.1-BPM increments |
| Knob 6 | Swing | Long/short sixteenth pairs, 50–70% long-step share |
| Knob 7 | Drive | Global pre-saturation gain |
| Knob 8 | Master | Global software output level |

The display shows voice parameters as normalized percentages. It does not claim that
50% Tune means 50 Hz, or that 50% Decay means half the maximum time. The musical ranges
are in the voice chapter. Parameter targets normally ramp over five milliseconds;
musically deliberate oscillator/noise content is not removed by that smoothing.

### Two switches

**SW1:** tap and release for Play/Stop. Holding SW1 alone does not create another menu.

**SW2:** hold for Shift; release to return to Home. Shift is momentary, not a latched mode.
The palette labels the available actions. Knobs retain their usual meanings.

**Both switches:** hold together for 500 ms for Panic. The transport stops, Fill is
released and voices enter a short release. Keep both released before using Play again.
The chord cannot accidentally start transport when its switches are released in either order.

### Shift palette

| While holding SW2 | Action |
|---|---|
| Key 1 / 2 / 3 / 4 | Select Hammer / Crack / Steel / Arc; audition only while stopped |
| Key 5 / 6 | Edit A / B and request it for playback |
| Key 7 | Edit the Fill bank without forcing it into playback |
| Hold key 8 | Use Fill for steps that occur while held |
| Key 9 / 10 / 11 / 12 | Toggle the four voice mutes |
| Hold key 13 for one second | Clear the selected track in the edited bank, stopped only |
| Key 14 / 15 | Preview previous / next tutorial preset, stopped only |
| Key 16 | Confirm the displayed tutorial load, stopped only |

Release a Shift key and Shift in either order: it will not leak an ordinary step edit.
For a Fill performance gesture, keep both SW2 and key 8 held. Releasing either ends
the request. Clear and preset loading do not operate while transport is running.

## 3. Reading Home

The top line shows **RUN/STOP**, the edited bank and the last bank actually played.
`A>B`, for example, means you are editing A while B supplied the last emitted step;
the arrow is an edit/play relationship, not a copy command. The implementation's
snapshot also exposes the queued A/B bank for development tools.

The next line shows selected voice, current step number and a Fill-request indicator.
Tune/Decay and Character/Level follow. A global line shows swing share, drive and master.
The tutorial name or pending-load prompt is below them.

`PICKUP` is a hexadecimal diagnostic mask: bit 0 is knob 1, bit 7 is knob 8.
A set bit means that knob has not picked up its target. This compact display is
functional but not a polished directional pickup guide. Move the relevant knob
through its stored value; do not interpret the mask as an error counter.

`MUTE` uses bits 0–3 for Hammer, Crack, Steel and Arc. `CPUmax` is the largest callback
load measured by the firmware since initialization. `E` counts detected DSP faults.
A zero error counter is not a substitute for a scope trace, nor is the displayed
callback load a whole-system worst-case execution proof.

Ordinary active steps are dimmer than accents. The current playing step is bright.
In Shift, the first four LEDs indicate voice selection. The screen palette takes
priority over the Home information; a preset preview remains visible in its bottom lines.

## 4. Rhythm, accents and variations

### Editing a step

Tap to toggle a step in the **edited bank and selected voice only**. Deleting a step
also deletes its accent. Hold for 400 ms to toggle accent once; this also creates the
step if it was empty. Releasing after a hold does not undo the result.

Unaccented sequencer hits use velocity 0.72. Accents use 1.0. This is a two-level
dynamic system, not a full per-step velocity editor. MIDI hits can use all 127 nonzero
velocity values. Velocity zero does not trigger a voice.

### A and B

A and B are separate sixteen-step banks containing all four tracks. They share the
four current voice settings, tempo, swing, drive and mutes. Changing Tune on A also
changes that voice when B plays. They are pattern variations, not independent kits.

Selecting A or B immediately changes the editing bank. While running, the playback
change occurs at the next bar's step 1. While stopped, bank selection takes effect
immediately. Stop and Start restart the sequence at step 1 rather than continuing
from the old position.

There is no automatic A-to-B copy command in this candidate. Build a different bank
manually or start from a tutorial containing both variations.

### Fill

Fill is a third bank. Key 7 in Shift selects it for editing, without switching the
underlying A/B arrangement. Hold Shift + key 8 to borrow the Fill bank for each
new step. The underlying sixteen-step phase continues. On release, the next step
uses the active A/B bank at that same phase.

This is **not** an automatic one-bar fill, a retrigger roll, or a parameter-lock system.
Pressing Fill between steps does not immediately strike a drum. A short hold that
contains no step boundary may produce no Fill hit. An A/B request still commits at
the next bar boundary even while Fill is held.

### Swing and tempo

Swing delays every second sixteenth within a two-step pair while preserving the pair's
nominal length. At 50% the steps are straight. At 70%, the first interval occupies
70% of the pair and the second 30%. Swing changes commit at a pair boundary;
tempo changes preserve the accumulated musical phase instead of restarting the clock.

Sequencer timing is sample-domain timing relative to the device audio clock.
It is not evidence that the physical oscillator has zero ppm error or that two
unconnected instruments stay synchronized forever.

## 5. The voices

All four voices are synthesized. There are no samples to manage or load.

| Voice | Tune range | Amplitude T60 range | Character |
|---|---|---|---|
| Hammer | 32–110 Hz | 60 ms–1.4 s | Strength of pitch punch and second harmonic |
| Crack | 120–330 Hz | 40–800 ms | Body/noise balance and noise brightness |
| Steel | 700–2300 Hz | 20–300 ms | Inharmonic metal/noise balance and brightness |
| Arc | 55–900 Hz | 50 ms–2.4 s | Modulator ratio and phase-modulation depth |

T60 means the exponential decay takes that duration to reach approximately 0.001 of
its starting amplitude, or −60 dB in amplitude ratio. It does not mean the entire
sound is cut off at that instant. The envelope reaches a very small tail threshold
before it is set to exact zero.

### Hammer: body and impact

Hammer combines a sine body with a small second harmonic. A short pitch envelope
gives the beginning of the hit more impact. Tune moves the body pitch; Decay controls
the amplitude tail; Character increases the transient pitch excursion and harmonic bite.

Start with low Drive. Raise Character while holding Tune steady, then shorten Decay.
The impact should become more pronounced without turning the basic kick into a
permanent oscillator. High Character is an exaggerated synthesized kick, not a
model of a particular commercial drum circuit.

### Crack: shell and snare noise

Two inharmonic sine components form the body. High-pass-shaped deterministic noise
provides the snare-like component. Low Character favors the pitched body; higher
Character adds a brighter, noisier crack. There is no high-Q resonator that can enter
self-oscillation.

Audition Crack while stopped. Change Tune first to hear the body. Then change Character
to find the point where the noise supplies enough edge. Avoid compensating a quiet
Level by immediately maximizing global Drive; learn the voice's own balance first.

### Steel: metallic rhythm

Six bounded sine oscillators are combined in three pairs to create an inharmonic
metallic spectrum, mixed with shaped noise. Short Decay gives a closed-hat role;
longer Decay makes the metallic body easier to hear. This is one voice, with no
separate open-hat lane or choke group.

Try alternating steps, then accent every fourth hit. Increase swing before adding
more notes. The tutorial exercises intentionally show that timing and dynamics can
change the feel without requiring a denser pattern.

### Arc: FM-style percussion

Arc uses feed-forward sine phase modulation, a close relative of sinusoidal FM
synthesis, with a moving modulation envelope. Character changes the modulator ratio
and depth. Low values can be rounded or bell-like; larger values are more metallic.

The carrier phase does not reset on a retrigger, and the modulation-envelope level
ramps from its current state rather than jumping. That reduces one specific
retrigger-click mechanism. It does not mean every extreme FM setting is spectrally
clean. High-pitch/high-Character settings can alias; the candidate does not claim
oversampled or alias-free synthesis.

## 6. Drive, level and stopping

Voice Level, mute gain, Drive and Master have different jobs. Level balances a drum;
mute temporarily removes it; Drive increases gain before the bounded saturator;
Master controls the final mix.

The path includes a 10-Hz DC blocker and a bounded rational saturator. With valid
internal state and Master no higher than 1, the final normalized samples are bounded
below approximately ±0.736. That arithmetic is about digital samples, not volts at
a jack, acoustic loudness or calibrated headroom.

Mute, Stop and Panic use short gain/envelope transitions. The oscillator and envelope
state is not abruptly zeroed while audible. A residual DC-filter tail may remain
briefly after the voice release; tests check return to silence. Normal percussion,
especially noise and high-frequency metallic voices, naturally has large adjacent
sample differences. The transition test compares matched signals with and without
a control event rather than treating every steep drum transient as a defect.

There is no feedback delay, reverb freeze or resonant filter in this candidate.
Unintended endless oscillation is not a musical feature. If a sound continues when
it should stop, lower the external listening level, use Panic and save the telemetry
and reproduction steps for investigation.

## 7. Tutorial presets

Tutorials are compiled original data, not extracted factory patterns. They load a
complete set of three banks, four voice settings, tempo, swing and drive. Loading
resets mutes and stops transport. **Master is retained** so a tutorial cannot silently
replace your chosen listening level.

To load: stop with SW1, hold SW2, tap key 14 or 15 to choose a tutorial, read its name,
then tap key 16. Release SW2. The audio path fades down for about 5 ms, applies the
new data at zero gain and ramps up again. Controls re-arm pickup. Selection is a
preview until confirmed; the preview expires after five seconds.

Loading a tutorial or clearing a track discards the affected unsaved edits.
There is **no user-save function, undo or persistence across power cycles** yet.
Write important edits in the patch notes sheet before loading something else.

| Tutorial | Main lesson |
|---|---|
| Blank Canvas | Build the first sixteen-step pattern without hidden content |
| First Pulse | Kick Tune, Decay and Character |
| Backbeat | Balancing three complementary drum roles |
| Broken Grid | Syncopation and A/B variation |
| Alloy Swing | Metallic hats, accents and swing |
| Arc Study | Tuned modulation-based percussion |
| Night Shift | Four-voice balance and controlled drive |
| Fracture Lab | Fill, mutes and more aggressive performance |

[Preset lessons](PRESETS.md) contain the exact values, banks and step grids.
The companion host WAVs demonstrate the programmed scenario, not recorded Field audio.

## 8. MIDI and external connections

Beat receives serial MIDI through the Field's initialized `MidiUartHandler`.
It does not configure USB MIDI. Connect a known-good source through the board's
documented MIDI input connection and verify it in Field Truth first.

| Message | Response |
|---|---|
| Channel 10 Note On 36 | Hammer |
| Channel 10 Note On 38 | Crack |
| Channel 10 Note On 42 | Steel |
| Channel 10 Note On 39 | Arc |
| Nonzero note velocity | Trigger level |
| Note On velocity 0 / Note Off | No new trigger; voices are one-shots |
| MIDI Start | Start from step 1 when stopped; ignored when already running |
| MIDI Stop / System Reset | Priority stop/panic and discard queued old commands |
| Channel 10 CC 120 / 123 | Priority panic |
| MIDI Clock / Continue | Ignored; not an external-clock follower |

Do not expect a DAW or Hydrasynth clock to synchronize Beat just because MIDI Start
works. There is no MIDI clock PLL, song-position handling or clock arbitration in this
candidate. The fixed queue rejects excess ordinary commands and reports drops.
Priority stop is separate from that queue.

Beat ignores both audio inputs and leaves CV/Gate outputs commanded off. Its CV
inputs and Gate In do not modulate or clock the instrument. Field Truth exercises
these interfaces, but future musical integration requires measured mappings and
an explicit control/clock specification.

## 9. Field Truth bench program

Field Truth is not a sound preset inside Beat. It is a separate diagnostic firmware.
Its stereo callback passes input to output at unity digital gain. Start with a
low-level calibrated source and disconnected CV/Gate cables. It is intentionally
not limited, so that gain/clipping tests measure the actual path.

Raw key press/release records use indices 0–15. Hold and release every physical key,
record its label and LED correspondence, and compare the complete set. Knob and CV
telemetry are normalized readings multiplied by 10,000. They are **not millivolts**.

The output test is off until SW1 has first been observed released. After that,
holding SW1 alone enables the test. Knobs 1 and 2 independently select DAC codes
0, 1024, 2048, 3072 or 4095 in five zones; Gate Out is commanded active.
Release SW1 to command zero/off. SW2 inhibits this output test.

This is a software interlock, not a fail-safe circuit. A crashed program can leave
an output asserted. Use an isolated bench fixture appropriate to the board and
do not connect this output test to equipment with unverified input limits.
Debounced switch handling also means release is not electrically instantaneous.

Holding SW2 alone runs the BSP LED/OLED light test. MIDI reception is counted and
the latest message metadata is reported. Gate In is sampled at the callback rate;
narrow pulses can be missed. Timing is exposed through the Seed testpoint and
callback counters. Check probe access and electrical loading before attaching
a scope or logic analyzer to an assembled Field.

The periodic `boot` record replays build/rate/board/memory/loader information.
`reset=unknown` is deliberate. Receiving this line after USB enumeration does not
prove a true cold boot, the reset source or absence of an earlier fault.

## 10. Troubleshooting

**No sound:** confirm which image is running. Truth needs an audio input. Beat needs
an audition, supported MIDI hit or active steps plus Play. Check Master pickup,
voice Level, mutes, edited versus playing bank, mixer connection and actual key mapping.

**Knob appears stuck:** it probably has not picked up the stored target. Read the
parameter percentage and sweep slowly through it. Preset loading and voice selection
intentionally re-arm pickup.

**Step changed differently from expected:** short release toggles the step; a
400-ms hold changes accent. A Shift gesture is consumed even if Shift is released first.

**Changing bank did not sound immediate:** A/B playback changes at the next bar.
Editing changes immediately. Fill is borrowed per step, not a restart.

**Preset controls do nothing:** Stop first. Key 14/15 preview; key 16 confirms.
The program deliberately refuses a running load.

**MIDI runs but drifts against another box:** external clock following is absent.
Start/Stop handling does not establish tempo synchronization.

**A click, dropout or runaway sound:** lower external volume, Panic, capture the build
manifest, raw log, control sequence and audio recording. Inspect `over`, `late`, drops,
faults and callback waveform. Do not diagnose solely from a successful host test.

## 11. Patch notes

Before discarding edits, record: tutorial, tempo, swing, drive, master, four normalized
voice parameter sets, A/B/Fill note and accent grids, and mute state. Also record the
binary hash and verified physical key mapping. A printed grid is a temporary backup
method, not a claim that the firmware implements storage.

## 12. Instructional sources

The supplied Hydrasynth Explorer manual's Home/shortcut/macro explanations
(pp. 14–18, 24, 66–67) informed direct access and clear distinctions between selection
and action. Its cover says Version 2.0 despite the attachment's 2.2.0 filename.
The supplied DFAM manual's signal-flow overview and progressive listening tour
(pp. 7–12), envelope explanation (pp. 20–21) and trigger/advance distinction
(pp. 22–23) informed the teaching order.

HydraPulse's controls, algorithms, presets, words and diagrams are its own candidate
design. It is not a Hydrasynth or DFAM emulation, and neither source manual is bundled.
