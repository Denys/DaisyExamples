# Subharmoniq Field Control Upgrade Design

Date: 2026-04-28
Scope: DaisyHost Subharmoniq host-side Field UI/control mapping

## Goal

Upgrade the DaisyHost Subharmoniq Field control model so it combines:

- hardware-faithful functional pages for VCO, VCF, and VCA/Mix controls
- a performance-first Seq/Rhythm hub as the default page
- globally stable A/B key behavior for sequencer, rhythm, quantize, octave, play, and reset

The design is based on the Moog Subharmonicon manual sections for oscillators/subharmonics, mixer, filter, envelopes, sequencers, transport, and polyrhythm controls.

## Manual Model

The instrument is centered on the relationship between:

- two VCOs, each with two subharmonics
- two 4-step sequencers, where Seq 1 controls VCO 1 and its subs, and Seq 2 controls VCO 2 and its subs
- four rhythm generators, each dividing tempo by an integer value from 1 to 16 and routing to Seq 1, Seq 2, or both
- shared quantize and sequencer octave settings
- filter and VCA envelope shaping

The sequencer/rhythm/subharmonic relationship is the main performance identity, so it should remain the most immediate Field surface.

## Core Interaction Rule

All 16 A/B short-press actions stay globally fixed across every page.

K1-K8 change by page.

Long-press or RSP/menu actions may expose secondary behavior, but short-press A/B behavior must not change per page.

## A/B Key Mapping

| Key | Short Press | LED Meaning |
|---|---|---|
| A1-A4 | Select Seq 1 Step 1-4 and jump/focus Seq page | active step on while running; lit while pressed |
| B1-B4 | Select Seq 2 Step 1-4 and jump/focus Seq page | active step on while running; lit while pressed |
| A5-A8 | Cycle Rhythm 1-4 target: Off -> Seq1 -> Seq2 -> Both | off = Off, blink = Seq1 or Seq2, on = Both |
| B5 | Cycle Quantize: Off -> 12ET -> 8ET -> 12JI -> 8JI | off = Off, blink = ET, on = JI |
| B6 | Cycle Seq Oct: +/-1 -> +/-2 -> +/-5 | off = 1, blink = 2, on = 5 |
| B7 | Play/Stop | on while playing |
| B8 | Reset | lit while pressed or reset pulse is active |

## Global Navigation

| Control | Action |
|---|---|
| SW1 | Previous page / Back |
| SW2 | Next page / Forward |
| A/B long press | Optional secondary action only |
| RSP Mod page | Shows selected target/source/base/result details |

## Page 1: Seq/Rhythm Hub

This is the default performance page.

| Knob | Control |
|---|---|
| K1 | Seq 1 Step 1 |
| K2 | Seq 1 Step 2 |
| K3 | Seq 1 Step 3 |
| K4 | Seq 1 Step 4 |
| K5 | Seq 2 Step 1 |
| K6 | Seq 2 Step 2 |
| K7 | Seq 2 Step 3 |
| K8 | Seq 2 Step 4 |

The display/RSP status area should show rhythm divisors, rhythm targets, quantize mode, sequencer octave, play state, current Seq 1 step, current Seq 2 step, and Seq Assign summary.

## Page 2: VCO/Subharmonics

| Knob | Control |
|---|---|
| K1 | VCO 1 pitch |
| K2 | VCO 2 pitch |
| K3 | VCO 1 Sub 1 divisor |
| K4 | VCO 1 Sub 2 divisor |
| K5 | VCO 2 Sub 1 divisor |
| K6 | VCO 2 Sub 2 divisor |
| K7 | Quantize view/edit mirror |
| K8 | Seq Oct view/edit mirror |

## Page 3: VCF

| Knob | Control |
|---|---|
| K1 | Cutoff |
| K2 | Resonance |
| K3 | VCF EG Amount |
| K4 | VCF Attack |
| K5 | VCF Decay |
| K6 | Tempo |
| K7 | Drive |
| K8 | Output |

## Page 4: VCA/Mix

| Knob | Control |
|---|---|
| K1 | VCO 1 Level |
| K2 | VCO 1 Sub 1 Level |
| K3 | VCO 1 Sub 2 Level |
| K4 | VCO 2 Level |
| K5 | VCO 2 Sub 1 Level |
| K6 | VCO 2 Sub 2 Level |
| K7 | VCA Decay, or VCA Attack if the core exposes it |
| K8 | Output |

If VCA Attack is not currently exposed by the portable core, do not invent unrelated DSP behavior in this UI pass. Prefer adding a host-side descriptor only if the core already has a meaningful parameter path.

## Seq Assign Handling

Seq Assign should be visible from the Seq page but should not steal normal A/B key meaning.

Required assignment states:

- Seq 1: OSC1, SUB1, SUB2
- Seq 2: OSC2, SUB1, SUB2

Preferred UI path:

- Select/focus a sequencer or step on the Seq page.
- Show assignment state in the RSP/display status area.
- Edit assignment toggles through RSP/menu rows.

Optional later extension:

- Long-press A1-A4 edits Seq 1 assignment.
- Long-press B1-B4 edits Seq 2 assignment.

## RSP Requirements

The RSP should make the performance state readable:

- current page name
- K1-K8 labels and values
- active sequencer steps
- rhythm divisors and rhythm targets
- quantize mode and seq octave
- play/reset state
- seq assignment summary
- modulation lane source, destination, amount, base, result, and clamp status

## LED Contract

LED values should keep the existing Field convention:

- `0.0` = off
- `0.5` = blink
- `1.0` = on

Pressed physical or mouse-held key state should take priority over app-derived LEDs so a held key is visibly lit.

## Testing Expectations

Add or update targeted tests before implementation:

- Subharmoniq page binding tests for Seq/Rhythm, VCO, VCF, and VCA/Mix pages
- Field key mapping tests proving A/B short-press behavior is stable across pages
- LED state tests for active steps, rhythm targets, quantize, seq octave, play, and reset
- Render/runtime tests proving Field surface events still route to the correct parameters
- Audio regression test confirming Cutoff and Field modulation do not mute output unexpectedly

Run the targeted Debug checks before handoff:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SubharmoniqCoreTest|RenderRuntimeTest|HostModulationTest)"
cmake --build build --config Debug --target DaisyHostPatch_Standalone
py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --timeout-seconds 60
```

## Non-Goals

- no firmware changes
- no DAW/VST behavior changes
- no unrelated DSP rewrite
- no Patch board behavior changes
- no page-sensitive short-press A/B behavior

