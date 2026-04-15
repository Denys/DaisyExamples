#ifndef POD_MULTIFX_PAGES_H
#define POD_MULTIFX_PAGES_H

#include <cmath>
#include <cstdint>

namespace pod_multifx
{
enum PageId
{
    PAGE_TUBE   = 0,
    PAGE_DELAY  = 1,
    PAGE_REVERB = 2,
    PAGE_LFO    = 3,
    PAGE_COUNT  = 4
};

enum LfoWaveform
{
    LFO_WAVE_SINE     = 0,
    LFO_WAVE_TRIANGLE = 1,
    LFO_WAVE_SQUARE   = 2,
    LFO_WAVE_COUNT    = 3
};

static constexpr float    kKnobMovementThreshold = 0.02f;
static constexpr float    kFullScaleSlewMs       = 300.0f;
static constexpr uint32_t kShiftHoldMs           = 250;
static constexpr float    kTubeDriveMin          = 1.0f;
static constexpr float    kTubeDriveMax          = 30.0f;
static constexpr float    kTubeDistortionMin     = 1.0f;
static constexpr float    kTubeDistortionMax     = 25.0f;
static constexpr float    kTubeBiasMaxAbs        = 1.0f;
static constexpr float    kTubeCurvePower        = 2.5f;
static constexpr float    kLfoDepthCurvePower    = 2.5f;
static constexpr float    kLfoFineRange          = 0.15f;
static constexpr float    kLfoMaxPeriodSeconds   = 5.0f;
static constexpr float    kLfoMinPeriodSeconds   = 0.1f;

struct LayerState
{
    float knob_values[2];
    float target_values[2];
    float raw_anchor[2];
    bool  attached[2];
    bool  anchor_initialized[2];
};

struct PageState
{
    LayerState primary_layer;
    LayerState shifted_layer;
    bool       has_shift_layer;
    bool       bypassed;
};

struct Sw1State
{
    bool     pressed;
    bool     shift_active;
    bool     shift_triggered;
    uint32_t held_ms;
    int      press_page;
};

struct ControlState
{
    int       selected_page;
    bool      global_bypass;
    int       lfo_waveform;
    PageState pages[PAGE_COUNT];
    Sw1State  sw1;
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

inline float ExpCurve01(float normalized, float power)
{
    return std::pow(Clamp01(normalized), power);
}

inline float MapExpRange(float normalized, float min_value, float max_value, float power)
{
    return min_value + ExpCurve01(normalized, power) * (max_value - min_value);
}

inline float MapTubeDrive(float normalized)
{
    return MapExpRange(normalized, kTubeDriveMin, kTubeDriveMax, kTubeCurvePower);
}

inline float MapTubeMix(float normalized)
{
    return Clamp01(normalized);
}

inline float MapTubeDistortion(float normalized)
{
    return MapExpRange(
        normalized, kTubeDistortionMin, kTubeDistortionMax, kTubeCurvePower);
}

inline float MapTubeBias(float normalized)
{
    const float signed_normalized = Clamp01(normalized) * 2.0f - 1.0f;
    const float magnitude
        = std::pow(std::fabs(signed_normalized), kTubeCurvePower) * kTubeBiasMaxAbs;
    return signed_normalized < 0.0f ? -magnitude : magnitude;
}

inline float MapDelayTime(float normalized)
{
    return 0.01f + Clamp01(normalized) * 0.09f;
}

inline float MapDelayFeedback(float normalized)
{
    return Clamp01(normalized) * 0.5f;
}

inline float MapReverbDecay(float normalized)
{
    return 0.55f + Clamp01(normalized) * 0.40f;
}

inline float MapReverbMix(float normalized)
{
    return Clamp01(normalized);
}

inline float ApplyFineAdjustment(float coarse_normalized, float fine_normalized)
{
    const float fine_offset = (Clamp01(fine_normalized) - 0.5f) * (2.0f * kLfoFineRange);
    return Clamp01(Clamp01(coarse_normalized) + fine_offset);
}

inline float GetEffectiveLfoAmplitudeNormalized(const ControlState& state)
{
    const PageState& lfo_page = state.pages[PAGE_LFO];
    return ApplyFineAdjustment(
        lfo_page.primary_layer.knob_values[0], lfo_page.shifted_layer.knob_values[0]);
}

inline float GetEffectiveLfoRateNormalized(const ControlState& state)
{
    const PageState& lfo_page = state.pages[PAGE_LFO];
    return ApplyFineAdjustment(
        lfo_page.primary_layer.knob_values[1], lfo_page.shifted_layer.knob_values[1]);
}

inline float MapLfoDepth(float normalized)
{
    return ExpCurve01(normalized, kLfoDepthCurvePower);
}

inline float MapLfoPeriodSeconds(float normalized)
{
    const float ratio = kLfoMinPeriodSeconds / kLfoMaxPeriodSeconds;
    return kLfoMaxPeriodSeconds * std::pow(ratio, Clamp01(normalized));
}

inline LayerState MakeLayerState(float knob1, float knob2)
{
    LayerState state          = {};
    state.knob_values[0]      = Clamp01(knob1);
    state.knob_values[1]      = Clamp01(knob2);
    state.target_values[0]    = state.knob_values[0];
    state.target_values[1]    = state.knob_values[1];
    state.raw_anchor[0]       = 0.0f;
    state.raw_anchor[1]       = 0.0f;
    state.attached[0]         = false;
    state.attached[1]         = false;
    state.anchor_initialized[0] = false;
    state.anchor_initialized[1] = false;
    return state;
}

inline PageState MakePageState(float primary_knob1,
                               float primary_knob2,
                               bool  has_shift_layer,
                               float shifted_knob1 = 0.5f,
                               float shifted_knob2 = 0.5f)
{
    PageState state       = {};
    state.primary_layer   = MakeLayerState(primary_knob1, primary_knob2);
    state.shifted_layer   = MakeLayerState(shifted_knob1, shifted_knob2);
    state.has_shift_layer = has_shift_layer;
    state.bypassed        = false;
    return state;
}

inline ControlState MakeDefaultControlState()
{
    ControlState state       = {};
    state.selected_page      = PAGE_TUBE;
    state.global_bypass      = false;
    state.lfo_waveform       = LFO_WAVE_SINE;
    state.pages[PAGE_TUBE]   = MakePageState(0.35f, 0.60f, true, 0.50f, 0.35f);
    state.pages[PAGE_DELAY]  = MakePageState(0.30f, 0.20f, false);
    state.pages[PAGE_REVERB] = MakePageState(0.60f, 0.25f, false);
    state.pages[PAGE_LFO]    = MakePageState(0.50f, 0.55f, true, 0.50f, 0.50f);
    state.sw1.pressed        = false;
    state.sw1.shift_active   = false;
    state.sw1.shift_triggered = false;
    state.sw1.held_ms        = 0;
    state.sw1.press_page     = PAGE_TUBE;
    return state;
}

inline int WrapPage(int page)
{
    while(page < 0)
    {
        page += PAGE_COUNT;
    }
    while(page >= PAGE_COUNT)
    {
        page -= PAGE_COUNT;
    }
    return page;
}

inline bool PageSupportsShift(const ControlState& state, int page)
{
    return state.pages[WrapPage(page)].has_shift_layer;
}

inline bool IsCurrentPageShiftActive(const ControlState& state)
{
    return state.sw1.shift_active && state.selected_page == WrapPage(state.sw1.press_page)
           && PageSupportsShift(state, state.selected_page);
}

inline LayerState& GetActiveLayer(ControlState& state)
{
    PageState& page = state.pages[state.selected_page];
    if(page.has_shift_layer && IsCurrentPageShiftActive(state))
    {
        return page.shifted_layer;
    }
    return page.primary_layer;
}

inline const LayerState& GetActiveLayer(const ControlState& state)
{
    const PageState& page = state.pages[state.selected_page];
    if(page.has_shift_layer && IsCurrentPageShiftActive(state))
    {
        return page.shifted_layer;
    }
    return page.primary_layer;
}

inline void ArmLayerForReattach(LayerState& layer)
{
    layer.attached[0]           = false;
    layer.attached[1]           = false;
    layer.anchor_initialized[0] = false;
    layer.anchor_initialized[1] = false;
    layer.target_values[0]      = layer.knob_values[0];
    layer.target_values[1]      = layer.knob_values[1];
}

inline void ArmCurrentEditableLayer(ControlState& state)
{
    ArmLayerForReattach(GetActiveLayer(state));
}

inline void SelectPageDelta(ControlState& state, int delta)
{
    if(delta == 0)
    {
        return;
    }

    state.selected_page    = WrapPage(state.selected_page + delta);
    state.sw1.shift_active = state.sw1.shift_triggered
                             && state.selected_page == WrapPage(state.sw1.press_page)
                             && PageSupportsShift(state, state.selected_page);
    ArmCurrentEditableLayer(state);
}

inline void CycleLfoWaveform(ControlState& state)
{
    state.lfo_waveform = (state.lfo_waveform + 1) % LFO_WAVE_COUNT;
}

inline void ToggleCurrentPageBypass(ControlState& state)
{
    state.pages[state.selected_page].bypassed = !state.pages[state.selected_page].bypassed;
}

inline void ToggleGlobalBypass(ControlState& state)
{
    state.global_bypass = !state.global_bypass;
}

inline float SlewToward(float current, float target, float delta_ms)
{
    const float step = Clamp01(delta_ms / kFullScaleSlewMs);
    const float diff = target - current;

    if(std::fabs(diff) <= step)
    {
        return target;
    }

    return current + (diff > 0.0f ? step : -step);
}

inline void UpdateKnobWithReattachAndSlew(LayerState& layer,
                                          int         knob_index,
                                          float       raw_value,
                                          float       delta_ms)
{
    raw_value = Clamp01(raw_value);

    if(!layer.anchor_initialized[knob_index])
    {
        layer.raw_anchor[knob_index]       = raw_value;
        layer.anchor_initialized[knob_index] = true;
        return;
    }

    if(!layer.attached[knob_index]
       && std::fabs(raw_value - layer.raw_anchor[knob_index]) >= kKnobMovementThreshold)
    {
        layer.attached[knob_index] = true;
    }

    if(layer.attached[knob_index])
    {
        layer.target_values[knob_index] = raw_value;
        layer.knob_values[knob_index]
            = SlewToward(layer.knob_values[knob_index], raw_value, delta_ms);
    }
}

inline void UpdateCurrentPageKnobs(ControlState& state,
                                   float         knob1,
                                   float         knob2,
                                   float         delta_ms)
{
    LayerState& layer = GetActiveLayer(state);
    UpdateKnobWithReattachAndSlew(layer, 0, knob1, delta_ms);
    UpdateKnobWithReattachAndSlew(layer, 1, knob2, delta_ms);
}

inline void HandleSw1Press(ControlState& state)
{
    state.sw1.pressed         = true;
    state.sw1.shift_active    = false;
    state.sw1.shift_triggered = false;
    state.sw1.held_ms         = 0;
    state.sw1.press_page      = state.selected_page;
}

inline void AdvanceSw1Hold(ControlState& state, uint32_t delta_ms)
{
    if(!state.sw1.pressed)
    {
        return;
    }

    const int press_page = WrapPage(state.sw1.press_page);
    if(!PageSupportsShift(state, press_page))
    {
        return;
    }

    state.sw1.held_ms += delta_ms;
    if(!state.sw1.shift_triggered && state.sw1.held_ms >= kShiftHoldMs)
    {
        state.sw1.shift_triggered = true;
        state.sw1.shift_active    = state.selected_page == press_page;
        ArmLayerForReattach(state.pages[press_page].shifted_layer);
    }
}

inline bool HandleSw1Release(ControlState& state)
{
    if(!state.sw1.pressed)
    {
        return false;
    }

    const int  press_page         = WrapPage(state.sw1.press_page);
    const bool release_from_shift = state.sw1.shift_triggered
                                    && PageSupportsShift(state, press_page);
    const bool should_toggle      = !release_from_shift;

    state.sw1.pressed         = false;
    state.sw1.shift_active    = false;
    state.sw1.shift_triggered = false;
    state.sw1.held_ms         = 0;

    if(release_from_shift && state.selected_page == press_page)
    {
        ArmLayerForReattach(state.pages[press_page].primary_layer);
    }

    if(should_toggle)
    {
        state.pages[press_page].bypassed = !state.pages[press_page].bypassed;
        return true;
    }

    return false;
}
} // namespace pod_multifx

#endif
