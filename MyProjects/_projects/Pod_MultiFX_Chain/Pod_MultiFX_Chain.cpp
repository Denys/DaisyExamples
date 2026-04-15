/**
 * Pod_MultiFX_Chain
 *
 * Four-page serial multi-effect pedal for Daisy Pod.
 *
 * Signal chain:
 *   LFO -> Tube -> Delay -> Reverb
 *
 * Edit page order:
 *   Tube -> Delay -> Reverb -> LFO
 *
 * Controls:
 * - Encoder turn: change selected effect page
 * - Encoder push on LFO page: cycle waveform Sine -> Triangle -> Square
 * - Knob 1 / Knob 2: edit the selected page with stored position recall and smooth reattach
 * - Button 1 (SW1) short press: bypass the selected page
 * - Button 1 (SW1) hold on Tube: temporary shift to Bias / Distortion controls
 * - Button 1 (SW1) hold on LFO: temporary shift to fine Amplitude / Rate trims
 * - Button 2 (SW2): global bypass for the full chain
 * - RGB LED 1: selected page color, dimmed when the selected page or whole chain is bypassed
 * - RGB LED 2: LFO waveform color with brightness tracking LFO rate
 *
 * Tube page:
 * - default: Drive / Mix
 * - while SW1 is held: Bias / Distortion
 *
 * LFO page:
 * - default: Amplitude coarse / Rate coarse
 * - while SW1 is held: Amplitude fine / Rate fine
 *
 * Stored positions and smooth reattach:
 * - each page keeps its stored settings when you leave it
 * - Tube default and shift layers keep independent stored values
 * - LFO coarse and fine layers keep independent stored values
 * - switching page or logical layer leaves the stored value active immediately
 * - a knob reattaches after real movement is detected
 * - once reattached, the effective value slews toward the physical knob target
 *
 * Note:
 * - DAFX WahWah is intentionally removed from the live pedal for now
 * - keep it in mind as a future revisit item
 */

#include "daisy_pod.h"
#include "daisysp.h"
#include "effects/tube.h"
#include "pod_multifx_pages.h"

using namespace daisy;
using namespace daisysp;

namespace
{
constexpr size_t kMaxDelaySamples = 48000;
constexpr float  kDelayWetMix     = 0.35f;
constexpr float  kFixedReverbLpHz = 12000.0f;

DaisyPod hw;

Oscillator                                lfo;
Tube                                      tube;
DelayLine<float, kMaxDelaySamples> DSY_SDRAM_BSS delay_line;
ReverbSc                                  reverb;

pod_multifx::ControlState control_state = pod_multifx::MakeDefaultControlState();

float sample_rate = 48000.0f;

int GetOscillatorWaveform(int waveform)
{
    switch(waveform)
    {
        case pod_multifx::LFO_WAVE_TRIANGLE: return Oscillator::WAVE_TRI;
        case pod_multifx::LFO_WAVE_SQUARE: return Oscillator::WAVE_SQUARE;
        case pod_multifx::LFO_WAVE_SINE:
        default: return Oscillator::WAVE_SIN;
    }
}

float GetLfoRateBrightness()
{
    const float rate_normalized = pod_multifx::GetEffectiveLfoRateNormalized(control_state);
    return 0.10f + 0.90f * rate_normalized;
}

void ApplyStoredParameters()
{
    const pod_multifx::PageState& tube_page   = control_state.pages[pod_multifx::PAGE_TUBE];
    const pod_multifx::PageState& delay_page  = control_state.pages[pod_multifx::PAGE_DELAY];
    const pod_multifx::PageState& reverb_page = control_state.pages[pod_multifx::PAGE_REVERB];

    lfo.SetFreq(1.0f / pod_multifx::MapLfoPeriodSeconds(
                           pod_multifx::GetEffectiveLfoRateNormalized(control_state)));
    lfo.SetWaveform(GetOscillatorWaveform(control_state.lfo_waveform));

    tube.SetDrive(pod_multifx::MapTubeDrive(tube_page.primary_layer.knob_values[0]));
    tube.SetMix(pod_multifx::MapTubeMix(tube_page.primary_layer.knob_values[1]));
    tube.SetBias(pod_multifx::MapTubeBias(tube_page.shifted_layer.knob_values[0]));
    tube.SetDistortion(
        pod_multifx::MapTubeDistortion(tube_page.shifted_layer.knob_values[1]));

    delay_line.SetDelay(sample_rate
                        * pod_multifx::MapDelayTime(delay_page.primary_layer.knob_values[0]));

    reverb.SetFeedback(
        pod_multifx::MapReverbDecay(reverb_page.primary_layer.knob_values[0]));
}

void UpdateLed()
{
    const int   selected_page    = control_state.selected_page;
    const bool  global_bypass    = control_state.global_bypass;
    const bool  selected_bypass  = control_state.pages[selected_page].bypassed;
    const bool  lfo_bypassed     = control_state.pages[pod_multifx::PAGE_LFO].bypassed;
    const float led1_brightness  = (global_bypass || selected_bypass) ? 0.12f : 1.0f;
    const float led2_brightness  = GetLfoRateBrightness() * ((global_bypass || lfo_bypassed) ? 0.12f : 1.0f);

    switch(selected_page)
    {
        case pod_multifx::PAGE_TUBE: hw.led1.Set(led1_brightness, 0.0f, 0.0f); break;
        case pod_multifx::PAGE_DELAY: hw.led1.Set(0.0f, led1_brightness, 0.0f); break;
        case pod_multifx::PAGE_REVERB: hw.led1.Set(0.0f, 0.0f, led1_brightness); break;
        case pod_multifx::PAGE_LFO:
            hw.led1.Set(led1_brightness, led1_brightness * 0.65f, 0.0f);
            break;
        default: hw.led1.Set(0.0f, 0.0f, 0.0f); break;
    }

    switch(control_state.lfo_waveform)
    {
        case pod_multifx::LFO_WAVE_TRIANGLE: hw.led2.Set(0.0f, 0.0f, led2_brightness); break;
        case pod_multifx::LFO_WAVE_SQUARE: hw.led2.Set(led2_brightness, 0.0f, 0.0f); break;
        case pod_multifx::LFO_WAVE_SINE:
        default: hw.led2.Set(0.0f, led2_brightness, 0.0f); break;
    }

    hw.UpdateLeds();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // BUG-005 rule: analog controls must be processed at audio callback rate.
    hw.ProcessAnalogControls();

    const float control_dt_ms
        = 1000.0f * static_cast<float>(size) / sample_rate;
    pod_multifx::UpdateCurrentPageKnobs(
        control_state, hw.knob1.Value(), hw.knob2.Value(), control_dt_ms);
    ApplyStoredParameters();

    const bool  global_bypass   = control_state.global_bypass;
    const bool  lfo_bypassed    = control_state.pages[pod_multifx::PAGE_LFO].bypassed;
    const bool  tube_bypassed   = control_state.pages[pod_multifx::PAGE_TUBE].bypassed;
    const bool  delay_bypassed  = control_state.pages[pod_multifx::PAGE_DELAY].bypassed;
    const bool  reverb_bypassed = control_state.pages[pod_multifx::PAGE_REVERB].bypassed;
    const float lfo_depth = pod_multifx::MapLfoDepth(
        pod_multifx::GetEffectiveLfoAmplitudeNormalized(control_state));
    const float delay_feedback = pod_multifx::MapDelayFeedback(
        control_state.pages[pod_multifx::PAGE_DELAY].primary_layer.knob_values[1]);
    const float reverb_mix = pod_multifx::MapReverbMix(
        control_state.pages[pod_multifx::PAGE_REVERB].primary_layer.knob_values[1]);

    for(size_t i = 0; i < size; i++)
    {
        const float input       = in[0][i];
        const float lfo_unipolar = 0.5f * (lfo.Process() + 1.0f);
        float       signal      = input;

        if(global_bypass)
        {
            out[0][i] = input;
            out[1][i] = input;
            continue;
        }

        if(!lfo_bypassed)
        {
            const float lfo_gain = (1.0f - lfo_depth) + lfo_depth * lfo_unipolar;
            signal *= lfo_gain;
        }

        if(!tube_bypassed)
        {
            signal = tube.Process(signal);
        }

        if(!delay_bypassed)
        {
            const float delayed = delay_line.Read();
            delay_line.Write(signal + delayed * delay_feedback);
            signal = signal + delayed * kDelayWetMix;
        }

        if(!reverb_bypassed)
        {
            float wet_left  = 0.0f;
            float wet_right = 0.0f;
            reverb.Process(signal, signal, &wet_left, &wet_right);
            out[0][i] = wet_left * reverb_mix + signal * (1.0f - reverb_mix);
            out[1][i] = wet_right * reverb_mix + signal * (1.0f - reverb_mix);
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

    lfo.Init(sample_rate);
    lfo.SetAmp(1.0f);

    tube.Init(sample_rate);
    tube.SetHighPassPole(0.99f);
    tube.SetLowPassPole(0.5f);

    delay_line.Init();

    reverb.Init(sample_rate);
    reverb.SetLpFreq(kFixedReverbLpHz);

    ApplyStoredParameters();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    UpdateLed();

    while(1)
    {
        hw.ProcessDigitalControls();

        if(hw.button1.RisingEdge())
        {
            pod_multifx::HandleSw1Press(control_state);
        }

        if(hw.button1.Pressed())
        {
            pod_multifx::AdvanceSw1Hold(control_state, 1);
        }

        const int page_delta = hw.encoder.Increment();
        if(page_delta != 0)
        {
            pod_multifx::SelectPageDelta(control_state, page_delta);
        }

        if(hw.encoder.RisingEdge() && control_state.selected_page == pod_multifx::PAGE_LFO)
        {
            pod_multifx::CycleLfoWaveform(control_state);
        }

        if(hw.button1.FallingEdge())
        {
            pod_multifx::HandleSw1Release(control_state);
        }

        if(hw.button2.RisingEdge())
        {
            pod_multifx::ToggleGlobalBypass(control_state);
        }

        UpdateLed();
        System::Delay(1);
    }
}
