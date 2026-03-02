/**
 * daisyApiIndex.ts — Static DaisySP API Reference Index
 *
 * CANONICAL SOURCE OF TRUTH for DaisySP method signatures.
 * Used by apiReferenceInjector.ts to generate grounding comment blocks
 * that prevent LLM hallucinations in generated Daisy C++ code.
 *
 * Signature verification protocol:
 *   1. Primary:   DaisySP/Source/<module>/<module>.h headers
 *   2. Secondary: DAISY_TUTORIALS_KNOWLEDGE.md + known working projects
 *   3. Uncertain: Mark with // TODO: verify signature
 *   4. NEVER infer signatures from class names or parameter names alone
 *
 * callRate notation:
 *   'init'    — Call once during initialization (before StartAudio)
 *   'control' — Call in main loop at control rate (~60Hz)
 *   'audio'   — Call per-sample inside AudioCallback
 */

export interface DaisyMethodRef {
    name: string;
    signature: string;        // Full C++ method signature
    paramRanges?: string;     // Human-readable range hint
    callRate: 'init' | 'control' | 'audio';
    notes?: string;           // Caveats, enum values, common mistakes
}

export interface DaisyAPIRef {
    className: string;        // Bare class name (no 'daisysp::' prefix)
    namespace: string;        // Usually 'daisysp'
    header: string;           // DaisySP source header filename
    methods: DaisyMethodRef[];
}

// Key = bare class name (no 'daisysp::' prefix)
export const DAISY_API_INDEX: Record<string, DaisyAPIRef> = {

    // ─────────────────────────────────────────────
    // OSCILLATORS
    // ─────────────────────────────────────────────

    'Oscillator': {
        className: 'Oscillator',
        namespace: 'daisysp',
        header: 'oscillator.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetAmp', signature: 'void SetAmp(float amp)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)', callRate: 'control',
                notes: 'WAVE_SIN | WAVE_TRI | WAVE_SAW | WAVE_RAMP | WAVE_SQUARE | WAVE_POLYBLEP_TRI | WAVE_POLYBLEP_SAW | WAVE_POLYBLEP_SQUARE'
            },
            {
                name: 'SetPw', signature: 'void SetPw(float pw)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: 'Pulse width — only affects WAVE_SQUARE waveforms'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
            {
                name: 'PhaseAdd', signature: 'void PhaseAdd(float _phase)', paramRanges: '0.0 - 1.0', callRate: 'audio',
                notes: 'Sync/phase modulation. Resets phase on next cycle.'
            },
            {
                name: 'Reset', signature: 'void Reset()', callRate: 'audio',
                notes: 'Resets phase to 0'
            },
        ]
    },

    'BlOsc': {
        className: 'BlOsc',
        namespace: 'daisysp',
        header: 'blosc.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetAmp', signature: 'void SetAmp(float amp)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)', callRate: 'control',
                notes: 'WAVE_SAW | WAVE_SQUARE | WAVE_TRI | WAVE_OFF'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'VariableShapeOscillator': {
        className: 'VariableShapeOscillator',
        namespace: 'daisysp',
        header: 'variableshapeoscillator.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetPW', signature: 'void SetPW(float pw)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetWaveshape', signature: 'void SetWaveshape(float waveshape)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: '0=sine, 0.5=triangle, 1.0=square. Morphs smoothly between shapes.'
            },
            { name: 'SetSync', signature: 'void SetSync(bool sync)', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'VariableSawOscillator': {
        className: 'VariableSawOscillator',
        namespace: 'daisysp',
        header: 'variablesawoscillator.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetPW', signature: 'void SetPW(float pw)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'Phasor': {
        className: 'Phasor',
        namespace: 'daisysp',
        header: 'phasor.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate, float freq)', paramRanges: 'freq: 0.0 - 20000.0 Hz', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'Process', signature: 'float Process()', callRate: 'audio',
                notes: 'Output: 0.0 to 1.0 ramp. Use as wavetable position or phase.'
            },
        ]
    },

    'WhiteNoise': {
        className: 'WhiteNoise',
        namespace: 'daisysp',
        header: 'whitenoise.h',
        methods: [
            { name: 'Init', signature: 'void Init()', callRate: 'init' },
            { name: 'SetAmp', signature: 'void SetAmp(float amp)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'Fm2': {
        className: 'Fm2',
        namespace: 'daisysp',
        header: 'fm2.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetFrequency', signature: 'void SetFrequency(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control',
                notes: '⚠ SetFrequency (not SetFreq) — only class in DaisySP with this name'
            },
            { name: 'SetRatio', signature: 'void SetRatio(float ratio)', paramRanges: '0.0 - 10.0', callRate: 'control' },
            { name: 'SetIndex', signature: 'void SetIndex(float index)', paramRanges: '0.0 - 12.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'OscillatorBank': {
        className: 'OscillatorBank',
        namespace: 'daisysp',
        header: 'oscillatorbank.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'SetAmp', signature: 'void SetAmp(size_t partial, float amp)', paramRanges: 'amp: 0.0 - 1.0', callRate: 'control',
                notes: 'partial: 0-6 (for 7-oscillator bank)'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'FormantOscillator': {
        className: 'FormantOscillator',
        namespace: 'daisysp',
        header: 'formantoscillator.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetFormantFreq', signature: 'void SetFormantFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetPhase', signature: 'void SetPhase(float phase)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // PHYSICAL MODELING
    // ─────────────────────────────────────────────

    'StringVoice': {
        className: 'StringVoice',
        namespace: 'daisysp',
        header: 'stringvoice.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetStructure', signature: 'void SetStructure(float structure)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetBrightness', signature: 'void SetBrightness(float brightness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDamping', signature: 'void SetDamping(float damping)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio',
                notes: 'trig: true to pluck/trigger. Subsequent samples return decay.'
            },
            {
                name: 'Trig', signature: 'void Trig()', callRate: 'audio',
                notes: 'Alternative trigger. Call once; then Process(false) for decay.'
            },
        ]
    },

    'ModalVoice': {
        className: 'ModalVoice',
        namespace: 'daisysp',
        header: 'modalvoice.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetStructure', signature: 'void SetStructure(float structure)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetBrightness', signature: 'void SetBrightness(float brightness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
            { name: 'Trig', signature: 'void Trig()', callRate: 'audio' },
        ]
    },

    'Pluck': {
        className: 'Pluck',
        namespace: 'daisysp',
        header: 'pluck.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate, float buf[], size_t nmax, int mode)', callRate: 'init',
                notes: 'mode: PLUCK_MODE_RECURSIVE | PLUCK_MODE_WEIGHTED_AVERAGE. buf must persist.'
            },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetColor', signature: 'void SetColor(float color)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetAmp', signature: 'void SetAmp(float amp)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    'Drip': {
        className: 'Drip',
        namespace: 'daisysp',
        header: 'drip.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate, float dettack)', callRate: 'init' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // DRUM SYNTHESIS
    // ─────────────────────────────────────────────

    'AnalogBassDrum': {
        className: 'AnalogBassDrum',
        namespace: 'daisysp',
        header: 'analogbassdrum.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 1000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetTone', signature: 'void SetTone(float tone)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetAttackFmAmount', signature: 'void SetAttackFmAmount(float amt)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: '⚠ SetAttackFmAmount (not SetAttackFm) — check exact name'
            },
            { name: 'SetSelfFmAmount', signature: 'void SetSelfFmAmount(float amt)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    'AnalogSnareDrum': {
        className: 'AnalogSnareDrum',
        namespace: 'daisysp',
        header: 'analogsnaredrum.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 1000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetSnappy', signature: 'void SetSnappy(float snappy)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetTone', signature: 'void SetTone(float tone)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    'HiHat': {
        className: 'HiHat',
        namespace: 'daisysp',
        header: 'hihat.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetTone', signature: 'void SetTone(float tone)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetNoisiness', signature: 'void SetNoisiness(float noisiness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // FILTERS
    // ─────────────────────────────────────────────

    'MoogLadder': {
        className: 'MoogLadder',
        namespace: 'daisysp',
        header: 'moogladder.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '10.0 - 20000.0 Hz', callRate: 'control',
                notes: '⚠ SetFreq (NOT SetCutoff, NOT SetCutoffFrequency)'
            },
            {
                name: 'SetRes', signature: 'void SetRes(float res)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: '⚠ SetRes (NOT SetQ, NOT SetResonance)'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Svf': {
        className: 'Svf',
        namespace: 'daisysp',
        header: 'svf.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '10.0 - 20000.0 Hz', callRate: 'control',
                notes: '⚠ SetFreq (NOT SetCutoff)'
            },
            { name: 'SetRes', signature: 'void SetRes(float res)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDrive', signature: 'void SetDrive(float drive)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'Process', signature: 'void Process(float in)', callRate: 'audio',
                notes: 'Void return! Query outputs via Low(), High(), Band(), Notch(), Peak() after Process().'
            },
            { name: 'Low', signature: 'float Low()', callRate: 'audio' },
            { name: 'High', signature: 'float High()', callRate: 'audio' },
            { name: 'Band', signature: 'float Band()', callRate: 'audio' },
            { name: 'Notch', signature: 'float Notch()', callRate: 'audio' },
            { name: 'Peak', signature: 'float Peak()', callRate: 'audio' },
        ]
    },

    'Biquad': {
        className: 'Biquad',
        namespace: 'daisysp',
        header: 'biquad.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetRes', signature: 'void SetRes(float res)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'OnePole': {
        className: 'OnePole',
        namespace: 'daisysp',
        header: 'onepole.h',
        methods: [
            { name: 'Init', signature: 'void Init()', callRate: 'init' },
            { name: 'SetFrequency', signature: 'void SetFrequency(float f)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'SetFilterMode', signature: 'void SetFilterMode(FilterMode mode)', callRate: 'control',
                notes: 'FILTER_MODE_LOW_PASS | FILTER_MODE_HIGH_PASS'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Tone': {
        className: 'Tone',
        namespace: 'daisysp',
        header: 'tone.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'ATone': {
        className: 'ATone',
        namespace: 'daisysp',
        header: 'atone.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control',
                notes: 'High-pass filter. ATone = "above Tone".'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Comb': {
        className: 'Comb',
        namespace: 'daisysp',
        header: 'comb.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate, float *buf, size_t size)', callRate: 'init',
                notes: 'buf must persist (static or DSY_SDRAM_BSS for large sizes)'
            },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetRevTime', signature: 'void SetRevTime(float revtime)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // ENVELOPES
    // ─────────────────────────────────────────────

    'Adsr': {
        className: 'Adsr',
        namespace: 'daisysp',
        header: 'adsr.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetTime', signature: 'void SetTime(Adsr::SegIndex seg, float time)',
                paramRanges: 'time: 0.001 - 10.0 s', callRate: 'control',
                notes: '⚠ SetTime (NOT SetAttack/SetDecay/SetRelease). seg: ADSR_SEG_ATTACK | ADSR_SEG_DECAY | ADSR_SEG_RELEASE'
            },
            { name: 'SetSustainLevel', signature: 'void SetSustainLevel(float level)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(bool gate)', callRate: 'audio' },
            { name: 'IsRunning', signature: 'bool IsRunning()', callRate: 'audio' },
        ]
    },

    'AdEnv': {
        className: 'AdEnv',
        namespace: 'daisysp',
        header: 'adenv.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetTime', signature: 'void SetTime(AdEnv::SegIndex seg, float time)',
                paramRanges: 'time: 0.001 - 10.0 s', callRate: 'control',
                notes: '⚠ SetTime (not SetAttack/SetDecay). seg: ADENV_SEG_ATTACK | ADENV_SEG_DECAY'
            },
            { name: 'SetCurve', signature: 'void SetCurve(float shape)', paramRanges: '-1.0 - 1.0', callRate: 'control' },
            { name: 'SetMax', signature: 'void SetMax(float max)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetMin', signature: 'void SetMin(float min)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'Trigger', signature: 'void Trigger()', callRate: 'audio',
                notes: 'Triggers the envelope. Also available: Process(bool trig) variant.'
            },
            { name: 'Process', signature: 'float Process(bool trig)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // EFFECTS
    // ─────────────────────────────────────────────

    'ReverbSc': {
        className: 'ReverbSc',
        namespace: 'daisysp',
        header: 'reverbsc.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init',
                notes: '⚠ Requires USE_DAISYSP_LGPL = 1 in Makefile'
            },
            { name: 'SetFeedback', signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetLpFreq', signature: 'void SetLpFreq(float lpfreq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'Process', signature: 'void Process(float in1, float in2, float *out1, float *out2)', callRate: 'audio',
                notes: 'Stereo. Void return — outputs via pointer args.'
            },
        ]
    },

    'Chorus': {
        className: 'Chorus',
        namespace: 'daisysp',
        header: 'chorus.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float freq)', paramRanges: '0.0 - 10.0 Hz', callRate: 'control' },
            { name: 'SetLfoDepth', signature: 'void SetLfoDepth(float depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDelay', signature: 'void SetDelay(float delay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
            { name: 'GetLeft', signature: 'float GetLeft()', callRate: 'audio' },
            { name: 'GetRight', signature: 'float GetRight()', callRate: 'audio' },
        ]
    },

    'Phaser': {
        className: 'Phaser',
        namespace: 'daisysp',
        header: 'phaser.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetFeedback', signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetLfoDepth', signature: 'void SetLfoDepth(float depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float lfofreq)', paramRanges: '0.0 - 20.0 Hz', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Flanger': {
        className: 'Flanger',
        namespace: 'daisysp',
        header: 'flanger.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFeedback', signature: 'void SetFeedback(float feedback)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetLfoFreq', signature: 'void SetLfoFreq(float lfofreq)', paramRanges: '0.0 - 20.0 Hz', callRate: 'control' },
            { name: 'SetLfoDepth', signature: 'void SetLfoDepth(float depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Tremolo': {
        className: 'Tremolo',
        namespace: 'daisysp',
        header: 'tremolo.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.01 - 100.0 Hz', callRate: 'control' },
            { name: 'SetDepth', signature: 'void SetDepth(float depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetWaveform', signature: 'void SetWaveform(uint8_t waveform)', callRate: 'control',
                notes: 'WAVE_SIN | WAVE_TRI | WAVE_SAW | WAVE_RAMP | WAVE_SQUARE'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Overdrive': {
        className: 'Overdrive',
        namespace: 'daisysp',
        header: 'overdrive.h',
        methods: [
            {
                name: 'Init', signature: 'void Init()', callRate: 'init',
                notes: 'No sample_rate argument!'
            },
            { name: 'SetDrive', signature: 'void SetDrive(float drive)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Decimator': {
        className: 'Decimator',
        namespace: 'daisysp',
        header: 'decimator.h',
        methods: [
            {
                name: 'Init', signature: 'void Init()', callRate: 'init',
                notes: 'No sample_rate argument!'
            },
            { name: 'SetDownsampleFactor', signature: 'void SetDownsampleFactor(float factor)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetBitDepth', signature: 'void SetBitDepth(float bitdepth)', paramRanges: '1.0 - 32.0 bits', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Bitcrush': {
        className: 'Bitcrush',
        namespace: 'daisysp',
        header: 'bitcrush.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetBitDepth', signature: 'void SetBitDepth(float bitdepth)', paramRanges: '1.0 - 32.0 bits', callRate: 'control' },
            { name: 'SetCrushRate', signature: 'void SetCrushRate(float crushrate)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'PitchShifter': {
        className: 'PitchShifter',
        namespace: 'daisysp',
        header: 'pitchshifter.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetTranspose', signature: 'void SetTranspose(float transpose)', paramRanges: '-24.0 - 24.0 semitones', callRate: 'control' },
            { name: 'SetDelSize', signature: 'void SetDelSize(size_t size)', callRate: 'init' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Compressor': {
        className: 'Compressor',
        namespace: 'daisysp',
        header: 'compressor.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetThreshold', signature: 'void SetThreshold(float threshold)', paramRanges: '-80.0 - 0.0 dBFS', callRate: 'control' },
            { name: 'SetRatio', signature: 'void SetRatio(float ratio)', paramRanges: '1.0 - 40.0', callRate: 'control' },
            { name: 'SetAttack', signature: 'void SetAttack(float attack)', paramRanges: '0.001 - 10.0 s', callRate: 'control' },
            { name: 'SetRelease', signature: 'void SetRelease(float release)', paramRanges: '0.001 - 10.0 s', callRate: 'control' },
            { name: 'SetMakeup', signature: 'void SetMakeup(float makeup)', paramRanges: '0.0 - 80.0 dB', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
            { name: 'ProcessStereo', signature: 'void ProcessStereo(float inL, float inR, float *outL, float *outR)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // UTILITIES
    // ─────────────────────────────────────────────

    'Metro': {
        className: 'Metro',
        namespace: 'daisysp',
        header: 'metro.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float freq, float sample_rate)', paramRanges: 'freq: 0.01 - 1000.0 Hz', callRate: 'init',
                notes: 'Note: freq is the FIRST argument (differs from most Init signatures)'
            },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '0.01 - 1000.0 Hz', callRate: 'control' },
            {
                name: 'Process', signature: 'bool Process()', callRate: 'audio',
                notes: 'Returns true only on each beat tick'
            },
        ]
    },

    'CrossFade': {
        className: 'CrossFade',
        namespace: 'daisysp',
        header: 'crossfade.h',
        methods: [
            { name: 'Init', signature: 'void Init()', callRate: 'init' },
            {
                name: 'SetPos', signature: 'void SetPos(float pos)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: '0.0 = full A, 1.0 = full B'
            },
            {
                name: 'SetCurve', signature: 'void SetCurve(CrossFade::Type type)', callRate: 'control',
                notes: 'CROSSFADE_LIN (linear) | CROSSFADE_CPOW (constant-power)'
            },
            { name: 'Process', signature: 'float Process(float in1, float in2)', callRate: 'audio' },
        ]
    },

    'Port': {
        className: 'Port',
        namespace: 'daisysp',
        header: 'port.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate, float htime)', paramRanges: 'htime: 0.0 - 1.0', callRate: 'init',
                notes: 'htime = half-time (portamento time constant). Slew limiter.'
            },
            { name: 'SetHtime', signature: 'void SetHtime(float htime)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'DcBlock': {
        className: 'DcBlock',
        namespace: 'daisysp',
        header: 'dcblock.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'Limiter': {
        className: 'Limiter',
        namespace: 'daisysp',
        header: 'limiter.h',
        methods: [
            { name: 'Init', signature: 'void Init()', callRate: 'init' },
            {
                name: 'ProcessBlock', signature: 'void ProcessBlock(float *in, size_t size, float pre_gain)', callRate: 'audio',
                notes: 'Processes a block in-place. pre_gain: 0.0+ (linear gain before limiting)'
            },
        ]
    },


    // ─────────────────────────────────────────────
    // CHUNK A — DaisySP Gap-Fill
    // ─────────────────────────────────────────────

    'Autowah': {
        className: 'Autowah',
        namespace: 'daisysp',
        header: 'autowah.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetWah', signature: 'void SetWah(float wah)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetDryWet', signature: 'void SetDryWet(float drywet)', paramRanges: '0.0 - 100.0', callRate: 'control',
                notes: '⚠ Range is 0..100, not 0..1'
            },
            { name: 'SetLevel', signature: 'void SetLevel(float level)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    'SyntheticBassDrum': {
        className: 'SyntheticBassDrum',
        namespace: 'daisysp',
        header: 'synthbassdrum.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 1000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetTone', signature: 'void SetTone(float tone)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDirtiness', signature: 'void SetDirtiness(float dirtiness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetFmEnvelopeAmount', signature: 'void SetFmEnvelopeAmount(float amt)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetFmEnvelopeDecay', signature: 'void SetFmEnvelopeDecay(float decay)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetSustain', signature: 'void SetSustain(bool sustain)', callRate: 'control' },
            {
                name: 'Trig', signature: 'void Trig()', callRate: 'audio',
                notes: 'Single-pulse trigger. Call once; Process() returns decay.'
            },
            { name: 'Process', signature: 'float Process(bool trigger = false)', callRate: 'audio' },
        ]
    },

    'SyntheticSnareDrum': {
        className: 'SyntheticSnareDrum',
        namespace: 'daisysp',
        header: 'synthsnaredrum.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float f0)', paramRanges: '20.0 - 1000.0 Hz', callRate: 'control' },
            { name: 'SetAccent', signature: 'void SetAccent(float accent)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetFmAmount', signature: 'void SetFmAmount(float fm_amount)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: 'positive values', callRate: 'control' },
            { name: 'SetSnappy', signature: 'void SetSnappy(float snappy)', paramRanges: '0.0 (drum) - 1.0 (snare)', callRate: 'control' },
            { name: 'SetSustain', signature: 'void SetSustain(bool sustain)', callRate: 'control' },
            { name: 'Trig', signature: 'void Trig()', callRate: 'audio' },
            { name: 'Process', signature: 'float Process(bool trigger = false)', callRate: 'audio' },
        ]
    },

    'Resonator': {
        className: 'Resonator',
        namespace: 'daisysp',
        header: 'resonator.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float position, int resolution, float sample_rate)', callRate: 'init',
                notes: 'position: 0-1 phase offset. resolution: quality vs speed (max 24 modes).'
            },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetStructure', signature: 'void SetStructure(float structure)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetBrightness', signature: 'void SetBrightness(float brightness)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDamping', signature: 'void SetDamping(float damping)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float in)', callRate: 'audio' },
        ]
    },

    'GrainletOscillator': {
        className: 'GrainletOscillator',
        namespace: 'daisysp',
        header: 'grainlet.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetFormantFreq', signature: 'void SetFormantFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'SetShape', signature: 'void SetShape(float shape)', paramRanges: '0.0 - 2.0+', callRate: 'control',
                notes: 'Shapes differently in ranges 0-1, 1-2, and >2'
            },
            { name: 'SetBleed', signature: 'void SetBleed(float bleed)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'VosimOscillator': {
        className: 'VosimOscillator',
        namespace: 'daisysp',
        header: 'vosim.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetForm1Freq', signature: 'void SetForm1Freq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetForm2Freq', signature: 'void SetForm2Freq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetShape', signature: 'void SetShape(float shape)', paramRanges: '-1.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'ZOscillator': {
        className: 'ZOscillator',
        namespace: 'daisysp',
        header: 'zoscillator.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetFormantFreq', signature: 'void SetFormantFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetShape', signature: 'void SetShape(float shape)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            {
                name: 'SetMode', signature: 'void SetMode(float mode)', paramRanges: '-1.0 - 1.0', callRate: 'control',
                notes: '<1/3 = phase shift only, >2/3 = offset only, between = both'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'HarmonicOscillator': {
        className: 'HarmonicOscillator',
        namespace: 'daisysp',
        header: 'harmonic_osc.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init',
                notes: 'Template class: HarmonicOscillator<N> where N = number of harmonics (default 16)'
            },
            { name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetFirstHarmIdx', signature: 'void SetFirstHarmIdx(int idx)', paramRanges: 'idx >= 1', callRate: 'control' },
            { name: 'SetSingleAmp', signature: 'void SetSingleAmp(const float amp, int idx)', paramRanges: 'amp: 0-1, idx: 0..N-1', callRate: 'control' },
            {
                name: 'SetAmplitudes', signature: 'void SetAmplitudes(const float* amplitudes)', callRate: 'control',
                notes: 'Array must be >= num_harmonics. Sum of all amplitudes must be < 1.'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'Particle': {
        className: 'Particle',
        namespace: 'daisysp',
        header: 'particle.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(float frequency)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            {
                name: 'SetResonance', signature: 'void SetResonance(float resonance)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: '⚠ Particle uses SetResonance not SetRes (unique to this class)'
            },
            { name: 'SetRandomFreq', signature: 'void SetRandomFreq(float freq)', callRate: 'control' },
            { name: 'SetDensity', signature: 'void SetDensity(float density)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetGain', signature: 'void SetGain(float gain)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetSpread', signature: 'void SetSpread(float spread)', paramRanges: 'positive values', callRate: 'control' },
            { name: 'SetSync', signature: 'void SetSync(bool sync)', callRate: 'control' },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
            {
                name: 'GetNoise', signature: 'float GetNoise()', callRate: 'audio',
                notes: 'Returns raw noise output. Must call Process() first.'
            },
        ]
    },

    'ClockedNoise': {
        className: 'ClockedNoise',
        namespace: 'daisysp',
        header: 'clockednoise.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            {
                name: 'SetFreq', signature: 'void SetFreq(float freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control',
                notes: 'Frequency at which new random sample is generated (sample & hold rate)'
            },
            {
                name: 'Sync', signature: 'void Sync()', callRate: 'audio',
                notes: 'Forces generation of next random float immediately'
            },
            { name: 'Process', signature: 'float Process()', callRate: 'audio' },
        ]
    },

    'Wavefolder': {
        className: 'Wavefolder',
        namespace: 'daisysp',
        header: 'wavefolder.h',
        methods: [
            {
                name: 'Init', signature: 'void Init()', callRate: 'init',
                notes: 'No sample_rate argument! Folding starts when input magnitude > 1.0'
            },
            {
                name: 'SetGain', signature: 'void SetGain(float gain)', callRate: 'control',
                notes: 'Negative values supported for thru-zero folding'
            },
            {
                name: 'SetOffset', signature: 'void SetOffset(float offset)', callRate: 'control',
                notes: 'Pre-gain offset for asymmetrical folding'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
        ]
    },

    // ─────────────────────────────────────────────
    // CHUNK B — DAFX_2_Daisy_lib Classes
    // Header: DaisyExamples/MyProjects/DAFX_2_Daisy_lib/src/
    // All in namespace daisysp. Methods renamed to match DaisySP conventions.
    // ─────────────────────────────────────────────

    'HighShelving': {
        className: 'HighShelving',
        namespace: 'daisysp',
        header: 'filters/highshelving.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetGain', signature: 'void SetGain(const float &gain)', paramRanges: '-20.0 to +20.0 dB', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'LowShelving': {
        className: 'LowShelving',
        namespace: 'daisysp',
        header: 'filters/lowshelving.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetGain', signature: 'void SetGain(const float &gain)', paramRanges: '-20.0 to +20.0 dB', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'PeakFilter': {
        className: 'PeakFilter',
        namespace: 'daisysp',
        header: 'filters/peakfilter.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '20.0 - 20000.0 Hz', callRate: 'control' },
            { name: 'SetBandwidth', signature: 'void SetBandwidth(const float &bw)', paramRanges: '10.0 - 10000.0 Hz', callRate: 'control' },
            { name: 'SetGain', signature: 'void SetGain(const float &gain)', paramRanges: '-20.0 to +20.0 dB', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'WahWah': {
        className: 'WahWah',
        namespace: 'daisysp',
        header: 'effects/wahwah.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '200.0 - 2000.0 Hz', callRate: 'control' },
            {
                name: 'SetRes', signature: 'void SetRes(const float &res)', paramRanges: '1.0 - 20.0', callRate: 'control',
                notes: '⚠ SetRes (not SetQ) — renamed from DAFX original to match DaisySP convention'
            },
            { name: 'SetDepth', signature: 'void SetDepth(const float &depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'Vibrato': {
        className: 'Vibrato',
        namespace: 'daisysp',
        header: 'modulation/vibrato.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '0.1 - 20.0 Hz', callRate: 'control' },
            {
                name: 'SetWidth', signature: 'void SetWidth(const float &width)', paramRanges: '0.0001 - 0.1 s', callRate: 'control',
                notes: 'Modulation depth in seconds (delay variation amount)'
            },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'RingModulator': {
        className: 'RingModulator',
        namespace: 'daisysp',
        header: 'modulation/ringmod.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetFreq', signature: 'void SetFreq(const float &freq)', paramRanges: '1.0 - 10000.0 Hz', callRate: 'control' },
            { name: 'SetDepth', signature: 'void SetDepth(const float &depth)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'ToneStack': {
        className: 'ToneStack',
        namespace: 'daisysp',
        header: 'effects/tonestack.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetBass', signature: 'void SetBass(const float &bass)', paramRanges: '-1.0 - +1.0', callRate: 'control' },
            { name: 'SetMiddle', signature: 'void SetMiddle(const float &middle)', paramRanges: '-1.0 - +1.0', callRate: 'control' },
            { name: 'SetTreble', signature: 'void SetTreble(const float &treble)', paramRanges: '-1.0 - +1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'Tube': {
        className: 'Tube',
        namespace: 'daisysp',
        header: 'effects/tube.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetDrive', signature: 'void SetDrive(const float &drive)', callRate: 'control' },
            { name: 'SetBias', signature: 'void SetBias(const float &bias)', callRate: 'control' },
            { name: 'SetDistortion', signature: 'void SetDistortion(const float &dist)', callRate: 'control' },
            { name: 'SetHighPassPole', signature: 'void SetHighPassPole(const float &rh)', callRate: 'control' },
            { name: 'SetLowPassPole', signature: 'void SetLowPassPole(const float &rl)', callRate: 'control' },
            { name: 'SetMix', signature: 'void SetMix(const float &mix)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'NoiseGate': {
        className: 'NoiseGate',
        namespace: 'daisysp',
        header: 'dynamics/noisegate.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetThreshold', signature: 'void SetThreshold(const float &thresh_db)', paramRanges: '-60.0 - 0.0 dB', callRate: 'control' },
            { name: 'SetHoldTime', signature: 'void SetHoldTime(const float &hold_time)', paramRanges: '0.001 - 1.0 s', callRate: 'control' },
            { name: 'SetAttackTime', signature: 'void SetAttackTime(const float &attack_time)', paramRanges: '0.0001 - 0.1 s', callRate: 'control' },
            { name: 'SetReleaseTime', signature: 'void SetReleaseTime(const float &release_time)', paramRanges: '0.001 - 1.0 s', callRate: 'control' },
            {
                name: 'SetAlpha', signature: 'void SetAlpha(const float &alpha)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: 'Envelope detection filter coefficient (higher = smoother)'
            },
            { name: 'Process', signature: 'float Process(const float &in)', callRate: 'audio' },
        ]
    },

    'CompressorExpander': {
        className: 'CompressorExpander',
        namespace: 'daisysp',
        header: 'dynamics/compressor_expander.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init',
                notes: 'Template: CompressorExpander<N> (MaxDelay samples, default 256). Alias: CompExp'
            },
            { name: 'SetCompThreshold', signature: 'void SetCompThreshold(float threshold_db)', paramRanges: 'negative dB', callRate: 'control' },
            { name: 'SetCompRatio', signature: 'void SetCompRatio(float ratio)', paramRanges: '1.0+ (e.g. 4.0 = 4:1)', callRate: 'control' },
            {
                name: 'SetCompSlope', signature: 'void SetCompSlope(float slope)', paramRanges: '0.0 - 1.0', callRate: 'control',
                notes: 'Slope = 1 - 1/ratio. Use SetCompRatio for easier control.'
            },
            { name: 'SetExpThreshold', signature: 'void SetExpThreshold(float threshold_db)', paramRanges: 'negative dB', callRate: 'control' },
            { name: 'SetExpRatio', signature: 'void SetExpRatio(float ratio)', paramRanges: '1.0+', callRate: 'control' },
            { name: 'SetAttackTime', signature: 'void SetAttackTime(float time_sec)', paramRanges: '0.001 - 1.0 s', callRate: 'control' },
            { name: 'SetReleaseTime', signature: 'void SetReleaseTime(float time_sec)', paramRanges: '0.001 - 5.0 s', callRate: 'control' },
            { name: 'SetLookahead', signature: 'void SetLookahead(size_t samples)', callRate: 'control' },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
            { name: 'GetCurrentGainDb', signature: 'float GetCurrentGainDb()', callRate: 'audio' },
        ]
    },

    'EnvelopeFollower': {
        className: 'EnvelopeFollower',
        namespace: 'daisysp',
        header: 'utility/envelopefollower.h',
        methods: [
            { name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init' },
            { name: 'SetAttackTime', signature: 'void SetAttackTime(float attack_time)', paramRanges: '0.0001 - 1.0 s', callRate: 'control' },
            { name: 'SetReleaseTime', signature: 'void SetReleaseTime(float release_time)', paramRanges: '0.001 - 5.0 s', callRate: 'control' },
            {
                name: 'SetMode', signature: 'void SetMode(EnvelopeMode mode)', callRate: 'control',
                notes: 'EnvelopeMode::Peak (fast) | EnvelopeMode::RMS (smoother, power-based)'
            },
            { name: 'Process', signature: 'float Process(float in)', paramRanges: 'returns 0.0 - 1.0+', callRate: 'audio' },
            {
                name: 'ProcessDB', signature: 'float ProcessDB(float in)', callRate: 'audio',
                notes: 'Returns envelope in dB scale'
            },
            { name: 'Reset', signature: 'void Reset()', callRate: 'control' },
        ]
    },

    'FDNReverb': {
        className: 'FDNReverb',
        namespace: 'daisysp',
        header: 'effects/fdn_reverb.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init',
                notes: 'Template: FDNReverb<N> (MaxDelay samples). Aliases: FDNReverb4K, FDNReverb8K, FDNReverb16K'
            },
            { name: 'SetDecay', signature: 'void SetDecay(float decay)', paramRanges: '0.9 - 0.999', callRate: 'control' },
            { name: 'SetMix', signature: 'void SetMix(float mix)', paramRanges: '0.0 - 1.0', callRate: 'control' },
            { name: 'SetDamping', signature: 'void SetDamping(float damping)', paramRanges: '0.0 - 0.99', callRate: 'control' },
            {
                name: 'SetDelayScale', signature: 'void SetDelayScale(float scale)', paramRanges: '0.1 - 4.0', callRate: 'control',
                notes: 'Scales all delay line lengths (room size)'
            },
            {
                name: 'SetReverbTime', signature: 'void SetReverbTime(float rt60)', callRate: 'control',
                notes: 'Converts RT60 seconds to decay coefficient automatically'
            },
            { name: 'Process', signature: 'float Process(float in)', callRate: 'audio' },
            { name: 'ProcessStereo', signature: 'void ProcessStereo(float in_l, float in_r, float *out_l, float *out_r)', callRate: 'audio' },
            {
                name: 'Clear', signature: 'void Clear()', callRate: 'control',
                notes: 'Clears all delay lines (removes reverb tail)'
            },
        ]
    },


};

// ─────────────────────────────────────────────
// CHUNK C — DaisySP Utility Classes + DAFX Analysis
// ─────────────────────────────────────────────

const DAISY_API_INDEX_C: Record<string, DaisyAPIRef> = {

    'SampleHold': {
        className: 'SampleHold',
        namespace: 'daisysp',
        header: 'samplehold.h',
        methods: [
            {
                name: 'Process',
                signature: 'float Process(bool trigger, float input, SampleHold::Mode mode = MODE_SAMPLE_HOLD)',
                callRate: 'audio',
                notes: 'No Init() needed. mode: MODE_SAMPLE_HOLD (latch on rising edge) | MODE_TRACK_HOLD (follow while trigger=true)'
            },
        ]
    },

    'DelayLine': {
        className: 'DelayLine',
        namespace: 'daisysp',
        header: 'delayline.h',
        methods: [
            {
                name: 'Init', signature: 'void Init()', callRate: 'init',
                notes: 'Template: DelayLine<float, MAX_SAMPLES> del; Declare at file scope or DSY_SDRAM_BSS for large delays.'
            },
            {
                name: 'SetDelay', signature: 'void SetDelay(size_t delay)', callRate: 'control',
                notes: 'Delay in samples. Float overload also available: SetDelay(float) enables linear interpolation.'
            },
            { name: 'Write', signature: 'void Write(const T sample)', callRate: 'audio' },
            {
                name: 'Read', signature: 'const T Read() const', callRate: 'audio',
                notes: 'Returns current delay tap with interpolation if float delay was set. Also: Read(float delay) for custom tap.'
            },
            {
                name: 'ReadHermite', signature: 'const T ReadHermite(float delay) const', callRate: 'audio',
                notes: 'Higher-quality hermite interpolation read at arbitrary tap position'
            },
            { name: 'Reset', signature: 'void Reset()', callRate: 'init' },
        ]
    },

    'Looper': {
        className: 'Looper',
        namespace: 'daisysp',
        header: 'looper.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float *mem, size_t size)', callRate: 'init',
                notes: 'mem must be a persistent float buffer (static or DSY_SDRAM_BSS). size = number of samples.'
            },
            {
                name: 'TrigRecord', signature: 'void TrigRecord()', callRate: 'control',
                notes: '1st call: start recording. 2nd call: stop and begin playback. 3rd call: overdub (mode-dependent).'
            },
            {
                name: 'SetMode', signature: 'void SetMode(Looper::Mode mode)', callRate: 'control',
                notes: 'Mode::NORMAL | Mode::ONETIME_DUB | Mode::REPLACE | Mode::FRIPPERTRONICS'
            },
            { name: 'SetReverse', signature: 'void SetReverse(bool state)', callRate: 'control' },
            { name: 'SetHalfSpeed', signature: 'void SetHalfSpeed(bool state)', callRate: 'control' },
            {
                name: 'Clear', signature: 'void Clear()', callRate: 'control',
                notes: 'Effectively erases loop (does not zero buffer, just resets state to EMPTY)'
            },
            { name: 'Recording', signature: 'bool Recording() const', callRate: 'audio' },
            { name: 'Process', signature: 'float Process(const float input)', callRate: 'audio' },
        ]
    },

    'YinPitchDetector': {
        className: 'YinPitchDetector',
        namespace: 'daisysp',
        header: 'analysis/yin.h',
        methods: [
            {
                name: 'Init', signature: 'void Init(float sample_rate)', callRate: 'init',
                notes: 'Template: YinPitchDetector<N> where N = window length (default 1024). Aliases: Yin1024, Yin2048. ~10-15% CPU at 48kHz.'
            },
            {
                name: 'SetTolerance', signature: 'void SetTolerance(float tolerance)', paramRanges: '0.1 - 0.5', callRate: 'control',
                notes: 'Default 0.15. Lower = fewer false positives, less sensitive.'
            },
            { name: 'SetFrequencyRange', signature: 'void SetFrequencyRange(float f0_min, float f0_max)', callRate: 'control' },
            {
                name: 'SetHopSize', signature: 'void SetHopSize(size_t hop_size)', callRate: 'control',
                notes: 'Samples between analyses. Default: YinLen/2 (~21ms latency at 48kHz with N=1024)'
            },
            {
                name: 'ProcessSample', signature: 'bool ProcessSample(float sample)', callRate: 'audio',
                notes: 'Streaming per-sample mode. Returns true when a new pitch estimate is ready. Check GetFrequency() after.'
            },
            {
                name: 'Process', signature: 'float Process(const float *input)', callRate: 'audio',
                notes: 'Block mode. Analyzes provided buffer. Returns detected Hz, or 0.0 if unvoiced.'
            },
            { name: 'GetFrequency', signature: 'float GetFrequency() const', callRate: 'control' },
            {
                name: 'GetConfidence', signature: 'float GetConfidence() const', callRate: 'control',
                notes: 'Returns 0-1. Higher = more confident detection.'
            },
            { name: 'IsVoiced', signature: 'bool IsVoiced() const', callRate: 'control' },
            {
                name: 'GetMidiNote', signature: 'float GetMidiNote() const', callRate: 'control',
                notes: 'Returns fractional MIDI note, or -1 if unvoiced. Formula: 69 + 12*log2(f/440)'
            },
            {
                name: 'GetCentsDeviation', signature: 'float GetCentsDeviation() const', callRate: 'control',
                notes: 'Cents deviation from nearest semitone (-50 to +50)'
            },
            {
                name: 'GetResult', signature: 'const YinResult& GetResult() const', callRate: 'control',
                notes: 'Full result struct with .frequency, .period, .confidence, .voiced fields'
            },
        ]
    },

};

// Merge Chunk C into the main index
Object.assign(DAISY_API_INDEX, DAISY_API_INDEX_C);

/**
 * Helper: get all class names currently in the index.
 * Useful for tooling and completeness checks.
 */
export function getIndexedClasses(): string[] {
    return Object.keys(DAISY_API_INDEX).sort();
}

/**
 * Helper: check if a custom (possibly namespaced) class name is in the index.
 * Strips 'daisysp::' prefix if present.
 */
export function lookupClass(rawClassName: string): DaisyAPIRef | undefined {
    const bare = rawClassName.startsWith('daisysp::')
        ? rawClassName.slice('daisysp::'.length)
        : rawClassName;
    return DAISY_API_INDEX[bare];
}
