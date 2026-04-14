#ifndef POD_MULTIFX_PAGES_H
#define POD_MULTIFX_PAGES_H

#include <cmath>

namespace pod_multifx
{
enum FxPage
{
    FX_OVERDRIVE = 0,
    FX_DELAY     = 1,
    FX_REVERB    = 2,
    FX_WAHWAH    = 3,
    FX_COUNT     = 4
};

static const float kSoftTakeoverThreshold = 0.025f;

struct EffectPageState
{
    float knob_values[2];
    bool  bypassed;
    bool  captured[2];
};

struct ControlState
{
    int             selected_page;
    bool            global_bypass;
    EffectPageState pages[FX_COUNT];
};

inline float Clamp01(float value)
{
    if(value < 0.0f)
    {
        return 0.0f;
    }
    if(value > 1.0f)
    {
        return 1.0f;
    }
    return value;
}

inline EffectPageState MakeEffectPageState(float knob1, float knob2, bool captured)
{
    EffectPageState state = {};
    state.knob_values[0]  = Clamp01(knob1);
    state.knob_values[1]  = Clamp01(knob2);
    state.bypassed        = false;
    state.captured[0]     = captured;
    state.captured[1]     = captured;
    return state;
}

inline float DelayTimeToKnob(float delay_time_seconds)
{
    return Clamp01((delay_time_seconds - 0.01f) / 0.99f);
}

inline float DelayFeedbackToKnob(float feedback)
{
    return Clamp01(feedback / 0.95f);
}

inline float ReverbFeedbackToKnob(float feedback)
{
    return Clamp01((feedback - 0.5f) / 0.49f);
}

inline float ReverbLpFreqToKnob(float frequency_hz)
{
    return Clamp01((frequency_hz - 1000.0f) / 17000.0f);
}

inline float WahFreqToKnob(float frequency_hz)
{
    return Clamp01((frequency_hz - 200.0f) / 1800.0f);
}

inline ControlState MakeDefaultControlState()
{
    ControlState state        = {};
    state.selected_page       = FX_OVERDRIVE;
    state.global_bypass       = false;
    state.pages[FX_OVERDRIVE] = MakeEffectPageState(0.5f, 0.0f, true);
    state.pages[FX_DELAY]
        = MakeEffectPageState(DelayTimeToKnob(0.3f), DelayFeedbackToKnob(0.5f), false);
    state.pages[FX_REVERB] = MakeEffectPageState(
        ReverbFeedbackToKnob(0.85f), ReverbLpFreqToKnob(10000.0f), false);
    state.pages[FX_WAHWAH]
        = MakeEffectPageState(WahFreqToKnob(500.0f), 0.8f, false);
    return state;
}

inline int WrapPage(int page)
{
    while(page < 0)
    {
        page += FX_COUNT;
    }
    while(page >= FX_COUNT)
    {
        page -= FX_COUNT;
    }
    return page;
}

inline void ArmSoftTakeover(EffectPageState& page)
{
    page.captured[0] = false;
    page.captured[1] = false;
}

inline void SelectPageDelta(ControlState& state, int delta)
{
    if(delta == 0)
    {
        return;
    }

    state.selected_page = WrapPage(state.selected_page + delta);
    ArmSoftTakeover(state.pages[state.selected_page]);
}

inline void ToggleCurrentPageBypass(ControlState& state)
{
    state.pages[state.selected_page].bypassed = !state.pages[state.selected_page].bypassed;
}

inline void ToggleGlobalBypass(ControlState& state)
{
    state.global_bypass = !state.global_bypass;
}

inline bool UpdateKnobWithSoftTakeover(EffectPageState& page,
                                       int              knob_index,
                                       float            raw_value,
                                       float            threshold = kSoftTakeoverThreshold)
{
    raw_value = Clamp01(raw_value);

    if(page.captured[knob_index])
    {
        page.knob_values[knob_index] = raw_value;
        return true;
    }

    if(std::fabs(raw_value - page.knob_values[knob_index]) <= threshold)
    {
        page.captured[knob_index]    = true;
        page.knob_values[knob_index] = raw_value;
        return true;
    }

    return false;
}

inline void UpdateCurrentPageKnobs(ControlState& state, float knob1, float knob2)
{
    EffectPageState& page = state.pages[state.selected_page];
    UpdateKnobWithSoftTakeover(page, 0, knob1);
    UpdateKnobWithSoftTakeover(page, 1, knob2);
}
} // namespace pod_multifx

#endif
