/** @brief DaisyPedal noise gate example. */
#include "daisy_pedal.h"
#include "daisysp.h"
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyPedal  hw;
NoiseGate   gate;
BypassFader mix_fader;

Parameter threshold_ctrl, hysteresis_ctrl, attack_ctrl, release_ctrl, range_ctrl, level_ctrl;

volatile float gate_threshold  = 0.03f;
volatile float gate_hysteresis = 0.01f;
volatile float gate_attack     = 0.002f;
volatile float gate_release    = 0.080f;
volatile float gate_range      = 0.0f;
volatile float output_level    = 1.0f;
volatile float meter_env       = 0.0f;
volatile float meter_gain      = 0.0f;
volatile bool  bypass_enabled  = false;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    const float threshold  = gate_threshold;
    const float hysteresis = gate_hysteresis;
    const float attack     = gate_attack;
    const float release    = gate_release;
    const float range      = gate_range;
    const float level      = output_level;
    const bool  bypass     = bypass_enabled;

    gate.SetThreshold(threshold);
    gate.SetHysteresis(hysteresis);
    gate.SetAttack(attack);
    gate.SetRelease(release);
    gate.SetRange(range);
    mix_fader.SetBypass(bypass);

    for(size_t i = 0; i < size; i++)
    {
        const float detector = 0.5f * (in[0][i] + in[1][i]);
        gate.Process(detector);
        mix_fader.Advance();
        const float dry_mix = mix_fader.GetDryMix();
        const float wet_mix = mix_fader.GetFadePosition();
        const float wet_l = in[0][i] * gate.GetGain() * level;
        const float wet_r = in[1][i] * gate.GetGain() * level;
        out[0][i]         = (in[0][i] * dry_mix) + (wet_l * wet_mix);
        out[1][i]         = (in[1][i] * dry_mix) + (wet_r * wet_mix);
    }

    meter_env  = gate.GetEnvelope();
    meter_gain = gate.GetGain();
}

static void UpdateDisplay()
{
    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];

    snprintf(line1, sizeof(line1), "Pedal Noise Gate");
    snprintf(line2, sizeof(line2), "Thresh:%3d", (int)(gate_threshold * 1000.0f));
    snprintf(line3, sizeof(line3), "Env:%3d Gain:%3d", (int)(meter_env * 1000.0f), (int)(meter_gain * 1000.0f));
    snprintf(line4, sizeof(line4), "Bypass:%s", bypass_enabled ? "On" : "Off");

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

    gate.Init(hw.AudioSampleRate());
    mix_fader.Init(hw.AudioSampleRate());
    mix_fader.SetFadeTime(0.01f);

    threshold_ctrl.Init(hw.knob[DaisyPedal::KNOB_1], 0.005f, 0.250f, Parameter::LOGARITHMIC);
    hysteresis_ctrl.Init(hw.knob[DaisyPedal::KNOB_2], 0.0f, 0.100f, Parameter::LINEAR);
    attack_ctrl.Init(hw.knob[DaisyPedal::KNOB_3], 0.0005f, 0.020f, Parameter::LOGARITHMIC);
    release_ctrl.Init(hw.knob[DaisyPedal::KNOB_4], 0.010f, 0.500f, Parameter::LOGARITHMIC);
    range_ctrl.Init(hw.knob[DaisyPedal::KNOB_5], 0.0f, 0.500f, Parameter::LINEAR);
    level_ctrl.Init(hw.knob[DaisyPedal::KNOB_6], 0.5f, 2.0f, Parameter::LINEAR);

    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();
        if(hw.switches[DaisyPedal::SW_BYPASS].RisingEdge())
            bypass_enabled = !bypass_enabled;

        gate_threshold  = threshold_ctrl.Process();
        gate_hysteresis = hysteresis_ctrl.Process();
        gate_attack     = attack_ctrl.Process();
        gate_release    = release_ctrl.Process();
        gate_range      = range_ctrl.Process();
        output_level    = level_ctrl.Process();

        hw.leds[DaisyPedal::LED_1].Set(bypass_enabled ? 0.15f : (meter_gain > 1.0f ? 1.0f : meter_gain));
        hw.leds[DaisyPedal::LED_2].Set(meter_env > 0.5f ? 1.0f : meter_env * 2.0f);
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
