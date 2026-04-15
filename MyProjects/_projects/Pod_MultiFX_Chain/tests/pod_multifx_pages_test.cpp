#include "../pod_multifx_pages.h"

#include <cassert>
#include <cmath>

namespace
{
bool NearlyEqual(float a, float b, float epsilon = 1.0e-6f)
{
    return std::fabs(a - b) <= epsilon;
}
}

int main()
{
    using namespace pod_multifx;

    {
        ControlState state = MakeDefaultControlState();
        assert(state.selected_page == PAGE_TUBE);
        assert(state.lfo_waveform == LFO_WAVE_SINE);

        SelectPageDelta(state, 1);
        assert(state.selected_page == PAGE_DELAY);

        SelectPageDelta(state, 2);
        assert(state.selected_page == PAGE_LFO);

        SelectPageDelta(state, -1);
        assert(state.selected_page == PAGE_REVERB);
    }

    {
        ControlState state = MakeDefaultControlState();
        const float   saved_delay_time = state.pages[PAGE_DELAY].primary_layer.knob_values[0];
        const float   saved_delay_fb   = state.pages[PAGE_DELAY].primary_layer.knob_values[1];

        SelectPageDelta(state, 1);
        assert(state.selected_page == PAGE_DELAY);
        assert(!state.pages[PAGE_DELAY].primary_layer.attached[0]);
        assert(!state.pages[PAGE_DELAY].primary_layer.attached[1]);
        assert(!state.pages[PAGE_DELAY].primary_layer.anchor_initialized[0]);
        assert(!state.pages[PAGE_DELAY].primary_layer.anchor_initialized[1]);

        UpdateCurrentPageKnobs(state, 0.90f, 0.10f, 1.0f);
        assert(state.pages[PAGE_DELAY].primary_layer.anchor_initialized[0]);
        assert(state.pages[PAGE_DELAY].primary_layer.anchor_initialized[1]);
        assert(NearlyEqual(state.pages[PAGE_DELAY].primary_layer.knob_values[0], saved_delay_time));
        assert(NearlyEqual(state.pages[PAGE_DELAY].primary_layer.knob_values[1], saved_delay_fb));

        UpdateCurrentPageKnobs(state, 0.95f, 0.05f, 40.0f);
        assert(state.pages[PAGE_DELAY].primary_layer.attached[0]);
        assert(state.pages[PAGE_DELAY].primary_layer.attached[1]);
        assert(state.pages[PAGE_DELAY].primary_layer.knob_values[0] > saved_delay_time);
        assert(state.pages[PAGE_DELAY].primary_layer.knob_values[0] < 0.95f);
        assert(state.pages[PAGE_DELAY].primary_layer.knob_values[1] < saved_delay_fb);
        assert(state.pages[PAGE_DELAY].primary_layer.knob_values[1] > 0.05f);

        UpdateCurrentPageKnobs(state, 0.95f, 0.05f, 400.0f);
        assert(NearlyEqual(state.pages[PAGE_DELAY].primary_layer.knob_values[0], 0.95f));
        assert(NearlyEqual(state.pages[PAGE_DELAY].primary_layer.knob_values[1], 0.05f));
    }

    {
        ControlState state = MakeDefaultControlState();
        const float   saved_drive = state.pages[PAGE_TUBE].primary_layer.knob_values[0];
        const float   saved_mix   = state.pages[PAGE_TUBE].primary_layer.knob_values[1];

        UpdateCurrentPageKnobs(state, 0.62f, 0.41f, 1.0f);
        assert(state.pages[PAGE_TUBE].primary_layer.anchor_initialized[0]);
        assert(state.pages[PAGE_TUBE].primary_layer.anchor_initialized[1]);
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[0], saved_drive));
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[1], saved_mix));

        UpdateCurrentPageKnobs(state, 0.67f, 0.46f, 300.0f);
        assert(state.pages[PAGE_TUBE].primary_layer.attached[0]);
        assert(state.pages[PAGE_TUBE].primary_layer.attached[1]);
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[0], 0.67f));
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[1], 0.46f));

        const float saved_bias = state.pages[PAGE_TUBE].shifted_layer.knob_values[0];
        const float saved_dist = state.pages[PAGE_TUBE].shifted_layer.knob_values[1];

        HandleSw1Press(state);
        AdvanceSw1Hold(state, kShiftHoldMs);
        assert(IsCurrentPageShiftActive(state));
        assert(!state.pages[PAGE_TUBE].shifted_layer.attached[0]);
        assert(!state.pages[PAGE_TUBE].shifted_layer.attached[1]);

        UpdateCurrentPageKnobs(state, 0.87f, 0.19f, 1.0f);
        assert(state.pages[PAGE_TUBE].shifted_layer.anchor_initialized[0]);
        assert(state.pages[PAGE_TUBE].shifted_layer.anchor_initialized[1]);
        assert(NearlyEqual(state.pages[PAGE_TUBE].shifted_layer.knob_values[0], saved_bias));
        assert(NearlyEqual(state.pages[PAGE_TUBE].shifted_layer.knob_values[1], saved_dist));

        UpdateCurrentPageKnobs(state, 0.92f, 0.24f, 30.0f);
        assert(state.pages[PAGE_TUBE].shifted_layer.attached[0]);
        assert(state.pages[PAGE_TUBE].shifted_layer.attached[1]);
        assert(state.pages[PAGE_TUBE].shifted_layer.knob_values[0] > saved_bias);
        assert(state.pages[PAGE_TUBE].shifted_layer.knob_values[0] < 0.92f);
        assert(state.pages[PAGE_TUBE].shifted_layer.knob_values[1] < saved_dist);
        assert(state.pages[PAGE_TUBE].shifted_layer.knob_values[1] > 0.24f);

        UpdateCurrentPageKnobs(state, 0.92f, 0.24f, 400.0f);
        assert(NearlyEqual(state.pages[PAGE_TUBE].shifted_layer.knob_values[0], 0.92f));
        assert(NearlyEqual(state.pages[PAGE_TUBE].shifted_layer.knob_values[1], 0.24f));

        const bool bypass_toggled = HandleSw1Release(state);
        assert(!bypass_toggled);
        assert(!IsCurrentPageShiftActive(state));
        assert(!state.pages[PAGE_TUBE].primary_layer.attached[0]);
        assert(!state.pages[PAGE_TUBE].primary_layer.attached[1]);
        assert(!state.pages[PAGE_TUBE].primary_layer.anchor_initialized[0]);
        assert(!state.pages[PAGE_TUBE].primary_layer.anchor_initialized[1]);
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[0], 0.67f));
        assert(NearlyEqual(state.pages[PAGE_TUBE].primary_layer.knob_values[1], 0.46f));
    }

    {
        ControlState state = MakeDefaultControlState();

        SelectPageDelta(state, 3);
        assert(state.selected_page == PAGE_LFO);
        assert(!state.pages[PAGE_LFO].primary_layer.attached[0]);
        assert(!state.pages[PAGE_LFO].primary_layer.attached[1]);
        assert(!state.pages[PAGE_LFO].shifted_layer.attached[0]);
        assert(!state.pages[PAGE_LFO].shifted_layer.attached[1]);

        CycleLfoWaveform(state);
        assert(state.lfo_waveform == LFO_WAVE_TRIANGLE);
        CycleLfoWaveform(state);
        assert(state.lfo_waveform == LFO_WAVE_SQUARE);
        CycleLfoWaveform(state);
        assert(state.lfo_waveform == LFO_WAVE_SINE);

        const float saved_amp_coarse  = state.pages[PAGE_LFO].primary_layer.knob_values[0];
        const float saved_rate_coarse = state.pages[PAGE_LFO].primary_layer.knob_values[1];
        const float saved_amp_fine    = state.pages[PAGE_LFO].shifted_layer.knob_values[0];
        const float saved_rate_fine   = state.pages[PAGE_LFO].shifted_layer.knob_values[1];

        UpdateCurrentPageKnobs(state, 0.95f, 0.90f, 1.0f);
        assert(NearlyEqual(state.pages[PAGE_LFO].primary_layer.knob_values[0], saved_amp_coarse));
        assert(NearlyEqual(state.pages[PAGE_LFO].primary_layer.knob_values[1], saved_rate_coarse));

        UpdateCurrentPageKnobs(state, 0.98f, 0.93f, 400.0f);
        assert(state.pages[PAGE_LFO].primary_layer.attached[0]);
        assert(state.pages[PAGE_LFO].primary_layer.attached[1]);
        assert(NearlyEqual(state.pages[PAGE_LFO].primary_layer.knob_values[0], 0.98f));
        assert(NearlyEqual(state.pages[PAGE_LFO].primary_layer.knob_values[1], 0.93f));

        HandleSw1Press(state);
        AdvanceSw1Hold(state, kShiftHoldMs);
        assert(IsCurrentPageShiftActive(state));

        UpdateCurrentPageKnobs(state, 0.99f, 0.01f, 1.0f);
        assert(NearlyEqual(state.pages[PAGE_LFO].shifted_layer.knob_values[0], saved_amp_fine));
        assert(NearlyEqual(state.pages[PAGE_LFO].shifted_layer.knob_values[1], saved_rate_fine));

        UpdateCurrentPageKnobs(state, 0.84f, 0.16f, 400.0f);
        assert(state.pages[PAGE_LFO].shifted_layer.attached[0]);
        assert(state.pages[PAGE_LFO].shifted_layer.attached[1]);
        assert(NearlyEqual(state.pages[PAGE_LFO].shifted_layer.knob_values[0], 0.84f));
        assert(NearlyEqual(state.pages[PAGE_LFO].shifted_layer.knob_values[1], 0.16f));

        assert(GetEffectiveLfoAmplitudeNormalized(state)
               > state.pages[PAGE_LFO].primary_layer.knob_values[0]);
        assert(GetEffectiveLfoRateNormalized(state)
               < state.pages[PAGE_LFO].primary_layer.knob_values[1]);

        const bool bypass_toggled = HandleSw1Release(state);
        assert(!bypass_toggled);
        assert(!IsCurrentPageShiftActive(state));
        assert(!state.pages[PAGE_LFO].primary_layer.attached[0]);
        assert(!state.pages[PAGE_LFO].primary_layer.attached[1]);
    }

    {
        ControlState state = MakeDefaultControlState();
        assert(!state.pages[PAGE_TUBE].bypassed);
        assert(!state.global_bypass);

        HandleSw1Press(state);
        const bool tube_bypass_toggled = HandleSw1Release(state);
        assert(tube_bypass_toggled);
        assert(state.pages[PAGE_TUBE].bypassed);

        SelectPageDelta(state, 1);
        HandleSw1Press(state);
        const bool delay_bypass_toggled = HandleSw1Release(state);
        assert(delay_bypass_toggled);
        assert(state.pages[PAGE_DELAY].bypassed);

        SelectPageDelta(state, 2);
        HandleSw1Press(state);
        const bool lfo_bypass_toggled = HandleSw1Release(state);
        assert(lfo_bypass_toggled);
        assert(state.pages[PAGE_LFO].bypassed);

        ToggleGlobalBypass(state);
        assert(state.global_bypass);
    }

    {
        assert(NearlyEqual(MapDelayTime(0.0f), 0.01f));
        assert(NearlyEqual(MapDelayTime(1.0f), 0.10f));
        assert(NearlyEqual(MapDelayFeedback(0.0f), 0.0f));
        assert(NearlyEqual(MapDelayFeedback(1.0f), 0.5f));
        assert(NearlyEqual(MapTubeMix(0.25f), 0.25f));
        assert(NearlyEqual(MapTubeBias(0.5f), 0.0f));
        assert(MapTubeDrive(0.5f) < 0.5f * (kTubeDriveMax + kTubeDriveMin));
        assert(MapTubeDistortion(0.5f)
               < 0.5f * (kTubeDistortionMax + kTubeDistortionMin));
        assert(MapTubeBias(0.75f) > 0.0f);
        assert(MapTubeBias(0.75f) < 0.5f);
        assert(NearlyEqual(MapLfoDepth(0.0f), 0.0f));
        assert(NearlyEqual(MapLfoDepth(1.0f), 1.0f));
        assert(MapLfoDepth(0.5f) < 0.5f);
        assert(NearlyEqual(MapLfoPeriodSeconds(0.0f), 5.0f));
        assert(NearlyEqual(MapLfoPeriodSeconds(1.0f), 0.1f));
        assert(MapLfoPeriodSeconds(0.5f) > 0.1f);
        assert(MapLfoPeriodSeconds(0.5f) < 5.0f);
    }

    return 0;
}
