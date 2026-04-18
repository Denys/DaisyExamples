/** @brief DaisyPedal passthrough and relay bypass example. */
#include "daisy_pedal.h"
#include "daisysp.h"
#include <cstdio>

using namespace daisy;
using namespace daisysp;

DaisyPedal hw;
TapTempo   tap_tempo;

bool     relay_pending = false;
bool     relay_target  = false;
uint8_t  relay_stage   = 0;
uint32_t relay_time_ms = 0;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        tap_tempo.Process();
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

static void StartRelayTransition(bool bypass_enabled)
{
    relay_target  = bypass_enabled;
    relay_pending = true;
    relay_stage   = 0;
    relay_time_ms = hw.seed.system.GetNow() + 6;
    hw.SetAudioMute(true);
}

static void ServiceRelayTransition()
{
    if(!relay_pending)
        return;

    const uint32_t now = hw.seed.system.GetNow();
    if(now < relay_time_ms)
        return;

    if(relay_stage == 0)
    {
        hw.SetAudioBypass(relay_target);
        relay_stage   = 1;
        relay_time_ms = now + 6;
    }
    else
    {
        hw.SetAudioMute(false);
        relay_pending = false;
    }
}

static void UpdateDisplay()
{
    char line1[32];
    char line2[32];
    char line3[32];

    snprintf(line1, sizeof(line1), "Pedal Passthru");
    snprintf(line2,
             sizeof(line2),
             "Mode: %s",
             hw.AudioBypassEnabled() ? "Relay Bypass" : "DSP Active");
    snprintf(line3, sizeof(line3), "Tap: %3d BPM", (int)(tap_tempo.GetTempoBpm() + 0.5f));

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line1, Font_7x10, true);
    hw.display.SetCursor(0, 18);
    hw.display.WriteString(line2, Font_7x10, true);
    hw.display.SetCursor(0, 36);
    hw.display.WriteString(line3, Font_7x10, true);
    hw.display.Update();
}

int main(void)
{
    uint32_t last_display_update = 0;

    hw.Init();
    tap_tempo.Init(hw.AudioSampleRate(), 120.0f);
    tap_tempo.SetMinMaxBpm(40.0f, 240.0f);
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();
        hw.midi.Listen();

        if(hw.switches[DaisyPedal::SW_BYPASS].RisingEdge() && !relay_pending)
        {
            StartRelayTransition(!hw.AudioBypassEnabled());
        }

        if(hw.switches[DaisyPedal::SW_AUX].RisingEdge())
        {
            tap_tempo.TriggerTap();
        }

        ServiceRelayTransition();

        const float phase = tap_tempo.GetPhase();
        const float pulse = phase < 0.18f ? 1.0f - (phase / 0.18f) : 0.0f;
        hw.leds[DaisyPedal::LED_1].Set(hw.AudioBypassEnabled() ? 0.15f : 1.0f);
        hw.leds[DaisyPedal::LED_2].Set(pulse);
        hw.UpdateLeds();

        const uint32_t now = hw.seed.system.GetNow();
        if(now - last_display_update >= 40)
        {
            UpdateDisplay();
            last_display_update = now;
        }

        hw.DelayMs(5);
    }
}
