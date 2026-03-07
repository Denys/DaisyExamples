#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

Oscillator vco;
Oscillator lfo;
WhiteNoise noise;
Svf        vcf;
Adsr       vca_env;
AdEnv      ar_env;

bool  gate              = false;
bool  hold_gate         = false;
bool  ar_to_pitch       = true;
bool  ar_trigger_pending = false;
int   active_key        = -1;
int   waveform_index    = 0;
float note_hz           = 110.0f;
float env_depth         = 0.45f;
float lfo_to_pitch_amt  = 0.0f;
float lfo_to_filter_amt = 0.0f;

const float kChromaticNotes[8] = {48.0f, 50.0f, 52.0f, 53.0f, 55.0f, 57.0f, 59.0f, 60.0f};
const char* kWaveNames[3]      = {"SAW", "SQR", "TRI"};

void SetWaveform(int idx)
{
    waveform_index = idx % 3;
    if(waveform_index < 0)
        waveform_index += 3;

    switch(waveform_index)
    {
        case 0: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW); break;
        case 1: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE); break;
        case 2: vco.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI); break;
        default: break;
    }
}

void TriggerKey(int key_index)
{
    active_key = key_index;
    note_hz    = mtof(kChromaticNotes[key_index]);
    gate       = true;
    ar_trigger_pending = true;
}

void UpdateOled()
{
    char line[48];
    hw.display.Fill(false);

    snprintf(line, sizeof(line), "MFOS NOISE TOASTER");
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "Wave:%s Hold:%s", kWaveNames[waveform_index], hold_gate ? "ON" : "OFF");
    hw.display.SetCursor(0, 14);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "K1 Tune K2 Mix K3 Cut");
    hw.display.SetCursor(0, 28);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "A1-A8 Notes B1-3 Wave");
    hw.display.SetCursor(0, 42);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line, sizeof(line), "B4 Hold SW2 Panic");
    hw.display.SetCursor(0, 54);
    hw.display.WriteString(line, Font_6x8, true);

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAllControls();

    for(int i = 0; i < 8; i++)
    {
        if(hw.KeyboardRisingEdge(i))
        {
            TriggerKey(i);
        }

        if(hw.KeyboardFallingEdge(i) && i == active_key && !hold_gate)
        {
            gate = false;
        }
    }

    if(hw.KeyboardRisingEdge(8))
        SetWaveform(0);
    if(hw.KeyboardRisingEdge(9))
        SetWaveform(1);
    if(hw.KeyboardRisingEdge(10))
        SetWaveform(2);

    if(hw.KeyboardRisingEdge(11))
    {
        hold_gate = !hold_gate;
        if(hold_gate && !gate)
        {
            gate = true;
            ar_trigger_pending = true;
        }
    }

    if(hw.KeyboardRisingEdge(12))
        ar_to_pitch = !ar_to_pitch;

    if(hw.sw[0].RisingEdge())
    {
        hold_gate = !hold_gate;
        if(hold_gate && !gate)
        {
            gate = true;
            ar_trigger_pending = true;
        }
    }

    if(hw.sw[1].RisingEdge())
    {
        gate      = false;
        hold_gate = false;
    }
}

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    (void)in;

    const float coarse_tune = hw.knob[0].Value();
    const float noise_mix   = hw.knob[1].Value();
    const float cutoff_knob = hw.knob[2].Value();
    const float res_knob    = hw.knob[3].Value();
    const float env_amt     = hw.knob[4].Value();
    const float lfo_rate    = hw.knob[5].Value();
    const float lfo_depth   = hw.knob[6].Value();
    const float output      = hw.knob[7].Value();

    env_depth         = env_amt;
    lfo_to_pitch_amt  = 0.3f * lfo_depth;
    lfo_to_filter_amt = 0.85f * lfo_depth;

    lfo.SetFreq(0.05f + 20.0f * lfo_rate * lfo_rate);

    for(size_t i = 0; i < size; i++)
    {
        // AdEnv is trigger-based in the installed DaisySP, not gate-driven.
        if(ar_trigger_pending)
        {
            ar_env.Trigger();
            ar_trigger_pending = false;
        }

        const float lfo_sig = lfo.Process();
        const float ar      = ar_env.Process();
        const float env     = vca_env.Process(gate || hold_gate);

        const float pitch_env = ar_to_pitch ? ar * env_depth : 0.0f;
        const float pitch_lfo = lfo_sig * lfo_to_pitch_amt;

        float tuned_note = note_hz;
        tuned_note *= std::pow(2.0f, (coarse_tune - 0.5f) * 2.0f);
        tuned_note *= std::pow(2.0f, pitch_env + pitch_lfo);
        tuned_note = fclamp(tuned_note, 20.0f, 8000.0f);

        vco.SetFreq(tuned_note);

        const float osc_sample   = vco.Process();
        const float noise_sample = noise.Process();
        const float mix_sample   = (1.0f - noise_mix) * osc_sample + noise_mix * noise_sample;

        const float cutoff_env = ar_to_pitch ? 0.0f : (ar * env_depth * 0.9f);
        const float cutoff_lfo = 0.45f * (lfo_sig + 1.0f) * lfo_to_filter_amt;
        const float cutoff     = 40.0f + (cutoff_knob + cutoff_env + cutoff_lfo) * 9000.0f;

        vcf.SetFreq(fclamp(cutoff, 40.0f, 12000.0f));
        vcf.SetRes(0.05f + 0.9f * res_knob);
        vcf.Process(mix_sample);

        float sig = vcf.Low() * env;
        sig *= (0.05f + 0.95f * output);
        sig = fclamp(sig, -0.95f, 0.95f);

        out[0][i] = sig;
        out[1][i] = sig;
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);

    const float sr = hw.AudioSampleRate();

    vco.Init(sr);
    vco.SetAmp(0.75f);
    SetWaveform(0);

    lfo.Init(sr);
    lfo.SetWaveform(Oscillator::WAVE_SIN);
    lfo.SetAmp(1.0f);
    lfo.SetFreq(2.0f);

    noise.Init();

    vcf.Init(sr);
    vcf.SetDrive(0.3f);
    vcf.SetRes(0.3f);

    vca_env.Init(sr);
    vca_env.SetTime(ADSR_SEG_ATTACK, 0.002f);
    vca_env.SetTime(ADSR_SEG_DECAY, 0.12f);
    vca_env.SetSustainLevel(0.75f);
    vca_env.SetTime(ADSR_SEG_RELEASE, 0.22f);

    ar_env.Init(sr);
    ar_env.SetMin(0.0f);
    ar_env.SetMax(1.0f);
    ar_env.SetCurve(-18.0f);
    ar_env.SetTime(ADENV_SEG_ATTACK, 0.005f);
    ar_env.SetTime(ADENV_SEG_DECAY, 0.28f);

    hw.seed.StartLog(true);
    hw.seed.PrintLine("Field_MFOS_NoiseToaster ready");
    hw.seed.PrintLine("A1-A8 notes | B1/B2/B3 waveform | B4 hold | B5 AR target");

    UpdateOled();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();
        UpdateOled();
        System::Delay(4);
    }
}
