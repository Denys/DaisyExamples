/** @brief DaisyPedal pitch drop example. */
#include "daisy_pedal.h"
#include "daisysp.h"
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyPedal   hw;
PitchShifter pitch;
BypassFader  mix_fader;

volatile float    semitone_drop  = 12.0f;
volatile uint32_t window_size    = 2048;
volatile float    modulation_fun = 0.0f;
volatile float    dry_level      = 1.0f;
volatile float    wet_level      = 1.0f;
volatile float    output_level   = 1.0f;
volatile bool     latch_enabled  = false;
volatile bool     effect_enabled = false;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    const float    drop     = semitone_drop;
    const uint32_t win_size = window_size;
    const float    fun      = modulation_fun;
    const float    dry      = dry_level;
    const float    wet      = wet_level;
    const float    level    = output_level;
    const bool     engage   = effect_enabled;

    pitch.SetTransposition(-drop);
    pitch.SetDelSize(win_size);
    pitch.SetFun(fun);
    mix_fader.SetBypass(!engage);

    for(size_t i = 0; i < size; i++)
    {
        float mono_in       = 0.5f * (in[0][i] + in[1][i]);
        const float shifted = pitch.Process(mono_in);
        const float mixed   = level * mix_fader.Process(dry * mono_in, wet * shifted);
        out[0][i]           = mixed;
        out[1][i]           = mixed;
    }
}

static void UpdateDisplay()
{
    char line1[32];
    char line2[32];
    char line3[32];
    char line4[32];

    snprintf(line1, sizeof(line1), "Pedal Pitch Drop");
    snprintf(line2, sizeof(line2), "Drop: -%2d st", (int)(semitone_drop + 0.5f));
    snprintf(line3, sizeof(line3), "Window:%4lu", (unsigned long)window_size);
    snprintf(line4,
             sizeof(line4),
             "Latch:%s Hold:%s",
             latch_enabled ? "On" : "Off",
             effect_enabled ? "On" : "Off");

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

    pitch.Init(hw.AudioSampleRate());
    mix_fader.Init(hw.AudioSampleRate());
    mix_fader.SetFadeTime(0.02f);
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();

        if(hw.switches[DaisyPedal::SW_BYPASS].RisingEdge())
            latch_enabled = !latch_enabled;

        effect_enabled = latch_enabled || hw.switches[DaisyPedal::SW_AUX].Pressed();
        semitone_drop  = 1.0f + (24.0f * hw.GetKnobValue(DaisyPedal::KNOB_1));
        window_size    = 512 + static_cast<uint32_t>(hw.GetKnobValue(DaisyPedal::KNOB_2) * 3584.0f);
        modulation_fun = hw.GetKnobValue(DaisyPedal::KNOB_3);
        dry_level      = hw.GetKnobValue(DaisyPedal::KNOB_4);
        wet_level      = hw.GetKnobValue(DaisyPedal::KNOB_5);
        output_level   = 0.5f + (1.5f * hw.GetKnobValue(DaisyPedal::KNOB_6));

        hw.leds[DaisyPedal::LED_1].Set(effect_enabled ? 1.0f : 0.1f);
        hw.leds[DaisyPedal::LED_2].Set(latch_enabled ? 0.8f : 0.0f);
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
