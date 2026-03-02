# DVPE — Static API Reference Injection
## Implementation Instructions for Claude Code

---

## Context & Goal

Implement **Tier 1 Static API Injection** in the DVPE code generation pipeline.
This feature prepends a structured DaisySP API reference comment block to every
generated `.cpp` file, grounding all subsequent code (human-edited or AI-generated)
against the real DaisySP method signatures present in the patch.

**Why this matters**: LLMs and developers both hallucinate DaisySP APIs.
Common failure patterns: `SetCutoff()` instead of `SetFreq()`, wrong parameter
order in `Adsr::SetTime()`, inventing `filt.setCutoffFrequency()`. The injected
comment block eliminates these by placing the ground truth inline.

**No external dependencies. No latency. Zero runtime cost.**

---

## Repository Layout (Relevant Paths)

```
dvpe_CLD/src/
├── core/
│   ├── blocks/
│   │   ├── definitions/          ← 100 block .ts files (read these to extract className)
│   │   └── BlockRegistry.ts      ← getBlock(type) returns BlockDefinition
│   └── codegen/
│       ├── CodeGenerator.ts      ← PRIMARY EDIT TARGET — add injection here
│       ├── daisyApiIndex.ts      ← CREATE THIS FILE
│       └── templates/            ← platform-specific templates (do not modify)
└── types/
    └── block.ts                  ← BlockDefinition interface lives here
```

---

## Step 1 — Understand the BlockDefinition Interface

Before editing anything, read `src/types/block.ts` and confirm the shape of
`BlockDefinition`. Specifically locate:

- `className: string` — should be `'daisysp::MoogLadder'` or similar
- `id: string` — block type identifier used as registry key

Also read `src/core/blocks/BlockRegistry.ts` to understand how `getBlock(type)`
works — you will call it during code generation.

If `className` is stored differently (e.g., as `cppClass` or nested under
`codegen`), note the actual field name and adjust Step 3 accordingly.

---

## Step 2 — Create `src/core/codegen/daisyApiIndex.ts`

Create this file from scratch. It is a static data file — no imports from the
rest of the codebase, no runtime logic.

**Structure:**

```typescript
export interface DaisyMethodRef {
  name: string;
  signature: string;       // full C++ signature string
  paramRanges?: string;    // human-readable range hint
  callRate: 'init' | 'control' | 'audio';
  notes?: string;          // extra caveats
}

export interface DaisyAPIRef {
  className: string;       // bare class name, no namespace
  methods: DaisyMethodRef[];
}

// Key = bare class name (no 'daisysp::' prefix)
export const DAISY_API_INDEX: Record<string, DaisyAPIRef> = {
  // ... entries below
};
```

**Required entries — populate ALL of these exactly:**

```typescript
'Oscillator': {
  className: 'Oscillator',
  methods: [
    { name: 'Init',        signature: 'void Init(float sample_rate)',                                        callRate: 'init' },
    { name: 'SetFreq',     signature: 'void SetFreq(float freq)',         paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetAmp',      signature: 'void SetAmp(float amp)',           paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)',                                  callRate: 'control',
      notes: 'WAVE_SIN WAVE_TRI WAVE_SAW WAVE_RAMP WAVE_SQUARE WAVE_POLYBLEP_TRI WAVE_POLYBLEP_SAW WAVE_POLYBLEP_SQUARE' },
    { name: 'SetPw',       signature: 'void SetPw(float pw)',             paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process',     signature: 'float Process()',                                                     callRate: 'audio' },
  ]
},

'BlOsc': {
  className: 'BlOsc',
  methods: [
    { name: 'Init',        signature: 'void Init(float sample_rate)',                                        callRate: 'init' },
    { name: 'SetFreq',     signature: 'void SetFreq(float freq)',         paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetAmp',      signature: 'void SetAmp(float amp)',           paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)',                                  callRate: 'control',
      notes: 'WAVE_SAW WAVE_SQUARE WAVE_TRI WAVE_OFF' },
    { name: 'Process',     signature: 'float Process()',                                                     callRate: 'audio' },
  ]
},

'MoogLadder': {
  className: 'MoogLadder',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',             paramRanges: '10.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetRes',  signature: 'void SetRes(float res)',               paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process', signature: 'float Process(float in)',                                                 callRate: 'audio' },
  ]
},

'Svf': {
  className: 'Svf',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',             paramRanges: '10.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetRes',  signature: 'void SetRes(float res)',               paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetDrive',signature: 'void SetDrive(float drive)',           paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process', signature: 'void Process(float in)',                                                  callRate: 'audio',
      notes: 'query outputs via Low() High() Band() Notch() Peak()' },
    { name: 'Low',     signature: 'float Low()',                                                             callRate: 'audio' },
    { name: 'High',    signature: 'float High()',                                                            callRate: 'audio' },
    { name: 'Band',    signature: 'float Band()',                                                            callRate: 'audio' },
    { name: 'Notch',   signature: 'float Notch()',                                                           callRate: 'audio' },
  ]
},

'Adsr': {
  className: 'Adsr',
  methods: [
    { name: 'Init',            signature: 'void Init(float sample_rate)',                                    callRate: 'init' },
    { name: 'SetTime',         signature: 'void SetTime(Adsr::SegIndex seg, float time)',
      paramRanges: 'time: 0.001 - 10.0 s', callRate: 'control',
      notes: 'seg: ADSR_SEG_ATTACK | ADSR_SEG_DECAY | ADSR_SEG_RELEASE' },
    { name: 'SetSustainLevel', signature: 'void SetSustainLevel(float level)', paramRanges: '0.0 - 1.0',   callRate: 'control' },
    { name: 'Process',         signature: 'float Process(bool gate)',                                        callRate: 'audio' },
    { name: 'IsRunning',       signature: 'bool IsRunning()',                                                callRate: 'audio' },
  ]
},

'AdEnv': {
  className: 'AdEnv',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetTime', signature: 'void SetTime(AdEnv::SegIndex seg, float time)',
      paramRanges: 'time: 0.001 - 10.0 s', callRate: 'control',
      notes: 'seg: ADENV_SEG_ATTACK | ADENV_SEG_DECAY' },
    { name: 'SetCurve',signature: 'void SetCurve(float shape)',           paramRanges: '-1.0 - 1.0',        callRate: 'control' },
    { name: 'SetMax',  signature: 'void SetMax(float max)',               paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetMin',  signature: 'void SetMin(float min)',               paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process', signature: 'float Process(bool trig)',                                                callRate: 'audio' },
  ]
},

'ReverbSc': {
  className: 'ReverbSc',
  methods: [
    { name: 'Init',      signature: 'void Init(float sample_rate)',                                          callRate: 'init' },
    { name: 'SetFeedback', signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0',        callRate: 'control' },
    { name: 'SetLpFreq',   signature: 'void SetLpFreq(float lpfreq)',    paramRanges: '0.0 - 20000.0 Hz',  callRate: 'control' },
    { name: 'Process',   signature: 'void Process(float in1, float in2, float *out1, float *out2)',          callRate: 'audio' },
  ]
},

'Chorus': {
  className: 'Chorus',
  methods: [
    { name: 'Init',       signature: 'void Init(float sample_rate)',                                         callRate: 'init' },
    { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float freq)',      paramRanges: '0.0 - 10.0 Hz',     callRate: 'control' },
    { name: 'SetLfoDepth',signature: 'void SetLfoDepth(float depth)',    paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetDelay',   signature: 'void SetDelay(float delay)',       paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process',    signature: 'float Process(float in)',                                              callRate: 'audio' },
    { name: 'GetLeft',    signature: 'float GetLeft()',                                                      callRate: 'audio' },
    { name: 'GetRight',   signature: 'float GetRight()',                                                     callRate: 'audio' },
  ]
},

'Phaser': {
  className: 'Phaser',
  methods: [
    { name: 'Init',       signature: 'void Init(float sample_rate)',                                         callRate: 'init' },
    { name: 'SetFreq',    signature: 'void SetFreq(float freq)',         paramRanges: '0.0 - 20000.0 Hz',  callRate: 'control' },
    { name: 'SetFeedback',signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetLfoDepth',signature: 'void SetLfoDepth(float depth)',    paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float lfofreq)',   paramRanges: '0.0 - 20.0 Hz',    callRate: 'control' },
    { name: 'Process',    signature: 'float Process(float in)',                                              callRate: 'audio' },
  ]
},

'Flanger': {
  className: 'Flanger',
  methods: [
    { name: 'Init',       signature: 'void Init(float sample_rate)',                                         callRate: 'init' },
    { name: 'SetFeedback',signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float lfofreq)',   paramRanges: '0.0 - 20.0 Hz',    callRate: 'control' },
    { name: 'SetLfoDepth',signature: 'void SetLfoDepth(float depth)',    paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process',    signature: 'float Process(float in)',                                              callRate: 'audio' },
  ]
},

'Tremolo': {
  className: 'Tremolo',
  methods: [
    { name: 'Init',     signature: 'void Init(float sample_rate)',                                           callRate: 'init' },
    { name: 'SetFreq',  signature: 'void SetFreq(float freq)',           paramRanges: '0.01 - 100.0 Hz',   callRate: 'control' },
    { name: 'SetDepth', signature: 'void SetDepth(float depth)',         paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)',                                  callRate: 'control' },
    { name: 'Process',  signature: 'float Process(float in)',                                                callRate: 'audio' },
  ]
},

'Compressor': {
  className: 'Compressor',
  methods: [
    { name: 'Init',          signature: 'void Init(float sample_rate)',                                      callRate: 'init' },
    { name: 'SetThreshold',  signature: 'void SetThreshold(float threshold)', paramRanges: '-80.0 - 0.0 dB', callRate: 'control' },
    { name: 'SetRatio',      signature: 'void SetRatio(float ratio)',    paramRanges: '1.0 - 40.0',         callRate: 'control' },
    { name: 'SetAttack',     signature: 'void SetAttack(float attack)',  paramRanges: '0.001 - 10.0 s',    callRate: 'control' },
    { name: 'SetRelease',    signature: 'void SetRelease(float release)',paramRanges: '0.001 - 10.0 s',    callRate: 'control' },
    { name: 'SetMakeup',     signature: 'void SetMakeup(float makeup)',  paramRanges: '0.0 - 80.0 dB',     callRate: 'control' },
    { name: 'Process',       signature: 'float Process(float in)',                                           callRate: 'audio' },
    { name: 'ProcessStereo', signature: 'void ProcessStereo(float inL, float inR, float *outL, float *outR)', callRate: 'audio' },
  ]
},

'Overdrive': {
  className: 'Overdrive',
  methods: [
    { name: 'Init',    signature: 'void Init()',                                                              callRate: 'init' },
    { name: 'SetDrive',signature: 'void SetDrive(float drive)',          paramRanges: '0.0 - 1.0',          callRate: 'control' },
    { name: 'Process', signature: 'float Process(float in)',                                                  callRate: 'audio' },
  ]
},

'Bitcrush': {
  className: 'Bitcrush',
  methods: [
    { name: 'Init',         signature: 'void Init(float sample_rate)',                                       callRate: 'init' },
    { name: 'SetBitDepth',  signature: 'void SetBitDepth(float bitdepth)', paramRanges: '1.0 - 32.0 bits', callRate: 'control' },
    { name: 'SetCrushRate', signature: 'void SetCrushRate(float crushrate)', paramRanges: '0.0 - 1.0',     callRate: 'control' },
    { name: 'Process',      signature: 'float Process(float in)',                                            callRate: 'audio' },
  ]
},

'Decimator': {
  className: 'Decimator',
  methods: [
    { name: 'Init',              signature: 'void Init()',                                                    callRate: 'init' },
    { name: 'SetDownsampleFactor', signature: 'void SetDownsampleFactor(float factor)', paramRanges: '0.0 - 1.0', callRate: 'control' },
    { name: 'SetBitDepth',       signature: 'void SetBitDepth(float bitdepth)', paramRanges: '1.0 - 32.0', callRate: 'control' },
    { name: 'Process',           signature: 'float Process(float in)',                                       callRate: 'audio' },
  ]
},

'PitchShifter': {
  className: 'PitchShifter',
  methods: [
    { name: 'Init',         signature: 'void Init(float sample_rate)',                                       callRate: 'init' },
    { name: 'SetTranspose', signature: 'void SetTranspose(float transpose)', paramRanges: '-24.0 - 24.0 semitones', callRate: 'control' },
    { name: 'SetDelSize',   signature: 'void SetDelSize(size_t size)',                                       callRate: 'init' },
    { name: 'Process',      signature: 'float Process(float in)',                                            callRate: 'audio' },
  ]
},

'Tone': {
  className: 'Tone',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',            paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'Process', signature: 'float Process(float in)',                                                 callRate: 'audio' },
  ]
},

'ATone': {
  className: 'ATone',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',            paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'Process', signature: 'float Process(float in)',                                                 callRate: 'audio' },
  ]
},

'Biquad': {
  className: 'Biquad',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',            paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetRes',  signature: 'void SetRes(float res)',              paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process', signature: 'float Process(float in)',                                                 callRate: 'audio' },
  ]
},

'WhiteNoise': {
  className: 'WhiteNoise',
  methods: [
    { name: 'Init',    signature: 'void Init()',                                                              callRate: 'init' },
    { name: 'SetAmp',  signature: 'void SetAmp(float amp)',              paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process', signature: 'float Process()',                                                          callRate: 'audio' },
  ]
},

'Fm2': {
  className: 'Fm2',
  methods: [
    { name: 'Init',     signature: 'void Init(float sample_rate)',                                           callRate: 'init' },
    { name: 'SetFreq',  signature: 'void SetFrequency(float freq)',      paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetRatio', signature: 'void SetRatio(float ratio)',         paramRanges: '0.0 - 10.0',        callRate: 'control' },
    { name: 'SetIndex', signature: 'void SetIndex(float index)',         paramRanges: '0.0 - 12.0',        callRate: 'control' },
    { name: 'Process',  signature: 'float Process()',                                                         callRate: 'audio' },
  ]
},

'StringVoice': {
  className: 'StringVoice',
  methods: [
    { name: 'Init',         signature: 'void Init(float sample_rate)',                                       callRate: 'init' },
    { name: 'SetFreq',      signature: 'void SetFreq(float freq)',       paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
    { name: 'SetAccent',    signature: 'void SetAccent(float accent)',   paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetStructure', signature: 'void SetStructure(float structure)', paramRanges: '0.0 - 1.0',    callRate: 'control' },
    { name: 'SetBrightness',signature: 'void SetBrightness(float brightness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
    { name: 'SetDamping',   signature: 'void SetDamping(float damping)',  paramRanges: '0.0 - 1.0',       callRate: 'control' },
    { name: 'Process',      signature: 'float Process(bool trig)',                                           callRate: 'audio' },
  ]
},

'AnalogBassDrum': {
  className: 'AnalogBassDrum',
  methods: [
    { name: 'Init',       signature: 'void Init(float sample_rate)',                                         callRate: 'init' },
    { name: 'SetFreq',    signature: 'void SetFreq(float freq)',         paramRanges: '20.0 - 1000.0 Hz',  callRate: 'control' },
    { name: 'SetAccent',  signature: 'void SetAccent(float accent)',     paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetDecay',   signature: 'void SetDecay(float decay)',       paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetTone',    signature: 'void SetTone(float tone)',         paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetAttackFm',signature: 'void SetAttackFmAmount(float amt)',paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetSelfFm',  signature: 'void SetSelfFmAmount(float amt)', paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process',    signature: 'float Process(bool trig)',                                              callRate: 'audio' },
  ]
},

'AnalogSnareDrum': {
  className: 'AnalogSnareDrum',
  methods: [
    { name: 'Init',      signature: 'void Init(float sample_rate)',                                          callRate: 'init' },
    { name: 'SetFreq',   signature: 'void SetFreq(float freq)',          paramRanges: '20.0 - 1000.0 Hz',  callRate: 'control' },
    { name: 'SetAccent', signature: 'void SetAccent(float accent)',      paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetDecay',  signature: 'void SetDecay(float decay)',        paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetSnappy', signature: 'void SetSnappy(float snappy)',      paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'SetTone',   signature: 'void SetTone(float tone)',          paramRanges: '0.0 - 1.0',         callRate: 'control' },
    { name: 'Process',   signature: 'float Process(bool trig)',                                              callRate: 'audio' },
  ]
},

'Metro': {
  className: 'Metro',
  methods: [
    { name: 'Init',    signature: 'void Init(float freq, float sample_rate)',  paramRanges: 'freq: 0.01 - 1000.0 Hz', callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',            paramRanges: '0.01 - 1000.0 Hz',  callRate: 'control' },
    { name: 'Process', signature: 'bool Process()',                                                           callRate: 'audio' },
  ]
},

'CrossFade': {
  className: 'CrossFade',
  methods: [
    { name: 'Init',    signature: 'void Init()',                                                              callRate: 'init' },
    { name: 'SetPos',  signature: 'void SetPos(float pos)',              paramRanges: '0.0 - 1.0',          callRate: 'control' },
    { name: 'SetCurve',signature: 'void SetCurve(CrossFade::Type type)',                                     callRate: 'control',
      notes: 'CROSSFADE_LIN | CROSSFADE_CPOW' },
    { name: 'Process', signature: 'float Process(float in1, float in2)',                                      callRate: 'audio' },
  ]
},

'Port': {
  className: 'Port',
  methods: [
    { name: 'Init',     signature: 'void Init(float sample_rate, float htime)',                              callRate: 'init' },
    { name: 'SetHtime', signature: 'void SetHtime(float htime)',         paramRanges: '0.0 - 1.0',          callRate: 'control' },
    { name: 'Process',  signature: 'float Process(float in)',                                                 callRate: 'audio' },
  ]
},

'DcBlock': {
  className: 'DcBlock',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate)',                                            callRate: 'init' },
    { name: 'Process', signature: 'float Process(float in)',                                                  callRate: 'audio' },
  ]
},

'Phasor': {
  className: 'Phasor',
  methods: [
    { name: 'Init',    signature: 'void Init(float sample_rate, float freq)',                                callRate: 'init' },
    { name: 'SetFreq', signature: 'void SetFreq(float freq)',            paramRanges: '0.0 - 20000.0 Hz',  callRate: 'control' },
    { name: 'Process', signature: 'float Process()',                                                          callRate: 'audio' },
  ]
},
```

Add remaining classes from the DaisySP module reference using the same pattern.
Use ONLY method signatures verified from `daisysp_reference.pdf` in project files.
Do NOT invent signatures — if uncertain, mark with `// TODO: verify signature`.

---

## Step 3 — Create `src/core/codegen/apiReferenceInjector.ts`

This module is the functional core. It takes the list of nodes from the patch
graph, extracts which DaisySP classes are needed, and builds the comment block.

```typescript
import { DAISY_API_INDEX } from './daisyApiIndex';
import { BlockRegistry } from '../blocks/BlockRegistry';

// Matches how BlockDefinition stores the C++ class name.
// Adjust the field access if your schema uses a different property name.
const NAMESPACE_PREFIX = 'daisysp::';

/**
 * Extracts the bare DaisySP class name from a block's className field.
 * Handles both 'daisysp::MoogLadder' and 'MoogLadder' formats.
 */
function extractClassName(rawClassName: string): string {
  return rawClassName.startsWith(NAMESPACE_PREFIX)
    ? rawClassName.slice(NAMESPACE_PREFIX.length)
    : rawClassName;
}

/**
 * Given an array of node type IDs (from the patch graph),
 * returns a formatted C++ comment block with API references
 * for every DaisySP class used in the patch.
 *
 * Returns empty string if no known DaisySP classes are found
 * (e.g., pure I/O-only patches).
 */
export function buildApiReferenceHeader(nodeTypes: string[]): string {
  // Collect unique class names present in this patch
  const classesUsed = new Set<string>();

  for (const nodeType of nodeTypes) {
    const blockDef = BlockRegistry.getBlock(nodeType);
    if (!blockDef?.className) continue;
    const bare = extractClassName(blockDef.className);
    if (DAISY_API_INDEX[bare]) {
      classesUsed.add(bare);
    }
  }

  if (classesUsed.size === 0) return '';

  const lines: string[] = [
    '// ============================================================',
    '// DAISYSP API REFERENCE — auto-generated by DVPE',
    '// Classes present in this patch:',
    '// ============================================================',
  ];

  for (const cls of [...classesUsed].sort()) {
    const ref = DAISY_API_INDEX[cls];
    lines.push(`//`);
    lines.push(`// [${cls}]`);

    for (const method of ref.methods) {
      // Align call-rate tag to column 60 for readability
      let line = `//   ${method.signature}`;
      const tag = `[${method.callRate}]`;

      if (method.paramRanges) {
        line += `  // ${method.paramRanges}  ${tag}`;
      } else {
        line += `  // ${tag}`;
      }
      lines.push(line);

      if (method.notes) {
        lines.push(`//     ^ ${method.notes}`);
      }
    }
  }

  lines.push('// ============================================================');
  lines.push('');

  return lines.join('\n') + '\n';
}
```

**Test this function in isolation before touching CodeGenerator.ts:**

```typescript
// Quick smoke test — run with ts-node or add as a Vitest test
import { buildApiReferenceHeader } from './apiReferenceInjector';
const header = buildApiReferenceHeader(['oscillator', 'moog_ladder', 'adsr']);
console.log(header);
// Expect: structured comment block with Oscillator, MoogLadder, Adsr entries
// Expect: no entries for unknown node types
// Expect: empty string for []
```

---

## Step 4 — Integrate into `CodeGenerator.ts`

Read `CodeGenerator.ts` fully before editing. Identify:

1. The main export function (likely `generateCode`, `generate`, or `exportCode`)
2. Where it receives the patch graph / node list
3. Where it assembles the final output string
4. Whether it returns a `string` or writes to a file handle

Then add the injection at the **top** of the generated C++ output, before the
`#include` directives:

```typescript
import { buildApiReferenceHeader } from './apiReferenceInjector';

// Inside the main generate function — locate where nodeTypes/graph is available
// and where the final cpp string is assembled:

export function generateCode(graph: PatchGraph, platform: HardwarePlatform): string {
  // --- EXISTING LOGIC: topological sort, template selection, etc. ---

  // Extract node types from graph
  const nodeTypes = graph.nodes.map(n => n.type);  // adjust to actual graph shape

  // Build API reference header
  const apiHeader = buildApiReferenceHeader(nodeTypes);

  // --- EXISTING LOGIC: build cppIncludes, declarations, callback, main ---

  // Prepend header to final output
  return apiHeader + existingOutput;
  //     ^^^^^^^^^^ ADD THIS — everything else stays unchanged
}
```

**Critical constraint**: The injection point must be BEFORE `#include` lines so
the comment appears at the very top of the file. If the existing code builds the
output in segments, inject into the first segment.

---

## Step 5 — Write Vitest Tests

Add to the existing test suite (follow the pattern in `src/test/`):

```typescript
// src/test/apiReferenceInjector.test.ts

import { describe, it, expect } from 'vitest';
import { buildApiReferenceHeader } from '../core/codegen/apiReferenceInjector';

describe('buildApiReferenceHeader', () => {
  it('returns empty string for empty node list', () => {
    expect(buildApiReferenceHeader([])).toBe('');
  });

  it('returns empty string for unknown node types only', () => {
    expect(buildApiReferenceHeader(['some_io_block', 'hardware_output'])).toBe('');
  });

  it('includes MoogLadder API when moog_ladder node present', () => {
    const header = buildApiReferenceHeader(['moog_ladder']);
    expect(header).toContain('[MoogLadder]');
    expect(header).toContain('SetFreq');
    expect(header).toContain('SetRes');
    expect(header).toContain('Process');
    // Verify it does NOT contain hallucinated methods
    expect(header).not.toContain('SetCutoff');
    expect(header).not.toContain('SetCutoffFrequency');
    expect(header).not.toContain('SetQ');
  });

  it('includes Adsr with correct SetTime signature', () => {
    const header = buildApiReferenceHeader(['adsr']);
    expect(header).toContain('SetTime(Adsr::SegIndex seg, float time)');
    expect(header).toContain('ADSR_SEG_ATTACK');
    // Must NOT appear:
    expect(header).not.toContain('SetAttack(');
    expect(header).not.toContain('SetDecay(');
    expect(header).not.toContain('SetRelease(');
  });

  it('deduplicates same class used multiple times', () => {
    const header = buildApiReferenceHeader(['oscillator', 'oscillator', 'oscillator']);
    const count = (header.match(/\[Oscillator\]/g) ?? []).length;
    expect(count).toBe(1);
  });

  it('sorts class entries alphabetically', () => {
    const header = buildApiReferenceHeader(['adsr', 'oscillator', 'moog_ladder']);
    const adsrPos = header.indexOf('[Adsr]');
    const moogPos = header.indexOf('[MoogLadder]');
    const oscPos  = header.indexOf('[Oscillator]');
    expect(adsrPos).toBeLessThan(moogPos);
    expect(moogPos).toBeLessThan(oscPos);
  });

  it('emits call-rate tags on every method', () => {
    const header = buildApiReferenceHeader(['moog_ladder']);
    // Every line with a method signature should have a [init|control|audio] tag
    const methodLines = header.split('\n')
      .filter(l => l.startsWith('//   '));
    for (const line of methodLines) {
      expect(line).toMatch(/\[(init|control|audio)\]/);
    }
  });

  it('full patch produces valid comment block structure', () => {
    const header = buildApiReferenceHeader([
      'oscillator', 'moog_ladder', 'adsr', 'reverb'
    ]);
    expect(header.startsWith('// =====')).toBe(true);
    expect(header.endsWith('\n\n')).toBe(true);
    // No stray TypeScript syntax in output
    expect(header).not.toContain('export');
    expect(header).not.toContain('import');
    expect(header).not.toContain('=>');
  });
});
```

Run: `npm test` — all 475+ existing tests must still pass.

---

## Step 6 — Verify Generated Output

Export any existing `.dvpe` example patch (use files in `_block_diagrams_code/`)
and inspect the `.cpp` output:

**Expected output header (example for OSC → MoogLadder → ADSR patch):**

```cpp
// ============================================================
// DAISYSP API REFERENCE — auto-generated by DVPE
// Classes present in this patch:
// ============================================================
//
// [Adsr]
//   void Init(float sample_rate)  // [init]
//   void SetTime(Adsr::SegIndex seg, float time)  // 0.001 - 10.0 s  [control]
//     ^ seg: ADSR_SEG_ATTACK | ADSR_SEG_DECAY | ADSR_SEG_RELEASE
//   void SetSustainLevel(float level)  // 0.0 - 1.0  [control]
//   float Process(bool gate)  // [audio]
//   bool IsRunning()  // [audio]
//
// [MoogLadder]
//   void Init(float sample_rate)  // [init]
//   void SetFreq(float freq)  // 10.0 - 20000.0 Hz  [control]
//   void SetRes(float res)  // 0.0 - 1.0  [control]
//   float Process(float in)  // [audio]
//
// [Oscillator]
//   void Init(float sample_rate)  // [init]
//   void SetFreq(float freq)  // 20.0 - 20000.0 Hz  [control]
//   void SetAmp(float amp)  // 0.0 - 1.0  [control]
//   void SetWaveform(uint8_t waveform)  // [control]
//     ^ WAVE_SIN WAVE_TRI WAVE_SAW WAVE_RAMP ...
//   void SetPw(float pw)  // 0.0 - 1.0  [control]
//   float Process()  // [audio]
//
// ============================================================

#include "daisy_field.h"
#include "daisysp.h"
// ... rest of generated code
```

**Failure criteria — fix before committing:**
- Any `#include` line appears ABOVE the comment block
- Any method not present in `daisyApiIndex.ts` appears in the block
- Any hallucinated method name appears (SetCutoff, SetAttack, etc.)
- Comment block appears for I/O-only patches (should be empty string)
- Existing test suite has regressions

---

## Step 7 — Extending the Index

When adding new blocks to DVPE, update `daisyApiIndex.ts` simultaneously.
This is a required step in the block development workflow defined in `CONTRIBUTING.md`.

**Signature verification protocol:**
1. Check `daisysp_reference.pdf` (in project files) for the exact method signature
2. Cross-reference against the DaisySP GitHub source if available
3. Never infer signatures from class names or parameter names alone
4. Mark unverified entries with `// TODO: verify` rather than guessing

---

## Files Modified / Created

| Action | Path |
|--------|------|
| CREATE | `src/core/codegen/daisyApiIndex.ts` |
| CREATE | `src/core/codegen/apiReferenceInjector.ts` |
| CREATE | `src/test/apiReferenceInjector.test.ts` |
| MODIFY | `src/core/codegen/CodeGenerator.ts` — prepend header in generate function |

`BlockRegistry.ts`, `block.ts`, `templates/` — **read only, do not modify.**

---

## Definition of Done

- [ ] `npm test` passes with 0 regressions
- [ ] New tests in `apiReferenceInjector.test.ts` all pass
- [ ] Exported `.cpp` from a test patch contains the API header block
- [ ] Header appears before `#include` directives
- [ ] No hallucinated method names in any header output
- [ ] Empty string returned for patches with no DaisySP DSP blocks
- [ ] `daisyApiIndex.ts` covers all 73 DaisySP classes or TODOs filed for missing ones
