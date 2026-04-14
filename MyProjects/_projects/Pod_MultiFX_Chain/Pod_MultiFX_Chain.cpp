/**
 * Pod_MultiFX_Chain
 *
 * Four-stage serial multi-effect pedal for Daisy Pod.
 *
 * Signal chain:
 *   Overdrive -> Delay -> WahWah -> Reverb
 *
 * Edit page order:
 *   Overdrive -> Delay -> Reverb -> WahWah
 *
 * Controls:
 * - Encoder turn: change selected effect page
 * - Knob 1 / Knob 2: edit the selected page with soft takeover
 * - Button 1 (SW1): bypass the selected effect page
 * - Button 2 (SW2): global bypass for the full chain
 * - RGB LED: page color, dimmed when the selected page or the whole chain is bypassed
 *
 * Soft takeover:
 * - each page keeps its stored settings when you leave it
 * - switching pages does not jump to the physical knob positions
 * - each knob must cross the stored value before it starts editing the new page
 */

#include "daisy_pod.h"
#include "daisysp.h"
#include "effects/wahwah.h"
#include "pod_multifx_pages.h"

using namespace daisy;
using namespace daisysp;

namespace
{
constexpr size_t kMaxDelaySamples = 48000;

DaisyPod hw;

Overdrive                                 overdrive;
DelayLine<float, kMaxDelaySamples> DSY_SDRAM_BSS delay_line;
ReverbSc                                  reverb;
WahWah                                    wah;

pod_multifx::ControlState control_state = pod_multifx::MakeDefaultControlState();

float sample_rate = 48000.0f;

float KnobToDelayTime(float normalized)
{
    return 0.01f + pod_multifx::Clamp01(normalized) * 0.99f;
}

float KnobToDelayFeedback(float normalized)
{
    return pod_multifx::Clamp01(normalized) * 0.95f;
}

float KnobToReverbFeedback(float normalized)
{
    return 0.5f + pod_multifx::Clamp01(normalized) * 0.49f;
}

float KnobToReverbLpFreq(float normalized)
{
    return 1000.0f + pod_multifx::Clamp01(normalized) * 17000.0f;
}

float KnobToWahFreq(float normalized)
{
    return 200.0f + pod_multifx::Clamp01(normalized) * 1800.0f;
}

void ApplyStoredParameters()
{
    const pod_multifx::EffectPageState& overdrive_page
        = control_state.pages[pod_multifx::FX_OVERDRIVE];
    const pod_multifx::EffectPageState& delay_page
        = control_state.pages[pod_multifx::FX_DELAY];
    const pod_multifx::EffectPageState& reverb_page
        = control_state.pages[pod_multifx::FX_REVERB];
    const pod_multifx::EffectPageState& wah_page
        = control_state.pages[pod_multifx::FX_WAHWAH];

    overdrive.SetDrive(overdrive_page.knob_values[0]);
    delay_line.SetDelay(sample_rate * KnobToDelayTime(delay_page.knob_values[0]));
    reverb.SetFeedback(KnobToReverbFeedback(reverb_page.knob_values[0]));
    reverb.SetLpFreq(KnobToReverbLpFreq(reverb_page.knob_values[1]));
    wah.SetFreq(KnobToWahFreq(wah_page.knob_values[0]));
    wah.SetDepth(wah_page.knob_values[1]);
}

void UpdateSelectedPageKnobs(float knob1, float knob2)
{
    const int selected_page = control_state.selected_page;
    pod_multifx::EffectPageState& selected = control_state.pages[selected_page];
    pod_multifx::UpdateKnobWithSoftTakeover(selected, 0, knob1);
    pod_multifx::UpdateKnobWithSoftTakeover(selected, 1, knob2);
}

void UpdateLed()
{
    const int  selected_page   = control_state.selected_page;
    const bool global_bypass   = control_state.global_bypass;
    const bool selected_bypass = control_state.pages[selected_page].bypassed;
    const float brightness = (global_bypass || selected_bypass) ? 0.1f : 1.0f;

    switch(selected_page)
    {
        case pod_multifx::FX_OVERDRIVE: hw.led1.Set(brightness, 0.0f, 0.0f); break;
        case pod_multifx::FX_DELAY: hw.led1.Set(0.0f, brightness, 0.0f); break;
        case pod_multifx::FX_REVERB: hw.led1.Set(0.0f, 0.0f, brightness); break;
        case pod_multifx::FX_WAHWAH: hw.led1.Set(0.0f, brightness, brightness); break;
        default: hw.led1.Set(0.0f, 0.0f, 0.0f); break;
    }

    hw.led1.Update();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // BUG-005 rule: analog controls must be processed at audio callback rate.
    hw.ProcessAnalogControls();

    UpdateSelectedPageKnobs(hw.knob1.Value(), hw.knob2.Value());
    ApplyStoredParameters();

    const bool  global_bypass = control_state.global_bypass;
    const bool  overdrive_bypassed
        = control_state.pages[pod_multifx::FX_OVERDRIVE].bypassed;
    const bool  delay_bypassed = control_state.pages[pod_multifx::FX_DELAY].bypassed;
    const bool  reverb_bypassed = control_state.pages[pod_multifx::FX_REVERB].bypassed;
    const bool  wah_bypassed = control_state.pages[pod_multifx::FX_WAHWAH].bypassed;
    const float delay_feedback
        = KnobToDelayFeedback(control_state.pages[pod_multifx::FX_DELAY].knob_values[1]);

    for(size_t i = 0; i < size; i++)
    {
        const float input = in[0][i];
        float       signal = input;

        if(global_bypass)
        {
            out[0][i] = input;
            out[1][i] = input;
            continue;
        }

        if(!overdrive_bypassed)
        {
            signal = overdrive.Process(signal);
        }

        if(!delay_bypassed)
        {
            const float delayed = delay_line.Read();
            signal              = signal + delayed * delay_feedback;
            delay_line.Write(signal);
        }

        if(!wah_bypassed)
        {
            signal = wah.Process(signal);
        }

        if(!reverb_bypassed)
        {
            float wet_left  = 0.0f;
            float wet_right = 0.0f;
            reverb.Process(signal, signal, &wet_left, &wet_right);
            out[0][i] = wet_left;
            out[1][i] = wet_right;
        }
        else
        {
            out[0][i] = signal;
            out[1][i] = signal;
        }
    }
}
} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    sample_rate = hw.AudioSampleRate();

    overdrive.Init();
    delay_line.Init();
    reverb.Init(sample_rate);
    wah.Init(sample_rate);
    wah.SetRes(5.0f);

    ApplyStoredParameters();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    UpdateLed();

    while(1)
    {
        hw.ProcessDigitalControls();

        const int page_delta = hw.encoder.Increment();
        if(page_delta != 0)
        {
            pod_multifx::SelectPageDelta(control_state, page_delta);
        }

        if(hw.button1.RisingEdge())
        {
            pod_multifx::ToggleCurrentPageBypass(control_state);
        }

        if(hw.button2.RisingEdge())
        {
            pod_multifx::ToggleGlobalBypass(control_state);
        }

        UpdateLed();
        System::Delay(1);
    }
}
