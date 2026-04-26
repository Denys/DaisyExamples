# SubharmoniqField Controls

## Menu Navigation

`SW1` and `SW2` act as a two-button encoder surrogate:

| Field control | Firmware behavior |
|---|---|
| SW1 tap | Menu/page backward |
| SW2 tap | Menu/page forward |
| SW1 hold + SW2 press | Encoder push / confirm / open-close menu |

Hold threshold is `500 ms`.

## Page Knobs

K1-K8 are live controls for the active page. On boot, the firmware records the
physical knob positions without applying them; each knob takes over only after
it moves far enough to avoid muting the playable startup patch by accident.

| Page | K1 | K2 | K3 | K4 | K5 | K6 | K7 | K8 |
|---|---|---|---|---|---|---|---|---|
| Home | Tempo | Cutoff | Resonance | VCA Decay | VCO 1 | VCO 2 | Drive | Output |
| Voice | VCO 1 | VCO 2 | VCO1 Sub1 divisor | VCO2 Sub1 divisor | VCO1 Sub2 divisor | VCO2 Sub2 divisor | - | - |
| Mix | VCO1 level | VCO1 Sub1 level | VCO1 Sub2 level | VCO2 level | VCO2 Sub1 level | VCO2 Sub2 level | Drive | Output |
| Seq | Seq1 Step1 | Seq1 Step2 | Seq1 Step3 | Seq1 Step4 | Seq2 Step1 | Seq2 Step2 | Seq2 Step3 | Seq2 Step4 |
| Rhythm | Rhythm 1 divisor | Rhythm 2 divisor | Rhythm 3 divisor | Rhythm 4 divisor | - | - | - | - |
| Filter | Cutoff | Resonance | VCF Env Amt | VCA Decay | VCF Attack | VCF Decay | Drive | Output |
| Patch | Root CV depth | Cutoff CV depth | Rhythm CV depth | Sub CV depth | - | - | - | - |

Filter page reminder for Round 2: implement selectable `SVF LPF`, `SVF BPF`,
and ladder-style `LPF`.

## A/B Keys

| Field key | Firmware behavior |
|---|---|
| A1-A4 | Select Sequencer 1 step 1-4 and jump to `Seq` page |
| B1-B4 | Select Sequencer 2 step 1-4 and jump to `Seq` page |
| A5-A8 | Cycle Rhythm 1-4 assignment: `off -> seq1 -> seq2 -> both` |
| B5 | Cycle quantize mode |
| B6 | Cycle sequencer octave range: `+/-1`, `+/-2`, `+/-5` |
| B7 | Toggle play/stop; play starts the internal tempo clock |
| B8 | Reset sequencers and rhythm phase |

The firmware compensates for the Field keyboard scan order so the table above
matches the physical panel labels: physical `B7` toggles play and physical
`B8` resets. Physical `A7` is intentionally a Rhythm 3 assignment key; it does
not start or stop playback.

## CV, Gate, MIDI, Outputs

| Field jack | Firmware behavior |
|---|---|
| CV1 | Root transpose / 1V-oct style offset |
| CV2 | Filter cutoff modulation |
| CV3 | Rhythm modulation input |
| CV4 | Subharmonic modulation input |
| Gate In | External clock pulse in Round 1 |
| Gate Out | Trigger pulse when envelopes fire |
| CV Out 1 | Sequencer 1 CV monitor |
| CV Out 2 | Sequencer 2 CV monitor |
| MIDI In | Note trigger/transpose and MIDI clock pulse handling |

CV1 is centered in firmware so an unpatched Field input leaves the root offset
at the core default instead of transposing the whole voice down.

## Manual Hardware Validation Checklist

Record date, build commit or diff state, and tester name before marking this
adapter hardware-validated.

- `make` succeeds. Passed on 2026-04-26; latest fresh build reported FLASH
  `126072 B` / `96.19%`.
- QAE validation succeeds. Passed on 2026-04-26 with `0 error(s), 0 warning(s)`.
- `make program` flashes successfully through ST-Link. Passed on 2026-04-26
  with STLINK `V3J7M2`, target voltage `3.274751`, and OpenOCD
  `** Verified OK **`.
- Audio outputs produce the SubharmoniqField voice after MIDI note, Gate In
  clock while playing, or the internal rhythm clock after B7 starts transport.
- K1-K8 change the active page controls without zipper noise or jumps severe
  enough to make the instrument unusable.
- SW1 tap moves backward, SW2 tap moves forward, and SW1-hold + SW2-press
  confirms/opens/closes the menu.
- A1-A4 and B1-B4 select sequencer steps.
- A5-A8 cycle rhythm assignments.
- B5, B6, B7, and B8 perform quantize, octave, transport, and reset actions.
- CV1-CV4 affect their documented patch-style destinations.
- CV Out 1 and CV Out 2 move with sequencer state.
- Gate Out pulses when envelopes are triggered.
- MIDI notes and MIDI clock are accepted.
- OLED shows active page/status and the Filter page Round 2 reminder.
- LEDs update without audible dropouts.
