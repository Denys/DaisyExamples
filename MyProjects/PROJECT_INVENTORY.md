# Daisy Projects Inventory
**Generated**: 2026-02-07  
**Purpose**: Catalog all buildable Daisy projects in the DVPE workspace

---

## Table of Contents

| Location | Projects | Status |
|----------|----------|--------|
| [MyProjects/_projects](#myprojects_projects) | 24 | Primary development |
| [MyProjects/_experiments](#myprojects_experiments) | 5 | Experimental/WIP |
| [field/ (official examples)](#field-official-examples) | 10 | Reference implementations |
| [pod/ (official examples)](#pod-official-examples) | ~20 | Reference implementations |
| [seed/ (official examples)](#seed-official-examples) | ~30 | Reference implementations |

---

## MyProjects/_projects

**Path**: `DaisyExamples/MyProjects/_projects/`  
**Status**: Primary development folder - these are YOUR projects

| Project | Platform | Description | Verified |
|---------|----------|-------------|----------|
| [DrumMachine](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/DrumMachine) | Field | Drum machine | ❓ |
| [FieldArpeggiator](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/FieldArpeggiator) | Field | MIDI Arpeggiator (works, has clicking) | ✅ |
| [Field_DrumMachinePro](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Field_DrumMachinePro) | ⚠️ Pod (mislabeled) | Pod prototype — uses daisy_pod.h despite Field name. Broken swing, hard-clip mix. | ❌ Wrong platform |
| [FieldOpus_DrumMachinePro](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/FieldOpus_DrumMachinePro) | Field | 6-voice 16-step drum machine, 8 patterns, OLED, soft-clip. Replaces Field_DrumMachinePro. | ❓ |
| [Field_EuclideanRhythmist](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Field_EuclideanRhythmist) | Field | Euclidean rhythm sequencer | ❓ |
| [Field_ModalBells](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Field_ModalBells) | Field | Modal synthesis bells | ❌ Clicking noise |
| [Field_WavetableDroneLab](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Field_WavetableDroneLab) | Field | Self-evolving drone with LFO wavetable morphing | ✅ Build verified |
| [GeneticSequencer](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/GeneticSequencer) | Field | Genetic algorithm sequencer | ❓ |
| [Midi_FIELD](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Midi_FIELD) | Field | MIDI example for Field | ❓ |
| [Midi_POD](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Midi_POD) | Pod | MIDI example for Pod | ❓ |
| [Pod_Harmonizer](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Pod_Harmonizer) | Pod | Pitch harmonizer | ❓ |
| [Pod_MultiFX_Chain](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Pod_MultiFX_Chain) | Pod | Multi-effects processor | ❓ |
| [Pomodoro_Pod](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Pomodoro_Pod) | Pod | Pomodoro timer | ❓ |
| [RetroStepSequencer_Field](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/RetroStepSequencer_Field) | Field | Step sequencer | ❓ |
| [Sequencer](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/Sequencer) | ? | Basic sequencer | ❓ |
| [dual_osc_subtractive](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/dual_osc_subtractive) | Field | Dual oscillator subtractive synth | ❓ |
| [ex_tube_field](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/ex_tube_field) | Field | Tube saturation effect | ❓ |
| [ex_tube_seed](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/ex_tube_seed) | Seed | Tube saturation effect | ❓ |
| [field_string_machine](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/field_string_machine) | Field | String machine synth | ❓ |
| [field_string_machine_poly](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/field_string_machine_poly) | Field | Polyphonic string machine | ❓ |
| [field_wavetable_morph_synth](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/field_wavetable_morph_synth) | Field | Wavetable morphing synthesizer | ✅ Known working |
| [fm_synth_field](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/fm_synth_field) | Field | FM synthesizer | ❓ |
| [fm_synth_field_simple](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/fm_synth_field_simple) | Field | Simple FM synth | ❓ |
| [multi_fx_synth_pod](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/multi_fx_synth_pod) | Pod | Multi-FX synth | ❓ |
| [stringvoice_overdrive_reverb](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_projects/stringvoice_overdrive_reverb) | Field | StringVoice with FX chain | ❓ |

---

## MyProjects/_experiments

**Path**: `DaisyExamples/MyProjects/_experiments/`  
**Status**: Experimental/work-in-progress

| Project | Description | Verified |
|---------|-------------|----------|
| [ExampleProjec_openrouter](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_experiments/ExampleProjec_openrouter) | OpenRouter integration test | ❓ |
| [ExampleProject](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_experiments/ExampleProject) | Basic example | ❓ |
| [MyExampleProject](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_experiments/MyExampleProject) | Custom example | ❓ |
| [MyKeyboardTest](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/_experiments/MyKeyboardTest) | Keyboard input testing | ❓ |

---

## field/ (Official Examples)

**Path**: `DaisyExamples/field/`  
**Status**: Official Electrosmith examples - reference implementations

| Project | Description |
|---------|-------------|
| [KeyboardTest](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/KeyboardTest) | Keyboard testing |
| [Midi](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/Midi) | **MIDI template (VoiceManager)** |
| [Nimbus](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/Nimbus) | Granular processor |
| [chorus](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/chorus) | Chorus effect |
| [flanger](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/flanger) | Flanger effect |
| [modalvoice](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/modalvoice) | Modal synthesis voice |
| [phaser](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/phaser) | Phaser effect |
| [sampler](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/sampler) | Audio sampler |
| [stringvoice](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/field/stringvoice) | String synthesis |

---

## Backup Projects

| Project | Description |
|---------|-------------|
| [field_wavetable_morph_synth_backup](file:///c:/Users/denko/Gemini/Antigravity/DVPE_Daisy-Visual-Programming-Environment/DaisyExamples/MyProjects/field_wavetable_morph_synth_backup) | Backup of wavetable synth |

---

## Verification Queue

Projects to verify (in priority order):

1. ❓ **Field_ModalBells** - Testing now
2. ❓ **Field_EuclideanRhythmist** - Previous debug session
3. ❓ **field_string_machine** / **field_string_machine_poly**
4. ❓ **fm_synth_field** / **fm_synth_field_simple**

---

## Notes

- ✅ = Verified working
- ❓ = Needs verification
- ❌ = Known broken
- 🔧 = Needs fixes

**Last verified working**:
- `field_wavetable_morph_synth` - Known good baseline
- `FieldArpeggiator` - Works but has clicking artifact
