#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"
#include "../../foundation_examples/field_instrument_ui.h"
#include "../../foundation_examples/field_parameter_banks.h"

#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;
using namespace FieldInstrumentUI;
using namespace FieldParameterBanks;

namespace
{
constexpr int      kMixerKnobCount           = 8;
constexpr float    kCatchThreshold           = 0.02f;
constexpr float    kCapturedLedBrightness    = 1.00f;
constexpr float    kUncapturedLedBrightness  = 0.30f;
constexpr uint32_t kZoomDurationMs           = 1400;
constexpr size_t   kMaxDelaySamples          = 48000;

enum MainParamIndex
{
    MAIN_HYDRA_LEVEL = 0,
    MAIN_EDGE_LEVEL,
    MAIN_HYDRA_TONE,
    MAIN_EDGE_TONE,
    MAIN_HYDRA_SEND,
    MAIN_EDGE_SEND,
    MAIN_DELAY_RETURN,
    MAIN_MASTER,
};

enum AltParamIndex
{
    ALT_HYDRA_TRIM = 0,
    ALT_EDGE_TRIM,
    ALT_HYDRA_HPF,
    ALT_EDGE_HPF,
    ALT_DELAY_TIME,
    ALT_DELAY_FEEDBACK,
    ALT_DELAY_TONE,
    ALT_DUCKING_AMOUNT,
};

enum UtilityParamIndex
{
    UTIL_HYDRA_FADE = 0,
    UTIL_EDGE_FADE,
    UTIL_THROW_AMOUNT,
    UTIL_FREEZE_FEEDBACK,
    UTIL_LIMIT_THRESHOLD,
    UTIL_SATURATION_AMOUNT,
    UTIL_INPUT_GATE,
    UTIL_OUTPUT_CEILING,
};

constexpr float kMainDefaults[kMixerKnobCount] = {
    0.75f, 0.70f, 0.50f, 0.42f, 0.10f, 0.15f, 0.20f, 0.80f,
};

constexpr float kAltDefaults[kMixerKnobCount] = {
    0.50f, 0.50f, 0.38f, 0.42f, 0.36f, 0.25f, 0.55f, 0.35f,
};

constexpr float kUtilityDefaults[kMixerKnobCount] = {
    0.20f, 0.20f, 0.60f, 0.78f, 0.78f, 0.25f, 0.45f, 0.86f,
};

const char* kMainLabels[kMixerKnobCount] = {
    "HydLvl", "EdgeLvl", "HydTone", "EdgeTone",
    "HydSend", "EdgeSend", "DlyRet", "Master",
};

const char* kAltLabels[kMixerKnobCount] = {
    "HydTrim", "EdgeTrim", "HydHPF", "EdgeHPF",
    "DlyTime", "DlyFb", "DlyTone", "DuckAmt",
};

const char* kUtilityLabels[kMixerKnobCount] = {
    "HydFade", "EdgeFade", "Throw", "FreezeFb",
    "Limit", "Saturate", "InputGate", "Ceiling",
};

struct FocusState
{
    char     label[16] = "";
    char     value[24] = "";
    uint32_t until_ms  = 0;
};

struct MixerState
{
    ParamBankSet banks;

    float utility[kMixerKnobCount]          = {};
    bool  utility_captured[kMixerKnobCount] = {};

    ParamBank active_bank       = ParamBank::Main;
    bool      utility_active    = false;
    bool      utility_was_held  = false;

    bool hydra_muted            = false;
    bool edge_muted             = false;
    bool delay_muted            = false;
    bool master_muted           = false;
    bool hydra_solo             = false;
    bool edge_solo              = false;
    bool delay_freeze           = false;
    bool delay_throw            = false;
    bool ducking_enabled        = false;
    bool saturation_enabled     = false;
    int  scene_index            = 0;
};

struct InputGateState
{
    float envelope = 0.0f;
    float gain     = 0.0f;
    bool  open     = false;
};

class OnePoleFilter
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        SetFrequency(1000.0f);
        z_ = 0.0f;
    }

    void SetFrequency(float hz)
    {
        const float clamped = fclamp(hz, 5.0f, sample_rate_ * 0.45f);
        coeff_ = 1.0f - expf(-2.0f * 3.14159265358979323846f * clamped / sample_rate_);
    }

    float Process(float in)
    {
        z_ += coeff_ * (in - z_);
        return z_;
    }

  private:
    float sample_rate_ = 48000.0f;
    float coeff_       = 0.1f;
    float z_           = 0.0f;
};

class HighPassFilter
{
  public:
    void Init(float sample_rate)
    {
        low_a_.Init(sample_rate);
        low_b_.Init(sample_rate);
    }

    void SetFrequency(float hz)
    {
        low_a_.SetFrequency(hz);
        low_b_.SetFrequency(hz);
    }

    float Process(float in)
    {
        const float first = in - low_a_.Process(in);
        return first - low_b_.Process(first);
    }

  private:
    OnePoleFilter low_a_;
    OnePoleFilter low_b_;
};

class ToneTilt
{
  public:
    void Init(float sample_rate)
    {
        low_.Init(sample_rate);
        low_.SetFrequency(950.0f);
    }

    float Process(float in, float tone)
    {
        const float t    = (fclamp(tone, 0.0f, 1.0f) - 0.5f) * 2.0f;
        const float low  = low_.Process(in);
        const float high = in - low;

        const float low_gain  = t < 0.0f ? 1.0f + (-t * 0.55f) : 1.0f - (t * 0.30f);
        const float high_gain = t > 0.0f ? 1.0f + (t * 0.65f) : 1.0f - (-t * 0.45f);
        return low * low_gain + high * high_gain;
    }

  private:
    OnePoleFilter low_;
};

class SmoothGain
{
  public:
    void Reset(float value) { value_ = value; }

    float Process(float target, float seconds, float sample_rate)
    {
        const float time = fclamp(seconds, 0.002f, 1.0f);
        const float coeff = 1.0f - expf(-1.0f / (time * sample_rate));
        value_ += coeff * (target - value_);
        return value_;
    }

  private:
    float value_ = 1.0f;
};

DaisyField           hw;
FieldTriStateKeyLEDs key_leds;
MixerState           state;
FocusState           focus;

DcBlock hydra_dc;
DcBlock edge_dc;
HighPassFilter hydra_hpf;
HighPassFilter edge_hpf;
ToneTilt hydra_tone;
ToneTilt edge_tone;
OnePoleFilter delay_tone_filter;
DelayLine<float, kMaxDelaySamples> DSY_SDRAM_BSS delay_line;
SmoothGain hydra_gain;
SmoothGain edge_gain;
SmoothGain master_gain;
InputGateState hydra_input_gate;
InputGateState edge_input_gate;

float sample_rate_hz = 48000.0f;
float hydra_meter    = 0.0f;
float edge_meter     = 0.0f;
float master_meter   = 0.0f;

float Clamp01(float value)
{
    return fclamp(value, 0.0f, 1.0f);
}

float MainValue(int idx)
{
    return state.banks.Read(ParamBank::Main, idx);
}

float AltValue(int idx)
{
    return state.banks.Read(ParamBank::Alt, idx);
}

float UtilityValue(int idx)
{
    return Clamp01(state.utility[idx]);
}

float LevelGain(float value)
{
    const float v = Clamp01(value);
    return v * v * 1.15f;
}

float TrimGain(float value)
{
    return 0.50f + Clamp01(value) * 1.50f;
}

float HpfFrequency(float value)
{
    const float v = Clamp01(value);
    return 20.0f + v * v * 780.0f;
}

float DelaySamples(float value)
{
    const float v  = Clamp01(value);
    const float ms = 60.0f + v * v * 790.0f;
    return fclamp(ms * sample_rate_hz * 0.001f, 1.0f, static_cast<float>(kMaxDelaySamples - 2));
}

float DelayFeedback(float value)
{
    return Clamp01(value) * 0.72f;
}

float FreezeFeedback()
{
    return 0.70f + UtilityValue(UTIL_FREEZE_FEEDBACK) * 0.29f;
}

float FadeTime(float value)
{
    const float v = Clamp01(value);
    return 0.006f + v * v * 0.45f;
}

float OutputCeiling()
{
    return 0.55f + UtilityValue(UTIL_OUTPUT_CEILING) * 0.43f;
}

float LimitThreshold()
{
    return 0.35f + UtilityValue(UTIL_LIMIT_THRESHOLD) * 0.55f;
}

float SaturationAmount()
{
    return UtilityValue(UTIL_SATURATION_AMOUNT);
}

float InputGateThreshold()
{
    const float v = UtilityValue(UTIL_INPUT_GATE);
    return v < 0.02f ? 0.0f : v * v * 0.22f;
}

void SetFocus(const char* label, const char* value_text)
{
    snprintf(focus.label, sizeof(focus.label), "%s", label);
    snprintf(focus.value, sizeof(focus.value), "%s", value_text);
    focus.until_ms = System::GetNow() + kZoomDurationMs;
}

void FormatPercentText(char* buffer, size_t size, float value)
{
    snprintf(buffer, size, "%d%%", static_cast<int>(Clamp01(value) * 100.0f + 0.5f));
}

void FormatMillisecondsText(char* buffer, size_t size, float seconds)
{
    snprintf(buffer, size, "%.0f ms", seconds * 1000.0f);
}

void FormatHertzText(char* buffer, size_t size, float hz)
{
    snprintf(buffer, size, "%.0f Hz", hz);
}

const char* LabelFor(ParamBank bank, int idx)
{
    return bank == ParamBank::Main ? kMainLabels[idx] : kAltLabels[idx];
}

void FormatBankValue(ParamBank bank, int idx, float value, char* buffer, size_t size)
{
    if(bank == ParamBank::Alt)
    {
        switch(idx)
        {
            case ALT_HYDRA_HPF:
            case ALT_EDGE_HPF: FormatHertzText(buffer, size, HpfFrequency(value)); return;
            case ALT_DELAY_TIME:
            {
                FormatMillisecondsText(buffer, size, DelaySamples(value) / sample_rate_hz);
                return;
            }
            case ALT_DELAY_FEEDBACK:
            {
                FormatPercentText(buffer, size, DelayFeedback(value));
                return;
            }
            default: break;
        }
    }

    FormatPercentText(buffer, size, value);
}

void FormatUtilityValue(int idx, float value, char* buffer, size_t size)
{
    switch(idx)
    {
        case UTIL_HYDRA_FADE:
        case UTIL_EDGE_FADE: FormatMillisecondsText(buffer, size, FadeTime(value)); break;
        case UTIL_FREEZE_FEEDBACK: FormatPercentText(buffer, size, FreezeFeedback()); break;
        case UTIL_LIMIT_THRESHOLD: FormatPercentText(buffer, size, LimitThreshold()); break;
        case UTIL_OUTPUT_CEILING: FormatPercentText(buffer, size, OutputCeiling()); break;
        case UTIL_INPUT_GATE:
        {
            snprintf(buffer, size, "%.3f", InputGateThreshold());
            break;
        }
        default: FormatPercentText(buffer, size, value); break;
    }
}

void ClearUtilityCapture()
{
    for(int i = 0; i < kMixerKnobCount; ++i)
        state.utility_captured[i] = false;
}

void ResetBank(ParamBank bank)
{
    const float* defaults = bank == ParamBank::Main ? kMainDefaults : kAltDefaults;
    for(int i = 0; i < kMixerKnobCount; ++i)
    {
        state.banks.Write(bank, i, defaults[i]);
        state.banks.SetCaptured(bank, i, false);
    }
}

void ResetUtility()
{
    for(int i = 0; i < kMixerKnobCount; ++i)
    {
        state.utility[i]          = kUtilityDefaults[i];
        state.utility_captured[i] = false;
    }
}

void SetScene(int scene)
{
    ResetBank(ParamBank::Main);
    ResetBank(ParamBank::Alt);

    state.scene_index = scene;
    switch(scene)
    {
        case 1:
            state.banks.Write(ParamBank::Main, MAIN_HYDRA_LEVEL, 0.86f);
            state.banks.Write(ParamBank::Main, MAIN_EDGE_LEVEL, 0.45f);
            state.banks.Write(ParamBank::Main, MAIN_HYDRA_SEND, 0.12f);
            state.banks.Write(ParamBank::Main, MAIN_EDGE_SEND, 0.08f);
            SetFocus("Scene", "Hydra lead");
            break;
        case 2:
            state.banks.Write(ParamBank::Main, MAIN_HYDRA_LEVEL, 0.46f);
            state.banks.Write(ParamBank::Main, MAIN_EDGE_LEVEL, 0.84f);
            state.banks.Write(ParamBank::Main, MAIN_HYDRA_SEND, 0.08f);
            state.banks.Write(ParamBank::Main, MAIN_EDGE_SEND, 0.18f);
            SetFocus("Scene", "Edge lead");
            break;
        case 3:
            state.banks.Write(ParamBank::Main, MAIN_HYDRA_SEND, 0.34f);
            state.banks.Write(ParamBank::Main, MAIN_EDGE_SEND, 0.44f);
            state.banks.Write(ParamBank::Main, MAIN_DELAY_RETURN, 0.45f);
            state.banks.Write(ParamBank::Alt, ALT_DELAY_FEEDBACK, 0.62f);
            SetFocus("Scene", "Delay perf");
            break;
        default:
            state.scene_index = 0;
            SetFocus("Scene", "Clean mix");
            break;
    }
}

void ResetPerformanceState()
{
    state.hydra_muted        = false;
    state.edge_muted         = false;
    state.delay_muted        = false;
    state.master_muted       = false;
    state.hydra_solo         = false;
    state.edge_solo          = false;
    state.delay_freeze       = false;
    state.delay_throw        = false;
    state.ducking_enabled    = false;
    state.saturation_enabled = false;
    state.scene_index        = 0;
}

void InitDefaults()
{
    state.banks.Init(0.0f);
    ResetBank(ParamBank::Main);
    ResetBank(ParamBank::Alt);
    ResetUtility();
    ResetPerformanceState();
    hydra_gain.Reset(1.0f);
    edge_gain.Reset(1.0f);
    master_gain.Reset(1.0f);
    SetFocus("field_mixer", "Ready");
}

void SetActiveBank(ParamBank bank)
{
    if(state.active_bank == bank)
        return;
    state.active_bank = bank;
    state.banks.SetActiveBank(bank);
}

void UpdateFocusForBank(ParamBank bank, int idx, float value)
{
    char text[24];
    FormatBankValue(bank, idx, value, text, sizeof(text));
    SetFocus(LabelFor(bank, idx), text);
}

void UpdateFocusForUtility(int idx, float value)
{
    char text[24];
    FormatUtilityValue(idx, value, text, sizeof(text));
    SetFocus(kUtilityLabels[idx], text);
}

void ProcessBankKnobs(ParamBank bank, const float raw_knobs[kMixerKnobCount])
{
    for(int i = 0; i < kMixerKnobCount; ++i)
    {
        if(!state.banks.IsCaptured(bank, i))
            state.banks.CatchIfClose(bank, i, raw_knobs[i], kCatchThreshold);

        if(!state.banks.IsCaptured(bank, i))
            continue;

        const float current = state.banks.Read(bank, i);
        const float updated = Clamp01(raw_knobs[i]);
        if(fabsf(updated - current) > 0.0005f)
        {
            state.banks.Write(bank, i, updated);
            UpdateFocusForBank(bank, i, updated);
        }
    }
}

void ProcessUtilityKnobs(const float raw_knobs[kMixerKnobCount])
{
    for(int i = 0; i < kMixerKnobCount; ++i)
    {
        if(!state.utility_captured[i]
           && fabsf(raw_knobs[i] - state.utility[i]) <= kCatchThreshold)
        {
            state.utility_captured[i] = true;
        }

        if(!state.utility_captured[i])
            continue;

        const float updated = Clamp01(raw_knobs[i]);
        if(fabsf(updated - state.utility[i]) > 0.0005f)
        {
            state.utility[i] = updated;
            UpdateFocusForUtility(i, updated);
        }
    }
}

void HandleKeybedControls()
{
    if(hw.KeyboardRisingEdge(kKeyAIndices[0]))
    {
        state.hydra_muted = !state.hydra_muted;
        SetFocus("Hydra", state.hydra_muted ? "Muted" : "Unmuted");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[1]))
    {
        state.edge_muted = !state.edge_muted;
        SetFocus("Edge", state.edge_muted ? "Muted" : "Unmuted");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[2]))
    {
        state.delay_muted = !state.delay_muted;
        SetFocus("Delay", state.delay_muted ? "Muted" : "Unmuted");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[3]))
    {
        state.master_muted = !state.master_muted;
        SetFocus("Master", state.master_muted ? "Muted" : "Unmuted");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[4]))
    {
        state.hydra_solo = !state.hydra_solo;
        SetFocus("Hydra", state.hydra_solo ? "Solo" : "Solo off");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[5]))
    {
        state.edge_solo = !state.edge_solo;
        SetFocus("Edge", state.edge_solo ? "Solo" : "Solo off");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[6]))
    {
        state.ducking_enabled    = false;
        state.saturation_enabled = false;
        SetFocus("Bypass", "Mods off");
    }

    if(hw.KeyboardRisingEdge(kKeyAIndices[7]))
    {
        ResetPerformanceState();
        SetFocus("Reset", "Perf clear");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[0]))
    {
        state.delay_freeze = !state.delay_freeze;
        SetFocus("Freeze", state.delay_freeze ? "On" : "Off");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[1]))
    {
        state.delay_throw = !state.delay_throw;
        SetFocus("Throw", state.delay_throw ? "On" : "Off");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[2]))
    {
        state.ducking_enabled = !state.ducking_enabled;
        SetFocus("Ducking", state.ducking_enabled ? "On" : "Off");
    }

    if(hw.KeyboardRisingEdge(kKeyBIndices[3]))
    {
        state.saturation_enabled = !state.saturation_enabled;
        SetFocus("Saturate", state.saturation_enabled ? "On" : "Off");
    }

    for(int i = 0; i < 4; ++i)
    {
        if(hw.KeyboardRisingEdge(kKeyBIndices[i + 4]))
            SetScene(i);
    }
}

void UpdateKnobLeds()
{
    for(int i = 0; i < kMixerKnobCount; ++i)
    {
        float value    = 0.0f;
        bool  captured = false;

        if(state.utility_active)
        {
            value    = state.utility[i];
            captured = state.utility_captured[i];
        }
        else
        {
            value    = state.banks.Read(state.active_bank, i);
            captured = state.banks.IsCaptured(state.active_bank, i);
        }

        const float brightness = captured ? kCapturedLedBrightness : kUncapturedLedBrightness;
        hw.led_driver.SetLed(kLedKnobs[i], Clamp01(value) * brightness);
    }

    hw.led_driver.SetLed(kLedSwitches[0], state.active_bank == ParamBank::Alt ? 1.0f : 0.12f);
    hw.led_driver.SetLed(kLedSwitches[1], state.utility_active ? 1.0f : 0.12f);
}

void UpdateKeyLeds()
{
    key_leds.Clear();

    key_leds.SetA(0, state.hydra_muted ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(1, state.edge_muted ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(2, state.delay_muted ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(3, state.master_muted ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetA(4, state.hydra_solo ? KeyLedState::Blink : KeyLedState::Off);
    key_leds.SetA(5, state.edge_solo ? KeyLedState::Blink : KeyLedState::Off);
    key_leds.SetA(6, (state.ducking_enabled || state.saturation_enabled) ? KeyLedState::Blink : KeyLedState::Off);
    key_leds.SetA(7, KeyLedState::Blink);

    key_leds.SetB(0, state.delay_freeze ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(1, state.delay_throw ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(2, state.ducking_enabled ? KeyLedState::On : KeyLedState::Off);
    key_leds.SetB(3, state.saturation_enabled ? KeyLedState::On : KeyLedState::Off);

    for(int i = 0; i < 4; ++i)
        key_leds.SetB(i + 4, state.scene_index == i ? KeyLedState::On : KeyLedState::Off);
}

void RenderOverview()
{
    hw.display.Fill(false);

    char line[32];
    const char* bank_name = state.utility_active
                                ? "UTIL"
                                : (state.active_bank == ParamBank::Main ? "MAIN" : "ALT");
    snprintf(line, sizeof(line), "field_mixer %s", bank_name);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line, Font_7x10, true);

    snprintf(line,
             sizeof(line),
             "H%02d%s%s E%02d%s%s",
             static_cast<int>(MainValue(MAIN_HYDRA_LEVEL) * 99.0f),
             state.hydra_muted ? "M" : " ",
             state.hydra_solo ? "S" : " ",
             static_cast<int>(MainValue(MAIN_EDGE_LEVEL) * 99.0f),
             state.edge_muted ? "M" : " ",
             state.edge_solo ? "S" : " ");
    hw.display.SetCursor(0, 12);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Dly%s%s Duck:%s Sat:%s",
             state.delay_muted ? "M" : " ",
             state.delay_freeze ? "F" : " ",
             state.ducking_enabled ? "Y" : "N",
             state.saturation_enabled ? "Y" : "N");
    hw.display.SetCursor(0, 22);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Meter H%02d E%02d M%02d",
             static_cast<int>(Clamp01(hydra_meter * 95.0f) * 99.0f),
             static_cast<int>(Clamp01(edge_meter * 95.0f) * 99.0f),
             static_cast<int>(Clamp01(master_meter * 85.0f) * 99.0f));
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line, Font_6x8, true);

    for(int i = 0; i < 4; ++i)
    {
        char value_text[16];
        const float value = state.utility_active
                                ? state.utility[i]
                                : state.banks.Read(state.active_bank, i);
        if(state.utility_active)
            FormatUtilityValue(i, value, value_text, sizeof(value_text));
        else
            FormatBankValue(state.active_bank, i, value, value_text, sizeof(value_text));

        const char* label = state.utility_active ? kUtilityLabels[i] : LabelFor(state.active_bank, i);
        snprintf(line, sizeof(line), "%s:%s", label, value_text);
        hw.display.SetCursor(i < 2 ? 0 : 64, 42 + ((i % 2) * 10));
        hw.display.WriteString(line, Font_6x8, true);
    }

    hw.display.Update();
}

void RenderZoom()
{
    hw.display.Fill(false);

    hw.display.SetCursor(0, 0);
    hw.display.WriteString("EDIT", Font_7x10, true);

    hw.display.SetCursor(0, 16);
    hw.display.WriteString(focus.label, Font_7x10, true);

    hw.display.SetCursor(0, 30);
    hw.display.WriteString(focus.value, Font_11x18, true);

    hw.display.SetCursor(0, 54);
    hw.display.WriteString("SW1=Alt  SW2=Utility", Font_6x8, true);
    hw.display.Update();
}

void RenderDisplay()
{
    if(focus.until_ms > System::GetNow())
        RenderZoom();
    else
        RenderOverview();
}

void ProcessUi()
{
    float raw_knobs[kMixerKnobCount];
    for(int i = 0; i < kMixerKnobCount; ++i)
        raw_knobs[i] = Clamp01(hw.GetKnobValue(i));

    const bool sw1_pressed = hw.sw[0].Pressed();
    const bool sw2_pressed = hw.sw[1].Pressed();

    state.utility_active = sw2_pressed;
    if(state.utility_active && !state.utility_was_held)
        ClearUtilityCapture();
    state.utility_was_held = state.utility_active;

    if(state.utility_active)
    {
        ProcessUtilityKnobs(raw_knobs);
    }
    else
    {
        SetActiveBank(sw1_pressed ? ParamBank::Alt : ParamBank::Main);
        ProcessBankKnobs(state.active_bank, raw_knobs);
    }

    HandleKeybedControls();
    UpdateKnobLeds();
    UpdateKeyLeds();
    key_leds.Update(System::GetNow(), 1.0f, 1.0f, 250);
    RenderDisplay();
}

float ProcessInput(float in,
                   DcBlock& dc_block,
                   HighPassFilter& hpf,
                   ToneTilt& tone,
                   float trim,
                   float hpf_hz,
                   float tone_value)
{
    hpf.SetFrequency(hpf_hz);
    float sig = dc_block.Process(in * trim);
    sig       = hpf.Process(sig);
    return tone.Process(sig, tone_value);
}

float ApplyInputGate(float in, InputGateState& gate, float threshold)
{
    if(threshold <= 0.0f)
    {
        gate.open     = true;
        gate.gain     = 1.0f;
        gate.envelope = fabsf(in);
        return in;
    }

    const float mag       = fabsf(in);
    const float env_coeff = mag > gate.envelope ? 0.080f : 0.00025f;
    gate.envelope += (mag - gate.envelope) * env_coeff;

    if(gate.open)
    {
        if(gate.envelope < threshold * 0.45f)
            gate.open = false;
    }
    else if(gate.envelope > threshold)
    {
        gate.open = true;
    }

    const float target = gate.open ? 1.0f : 0.0f;
    const float slew   = target > gate.gain ? 0.0025f : 0.0005f;
    gate.gain += (target - gate.gain) * slew;
    return in * gate.gain;
}

float ApplyOutputShape(float in)
{
    float sig = in;
    if(state.saturation_enabled)
    {
        const float drive = 1.0f + SaturationAmount() * 3.5f;
        sig = tanhf(sig * drive) / tanhf(drive);
    }

    const float threshold = LimitThreshold();
    const float ceiling   = OutputCeiling();
    const float mag       = fabsf(sig);
    if(mag <= threshold)
        return sig;

    const float over    = mag - threshold;
    const float limited = fminf(threshold + over / (1.0f + over * 8.0f), ceiling);
    return sig < 0.0f ? -limited : limited;
}

void WriteCommonOutputs(AudioHandle::OutputBuffer out, size_t frame, float common_mix)
{
    out[0][frame] = common_mix;
    out[1][frame] = common_mix;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    const float hydra_level = LevelGain(MainValue(MAIN_HYDRA_LEVEL));
    const float edge_level  = LevelGain(MainValue(MAIN_EDGE_LEVEL));
    const float hydra_send  = Clamp01(MainValue(MAIN_HYDRA_SEND));
    const float edge_send   = Clamp01(MainValue(MAIN_EDGE_SEND));
    const float dly_return  = Clamp01(MainValue(MAIN_DELAY_RETURN));
    const float master      = LevelGain(MainValue(MAIN_MASTER));

    const float hydra_trim  = TrimGain(AltValue(ALT_HYDRA_TRIM));
    const float edge_trim   = TrimGain(AltValue(ALT_EDGE_TRIM));
    const float hydra_hz    = HpfFrequency(AltValue(ALT_HYDRA_HPF));
    const float edge_hz     = HpfFrequency(AltValue(ALT_EDGE_HPF));
    const float delay_time  = DelaySamples(AltValue(ALT_DELAY_TIME));
    const float base_fb     = DelayFeedback(AltValue(ALT_DELAY_FEEDBACK));
    const float duck_amount = state.ducking_enabled ? Clamp01(AltValue(ALT_DUCKING_AMOUNT)) : 0.0f;
    const float gate_threshold = InputGateThreshold();

    delay_line.SetDelay(delay_time);
    delay_tone_filter.SetFrequency(700.0f + AltValue(ALT_DELAY_TONE) * 9000.0f);

    const bool any_solo     = state.hydra_solo || state.edge_solo;
    const float hydra_goal  = (state.master_muted || state.hydra_muted || (any_solo && !state.hydra_solo)) ? 0.0f : 1.0f;
    const float edge_goal   = (state.master_muted || state.edge_muted || (any_solo && !state.edge_solo)) ? 0.0f : 1.0f;
    const float master_goal = state.master_muted ? 0.0f : 1.0f;

    float hydra_peak = hydra_meter;
    float edge_peak  = edge_meter;
    float mix_peak   = master_meter;

    for(size_t i = 0; i < size; ++i)
    {
        float hydra = ProcessInput(in[0][i],
                                   hydra_dc,
                                   hydra_hpf,
                                   hydra_tone,
                                   hydra_trim,
                                   hydra_hz,
                                   MainValue(MAIN_HYDRA_TONE));
        float edge  = ProcessInput(in[1][i],
                                  edge_dc,
                                  edge_hpf,
                                  edge_tone,
                                  edge_trim,
                                  edge_hz,
                                  MainValue(MAIN_EDGE_TONE));

        hydra = ApplyInputGate(hydra, hydra_input_gate, gate_threshold);
        edge  = ApplyInputGate(edge, edge_input_gate, gate_threshold);

        hydra_peak = fmaxf(hydra_peak * 0.995f, fabsf(hydra));
        edge_peak  = fmaxf(edge_peak * 0.995f, fabsf(edge));

        const float hydra_env = fclamp(fabsf(hydra) * 2.0f, 0.0f, 1.0f);
        const float duck_gain = 1.0f - duck_amount * hydra_env * 0.70f;

        const float hydra_fade = hydra_gain.Process(hydra_goal, FadeTime(UtilityValue(UTIL_HYDRA_FADE)), sample_rate_hz);
        const float edge_fade  = edge_gain.Process(edge_goal, FadeTime(UtilityValue(UTIL_EDGE_FADE)), sample_rate_hz);
        const float master_fade = master_gain.Process(master_goal, 0.010f, sample_rate_hz);

        hydra *= hydra_level * hydra_fade;
        edge *= edge_level * edge_fade * duck_gain;

        const float throw_amount = state.delay_throw ? UtilityValue(UTIL_THROW_AMOUNT) : 0.0f;
        float send = hydra * fclamp(hydra_send + throw_amount, 0.0f, 1.0f);
        send += edge * fclamp(edge_send + throw_amount, 0.0f, 1.0f);

        const float delayed = delay_line.Read();
        const float fb      = state.delay_freeze ? FreezeFeedback() : base_fb;
        const float write_in = state.delay_freeze ? 0.0f : send;
        delay_line.Write(fclamp(write_in + delayed * fb, -1.2f, 1.2f));

        const float dly = state.delay_muted ? 0.0f : delay_tone_filter.Process(delayed) * dly_return;
        float mix = (hydra + edge + dly) * master * master_fade;
        mix       = ApplyOutputShape(mix);

        mix_peak = fmaxf(mix_peak * 0.995f, fabsf(mix));
        WriteCommonOutputs(out, i, mix);
    }

    hydra_meter  = hydra_peak;
    edge_meter   = edge_peak;
    master_meter = mix_peak;
}

} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kRecommendedBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    sample_rate_hz = hw.AudioSampleRate();

    key_leds.Init(&hw);
    InitDefaults();

    hydra_dc.Init(sample_rate_hz);
    edge_dc.Init(sample_rate_hz);
    hydra_hpf.Init(sample_rate_hz);
    edge_hpf.Init(sample_rate_hz);
    hydra_tone.Init(sample_rate_hz);
    edge_tone.Init(sample_rate_hz);
    delay_tone_filter.Init(sample_rate_hz);
    delay_line.Init();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessAllControls();
        ProcessUi();
        System::Delay(1);
    }
}
