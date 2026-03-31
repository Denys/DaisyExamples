#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// -----------------------------------------------------------------------------
// Synth engine
// -----------------------------------------------------------------------------
Oscillator osc1, osc2, osc3, lfo;
Oscillator noiseosc;
Svf        filter;
Adsr       amp_env, filt_env;

constexpr size_t kNumKnobs        = 8;
constexpr size_t kNumModes        = 3;
constexpr size_t kNumParams       = 24;
constexpr size_t kNumKeyboardLeds = 16;

enum Mode
{
    MODE_DEFAULT = 0,
    MODE_SW1,
    MODE_SW2,
};

Mode current_mode = MODE_DEFAULT;

enum ParamId
{
    P_OSC12_MIX = 0,
    P_OSC3_LEVEL,
    P_OSC2_DETUNE,
    P_OSC3_DETUNE,
    P_FILTER_CUTOFF,
    P_FILTER_RESONANCE,
    P_AMP_ATTACK,
    P_AMP_RELEASE,

    P_FILTER_ENV_AMT,
    P_FILTER_DECAY,
    P_FILTER_SUSTAIN,
    P_DRIVE,
    P_LFO_RATE,
    P_LFO_PITCH_DEPTH,
    P_LFO_FILTER_DEPTH,
    P_GLIDE,

    P_AMP_DECAY,
    P_AMP_SUSTAIN,
    P_NOISE_LEVEL,
    P_SUB_LEVEL,
    P_PAN_SPREAD,
    P_VELOCITY_AMT,
    P_LFO_AMP_DEPTH,
    P_MASTER_VOL,
};

float params[kNumParams] = {
    0.5f, 0.45f, 0.5f, 0.5f, 0.45f, 0.2f, 0.05f, 0.35f, // default
    0.5f, 0.35f, 0.3f, 0.15f, 0.18f, 0.0f, 0.2f, 0.0f,  // sw1
    0.35f, 0.75f, 0.0f, 0.15f, 0.5f, 0.4f, 0.0f, 0.65f  // sw2
};

const ParamId mode_param_map[kNumModes][kNumKnobs] = {
    {P_OSC12_MIX,
     P_OSC3_LEVEL,
     P_OSC2_DETUNE,
     P_OSC3_DETUNE,
     P_FILTER_CUTOFF,
     P_FILTER_RESONANCE,
     P_AMP_ATTACK,
     P_AMP_RELEASE},
    {P_FILTER_ENV_AMT,
     P_FILTER_DECAY,
     P_FILTER_SUSTAIN,
     P_DRIVE,
     P_LFO_RATE,
     P_LFO_PITCH_DEPTH,
     P_LFO_FILTER_DEPTH,
     P_GLIDE},
    {P_AMP_DECAY,
     P_AMP_SUSTAIN,
     P_NOISE_LEVEL,
     P_SUB_LEVEL,
     P_PAN_SPREAD,
     P_VELOCITY_AMT,
     P_LFO_AMP_DEPTH,
     P_MASTER_VOL},
};

const char* mode_names[kNumModes] = {"DEFAULT", "SW1", "SW2"};

const char* param_names[kNumParams] = {
    "OSC1<->OSC2 MIX", "OSC3 LEVEL",   "OSC2 DETUNE",   "OSC3 DETUNE",
    "FILTER CUTOFF",   "FILTER RES",   "AMP ATTACK",    "AMP RELEASE",
    "FILT ENV AMT",    "FILT DECAY",   "FILT SUSTAIN",  "DRIVE",
    "LFO RATE",        "LFO->PITCH",   "LFO->FILTER",   "GLIDE",
    "AMP DECAY",       "AMP SUSTAIN",  "NOISE LEVEL",   "SUB LEVEL",
    "PAN SPREAD",      "VELOCITY AMT", "LFO->AMP",      "MASTER VOL",
};

float previous_knobs[kNumKnobs] = {0.f};
const float kKnobChangeThreshold = 0.008f;

int      active_param      = -1;
uint32_t active_param_time = 0;

uint32_t now_ms = 0;

// -----------------------------------------------------------------------------
// MIDI state (external keyboard/sequencer)
// -----------------------------------------------------------------------------
bool  note_held[128]      = {false};
int   current_note        = 60;
float current_velocity    = 0.8f;
bool  gate                = false;
int   midi_channel        = 0; // 0 == OMNI

float smoothed_freq = 261.63f;

// -----------------------------------------------------------------------------
// LED key function selectors (tri-state: off/blink/on)
// -----------------------------------------------------------------------------
enum LfoWave
{
    LFO_SINE = 0,
    LFO_TRI,
    LFO_SQUARE,
};
LfoWave lfo_wave = LFO_SINE;

enum OscWave
{
    OSC_SAW = 0,
    OSC_TRI,
    OSC_SQUARE,
    OSC_POLYBLEP_SAW,
};
OscWave osc_wave = OSC_SAW;

enum FilterMode
{
    FILT_LOW = 0,
    FILT_BAND,
    FILT_HIGH,
};
FilterMode filter_mode = FILT_LOW;

int keytrack_mode   = 2; // 0, 33, 66, 100%
int transpose_index = 1; // -12, 0, +12, +24
int lfo_target_mode = 1; // pitch/filter/amp/all
bool hard_sync      = false;
bool legato_mode    = false;

const int transpose_values[4] = {-12, 0, 12, 24};

float MidiToHz(float note)
{
    return 440.f * powf(2.f, (note - 69.f) / 12.f);
}

void RecomputeCurrentNote()
{
    for(int n = 127; n >= 0; --n)
    {
        if(note_held[n])
        {
            current_note = n;
            gate         = true;
            return;
        }
    }
    gate = false;
}

void HandleMidiMessage(MidiEvent m)
{
    if(midi_channel > 0 && m.channel != static_cast<uint8_t>(midi_channel - 1))
        return;

    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent note_on = m.AsNoteOn();
            if(note_on.velocity == 0)
            {
                note_held[note_on.note] = false;
                RecomputeCurrentNote();
            }
            else
            {
                note_held[note_on.note] = true;
                current_note            = note_on.note;
                current_velocity = static_cast<float>(note_on.velocity) / 127.f;
                gate             = true;
            }
        }
        break;

        case NoteOff:
        {
            NoteOffEvent note_off = m.AsNoteOff();
            note_held[note_off.note] = false;
            RecomputeCurrentNote();
        }
        break;

        case PitchBend:
        {
            auto bend = m.AsPitchBend();
            // reserved for future extension
            (void)bend;
        }
        break;

        default: break;
    }
}

void HandleLedKeyFunctions()
{
    // Keyboard index mapping: 0-7 => A1..A8, 8-15 => B1..B8
    for(size_t i = 0; i < kNumKeyboardLeds; ++i)
    {
        if(!hw.KeyboardRisingEdge(i))
            continue;

        if(current_mode == MODE_DEFAULT)
        {
            if(i <= 2)
            {
                lfo_wave = static_cast<LfoWave>(i);
            }
            else if(i >= 3 && i <= 6)
            {
                osc_wave = static_cast<OscWave>(i - 3);
            }
            else if(i == 7)
            {
                hard_sync = !hard_sync;
            }
            else if(i == 8)
            {
                legato_mode = !legato_mode;
            }
        }
        else if(current_mode == MODE_SW1)
        {
            if(i >= 0 && i <= 3)
            {
                keytrack_mode = static_cast<int>(i);
            }
            else if(i >= 4 && i <= 6)
            {
                filter_mode = static_cast<FilterMode>(i - 4);
            }
            else if(i >= 8 && i <= 11)
            {
                midi_channel = static_cast<int>(i - 8) + 1;
            }
            else if(i == 12)
            {
                midi_channel = 0; // omni
            }
        }
        else if(current_mode == MODE_SW2)
        {
            if(i >= 0 && i <= 3)
            {
                transpose_index = static_cast<int>(i);
            }
            else if(i >= 4 && i <= 7)
            {
                lfo_target_mode = static_cast<int>(i - 4);
            }
        }
    }
}

void UpdateModeFromSwitches()
{
    if(hw.sw[0].RisingEdge() && hw.sw[1].Pressed())
    {
        current_mode = MODE_DEFAULT;
    }
    else if(hw.sw[0].RisingEdge())
    {
        current_mode = MODE_SW1;
    }
    else if(hw.sw[1].RisingEdge())
    {
        current_mode = MODE_SW2;
    }
}

void UpdateControlBanks()
{
    for(size_t k = 0; k < kNumKnobs; ++k)
    {
        float   raw = hw.knob[k].Process();
        ParamId p   = mode_param_map[current_mode][k];

        params[p] = raw;

        if(fabsf(raw - previous_knobs[k]) > kKnobChangeThreshold)
        {
            active_param      = p;
            active_param_time = now_ms;
            previous_knobs[k] = raw;
        }
    }
}

void UpdateLeds()
{
    const bool blink = ((now_ms / 220U) % 2U) == 0U;

    for(size_t i = 0; i < kNumKeyboardLeds; ++i)
    {
        float v = 0.f; // OFF

        if(current_mode == MODE_DEFAULT)
        {
            if(i <= 2)
                v = (i == static_cast<size_t>(lfo_wave)) ? 1.f : (blink ? 0.35f : 0.f);
            else if(i >= 3 && i <= 6)
                v = (i == static_cast<size_t>(osc_wave + 3)) ? 1.f : (blink ? 0.35f : 0.f);
            else if(i == 7)
                v = hard_sync ? 1.f : (blink ? 0.35f : 0.f);
            else if(i == 8)
                v = legato_mode ? 1.f : (blink ? 0.35f : 0.f);
        }
        else if(current_mode == MODE_SW1)
        {
            if(i <= 3)
                v = (i == static_cast<size_t>(keytrack_mode)) ? 1.f : (blink ? 0.35f : 0.f);
            else if(i >= 4 && i <= 6)
                v = (i == static_cast<size_t>(filter_mode + 4)) ? 1.f : (blink ? 0.35f : 0.f);
            else if(i >= 8 && i <= 11)
                v = ((i - 8 + 1) == static_cast<size_t>(midi_channel)) ? 1.f
                                                                       : (blink ? 0.35f : 0.f);
            else if(i == 12)
                v = (midi_channel == 0) ? 1.f : (blink ? 0.35f : 0.f);
        }
        else if(current_mode == MODE_SW2)
        {
            if(i <= 3)
                v = (i == static_cast<size_t>(transpose_index)) ? 1.f : (blink ? 0.35f : 0.f);
            else if(i >= 4 && i <= 7)
                v = ((i - 4) == static_cast<size_t>(lfo_target_mode)) ? 1.f
                                                                       : (blink ? 0.35f : 0.f);
        }

        hw.led_driver.SetLed(i, v);
    }

    // Ring LEDs reflect 8 active params from current mode
    for(size_t k = 0; k < kNumKnobs; ++k)
    {
        ParamId pid = mode_param_map[current_mode][k];
        hw.led_driver.SetLed(DaisyField::LED_KNOB_1 + k, params[pid]);
    }

    hw.led_driver.SwapBuffersAndTransmit();
}

void DrawModeSummary(char* line, size_t max)
{
    if(current_mode == MODE_DEFAULT)
    {
        const char* lfo_name = lfo_wave == LFO_SINE ? "SIN" : (lfo_wave == LFO_TRI ? "TRI" : "SQR");
        snprintf(line,
                 max,
                 "LFO:%s OSC:%d F:%s",
                 lfo_name,
                 static_cast<int>(osc_wave),
                 filter_mode == FILT_LOW ? "LP" : (filter_mode == FILT_BAND ? "BP" : "HP"));
    }
    else if(current_mode == MODE_SW1)
    {
        snprintf(line,
                 max,
                 "KEYTRACK:%d%% MIDI:%s",
                 keytrack_mode * 33,
                 midi_channel == 0 ? "OMNI" : (midi_channel == 1 ? "CH1" : (midi_channel == 2 ? "CH2" : (midi_channel == 3 ? "CH3" : "CH4"))));
    }
    else
    {
        snprintf(line,
                 max,
                 "TRANS:%+d LFO-TGT:%d",
                 transpose_values[transpose_index],
                 lfo_target_mode);
    }
}

void UpdateDisplay()
{
    hw.display.Fill(false);

    char line[32];

    hw.display.SetCursor(0, 0);
    snprintf(line, sizeof(line), "3OSC SUB SYNTH | %s", mode_names[current_mode]);
    hw.display.WriteString(line, Font_6x8, true);

    DrawModeSummary(line, sizeof(line));
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line, Font_6x8, true);

    bool zoom_active = active_param >= 0 && (now_ms - active_param_time < 1200);

    if(zoom_active)
    {
        hw.display.SetCursor(0, 24);
        hw.display.WriteString(param_names[active_param], Font_7x10, true);

        hw.display.SetCursor(0, 40);
        snprintf(line, sizeof(line), "%.2f", params[active_param]);
        hw.display.WriteString(line, Font_11x18, true);
    }
    else
    {
        for(size_t i = 0; i < 3; ++i)
        {
            ParamId p = mode_param_map[current_mode][i];
            snprintf(line, sizeof(line), "%s: %.2f", param_names[p], params[p]);
            hw.display.SetCursor(0, 24 + i * 12);
            hw.display.WriteString(line, Font_6x8, true);
        }
    }

    hw.display.Update();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    (void)in;
    hw.ProcessAllControls();

    UpdateModeFromSwitches();
    HandleLedKeyFunctions();
    UpdateControlBanks();

    // Continuous params
    const float osc12mix  = params[P_OSC12_MIX];
    const float osc3level = params[P_OSC3_LEVEL];
    const float det2      = (params[P_OSC2_DETUNE] - 0.5f) * 24.f;
    const float det3      = (params[P_OSC3_DETUNE] - 0.5f) * 24.f;

    const float cutoff = 40.f + params[P_FILTER_CUTOFF] * params[P_FILTER_CUTOFF] * 18000.f;
    const float res    = 0.05f + params[P_FILTER_RESONANCE] * 0.93f;

    const float attack      = 0.001f + params[P_AMP_ATTACK] * 2.2f;
    const float release     = 0.005f + params[P_AMP_RELEASE] * 3.0f;
    const float amp_decay   = 0.005f + params[P_AMP_DECAY] * 2.0f;
    const float amp_sustain = params[P_AMP_SUSTAIN];

    const float filt_env_amt = params[P_FILTER_ENV_AMT] * 12000.f;
    const float filt_decay   = 0.005f + params[P_FILTER_DECAY] * 2.0f;
    const float filt_sus     = params[P_FILTER_SUSTAIN];

    const float drive       = 1.f + params[P_DRIVE] * 8.f;
    const float lfo_rate    = 0.05f + params[P_LFO_RATE] * 24.f;
    const float lfo_pitch   = params[P_LFO_PITCH_DEPTH] * 0.35f;
    const float lfo_filter  = params[P_LFO_FILTER_DEPTH] * 6000.f;
    const float lfo_amp     = params[P_LFO_AMP_DEPTH] * 0.7f;
    const float glide_time  = 0.001f + params[P_GLIDE] * 0.25f;
    const float noise_level = params[P_NOISE_LEVEL] * 0.35f;
    const float sub_level   = params[P_SUB_LEVEL] * 0.4f;

    const float velocity_amt = params[P_VELOCITY_AMT];
    const float velocity_mul = (1.f - velocity_amt) + velocity_amt * current_velocity;

    float master = params[P_MASTER_VOL];

    amp_env.SetTime(ADSR_SEG_ATTACK, attack);
    amp_env.SetTime(ADSR_SEG_DECAY, amp_decay);
    amp_env.SetSustainLevel(amp_sustain);
    amp_env.SetTime(ADSR_SEG_RELEASE, release);

    filt_env.SetTime(ADSR_SEG_ATTACK, attack * 0.75f);
    filt_env.SetTime(ADSR_SEG_DECAY, filt_decay);
    filt_env.SetSustainLevel(filt_sus);
    filt_env.SetTime(ADSR_SEG_RELEASE, release);

    lfo.SetFreq(lfo_rate);
    lfo.SetWaveform(lfo_wave == LFO_SINE ? Oscillator::WAVE_SIN
                    : lfo_wave == LFO_TRI ? Oscillator::WAVE_TRI
                                          : Oscillator::WAVE_SQUARE);

    switch(osc_wave)
    {
        case OSC_SAW:
            osc1.SetWaveform(Oscillator::WAVE_SAW);
            osc2.SetWaveform(Oscillator::WAVE_SAW);
            osc3.SetWaveform(Oscillator::WAVE_SAW);
            break;
        case OSC_TRI:
            osc1.SetWaveform(Oscillator::WAVE_TRI);
            osc2.SetWaveform(Oscillator::WAVE_TRI);
            osc3.SetWaveform(Oscillator::WAVE_TRI);
            break;
        case OSC_SQUARE:
            osc1.SetWaveform(Oscillator::WAVE_SQUARE);
            osc2.SetWaveform(Oscillator::WAVE_SQUARE);
            osc3.SetWaveform(Oscillator::WAVE_SQUARE);
            break;
        case OSC_POLYBLEP_SAW:
            osc1.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
            osc2.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
            osc3.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
            break;
    }

    float target_note = static_cast<float>(current_note + transpose_values[transpose_index]);
    float target_freq = MidiToHz(target_note);
    float glide_alpha = 1.f - expf(-1.f / (hw.AudioSampleRate() * glide_time));
    smoothed_freq += glide_alpha * (target_freq - smoothed_freq);

    for(size_t i = 0; i < size; i++)
    {
        const float lfo_value = lfo.Process();

        float pitch_mod = 0.f;
        if(lfo_target_mode == 0 || lfo_target_mode == 3)
            pitch_mod += lfo_value * lfo_pitch;

        const float f1 = smoothed_freq * powf(2.f, pitch_mod);
        const float f2 = MidiToHz(target_note + det2 + pitch_mod * 12.f);
        const float f3 = MidiToHz(target_note + det3 + pitch_mod * 12.f);

        osc1.SetFreq(f1);
        osc2.SetFreq(f2);
        osc3.SetFreq(f3);

        float sig = osc1.Process() * (1.f - osc12mix) + osc2.Process() * osc12mix;
        sig += osc3.Process() * osc3level;
        sig += noiseosc.Process() * noise_level;

        // Optional sub oscillator from osc1 fundamental
        float sub = sinf(TWOPI_F * osc1.GetPhase() * 0.5f);
        sig += sub * sub_level;

        float env_amp = amp_env.Process(gate);
        if(legato_mode && gate)
            env_amp = fmaxf(env_amp, 0.25f);

        float env_filt = filt_env.Process(gate);

        float cutoff_mod = cutoff + env_filt * filt_env_amt;
        if(lfo_target_mode == 1 || lfo_target_mode == 3)
            cutoff_mod += lfo_value * lfo_filter;

        const float keytrack = (keytrack_mode == 0
                                    ? 0.f
                                    : (keytrack_mode == 1 ? 0.33f : (keytrack_mode == 2 ? 0.66f : 1.0f)));
        cutoff_mod += (target_note - 60.f) * keytrack * 120.f;
        cutoff_mod = fclamp(cutoff_mod, 20.f, 20000.f);

        filter.SetRes(res);
        filter.SetFreq(cutoff_mod);
        filter.Process(tanhf(sig * drive));

        float filtered = filter_mode == FILT_LOW   ? filter.Low()
                         : filter_mode == FILT_BAND ? filter.Band()
                                                    : filter.High();

        float amp_lfo = 1.f;
        if(lfo_target_mode == 2 || lfo_target_mode == 3)
            amp_lfo = 1.f - (0.5f + 0.5f * lfo_value) * lfo_amp;

        float out_sig = filtered * env_amp * amp_lfo * velocity_mul * master;

        // Stereo spread via simple polarity skew on OSC3 component
        float spread = (params[P_PAN_SPREAD] - 0.5f) * 0.4f;
        out[0][i]    = out_sig * (1.f - spread);
        out[1][i]    = out_sig * (1.f + spread);
    }
}

int main(void)
{
    hw.Init();

    const float sr = hw.AudioSampleRate();

    osc1.Init(sr);
    osc2.Init(sr);
    osc3.Init(sr);
    lfo.Init(sr);
    noiseosc.Init(sr);

    osc1.SetAmp(0.35f);
    osc2.SetAmp(0.35f);
    osc3.SetAmp(0.25f);
    noiseosc.SetWaveform(Oscillator::WAVE_NOISE);
    noiseosc.SetAmp(1.0f);

    lfo.SetAmp(1.f);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetFreq(2.f);

    filter.Init(sr);
    filter.SetFreq(1200.f);
    filter.SetRes(0.2f);

    amp_env.Init(sr);
    amp_env.SetTime(ADSR_SEG_ATTACK, 0.01f);
    amp_env.SetTime(ADSR_SEG_DECAY, 0.2f);
    amp_env.SetSustainLevel(0.7f);
    amp_env.SetTime(ADSR_SEG_RELEASE, 0.3f);

    filt_env.Init(sr);
    filt_env.SetTime(ADSR_SEG_ATTACK, 0.01f);
    filt_env.SetTime(ADSR_SEG_DECAY, 0.25f);
    filt_env.SetSustainLevel(0.3f);
    filt_env.SetTime(ADSR_SEG_RELEASE, 0.4f);

    hw.midi.StartReceive();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        now_ms = System::GetNow();

        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }

        UpdateLeds();
        UpdateDisplay();
        System::Delay(2);
    }
}
