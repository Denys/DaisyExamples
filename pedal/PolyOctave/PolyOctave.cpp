/** @brief DaisyPedal poly octave example. */
#include "daisy_pedal.h"
#include "daisysp.h"
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyPedal      hw;
Multirate       conditioner;
OctaveGenerator octave;
BypassFader     mix_fader;

volatile float  sub_gain      = 1.0f;
volatile float  up_gain       = 0.0f;
volatile float  input_drive   = 2.0f;
volatile float  tone_amount   = 0.5f;
volatile size_t rate_ratio    = 2;
volatile float  output_gain   = 1.0f;
volatile bool   bypass_active = false;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    const float  sub   = sub_gain;
    const float  up    = up_gain;
    const float  drive = input_drive;
    const float  tone  = tone_amount;
    const size_t ratio = rate_ratio;
    const float  gain  = output_gain;
    const bool   bypass = bypass_active;

    conditioner.SetRatio(ratio);
    conditioner.SetCutoff(250.0f + (tone * 3500.0f));
    octave.SetSubGain(sub);
    octave.SetUpGain(up);
    octave.SetDrive(drive);
    octave.SetTone(tone);
    mix_fader.SetBypass(bypass);

    for(size_t i = 0; i < size; i++)
    {
        const float mono_in      = 0.5f * (in[0][i] + in[1][i]);
        const float conditioned  = conditioner.Process(mono_in);
        const float octave_voice = octave.Process(conditioned);
        const float mixed        = gain * mix_fader.Process(mono_in, octave_voice);
        out[0][i]                = mixed;
        out[1][i]                = mixed;
    }
}

static void UpdateDisplay()
{
    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];

    snprintf(line1, sizeof(line1), "Pedal PolyOctave");
    snprintf(line2, sizeof(line2), "Sub:%3d Up:%3d", (int)(sub_gain * 100.0f), (int)(up_gain * 100.0f));
    snprintf(line3, sizeof(line3), "Drive:%2d Ratio:%1d", (int)(input_drive * 10.0f), (int)rate_ratio);
    snprintf(line4, sizeof(line4), "Bypass:%s", bypass_active ? "On" : "Off");

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line1, Font_7x10, true);
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(line2, Font_7x10, true);
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line3, Font_7x10, true);
    hw.display.SetCursor(0, 48);
    hw.display.WriteString(line4, Font_7x10, true);
    hw.display.Update();
}

int main(void)
{
    uint32_t last_display_update = 0;

    hw.Init();
    hw.StartAdc();

    conditioner.Init(hw.AudioSampleRate(), 2);
    octave.Init(hw.AudioSampleRate());
    mix_fader.Init(hw.AudioSampleRate());
    mix_fader.SetFadeTime(0.02f);
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();
        if(hw.switches[DaisyPedal::SW_BYPASS].RisingEdge())
            bypass_active = !bypass_active;

        sub_gain    = hw.GetKnobValue(DaisyPedal::KNOB_1);
        up_gain     = hw.GetKnobValue(DaisyPedal::KNOB_2);
        input_drive = 0.5f + (7.5f * hw.GetKnobValue(DaisyPedal::KNOB_3));
        tone_amount = hw.GetKnobValue(DaisyPedal::KNOB_4);
        rate_ratio  = 1 + static_cast<size_t>(hw.GetKnobValue(DaisyPedal::KNOB_5) * 7.0f);
        output_gain = 0.5f + (1.5f * hw.GetKnobValue(DaisyPedal::KNOB_6));

        hw.leds[DaisyPedal::LED_1].Set(bypass_active ? 0.1f : sub_gain);
        hw.leds[DaisyPedal::LED_2].Set(up_gain);
        hw.UpdateLeds();

        const uint32_t now = hw.seed.system.GetNow();
        if(now - last_display_update >= 50)
        {
            UpdateDisplay();
            last_display_update = now;
        }

        hw.DelayMs(5);
    }
}
